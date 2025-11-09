#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>   // For strcmp, strncpy, bzero, strdup
#include <unistd.h>   // For chdir, fork, execvp
#include <sys/wait.h> // For waitpid
#include <errno.h>    // For perror

// -----------------------------------------------------------------
// BUILT-IN COMMAND HANDLER (MODIFIED)
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
        printf("  history          - Show the last 20 commands.\n");
        printf("  !n               - Execute the nth command from history.\n");
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

    // *** TASK 3: IMPLEMENT 'history' BUILT-IN ***
    if (strcmp(cmd, "history") == 0) {
        int start_num = 1;      // The logical number of the first command to print
        int start_index = 0;    // The array index of the first command

        // If history is full and has wrapped around
        if (history_total_count > HISTORY_SIZE) {
            start_num = history_total_count - HISTORY_SIZE;
            start_index = history_total_count % HISTORY_SIZE;
        }
        
        for (int i = 0; i < history_current_size; i++) {
            int current_index = (start_index + i) % HISTORY_SIZE;
            int current_num = start_num + i;
            
            // Print formatted: "  [num]  [command]"
            printf("%5d  %s\n", current_num, history[current_index]);
        }
        return 1; // Command was handled
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

// -----------------------------------------------------------------
// HISTORY HELPER FUNCTIONS (NEW)
// -----------------------------------------------------------------

/**
 * @brief Adds a command to the circular history buffer.
 */
void add_to_history(char* cmdline) {
    // Get the next available index in our circular buffer
    int index = history_total_count % HISTORY_SIZE;
    
    // Free the old command at this slot, if it exists
    if (history[index] != NULL) {
        free(history[index]);
    }
    
    // Copy the new command into the slot
    // strdup combines malloc() and strcpy()
    history[index] = strdup(cmdline);
    
    history_total_count++;
    
    if (history_current_size < HISTORY_SIZE) {
        history_current_size++;
    }
}

/**
 * @brief Handles the !n command re-execution.
 * @return A *new* string (a copy from history) if !n is valid.
 * The *original* cmdline pointer if it's not a '!' command.
 * `NULL` if it's a '!' command but has an error (e.g., out of bounds).
 */
char* handle_bang_exec(char* cmdline) {
    // Not a '!' command, return the original pointer
    if (cmdline[0] != '!') {
        return cmdline;
    }

    // `atoi` converts the string "123" to the integer 123
    // It returns 0 if the string is not a valid number (e.g., "!" or "!abc")
    int n = atoi(&cmdline[1]);

    if (n == 0) {
        fprintf(stderr, "sshell: %s: event not found\n", cmdline);
        return NULL; // Signal error
    }

    // --- Check if 'n' is in the valid range ---
    
    // Find the smallest valid command number
    int min_n = 1;
    if (history_total_count >= HISTORY_SIZE) {
        min_n = history_total_count - HISTORY_SIZE;
    }
    
    // The largest valid command number is the one we *just* ran (or are about to)
    // But since add_to_history hasn't run yet, the max is the *previous* total.
    int max_n = history_total_count;

    if (n < min_n || n > max_n) {
        fprintf(stderr, "sshell: %s: event not found\n", cmdline);
        return NULL; // Signal error
    }

    // --- 'n' is valid, find its index in the array ---

    // Find the array index of the *oldest* command (min_n)
    int start_index = 0;
    if (history_total_count > HISTORY_SIZE) {
        start_index = (history_total_count - HISTORY_SIZE) % HISTORY_SIZE;
    }
    
    // The command 'n' is (n - min_n) steps *after* the oldest command
    int target_index = (start_index + (n - min_n)) % HISTORY_SIZE;

    // Return a *new copy* of the command from history
    return strdup(history[target_index]);
}