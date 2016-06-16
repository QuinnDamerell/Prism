#include <iostream>
#include <exception>
#include <memory>
#include <chrono>
#include <ctime>

#include "Prism.h"
#include "libusb.h"

#include "Panel.h"
#include "OutputBitmap.h"
#include "Drawables/Drawable.h"
#include "Fadables/Fader.h"

#include "Gems/SolidColorGem.h"
#include "Gems/RandomColorGem.h"
#include "Gems/ColorPeaks.h"
#include "Gems/RunningPixel.h"
#include "Gems/SwipeColorGem.h"
#include "Gems/ExpandingDropsGem.h"
#include "Gems/RowRunnerGem.h"

using namespace Rapcom;
using namespace LightFx;
using namespace LightFx::Drawables;
using namespace LightFx::Fadables;
using namespace Gems;

// Setup
void Prism::AlignCrystals()
{
    // Start up the usb device manager, this will watch for new usb devices.
    m_usbDeviceManager = std::make_shared<UsbDeviceManager>();
    if (int ret = m_usbDeviceManager->Setup(std::dynamic_pointer_cast<IDeviceDiscoverListener>(shared_from_this())) < 0)
    {
        std::cout << "Failed to setup usb device manager";
        throw std::runtime_error("Failed to setup usb device manager");
    }

    // Create the main light panel
    m_lightPanel = std::make_shared<Panel>(8, 8);

    // Register for render complete callbacks
    m_lightPanel->SetPanelRenderedCallback(std::dynamic_pointer_cast<IPanelRenderedCallback>(shared_from_this()));

    // Create the driver for the panel
    m_driver = std::make_shared<ConstantRateDriver>();

    // Add us so we can update before the render
    m_driver->AddDriveable(std::dynamic_pointer_cast<IDrivable>(shared_from_this()));

    // Add the panel
    m_driver->AddDriveable(std::dynamic_pointer_cast<IDrivable>(m_lightPanel)); 

    // Make our rapcom host
    m_rapcomHost = std::make_shared<RapcomHost>(GetWeakPtr<IControlCommandHandler>());
    m_rapcomHost->Initialize();
}

// Starts the prism
void Prism::Prismify()
{
    if (!m_driver)
    {
        throw std::runtime_error("Setup hasn't been called!");
    }

    const uint8_t gemCount = 7;
    for (uint8_t i = 0; i < gemCount; i++)
    {
        // Create the Gem
        IGemPtr gem;
        switch (i)
        {
        case 6:
            gem = std::make_shared<SolidColorGem>();
            break;
        case 5:
            gem = std::make_shared<RandomColorGem>();
            break;
        case 4:
            gem = std::make_shared<SwipeColorGem>();
            break;
        case 3:
            gem = std::make_shared<ExpandingDropsGem>();
            break;
        case 2:
            gem = std::make_shared<RowRunnerGem>();
            break;
        case 1:
            gem = std::make_shared<RunningPixel>();
            break;
        case 0:
        default:
            gem = std::make_shared<ColorPeaks>();
            break;
        }

        // Make a new panel for the Gem
        IDrawablePtr gemLayer = std::make_shared<Drawable>();

        // Add the layer to our base
        m_lightPanel->AddDrawable(gemLayer, i);

        // Setup the gem
        gem->OnSetup(gemLayer);

        // And add it to the list
        m_gemList.insert(m_gemList.end(), GemPanelPair(gem, gemLayer));

        // Turn off the panel
        gemLayer->SetIntensity(0);

        // Make a fader and add it to the drawable
        IFaderPtr fader = std::make_shared<Fader>();
        fader->SetFinishedCallback(GetSharedPtr<ITimelineObjectCallback>());
        gemLayer->SetFader(fader);
    }

    // Set our initial intensity
    IntensityChanged(GetDoubleOrDefault(m_rapcomHost->GetConfig(), "Intensity", 0.85));

    // Get the running times
    m_maxActiveGemTimeSeconds = GetIntOrDefault(m_rapcomHost->GetConfig(), "MaxGemRunningTimeSeconds", GEM_RUNNING_TIME_MAX_SECONDS);
    m_minActiveGemTimeSeconds = GetIntOrDefault(m_rapcomHost->GetConfig(), "MinGemRunningTimeSeconds", GEM_RUNNING_TIME_MIN_SECONDS);

    // Update our enabled list
    UpdateEnabledGemList();

    // Save the config
    m_rapcomHost->SaveConfig();

    // Run at 60fps.
    m_driver->Start(milliseconds(16));
}

void Prism::OnTick(uint64_t tick, milliseconds elapsedTime)
{
    // Check if we need to switch gems
    CheckForGemSwtich(elapsedTime);

    // Check for an active hours change
    CheckForActiveHoursChange();

    // If we have a panel we are fading out keep animating them until
    // they are faded out
    if (auto local = m_animateOutGem)
    {
        local->OnTick(tick, elapsedTime);
    }

    // Send the tick to the active panel
    if (m_activeGemIndex != -1)
    {
        m_gemList[m_activeGemIndex].first->OnTick(tick, elapsedTime);
    }
}

