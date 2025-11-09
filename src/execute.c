 #include "shell.h"

/**
 * @brief Handles I/O redirection for a single command.
 * This function is called from *within the child process* before execvp.
 */
static void setup_redirection(SimpleCommand* cmd) {
    // Handle Input Redirection (<)
    if (cmd->inputFile) {
        int fd_in = open(cmd->inputFile, O_RDONLY);
        if (fd_in == -1) {
            perror("open (input)");
            exit(1); // Exit child process on failure
        }
        // Redirect standard input to the file
        if (dup2(fd_in, STDIN_FILENO) == -1) {
            perror("dup2 (input)");
            exit(1);
        }
        close(fd_in); // We're done with the original file descriptor
    }

    // Handle Output Redirection (>)
    if (cmd->outputFile) {
        // Open the file with:
        // O_WRONLY: Write-only
        // O_CREAT: Create if it doesn't exist
        // O_TRUNC: Truncate (clear) if it does exist
        // 0644: File permissions (rw-r--r--)
        int fd_out = open(cmd->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd_out == -1) {
            perror("open (output)");
            exit(1);
        }
        // Redirect standard output to the file
        if (dup2(fd_out, STDOUT_FILENO) == -1) {
            perror("dup2 (output)");
            exit(1);
        }
        close(fd_out); // Done with the original file descriptor
    }
}

/**
 * @brief Executes a single simple command (no pipes).
 */
static void execute_simple_command(SimpleCommand* cmd) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        // --- Child Process ---
        // 1. Set up I/O redirections
        setup_redirection(cmd);

        // 2. Execute the command
        execvp(cmd->args[0], cmd->args);
        
        // execvp only returns if an error occurred
        perror("execvp");
        exit(1);
    } else {
        // --- Parent Process ---
        // Wait for the child to complete
        waitpid(pid, NULL, 0);
    }
}

/**
 * @brief Executes a full pipeline of one or more commands.
 */
int execute_pipeline(Pipeline* pipeline) {
    int num_cmds = pipeline->num_commands;

    // Base case: A single command
    if (num_cmds == 1) {
        execute_simple_command(&pipeline->commands[0]);
        return 1;
    }

    // --- Pipeline Case (num_cmds > 1) ---

    int pipe_fds[2];     // [0] = read end, [1] = write end
    int in_fd = STDIN_FILENO; // Input for the *next* command
    pid_t pids[num_cmds];

    for (int i = 0; i < num_cmds; i++) {
        SimpleCommand* cmd = &pipeline->commands[i];

        // Create a pipe, *unless* this is the last command
        if (i < num_cmds - 1) {
            if (pipe(pipe_fds) == -1) {
                perror("pipe");
                return 0; // Fatal error
            }
        }

        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            return 0; // Fatal error
        }

        if (pids[i] == 0) {
            // --- Child Process ---

            // 1. Set up STDIN
            // If in_fd is not STDIN, it means it's the read-end of the previous pipe.
            if (in_fd != STDIN_FILENO) {
                dup2(in_fd, STDIN_FILENO);
                close(in_fd); // Child doesn't need this descriptor anymore
            }

            // 2. Set up STDOUT
            // If this is not the last command, pipe its output to the next one.
            if (i < num_cmds - 1) {
                dup2(pipe_fds[1], STDOUT_FILENO);
                // Child *must* close both ends of the new pipe
                close(pipe_fds[0]);
                close(pipe_fds[1]);
            }

            // 3. Handle file redirections (which override pipes)
            setup_redirection(cmd);

            // 4. Execute the command
            execvp(cmd->args[0], cmd->args);
            perror("execvp");
            exit(1); // Exit child on error
        }
        else {
            // --- Parent Process ---

            // 1. Close the previous pipe's read end (if it's not STDIN)
            // The parent is done with it; the child has a copy.
            if (in_fd != STDIN_FILENO) {
                close(in_fd);
            }

            // 2. Close the new pipe's write end
            // The parent doesn't write; the child has a copy.
            if (i < num_cmds - 1) {
                close(pipe_fds[1]);
                // 3. Save the new pipe's read end for the *next* child
                in_fd = pipe_fds[0];
            }
        }
    }

    // --- Parent: Wait for ALL children to finish ---
    for (int i = 0; i < num_cmds; i++) {
        waitpid(pids[i], NULL, 0);
    }

    return 1;
}