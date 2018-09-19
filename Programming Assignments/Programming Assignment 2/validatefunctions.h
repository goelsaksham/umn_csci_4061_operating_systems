/*
 *  validatefunctions.h
 *  This file contains function definitions that specialize in error handling related to the directory traversal.
 *  These helper functions that are used in other scripts like Aggregate_Votes.c and Leaf_Counter.c
 *
 *  Created on: Feb 19, 2018
 *  Author: Saksham Goel, Franklin Hartman
 *
 */
#define _BSD_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include "util.h"

#define MAX_LEN 1024

// function to check whether the path belongs to a directory or some file.
void checkPathIsValid(char *path, int *status) {
  struct stat statbuf;
  if (stat(path, &statbuf) < 0) {
    fprintf(stderr, "%s\n", "stat call failed");
    exit(0);
  }
  if(S_ISDIR(statbuf.st_mode)) {
    // the path is a directory. Hence allowed.
  } else {
    // else the path is not a directory hence not suitable for the program.
    *status = 0;
		return;
  }
  *status = 1;
  return;
}

// function to check whether the path corresponds a valid directory or not.
void checkDirectoryExists(char *path, int *status) {
  DIR *currDir = opendir(path);
	if (currDir == NULL) {
    closedir(currDir);
		*status = 0;
		return;
	}
  *status = 1;
  return;
}

// function to check whether the directory only contains files and no other directory.
// Helps in determining whether the directory could potentially be a leaf node.
void checkIsLeafNode(char *path, int *status) {
  DIR *currDir = opendir(path);
  // currDir would never bee NULL in this case because that has already been checked.
  struct dirent *pDirent;
	while ((pDirent = readdir(currDir)) != NULL) {
    if(pDirent->d_type == DT_DIR) {
      if (strcmp(".",pDirent->d_name) == 0 || strcmp("..",pDirent->d_name) == 0) {
        // do nothing these need not be considered.
      } else {
        int flagCounter = 0;
        while (closedir(currDir) != 0 && flagCounter < 5) {
          // keep on closing the directory.
          fprintf(stderr, "%s\n", "closedir call failed, trying again!");
          flagCounter += 1;
        }
        *status = 0;
        return;
      }
    }
  }
  *status = 1;
  return;
}

// function to check whether the file votes.txt exist in the potential leaf directory or not.
// This function confirms that the directory is indeed a leaf directory.
void checkVotesTXTExists(char *path, int *status) {
  DIR *currDir = opendir(path);
  struct dirent *pDirent;
	while ((pDirent = readdir(currDir)) != NULL) {
    // only check if the current entry is a file.
    if(pDirent->d_type != DT_DIR) {
      chdir(path);
			struct stat buf;
      // load the file details in the struct stat using the stat command.
      if (stat(pDirent->d_name, &buf) < 0) {
        fprintf(stderr, "%s\n", "stat call failed when trying to find votes.txt!");
        exit(0);
      }
      // if votes.txt exist then return succesfully.
			if (strcmp("votes.txt", pDirent->d_name) == 0) {
        *status = 1;
        return;
      }
    }
  }
  *status = 0;
  return;
}