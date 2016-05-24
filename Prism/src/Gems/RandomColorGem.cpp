#include "Gems/RandomColorGem.h"
#include "Fadables/Fader.h"

using namespace LightFx;
using namespace LightFx::Drawables;
using namespace LightFx::Fadables;
using namespace Gems;

// Called when the gem should setup.
void RandomColorGem::OnSetup(IDrawablePtr mainLayer)
{
    // Make all of the drawables
    m_pixelArray.resize(mainLayer->GetWitdh());
    for (int x = 0; x < mainLayer->GetWitdh(); x++)
    {
        m_pixelArray[x].resize(mainLayer->GetHeight());
        for (int y = 0; y < mainLayer->GetHeight(); y++)
        {
            // make the layer
            m_pixelArray[x][y] = std::make_shared<SolidDrawable>();
            m_pixelArray[x][y]->SetPosition(x, y, 1, 1);
            mainLayer->AddDrawable(m_pixelArray[x][y], 5);

            // Make faders for them also.
            FaderPtr fader = std::make_shared<Fader>();
            fader->SetFrom(0);
            fader->SetTo(1);
            m_pixelArray[x][y]->SetFader(fader);
        }
    }
}

// Called when it is activated and will be displayed.
void RandomColorGem::OnActivated()
{
}

// Called when the gem is going to stop being displayed.
void RandomColorGem::OnDeactivated()
{
}

// Called just before the prism will render
void RandomColorGem::OnTick(uint64_t tick, std::chrono::milliseconds elapsedTime)
{
    // Only make a change every 5 renders.
    m_skipValue--;
    if (m_skipValue > 0)
    {
        return;       
    }
    m_skipValue = 5;

    // Select a random pixel
    std::uniform_int_distribution<int> dist(0, m_pixelArray.size() - 1);
    int x = dist(m_randomDevice);
    int y = dist(m_randomDevice);
    m_pixelArray[x][y]->SetColor(GenerateRandomColor(true));
    m_pixelArray[x][y]->GetFader()->SetDuration(milliseconds(2000));
}