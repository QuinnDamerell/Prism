#pragma once

#include <mutex>

#include "Common.h"
#include "UsbDeviceManager.h"

DECLARE_SMARTPOINTER(PrismBase);
class PrismBase
{
public:
    PrismBase() {}
    ~PrismBase() {}

    int Setup();

private:
    UsbDeviceManagerUniquePtr m_usbDeviceManager;
};