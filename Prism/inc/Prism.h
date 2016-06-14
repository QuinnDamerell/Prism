#pragma once

#include <random>

#include "Common.h"

#include "RapcomHost.h"
#include "UsbDeviceManager.h"
#include "IControlCommandHandler.h"
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
    public IControlCommandHandler,
    public LightFx::IPanelRenderedCallback,
    public LightFx::IDrivable,
    public LightFx::SharedFromThis,
    public LightFx::ITimelineObjectCallback
{
public:
    Prism() :
        m_activeGemIndex(-1),
        m_activeGemTimeRemaing(0),
        m_animateOutGem(nullptr),
        m_maxActiveGemTimeSeconds(60),
        m_minActiveGemTimeSeconds(60),
        m_forceGemSwitch(false)
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

    // Fired when a panel fade is complete
    void OnTimelineFinished(LightFx::ITimelineObjectPtr timeline);

    //
    // IControlCommandHandler

    // Sets the intensity on the main panel.
    void IntensityChanged(double intensity);

    // Fired when the enabled gems list is changed.
    void EnabledGemsChanged();

    // Running time changed
    void GemRunningTimeChanged();

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

    // The object that will wrap rapcom
    RapcomHostPtr m_rapcomHost;

    // 
    // Gem logic

    // The active gem
    int8_t m_activeGemIndex;

    // The time remaining on this gem
    LightFx::milliseconds m_activeGemTimeRemaing;

    // The list of possible gems
    std::vector<GemPanelPair> m_gemList;

    // The list of enabled gems
    std::vector<bool> m_enabledGemList;

    // The gem that is being animated out.
    IGemPtr m_animateOutGem;

    // Forces the system to switch gems
    bool m_forceGemSwitch;

    // The min an max times a gem can be active.
    uint64_t m_maxActiveGemTimeSeconds;
    uint64_t m_minActiveGemTimeSeconds;

    // Checks if we should change gems
    void CheckForGemSwtich(LightFx::milliseconds elapsedTime);

    // Updates the enabled gem list.
    void UpdateEnabledGemList();
};