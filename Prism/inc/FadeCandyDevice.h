#pragma once

#include <mutex>
#include <vector>
#include <libusb.h>

#include "Common.h"
#include "IWriteablePixelEndpoint.h"

#ifdef OS_LINUX
// No need to copy the buffer
#elif defined(_MSC_VER)
#define NEED_COPY_USB_TRANSFER_BUFFER 1
#elif OS_DARWIN
#define NEED_COPY_USB_TRANSFER_BUFFER 1
#else
#error Dont know whether we need to copy the USB transfer buffer
#endif


struct Packet
{
    uint8_t control;
    uint8_t data[63];
};


enum PacketType
{
    OTHER = 0,
    FRAME,
};

// Used to transfer data to the USB device
DECLARE_SMARTPOINTER(FadeCandyDevice);
DECLARE_SMARTPOINTER(UsbTransfer);
class UsbTransfer
{
    static const unsigned OUT_ENDPOINT = 1;

public:
    UsbTransfer(FadeCandyDevicePtr device, void *buffer, int length, PacketType type, bool synchronus);
    ~UsbTransfer();

    libusb_transfer* transfer;
#if NEED_COPY_USB_TRANSFER_BUFFER
    void *bufferCopy;
#endif
    PacketType packetType;
    bool finished;
    bool m_synchronus;
};

class FadeCandyDevice :
    public IWriteablePixelEndpoint,
    public std::enable_shared_from_this<FadeCandyDevice>
{
public:
    // Checks to see if this is a FC device
    static bool Probe(libusb_device *device);

    FadeCandyDevice(libusb_device* device);
    ~FadeCandyDevice();

    // Called when the device should be setup and inited.
    int Open();

    // Called when we should write the config
    void WriteConfiguration();

    // Called when we want to write pixels.
    void WritePixels(uint8_t* pixelArray, uint32_t length);

    // Called to get the libusb device
    libusb_device* GetDevice() { return m_Device; }

    // Called to get the libusb m_Handle
    libusb_device_handle* GetHandle() { return m_Handle; }

private:
    static const unsigned PIXELS_PER_PACKET = 21;
    static const unsigned LUT_ENTRIES_PER_PACKET = 31;
    static const unsigned FRAMEBUFFER_PACKETS = 25;
    static const unsigned LUT_PACKETS = 25;
    static const unsigned LUT_ENTRIES = 257;
    static const unsigned MAX_FRAMES_PENDING = 2;

    static const uint8_t TYPE_FRAMEBUFFER = 0x00;
    static const uint8_t TYPE_LUT = 0x40;
    static const uint8_t TYPE_CONFIG = 0x80;
    static const uint8_t FINAL = 0x20;
    static const unsigned NUM_PIXELS = 512;

    static const uint8_t CFLAG_NO_DITHERING = (1 << 0);
    static const uint8_t CFLAG_NO_INTERPOLATION = (1 << 1);
    static const uint8_t CFLAG_NO_ACTIVITY_LED = (1 << 2);
    static const uint8_t CFLAG_LED_CONTROL = (1 << 3);


    // Device
    libusb_device* m_Device;
    libusb_device_descriptor m_deviceDescriptor;
    libusb_device_handle* m_Handle;

    // Packet buffers
    Packet m_FirmwareConfig;
    Packet m_framebuffer[FRAMEBUFFER_PACKETS];

    // Accessors
    uint8_t *fbPixel(unsigned num)
    {
        return &m_framebuffer[num / PIXELS_PER_PACKET].data[3 * (num % PIXELS_PER_PACKET)];
    }

    // Transfer queue things
    std::mutex m_pendingTransfersLock;
    std::vector<UsbTransferPtr> m_pendingTransfers;
    uint32_t m_NumFramesPending;

    // Private functions
    bool SubmitTransfer(UsbTransferPtr transPtr); 
    void CleanupFinishedTransfers();
    void WriteFramebuffer();
};