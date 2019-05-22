#include "Gems/ExpandingDropsGem.h"
#include "Drawables/ExpandingDrawable.h"


using namespace LightFx;
using namespace LightFx::Drawables;
using namespace LightFx::Colorables;
using namespace Gems;

// Called when the gem should setup.
void ExpandingDropsGem::OnSetup(IDrawablePtr mainLayer)
{
    // Create a layer so we can dim this a little
    DrawablePtr dimmedLayer = std::make_shared<Drawable>();
    dimmedLayer->SetIntensity(0.3);
    m_mainLayer = dimmedLayer;

    // Add the layer
    mainLayer->AddDrawable(dimmedLayer, 50);
}

// Called when it is activated and will be displayed.
void ExpandingDropsGem::OnActivated()
{
}

// Called when the gem is going to stop being displayed.
void ExpandingDropsGem::OnDeactivated()
{
}

// Called just before the prism will render
void ExpandingDropsGem::OnTick(uint64_t tick, std::chrono::milliseconds elapsedTime)
{
    // Take away time.
    m_timeUntilNextDrop -= elapsedTime;

    if (m_timeUntilNextDrop.count() < 0)
    {
        // Update times
        m_minTime += m_isTimeGoingUp ? 20 : -20;
        m_maxTime += m_isTimeGoingUp ? 26 : -26;

        // Update time direction if needed
        if (m_minTime < 100 || m_maxTime < 300)
        {
            m_isTimeGoingUp = true;
        }
        if (m_minTime > 600 || m_maxTime > 1000)
        {
            m_isTimeGoingUp = false;
        }

        // Reset time.
		float scaler = GetScaledRealtimeValue(1, 0.15, 2, 1);
        std::uniform_int_distribution<int> dist(m_minTime * scaler, m_maxTime * scaler);
        m_timeUntilNextDrop = milliseconds(dist(m_randomDevice));

        // Make the swipe
        ExpandingDrawablePtr drop = std::make_shared<ExpandingDrawable>();

        // Set the direction.
        std::uniform_int_distribution<int> startingDist(0, m_mainLayer->GetWitdh() - 1);
        drop->SetStartingPoint(startingDist(m_randomDevice), startingDist(m_randomDevice));

		// If the color factor is less than 0.3, use random.
		float colorFactor = GetScaledRealtimeValue(2, 0, 0.5, 0);
		if (colorFactor < 0.1)
		{
			drop->SetColor(GenerateRandomColor());
		}
		else
		{
			m_currentColor += (colorFactor * 0.2);
			while(m_currentColor > 1)
			{
				m_currentColor -= 1.0;
			}
			drop->SetColor(GetRainbowColor(m_currentColor));
		}

        // Add the layer
        m_mainLayer->AddDrawable(drop, 100);
    }
}