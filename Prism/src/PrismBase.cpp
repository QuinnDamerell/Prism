#include <iostream>

#include "libusb.h"
#include "PrismBase.h"

// Sets us up.
int PrismBase::Setup()
{
    // Start up the usb device manager
    m_usbDeviceManager = std::make_unique<UsbDeviceManager>();
    return m_usbDeviceManager->Setup();
}