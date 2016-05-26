#include "Gems/RandomColorGem.h"
#include "Colorables/CrossfadeColorer.h"
#include "Fadables/Fader.h"


using namespace LightFx;
using namespace LightFx::Drawables;
using namespace LightFx::Colorables;
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
            fader->SetFinishedCallback(GetSharedPtr<ITimelineObjectCallback>());
            m_pixelArray[x][y]->SetFader(fader);

            CrossfadeColorerPtr colorer = std::make_shared<CrossfadeColorer>();
            colorer->SetFromColor(Pixel(0,0,0));
            m_pixelArray[x][y]->SetColorer(colorer);
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
    // Update the color swtich time
    m_timeUntilColorSwitch -= elapsedTime;
    if (m_timeUntilColorSwitch.count() < 0)
    {
        m_useRainbowColor = !m_useRainbowColor;
        m_timeUntilColorSwitch = m_colorCycleTime;
    }

    // Only make a change every 5 renders.
    m_skipValue--;
    if (m_skipValue > 0)
    {
        return;
    }
    m_skipValue = 3;

    // Select a random pixel
    std::uniform_int_distribution<int> dist(0, m_pixelArray.size() - 1);
    int x = dist(m_randomDevice);
    int y = dist(m_randomDevice);
    
    // Get the colorer
    CrossfadeColorerPtr crossFade = std::dynamic_pointer_cast<CrossfadeColorer>(m_pixelArray[x][y]->GetColorer());
    crossFade->SetFromColor(m_pixelArray[x][y]->GetColor());
    crossFade->SetDuration(milliseconds(2000));

    // Set the to color
    if (m_useRainbowColor)
    {
        // Update the current color value
        m_currentColorValue += .002;
        if (m_currentColorValue > 1)
        {
            m_currentColorValue = 0;
        }

        crossFade->SetToColor(GetRainbowColor(m_currentColorValue));
    }
    else
    {
        crossFade->SetToColor(GenerateRandomColor(true));
    }

    // Set the fader
    m_pixelArray[x][y]->GetFader()->SetTo(1.0);
    m_pixelArray[x][y]->GetFader()->SetFrom(m_pixelArray[x][y]->GetIntensity());
    m_pixelArray[x][y]->GetFader()->SetDuration(milliseconds(2000));    
}

void RandomColorGem::OnTimelineFinished(ITimelineObjectPtr timeline)
{
    // Try to get the fader
    FaderPtr fader = std::dynamic_pointer_cast<Fader>(timeline);
    if (fader && fader->GetTo() > 0.99)
    {
        // If this fader was going up reverse it back down.
        fader->SetTo(0);
        fader->SetFrom(1);
        fader->SetDuration(milliseconds(800));
    }
}
