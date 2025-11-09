#ifndef SHELL_H
#define SHELL_H

// --- Includes ---
// These are needed for the function prototypes and definitions
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

// --- Constants ---
#define MAX_LEN 512    // Maximum length of a command line
#define MAXARGS 10     // Maximum number of arguments
#define ARGLEN 30      // Maximum length of an argument
#define PROMPT "shell> " // Your shell's prompt

// *** HISTORY CONSTANT ***
#define HISTORY_SIZE 20 // Store at least 20 commands

// --- Global History Variables ---
// These are *defined* in main.c, but declared 'extern' here
// so shell.c can also access them.
extern char* history[HISTORY_SIZE];
extern int history_total_count;    // Total commands ever run (for numbering)
extern int history_current_size; // Number of items currently in the array

// --- Function Prototypes ---

// Defined in shell.c
char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int    handle_builtin(char** arglist);

// Defined in execute.c
int    execute(char* arglist[]);

// Defined in shell.c (for history)
void   add_to_history(char* cmdline);
char* handle_bang_exec(char* cmdline);

#endif // SHELL_H