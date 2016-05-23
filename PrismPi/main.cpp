#include "PrismCommandHost.h"

int main(int argc, char **argv)
{
    // Create the host
    PrismCommandHostPtr prism = std::make_shared<PrismCommandHost>();

    // This will block forever
    prism->Run();

    // This should never return.
    return 0;
}
