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
    bool isBackground = false;
    if(argc > 1)
    {
        // Indicate we are running in the background.
        isBackground = true;
        
        // If we have an arguemnt launch as a daemon        
        pid_t pid, sid;
        
        std::cout << "Launching Prism as a background process\n";
        
        // Fork off of the parent process
        pid = fork();
        if (pid < 0) 
        {
            std::cout << "Failed to launch!\n";       
            exit(EXIT_FAILURE);
        }

        // If success kill the parent process
        if (pid > 0) 
        {
            std::cout << "Successfully launched!\n";            
            exit(EXIT_SUCCESS);
        }

        // Change the file mode mask
        umask(0);
                
        // Create a new SID for the child process
        sid = setsid();
        if (sid < 0) 
        {
            exit(EXIT_FAILURE);
        }
        
        // Change the current working directory
        if ((chdir("/")) < 0) 
        {
            exit(EXIT_FAILURE);
        }
        
        /* Close out the standard file descriptors */
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }   
    
    // Create the host
    PrismCommandHostPtr prism = std::make_shared<PrismCommandHost>();

    // This will block forever
    prism->Run(!isBackground);

    // This should never return.
    return 0;
}
