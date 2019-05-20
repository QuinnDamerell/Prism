#include <iostream>

#include "Gems/ColorCollider.h"
#include "Drawables/ExpandingDrawable.h"
#include "Fadables/Fader.h"
#include "Fadables/Strober.h"

using namespace LightFx;
using namespace LightFx::Drawables;
using namespace LightFx::Fadables;
using namespace Gems;

// Called when the gem should setup.
void ColorCollider::OnSetup(LightFx::Drawables::IDrawablePtr mainLayer)
{
    // Capture the main layer, we will draw to this
    m_mainLayer = mainLayer;

	// Create the pixels.
	for (int i = 0; i < 2; i++)
	{
		m_pixels.emplace_back(ColorColliderPixel(static_cast<int16_t>(i)));
	}

	std::uniform_int_distribution<int> dist(0, static_cast<int>(mainLayer->GetHeight()) - 1);
	for (ColorColliderPixel& pixel : m_pixels)
	{
		pixel.m_currentX = dist(m_randomDevice);
		pixel.m_currentY = dist(m_randomDevice);
		pixel.m_currentDirection = ColorColliderPixel::Direction::Down;
		pixel.m_moveRedirect = -1;
	}
}

// Called when it is activated and will be displayed.
void ColorCollider::OnActivated()
{ }

// Called when the gem is going to stop being displayed.
void ColorCollider::OnDeactivated()
{ }

// Called just before the prism will render
void ColorCollider::OnTick(uint64_t tick, std::chrono::milliseconds elapsedTime)
{
	// Only update every 6 ticks, which will make the snakes move more slowly.
	m_updateDelayCount++;
	if (m_updateDelayCount < GetScaledRealtimeValue(1, 1, 12, 6))
	{
		return;
	}
	m_updateDelayCount = 0;

	// Update the color value.
	m_currentColor += .005;
	if (m_currentColor > 1)
	{
		m_currentColor = 0;
	}

	int count = 0;
	for (ColorColliderPixel& pixel : m_pixels)
	{
		count++;

		// Update position
		switch (pixel.m_currentDirection)
		{
		case ColorColliderPixel::Direction::Down:
			pixel.m_currentY++;
			break;
		case ColorColliderPixel::Direction::Up:
			pixel.m_currentY--;
			break;
		case ColorColliderPixel::Direction::Left:
			pixel.m_currentX--;
			break;
		case ColorColliderPixel::Direction::Right:
			pixel.m_currentX++;
			break;
		}

		// Bounds check
		bool changeDirection = false;
		if (pixel.m_currentX >= static_cast<int64_t>(m_mainLayer->GetWitdh()))
		{
			pixel.m_currentX = m_mainLayer->GetWitdh() - 1;
			changeDirection = true;
		}
		if (pixel.m_currentX < 0)
		{
			pixel.m_currentX = 0;
			changeDirection = true;
		}
		if (pixel.m_currentY >= static_cast<int64_t>(m_mainLayer->GetHeight()))
		{
			pixel.m_currentY = m_mainLayer->GetHeight() - 1;
			changeDirection = true;
		}
		if (pixel.m_currentY < 0)
		{
			pixel.m_currentY = 0;
			changeDirection = true;
		}

		// Force a move if we haven't in sometime.
		pixel.m_moveRedirect--;
		if (pixel.m_moveRedirect < 0)
		{
			changeDirection = true;
		}

		// Change direction if needed
		if (changeDirection)
		{
			// Get a new direction, make sure it isn't the opposite of the current or current.
			ColorColliderPixel::Direction newDirection;
			std::uniform_int_distribution<int> dist(0, 3);
			do
			{
				newDirection = (ColorColliderPixel::Direction)dist(m_randomDevice);
			} while (newDirection == pixel.m_currentDirection
				|| (newDirection == ColorColliderPixel::Direction::Down && pixel.m_currentDirection == ColorColliderPixel::Direction::Up)
				|| (newDirection == ColorColliderPixel::Direction::Up && pixel.m_currentDirection == ColorColliderPixel::Direction::Down)
				|| (newDirection == ColorColliderPixel::Direction::Left && pixel.m_currentDirection == ColorColliderPixel::Direction::Right)
				|| (newDirection == ColorColliderPixel::Direction::Right && pixel.m_currentDirection == ColorColliderPixel::Direction::Left)
				|| (newDirection == ColorColliderPixel::Direction::Up && pixel.m_currentY == 0)
				|| (newDirection == ColorColliderPixel::Direction::Left && pixel.m_currentX == 0)
				|| (newDirection == ColorColliderPixel::Direction::Down && pixel.m_currentY == m_mainLayer->GetHeight() - 1)
				|| (newDirection == ColorColliderPixel::Direction::Right && pixel.m_currentX == m_mainLayer->GetWitdh() - 1));

			// Set the direction
			pixel.m_currentDirection = newDirection;
			pixel.m_moveRedirect = -1;
		}

		// Now update the move redirect if we need to.
		if (pixel.m_moveRedirect < 0)
		{
			std::uniform_int_distribution<int> dist(1, 6);
			pixel.m_moveRedirect = dist(m_randomDevice);
		}

		// Draw the new element.
		SolidDrawablePtr drawable = std::make_shared<SolidDrawable>(true);
		drawable->SetPosition(pixel.m_currentX, pixel.m_currentY, 1, 1);
		double color = m_currentColor + (count * GetScaledRealtimeValue(2, 0.0, 0.5, 0.1));
		color = color > 1.0 ? color - 1.0 : color;
		drawable->SetColor(GetRainbowColor(color));

		// Make a fader
		StroberPtr fader = std::make_shared<Strober>(milliseconds(500), milliseconds(500));
		drawable->SetFader(fader);
		
		// Add the layer to the grid
		m_mainLayer->AddDrawable(drawable, 100);
	}

	bool collisionFound = false;
	for (ColorColliderPixel& px1 : m_pixels)
	{
		for (ColorColliderPixel& px2 : m_pixels)
		{
			if (px1.m_id != px2.m_id && px1.m_currentX == px2.m_currentX && px1.m_currentY == px2.m_currentY)
			{
				// Make the swipe
				ExpandingDrawablePtr drop = std::make_shared<ExpandingDrawable>();

				// Set the direction.
				drop->SetStartingPoint(px1.m_currentX, px1.m_currentY);

				// Set the color
				drop->SetColor(GenerateRandomColor(1 - m_currentColor));

				// Add the layer
				m_mainLayer->AddDrawable(drop, 100);

				collisionFound = true;
				break;
			}
		}
		if (collisionFound)
		{
			break;
		}
	}
}