#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char** argv) {

	if (argc < 2) {
		printf("Incorrect number of args: given %d, expected 1.\n", argc - 1);
		exit(1);
	} else {
		for (int i = 0; i < 2; i++) {
			pid_t child_pid = fork();
			wait(NULL);
			if (child_pid == 0) {
				if (i%2 == 0) {
					printf("%s\n", "This is a child process. Do a call to the execl");
					execl("./rtime.o", "./rtime.o",argv[1], NULL);
					perror("execl failed: ");
				} else {
					printf("%s\n", "This is a child process. Do a call to the execv");
					execv("./rtime.o", argv);
					perror("execv failed: ");
				}
			}
		}
	}

	// NOTE : use g++ -o rec3 rec3.c command on commandline to run this file. I have not made a make file for this assignments because it was not required.
	// TODO: Fill in your code here.

}
