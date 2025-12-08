#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <wait.h>

int main(void) {

    char input[20];
    printf("Welcome to ENSEA Shell. \n");
    printf("To exit, type 'exit'.\n");
    while(1) {
        printf("enseash %% ");
        fflush(stdout);
        fgets(input, 20, stdin);
        if (strcmp(input, "fortune\n") == 0) {
            int ret = fork();                               // Create a child process
            if (ret == 0) {
                execl("/bin/fortune", "fortune", NULL);     // child executes fortune command
                exit(0);                                    // Exit child process, if not done by execl
            }   
            int status;
            wait(&status);                                  // Wait for child process to conclude 
        }
        else {
            printf("Unknown command : %s", input);
        }
    }
}

