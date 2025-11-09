 #include <stdio.h>
#include <stdlib.h>
#include <string.h>   // For strcmp, strncpy, bzero
#include <unistd.h>   // For chdir, fork, execvp
#include <sys/wait.h> // For waitpid
#include <errno.h>    // For perror

// --- Constants ---
#define MAX_LEN 512    // Maximum length of a command line
#define MAXARGS 10     // Maximum number of arguments
#define ARGLEN 30      // Maximum length of an argument
#define PROMPT "shell> " // Your shell's prompt

// --- Function Prototypes ---
// We declare all our functions first so 'main' can find them.
char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int handle_builtin(char** arglist);
int execute(char* arglist[]);
 
// -----------------------------------------------------------------
// BUILT-IN COMMAND HANDLER
// -----------------------------------------------------------------
/**
 * handle_builtin(char** arglist)
 *
 * Checks if the user's command is a built-in.
 * If it is, it executes the built-in logic and returns 1.
 * If it's not a built-in, it returns 0.
 */
int handle_builtin(char** arglist) {
    
    // Get the command (the first argument)
    char* cmd = arglist[0];

    // Check for the "exit" command
    if (strcmp(cmd, "exit") == 0) {
        // This gracefully terminates the entire shell program
        exit(0);
    }
    
    // Check for the "cd" (change directory) command
    if (strcmp(cmd, "cd") == 0) {
        // Check if a directory was provided
        if (arglist[1] == NULL) {
            fprintf(stderr, "cd: expected argument to \"cd\"\n");
        } else {
            // Use chdir() to change the directory
            // If it returns -1, an error occurred
            if (chdir(arglist[1]) != 0) {
                perror("cd"); // perror prints "cd: No such file or directory"
            }
        }
        // Return 1 because we handled the command
        return 1;
    }

    // Check for the "help" command
    if (strcmp(cmd, "help") == 0) {
        printf("--- My Shell Help ---\n");
        printf("Built-in commands:\n");
        printf("  cd <directory>   - Change the current directory.\n");
        printf("  help             - Display this help message.\n");
        printf("  jobs             - (Not yet implemented).\n");
        printf("  exit             - Exit the shell.\n");
        printf("\n");
        printf("Any other command will be executed externally.\n");
        // Return 1 because we handled the command
        return 1;
    }

    // Check for the "jobs" command
    if (strcmp(cmd, "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        // Return 1 because we handled the command
        return 1;
    }

    // If no built-in commands matched, return 0
    return 0;
}

 
// -----------------------------------------------------------------
// COMMAND LINE READER
// -----------------------------------------------------------------
char* read_cmd(char* prompt, FILE* fp) {
    printf("%s", prompt);
    char* cmdline = (char*) malloc(sizeof(char) * MAX_LEN);
    int c, pos = 0;

    while ((c = getc(fp)) != EOF) {
        if (c == '\n') break;
        cmdline[pos++] = c;
    }

    if (c == EOF && pos == 0) {
        free(cmdline);
        return NULL; // Handle Ctrl+D (End-of-File)
    }
    
    cmdline[pos] = '\0';
    return cmdline;
}


// -----------------------------------------------------------------
// COMMAND TOKENIZER
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

    if (argnum == 0) { // No arguments were parsed (e.g., only whitespace)
        // Free all the allocated memory if no tokens were found
        for(int i = 0; i < MAXARGS + 1; i++) free(arglist[i]);
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL; // Null-terminate the argument list
    return arglist;
}