 #include "shell.h"
#include <ctype.h> // for isspace()

// (Helper function prototypes)
static void parse_simple_command(char* cmd_str, SimpleCommand* cmd);
void list_jobs();


/**
 * @brief Parses the entire command line segment into a Pipeline structure.
 * - Detects background operator '&'
 * - Splits the line by pipes '|'
 * - Calls parse_simple_command for each segment
 */
Pipeline* parse_cmdline(char* cmdline) {
    Pipeline* pipeline = (Pipeline*)malloc(sizeof(Pipeline));
    pipeline->num_commands = 0;
    pipeline->is_background = 0; // Default to foreground

    // --- TASK 2: Detect Background Operator (&) ---
    // We check for '&' at the *end* of the command string.
    char* bg_char = strrchr(cmdline, '&');
    if (bg_char != NULL) {
        // Found '&'. Is it at the end?
        char* next_char = bg_char + 1;
        // Skip any trailing whitespace
        while (*next_char != '\0' && isspace((unsigned char)*next_char)) {
            next_char++;
        }
        
        if (*next_char == '\0') {
            // Yes, it's at the end. This is a background command.
            pipeline->is_background = 1;
            // Remove the '&' from the string so it's not parsed as an argument.
            *bg_char = '\0';
        }
    }

    // --- Stage 1: Split by pipes '|' ---
    char* pipe_segment;
    char* rest = cmdline;
    while ((pipe_segment = strsep(&rest, "|")) != NULL && 
           pipeline->num_commands < MAX_PIPE_SEGS) {
        
        if (*pipe_segment == '\0') {
            fprintf(stderr, "Syntax error: empty command in pipe.\n");
            free_pipeline(pipeline);
            return NULL;
        }

        // --- Stage 2: Parse the individual command segment ---
        parse_simple_command(pipe_segment, &pipeline->commands[pipeline->num_commands]);
        
        if (pipeline->commands[pipeline->num_commands].args[0] == NULL) {
             if (pipeline->commands[pipeline->num_commands].inputFile ||
                 pipeline->commands[pipeline->num_commands].outputFile) {
                 fprintf(stderr, "Syntax error: redirection with no command.\n");
                 free_pipeline(pipeline);
                 return NULL;
             }
        } else {
            pipeline->num_commands++;
        }
    }
    
    if (pipeline->num_commands == 0) {
        free(pipeline);
        return NULL;
    }

    return pipeline;
}


/**
 * @brief Helper function to parse a single command segment (e.g., "ls -l > file.txt")
 * (This function is unchanged from the previous step)
 */
static void parse_simple_command(char* cmd_str, SimpleCommand* cmd) {
    int arg_index = 0;
    cmd->inputFile = NULL;
    cmd->outputFile = NULL;
    
    char* token;
    char* rest = cmd_str;

    while ((token = strsep(&rest, " \t\n")) != NULL) {
        if (*token == '\0') {
            continue;
        }

        if (strcmp(token, "<") == 0) {
            token = strsep(&rest, " \t\n");
            if (token == NULL || *token == '\0') {
                fprintf(stderr, "Syntax error: no file for input redirection.\n");
                return; 
            }
            cmd->inputFile = strdup(token);
        }
        else if (strcmp(token, ">") == 0) {
            token = strsep(&rest, " \t\n");
            if (token == NULL || *token == '\0') {
                fprintf(stderr, "Syntax error: no file for output redirection.\n");
                return;
            }
            cmd->outputFile = strdup(token);
        }
        else {
            if (arg_index < MAX_ARGS) {
                cmd->args[arg_index] = strdup(token);
                arg_index++;
            }
        }
    }
    cmd->args[arg_index] = NULL;
}


/**
 * @brief Frees all memory associated with a Pipeline struct.
 * (This function is unchanged)
 */
void free_pipeline(Pipeline* pipeline) {
    if (pipeline == NULL) return;
    for (int i = 0; i < pipeline->num_commands; i++) {
        SimpleCommand* cmd = &pipeline->commands[i];
        for (int j = 0; cmd->args[j] != NULL; j++) free(cmd->args[j]);
        if (cmd->inputFile) free(cmd->inputFile);
        if (cmd->outputFile) free(cmd->outputFile);
    }
    free(pipeline);
}


// -----------------------------------------------------------------
// BUILT-IN COMMAND HANDLER (MODIFIED)
// -----------------------------------------------------------------

/**
 * @brief NEW: Implements the 'jobs' built-in command.
 * Lists all currently running background jobs from the global list.
 */
void list_jobs() {
    printf("Current background jobs:\n");
    if (job_count == 0) {
        printf("  No jobs.\n");
        return;
    }
    
    for (int i = 0; i < job_count; i++) {
        // We could add status here, but "Running" is fine for now
        printf("  [Job %d] (PID %d) Running: %s\n", 
               i + 1, job_list[i].pid, job_list[i].cmd_name);
    }
}


int handle_builtin(char** arglist) {
    char* cmd = arglist[0];

    if (strcmp(cmd, "exit") == 0) {
        exit(0);
    }
    
    if (strcmp(cmd, "cd") == 0) {
        if (arglist[1] == NULL) {
            fprintf(stderr, "cd: expected argument to \"cd\"\n");
        } else {
            if (chdir(arglist[1]) != 0) {
                perror("cd");
            }
        }
        return 1;
    }

    if (strcmp(cmd, "help") == 0) {
        printf("--- My Shell Help ---\n");
        printf("Built-in commands:\n");
        printf("  cd <dir>    - Change the current directory.\n");
        printf("  help        - Display this help message.\n");
        printf("  jobs        - List background jobs.\n");
        printf("  history     - Show command history.\n");
        printf("  exit        - Exit the shell.\n");
        printf("\n");
        printf("Features:\n");
        printf("  cmd1 | cmd2 - Pipe output of cmd1 to cmd2.\n");
        printf("  cmd < file  - Redirect stdin from file.\n");
        printf("  cmd > file  - Redirect stdout to file.\n");
        printf("  cmd &       - Run command in the background.\n");
        printf("  cmd1 ; cmd2 - Run cmd1, then run cmd2.\n");
        return 1;
    }

    // --- TASK 4: Implement 'jobs' built-in ---
    if (strcmp(cmd, "jobs") == 0) {
        list_jobs(); // Call the new function
        return 1;
    }

    if (strcmp(cmd, "history") == 0) {
        HIST_ENTRY **list = history_list();
        if (list) {
            for (int i = 0; list[i] != NULL; i++) {
                printf("%5d  %s\n", i + history_base, list[i]->line);
            }
        }
        return 1;
    }

    return 0; // Not a built-in
}