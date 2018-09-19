#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char** argv) {

	if (argc < 2) {
		printf("Wrong number of args, expected 1, given %d", argc - 1);
		exit(1);
	} else {

		int fd1[2];
		pipe(fd1);
		int num = getInt(argv[1]);
		pid_t childPid = fork();
		wait(NULL);
		if (childPid == 0) {
			write(fd1[1], "input.txt", 10);
			chmod("input.txt", 0700);
			int oFile = open("input.txt", O_CREAT | O_RDWR);
			dup2(oFile, 1);
			execl("./rec3.o", "./rec3.o", argv[1], NULL);
		} else {
			char* buf = malloc(1024*sizeof(char));
			read(fd1[0], buf, 10);
			chmod(buf, 0700);
			int oFile = open(buf, O_RDONLY);
			int arr[2*num];
			for(int i = 0; i < 2*num; i++) {
				read(oFile, buf, 11);
				arr[i] = atoi(buf);
				// printf("%d\n", atoi(buf));
			}
			int status = 1;
			for(int i = 0; i < 2*num-1; i++) {
				status = status & (arr[i] < arr[i+1]);
			}
			if (status) {
				printf("%s\n", "Correct Solution");
			} else {
				printf("%s\n", "Wrong Solution");
			}
		}
	}

	// TODO: Your code goes here.
}

int getInt(char* num) {
	return atoi(num);
}
