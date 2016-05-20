#include <iostream>
#include <random>

#include "libusb.h"
#include "PrismBase.h"

#include "Panel.h"
#include "ConstantRateDriver.h"
#include "OutputBitmap.h"
#include "Layers\SolidColorLayer.h"
#include "Layers\Drawable\DrawableLayer.h"
#include "Layers/Drawable/Draw/DrawablePixel.h"
#include "Layers/Drawable/Color/ConstantColorable.h"
#include "Layers/Drawable/Fade/SimpleFade.h"

using namespace LightFx;
using namespace LightFx::Layers;
using namespace LightFx::Layers::Drawable;


// Sets us up.
int PrismBase::Setup()
{
    // Start up the usb device manager
    m_usbDeviceManager = std::make_unique<UsbDeviceManager>();
    if (int ret = m_usbDeviceManager->Setup(std::dynamic_pointer_cast<IDeviceDiscoverListener>(shared_from_this())) < 0)
    {
        std::cout << "Failed to setup usb device manager";
        return ret;
    }

    m_lightPanel = std::make_shared<Panel>(8, 8);
    m_lightPanel->SetPanelRenderedCallback(std::dynamic_pointer_cast<IPanelRenderedCallback>(shared_from_this()));
    
    SolidColorLayerPtr layer = std::make_shared<SolidColorLayer>();
    layer->SetColor(Pixel(1, 0, 0, .2));
    m_lightPanel->AddLayer(std::dynamic_pointer_cast<ILayer>(layer), 100);

    // Create a drawable layer, we will use this later
    m_drawable = std::make_shared<DrawableLayer>();
    m_lightPanel->AddLayer(std::dynamic_pointer_cast<ILayer>(m_drawable), 105);

    m_driver = std::make_shared<ConstantRateDriver>();
    m_driver->AddDriveable(std::dynamic_pointer_cast<IDrivable>(m_lightPanel));
    m_driver->Start(milliseconds(16));

    //// Make the prism panel
    //m_panel = std::make_shared<PrismPanel>(std::dynamic_pointer_cast<IWriteablePixelEndpoint>(shared_from_this()));

    //// Make the fx root
    //m_fxRoot = std::make_shared<LightFxRoot>(std::dynamic_pointer_cast<ILightFxListener>(m_panel));
    //m_fxRoot->StartAnimation(33);

    //// Make the random color effect
    //RandomColorPeakFadeEffectPtr randomColor = std::make_shared<RandomColorPeakFadeEffect>("RandomColorEffect");

    //// Make the color layer
    //SolidRectColorLayerPtr colorLayer = std::make_shared<SolidRectColorLayer>("ColorLayer", LightFxCordornate(8,8), LightFxCordornate(0,0), LightFxColor(1,0,0,0));
    //randomColor->SetChildLayer(colorLayer);

    //// Add the effect
    //m_fxRoot->AddLayer(randomColor);
    //
    //// Tell it to start
    //randomColor->Begin();


  //  std::default_random_engine generator;
  //  std::uniform_int_distribution<int> distribution(0, 7);
  //  std::uniform_int_distribution<int> colorDistro(0, 200);
  //  int dice_roll = distribution(generator);

  //  while (true)
  //  {
  //      int randX = distribution(generator);
  //      int randY = distribution(generator);

  //      for (int x = 0; x < 8; x++)
  //      {
  //          for (int y = 0; y < 8; y++)
  //          {
  //              if (x == randX && y == randY)
  //              {
  //                  if (cou == 3)
  //                  {
  //                      PanelPixel pixel((uint8_t)colorDistro(generator), (uint8_t)colorDistro(generator), (uint8_t)colorDistro(generator));
  //                      m_panel->PixelUpdate(x, y, pixel);
  //                  }
  //                  cou++;
  //                  if (cou > 3)
  //                  {
  //                      cou = 0;
  //                  }                   
  //                 
  //              }
  //              else
  //              {
  //                  PanelPixel pixel = m_panel->GetCurrentPixel(x, y);
  //                  if (pixel.red < 10)
  //                  {
  //                      pixel.red = 10;
  //                  }
  //                  if (pixel.blue < 10)
  //                  {
  //                      pixel.blue = 10;
  //                  }
  //                  if (pixel.green < 10)
  //                  {
  //                      pixel.green = 10;
  //                  }

  //                  pixel.red -= 10;
  //                  pixel.blue -= 10;
  //                  pixel.green -= 10;

  //                  m_panel->PixelUpdate(x, y, pixel);
  //              }
  //          }

  //      }
  //       /*   if (c == cout)
  //          {
  //       
  //          }
  //          else
  //          {
  //              PanelPixel pixel = m_panel->GetCurrentPixel(c, c);

  //              if (pixel.red < 80)
  //              {
  //                  pixel.red = 80;
  //              }
  //              if (pixel.blue < 40)
  //              {
  //                  pixel.blue = 40;
  //              }
  //              if (pixel.green < 80)
  //              {
  //                  pixel.green = 80;
  //              }


  //              pixel.red -= 80;
  //              pixel.blue -= 40;
  //              pixel.green -= 80;
  //

  //              m_panel->PixelUpdate(c, c, pixel);
  //          }
  //      }*/
  //      m_panel->CommitFrame();

  //      std::this_thread::sleep_for(std::chrono::milliseconds(50));
  //  }

    return 0;


}

