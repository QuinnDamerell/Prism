#pragma once

#include "Common.h"
#include "FadeCandyDevice.h"
#include "IWriteablePixelEndpoint.h"

// Used to express when a device is 
DECLARE_SMARTPOINTER(IDeviceDiscoverListener);
class IDeviceDiscoverListener
{
public:

    virtual void OnDeviceAdded(IWriteablePixelEndpointPtr device) = 0;

    virtual void OnDeviceRemoved(IWriteablePixelEndpointPtr device) = 0;
};
