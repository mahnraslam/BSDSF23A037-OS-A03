#ifndef SHELL_H
#define SHELL_H

// --- Includes ---
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h> // For open() flags

// Readline headers
#include <readline/readline.h>
#include <readline/history.h>

// --- Constants ---
#define MAX_LEN 512
#define MAX_ARGS 20      // Max args per simple command
#define MAX_PIPE_SEGS 10 // Max commands in a single pipeline
#define PROMPT "shell> "

// --- Data Structures ---

/**
 * @brief Represents a single simple command (e.g., "ls -l > out.txt")
 */
typedef struct {
    char* args[MAX_ARGS + 1]; // Argument list for execvp (e.g., {"ls", "-l", NULL})
    char* inputFile;          // Filename for input redirection (<)
    char* outputFile;         // Filename for output redirection (>)
} SimpleCommand;

/**
 * @brief Represents a full pipeline of one or more commands.
 */
typedef struct {
    SimpleCommand commands[MAX_PIPE_SEGS]; // Array of simple commands
    int num_commands;                      // Number of commands in the pipeline
} Pipeline;

// --- Function Prototypes ---

// --- from shell.c ---
Pipeline* parse_cmdline(char* cmdline);
void      free_pipeline(Pipeline* pipeline);
int       handle_builtin(char** arglist);

// --- from execute.c ---
int execute_pipeline(Pipeline* pipeline);

#endif // SHELL_H