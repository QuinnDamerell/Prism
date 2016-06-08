#include <iostream>
#include <exception>
#include <memory>

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

    // Make the web server
    m_webServer = std::make_shared<WebServer>();
    m_webServer->Setup();
}

// Starts the prism
void Prism::Prismify()
{
    if (!m_driver)
    {
        throw std::runtime_error("Setup hasn't been called!");
    }

    const uint8_t gemCount = 6; // Exclude SoldColorGem
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
            gem = std::make_shared<ColorPeaks>();
            break;
        case 3:
            gem = std::make_shared<SwipeColorGem>();
            break;
        case 2:
            gem = std::make_shared<RunningPixel>();
            break;
        case 1:
            gem = std::make_shared<ExpandingDropsGem>();
            break;
        case 0:
        default:
            gem = std::make_shared<RowRunnerGem>();
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

    // Run at 60fps.
    m_driver->Start(milliseconds(16));

    // Run the web server
    m_webServer->Start();
}

void Prism::OnTick(uint64_t tick, milliseconds elapsedTime)
{
    // Check if we need to switch gems
    CheckForGemSwtich(elapsedTime);

    // If we have a panel we are fading out keep animating them until
    // they are faded out
    if (auto local = m_animateOutGem)
    {
        local->OnTick(tick, elapsedTime);
    }

    // Send the tick to the active panel
    m_gemList[m_activeGemIndex].first->OnTick(tick, elapsedTime);
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
    if (m_activeGemTimeRemaing.count() <= 0)
    {
        // Update the time reaming
        std::uniform_int_distribution<int> timeRemainDist(GEM_RUNNING_TIME_MIN_SECONDS * 1000, GEM_RUNNING_TIME_MAX_SECONDS * 1000);
        m_activeGemTimeRemaing = milliseconds(timeRemainDist(m_randomDevice));

        // Find the next gem number, make sure it isn't the same as this gem
        std::uniform_int_distribution<int> nextGemDist(0, m_gemList.size() - 1);
        uint8_t newActiveGemIndex = 0;
        do 
        {
            newActiveGemIndex = nextGemDist(m_randomDevice);
        } while (newActiveGemIndex == m_activeGemIndex && m_gemList.size() != 1);

        // Now switch the gems
        // Tell the current gem it is going away.
        if (m_activeGemIndex < m_gemList.size())
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

        // Activate it
        m_gemList[m_activeGemIndex].first->OnActivated();

        // Fade in the panel        
        IFaderPtr fader = m_gemList[m_activeGemIndex].second->GetFader();
        fader->SetFrom(m_gemList[m_activeGemIndex].second->GetIntensity());
        fader->SetTo(1.0);
        fader->SetDuration(milliseconds(3000));
    }
}

// Sets the intensity of the main panel.
void Prism::SetIntensity(double intensity)
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