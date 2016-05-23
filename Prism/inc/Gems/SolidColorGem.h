#pragma once

#include "Common.h"
#include "IGem.h"
#include "Drawables/SolidDrawable.h"

namespace Gems
{
    DECLARE_SMARTPOINTER(SolidColorGem);
    class SolidColorGem :
        public IGem
    {
    public:
        SolidColorGem() :
            m_fullCycleTime(20000),
            m_timeUsedInCycle(m_fullCycleTime)
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

        // The main drawable
        LightFx::Drawables::SolidDrawablePtr m_soldColor;

        // The time left in this cycle
        std::chrono::milliseconds m_timeUsedInCycle;

        // The full time of the cycle.
        std::chrono::milliseconds m_fullCycleTime;
    };
}
