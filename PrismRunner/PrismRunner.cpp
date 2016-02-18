// PrismRunner.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <string>
#include "stdafx.h"
#include "PrismBase.h"

int main()
{
     PrismBasePtr base = std::make_shared<PrismBase>();
    base->Setup();

    for (std::string line; std::getline(std::cin, line);) {
        std::cout << line << std::endl;
    }
    return 0;
}

