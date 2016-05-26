#include <iostream>

#include "Gems/RowRunnerGem.h"
#include "Drawables/SwipeDrawable.h"
#include "Fadables/Fader.h"

using namespace LightFx;
using namespace LightFx::Drawables;
using namespace LightFx::Fadables;
using namespace Gems;

// Called when the gem should setup.
void RowRunnerGem::OnSetup(LightFx::Drawables::IDrawablePtr mainLayer)
{
    // Capture the main layer, we will draw to this
    m_mainLayer = mainLayer;
}

// Called when it is activated and will be displayed.
void RowRunnerGem::OnActivated()
{ }

// Called when the gem is going to stop being displayed.
void RowRunnerGem::OnDeactivated()
{ }


// Called just before the prism will render
void RowRunnerGem::OnTick(uint64_t tick, std::chrono::milliseconds elapsedTime)
{
    // Take away time.
    m_timeUntilNextRow -= elapsedTime;

    if (m_timeUntilNextRow.count() < 0)
    {
        // Update times
        m_minTime += m_isTimeGoingUp ? 1.5 : -1.5;
        m_maxTime += m_isTimeGoingUp ? 2.4 : -2.4;

        // Update time direction if needed
        if (m_minTime < 20 || m_maxTime < 40)
        {
            m_isTimeGoingUp = true;
        }
        if (m_minTime > 150 || m_maxTime > 220)
        {
            m_isTimeGoingUp = false;
        }

        // Reset time.
        std::uniform_int_distribution<int> dist(m_minTime, m_maxTime);
        m_timeUntilNextRow = milliseconds(dist(m_randomDevice));

        // Make a swipe drawable
        SwipeDrawablePtr swipe = std::make_shared<SwipeDrawable>();

        // Add it to the main layer now bc this will set the size, and then we will
        // change it again later
        m_mainLayer->AddDrawable(swipe, 10);

        // To to a random color
        swipe->SetColor(GenerateRandomColor(true));

        // Pick a direction randomly
        std::uniform_int_distribution<int> directionDist(0, 3);
        SwipeDirection direction = (SwipeDirection)directionDist(m_randomDevice);
        swipe->SetDirection(direction);

        // Pick a random starting point
        std::uniform_int_distribution<int> posDist(0, m_mainLayer->GetWitdh() - 1);
        int64_t startingPos = posDist(m_randomDevice);

        // Set the postition of the drawable.
        if (direction == SwipeDirection::Down || direction == SwipeDirection::Up)
        {
            swipe->SetPosition(startingPos, 0);
            swipe->SetSize(m_mainLayer->GetWitdh(), 1);
        }
        else
        {
            swipe->SetPosition(0, startingPos);
            swipe->SetSize(1, m_mainLayer->GetHeight());
        }
    }
}