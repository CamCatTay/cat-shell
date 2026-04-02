// mysh - a simple Unix shell

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

// Additional headers
#include <stdbool.h>
#include <pwd.h>


// What the shell prompts the user
// Made a constant in case I want to change it later
char SHELL_INPUT_PROMPT[6] = "mysh> ";

int main(void) {

    char line[1024];
    char *p_args[64];
    int arg_count;

    while (true) {
        printf("%s", SHELL_INPUT_PROMPT);
        fflush(stdout);

        // Get user input
        if  (fgets(line, sizeof(line), stdin) != NULL) {
            // Strip new line character
            char *p_newline = strchr(line, '\n');
            if (p_newline) *p_newline = '\0';
        }
        else {
            // EOF
            printf("%s", "\n");
            break;
        }

        // Tokenize user input
        arg_count = 0;
        p_args[arg_count] = strtok(line, " ");

        while (p_args[arg_count] != NULL) {
            arg_count++;
            p_args[arg_count] = strtok(NULL, " ");
        }

        // Echo back tokenized user input (Debug)
        /*
        for (int i = 0; i < arg_count; ++i) {
            printf("Token %d: %s\n", i, args[i]);
        }
        */

        // Built-in commands

        // cd
        if (strcmp(p_args[0], "cd") == 0) {
            if (p_args[1] != NULL) {
                int result = chdir(p_args[1]);

                // Debug print statement to see result of chdir
                //printf("%d\n", result);

                if (result == -1)  {
                    perror("cd failed");
                }
            }
            else {
                // Default to user's home directory if no path is provided
                char *p_home = getenv("HOME");
                chdir(p_home);
            }

            // Do not continue to fork
            continue;
        }

        // exit
        if (strcmp(p_args[0], "exit") == 0) {
            break;
        }

        // Output Redirection (Look for >)

        char output_symbol = '>';
        char input_symbol = '<';
        int output_index = -1;
        int input_index = -1;
        char *p_output_filename = NULL;
        char *p_input_filename = NULL;

        // Look for target
        for (int i = 0; i < arg_count; ++i) {
            char *p_arg = p_args[i];
            char first_char = *p_arg;
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
            p_output_filename = p_args[output_index + 1];
            p_args[output_index] = NULL;
        }
        if (input_index > -1) {
            p_input_filename = p_args[input_index + 1];
            p_args[input_index] = NULL;
        }

        // Pipe

        char pipe_symbol = '|';
        int pipe_index = -1; // Midpoint

        // Look for pipe symbol
        for (int i = 0; i < arg_count; ++i) {
            char* p_arg = p_args[i];
            char first_char = *p_arg;
            if (first_char == pipe_symbol) {
                pipe_index = i;
                break;
            }
        }

        // If pipe symbol was found
        if (pipe_index > -1) {
            p_args[pipe_index] = NULL;

            char **left_command = &p_args[0];
            char **right_command = &p_args[pipe_index];

            int pipefd[2];
            pipe(pipefd);

            // Child 1 (Producer)
            pid_t pid1 = fork();
            if (pid1 == 0) {
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO); // Redirect std out to pipe write
                close(pipefd[1]);
                execvp(p_args[0], &p_args[0]);
                perror("execvp failed");
                exit(EXIT_FAILURE);
            }

            // Child 2 (Consumer)
            pid_t pid2 = fork();
            if (pid2 == 0) {
                close(pipefd[1]);
                dup2(pipefd[0], STDIN_FILENO); // Redirect std out to pipe write
                close(pipefd[0]);
                execvp(p_args[pipe_index + 1], &p_args[pipe_index + 1]);
                perror("execvp failed");
                exit(EXIT_FAILURE);
            }

            // Parent
            close(pipefd[0]);
            close(pipefd[1]);
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);

            // Seperate pipe fork logic from general fork logic for simplicity
            continue;
        }

        // Fork

        pid_t pid = fork();

        if (pid == 0) {
            // CHILD
            // args[0] = program name to execute

            // Redirect output to file
            if (output_index > -1) {
                // 0644 sets the mode of the file (6 = Owner: Read + Write)
                int fd = open(p_output_filename, O_WRONLY | O_TRUNC | O_CREAT, 0644);

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
                int fd = open(p_input_filename, O_RDONLY);
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

            execvp(p_args[0], p_args);
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