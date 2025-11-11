#include "shell.h"

// (setup_redirection is unchanged, include it here)
static void setup_redirection(SimpleCommand* cmd) {
    if (cmd->inputFile) {
        int fd_in = open(cmd->inputFile, O_RDONLY);
        if (fd_in == -1) {
            perror("open (input)");
            exit(1);
        }
        if (dup2(fd_in, STDIN_FILENO) == -1) {
            perror("dup2 (input)");
            exit(1);
        }
        close(fd_in);
    }
    if (cmd->outputFile) {
        int fd_out = open(cmd->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd_out == -1) {
            perror("open (output)");
            exit(1);
        }
        if (dup2(fd_out, STDOUT_FILENO) == -1) {
            perror("dup2 (output)");
            exit(1);
        }
        close(fd_out);
    }
}


// (add_job is unchanged, include it here)
void add_job(pid_t pid, Pipeline* pipeline) {
    if (job_count >= MAX_JOBS) {
        fprintf(stderr, "Job list full. Cannot add new job.\n");
        return;
    }
    
    job_list[job_count].pid = pid;
    job_list[job_count].is_running = 1;
    
    char* job_name = (char*)malloc(MAX_LEN);
    job_name[0] = '\0';
    for (int i = 0; i < pipeline->num_commands; i++) {
        for (int j = 0; pipeline->commands[i].args[j] != NULL; j++) {
            strcat(job_name, pipeline->commands[i].args[j]);
            strcat(job_name, " ");
        }
        if (i < pipeline->num_commands - 1) {
            strcat(job_name, "| ");
        }
    }
    
    job_list[job_count].cmd_name = job_name;
    printf("[Job %d] (PID %d) started: %s\n", 
           job_count + 1, pid, job_list[job_count].cmd_name);
    
    job_count++;
}


/**
 * @brief Executes a full pipeline of one or more commands.
 * --- NEW (v7) ---
 * Now captures and returns the *exit status* of the last command.
 * @return Returns the exit status (0 for success, non-zero for failure).
 */
int execute_pipeline(Pipeline* pipeline) {
    int num_cmds = pipeline->num_commands;
    
    // --- NEW (v7): Variables to hold the process status ---
    int status = 0;      // This holds the "report" from waitpid
    int exit_status = 0; // This holds the final exit code (e.g., 0 or 1)
    
    // --- Case 1: Single Command ---
    if (num_cmds == 1) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            return 1; // Return 1 (failure)
        }
        
        if (pid == 0) {
            // --- Child Process ---
            setup_redirection(&pipeline->commands[0]);
            execvp(pipeline->commands[0].args[0], pipeline->commands[0].args);
            perror("execvp");
            exit(127); // 127 is the standard code for "command not found"
        } else {
            // --- Parent Process ---
            if (!pipeline->is_background) {
                // --- MODIFIED (v7) ---
                // We now pass &status to capture the report
                waitpid(pid, &status, 0);
                
                // --- NEW (v7): Decode the status report ---
                if (WIFEXITED(status)) {
                    // WIFEXITED: "Was it a normal exit?" (not a crash)
                    // WEXITSTATUS: "What was its exit code?"
                    exit_status = WEXITSTATUS(status);
                } else {
                    // The process crashed or was killed. Report failure.
                    exit_status = 1; 
                }
            } else {
                // Background job: add to list, report 0 (success)
                add_job(pid, pipeline);
                exit_status = 0; 
            }
        }
        return exit_status; // Return the final code
    }
    
    // --- Case 2: Pipeline (num_cmds > 1) ---
    int pipe_fds[2];
    int in_fd = STDIN_FILENO;
    pid_t pids[num_cmds];

    for (int i = 0; i < num_cmds; i++) {
        // (Forking and piping logic is unchanged)
        SimpleCommand* cmd = &pipeline->commands[i];
        if (i < num_cmds - 1) {
            if (pipe(pipe_fds) == -1) { perror("pipe"); return 1; }
        }
        pids[i] = fork();
        if (pids[i] == -1) { perror("fork"); return 1; }

        if (pids[i] == 0) {
            // --- Child Process (in pipe) ---
            if (in_fd != STDIN_FILENO) {
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }
            if (i < num_cmds - 1) {
                dup2(pipe_fds[1], STDOUT_FILENO);
                close(pipe_fds[0]);
                close(pipe_fds[1]);
            }
            setup_redirection(cmd);
            execvp(cmd->args[0], cmd->args);
            perror("execvp");
            exit(127);
        } else {
            // --- Parent Process (in pipe) ---
            if (in_fd != STDIN_FILENO) {
                close(in_fd);
            }
            if (i < num_cmds - 1) {
                close(pipe_fds[1]);
                in_fd = pipe_fds[0];
            }
        }
    }

    // --- Parent: Wait for ALL children in the pipeline ---
    if (!pipeline->is_background) {
        for (int i = 0; i < num_cmds; i++) {
            // --- MODIFIED (v7) ---
            // We only care about the status of the *LAST* command
            if (i == num_cmds - 1) {
                waitpid(pids[i], &status, 0); // Get status for last cmd
                if (WIFEXITED(status)) {
                    exit_status = WEXITSTATUS(status);
                } else {
                    exit_status = 1;
                }
            } else {
                waitpid(pids[i], NULL, 0); // Wait for others, but ignore status
            }
        }
    } else {
        add_job(pids[0], pipeline);
        exit_status = 0; // Background pipe launch is "success"
    }

    return exit_status; // Return the final command's status
}