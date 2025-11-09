 #ifndef SHELL_H
#define SHELL_H

// --- Includes ---
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
// *** ADD READLINE HEADERS ***
#include <readline/readline.h>
#include <readline/history.h>

// --- Constants ---
#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "shell> "

// *** The HISTORY_SIZE constant is no longer needed. ***
// *** The 'extern' history variables are no longer needed. ***

// --- Function Prototypes ---

// *** REMOVED read_cmd prototype ***
char** tokenize(char* cmdline);
int    handle_builtin(char** arglist);

// Defined in execute.c
int    execute(char* arglist[]);

// *** REMOVED add_to_history and handle_bang_exec prototypes ***

#endif // SHELL_H