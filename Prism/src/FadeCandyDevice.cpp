#include <algorithm>
#include <iostream>
#include <cstring>

#include "FadeCandyDevice.h"

FadeCandyDevice::FadeCandyDevice(libusb_device* device) :
    m_Device(device),
    m_NumFramesPending(0),
    m_Handle(nullptr)
{
    // Setup firmware config
    memset(&m_FirmwareConfig, 0, sizeof m_FirmwareConfig);
    m_FirmwareConfig.control = TYPE_CONFIG;

    // Framebuffer headers
    memset(m_framebuffer, 0, sizeof m_framebuffer);
    for (unsigned i = 0; i < FRAMEBUFFER_PACKETS; ++i) 
    {
        m_framebuffer[i].control = TYPE_FRAMEBUFFER | i;
    }
    m_framebuffer[FRAMEBUFFER_PACKETS - 1].control |= FINAL;
}

FadeCandyDevice::~FadeCandyDevice()
{
    std::lock_guard<std::mutex> lock(m_pendingTransfersLock);
    for (std::vector<UsbTransferPtr>::iterator i = m_pendingTransfers.begin(), e = m_pendingTransfers.end(); i != e; ++i) 
    {
        UsbTransferPtr trans = *i;
        libusb_cancel_transfer(trans->transfer);
    }

    // Close the device if we opened it.
    if (m_Handle)
    {
        libusb_release_interface(m_Handle, 0);
        libusb_close(m_Handle);
        m_Handle = nullptr;
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

    libusb_config_descriptor *config;
    r = libusb_get_config_descriptor(m_Device, 0, &config);
    if (r < 0) {
        return r;
    }
    
    // Detach the kernel if it is bound.
    if (libusb_kernel_driver_active(m_Handle, 0) == 1)
    {
        r = libusb_detach_kernel_driver(m_Handle, 0);
        if (r < 0) {
            return r;
        }
    }

    // Claim it
    return libusb_claim_interface(m_Handle, 0);
}

void FadeCandyDevice::WriteConfiguration()
{
    // Set the config
    m_FirmwareConfig.data[0] = CFLAG_LED_CONTROL;

    // Write the config to the device
    SubmitTransfer(std::make_shared<UsbTransfer>(GetSharedPtr<FadeCandyDevice>(), &m_FirmwareConfig, sizeof m_FirmwareConfig, OTHER, false));
}

void FadeCandyDevice::WriteColorConfig()
{
    // Setup the packets
    Packet mColorLUT[LUT_PACKETS];
    memset(mColorLUT, 0, sizeof mColorLUT);
    for (unsigned i = 0; i < LUT_PACKETS; ++i) 
    {
        mColorLUT[i].control = TYPE_LUT | i;
    }
    mColorLUT[LUT_PACKETS - 1].control |= FINAL;

    Packet *packet = mColorLUT;
    const unsigned firstByteOffset = 1;  // Skip padding byte
    unsigned byteOffset = firstByteOffset;

    // Use the default values
    double gamma = 1.0;                           // Power for nonlinear portion of curve
    double whitepoint[3] = { 1.0, 1.0, 1.0 };     // White-point RGB value (also, global brightness)
    double linearSlope = 1.0;                     // Slope (output / input) of linear section of the curve, near zero
    double linearCutoff = 0.0;

    for (unsigned channel = 0; channel < 3; channel++) 
    {
        for (unsigned entry = 0; entry < LUT_ENTRIES; entry++) 
        {
            double output;

            /*
            * Normalized input value corresponding to this LUT entry.
            * Ranges from 0 to slightly higher than 1. (The last LUT entry
            * can't quite be reached.)
            */
            double input = (entry << 8) / 65535.0;

            // Scale by whitepoint before anything else
            input *= whitepoint[channel];

            // Is this entry part of the linear section still?
            if (input * linearSlope <= linearCutoff) 
            {
                // Output value is below linearCutoff. We're still in the linear portion of the curve
                output = input * linearSlope;

            }
            else 
            {
                // Nonlinear portion of the curve. This starts right where the linear portion leaves
                // off. We need to avoid any discontinuity.
                double nonlinearInput = input - (linearSlope * linearCutoff);
                double scale = 1.0 - linearCutoff;
                output = linearCutoff + pow(nonlinearInput / scale, gamma) * scale;
            }

            // Round to the nearest integer, and clamp. Overflow-safe.
            int64_t longValue = (output * 0xFFFF) + 0.5;
            int intValue = std::max<int64_t>(0, std::min<int64_t>(0xFFFF, longValue));

            // Store LUT entry, little-endian order.
            packet->data[byteOffset++] = uint8_t(intValue);
            packet->data[byteOffset++] = uint8_t(intValue >> 8);
            if (byteOffset >= sizeof packet->data) 
            {
                byteOffset = firstByteOffset;
                packet++;
            }
        }
    }

    // Submit the config
    SubmitTransfer(std::make_shared<UsbTransfer>(GetSharedPtr<FadeCandyDevice>(), &mColorLUT, sizeof mColorLUT, OTHER, false));
}

void FadeCandyDevice::WritePixels(uint8_t* pixelArray, uint64_t length)
{
    // Truncate to the framebuffer size, and only deal in whole pixels.
    int numPixels = length / 3;
    if (numPixels > NUM_PIXELS)
    {
        numPixels = NUM_PIXELS;
    }

    // Load the array into the frame buffer.
    for (int i = 0; i < numPixels; i++) 
    {
        uint8_t *out = fbPixel(i);
        out[0] = pixelArray[i * 3 + 0];
        out[1] = pixelArray[i * 3 + 1];
        out[2] = pixelArray[i * 3 + 2];
    }

    // Send it out!
    WriteFramebuffer();
}

void FadeCandyDevice::WriteFramebuffer()
{
    // Check for frames we can clean up.
    CleanupFinishedTransfers();

    // Check if we have too many frames pending.
    if (m_NumFramesPending >= MAX_FRAMES_PENDING)
    {
        std::cout << "Frame skipped, we have submitted too many\n";
        return;
    }

    // Submit the frame.
    SubmitTransfer(std::make_shared<UsbTransfer>(GetSharedPtr<FadeCandyDevice>(), &m_framebuffer, sizeof m_framebuffer, FRAME, true));
}


bool FadeCandyDevice::SubmitTransfer(UsbTransferPtr transPtr)
{
    // Do a sync transfer
    if (transPtr->m_synchronus)
    {
        int transferedAmmount = 0;

        // Do the transfer
        int r = libusb_bulk_transfer(m_Handle, 1, (unsigned char*)(transPtr->transfer->buffer), transPtr->transfer->length, &transferedAmmount, 2000);

        // Write the error if write failed
        if (r < 0)
        {
            std::clog << "Error submitting USB transfer: " << libusb_strerror(libusb_error(r)) << "\n";
        }

        // Return if we succeeded.
        return r <= 0;
    }
    else
    {
        // Clean up any pending transfers
        CleanupFinishedTransfers();

        // Do a async transfer
        int r = libusb_submit_transfer(transPtr->transfer);

        if (r < 0)
        {
            std::clog << "Error submitting USB transfer: " << libusb_strerror(libusb_error(r)) << "\n";
            return false;
        }
        else
        {
            std::lock_guard<std::mutex> lock(m_pendingTransfersLock);
            m_pendingTransfers.push_back(transPtr);
            return true;
        }
    }
}

static void LIBUSB_CALL cbCompleteTransfer(struct libusb_transfer *transfer)
{
    UsbTransfer *trans = static_cast<UsbTransfer*>(transfer->user_data);
    trans->finished = true;
}

void FadeCandyDevice::CleanupFinishedTransfers()
{
    std::lock_guard<std::mutex> lock(m_pendingTransfersLock);
    std::vector<UsbTransferPtr>::iterator i = m_pendingTransfers.begin();
    while (i != m_pendingTransfers.end())
    {
        UsbTransferPtr trans = *i;
        if (trans->finished)
        {
            // We are finished, remove it.
            if (trans->transfer->status != LIBUSB_TRANSFER_COMPLETED)
            {
                std::cout << "Async transfer failed\n";
            }

            // Update the pending frame count.
            if (trans->packetType == FRAME)
            {
                m_NumFramesPending--;
            }

            // Erase the element
            i = m_pendingTransfers.erase(i);
        }
        else
        {
            i++;
        }
    }
}

UsbTransfer::UsbTransfer(FadeCandyDevicePtr device, void *buffer, int length, PacketType type, bool synchronus) :
    transfer(libusb_alloc_transfer(0)),
    finished(false),
    packetType(type),
    m_synchronus(synchronus),
    bufferCopy(nullptr)
{
    // Set the buffer
    uint8_t *data = (uint8_t*)buffer;

    // If we need to copy and this is not sync we should copy.
#if NEED_COPY_USB_TRANSFER_BUFFER
    if (!synchronus)
    {
        bufferCopy = malloc(length);
        memcpy(bufferCopy, buffer, length);
        data = (uint8_t*)bufferCopy;
    }
#endif

    libusb_fill_bulk_transfer(transfer, device->GetHandle(), OUT_ENDPOINT, data, length, cbCompleteTransfer, this, 2000);
}

UsbTransfer::~UsbTransfer()
{
    libusb_free_transfer(transfer);
    if (bufferCopy)
    {
        free(bufferCopy);
    }
}
