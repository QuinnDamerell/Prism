#pragma once

#include <random>

#include "Common.h"
#include "IGem.h"

#include "Layers/Drawable/DrawableLayer.h"
#include "Layers/Drawable/TimelineObject.h"

#define PEAK_CREATE_TIME_MAX_MS 2000
#define PEAK_CREATE_TIME_MIN_MS 8000

namespace Gems
{
    //DECLARE_SMARTPOINTER(ColorPeaks);
    //class ColorPeaks :
    //    public IGem,
    //    public LightFx::Layers::Drawable::ITimelineObjectCallback,
    //    public std::enable_shared_from_this<ColorPeaks>
    //{
    //public:
    //    ColorPeaks() :
    //        m_timeUntilPeak(std::chrono::milliseconds(0)),
    //        m_gridSize(0)
    //    { }

    //    // Called when the gem should setup.
    //    void OnSetup(LightFx::PanelPtr mainPanel);

    //    // Called when it is activated and will be displayed.
    //    void OnActivated() {};

    //    // Called when the gem is going to stop being displayed.
    //    void OnDeactivated() {};

    //    // Called just before the prism will render
    //    void OnPreRender(uint64_t tick, std::chrono::milliseconds elapsedTime);

    //    // Called when one of the timeline's finish
    //    void OnTimelineFinished(LightFx::Layers::Drawable::TimelineObjectPtr timeline);

    //private:
    //    // The main reference to our drawable panel.
    //    LightFx::Layers::Drawable::DrawableLayerPtr m_drawableLayer;

    //    // A random device
    //    std::random_device m_randomDevice;

    //    // Time until the next peak
    //    std::chrono::milliseconds m_timeUntilPeak;

    //    // Grid size
    //    uint64_t m_gridSize;
    //};
}
