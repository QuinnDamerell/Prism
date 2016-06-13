#pragma once

#include "Common.h"

DECLARE_SMARTPOINTER(IControlCommandHandler);
class IControlCommandHandler
{
public:    
    // Sets the intensity
    virtual void SetIntensity(double intensity) = 0;

    // Gets the current intensity
    virtual double GetIntensity() = 0;
};
