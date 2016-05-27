#pragma once

#include <random>

#include "Common.h"
#include "UsbDeviceManager.h"
#include "IDeviceDiscoveryListener.h"
#include "IWriteablePixelEndpoint.h"
#include "SharedFromThisHelper.h"
#include "Panel.h"
#include "ConstantRateDriver.h"
#include "IDrivable.h"
#include "IGem.h"
#include "Drawables/IDrawable.h"

#define GEM_RUNNING_TIME_MAX_SECONDS 60
#define GEM_RUNNING_TIME_MIN_SECONDS 40

typedef std::pair<IGemPtr, LightFx::Drawables::IDrawablePtr> GemPanelPair;

DECLARE_SMARTPOINTER(Prism);
class Prism :
    public IDeviceDiscoverListener,
    public LightFx::IPanelRenderedCallback,
    public LightFx::IDrivable,
    public LightFx::SharedFromThis,
    public LightFx::ITimelineObjectCallback
{
public:
    Prism() :
        m_activeGemIndex(999),
        m_activeGemTimeRemaing(0),
        m_animateOutGem(nullptr)
    { }

    // Preforms setup.
    void AlignCrystals();

    // Start the prism.
    void Prismify();

    // Fired before the prism is rendered.
    void OnTick(uint64_t tick, LightFx::milliseconds elapsedTime);

    // Called when a device is added
    void OnDeviceAdded(IWriteablePixelEndpointPtr device);

    // Called when a device is removed
    void OnDeviceRemoved(IWriteablePixelEndpointPtr device);

    // Fired when the panel has rendered
    void OnPanelRendered() override;

    // Sets the intensity on the main panel.
    void SetIntensity(double intensity);

    // Fired when a panel fade is complete
    void OnTimelineFinished(LightFx::ITimelineObjectPtr timeline);

private:

    // The device manager
    UsbDeviceManagerPtr m_usbDeviceManager;

    // Holds a pixel endpoint if we have one.
    IWriteablePixelEndpointWeakPtr m_pixelEndpoint;

    // Holds the main light panel
    LightFx::PanelPtr m_lightPanel;

    // Holds the main driver for the panel.
    LightFx::ConstantRateDriverPtr m_driver;

    // A random engine
    std::random_device m_randomDevice;

    // 
    // Gem logic
    uint8_t m_activeGemIndex;
    LightFx::milliseconds m_activeGemTimeRemaing;
    std::vector<GemPanelPair> m_gemList;
    IGemPtr m_animateOutGem;

    // Checks if we should change gems
    void CheckForGemSwtich(LightFx::milliseconds elapsedTime);
};