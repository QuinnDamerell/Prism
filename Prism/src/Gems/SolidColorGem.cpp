#include "Gems/SolidColorGem.h"
#include "Colorables/RainbowColorer.h"

using namespace LightFx;
using namespace LightFx::Drawables;
using namespace LightFx::Colorables;
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

    // Make a rainbow color
    RainbowColorerPtr coloer = std::make_shared<RainbowColorer>();
    m_soldColor->SetColorer(coloer);

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
}