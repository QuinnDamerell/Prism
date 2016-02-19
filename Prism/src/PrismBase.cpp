#include <iostream>

#include "libusb.h"
#include "PrismBase.h"

int count = 0;
// Sets us up.
int PrismBase::Setup()
{
    // Start up the usb device manager
    m_usbDeviceManager = std::make_unique<UsbDeviceManager>();
    if (int ret = m_usbDeviceManager->Setup(std::dynamic_pointer_cast<IDeviceDiscoverListener>(shared_from_this())) < 0)
    {
        std::cout << "Failed to setup usb device manager";
        return ret;
    }

    while (true)
    {
        if (IWriteablePixelEndpointPtr endpoint = m_pixelEndpoint.lock())
        {
            uint8_t pixels[] = { count, count, count, count, count, count, count, count, count, count, count, count };
            endpoint->WritePixels(pixels, 12);
            count++;

            if (count > 255)
            {
                count = 0;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }
}

void PrismBase::OnDeviceAdded(IWriteablePixelEndpointPtr device)
{
    m_pixelEndpoint = device;
}

void PrismBase::OnDeviceRemoved(IWriteablePixelEndpointPtr device)
{
    m_pixelEndpoint = std::weak_ptr<IWriteablePixelEndpoint>();
}