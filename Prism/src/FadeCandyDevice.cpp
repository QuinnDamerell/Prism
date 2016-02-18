#include <iostream>
#include "FadeCandyDevice.h"

FadeCandyDevice::FadeCandyDevice(libusb_device* device) :
    m_Device(device)
{
    m_SerialBuffer[0] = '\0';

    memset(&m_FirmwareConfig, 0, sizeof m_FirmwareConfig);
    m_FirmwareConfig.control = TYPE_CONFIG;

    // Framebuffer headers
    memset(m_framebuffer, 0, sizeof m_framebuffer);
    for (unsigned i = 0; i < FRAMEBUFFER_PACKETS; ++i) {
        m_framebuffer[i].control = TYPE_FRAMEBUFFER | i;
    }
    m_framebuffer[FRAMEBUFFER_PACKETS - 1].control |= FINAL;
}

FadeCandyDevice::~FadeCandyDevice()
{
    for (std::set<UsbTransferPtr>::iterator i = m_pendingTransfers.begin(), e = m_pendingTransfers.end(); i != e; ++i) 
    {
        UsbTransferPtr trans = *i;
        libusb_cancel_transfer(trans->transfer);
    }
}

bool FadeCandyDevice::Probe(libusb_device *device)
{
    libusb_device_descriptor dd;

    if (libusb_get_device_descriptor(device, &dd) < 0) 
    {
        return false;
    }

    return dd.idVendor == 0x1d50 && dd.idProduct == 0x607a;
}

int FadeCandyDevice::Open()
{
    // Get the descriptor
    int r = libusb_get_device_descriptor(m_Device, &m_deviceDescriptor);
    if (r < 0) {
        return r;
    }

    // Open the device
    r = libusb_open(m_Device, &m_Handle);
    if (r < 0) {
        return r;
    }

    // Claim it
    r = libusb_claim_interface(m_Handle, 0);
    if (r < 0) {
        return r;
    }

    return libusb_get_string_descriptor_ascii(m_Handle, m_deviceDescriptor.iSerialNumber, (uint8_t*)m_SerialBuffer, sizeof m_SerialBuffer);
}

void FadeCandyDevice::WriteConfiguration()
{
    // Set the config
    m_FirmwareConfig.data[0] = CFLAG_LED_CONTROL;

    // Write the config to the device
    SubmitTransfer(std::make_shared<UsbTransfer>(shared_from_this(), &m_FirmwareConfig, sizeof m_FirmwareConfig, OTHER));

    byte buffer[] = { 100,100,100,100,100 };
    WritePixels(buffer, 5);
}


void FadeCandyDevice::WritePixels(byte* pixels, uint32_t length)
{
    // Truncate to the framebuffer size, and only deal in whole pixels.
    int numPixels = length / 3;
    if (numPixels > NUM_PIXELS)
    {
        numPixels = NUM_PIXELS;
    }

    for (int i = 0; i < numPixels; i++) {
        uint8_t *out = fbPixel(i);
        out[0] = pixels[i * 3 + 0];
        out[1] = pixels[i * 3 + 1];
        out[2] = pixels[i * 3 + 2];
    }

    WriteFramebuffer();
}

void FadeCandyDevice::WriteFramebuffer()
{
    /*
    * Asynchronously write the current framebuffer.
    *
    * TODO: Currently if this gets ahead of what the USB device is capable of,
    *       we always drop frames. Alternatively, it would be nice to have end-to-end
    *       flow control so that the client can produce frames slower.
    */

    //if (mNumFramesPending >= MAX_FRAMES_PENDING) {
    //    // Too many outstanding frames. Wait to submit until a previous frame completes.
    //    mFrameWaitingForSubmit = true;
    //    return;
    //}

    if (SubmitTransfer(std::make_shared<UsbTransfer>(shared_from_this(), &m_framebuffer, sizeof m_framebuffer, FRAME)))
    {
       // mFrameWaitingForSubmit = false;
       // mNumFramesPending++;
    }
}


bool FadeCandyDevice::SubmitTransfer(UsbTransferPtr transPtr)
{
    int r = libusb_submit_transfer(transPtr->transfer);

    if (r < 0) 
    {
        std::clog << "Error submitting USB transfer: " << libusb_strerror(libusb_error(r)) << "\n";
        return false;
    }
    else 
    {
        m_pendingTransfers.insert(transPtr);
        return true;
    }    
}

void FadeCandyDevice::CompleteTransfer(libusb_transfer *transfer)
{
    UsbTransfer *trans = static_cast<UsbTransfer*>(transfer->user_data);
    trans->finished = true;
}

UsbTransfer::UsbTransfer(FadeCandyDevicePtr device, void *buffer, int length, PacketType type) :
    transfer(libusb_alloc_transfer(0)),
    finished(false),
    packetType(type)
{
#if NEED_COPY_USB_TRANSFER_BUFFER
    bufferCopy = malloc(length);
    memcpy(bufferCopy, buffer, length);
    uint8_t *data = (uint8_t*)bufferCopy;
#else
    uint8_t *data = (uint8_t*)buffer;
#endif

    libusb_fill_bulk_transfer(transfer, device->GetHandle(), OUT_ENDPOINT, data, length, FadeCandyDevice::CompleteTransfer, this, 2000);
}

UsbTransfer::~UsbTransfer()
{
    libusb_free_transfer(transfer);
#if NEED_COPY_USB_TRANSFER_BUFFER
    free(bufferCopy);
#endif
}
