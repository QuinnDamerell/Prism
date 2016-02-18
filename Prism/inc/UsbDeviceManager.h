#pragma once

#include <mutex>
#include <vector>
#include <libusb.h>



#include "Common.h"
#include "FadeCandyDevice.h"

DECLARE_SMARTPOINTER(UsbDeviceManager);
class UsbDeviceManager
{
public:
    UsbDeviceManager() {}
    ~UsbDeviceManager() {}

    int Setup();

private:
    // Fired by lib usb when a new device is found
    static int LIBUSB_CALL cbNewHotplugDeivce(libusb_context *ctx, libusb_device *device, libusb_hotplug_event event, void *user_data);

    // Called when a usb is added
    void UsbDeviceArrived(libusb_device *device);

    // Called when a usb is lost
    void UsbDeviceLeft(libusb_device *device);
    void UsbDeviceLeft(std::vector<FadeCandyDevicePtr>::iterator iter);

    // Used to fake the hot plug logic
    void FakeHotPlugThreadLoop();

    // The context for lib usb.
    libusb_context* m_libusbContext;

    // Locks the usb events    
    std::mutex m_deivceListLock;

    // A thread that gives us fake hot plug if not supported
    std::unique_ptr<std::thread> m_fakeHotPlugThread;

    // Holds a list of current devices.
    std::vector<FadeCandyDevicePtr> m_deviceList;
};
