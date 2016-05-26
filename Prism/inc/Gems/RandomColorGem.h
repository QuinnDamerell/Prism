#pragma once

#include <random>

#include "Common.h"
#include "IGem.h"
#include "Drawables/SolidDrawable.h"
#include "TimelineObject.h"
#include "SharedFromThisHelper.h"

namespace Gems
{
    DECLARE_SMARTPOINTER(RandomColorGem);
    class RandomColorGem :
        public IGem,
        public LightFx::ITimelineObjectCallback,
        public LightFx::SharedFromThis
    {
    public:
        RandomColorGem() :
            m_skipValue(0),
            m_useRainbowColor(false),
            m_colorCycleTime(std::chrono::milliseconds(20000)),
            m_timeUntilColorSwitch(m_colorCycleTime)
        { }

        // Called when the gem should setup.
        void OnSetup(LightFx::Drawables::IDrawablePtr mainLayer);

        // Called when it is activated and will be displayed.
        void OnActivated();

        // Called when the gem is going to stop being displayed.
        void OnDeactivated();

        // Called just before the prism will render
        void OnTick(uint64_t tick, std::chrono::milliseconds elapsedTime);

        // Fired when a fade is complete
        void OnTimelineFinished(LightFx::ITimelineObjectPtr timeline);

    private:

        // Holds how long we run until we switch color moes
        std::chrono::milliseconds m_timeUntilColorSwitch;
        std::chrono::milliseconds m_colorCycleTime;

        // Indicates if we are using rainbow color or not
        bool m_useRainbowColor;

        // Holds the current rainbow color
        double m_currentColorValue = 0;

        // Make a grid for each pixel
        std::vector<std::vector<LightFx::Drawables::SolidDrawablePtr>> m_pixelArray;

        // Rand device
        std::random_device m_randomDevice;

        // Skip value
        int64_t m_skipValue;
    };
}
