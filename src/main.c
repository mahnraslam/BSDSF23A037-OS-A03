#include "shell.h"
#include <stdio.h>
#include <stdlib.h>

// --- Global Job List Definition (v6) ---
Job job_list[MAX_JOBS];
int job_count = 0;

// --- NEW: Global Variable Storage Definition (v8) ---
Variable var_storage[MAX_VARS];
int var_count = 0;


// (reap_zombies is unchanged from v7)
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

// (execute_command_block is updated for v8)
void execute_command_block(char* block[], int count) {
    char* line_to_run;
    char* segment;
    Pipeline* pipeline;

    for (int i = 0; i < count; i++) {
        line_to_run = strdup(block[i]);
        char* free_ptr = line_to_run; 

        while ((segment = strsep(&line_to_run, ";")) != NULL) {
            if (*segment == '\0') continue;
            
            pipeline = parse_cmdline(segment);
            if (pipeline == NULL || pipeline->num_commands == 0) {
                if (pipeline) free_pipeline(pipeline);
                continue;
            }

            // --- NEW: Expand variables *before* executing block ---
            expand_variables(pipeline);

            // (Check for built-ins in a block)
            int builtin_status = 0;
            if (pipeline->num_commands == 1 && !pipeline->is_background &&
                pipeline->commands[0].inputFile == NULL && 
                pipeline->commands[0].outputFile == NULL) {
                
                builtin_status = handle_builtin(pipeline->commands[0].args);
            }
            if (builtin_status == 0) {
                execute_pipeline(pipeline);
            }
            
            free_pipeline(pipeline);
        }
        free(free_ptr);
    }
}


int main() {
    char* cmdline;
    Pipeline* pipeline;

    rl_bind_key('\t', rl_complete);

    while (1) {
        reap_zombies();
        
        cmdline = readline(PROMPT);
        if (cmdline == NULL) break;
        if (cmdline[0] == '\0') {
            free(cmdline);
            continue;
        }

        add_history(cmdline);

        // (if-then-else logic is updated for v8)
        if (strncmp(cmdline, "if ", 3) == 0) {
            char* if_command_str = cmdline + 3;
            char* then_block[MAX_LEN];
            char* else_block[MAX_LEN];
            int then_count = 0;
            int else_count = 0;
            int current_block = 0;
            char* block_line;
            
            while ((block_line = readline("if> ")) != NULL) {
                while (isspace(*block_line)) block_line++;
                if (strcmp(block_line, "then") == 0) {
                    current_block = 0;
                } else if (strcmp(block_line, "else") == 0) {
                    current_block = 1;
                } else if (strcmp(block_line, "fi") == 0) {
                    free(block_line);
                    break;
                } else {
                    if (current_block == 0) {
                        then_block[then_count++] = strdup(block_line);
                    } else {
                        else_block[else_count++] = strdup(block_line);
                    }
                }
                free(block_line);
            }
            
            pipeline = parse_cmdline(if_command_str);
            int exit_status = 1;

            if (pipeline == NULL || pipeline->num_commands == 0) {
                 fprintf(stderr, "if: syntax error\n");
                 exit_status = 1;
            } else {
                // --- NEW: Expand variables in the 'if' condition ---
                expand_variables(pipeline); 
                
                int builtin_status = 0;
                if (pipeline->num_commands == 1 && !pipeline->is_background &&
                    pipeline->commands[0].inputFile == NULL && 
                    pipeline->commands[0].outputFile == NULL) {
                     builtin_status = handle_builtin(pipeline->commands[0].args);
                }
                
                if (builtin_status != 0) {
                    exit_status = (builtin_status == 1) ? 0 : 1;
                } else {
                    exit_status = execute_pipeline(pipeline);
                }
                free_pipeline(pipeline);
            }
            
            if (exit_status == 0) {
                execute_command_block(then_block, then_count);
            } else {
                execute_command_block(else_block, else_count);
            }
            
            for (int i = 0; i < then_count; i++) free(then_block[i]);
            for (int i = 0; i < else_count; i++) free(else_block[i]);
            
            free(cmdline);
        }
        else {
            // --- NORMAL COMMAND MODE (Updated for v8) ---
            char* full_cmdline_for_free = cmdline;
            char* command_segment;

            while ((command_segment = strsep(&cmdline, ";")) != NULL) {
                if (*command_segment== '\0') continue;

                pipeline = parse_cmdline(command_segment);
                if (pipeline == NULL || pipeline->num_commands == 0) {
                    if (pipeline) free_pipeline(pipeline);
                    continue;
                }

                // --- NEW (v8): Check for Variable Assignment ---
                if (pipeline->num_commands == 1 &&
                    !pipeline->is_background &&
                    pipeline->commands[0].inputFile == NULL &&
                    pipeline->commands[0].outputFile == NULL &&
                    pipeline->commands[0].args[1] == NULL &&
                    strchr(pipeline->commands[0].args[0], '=') != NULL) 
                {
                    handle_assignment(pipeline->commands[0].args[0]);
                }
                else 
                {
                    // --- NEW (v8): Expand variables ---
                    expand_variables(pipeline);
                    
                    int builtin_status = 0;
                    if (pipeline->num_commands == 1 && !pipeline->is_background &&
                        pipeline->commands[0].inputFile == NULL && 
                        pipeline->commands[0].outputFile == NULL) {
                        
                        builtin_status = handle_builtin(pipeline->commands[0].args);
                    }

                    if (builtin_status == 0) {
                        execute_pipeline(pipeline);
                    }
                }
                free_pipeline(pipeline);
            }
            free(full_cmdline_for_free);
        }
    }

    printf("\nShell exited.\n");
    return 0;
}