#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/fb.h>
#include <sys/ioctl.h>

int main(int argc, char* argv[]){
    setenv("PATH", "/usr/bin", 1);

    printf("\e[0;33m--- Welcome to Kot ---\n");
    printf("If you encounter any bugs during your session, please report them on the github repository: \nhttps://github.com/kot-org/new-kot\e[0;37m\n");

    char *exe_argv[2] = {"/usr/bin/bash", NULL};
    execvp("/usr/bin/bash", exe_argv);

    perror("init: /usr/bin/bash");

    return EXIT_FAILURE;
}