#include <iostream>

#include "Gems/RunningPixel.h"
#include "Fadables/Fader.h"

using namespace LightFx;
using namespace LightFx::Drawables;
using namespace LightFx::Fadables;
using namespace Gems;

// Called when the gem should setup.
void RunningPixel::OnSetup(LightFx::Drawables::IDrawablePtr mainLayer)
{
    // Capture the main layer, we will draw to this
    m_mainLayer = mainLayer;
    
    // Start off in a random position
    std::uniform_int_distribution<int> dist(0, mainLayer->GetHeight() - 1);
    m_currentX = dist(m_randomDevice);
    m_currentY = dist(m_randomDevice);
    m_currentDirection = Direction::Down;
}

// Called when it is activated and will be displayed.
void RunningPixel::OnActivated()
{ }

// Called when the gem is going to stop being displayed.
void RunningPixel::OnDeactivated()
{ }

int moveRedirect = 0;
// Called just before the prism will render
void RunningPixel::OnTick(uint64_t tick, std::chrono::milliseconds elapsedTime)
{
    // Update position
    switch (m_currentDirection)
    {
    case RunningPixel::Direction::Down:
        m_currentY++;
        break;
    case RunningPixel::Direction::Up:
        m_currentY--;
        break;
    case RunningPixel::Direction::Left:
        m_currentX--;
        break;
    case RunningPixel::Direction::Right:
        m_currentX++;
        break;
    }

    // Bounds check
    bool changeDirection = false;
    if (m_currentX >= static_cast<int64_t>(m_mainLayer->GetWitdh()))
    {
        m_currentX = m_mainLayer->GetWitdh() - 1;
        changeDirection = true;
    }
    if (m_currentX < 0)
    {
        m_currentX = 0;
        changeDirection = true;
    }
    if (m_currentY >= static_cast<int64_t>(m_mainLayer->GetHeight()))
    {
        m_currentY = m_mainLayer->GetHeight() - 1;
        changeDirection = true;
    }
    if (m_currentY < 0)
    {
        m_currentY = 0;
        changeDirection = true;
    }

    // Force a move if we haven't in sometime.
    m_moveRedirect--;
    if (m_moveRedirect < 0)
    {
        changeDirection = true;
    }

    // Change direction if needed
    if (changeDirection)
    {
        // Get a new direction, make sure it isn't the opposite of the current or current.
        Direction newDirection;
        std::uniform_int_distribution<int> dist(0, 3);
        do
        {
            newDirection = (Direction)dist(m_randomDevice);
        } while (newDirection == m_currentDirection 
            || (newDirection == RunningPixel::Direction::Down && m_currentDirection == RunningPixel::Direction::Up)
            || (newDirection == RunningPixel::Direction::Up && m_currentDirection == RunningPixel::Direction::Down)
            || (newDirection == RunningPixel::Direction::Left && m_currentDirection == RunningPixel::Direction::Right)
            || (newDirection == RunningPixel::Direction::Right && m_currentDirection == RunningPixel::Direction::Left));

        // Set the direction
        m_currentDirection = newDirection;
        moveRedirect = -1;
    }

    // Now update the move redirect if we need to.
    if (m_moveRedirect < 0)
    {
        std::uniform_int_distribution<int> dist(3, 6);
        m_moveRedirect = dist(m_randomDevice);
    }

    // Update the color
    m_currentColorValue += .008;
    if (m_currentColorValue > 1)
    {
        m_currentColorValue = 0;
    }

    // Draw the new element.
    SolidDrawablePtr drawable = std::make_shared<SolidDrawable>(true);
    drawable->SetPosition(m_currentX, m_currentY, 1, 1);
    drawable->SetColor(GetRainbowColor(m_currentColorValue));

    // Update the current fade
    m_currentFade += m_fadeIsGoingUp ? 2 : -2;
    if (m_currentFade < 200)
    {
        m_fadeIsGoingUp = true;
    }
    if (m_currentFade > 1500)
    {
        m_fadeIsGoingUp = false;
    }

    // Make a fader
    FaderPtr fader = std::make_shared<Fader>(0, 1, milliseconds(m_currentFade));
    drawable->SetFader(fader);

    // Add the layer to the grid
    m_mainLayer->AddDrawable(drawable, 100);
}