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
		m_snakes.emplace_back(ColorColliderSnake(static_cast<int16_t>(i)));
	}

	std::uniform_int_distribution<int> dist(0, static_cast<int>(mainLayer->GetHeight()) - 1);
	for (ColorColliderSnake& snake : m_snakes)
	{
		// Create a drawable to start off with.
		SolidDrawablePtr tmp = std::make_shared<SolidDrawable>();
		tmp->SetPosition(dist(m_randomDevice), dist(m_randomDevice), 1, 1);
		snake.m_pixels.emplace_back(tmp);
		snake.m_currentDirection = ColorColliderSnake::Direction::Down;
		snake.m_moveRedirect = -1;
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

	int collisionX = -1;
	int collisionY = -1;

	int count = 0;
	for (ColorColliderSnake& snake : m_snakes)
	{
		count++;

		// Make a move
		int newX = 0;
		int newY = 0;

		bool madeMove = false;
		bool needsToChangeDirection = false;
		bool triedUp = false;
		bool triedDown = false;
		bool triedLeft = false;
		bool triedRight = false;

		// Don't move in the oposite direct that we started.
		switch(snake.m_currentDirection)
		{
			case ColorColliderSnake::Direction::Down:
				triedUp = true;
				break;
			case ColorColliderSnake::Direction::Up:
				triedDown = true;
				break;
			case ColorColliderSnake::Direction::Left:
				triedRight = true;
				break;
			case ColorColliderSnake::Direction::Right:
				triedLeft = true;
				break;
		}

		// Check if we just want to randomly change position
		snake.m_moveRedirect--;
		if (snake.m_moveRedirect < 0)
		{
			needsToChangeDirection = true;
		}

		while (!madeMove)
		{
			// Take the current position and try to keep moving as we want to.
			newX = snake.m_pixels.front()->GetSolidDrawableX();
			newY = snake.m_pixels.front()->GetSolidDrawableY();

			// Update position
			switch (snake.m_currentDirection)
			{
			case ColorColliderSnake::Direction::Down:
				newY++;
				break;
			case ColorColliderSnake::Direction::Up:
				newY--;
				break;
			case ColorColliderSnake::Direction::Left:
				newX--;
				break;
			case ColorColliderSnake::Direction::Right:
				newX++;
				break;
			}

			// Check if we are moving out of bounds
			if (newX > static_cast<int64_t>(m_mainLayer->GetWitdh() - 1))
			{
				triedRight = true;
				needsToChangeDirection = true;
			}
			if (newX < 0)
			{
				triedLeft = true;
				needsToChangeDirection = true;
			}
			if (newY > static_cast<int64_t>(m_mainLayer->GetHeight() - 1))
			{
				triedDown = true;
				needsToChangeDirection = true;
			}
			if (newY < 0)
			{
				triedUp = true;
				needsToChangeDirection = true;
			}

			// Make sure we don't interect ourselves or anyother snake
			if (!needsToChangeDirection)
			{
				for (ColorColliderSnake& itr : m_snakes)
				{
					int count = 0;
					for (SolidDrawablePtr& ptr : itr.m_pixels)
					{
						if (newY == ptr->GetSolidDrawableY() && newX == ptr->GetSolidDrawableX())
						{
							// If the fronts hit make a collision.
							if (count == 0 && itr.m_id != snake.m_id)
							{
								collisionX = newX;
								collisionY = newY;
							}							

							// Invalidate the current position.
							switch (snake.m_currentDirection)
							{
							case ColorColliderSnake::Direction::Down:
								triedDown = true;
								break;
							case ColorColliderSnake::Direction::Up:
								triedUp = true;
								break;
							case ColorColliderSnake::Direction::Left:
								triedLeft = true;
								break;
							case ColorColliderSnake::Direction::Right:
								triedRight = true;
								break;
							}
							needsToChangeDirection = true;
							break;
						}
						count++;
					}
					if (needsToChangeDirection)
					{
						break;
					}
				}
			}

			// Try to change the direction if needed, we are only allowed to pick valid directions.
			if (needsToChangeDirection)
			{
				// If we don't have any where to go, give up.
				if (triedUp && triedDown && triedLeft && triedRight)
				{
					newX = snake.m_pixels.front()->GetSolidDrawableX();
					newY = snake.m_pixels.front()->GetSolidDrawableY();
					madeMove = true;
					break;
				}

				ColorColliderSnake::Direction newDirection;
				std::uniform_int_distribution<int> dist(0, 3);
				do
				{
					newDirection = (ColorColliderSnake::Direction)dist(m_randomDevice);
				} while (newDirection == ColorColliderSnake::Direction::Down && triedDown
					||   newDirection == ColorColliderSnake::Direction::Up && triedUp
					||   newDirection == ColorColliderSnake::Direction::Left && triedLeft
					||   newDirection == ColorColliderSnake::Direction::Right && triedRight);

				// Set our new state and loop again to try a new direction.
				snake.m_currentDirection = newDirection;
				needsToChangeDirection = false;
				continue;
			}
			else
			{
				madeMove = true;
			}
		}

		// Now update the move redirect if we need to.
		if (snake.m_moveRedirect < 0)
		{
			std::uniform_int_distribution<int> dist(1, 6);
			snake.m_moveRedirect = dist(m_randomDevice);
		}

		// Draw the new element.
		SolidDrawablePtr drawable = std::make_shared<SolidDrawable>(true);
		drawable->SetPosition(newX, newY, 1, 1);
		double color = m_currentColor + (count * 0.1);
		color = color > 1.0 ? color - 1.0 : color;
		drawable->SetColor(GetRainbowColor(color));

		// Add the new pixel to our list.
		snake.m_pixels.emplace_front(drawable);
	
		// Add the layer to the grid
		m_mainLayer->AddDrawable(drawable, 100);

		// Clean up any extra pixels in the list
		int count = 0;
		while (snake.m_pixels.size() > GetScaledRealtimeValue(2, 1, 20, 5))
		{
			// Only take a max of two off per cycle.
			count++;
			if (count > 2)
			{
				break;
			}
			DrawablePtr ptr = snake.m_pixels.back();
			snake.m_pixels.pop_back();
			ptr->SetCleanupFlag(true);
		}
	}

	// Make the collision effect if we hit
	if (collisionX != -1 && collisionY != -1)
	{
		//Make the swipe
		ExpandingDrawablePtr drop = std::make_shared<ExpandingDrawable>();

		// Set the direction.
		drop->SetStartingPoint(collisionX, collisionY);

		// Set the color
		drop->SetColor(GetRainbowColor(1 - m_currentColor));

		// Add the layer
		m_mainLayer->AddDrawable(drop, 100);
	}
}