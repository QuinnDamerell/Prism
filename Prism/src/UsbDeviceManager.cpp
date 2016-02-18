#include <iostream>

#include "libusb.h"
#include "UsbDeviceManager.h"

int UsbDeviceManager::Setup()
{
    // Init lib usb.
    if (libusb_init(&m_libusbContext))
    {
        std::clog << "Error initializing USB library!\n";
        return 7;
    }

    libusb_set_debug(m_libusbContext, 3);

    // Register for hotplug callbacks and enumerate the devices.
    int status = libusb_hotplug_register_callback(m_libusbContext,
        libusb_hotplug_event(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT),
        LIBUSB_HOTPLUG_ENUMERATE,
        LIBUSB_HOTPLUG_MATCH_ANY,
        LIBUSB_HOTPLUG_MATCH_ANY,
        LIBUSB_HOTPLUG_MATCH_ANY,
        cbNewHotplugDeivce, this, 0);

    if (status != LIBUSB_SUCCESS || !libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG))
    {
        // If we can't hotplug make a thread to fake it.
        m_fakeHotPlugThread.reset(new std::thread(std::bind(&UsbDeviceManager::FakeHotPlugThreadLoop, this)));
    }
}

/*static*/ int UsbDeviceManager::cbNewHotplugDeivce(libusb_context *ctx, libusb_device *device, libusb_hotplug_event event, void *user_data)
{
    UsbDeviceManager *self = static_cast<UsbDeviceManager*>(user_data);

    std::lock_guard<std::mutex> lock(self->m_deivceListLock);

    if (event & LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED)
    {
        self->UsbDeviceArrived(device);
    }
    if (event & LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT)
    {
        self->UsbDeviceLeft(device);
    }
    return false;
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
        std::clog << "Device found but couldn't be opened.\n";
        return;
    }

    // Write the config
    device->WriteConfiguration();
    
    // Write color correction
    //dev->writeColorCorrection(mColor);

    // Add to our list
    m_deviceList.push_back(device);
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

void UsbDeviceManager::FakeHotPlugThreadLoop()
{
    while (true)
    {
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

        // Sleep
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}