#pragma once

#include <mutex>

#include "Common.h"
#include "UsbDeviceManager.h"
#include "IDeviceDiscoveryListener.h"
#include "IWriteablePixelEndpoint.h"
#include "Panel.h"
#include "ConstantRateDriver.h"

#include "Layers/Drawable/TimelineObject.h"
#include "Layers/Drawable/DrawableLayer.h"

DECLARE_SMARTPOINTER(PrismBase);
class PrismBase :
    public IDeviceDiscoverListener,
    public LightFx::IPanelRenderedCallback,
    public LightFx::Layers::Drawable::ITimelineObjectCallback,
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

    // Fired when the panel has rendered
    void OnPanelRendered() override;

    // Fired when an animation finishes
    void OnTimelineFinished(std::shared_ptr<LightFx::Layers::Drawable::TimelineObject> timeline) override;


private:

    // The device manager
    UsbDeviceManagerUniquePtr m_usbDeviceManager;

    LightFx::Layers::Drawable::DrawableLayerPtr m_drawable;

    // Holds a pixel endpoint if we have one.
    IWriteablePixelEndpointWeakPtr m_pixelEndpoint;

    LightFx::PanelPtr m_lightPanel;
    LightFx::ConstantRateDriverPtr m_driver;
};