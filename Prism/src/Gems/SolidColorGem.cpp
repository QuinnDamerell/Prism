#include "Gems/SolidColorGem.h"

using namespace LightFx;
using namespace LightFx::Drawables;
using namespace Gems;

// Called when the gem should setup.
void SolidColorGem::OnSetup(IDrawablePtr mainLayer)
{
    // make the layer
    m_soldColor = std::make_shared<SolidDrawable>();

    // Set the size
    m_soldColor->SetPosition(0, 0, mainLayer->GetHeight(), mainLayer->GetWitdh());

    // Dim the panel a little so it isn't so bright.
    m_soldColor->SetIntensity(0.4);

    // Add the layer
    mainLayer->AddDrawable(m_soldColor, 5);
}

// Called when it is activated and will be displayed.
void SolidColorGem::OnActivated()
{
}

// Called when the gem is going to stop being displayed.
void SolidColorGem::OnDeactivated()
{
}

// Called just before the prism will render
void SolidColorGem::OnTick(uint64_t tick, std::chrono::milliseconds elapsedTime)
{
    m_timeUsedInCycle -= elapsedTime;
    if (m_timeUsedInCycle.count() < 0)
    {
        m_timeUsedInCycle = m_fullCycleTime;
    }

    // Figure out how far we are
    double progress = static_cast<double>(m_timeUsedInCycle.count()) / static_cast<double>(m_fullCycleTime.count());

    // Set the color    
    m_soldColor->SetColor(GetRainbowColor(progress));
}