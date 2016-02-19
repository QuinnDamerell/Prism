#pragma once

#include <mutex>

#include "Common.h"
#include "UsbDeviceManager.h"
#include "IDeviceDiscoveryListener.h"

DECLARE_SMARTPOINTER(PrismBase);
class PrismBase :
    public IDeviceDiscoverListener,
    public std::enable_shared_from_this<PrismBase>
{
public:
    PrismBase() {}
    ~PrismBase() {}

    // Does setup on the base.
    int Setup();

    // Called when a device is added
    void OnDeviceAdded(IWriteablePixelEndpointPtr device);

    // Called when a device is removed
    void OnDeviceRemoved(IWriteablePixelEndpointPtr device);

private:

    // The device manager
    UsbDeviceManagerUniquePtr m_usbDeviceManager;

    // Holds a pixel endpoint if we have one.
    IWriteablePixelEndpointWeakPtr m_pixelEndpoint;
};