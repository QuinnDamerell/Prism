#pragma once

#include <random>

#include "Common.h"
#include "IGem.h"
#include "Drawables/SolidDrawable.h"

namespace Gems
{
    DECLARE_SMARTPOINTER(RunningPixel);
    class RunningPixel :
        public IGem
    {
    public:
        RunningPixel():
            m_currentY(0),
            m_currentX(0),
            m_currentColorValue(0),
            m_moveRedirect(0),
            m_currentFade(1000),
            m_fadeIsGoingUp(true)
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

        // Holds the X and Y pos.
        int64_t m_currentX;
        int64_t m_currentY;
        double m_currentColorValue;
        int64_t m_moveRedirect;

        // Holds the current fade time
        bool m_fadeIsGoingUp;
        int64_t m_currentFade;

        enum class  Direction
        {
            Up,
            Down,
            Left,
            Right
        };
        Direction m_currentDirection;    
    };
}
