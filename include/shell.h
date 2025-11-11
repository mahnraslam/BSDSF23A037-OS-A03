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
#include <ctype.h>

// Readline headers
#include <readline/readline.h>
#include <readline/history.h>

// --- Constants ---
#define MAX_LEN 512
#define MAX_ARGS 20
#define MAX_PIPE_SEGS 10
#define MAX_JOBS 50
#define MAX_VARS 100     // --- NEW ---
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
    int is_background;
} Pipeline;

typedef struct {
    pid_t pid;
    char* cmd_name;
    int is_running;
} Job;

// --- NEW: Struct for variables (e.g., "MESSAGE=Hello") ---
typedef struct {
    char* key;
    char* value;
} Variable;


// --- Extern Globals (Visible to all files) ---
extern Job job_list[MAX_JOBS];
extern int job_count;
extern Variable var_storage[MAX_VARS]; // --- NEW ---
extern int var_count;                 // --- NEW ---


// --- Function Prototypes ---

// --- from shell.c ---
Pipeline* parse_cmdline(char* cmdline);
void      free_pipeline(Pipeline* pipeline);
int       handle_builtin(char** arglist);
void      list_jobs(); // The missing promise!

// --- NEW: Prototypes for variable handling ---
// (This section fixes your 'implicit declaration' error)
void handle_assignment(char* assignment_str);
void expand_variables(Pipeline* pipeline);
void list_variables();


// --- from execute.c ---
int  execute_pipeline(Pipeline* pipeline);
void add_job(pid_t pid, Pipeline* pipeline);

#endif // SHELL_H