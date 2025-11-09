 #ifndef SHELL_H
#define SHELL_H

// --- Includes ---
// We need these here for the function prototypes (e.g., FILE*)
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

// --- Function Prototypes ---
// These functions are defined in shell.c but used by main.c
char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int    handle_builtin(char** arglist);

// This function is defined in execute.c but used by main.c
int    execute(char* arglist[]);

#endif // SHELL_H