#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <wait.h>
#include <curses.h>
#include <sys/time.h>
#include <linux/time.h>

#define MAX_INPUT_SIZE 64
#define MAX_ARG_COUNT 7

struct timespec start, end;

int execvcommand(int i, char *args[], char *outfile, char *command)
{
    int ret = fork();
    if (ret < 0) // fork failed
    {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (ret == 0) // child process
    {
        // child
        if (outfile != NULL)
        {
            int fd = open(outfile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

            dup2(fd, 1); // make stdout go to file
            dup2(fd, 2); // make stderr go to file - you may choose to not do this

            close(fd); // fd no longer needed - the dup'ed handles are sufficient
        }

        args[i] = NULL;
        execv(command, args);
    }

    int status;
    wait(&status);
    return WEXITSTATUS(status);
}

int readcommand(int i, char *args[], char *outfile)
{
    int status = -1;

    if (strcmp(args[0], "exit") == 0)
    {
        printf("Exiting...\n");
        exit(EXIT_SUCCESS); // Exit shell with code 0
    }

    else if (strcmp(args[0], "fortune") == 0)
    {
        status = execvcommand(i, args, outfile, "/bin/fortune");
    }

    else if (strcmp(args[0], "ls") == 0)
    {
        status = execvcommand(i, args, outfile, "/bin/ls");
    }

    else if (strcmp(args[0], "wc") == 0)
    {
        status = execvcommand(i, args, outfile, "/bin/wc");
    }

    else if (strcmp(args[0], "\n") != 0)
    {
        printf("Unknown command : %s \n", args[0]);
    }

    return status;
}

int printnextprompt(int status, uint64_t delta_us)
{
    if (status != -1)
    {
        printf("enseash [exit:%d|%ld ms] %% ", status, delta_us); // Display exit status and time taken
        status = -1;                                              // Reset status after displaying it
    }
    else
    {
        printf("enseash [%ld ms] %% ", delta_us); // If no status was received, display only time taken
    }
    fflush(stdout);
    return status + 1;
}

int addargsfromfile(FILE *file, char *args[], int i)
{
    char line[MAX_INPUT_SIZE];

    while (fgets(line, sizeof(line), file) != NULL && i < MAX_ARG_COUNT)
    {
        line[strcspn(line, "\n")] = 0; // Remove trailing newline character
        args[i++] = strdup(line);      // Add argument from file
    }

    if (strtok(NULL, " ") != NULL)
    {
        printf("Warning: Maximum argument count exceeded. Extra arguments ignored.\n");
    }

    return i;
}

int execute_pipe(char *cmd1[], char *cmd2[], int j, int k)
{
    int fd[2];
    pipe(fd);
    int status1 = 0;
    int status2 = 0;

    if (fork() == 0)
    {
        int status1;
        // First child: left command
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        status1 = readcommand(j, cmd1, NULL);
        //perror("execvp");
        exit(status1);
    }
    if (fork() == 0)
    {
        int status2;
        // Second child: right command
        dup2(fd[0], STDIN_FILENO);
        close(fd[1]);
        close(fd[0]);
        status2 = readcommand(k, cmd2, NULL);
        //perror("execvp");
        exit(status2);
    }
    close(fd[0]);
    close(fd[1]);
    status1 = wait(NULL);
    status2 = wait(NULL);
    return status1 || status2;
}

int formatinput(char *input, char *args[], char **outfile, int status)
{
    int i = 0;                       // Argument counter
    input[strcspn(input, "\n")] = 0; // Remove trailing newline character

    for (char *p = strtok(input, " "); p != NULL && i < MAX_ARG_COUNT; p = strtok(NULL, " "))
    {
        args[i++] = p; // Extract arguments
    }

    if (strtok(NULL, " ") != NULL)
    {
        printf("Warning: Maximum argument count exceeded. Extra arguments ignored.\n");
    }

    if ((i > 1) && (strcmp(args[i - 2], ">") == 0))
    {
        *outfile = args[i - 1];
        i -= 2;
    }

    else if ((i > 1) && (strcmp(args[i - 2], "<") == 0))
    {
        i = addargsfromfile(fopen(args[i - 1], "r"), args, i - 2);
    }

    for (int j = 0; j < i; j++)
    {
        if (strcmp(args[j], "|") == 0)
        {
            char *args1[MAX_ARG_COUNT + 1];
            char *args2[MAX_ARG_COUNT + 1];
            for (int k = 0; k < j; k++)
            {
                args1[k] = args[k];
            }
            for (int k = j + 1; k < i; k++)
            {
                args2[k - j - 1] = args[k];
            }
            status = execute_pipe(args1, args2, j, i - j - 1);  // Execute piped commands, works for two commands only
            return status;
        }
    }
    status = readcommand(i, args, *outfile); // Process command, return status, should be 0 for success, 1 for failure and -1 for no command
    return status;
}

int main(void)
{
    int status = -1;
    char input[MAX_INPUT_SIZE];
    printf("Welcome to ENSEA Shell. \n");
    printf("To exit, type 'exit'.\n");
    printf("enseash %% ");
    fflush(stdout);

    while (fgets(input, MAX_INPUT_SIZE, stdin))
    {
        char *args[MAX_ARG_COUNT + 1]; // Tokenize input to get command

        char *outfile = NULL; // Output file for redirection

        clock_gettime(CLOCK_MONOTONIC_RAW, &start); // Start timer

        status = formatinput(input, args, &outfile, status); // Format input to extract arguments and handle redirection, returns argument count

        clock_gettime(CLOCK_MONOTONIC_RAW, &end);
        uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000; // Calculate elapsed time in milliseconds

        status = printnextprompt(status, delta_us) - 1; // Print prompt with status and time taken, should return 0
    }

    printf("\nExiting...\n");
    exit(EXIT_SUCCESS); // Exit shell with code 0, when ^D is pressed
}
