#include "shell.h"
#include <ctype.h>

// (Helper function prototypes)
static void parse_simple_command(char* cmd_str, SimpleCommand* cmd);

// -----------------------------------------------------------------
// --- FIX FOR YOUR 'undefined reference' ERROR ---
// This function was promised in shell.h, but the code was missing.
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
// --- NEW FUNCTIONS FOR FEATURE 8 (VARIABLES) ---
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
    
    if (eq_ptr == NULL) return; // Should not happen

    // 1. Get the key
    *eq_ptr = '\0'; // Temporarily split string at '='
    key = strdup(assignment_str);
    
    // 2. Get the value
    value = strdup(eq_ptr + 1); // Get the part *after* '='
    
    // 3. Put the original string back (good practice)
    *eq_ptr = '='; 

    // 4. Check if variable already exists
    for (int i = 0; i < var_count; i++) {
        if (strcmp(var_storage[i].key, key) == 0) {
            // Found it! Update the value.
            free(var_storage[i].value); // Free the OLD value
            var_storage[i].value = value; // Set the NEW value
            free(key); // We don't need the new key copy
            return;
        }
    }

    // 5. Not found. Add as a new variable.
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
            
            // Check if the arg starts with '$'
            if (pipeline->commands[i].args[j][0] == '$') {
                char* key = pipeline->commands[i].args[j] + 1; // Skip '$'
                char* value = get_variable(key);
                
                if (value != NULL) {
                    // Found it! Replace the arg.
                    free(pipeline->commands[i].args[j]); // Free old "$VAR"
                    pipeline->commands[i].args[j] = strdup(value);
                } else {
                    // Not found. Replace with empty string.
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

    if (strcmp(cmd, "help") == 0) {
        printf("--- My Shell Help ---\n");
        printf("  VAR=value   - Assign a variable.\n");
        printf("  echo $VAR   - Use a variable.\n");
        printf("  set         - Show local variables.\n");
        // ... (add other help text) ...
        return 1;
    }

    if (strcmp(cmd, "jobs") == 0) {
        list_jobs(); // This now calls the function above
        return 1;
    }

    // --- NEW: The 'set' command ---
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
// (These are needed for the file to be complete)

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