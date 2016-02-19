#pragma once

#include <chrono>
#include <mutex>
#include <vector>
#include <libusb.h>

#include "Common.h"
#include "FadeCandyDevice.h"
#include "IDeviceDiscoveryListener.h"

DECLARE_SMARTPOINTER(UsbDeviceManager);
class UsbDeviceManager
{
public:
    UsbDeviceManager() {}
    ~UsbDeviceManager() { m_workerThread->join(); }

    int Setup(IDeviceDiscoverListenerWeakPtr discoverListener);

private:
    // Called when a usb is added
    void UsbDeviceArrived(libusb_device *device);

    // Called when a usb is lost
    void UsbDeviceLeft(libusb_device *device);
    void UsbDeviceLeft(std::vector<FadeCandyDevicePtr>::iterator iter);

    // A function used for the worker thread.
    void WorkerLoop();

    // The context for lib usb.
    libusb_context* m_libusbContext;

    // Locks the usb events    
    std::mutex m_deivceListLock;

    // A thread that gives us fake hot plug if not supported
    std::unique_ptr<std::thread> m_workerThread;

    // Holds a list of current devices.
    std::vector<FadeCandyDevicePtr> m_deviceList;

    // The last time we check for new devices.
    std::chrono::high_resolution_clock::time_point m_lastDeviceCheckTime;

    // Holds a refrernce to the device listener
    IDeviceDiscoverListenerWeakPtr m_discoverListener;
};
