#ifndef SHELL_H
#define SHELL_H

// --- Includes ---
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h> // For isspace()

// Readline headers
#include <readline/readline.h>
#include <readline/history.h>

// --- Constants ---
#define MAX_LEN 512
#define MAX_ARGS 20
#define MAX_PIPE_SEGS 10
#define MAX_JOBS 50      // Max background jobs
#define PROMPT "shell> "

// --- Data Structures ---

/**
 * @brief Represents a single simple command (e.g., "ls -l > out.txt")
 */
typedef struct {
    char* args[MAX_ARGS + 1]; // Argument list for execvp
    char* inputFile;          // Filename for input redirection (<)
    char* outputFile;         // Filename for output redirection (>)
} SimpleCommand;

/**
 * @brief Represents a full pipeline of one or more commands.
 */
typedef struct {
    SimpleCommand commands[MAX_PIPE_SEGS]; // Array of simple commands
    int num_commands;                      // Number of commands
    int is_background; // 1 if background (&), 0 if foreground
} Pipeline;

/**
 * @brief Represents a single background job.
 */
typedef struct {
    pid_t pid;         // Process ID
    char* cmd_name;    // Full command string (for 'jobs' command)
    int is_running;  // (We're not using this yet, but good to have)
} Job;

// --- Global Job List (from v6) ---
// Declared here as 'extern' to be visible to all .c files.
// Defined *once* in src/main.c.
extern Job job_list[MAX_JOBS];
extern int job_count;


// --- Function Prototypes ---

// --- from src/shell.c ---
Pipeline* parse_cmdline(char* cmdline);
void      free_pipeline(Pipeline* pipeline);
void      list_jobs();
/**
 * @brief Handles built-in commands
 * @return 0 - Not a built-in
 * 1 - Built-in, Succeeded
 * 2 - Built-in, Failed (e.g., cd to non-existent dir)
 */
int       handle_builtin(char** arglist); // <-- MODIFIED for v7

// --- from src/execute.c ---
void add_job(pid_t pid, Pipeline* pipeline);
/**
 * @brief Executes a full pipeline.
 * @return Returns the exit status of the *last* command in the
 * pipeline, or 0 for a successful background job launch.
 */
int  execute_pipeline(Pipeline* pipeline); // <-- MODIFIED for v7

#endif // SHELL_H