#include <string>
#include <iostream>
#include <thread>

#include "PrismCommandHost.h"
#include "Prism.h"

void PrismCommandHost::Run(bool blockOnInput)
{
    PrintMessage("Creating the prism...");

    // Create our stone
    PrismPtr mysticalStone = std::make_shared<Prism>();

    PrintMessage("Setting up...");

    // Set it up
    mysticalStone->AlignCrystals();

    // Kick it off
    mysticalStone->Prismify();

    PrintMessage("Running");

    if (blockOnInput)
    {
        PrintMessage("Enter An Intensity: ");

        // Now we will block forever waiting for command line input.
        for (std::string line; std::getline(std::cin, line);)
        {
            // try to get an intensity.
            double intensity = ::atof(line.c_str());
            mysticalStone->SetIntensity(intensity);
            std::cout << "Setting intensity to " << intensity << std::endl;
        }
    }
    else
    {
        // If we don't want to block on input just sleep this thread for a long time.
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::hours(6));
        }
    }



    // This should never return.
    return;
}

void PrismCommandHost::PrintMessage(std::string message)
{
    std::cout << message << std::endl;
}