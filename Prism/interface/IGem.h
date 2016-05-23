#pragma once

#include <chrono>

#include "Common.h"
#include "Drawables/IDrawable.h"
#include "IDrivable.h"

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
};
#pragma once