void PrismBase::OnDeviceAdded(IWriteablePixelEndpointPtr device)
{
    m_pixelEndpoint = device;
}

void PrismBase::OnDeviceRemoved(IWriteablePixelEndpointPtr device)
{
    m_pixelEndpoint = std::weak_ptr<IWriteablePixelEndpoint>();
}

int renderCount = 0;
std::random_device rd;
std::uniform_int_distribution<int> dist(0, 7);
std::uniform_int_distribution<int> largerDist(50, 300);
std::uniform_real_distribution<double> doubleDist(0, 1);

void PrismBase::OnPanelRendered() 
{
    if (auto pixelEndpoint = m_pixelEndpoint.lock())
    {
        OutputBitmapPtr bitmap = m_lightPanel->GetOutputBitmap();
        pixelEndpoint->WritePixels(bitmap->GetPixelArray(), bitmap->GetPixelArrayLength());
    }   

    // Every 20 renders lets add a new pixel
    renderCount--;
    if (renderCount < 1)
    {
        renderCount = largerDist(rd);

        // Create a drawable pixel and place is randomly
        DrawablePixelPtr pixel = std::make_shared<DrawablePixel>(dist(rd), dist(rd));

        // Give it a constant color but random color
        Pixel pix(.6, doubleDist(rd), doubleDist(rd), doubleDist(rd));
        ConstantColorablePtr color = std::make_shared<ConstantColorable>(pix);

        // And make a nice fade effect
        SimpleFadePtr fade = std::make_shared<SimpleFade>(0, 1.0, milliseconds(800));
        fade->SetFinishedCallback(shared_from_this());

        // Set the color to the pixel
        pixel->SetColorable(color);

        // Add the fade to the color
        color->SetFadable(fade);

        // And add the pixel to the drawing panel
        m_drawable->AddDrawable(pixel, 99);
    }
    
}

void PrismBase::OnTimelineFinished(std::shared_ptr<TimelineObject> timeline)
{
    std::weak_ptr<ITimelineObjectCallback> temp;
    timeline->SetFinishedCallback(temp);
    timeline->SetDuration(milliseconds(6000));
    SimpleFadePtr simpleFade = std::dynamic_pointer_cast<SimpleFade>(timeline);
    if (simpleFade)
    {
        simpleFade->SetToAndFrom(0, 1);
    }
}

//void PrismBase::WritePixels(uint8_t* pixelArray, uint32_t length)
//{
//    if (auto endpoint = m_pixelEndpoint.lock())
//    {
//        endpoint->WritePixels(pixelArray, length);
//    }
//}