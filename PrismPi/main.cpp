#include "Prism.h"

int main(int argc, char **argv)
{
    // Create our stone
    PrismPtr mysticalStone = std::make_shared<Prism>();

    // Set it up
    mysticalStone->AlignCrystals();

    // Kick it off
    mysticalStone->Prismify();

    // This should never return.
    return 0;
}
