 #include "shell.h"
#include <stdio.h>
#include <stdlib.h>

// *** REMOVED GLOBAL HISTORY VARIABLE DEFINITIONS ***
// (char* history[...], history_total_count, etc. are gone)

int main() {
    char* cmdline;
    char** arglist;

    // *** REMOVED history array initialization loop ***
    
    // *** ADD READLINE INITIALIZATION ***
    // This enables TAB-completion
    rl_bind_key('\t', rl_complete);

    // Loop indefinitely, reading commands
    // *** REPLACE read_cmd with readline ***
    // readline() returns NULL on Ctrl+D (EOF)
    while ((cmdline = readline(PROMPT)) != NULL) {
        
        // Skip empty commands (just pressing Enter)
        // We only add non-empty commands to history
        if (cmdline[0] != '\0') {
            
            // *** REPLACE add_to_history with Readline's version ***
            add_history(cmdline);

            // *** REMOVED handle_bang_exec call ***
            // Readline handles history and line editing,
            // and we'll use its history list for the 'history' command.

            // --- Tokenize and Execute (This logic is unchanged) ---
            if ((arglist = tokenize(cmdline)) != NULL) {
                
                if (handle_builtin(arglist) == 0) {
                    execute(arglist);
                }

                // Free the memory allocated by tokenize()
                for (int i = 0; arglist[i] != NULL; i++) {
                    free(arglist[i]);
                }
                free(arglist);
            }
        }
        
        // Free the memory allocated by readline()
        free(cmdline);
    }

    // *** REMOVED custom history cleanup loop ***

    printf("\nShell exited.\n");
    return 0;
}