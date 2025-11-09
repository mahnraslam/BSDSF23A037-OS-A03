 #include "shell.h"
#include <stdio.h>  // For printf, NULL, stdin
#include <stdlib.h> // For free

/*
 * Note: The prototypes for read_cmd, tokenize, handle_builtin,
 * and execute should be in your "shell.h" file.
 */

int main() {
    char* cmdline;
    char** arglist;

    // Loop indefinitely, reading commands
    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {
        
        // Parse the command line into arguments
        if ((arglist = tokenize(cmdline)) != NULL) {
            
            // *** THIS IS THE MODIFIED LOGIC ***
            //
            // First, check if the command is a built-in.
            // handle_builtin() will execute it and return 1 if it is.
            if (handle_builtin(arglist) == 0) {
                // If handle_builtin() returns 0, it was not a built-in.
                // Proceed to execute it as an external command.
                execute(arglist);
            }

            // Free the memory allocated by tokenize()
            for (int i = 0; arglist[i] != NULL; i++) {
                free(arglist[i]);
            }
            free(arglist);
        }
        
        // Free the memory allocated by read_cmd()
        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}