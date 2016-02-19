#pragma once

#include <cstdint>

#include "Common.h"

DECLARE_SMARTPOINTER(IWriteablePixelEndpoint);
class IWriteablePixelEndpoint
{
public:
    // Write pixels to the device.
    virtual void WritePixels(uint8_t* pixelArray, uint32_t length) = 0;
};