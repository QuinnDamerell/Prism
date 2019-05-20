#pragma once

#include <chrono>

#include "Common.h"
#include "Drawables/IDrawable.h"
#include "IDrivable.h"
#include "IRealtimeController.h"

DECLARE_SMARTPOINTER(IGem);
class IGem :
    public LightFx::IDrivable
{
public:
    // Called when the gem should setup.
    virtual void OnSetup(LightFx::Drawables::IDrawablePtr mainLayer) = 0;

    // Called when it is activated and will be displayed.
    virtual void OnActivated() = 0;

    // Called when the gem is going to stop being displayed.
    virtual void OnDeactivated() = 0;

    // Called just before the prism will render
    virtual void OnTick(uint64_t tick, std::chrono::milliseconds elapsedTime) = 0;

	void SetRealtimeController(IRealtimeControllerWeakPtr realtimeController)
	{
		m_realtimeController = realtimeController;
	}

protected:

	IRealtimeControllerPtr GetRealtimeController()
	{
		return m_realtimeController.lock();
	}

	float GetRealtimeValue(int valueNumber)
	{
		auto ptr = GetRealtimeController();
		if (ptr != nullptr)
		{
			return ptr->GetRealtimeValue(valueNumber);
		}
		return -1;
	}

	float GetScaledRealtimeValue(int valueNumber, float min, float max, float def)
	{
		float scale = GetRealtimeValue(valueNumber);
		if (scale == -1)
		{
			return def;
		}

		// Find the correct value.
		float range = max - min;
		float offset = range * scale;
		return min + offset;
	}
	
private:
	IRealtimeControllerWeakPtr m_realtimeController;
};
#pragma once
