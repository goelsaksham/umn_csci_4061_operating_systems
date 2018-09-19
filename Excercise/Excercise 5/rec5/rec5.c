#define _BSD_SOURCE

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

void printStats(char* path, int max) {
	DIR *currDir = opendir(path);
	if (currDir == NULL) {
		printf("%s\n", "Cannot open current Directory.");
		return 1;
	}
	int counter = 0;
	struct dirent *pDirent;
	while ((pDirent = readdir(currDir)) != NULL && counter < max) {
		if(pDirent->d_type == DT_DIR) {
			if (strcmp(".",pDirent->d_name) == 0 || strcmp("..",pDirent->d_name) == 0) {
				// do nothing
			} else {
				printf("%s\n", "//////////////////////////////////////////////");
				printf("%s %s\n", "The Directory names is : ", pDirent->d_name);
				printf("%s\n", "//////////////////////////////////////////////");
				char newPath[1024];
				newPath[0] = '\0';
				strcat(newPath, path);
				strcat(newPath, "/");
				strcat(newPath, pDirent->d_name);
				printStats(newPath, max);
				counter += 1;
				// printStats(pDirent->d_name, max);
			}
		} else {
			chdir(path);
			struct stat buf;
			stat(pDirent->d_name, &buf);
			printf("%s\n", "********************************************");
			printf("%s %s\n", "The file names is : ", pDirent->d_name);
			printf("The user id of the owner : %d\n", buf.st_uid);
			printf("The size of the file : %d\n", buf.st_size);
			printf("The time of last access : %d\n", buf.st_atime);
			printf("%s\n", "********************************************");
			counter += 1;
		}
	}
	closedir(currDir);
	return 0;
	// TODO: Put code here.
}

int getSize(char* path) {
	DIR *currDir = opendir(path);
	int size = 0;

	if (currDir == NULL) {
		printf("%s\n", "Cannot open current Directory.");
		return 1;
	}

	struct dirent *pDirent;

	while ((pDirent = readdir(currDir)) != NULL) {
		if(pDirent->d_type == DT_DIR) {
			// do nothing
		} else {
			struct stat *buf;
			stat(pDirent->d_name, buf);
			size += buf->st_size;
		}
	}
	closedir(currDir);
	return size;
}

int main(int argc, char** argv) {

	if (argc < 3) {

		printf("Wrong number of args, expected 2, given %d", argc - 1);
		exit(1);
	}

	printStats(argv[1], atoi(argv[2]));
}
