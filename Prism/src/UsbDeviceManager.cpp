#include <iostream>
#include <ratio>

#include "libusb.h"
#include "UsbDeviceManager.h"
#include "IWriteablePixelEndpoint.h"

int UsbDeviceManager::Setup(IDeviceDiscoverListenerWeakPtr discoverListener)
{
    // Init lib usb.
    if (libusb_init(&m_libusbContext))
    {
        std::clog << "Error initializing USB library!\n";
        return 7;
    }

    // Grab the listener
    m_discoverListener = discoverListener;

    // Start our worker thread.
    m_workerThread.reset(new std::thread(std::bind(&UsbDeviceManager::WorkerLoop, this)));    
}

void UsbDeviceManager::UsbDeviceArrived(libusb_device *newDevice)
{
    // Test to see if this is a device we want
    if (!FadeCandyDevice::Probe(newDevice))
    {
        return;
    }

    FadeCandyDevicePtr device = std::make_shared<FadeCandyDevice>(newDevice);

    // Open the device
    int ret = device->Open();
    if (ret < 0)
    {
        std::clog << "Device found but couldn't be opened. (this is normal if it only happens once or twice)\n";
        return;
    }

    // Write the config
    device->WriteConfiguration();
    
    // Add to our list
    m_deviceList.push_back(device);

    if (IDeviceDiscoverListenerPtr listener = m_discoverListener.lock())
    {
        listener->OnDeviceAdded(std::dynamic_pointer_cast<IWriteablePixelEndpoint>(device));
    }
}

void UsbDeviceManager::UsbDeviceLeft(libusb_device *device)
{
    for (std::vector<FadeCandyDevicePtr>::iterator i = m_deviceList.begin(), e = m_deviceList.end(); i != e; ++i)
    {
        FadeCandyDevicePtr dev = *i;
        if (dev->GetDevice() == device)
        {
            UsbDeviceLeft(i);
            break;
        }
    }
}

void UsbDeviceManager::UsbDeviceLeft(std::vector<FadeCandyDevicePtr>::iterator iter)
{
    FadeCandyDevicePtr device = *iter;
    std::clog << "USB device removed.\n";
    
    m_deviceList.erase(iter);
}

void UsbDeviceManager::WorkerLoop()
{
    using namespace std::chrono;

    while (true)
    {
        // Check if we should look for new devices.
        high_resolution_clock::duration timeSinceLastCheck = high_resolution_clock::now() - m_lastDeviceCheckTime;
        auto timeSinceLastCheckMs = duration_cast<milliseconds>(timeSinceLastCheck);

        if (timeSinceLastCheckMs.count() > 1000)
        {
            // Update the time checked
            m_lastDeviceCheckTime = high_resolution_clock::now();

            // Try to get the device list.
            libusb_device **list;
            ssize_t listSize;
            listSize = libusb_get_device_list(m_libusbContext, &list);
            if (listSize < 0)
            {
                std::clog << "Error polling for USB devices: " << libusb_strerror(libusb_error(listSize)) << "\n";
                return;
            }

            // Grab the device lock
            std::lock_guard<std::mutex> lock(m_deivceListLock);

            // Look for devices that were added
            for (ssize_t listItem = 0; listItem < listSize; ++listItem) {
                bool isNew = true;
                for (std::vector<FadeCandyDevicePtr>::iterator i = m_deviceList.begin(), e = m_deviceList.end(); i != e; ++i)
                {
                    FadeCandyDevicePtr device = *i;
                    if (device->GetDevice() == list[listItem])
                    {
                        isNew = false;
                    }
                }
                if (isNew)
                {
                    UsbDeviceArrived(list[listItem]);
                }
            }

            // Look for devices that were removed
            for (std::vector<FadeCandyDevicePtr>::iterator i = m_deviceList.begin(), e = m_deviceList.end(); i != e; ++i)
            {
                FadeCandyDevicePtr device = *i;
                libusb_device *usbdev = device->GetDevice();
                bool isRemoved = true;

                for (ssize_t listItem = 0; listItem < listSize; ++listItem)
                {
                    if (usbdev == list[listItem])
                    {
                        isRemoved = false;
                    }
                }

                if (isRemoved)
                {
                    UsbDeviceLeft(i);
                }
            }

            // Free the list
            libusb_free_device_list(list, true);
        }

        // Now sit and wait for events that need to be handled from lib usb.
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 10000;

        // Check for events
        int r = libusb_handle_events_timeout_completed(m_libusbContext, &timeout, 0);
        if (r) 
        {
            std::clog << "Error handling USB events: " << libusb_strerror(libusb_error(r)) << "\n";
        }
    }
}