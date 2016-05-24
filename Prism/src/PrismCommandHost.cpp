#include <string>
#include <iostream>

#include "PrismCommandHost.h"
#include "Prism.h"

void PrismCommandHost::Run()
{
    PrintMessage("Creating the prism...");

    // Create our stone
    PrismPtr mysticalStone = std::make_shared<Prism>();

    PrintMessage("Setting up...");

    // Set it up
    mysticalStone->AlignCrystals();

    // Turn it down a little
    mysticalStone->SetIntensity(.8);

    // Kick it off
    mysticalStone->Prismify();

    PrintMessage("Running");

    PrintMessage("Enter An Intensity: ");

    // Now we will block forever waiting for command line input.
    for (std::string line; std::getline(std::cin, line);)
    {
        // try to get an intensity.
        double intensity = ::atof(line.c_str());
        mysticalStone->SetIntensity(intensity);
        std::cout << "Setting intensity to " << intensity << std::endl;
    }

    // This should never return.
    return;
}

void PrismCommandHost::PrintMessage(std::string message)
{
    std::cout << message << std::endl;
}