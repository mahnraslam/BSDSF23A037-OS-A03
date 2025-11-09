 #ifndef SHELL_H
#define SHELL_H

// --- Includes ---
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

// --- Constants ---
#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "shell> "

// *** NEW HISTORY CONSTANT ***
#define HISTORY_SIZE 20 // Store at least 20 commands

// --- Global History Variables ---
// (These will be *defined* in main.c)
extern char* history[HISTORY_SIZE];
extern int history_total_count;    // Total commands ever run (for numbering)
extern int history_current_size; // Number of items currently in the array

// --- Function Prototypes ---
char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int    handle_builtin(char** arglist);
int    execute(char* arglist[]);

// *** NEW HISTORY FUNCTIONS ***
// (These will be *defined* in shell.c)
void  add_to_history(char* cmdline);
char* handle_bang_exec(char* cmdline);

#endif // SHELL_H