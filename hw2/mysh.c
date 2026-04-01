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

        // Output Redirection (Look for >)

        char output_symbol = '>';
        char input_symbol = '<';
        int output_index = -1;
        int input_index = -1;
        char* output_filename;
        char* input_filename;

        // Look for target
        for (int i = 0; i < arg_count; ++i) {
            char* arg = args[i];
            char first_char = *arg;
            if (first_char == output_symbol) {
                output_index = i;
            }
            else if (first_char == input_symbol) {
                input_index = i;
            }
        }

        // Set filename variable and then set args of > and filename to NULL
        // So the command is run properly
        if (output_index > -1 ) {
            output_filename = args[output_index + 1];
            args[output_index] = NULL;
        }
        if (input_index > -1) {
            input_filename = args[input_index + 1];
            args[input_index] = NULL;
        }

        // Fork

        pid_t pid = fork();

        if (pid == 0) {
            // CHILD
            // args[0] = program name to execute

            // Redirect output to file
            if (output_index > -1) {
                // 0644 sets the mode of the file (6 = Owner: Read + Write)
                int fd = open(output_filename, O_WRONLY | O_TRUNC | O_CREAT, 0644);

                if (fd == -1) {
                    // Invalid file descriptor
                    perror("File could not be created");
                    exit(EXIT_FAILURE);
                }
                else {
                    // Valid file descriptor
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }
            }

            // Read input from file
            if (input_index > -1) {
                int fd = open(input_filename, O_RDONLY);
                if (fd == -1) {
                    // Invalid file descriptor
                    perror("File could not be read");
                    exit(EXIT_FAILURE);
                }
                else {
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }
            }

            execvp(args[0], args);
            perror("execvp failed");
            exit(EXIT_FAILURE);
        }
        else if (pid > 0) {
            // PARENT
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