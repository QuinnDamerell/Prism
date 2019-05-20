#pragma once

#include "Common.h"

DECLARE_SMARTPOINTER(IControlCommandHandler);
class IControlCommandHandler
{
public:   

    // Fired when the intensity has changed.
    virtual void IntensityChanged(double intensity) = 0;

    // Fired when the enabled gems is changed.
    virtual void EnabledGemsChanged() = 0;

    // Running time changed
    virtual void GemRunningTimeChanged() = 0;

    // Active hours updated
    virtual void ActiveHoursUpdate() = 0;

	// Fired when realtime contorls are updated.
	virtual void IncomingRealTimeControl(float value1, float value2, float value3, float value4) = 0;
};
