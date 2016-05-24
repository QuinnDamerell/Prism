#pragma once

#include <random>

#include "Common.h"
#include "IGem.h"
#include "Drawables/Drawable.h"

namespace Gems
{
    DECLARE_SMARTPOINTER(SwipeColorGem);
    class SwipeColorGem :
        public IGem
    {
    public:
        SwipeColorGem() :
            m_currentColor(0),
            m_timeUntilNextSwipe(std::chrono::milliseconds(0)),
            m_minTime(200),
            m_maxTime(400),
            m_isTimeGoingUp(false)
        { }

        // Called when the gem should setup.
        void OnSetup(LightFx::Drawables::IDrawablePtr mainLayer);

        // Called when it is activated and will be displayed.
        void OnActivated();

        // Called when the gem is going to stop being displayed.
        void OnDeactivated();

        // Called just before the prism will render
        void OnTick(uint64_t tick, std::chrono::milliseconds elapsedTime);

    private:
        // The main drawable layer
        LightFx::Drawables::IDrawablePtr m_mainLayer;

        // The current color
        double m_currentColor;

        // Time until the next swipe
        std::chrono::milliseconds m_timeUntilNextSwipe;

        // Random
        std::random_device m_randomDevice;

        // Holds time adjustment.
        bool m_isTimeGoingUp;
        int64_t m_minTime;
        int64_t m_maxTime;
    };
}
