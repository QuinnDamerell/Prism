// PrismRunner.cpp : Defines the entry point for the console application.
//

#include <windows.h>

#include <iostream>
#include <string>
#include "stdafx.h"
#include "PrismBase.h"

int main()
{

    PrismBasePtr base = std::make_shared<PrismBase>();
    base->Setup();

    HANDLE handle = CreateEvent(NULL, false, false, NULL);
    WaitForSingleObject(handle, INFINITE);
    return 0;
}

