#pragma once

#include <random>

#include "Common.h"
#include "IGem.h"
#include "Drawables/SolidDrawable.h"

namespace Gems
{
    DECLARE_SMARTPOINTER(RowRunnerGem);
    class RowRunnerGem :
        public IGem
    {
    public:
        RowRunnerGem() :
            m_timeUntilNextRow(std::chrono::milliseconds(0)),
            m_minTime(150),
            m_maxTime(220),
            m_isTimeGoingUp(false)
        {}

        // Called when the gem should setup.
        void OnSetup(LightFx::Drawables::IDrawablePtr mainLayer);

        // Called when it is activated and will be displayed.
        void OnActivated();

        // Called when the gem is going to stop being displayed.
        void OnDeactivated();

        // Called just before the prism will render
        void OnTick(uint64_t tick, std::chrono::milliseconds elapsedTime);

    private:

        // Make a grid for each pixel
        LightFx::Drawables::IDrawablePtr m_mainLayer;

        // Rand device
        std::random_device m_randomDevice;

        // Time until the next row
        std::chrono::milliseconds m_timeUntilNextRow;

        // Holds time adjustment.
        bool m_isTimeGoingUp;
        int64_t m_minTime;
        int64_t m_maxTime;
    };
}
