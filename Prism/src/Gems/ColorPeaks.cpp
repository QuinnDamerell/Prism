
#include "Gems/ColorPeaks.h"

#include "Drawables/SolidDrawable.h"
#include "Fadables/Fader.h"

using namespace LightFx;
using namespace LightFx::Drawables;
using namespace LightFx::Fadables;
using namespace Gems;

// Called when the gem should setup.
void ColorPeaks::OnSetup(LightFx::Drawables::IDrawablePtr mainPanel)
{
    // Create the background layer and set it into the main panel
    SolidDrawablePtr background = std::make_shared<SolidDrawable>();
    background->SetPosition(0, 0, mainPanel->GetHeight(), mainPanel->GetWitdh());
    background->SetColor(LightColor(1, 0, 0, 0.05));
    mainPanel->AddDrawable(background, 100);

    // We will just draw to the main panel.
    m_drawableLayer = mainPanel;

    // Grab the panel size
    m_gridSize = mainPanel->GetWitdh();
}

// Called just before the prism will render
void ColorPeaks::OnTick(uint64_t tick, std::chrono::milliseconds elapsedTime)
{
    // Remove the time
    m_timeUntilPeak -= elapsedTime;

    // Check if we should do anything
    if (m_timeUntilPeak.count() <= 0)
    {
        // Get an new time
        std::uniform_int_distribution<int> timeRemainDist(PEAK_CREATE_TIME_MIN_MS, PEAK_CREATE_TIME_MAX_MS);
        m_timeUntilPeak = milliseconds(timeRemainDist(m_randomDevice));

        // Create a drawable pixel and place is randomly
        // NOTE: We send true to solid drawable to incate we want it to be auto cleaned up
        // when the fade is complete.
        std::uniform_int_distribution<int> postitionDist(0, m_gridSize - 1);
        SolidDrawablePtr pixel = std::make_shared<SolidDrawable>(true);
        pixel->SetPosition(postitionDist(m_randomDevice), postitionDist(m_randomDevice), 1, 1);

        // Give it a constant color but random color
        pixel->SetColor(GenerateRandomColor());

        // Add a fade effect to fade the pixel in quickly. 
        FaderPtr fade = std::make_shared<Fader>(1, 0, milliseconds(800));

        // When this fade is done we will reverse it in the callback and then kill the drawable
        fade->SetFinishedCallback(GetSharedPtr<ITimelineObjectCallback>());

        // Add the fade to the pixel
        pixel->SetFader(fade);

        // And add the pixel to the drawing panel
        m_drawableLayer->AddDrawable(pixel, 105);
    }
}

// Called when a timeline completes
void ColorPeaks::OnTimelineFinished(ITimelineObjectPtr timeline)
{
    // This fires when the fade in for a peak is complete

    // First, remove our callback.
    std::weak_ptr<ITimelineObjectCallback> emptyCallback;
    timeline->SetFinishedCallback(emptyCallback);
  
    // Cast to a simple fade to set the to and from
    FaderPtr simpleFade = std::dynamic_pointer_cast<Fader>(timeline);
    if (simpleFade)
    {
        simpleFade->SetTo(0);
        simpleFade->SetFrom(1);
    }

    // Now reset the animation.
    timeline->SetDuration(milliseconds(4000));
}