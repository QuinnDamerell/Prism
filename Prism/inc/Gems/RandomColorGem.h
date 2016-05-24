#pragma once

#include <random>

#include "Common.h"
#include "IGem.h"
#include "Drawables/SolidDrawable.h"

namespace Gems
{
    DECLARE_SMARTPOINTER(RandomColorGem);
    class RandomColorGem :
        public IGem
    {
    public:
        RandomColorGem() :
            m_skipValue(0)
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

        // Make a grid for each pixel
        std::vector<std::vector<LightFx::Drawables::SolidDrawablePtr>> m_pixelArray;

        // Rand device
        std::random_device m_randomDevice;

        // Skip value
        int64_t m_skipValue;
    };
}
