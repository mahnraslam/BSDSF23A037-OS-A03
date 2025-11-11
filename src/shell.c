#include "shell.h"
#include <ctype.h>

// (Helper function prototypes)
static void parse_simple_command(char* cmd_str, SimpleCommand* cmd);

// -----------------------------------------------------------------
// This is the function from v6 that was missing in your build
// -----------------------------------------------------------------
void list_jobs() {
    printf("Current background jobs:\n");
    if (job_count == 0) {
        printf("  No jobs.\n");
        return;
    }
    
    for (int i = 0; i < job_count; i++) {
        printf("  [Job %d] (PID %d) Running: %s\n", 
               i + 1, job_list[i].pid, job_list[i].cmd_name);
    }
}


// -----------------------------------------------------------------
// --- Functions for v8 (Variables) ---
// -----------------------------------------------------------------

/**
 * @brief Helper to find a variable's value by its key.
 */
char* get_variable(char* key) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(var_storage[i].key, key) == 0) {
            return var_storage[i].value;
        }
    }
    return NULL;
}

/**
 * @brief Stores or updates a variable (e.g., "VAR=value").
 */
void handle_assignment(char* assignment_str) {
    char* key;
    char* value;
    char* eq_ptr = strchr(assignment_str, '=');
    
    if (eq_ptr == NULL) return; 

    *eq_ptr = '\0'; 
    key = strdup(assignment_str);
    
    value = strdup(eq_ptr + 1); 
    
    *eq_ptr = '='; 

    for (int i = 0; i < var_count; i++) {
        if (strcmp(var_storage[i].key, key) == 0) {
            free(var_storage[i].value); 
            var_storage[i].value = value; 
            free(key); 
            return;
        }
    }

    if (var_count < MAX_VARS) {
        var_storage[var_count].key = key;
        var_storage[var_count].value = value;
        var_count++;
    } else {
        fprintf(stderr, "Variable storage full.\n");
        free(key);
        free(value);
    }
}


/**
 * @brief Scans all arguments for $VAR and replaces them.
 */
void expand_variables(Pipeline* pipeline) {
    for (int i = 0; i < pipeline->num_commands; i++) {
        for (int j = 0; pipeline->commands[i].args[j] != NULL; j++) {
            
            if (pipeline->commands[i].args[j][0] == '$') {
                char* key = pipeline->commands[i].args[j] + 1; 
                char* value = get_variable(key);
                
                if (value != NULL) {
                    free(pipeline->commands[i].args[j]); 
                    pipeline->commands[i].args[j] = strdup(value);
                } else {
                    free(pipeline->commands[i].args[j]);
                    pipeline->commands[i].args[j] = strdup("");
                }
            }
        }
    }
}

/**
 * @brief Implements the 'set' built-in command.
 */
void list_variables() {
    printf("Current variables:\n");
    if (var_count == 0) {
        printf("  No variables set.\n");
        return;
    }
    for (int i = 0; i < var_count; i++) {
        printf("  %s=%s\n", var_storage[i].key, var_storage[i].value);
    }
}

// -----------------------------------------------------------------
// BUILT-IN COMMAND HANDLER (MODIFIED for v8)
// -----------------------------------------------------------------
int handle_builtin(char** arglist) {
    if (arglist == NULL || arglist[0] == NULL) {
        return 0;
    }
    char* cmd = arglist[0];

    if (strcmp(cmd, "exit") == 0) {
        exit(0);
    }
    
    if (strcmp(cmd, "cd") == 0) {
        if (arglist[1] == NULL) {
            fprintf(stderr, "cd: expected argument to \"cd\"\n");
            return 2; // FAILED
        } else {
            if (chdir(arglist[1]) != 0) {
                perror("cd");
                return 2; // FAILED
            }
            return 1; // SUCCEEDED
        }
    }

    // --- THIS IS THE "FIX" ---
    // The 'set' command has been added to this help message.
    if (strcmp(cmd, "help") == 0) {
        printf("--- My Shell Help ---\n");
        printf("Built-in commands:\n");
        printf("  VAR=value   - Assign a variable.\n");
        printf("  echo $VAR   - Use a variable.\n");
        printf("  set         - Show local variables. (This is the fix!)\n");
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
        printf("  if...then...else...fi - Conditional execution.\n");
        return 1; // SUCCEEDED
    }

    if (strcmp(cmd, "jobs") == 0) {
        list_jobs(); 
        return 1;
    }

    if (strcmp(cmd, "set") == 0) {
        list_variables();
        return 1; // SUCCEEDED
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


// --- UNCHANGED FUNCTIONS from v7 ---
// (These are just here so the file is complete)

Pipeline* parse_cmdline(char* cmdline) {
    Pipeline* pipeline = (Pipeline*)malloc(sizeof(Pipeline));
    pipeline->num_commands = 0;
    pipeline->is_background = 0;

    char* bg_char = strrchr(cmdline, '&');
    if (bg_char != NULL) {
        char* next_char = bg_char + 1;
        while (*next_char != '\0' && isspace((unsigned char)*next_char)) {
            next_char++;
        }
        if (*next_char == '\0') {
            pipeline->is_background = 1;
            *bg_char = '\0';
        }
    }

    char* pipe_segment;
    char* rest = cmdline;
    while ((pipe_segment = strsep(&rest, "|")) != NULL && 
           pipeline->num_commands < MAX_PIPE_SEGS) {
        
        if (*pipe_segment == '\0') {
            fprintf(stderr, "Syntax error: empty command in pipe.\n");
            free_pipeline(pipeline);
            return NULL;
        }

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