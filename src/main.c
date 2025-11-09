 #include "shell.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    char* cmdline;
    Pipeline* pipeline;

    // Enable TAB-completion
    rl_bind_key('\t', rl_complete);

    // Loop indefinitely, reading commands
    while ((cmdline = readline(PROMPT)) != NULL) {
        
        // Skip empty commands
        if (cmdline[0] != '\0') {
            
            // Add non-empty commands to Readline's history
            add_history(cmdline);

            // --- Parse the command line into a pipeline ---
            pipeline = parse_cmdline(cmdline);

            if (pipeline == NULL || pipeline->num_commands == 0) {
                // Parse error or empty command (e.g., just whitespace)
                free(cmdline);
                if (pipeline) free_pipeline(pipeline);
                continue;
            }

            // --- Handle Commands ---
            // Built-in commands (cd, exit, help) are special.
            // They must run in the parent shell and cannot be piped (in this simple shell).
            if (pipeline->num_commands == 1) {
                SimpleCommand* cmd = &pipeline->commands[0];
                
                // A built-in command cannot have I/O redirection in this shell
                if (cmd->inputFile == NULL && cmd->outputFile == NULL) {
                    if (handle_builtin(cmd->args) == 1) {
                        // It was a built-in, so we're done.
                        // free_pipeline and free(cmdline) are below.
                    } else {
                        // Not a built-in, execute it
                        execute_pipeline(pipeline);
                    }
                } else {
                     // A simple command with redirection, execute it
                     execute_pipeline(pipeline);
                }
            } else {
                // It's a pipeline, execute it
                execute_pipeline(pipeline);
            }

            // Free the memory allocated by the parser
            free_pipeline(pipeline);
        }
        
        // Free the memory allocated by readline()
        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}