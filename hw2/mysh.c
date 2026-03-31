// mysh - a simple Unix shell

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

// Additional headers
#include <stdbool.h>
#include <pwd.h>


// What the shell prompts the user
// Made a constant in case I want to change it later
char SHELL_INPUT_PROMPT[6] = "mysh> ";

int main(void) {

    char line[1024];
    char *args[64];
    int arg_count;

    while (true) {
        printf("%s", SHELL_INPUT_PROMPT);
        fflush(stdout);

        // Get user input
        if  (fgets(line, sizeof(line), stdin) != NULL) {
            // Strip new line character
            char *newline = strchr(line, '\n');
            if (newline) *newline = '\0';
        }
        else {
            // EOF
            printf("%s", "\n");
            break;
        }

        // Tokenize user input
        arg_count = 0;
        args[arg_count] = strtok(line, " ");

        while (args[arg_count] != NULL) {
            arg_count++;
            args[arg_count] = strtok(NULL, " ");
        }

        // Echo back tokenized user input (Debug)
        /*
        for (int i = 0; i < arg_count; ++i) {
            printf("Token %d: %s\n", i, args[i]);
        }
        */

        // Built-in commands

        // cd
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] != NULL) {
                int result = chdir(args[1]);

                // Debug print statement to see result of chdir
                //printf("%d\n", result);

                if (result == -1)  {
                    perror("cd failed");
                }
            }
            else {
                // Default to user's home directory if no path is provided
                char *home = getenv("HOME");
                chdir(home);
            }

            // Do not continue to fork
            continue;
        }

        // exit
        if (strcmp(args[0], "exit") == 0) {
            break;
        }

        // Fork

        // -1 failed; 0 child; >0 child pid
        pid_t pid = fork();

        if (pid == 0) {
            // args[0] = program name to execute
            execvp(args[0], args);
            perror("execvp failed");
            exit(EXIT_FAILURE);
        }
        else if (pid > 0) {
            // Wait for the child
            waitpid(pid, NULL, 0);
        }
        else if (pid < 0) {
            perror("fork failed");
        }
        else {
            perror("something went terribly wrong");
        }

    }

    return EXIT_SUCCESS;
}