#define _BSD_SOURCE
#define NUM_ARGS 3

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <time.h>

int copyfile(char *old, char *new)
{
    FILE *fptr1, *fptr2;
    char c;

    // Open one file for reading
    fptr1 = fopen(old, "r");
    if (fptr1 == NULL)
    {
        exit(0);
    }

    // Open another file for writing
    fptr2 = fopen(new, "w");
    if (fptr2 == NULL)
    {
        exit(0);
    }

    // Read contents from file
    c = fgetc(fptr1);
    while (c != EOF)
    {
        fputc(c, fptr2);
        c = fgetc(fptr1);
    }

    fclose(fptr1);
    fclose(fptr2);
    return 0;
}

void shallowCopy(char* old, char* new) {
	// TODO: Create links to all files in old in new.
	DIR *oldDir = opendir(old);
	if (oldDir == NULL) {
		printf("%s\n", "Cannot open current Directory.");
		return 1;
	}
	struct dirent *pDirent;
	while ((pDirent = readdir(oldDir)) != NULL) {
		if(pDirent->d_type == DT_DIR) {
			// do nothing
		} else {
			DIR *newDir = opendir(new);
			struct stat buf;
			stat(pDirent->d_name, &buf);
			if (newDir == NULL) {
				pid_t child_pid = fork();
				if (child_pid == 0) {
					char *args[3];
					for (int i = 0; i < 3; i++) {
						args[i] = (char *)malloc(sizeof(char)*1024);
					}
					args[0] = "mkdir";
					strcpy(args[1], new);
					args[2] = NULL;
					execvp(args[0], args);
					perror("Error encountered while executing the process : ");
				} else {
					wait(NULL);
				}
			}

			pid_t child_pid = fork();
			if (child_pid == 0) {
				char *args[4];
				for (int i = 0; i < 4; i++) {
					args[i] = (char *)malloc(sizeof(char)*1024);
				}
				args[0] = "ln";
				strcpy(args[1], old);
				strcat(args[1], "/");
				strcat(args[1], pDirent->d_name);
				args[2] = new;
				strcat(args[2], "/");
				strcat(args[2], pDirent->d_name);
				args[3] = NULL;
				execvp(args[0], args);
				perror("Error encountered while executing the process : ");
			} else {
				wait(NULL);
			}
		}
	}
}

void deepCopy(char* old, char* new) {
	// TODO: Copy the contents of all files in old to new.
	DIR *oldDir = opendir(old);
	if (oldDir == NULL) {
		printf("%s\n", "Cannot open current Directory.");
		return 1;
	}
	struct dirent *pDirent;
	while ((pDirent = readdir(oldDir)) != NULL) {
		if(pDirent->d_type == DT_DIR) {
			// do nothing
		} else {
			DIR *newDir = opendir(new);
			struct stat buf;
			stat(pDirent->d_name, &buf);
			if (newDir == NULL) {
				pid_t child_pid = fork();
				if (child_pid == 0) {
					char *args[3];
					args[0] = "mkdir";
					args[1] = new;
					args[2] = NULL;
					execvp(args[0], args);
					perror("Error encountered while executing the process : ");
				} else {
					wait(NULL);
				}
			}
			char *args[4];
			for (int i = 0; i < 4; i++) {
				args[i] = (char *)malloc(sizeof(char)*1024);
			}
			args[0] = "ln";
			strcpy(args[1], old);
			strcat(args[1], "/");
			strcat(args[1], pDirent->d_name);
			strcpy(args[2], new);
			strcat(args[2], "/");
			strcat(args[2], pDirent->d_name);
			args[3] = NULL;
			copyfile(args[1], args[2]);
			/*
			pid_t child_pid = fork();
			if (child_pid == 0) {
				char *args[4];
				args[0] = "cp";
				args[1] = old;
				strcat(args[1], "/");
				strcat(args[1], pDirent->d_name);
				args[2] = new;
				strcat(args[2], "/");
				strcat(args[2], pDirent->d_name);
				args[3] = NULL;
				execvp(args[0], args);
				perror("Error encountered while executing the process : ");
			} else {
				wait(NULL);
			}*/
		}
	}
	closedir(oldDir);
	return 0;
}

int main(int argc, char** argv) {

	if (argc < NUM_ARGS + 1) {
		printf("Wrong number of args, expected %d, given %d\n", NUM_ARGS, argc - 1);
		exit(1);
	}
	clock_t start, end;
  double cpu_time_used;
	start = clock();
	shallowCopy(argv[1], argv[2]);
  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("Shallow Copy -> %f\n", cpu_time_used);
	start = clock();
	deepCopy(argv[1], argv[3]);
  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("Hard Copy -> %f\n", cpu_time_used);
}
