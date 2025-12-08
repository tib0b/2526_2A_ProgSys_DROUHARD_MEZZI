#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
    printf("Bienvenue dans le Shell ENSEA. \n");
    printf("Pour quitter, tapez 'exit'.\n");
    printf("enseash %%");
    fflush(stdout);
    sleep(100);
}