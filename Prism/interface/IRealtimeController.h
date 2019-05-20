#pragma once

#include "Common.h"

DECLARE_SMARTPOINTER(IRealtimeController);
class IRealtimeController 
{
public:
	// Gets the user supplied realtime value if one exists.
	virtual float GetRealtimeValue(int valueNumber) = 0;
};
#pragma once