// Fired when a device is added.
void Prism::OnDeviceAdded(IWriteablePixelEndpointPtr device)
{
    // Replace any device that may exists.
    m_pixelEndpoint = device;
}

// Fired when a device is removed.
void Prism::OnDeviceRemoved(IWriteablePixelEndpointPtr device)
{
    // When the device is removed clear our current device.
    m_pixelEndpoint = std::weak_ptr<IWriteablePixelEndpoint>();
}

// Fired when the panel renders
void Prism::OnPanelRendered()
{
    // Try to lock the endpoint
    if (auto pixelEndpoint = m_pixelEndpoint.lock())
    {
        // Get the output bitmap from the panel.
        // Note! This bitmap owns the pixel array and will delete it when
        // it goes out of scope.
        OutputBitmapPtr bitmap = m_lightPanel->GetOutputBitmap();

        // Send the pixel buffer to the device to display.
        pixelEndpoint->WritePixels(bitmap->GetPixelArray(), bitmap->GetPixelArrayLength());
    }      
}

// Checks if we need to switch gems.
void Prism::CheckForGemSwtich(milliseconds elapsedTime)
{
    // Remove the elapsedTime from our time remain
    m_activeGemTimeRemaing -= elapsedTime;

    // Check if we need to switch and make sure we have more than one gem
    if (m_activeGemTimeRemaing.count() <= 0 || m_forceGemSwitch)
    {
        // Update the time reaming
        std::uniform_int_distribution<int> timeRemainDist(m_minActiveGemTimeSeconds * 1000, m_maxActiveGemTimeSeconds * 1000);
        m_activeGemTimeRemaing = milliseconds(timeRemainDist(m_randomDevice));

        // Unset the update flag
        m_forceGemSwitch = false;

        // Ensure there is a gem we can switch to
        int8_t onlyEnabledGem = -1;
        bool isAtLeastTwoEnabled = false;
        for (int i = 0; i < m_enabledGemList.size(); i++)
        {
            if (m_enabledGemList[i])
            {
                if (onlyEnabledGem == -1)
                {
                    onlyEnabledGem = i;
                    continue;
                }
                isAtLeastTwoEnabled = true;
                break;
            }
        }

        uint8_t newActiveGemIndex = -1;

        // Figure out what to switch to.
        // If we have at least two pick on randomly.
        if (isAtLeastTwoEnabled)
        {
            // Find the next gem number, make sure it isn't the same as this gem
            std::uniform_int_distribution<int> nextGemDist(0, m_gemList.size() - 1);
            do
            {
                newActiveGemIndex = nextGemDist(m_randomDevice);
            } while (newActiveGemIndex == m_activeGemIndex && !m_enabledGemList[newActiveGemIndex]);
        }
        // If we only have one, set it active
        else if (onlyEnabledGem != -1)
        {
            newActiveGemIndex = onlyEnabledGem;
        }    

        // Now switch the gems
        // Tell the current gem it is going away.
        if (m_activeGemIndex != -1)
        {
            m_gemList[m_activeGemIndex].first->OnDeactivated();

            // Fade the panel out
            IFaderPtr fader = m_gemList[m_activeGemIndex].second->GetFader();
            fader->SetFrom(1.0);
            fader->SetTo(0);
            fader->SetDuration(milliseconds(3000));

            // Set the panel as our animate fade out so the panel keeps animating as it fades away
            m_animateOutGem = m_gemList[m_activeGemIndex].first;
        }

        // Update the active number
        m_activeGemIndex = newActiveGemIndex;

        if (m_activeGemIndex != -1)
        {
            // Activate it
            m_gemList[m_activeGemIndex].first->OnActivated();

            // Fade in the panel        
            IFaderPtr fader = m_gemList[m_activeGemIndex].second->GetFader();
            fader->SetFrom(m_gemList[m_activeGemIndex].second->GetIntensity());
            fader->SetTo(1.0);
            fader->SetDuration(milliseconds(3000));
        }
    }
}

