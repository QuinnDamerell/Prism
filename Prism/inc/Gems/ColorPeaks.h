#pragma once

#include <random>

#include "Common.h"
#include "IGem.h"

#include "TimelineObject.h"
#include "SharedFromThisHelper.h"
#include "Drawables/Drawable.h"

#define PEAK_CREATE_TIME_MAX_MS 1700
#define PEAK_CREATE_TIME_MIN_MS 400

namespace Gems
{
    DECLARE_SMARTPOINTER(ColorPeaks);
    class ColorPeaks :
        public IGem,
        public LightFx::ITimelineObjectCallback,
        public LightFx::SharedFromThis
    {
    public:
        ColorPeaks() :
            m_timeUntilPeak(std::chrono::milliseconds(0)),
            m_gridSize(0)
        { }

        // Called when the gem should setup.
        void OnSetup(LightFx::Drawables::IDrawablePtr mainLayer);

        // Called when it is activated and will be displayed.
        void OnActivated() {}

        // Called when the gem is going to stop being displayed.
        void OnDeactivated() {}

        // Called just before the prism will render
        void OnTick(uint64_t tick, std::chrono::milliseconds elapsedTime);

        // Called when one of the timeline's finish
        void OnTimelineFinished(LightFx::ITimelineObjectPtr timeline);

    private:
        // The main reference to our drawable panel.
        LightFx::Drawables::IDrawablePtr m_drawableLayer;

        // A random device
        std::random_device m_randomDevice;

        // Time until the next peak
        std::chrono::milliseconds m_timeUntilPeak;

        // Grid size
        int64_t m_gridSize;
    };
}
