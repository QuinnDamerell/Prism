#include "Gems/SwipeColorGem.h"
#include "Drawables/SwipeDrawable.h"


using namespace LightFx;
using namespace LightFx::Drawables;
using namespace LightFx::Colorables;
using namespace Gems;

// Called when the gem should setup.
void SwipeColorGem::OnSetup(IDrawablePtr mainLayer)
{
    // Capture the main layer
    m_mainLayer = mainLayer;
}

// Called when it is activated and will be displayed.
void SwipeColorGem::OnActivated()
{
}

// Called when the gem is going to stop being displayed.
void SwipeColorGem::OnDeactivated()
{
}

// Called just before the prism will render
void SwipeColorGem::OnTick(uint64_t tick, std::chrono::milliseconds elapsedTime)
{
    // Take away time.
    m_timeUntilNextSwipe -= elapsedTime;

    if (m_timeUntilNextSwipe.count() < 0)
    {
        // Update times
        m_minTime += m_isTimeGoingUp ? 2 : -2;
        m_maxTime += m_isTimeGoingUp ? 4 : -3.5;

        // Update time direction if needed
        if (m_minTime < 15 || m_maxTime < 40)
        {
            m_isTimeGoingUp = true;
        }
        if (m_minTime > 200 || m_maxTime > 400)
        {
            m_isTimeGoingUp = false;
        }

        // Reset time.
        std::uniform_int_distribution<int> dist(m_minTime, m_maxTime);
        m_timeUntilNextSwipe = milliseconds(dist(m_randomDevice));

        // Make the swipe
        SwipeDrawablePtr swipe = std::make_shared<SwipeDrawable>();

        // Set the direction.
        std::uniform_int_distribution<int> directionDist(0, 3);
        swipe->SetDirection((SwipeDirection)directionDist(m_randomDevice));

        // Set the color
        swipe->SetColor(GenerateRandomColor());

        // Add the layer
        m_mainLayer->AddDrawable(swipe, 100);
    }
}