// Checks if active hours should be changed.
void Prism::CheckForActiveHoursChange()
{
    // This can be quite a bit of work, so only do it once 30 seconds or so.
    m_activeHoursTimeCheck++;
    if (m_activeHoursTimeCheck < 1800)
    {
        return;
    }
    m_activeHoursTimeCheck = 0;

    // Get the current time.
    std::time_t timeNow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm* localTime = std::localtime(&timeNow);

    // Check if we hit one of the boundaries of active hours
    uint8_t offHour = GetIntOrDefault(m_rapcomHost->GetConfig(), "ActiveHoursOffHour", 1);
    uint8_t offMin = GetIntOrDefault(m_rapcomHost->GetConfig(), "ActiveHoursOffMin", 0);
    std::string offTimeOfDay;
    GetStringOrDefault(m_rapcomHost->GetConfig(), "ActiveHoursOffTimeOfDay", "am", offTimeOfDay);
    uint8_t onHour = GetIntOrDefault(m_rapcomHost->GetConfig(), "ActiveHoursOnHour", 1);
    uint8_t onMin = GetIntOrDefault(m_rapcomHost->GetConfig(), "ActiveHoursOnMin", 0);
    std::string onTimeOfDay;
    GetStringOrDefault(m_rapcomHost->GetConfig(), "ActiveHoursOnTimeOfDay", "am", onTimeOfDay);

    // Account for 24 hour time.
    if (offTimeOfDay == "pm")
    {
        offHour += 12;
    }
    // Account for 24 hour time.
    if (onTimeOfDay == "pm")
    {
        onHour += 12;
    }

    // If are the times are the same we are disabled.
    if (offHour == onHour && offMin == onMin)
    {
        return;
    }

    // Note we only change the intensity on the time boundaries. This is so it can be overwritten by the
    // user.

    // Check if we should so something.
    if (localTime->tm_hour == offHour && localTime->tm_min == offMin && m_lightPanel->GetIntensity() != 0)
    {
        // Capture the value we turned off at.
        m_rapcomHost->GetConfig().AddMember("ActiveHoursLastIntensity", m_lightPanel->GetIntensity(), m_rapcomHost->GetConfig().GetAllocator());

        // We need to turn off.
        IntensityChanged(0);
    }

    // Check if we should so something.
    if (localTime->tm_hour == onHour && localTime->tm_min == onMin && m_lightPanel->GetIntensity() == 0)
    {
        // Get the value we turned off at
        double lastOffValue = GetDoubleOrDefault(m_rapcomHost->GetConfig(), "ActiveHoursLastIntensity", 0.85);

        // We need to turn off.
        IntensityChanged(lastOffValue);
    }
}

// Sets the intensity of the main panel.
void Prism::IntensityChanged(double intensity)
{
    // Create a fader if we don't have one.
    if (!m_lightPanel->GetFader())
    {
        FaderPtr fader = std::make_shared<Fader>(0, 1, milliseconds(0));
        m_lightPanel->SetFader(fader);
    }

    IFaderPtr fader = m_lightPanel->GetFader();

    // And now set the fade
    fader->SetFrom(m_lightPanel->GetIntensity());
    fader->SetTo(intensity);
    fader->SetDuration(milliseconds(2000));

    std::cout << "Setting intensity to " << intensity << std::endl;

    // Make sure the config is updated
    m_rapcomHost->GetConfig().AddMember("Intensity", intensity, m_rapcomHost->GetConfig().GetAllocator());
}

void Prism::EnabledGemsChanged()
{
    // Update our gem list
    UpdateEnabledGemList();

    // Make sure our active gem isn't disabled
    if (m_activeGemIndex == -1 || !m_enabledGemList[m_activeGemIndex])
    {
        // If so force a update
        m_forceGemSwitch = true;
    }

    std::cout << "Enabled Gems Changed" << std::endl;
}

// Running time changed
void Prism::GemRunningTimeChanged()
{
    m_maxActiveGemTimeSeconds = GetIntOrDefault(m_rapcomHost->GetConfig(), "MaxGemRunningTimeSeconds", GEM_RUNNING_TIME_MAX_SECONDS);
    m_minActiveGemTimeSeconds = GetIntOrDefault(m_rapcomHost->GetConfig(), "MinGemRunningTimeSeconds", GEM_RUNNING_TIME_MAX_SECONDS);
    int64_t timeRemain = m_activeGemTimeRemaing.count() / 1000;
    if (timeRemain > m_maxActiveGemTimeSeconds)
    {
        m_activeGemTimeRemaing = milliseconds(m_maxActiveGemTimeSeconds * 1000);
    }

    std::cout << "Gem run time changed" << std::endl;
}

// Active hours updated
void Prism::ActiveHoursUpdate()
{
    // Don't do anything for now.
}

// Fired when a panel fade is complete
void Prism::OnTimelineFinished(ITimelineObjectPtr timeline)
{
    IFaderPtr fader = std::dynamic_pointer_cast<IFader>(timeline);
    if (fader && fader->GetTo() < 0.001)
    {
        // If this was a fade out clear our animate out gem ptr
        m_animateOutGem = nullptr;
    }
}

void Prism::UpdateEnabledGemList()
{
    // Get the config
    rapidjson::Document& config = m_rapcomHost->GetConfig();

    // Clear our local and size it correctly
    m_enabledGemList.clear();
    m_enabledGemList.resize(m_gemList.size());

    // The array
    rapidjson::Value jsonArray;

    // Check if it already exists
    auto arrayIter = config.FindMember("EnabledGems");
    if (arrayIter != config.MemberEnd() && arrayIter->value.IsArray())
    {
        // Get the array
        jsonArray = arrayIter->value.GetArray();
    }
    else
    {
        // Create a new array and add it to the document
        rapidjson::Value newArray;
        newArray.SetArray();
        jsonArray = newArray;
    }

    // Make sure the array is the correct size
    while (jsonArray.Size() < m_gemList.size())
    {
        jsonArray.PushBack(true, config.GetAllocator());
    }

    // Now read the array
    for (int i = 0; i < m_enabledGemList.size(); i++)
    {
        m_enabledGemList[i] = jsonArray[i].GetBool();
    }

    // Set our new list
    config.RemoveMember("EnabledGems");
    config.AddMember("EnabledGems", jsonArray, config.GetAllocator());
}