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

struct timespec start, end;

int main(void) {
    int status = -1;
    char input[MAX_INPUT_SIZE];
    printf("Welcome to ENSEA Shell. \n");
    printf("To exit, type 'exit'.\n");
    {printf("enseash %% ");}
    fflush(stdout);

    while(fgets(input, MAX_INPUT_SIZE, stdin)) {

        clock_gettime(CLOCK_MONOTONIC_RAW, &start);                // Record start time
        if (strcmp(input, "exit\n") == 0) {
            printf("Exiting...\n");  
            exit(EXIT_SUCCESS);                             // Exit shell with code 0
        }
        else if (strcmp(input, "fortune\n") == 0) {
            int ret = fork();                               // Create a child process
            if (ret == 0) {
                execl("/bin/fortune", "fortune", NULL);     // child executes fortune command
                exit(0);                                    // Exit child process, if not done by execl
            }
            wait(&status);                                  // Wait for child process to conclude 
        }
        else if (strcmp(input, "\n") != 0) {
            printf("Unknown command : %s", input);
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