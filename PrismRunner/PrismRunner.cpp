// PrismRunner.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Prism.h"

#include <windows.h>

int main()
{
    // Create our stone
    PrismPtr mysticalStone = std::make_shared<Prism>();

    // Set it up
    mysticalStone->AlignCrystals();

    // Kick it off
    mysticalStone->Prismify();

    // Block forever on a handle.
    HANDLE handle = CreateEvent(NULL, false, false, NULL);
    WaitForSingleObject(handle, INFINITE);

    // This should never return.
    return 0;
}

