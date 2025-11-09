#include "shell.h"

// Helper function to parse a single command segment (e.g., "ls -l > file.txt")
// It populates a SimpleCommand struct.
static void parse_simple_command(char* cmd_str, SimpleCommand* cmd) {
    int arg_index = 0;
    cmd->inputFile = NULL;
    cmd->outputFile = NULL;
    
    // We use strsep to tokenize the string by spaces
    char* token;
    char* rest = cmd_str;

    while ((token = strsep(&rest, " \t\n")) != NULL) {
        // Skip empty tokens (e.g., from multiple spaces)
        if (*token == '\0') {
            continue;
        }

        // Check for input redirection
        if (strcmp(token, "<") == 0) {
            // The next token MUST be the filename
            token = strsep(&rest, " \t\n");
            if (token == NULL || *token == '\0') {
                fprintf(stderr, "Syntax error: no file for input redirection.\n");
                // In a real shell, we'd set an error flag
                return; 
            }
            cmd->inputFile = strdup(token);
        }
        // Check for output redirection
        else if (strcmp(token, ">") == 0) {
            // The next token MUST be the filename
            token = strsep(&rest, " \t\n");
            if (token == NULL || *token == '\0') {
                fprintf(stderr, "Syntax error: no file for output redirection.\n");
                return;
            }
            cmd->outputFile = strdup(token);
        }
        // It's a regular argument
        else {
            if (arg_index < MAX_ARGS) {
                cmd->args[arg_index] = strdup(token);
                arg_index++;
            } else {
                fprintf(stderr, "Too many arguments for command.\n");
                break; // Stop parsing args for this command
            }
        }
    }
    // Null-terminate the argument list for execvp
    cmd->args[arg_index] = NULL;
}

/**
 * @brief Parses the entire command line into a Pipeline structure.
 * First, it splits the line by pipes '|'.
 * Then, it parses each segment into a SimpleCommand.
 */
Pipeline* parse_cmdline(char* cmdline) {
    // Edge case: empty command line
    if (cmdline == NULL || cmdline[0] == '\0' || cmdline[0] == '\n') {
        return NULL;
    }

    Pipeline* pipeline = (Pipeline*)malloc(sizeof(Pipeline));
    pipeline->num_commands = 0;

    char* pipe_segment;
    char* rest = cmdline;

    // First stage: Split by pipes '|'
    while ((pipe_segment = strsep(&rest, "|")) != NULL && 
           pipeline->num_commands < MAX_PIPE_SEGS) {
        
        if (*pipe_segment == '\0') {
            fprintf(stderr, "Syntax error: empty command in pipe.\n");
            free_pipeline(pipeline);
            return NULL;
        }

        // Second stage: Parse the individual command segment
        parse_simple_command(pipe_segment, &pipeline->commands[pipeline->num_commands]);
        
        // If the parser found no args (e.g., just "< file"), it's an error
        if (pipeline->commands[pipeline->num_commands].args[0] == NULL) {
             if (pipeline->commands[pipeline->num_commands].inputFile ||
                 pipeline->commands[pipeline->num_commands].outputFile) {
                 fprintf(stderr, "Syntax error: redirection with no command.\n");
             }
             // If the segment was just whitespace, we just ignore it.
             // Otherwise, we increment the command count.
             if (pipeline->commands[pipeline->num_commands].inputFile ||
                 pipeline->commands[pipeline->num_commands].outputFile) {
                 free_pipeline(pipeline);
                 return NULL;
             }
             // else: it was just whitespace between pipes, so we don't inc num_commands
        } else {
            pipeline->num_commands++;
        }
    }
    
    if (rest != NULL) {
         fprintf(stderr, "Exceeded maximum number of pipe segments.\n");
    }

    if (pipeline->num_commands == 0) {
        free(pipeline);
        return NULL;
    }

    return pipeline;
}

/**
 * @brief Frees all memory associated with a Pipeline struct.
 */
void free_pipeline(Pipeline* pipeline) {
    if (pipeline == NULL) {
        return;
    }

    for (int i = 0; i < pipeline->num_commands; i++) {
        SimpleCommand* cmd = &pipeline->commands[i];
        
        // Free all arguments
        for (int j = 0; cmd->args[j] != NULL; j++) {
            free(cmd->args[j]);
        }
        
        // Free I/O filenames
        if (cmd->inputFile) {
            free(cmd->inputFile);
        }
        if (cmd->outputFile) {
            free(cmd->outputFile);
        }
    }
    // Free the pipeline struct itself
    free(pipeline);
}


// -----------------------------------------------------------------
// BUILT-IN COMMAND HANDLER (Unchanged from previous step)
// -----------------------------------------------------------------
int handle_builtin(char** arglist) {
    
    char* cmd = arglist[0];

    // --- Built-in: exit ---
    if (strcmp(cmd, "exit") == 0) {
        exit(0);
    }
    
    // --- Built-in: cd ---
    if (strcmp(cmd, "cd") == 0) {
        if (arglist[1] == NULL) {
            fprintf(stderr, "cd: expected argument to \"cd\"\n");
        } else {
            if (chdir(arglist[1]) != 0) {
                perror("cd");
            }
        }
        return 1; // Command was handled
    }

    // --- Built-in: help ---
    if (strcmp(cmd, "help") == 0) {
        printf("--- My Shell Help ---\n");
        printf("Built-in commands:\n");
        printf("  cd <directory>   - Change the current directory.\n");
        printf("  help             - Display this help message.\n");
        printf("  jobs             - (Not yet implemented).\n");
        printf("  history          - Show command history.\n");
        printf("  exit             - Exit the shell.\n");
        printf("\n");
        printf("Use 'cmd1 | cmd2' for pipes.\n");
        printf("Use 'cmd < file' for input redirection.\n");
        printf("Use 'cmd > file' for output redirection.\n");
        return 1; // Command was handled
    }

    // --- Built-in: jobs ---
    if (strcmp(cmd, "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        return 1; // Command was handled
    }

    // --- Built-in: history ---
    if (strcmp(cmd, "history") == 0) {
        
        HIST_ENTRY **list = history_list();

        if (list == NULL) {
            return 1; // No history, but command was handled
        }

        for (int i = 0; list[i] != NULL; i++) {
            printf("%5d  %s\n", i + history_base, list[i]->line);
        }
        
        return 1; // Command was handled
    }

    // If no built-in commands matched, return 0
    return 0;
}

// *** The old 'tokenize' function is REMOVED from this file. ***