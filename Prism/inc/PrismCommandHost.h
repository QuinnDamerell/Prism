#pragma once

#include <string>

#include "Common.h"

DECLARE_SMARTPOINTER(PrismCommandHost);
class PrismCommandHost
{
public:
    // When called this will setup prism and block forever waiting on
    // commands.
    void Run();

private:
    void PrintMessage(std::string message);
};