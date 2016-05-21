#include <unistd.h>

#include "Prism.h"

int main(int argc, char **argv)
{
    // Create our stone
    PrismPtr mysticalStone = std::make_shared<Prism>();

    // Set it up
    mysticalStone->AlignCrystals();

    // Kick it off
    mysticalStone->Prismify();
    
    while(true)
    {
        sleep(999999999);
    }

    // This should never return.
    return 0;
}
