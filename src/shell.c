#include "shell.h"
// (All necessary headers, including <readline/history.h>,
// are included via "shell.h")

// -----------------------------------------------------------------
// BUILT-IN COMMAND HANDLER (MODIFIED)
// -----------------------------------------------------------------
int handle_builtin(char** arglist) {
    
    char* cmd = arglist[0];

    // --- Built-in: exit ---
    if (strcmp(cmd, "exit") == 0) {
        exit(0);
    }
    
    // --- Built-in: cd ---
    if (strcmp(cmd, "cd") == 0) {
        if (arglist[1] == NULL) {
            fprintf(stderr, "cd: expected argument to \"cd\"\n");
        } else {
            if (chdir(arglist[1]) != 0) {
                perror("cd");
            }
        }
        return 1;
    }

    // --- Built-in: help ---
    if (strcmp(cmd, "help") == 0) {
        printf("--- My Shell Help ---\n");
        printf("Built-in commands:\n");
        printf("  cd <directory>   - Change the current directory.\n");
        printf("  help             - Display this help message.\n");
        printf("  jobs             - (Not yet implemented).\n");
        printf("  history          - Show command history.\n");
        printf("  exit             - Exit the shell.\n");
        printf("\n");
        printf("Use Up/Down arrows to navigate history.\n");
        printf("Use TAB for command/file completion.\n");
        return 1;
    }

    // --- Built-in: jobs ---
    if (strcmp(cmd, "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        return 1;
    }

    // *** TASK 3: RE-IMPLEMENT 'history' BUILT-IN ***
    if (strcmp(cmd, "history") == 0) {
        
        // Get the list of history entries from Readline
        HIST_ENTRY **list = history_list();

        if (list == NULL) {
            return 1; // No history, but command was handled
        }

        // Loop through the history list
        // 'history_base' is a global int from Readline (usually 1)
        for (int i = 0; list[i] != NULL; i++) {
            // Print the history number and the command
            printf("%5d  %s\n", i + history_base, list[i]->line);
        }
        
        return 1; // Command was handled
    }

    // If no built-in commands matched, return 0
    return 0;
}

 
// *** The read_cmd(...) function is REMOVED from this file. ***


// -----------------------------------------------------------------
// COMMAND TOKENIZER (This function is unchanged)
// -----------------------------------------------------------------
char** tokenize(char* cmdline) {
    // Edge case: empty command line
    if (cmdline == NULL || cmdline[0] == '\0' || cmdline[0] == '\n') {
        return NULL;
    }

    // Allocate memory for the array of strings
    char** arglist = (char**)malloc(sizeof(char*) * (MAXARGS + 1));
    for (int i = 0; i < MAXARGS + 1; i++) {
        arglist[i] = (char*)malloc(sizeof(char) * ARGLEN);
        bzero(arglist[i], ARGLEN);
    }

    char* cp = cmdline; // Pointer to the command line string
    char* start;
    int len;
    int argnum = 0;

    // Loop through the command line string
    while (*cp != '\0' && argnum < MAXARGS) {
        while (*cp == ' ' || *cp == '\t') cp++; // Skip leading whitespace
        
        if (*cp == '\0') break; // Line was only whitespace

        start = cp; // Mark the start of the token
        len = 1;
        // Find the end of the token
        while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t')) {
            len++;
        }
        strncpy(arglist[argnum], start, len); // Copy the token
        arglist[argnum][len] = '\0';          // Null-terminate the token
        argnum++;                             // Move to the next argument
    }

    if (argnum == 0) { // No arguments were parsed
        for(int i = 0; i < MAXARGS + 1; i++) free(arglist[i]);
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL; // Null-terminate the argument list
    return arglist;
}

// *** The add_to_history(...) function is REMOVED from this file. ***

// *** The handle_bang_exec(...) function is REMOVED from this file. ***