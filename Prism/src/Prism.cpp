#include <iostream>
#include <exception>
#include <memory>

#include "Prism.h"
#include "libusb.h"

#include "Panel.h"
#include "OutputBitmap.h"

#include "Gems/ColorPeaks.h"

using namespace LightFx;
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
}

// Starts the prism
void Prism::Prismify()
{
    if (!m_driver)
    {
        throw std::runtime_error("Setup hasn't been called!");
    }

    // Create the gems
    IGemPtr gem = std::make_shared<ColorPeaks>();
    gem->OnSetup(m_lightPanel);
    m_gemList.insert(m_gemList.end(), gem);

    // Run at 60fps.
    m_driver->Start(milliseconds(16));
}

void Prism::OnTick(uint64_t tick, milliseconds elapsedTime)
{
    // Check if we need to switch gems
    CheckForGemSwtich(elapsedTime);

    // Send the tick to the active panel
    m_gemList[m_activeGemIndex]->OnPreRender(tick, elapsedTime);
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
            m_gemList[m_activeGemIndex]->OnDeactivated();
        }

        // Update the active number
        m_activeGemIndex = newActiveGemIndex;

        // Activate it
        m_gemList[m_activeGemIndex]->OnActivated();
    }
}