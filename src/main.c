#include "shell.h"
#include <stdio.h>
#include <stdlib.h>

// (Global job list and reap_zombies are UNCHANGED from v6)
Job job_list[MAX_JOBS];
int job_count = 0;

void reap_zombies() {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < job_count; i++) {
            if (job_list[i].pid == pid) {
                char* status_msg;
                if (WIFEXITED(status)) status_msg = "Done";
                else if (WIFSIGNALED(status)) status_msg = "Terminated";
                else status_msg = "Stopped";
                printf("\n[Job %d] %s: %s\n", i + 1, status_msg, job_list[i].cmd_name);
                free(job_list[i].cmd_name);
                job_list[i] = job_list[job_count - 1];
                job_count--;
                break;
            }
        }
    }
}


/**
 * @brief --- NEW (v7) ---
 * Helper function to execute a block of commands (then/else).
 * This re-uses our existing semicolon parser!
 */
void execute_command_block(char* block[], int count) {
    char* line_to_run;
    char* segment;
    Pipeline* pipeline;

    for (int i = 0; i < count; i++) {
        // We must make a copy, as strsep is destructive
        line_to_run = strdup(block[i]);
        char* free_ptr = line_to_run; // Save original pointer to free

        // --- We re-use our *existing* semicolon parser! ---
        while ((segment = strsep(&line_to_run, ";")) != NULL) {
            if (*segment == '\0') continue;
            
            pipeline = parse_cmdline(segment);
            if (pipeline == NULL || pipeline->num_commands == 0) {
                if (pipeline) free_pipeline(pipeline);
                continue;
            }

            // We now check for built-ins first
            int builtin_status = 0;
            if (pipeline->num_commands == 1 && !pipeline->is_background &&
                pipeline->commands[0].inputFile == NULL && 
                pipeline->commands[0].outputFile == NULL) {
                
                builtin_status = handle_builtin(pipeline->commands[0].args);
            }
            
            if (builtin_status == 0) {
                // Not a built-in, execute it
                // We don't care about the exit status *inside*
                // a 'then' or 'else' block, so we don't store it.
                execute_pipeline(pipeline);
            }
            
            free_pipeline(pipeline);
        }
        free(free_ptr); // Free the strdup'd line
    }
}


int main() {
    char* cmdline;
    Pipeline* pipeline;

    rl_bind_key('\t', rl_complete);

    while (1) {
        // --- 1. Reap Zombies (Unchanged) ---
        reap_zombies();

        // --- 2. Read Command (Unchanged) ---
        cmdline = readline(PROMPT);
        if (cmdline == NULL) break;
        if (cmdline[0] == '\0') {
            free(cmdline);
            continue;
        }

        add_history(cmdline);

        // --- NEW (v7): The "IF" logic fork ---
        // Does the line start with "if "?
        if (strncmp(cmdline, "if ", 3) == 0) {
            
            // --- YES: ENTER "IF-BLOCK" MODE ---
            
            // Get the command *after* "if "
            char* if_command_str = cmdline + 3;
            
            // --- Arrays to store the command lines ---
            char* then_block[MAX_LEN];
            char* else_block[MAX_LEN];
            int then_count = 0;
            int else_count = 0;
            
            // State variable: 0 = saving to 'then', 1 = saving to 'else'
            int current_block = 0; 
            char* block_line;
            
            // --- STEP A: Read the 'then'/'else'/'fi' blocks ---
            // This loop reads lines until it hits "fi"
            while ((block_line = readline("if> ")) != NULL) {
                
                // Trim leading whitespace (simple version)
                char* clean_line = block_line;
                while (isspace(*clean_line)) clean_line++;
                
                if (strcmp(clean_line, "then") == 0) {
                    current_block = 0; // Start saving in 'then' block
                } 
                else if (strcmp(clean_line, "else") == 0) {
                    current_block = 1; // Switch to saving in 'else' block
                }
                else if (strcmp(clean_line, "fi") == 0) {
                    free(block_line);
                    break; // Found the end, exit "Block-Reading Mode"
                }
                else {
                    // It's a command line, save it
                    if (current_block == 0) {
                        then_block[then_count++] = strdup(block_line);
                    } else {
                        else_block[else_count++] = strdup(block_line);
                    }
                }
                free(block_line); // readline gives us memory we must free
            }
            
            // --- STEP B: Execute the 'if' condition ---
            pipeline = parse_cmdline(if_command_str);
            int exit_status = 1; // Default to failure

            if (pipeline == NULL || pipeline->num_commands == 0) {
                 fprintf(stderr, "if: syntax error\n");
                 exit_status = 1;
            } else {
                // We MUST check for built-ins first
                int builtin_status = handle_builtin(pipeline->commands[0].args);
                
                if (builtin_status != 0) {
                    // It was a built-in. 1 = success(0), 2 = failure(1).
                    exit_status = (builtin_status == 1) ? 0 : 1;
                } else {
                    // Not a built-in, execute it and get the status.
                    exit_status = execute_pipeline(pipeline);
                }
                free_pipeline(pipeline);
            }
            
            // --- STEP C: Decide which block to run ---
            printf("Exit status of 'if' was: %d\n", exit_status);
            if (exit_status == 0) {
                // SUCCESS: Run the 'then' block
                printf("--- Executing 'then' block ---\n");
                execute_command_block(then_block, then_count);
            } else {
                // FAILURE: Run the 'else' block
                printf("--- Executing 'else' block ---\n");
                execute_command_block(else_block, else_count);
            }
            
            // --- STEP D: Clean up stored commands ---
            for (int i = 0; i < then_count; i++) free(then_block[i]);
            for (int i = 0; i < else_count; i++) free(else_block[i]);
            
            free(cmdline); // Free the original "if ..." line
        }
        else {
            // --- NO: NORMAL COMMAND MODE ---
            // This is our *old* loop from v6, for normal commands.
            char* full_cmdline_for_free = cmdline;
            char* command_segment;

            // (This is the semicolon loop from v6)
            while ((command_segment = strsep(&cmdline, ";")) != NULL) {
                if (*command_segment == '\0') continue;

                pipeline = parse_cmdline(command_segment);
                if (pipeline == NULL || pipeline->num_commands == 0) {
                    if (pipeline) free_pipeline(pipeline);
                    continue;
                }

                // --- MODIFIED (v7): Check for built-ins here too ---
                int builtin_status = 0;
                if (pipeline->num_commands == 1 && !pipeline->is_background &&
                    pipeline->commands[0].inputFile == NULL && 
                    pipeline->commands[0].outputFile == NULL) {
                    
                    // handle_builtin returns 0, 1, or 2
                    builtin_status = handle_builtin(pipeline->commands[0].args);
                }

                if (builtin_status == 0) {
                    // It's not a built-in, so execute it
                    execute_pipeline(pipeline);
                }
                // If builtin_status was 1 or 2, the command
                // already ran, so we do nothing.

                free_pipeline(pipeline);
            }
            free(full_cmdline_for_free);
        }
    }

    printf("\nShell exited.\n");
    return 0;
}