/*
 *  Leaf_Counter.c
 *  This file contains function definitions that implement the working for the Leaf_Counter executable.
 *  This file uses the functions specified in validatefunctions.h and util.h when dealing with directory traversal and string manipulation.
 *
 *  To check whether the specified directory is eligible to be a leaf directory we first check it is a directory that does not contain any other directory.
 *  Then we check whether the directory contains votes.txt. If both of these conditions are true then implement the algorithm.
 *
 *  Input format:
 *  <candidatename>\n<candidatename>\n.....<candidatename>\n<candidatename>
 *
 *  Output format:
 *  Create a file named as the path.txt which contains the information about votes for all the candidates in form:
 *    <candidate1>:<candidatecount1>,<candidate2>:<candidatecount2>,.......,<candidatek>:<candidatecountk>
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
#include "validatefunctions.h"

// special structure that is used to create a linked list structure for the candidates so that I can get their names from the input file.
struct candidateInformation {
  char * name;
  struct candidateInformation * next;
} candidateNode;

// function that accumulates different candidates in a string array (char * array[]). This helps to keep the different candidate names seperate from the file so that they can be appropriately given credit for a vote.
void accumulateAllCandidates(struct candidateInformation * cands, int *numberOfDistinctCandidates, char *candidate) {
  struct candidateInformation * iter = cands;
  for(int i = 0; i < *numberOfDistinctCandidates; i++) {
    if (strcmp(iter->name, candidate) == 0) {
      return;
    }
    iter = iter->next;
  }
  iter->name = (char *) malloc(MAX_IO_BUFFER_SIZE*sizeof(char));
  iter->name[0] = '\0';
  iter->next = (struct candidateInformation *) malloc(sizeof(struct candidateInformation));
  strcat(iter->name, candidate);
  *numberOfDistinctCandidates += 1;
  return;
}


void freeCandidateLinkedList(struct candidateInformation * cands) {
  if (cands->next == NULL) {
  } else {
    freeCandidateLinkedList(cands->next);
    free(cands->name);
  }
  free(cands);
}


// function that reads the file (votes.txt) at the specified path and returns a string of different candidates seperated using ";".
// This function is also responsible to make sure that the returned candidates string contains only non duplicate candidates.
char* getCandidatesString(char *path, int *status) {
  chdir(path);
  char mode[] = "0777";
  int perBits;
  perBits = strtol(mode, 0, 8);
  if (chmod ("votes.txt",perBits) < 0)  {	//getting the right permissions.
    fprintf(stderr, "%s\n", "chmod command on votes.txt didnt work. Exiting the program.");
    exit(-1);
  }
  FILE *fp = fopen("votes.txt", "r");
  char * buf = (char *) malloc(MAX_IO_BUFFER_SIZE*sizeof(char));
  int size = MAX_IO_BUFFER_SIZE;
  // needs to be a malloc because using linkedlist
  struct candidateInformation * cands = (struct candidateInformation *) malloc(sizeof(struct candidateInformation));
  cands->name = (char *) malloc(MAX_IO_BUFFER_SIZE*sizeof(char));
  cands->name[0] = '\0';
  cands->next = (struct candidateInformation *) malloc(sizeof(struct candidateInformation));
  if (getline(&buf, &size, fp)  != EOF) {
    strcat(cands->name, trimwhitespace(buf));
  } else {
    fprintf(stderr, "%s %s\n", "Input votes.txt file empty in", path);
    exit(0);
  }
  int numberOfDistinctCandidates = 1;
  while(getline(&buf, &size, fp)  != EOF) {
		if (buf[0] == '\n') {
      // fprintf(stderr, "%s\n", "Empty line found!");
			// do nothing - Empty Line
		} else {
			accumulateAllCandidates(cands, &numberOfDistinctCandidates, trimwhitespace(buf));
		}
	}
  // need to be malloc because is being passed on as the return value and will be used by another function afterwards.
  char *distinctCandidates = (char *) malloc(sizeof(char)*numberOfDistinctCandidates*MAX_IO_BUFFER_SIZE);
  *distinctCandidates = '\0';
  struct candidateInformation * iter = cands;
  for(int i = 0; i < numberOfDistinctCandidates; i++) {
    strcat(distinctCandidates, iter->name);
    strcat(distinctCandidates, ";");
    iter = iter->next;
  }
  freeCandidateLinkedList(cands);
  *status = 1;
  trimwhitespace(distinctCandidates);
  distinctCandidates[strlen(distinctCandidates)-1] = '\0';
  return distinctCandidates;
}

// function that returns the correct index of a candidate in the array of accumulated candidate names.
int getIndex(char ** argvp, int numberOfDistinctCandidates, char * candidate) {
  for (int i = 0; i < numberOfDistinctCandidates; i++) {
    if (strcmp(argvp[i], candidate) == 0) {
      return i;
    }
  }
  return -1;
}

// function that is responsible to actually count votes for each candidate in the file and then update them in the array of votes.
void countVotes(FILE *fr, char **argvp, int **votecount, int numberOfDistinctCandidates) {
  char * buf = (char *) malloc(MAX_IO_BUFFER_SIZE*sizeof(char));
  int size = MAX_IO_BUFFER_SIZE;
  while(getline(&buf, &size, fr)  != EOF) {
		if (buf[0] == '\n') {
      // fprintf(stderr, "%s\n", "Empty line found!");
			// do nothing - Commented out line
		} else {
      *votecount[getIndex(argvp, numberOfDistinctCandidates, trimwhitespace(buf))] += 1;
		}
	}
  free(buf);
}

// function that writes a particular candidate name and their corresponding vote count in the file.
void writeCandidateVoteToFile(FILE *fw, char **argvp, int **votecount, int index) {
  fprintf(fw, "%s", argvp[index]);
  fprintf(fw, "%s", ":");
  fprintf(fw,"%d", *votecount[index]);
}

// function responsible to print the particular file name generated to the STDOUT
void printFileNametoTerminal(char *path, char **args, int length) {
  if (path[strlen(path)-1] != '/') {
    strcat(path, "/");
  }
  strcat(path, args[length-1]);
  strcat(path, ".txt");
  printf("%s\n", path);
}

// function responsible to write the counted votes of each candidate to a file.
void writeToFile(char *path, char **argvp, int numberOfDistinctCandidates, int **votecount) {
  char *filename = (char *)malloc(sizeof(char)*MAX_FILE_NAME_SIZE);
  char ** args = NULL;
  int length = makeargv(trimwhitespace(path), "/", &args);
  strcpy(filename, args[length-1]);
  strcat(filename, ".txt");
  FILE *fw = fopen(filename, "w");
  for (int i = 0; i < numberOfDistinctCandidates - 1; i++) {
    writeCandidateVoteToFile(fw, argvp, votecount, i);
    fprintf(fw, "%s", ",");
  }
  writeCandidateVoteToFile(fw, argvp, votecount, numberOfDistinctCandidates-1);
  printFileNametoTerminal(path, args, length);
  free(filename);
  return;
}

// function that uses upper utility functions to accumulate all information from votes.txt and then write the counted votes to the correct file.
void writeVotes(char *path, char *candidates, int *status) {
  char mode[] = "0777";
  int perBits;
  perBits = strtol(mode, 0, 8);
  if (chmod ("votes.txt",perBits) < 0)  {	//getting the right permissions.
    fprintf(stderr, "%s\n", "chmod command on votes.txt didnt work. Exiting the program.");
    exit(-1);
  }
  FILE *fr = fopen("votes.txt", "r");
  char ** argvp = NULL;
  int numberOfDistinctCandidates = makeargv(trimwhitespace(candidates), ";", &argvp);
  int *votecount[numberOfDistinctCandidates];
  for (int i = 0; i < numberOfDistinctCandidates; i++) {
    votecount[i] = (int *) malloc(sizeof(int));
    *votecount[i] = 0;
  }
  countVotes(fr, argvp, votecount, numberOfDistinctCandidates);
  writeToFile(path, argvp, numberOfDistinctCandidates, votecount);
  for (int i = 0; i < numberOfDistinctCandidates; i++) {
    free(votecount[i]);
  }
  *status = 1;
  return;
}

/*
  Function - parseInput
  - Is responsible for calling various helper functions that are responsible to implement the LeafCounter execution.
    The function has a step by step approach (each step corresponds to a function call)
*/
int parseInput(char *path) {
  int status = 0;
  char mode[] = "0777";
  int perBits;
  perBits = strtol(mode, 0, 8);
  if (chmod (path,perBits) < 0)  {	//getting the right permissions.
    if (errno == ENOENT) {
      fprintf(stderr, "%s\n", "The path given does not correspond to any existing directory.");
      exit(-1);
    }
    fprintf(stderr, "%s %s %s\n", "chmod command on directory", path, "didnt work. Exiting the program.");
    exit(-1);
  }
  // then check whether the directory exists or not
  checkDirectoryExists(path, &status);
  if (status) {
    status = 0;
  } else {
    fprintf(stderr, "%s\n", "The path given does not correspond to any existing directory.");
    return 1;
  }
  // first check whether the path is correct - does not point to a file.
  checkPathIsValid(path, &status);
  if (status) {
    status = 0;
  } else {
    fprintf(stderr, "%s\n", "The path given is not correct.");
    return 1;
  }
  // parse the directory to find whether other folders exist or not.
  checkIsLeafNode(path, &status);
  if (status) {
    status = 0;
  } else {
    fprintf(stderr, "%s %s\n", "The input directory is not a Leaf directory (Contains other subdirectories in it). Path :", path);
    return 1;
  }
  // parse the directory to find whether a file named votes.txt exist or not
  checkVotesTXTExists(path, &status);
  if (status) {
    status = 0;
  } else {
    fprintf(stderr, "%s %s\n", "The input directory does not contain any file named - votes.txt, Input directory :", path);
    printf("%s\n", path);
    return 1;
  }
  // call helper function to get a string of all the different candidate names
  char *candidates = getCandidatesString(path, &status);
  if (status) {
    status = 0;
  } else {
    fprintf(stderr, "%s\n", "votes.txt does not contain the correct information about the candidates");
    return 1;
  }
  // call helper function to count votes and actually output a file
  // return the correct status 1 - denoting correct execution.
  writeVotes(path, candidates, &status);
  free(candidates);
  if (status) {
    return 1;
  } else {
    fprintf(stderr, "%s\n", "Not able to make the output file. Some error occured in countvotes.");
    return 0;
  }
}

/*
  Function - main
  - calls the helper parseInput function that executes everything and returns the correct status.
    status determines whether the functions executed properly or not.
*/
int main(int argc, char **argv){
  // check whether the number of arguments is correct.
	if (argc != 2){
		printf("Usage: %s %s\n", argv[0], "relative path (form - /home/dir1/dir2)");
		return -1;
	}
  // now use the helper function to go to the directory and colect the information figuring out whether the directory is actually a leaf node or not.
  // status represents whether some error happened in the function - parseInput. Print appropriate error message.
  int status = parseInput(argv[1]);
  if (status) {
    return 0;
  } else {
    fprintf(stderr, "%s\n", "Error in parseInput. Please try again."); 
    return -1;
  }
}