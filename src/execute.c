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


/**
 * @brief Adds a new background job to the global job list.
 */
void add_job(pid_t pid, Pipeline* pipeline) {
    if (job_count >= MAX_JOBS) {
        fprintf(stderr, "Job list full. Cannot add new job.\n");
        return;
    }
    
    job_list[job_count].pid = pid;
    job_list[job_count].is_running = 1;
    
    // --- Build a representative name for the job ---
    // e.g., "ls -l | grep src "
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
 * Now handles background/foreground execution.
 */
int execute_pipeline(Pipeline* pipeline) {
    int num_cmds = pipeline->num_commands;
    
    // --- Refactor: Handle num_cmds == 1 separately ---
    if (num_cmds == 1) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            return 0;
        }
        
        if (pid == 0) {
            // --- Child Process ---
            setup_redirection(&pipeline->commands[0]);
            execvp(pipeline->commands[0].args[0], pipeline->commands[0].args);
            perror("execvp");
            exit(1); // Exit child on execvp failure
        } else {
            // --- Parent Process ---
            // --- TASK 3: Background Execution ---
            if (!pipeline->is_background) {
                // Foreground: Wait for the child to finish
                waitpid(pid, NULL, 0);
            } else {
                // Background: Add to job list and DON'T wait
                add_job(pid, pipeline);
            }
        }
        return 1;
    }
    
    // --- Pipeline Case (num_cmds > 1) ---
    int pipe_fds[2];
    int in_fd = STDIN_FILENO; // Input for the *next* command
    pid_t pids[num_cmds];

    for (int i = 0; i < num_cmds; i++) {
        SimpleCommand* cmd = &pipeline->commands[i];

        if (i < num_cmds - 1) {
            if (pipe(pipe_fds) == -1) {
                perror("pipe");
                return 0;
            }
        }

        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            return 0;
        }

        if (pids[i] == 0) {
            // --- Child Process ---
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
            exit(1);
        } else {
            // --- Parent Process ---
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
    // --- TASK 3: Background Execution ---
    if (!pipeline->is_background) {
        // Foreground: Wait for all processes in the pipe
        for (int i = 0; i < num_cmds; i++) {
            waitpid(pids[i], NULL, 0);
        }
    } else {
        // Background: Add job (using the PID of the *first* command)
        // and DON'T wait.
        add_job(pids[0], pipeline);
    }

    return 1;
}