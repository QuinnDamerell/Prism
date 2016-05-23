// PrismRunner.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "PrismCommandHost.h"

int main()
{
    // Create the host
    PrismCommandHostPtr prism = std::make_shared<PrismCommandHost>();

    // This will block forever
    prism->Run();

    // This should never return.
    return 0;
}

