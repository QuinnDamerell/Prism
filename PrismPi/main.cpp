#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <iostream>

#include "PrismCommandHost.h"

int main(int argc, char **argv)
{
    bool dontTakeInput = argc > 1;
    
    // Create the host
    PrismCommandHostPtr prism = std::make_shared<PrismCommandHost>();

    // This will block forever
    prism->Run(!dontTakeInput);

    // This should never return.
    return 0;
}
