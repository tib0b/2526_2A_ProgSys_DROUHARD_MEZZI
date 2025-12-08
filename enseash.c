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

int main(void) {
    int status = -1;
    char input[MAX_INPUT_SIZE];
    printf("Welcome to ENSEA Shell. \n");
    printf("To exit, type 'exit'.\n");
    printf("enseash %% ");
    fflush(stdout);

    while(fgets(input, MAX_INPUT_SIZE, stdin)) {                // Duplicate input for tokenization
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);            // Start timer

        input[strcspn(input, "\n")] = 0;
        char * args[MAX_ARG_COUNT + 1] ;         // Tokenize input to get command
        int i = 0;
        for (char *p = strtok(input, " "); p != NULL && i < MAX_ARG_COUNT; p = strtok(NULL, " ")) {
            args[i++] = p;                                  // Extract arguments
        }
        if (strtok(NULL, " ") != NULL) {printf("Warning: Maximum argument count exceeded. Extra arguments ignored.\n");}

        if (strcmp(args[0], "exit") == 0) {
            printf("Exiting...\n");  
            exit(EXIT_SUCCESS);                             // Exit shell with code 0
        }
        else if (strcmp(args[0], "fortune") == 0) {
            args[i] = NULL;                                  // Null-terminate argument list for execv
            int ret = fork();                               // Create a child process
            if (ret == 0) {
                execv("/bin/fortune", args);      // Execute fortune command
                exit(0);                                    // Exit child process, if not done by execl
            }
            wait(&status);                                  // Wait for child process to conclude 
        }
        else if (strcmp(args[0], "\n") != 0) {
            printf("Unknown command : %s \n", input);
        }   

        clock_gettime(CLOCK_MONOTONIC_RAW, &end);
        uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000; // Calculate elapsed time in milliseconds
        if (status != -1) {
            printf("enseash [exit:%d|%ld ms] %% ", status, delta_us); // Display exit status and time taken
            status = -1;                                    // Reset status after displaying it
        }
        else {printf("enseash %% ");}
        fflush(stdout);
    }
    printf("\nExiting...\n");
    exit(EXIT_SUCCESS);                                   // Exit shell with code 0, when ^D is pressed
}