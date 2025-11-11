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

typedef struct {
    char* args[MAX_ARGS + 1];
    char* inputFile;
    char* outputFile;
} SimpleCommand;

typedef struct {
    SimpleCommand commands[MAX_PIPE_SEGS];
    int num_commands;
    int is_background; // NEW: 1 if background (&), 0 if foreground
} Pipeline;

// NEW: Struct to track a background job
typedef struct {
    pid_t pid;         // Process ID
    char* cmd_name;    // Full command string
    int is_running;  // 1 if running, 0 if done (ready to be removed)
} Job;

// NEW: Global job list (defined in main.c, declared here)
extern Job job_list[MAX_JOBS];
extern int job_count;


// --- Function Prototypes ---

// --- from shell.c ---
Pipeline* parse_cmdline(char* cmdline);
void      free_pipeline(Pipeline* pipeline);
int       handle_builtin(char** arglist);
void      list_jobs(); // NEW: For the 'jobs' built-in

// --- from execute.c ---
int  execute_pipeline(Pipeline* pipeline);
void add_job(pid_t pid, Pipeline* pipeline); // NEW: To add a bg job

#endif // SHELL_H