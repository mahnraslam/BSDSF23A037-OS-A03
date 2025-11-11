 #include "shell.h"
#include <stdio.h>
#include <stdlib.h>

// --- NEW: Global Job List Definition ---
// These are *defined* here (in one .c file)
// and *declared* with 'extern' in shell.h (to be visible everywhere)
Job job_list[MAX_JOBS];
int job_count = 0;


/**
 * @brief Cleans up "zombie" processes.
 * Called before each new prompt to check for any background jobs
 * that have finished.
 */
void reap_zombies() {
    int status;
    pid_t pid;

    // waitpid(-1, ...) -> wait for *any* child process
    // WNOHANG -> "Don't hang" (non-blocking). If no child has exited,
    //            return 0 immediately.
    
    // We loop as long as waitpid() finds a completed child
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // A child has exited! Find it in our job list.
        for (int i = 0; i < job_count; i++) {
            if (job_list[i].pid == pid) {
                // Found it.
                char* status_msg;
                if (WIFEXITED(status)) {
                    status_msg = "Done";
                } else if (WIFSIGNALED(status)) {
                    status_msg = "Terminated";
                } else {
                    status_msg = "Stopped";
                }
                
                // Notify the user
                printf("\n[Job %d] %s: %s\n", i + 1, status_msg, job_list[i].cmd_name);
                
                // Remove from list by swapping with the last job
                free(job_list[i].cmd_name); // Free the stored command name
                job_list[i] = job_list[job_count - 1];
                job_count--;
                
                break; // Found and removed, exit inner loop
            }
        }
    }
}


int main() {
    char* cmdline;
    Pipeline* pipeline;

    rl_bind_key('\t', rl_complete);

    while (1) {
        // --- 1. Reap Zombies ---
        // Before we do anything, check for completed background jobs.
        reap_zombies();

        // --- 2. Read Command ---
        cmdline = readline(PROMPT);
        if (cmdline == NULL) { // Ctrl+D (EOF)
            break;
        }

        // --- 3. Process Command ---
        if (cmdline[0] != '\0') {
            add_history(cmdline);

            char* full_cmdline_for_free = cmdline; // Save original pointer to free
            char* command_segment;

            // --- TASK 1: Command Chaining (;) Loop ---
            // strsep will modify cmdline, which is fine.
            while ((command_segment = strsep(&cmdline, ";")) != NULL) {
                
                if (*command_segment == '\0') {
                    continue; // Skip empty segments (e.g., "cmd1;;cmd2")
                }

                // --- 4. Parse (Pipes, Redirection, Background) ---
                pipeline = parse_cmdline(command_segment);

                if (pipeline == NULL || pipeline->num_commands == 0) {
                    if (pipeline) free_pipeline(pipeline);
                    continue;
                }

                // --- 5. Execute ---
                if (pipeline->num_commands == 1 && !pipeline->is_background) {
                    SimpleCommand* cmd = &pipeline->commands[0];
                    
                    // Only run built-in if it's NOT background and NOT
                    // part of a pipe (which is already true)
                    if (cmd->inputFile == NULL && cmd->outputFile == NULL) {
                        if (handle_builtin(cmd->args) == 1) {
                            // It was a built-in.
                        } else {
                            execute_pipeline(pipeline);
                        }
                    } else {
                         execute_pipeline(pipeline);
                    }
                } else {
                    // It's a pipeline or a background job, execute it.
                    execute_pipeline(pipeline);
                }

                free_pipeline(pipeline);
            
            } // --- End of semicolon loop ---
            
            free(full_cmdline_for_free); // Free the original line
        }
    }

    printf("\nShell exited.\n");
    return 0;
}