  #include "shell.h"
#include <stdio.h>  // For printf, NULL, stdin
#include <stdlib.h> // For free, exit

// *** DEFINE GLOBAL HISTORY VARIABLES ***
// (These are declared 'extern' in shell.h)
char* history[HISTORY_SIZE];
int history_total_count = 0;
int history_current_size = 0;

int main() {
    char* cmdline;
    char** arglist;

    // *** Initialize history array to NULLs ***
    for (int i = 0; i < HISTORY_SIZE; i++) {
        history[i] = NULL;
    }

    // Loop indefinitely, reading commands
    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {
        
        // Skip empty commands (just pressing Enter)
        if (cmdline[0] == '\0') {
            free(cmdline);
            continue;
        }

        // *** TASK 4: HANDLE !n RE-EXECUTION ***
        // This must happen *before* adding to history
        char* new_cmdline = handle_bang_exec(cmdline);
        
        if (new_cmdline == NULL) {
            // Error (e.g., "!500" out of bounds), message already printed
            free(cmdline); // Free the original "!500" string
            continue;      // Get a new prompt
        } 
        else if (new_cmdline != cmdline) {
            // !n was successful
            free(cmdline);         // Free the original "!n" string
            cmdline = new_cmdline; // cmdline now points to the *new* string from history
            printf("%s\n", cmdline); // Echo the command being executed
        }
        // If new_cmdline == cmdline, it wasn't a '!' command, so we just proceed.


        // *** TASK 1: ADD TO HISTORY STORAGE ***
        // Add the (potentially expanded) command to history
        add_to_history(cmdline);

        // --- Tokenize and Execute ---
        if ((arglist = tokenize(cmdline)) != NULL) {
            
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

    // --- Cleanup ---
    // Free any remaining strings in the history array
    for (int i = 0; i < history_current_size; i++) {
        if (history[i] != NULL) {
            free(history[i]);
        }
    }

    printf("\nShell exited.\n");
    return 0;
}