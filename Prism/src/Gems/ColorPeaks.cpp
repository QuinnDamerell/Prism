
#include "Gems/ColorPeaks.h"

#include "Layers/SolidColorLayer.h"
#include "Layers/Drawable/Draw/DrawablePixel.h"
#include "Layers/Drawable/Color/ConstantColorable.h"
#include "Layers/Drawable/Fade/SimpleFade.h"

using namespace LightFx;
using namespace LightFx::Layers;
using namespace LightFx::Layers::Drawable;
using namespace Gems;

// Called when the gem should setup.
void ColorPeaks::OnSetup(PanelPtr mainPanel)
{
    // Create the background layer and set it into the main panel
    SolidColorLayerPtr background = std::make_shared<SolidColorLayer>();
    background->SetColor(Pixel(1, 0, .2, 0));
    mainPanel->AddLayer(std::dynamic_pointer_cast<ILayer>(background), 100);
    
    // Create a drawable layer, we will use this later to make the peaks
    m_drawableLayer = std::make_shared<DrawableLayer>();
    mainPanel->AddLayer(std::dynamic_pointer_cast<ILayer>(m_drawableLayer), 105);

    // Grab the panel size
    m_gridSize = mainPanel->GetOutputBitmap()->GetWidth();
}

// Called just before the prism will render
void ColorPeaks::OnPreRender(uint64_t tick, std::chrono::milliseconds elapsedTime)
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
        std::uniform_int_distribution<int> postitionDist(0, m_gridSize - 1);
        DrawablePixelPtr pixel = std::make_shared<DrawablePixel>(postitionDist(m_randomDevice), postitionDist(m_randomDevice));

        // Give it a constant color but random color
        Pixel randomColor = GenerateRandomColor();
        ConstantColorablePtr color = std::make_shared<ConstantColorable>(randomColor);

        // Add a fade effect to fade the pixel in quickly. 
        SimpleFadePtr fade = std::make_shared<SimpleFade>(0, 1.0, milliseconds(800));

        // When this fade is done we will reverse it in the callback and then kill the drawable
        fade->SetFinishedCallback(shared_from_this());

        // Set the color to the pixel
        pixel->SetColorable(color);

        // Add the fade to the color
        color->SetFadable(fade);

        // And add the pixel to the drawing panel
        m_drawableLayer->AddDrawable(pixel, 99);

    }
}

// Called when a timeline completes
void ColorPeaks::OnTimelineFinished(TimelineObjectPtr timeline)
{
    // This fires when the fade in for a peak is complete

    // First, remove our callback.
    std::weak_ptr<ITimelineObjectCallback> emptyCallback;
    timeline->SetFinishedCallback(emptyCallback);

    // Also set the drawable to clean up when this fade out is done.
    timeline->ShouldCleanupWhenFinshed(true);

    // Now reset the animation.
    timeline->SetDuration(milliseconds(6000));

    // Cast to a simple fade to set the to and from
    SimpleFadePtr simpleFade = std::dynamic_pointer_cast<SimpleFade>(timeline);
    if (simpleFade)
    {
        simpleFade->SetToAndFrom(0, 1);
    }
}