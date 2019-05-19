#pragma once

#include <random>
#include <list>

#include "Common.h"
#include "IGem.h"
#include "Drawables/SolidDrawable.h"

namespace Gems
{
	class ColorColliderPixel 
	{
	public:
		enum class  Direction
		{
			Up,
			Down,
			Left,
			Right
		};

		ColorColliderPixel(int16_t id) :
			m_id(id),
			m_currentX(0),
			m_currentY(0),
			m_currentDirection(Direction::Up)
		{}	

		int16_t m_id;
		int64_t m_currentX;
		int64_t m_currentY;
		int64_t m_moveRedirect;
		Direction m_currentDirection;
	};

    DECLARE_SMARTPOINTER(ColorCollider);
    class ColorCollider :
        public IGem
    {
    public:
        ColorCollider() :
			m_updateDelayCount(0),
			m_currentColor(0)
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

		// Update Delay
		int m_updateDelayCount;
		double m_currentColor;

        // The running pixels
		std::list<ColorColliderPixel> m_pixels;    
    };
}
