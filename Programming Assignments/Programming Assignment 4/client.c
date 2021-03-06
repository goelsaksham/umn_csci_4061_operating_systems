/*login: goelx029, hartm433
  date: 04/10/18
  name: saksham goel, franklin hartman
  id: goelx029, hartm433
  Extra credits Yes*/

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
#include <ftw.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include "validatefunctions.h"

// ******************************************************************** //
// Struct Section //
// ******************************************************************** //
struct DAGNode {
  int id;
  char* name;
  char* nameOfFile;
  int numberOfChild;
  int numberOfProcessedChild;
  int* childNodeID;
  int visited;
};

struct DAGListNode {
  struct DAGNode* node;
  struct DAGListNode* next;
};

struct DAGList {
  struct DAGListNode* head;
};

struct candidateNode {
  int id;
  char* name;
  int numOfVotes;
};

struct candidateListNode {
  struct candidateNode* node;
  struct candidateListNode* next;
};

struct candidateList {
  struct candidateListNode* head;
};

struct QNode {
  char* fileName;
  int nodePriority;
  struct QNode* next;
};

struct Queue {
  struct QNode* head;
  int size;
};

struct decryptThreadArgs {
  struct Queue* fileQueue;
  struct DAGList* nodesList;
  char* logTxtFilePath;
  pthread_mutex_t* lock;
  pthread_cond_t* cond;
};
// ******************************************************************** //
// ******************************************************************** //
// ******************************************************************** //


// ******************************************************************** //
// DAG Section //
// ******************************************************************** //
// Returns the number of lines in file
int getNumberOfLinesInFile(FILE* readFile) {
  int numberOfLines = 0;
  int chr;
  while ((chr = fgetc(readFile)) != EOF) { //parses through until end of file
    if ((char) chr == '\n') {
      numberOfLines++;
    }
  } 
  fseek(readFile, 0, SEEK_SET);
  return numberOfLines;
}

// Returns the size of the current line.
int getLineSize(FILE* readFile) {
  int sizeOfLine = 0;
  int chr;
  while (((chr = fgetc(readFile)) != EOF) && ((char) chr) != '\n') { //parses through a single line, one chr at a time
    sizeOfLine++;
  } 
  fseek(readFile, -(sizeOfLine+1), SEEK_CUR);
  return sizeOfLine;
}

// Prints the information about each and every node of the DAG.
void printDAGNodeList(struct DAGList* DAGNodesList) {
  struct DAGListNode* currDAGListNode =  DAGNodesList->head;
  while (currDAGListNode->node != NULL && currDAGListNode->next != NULL) {
    printf("Node ID : %d\n", currDAGListNode->node->id);
    printf("Node Name : %s\n", currDAGListNode->node->name);
    printf("Node FileName : %s\n", currDAGListNode->node->nameOfFile);
    printf("Node Number Of Child : %d\n", currDAGListNode->node->numberOfChild);
    currDAGListNode = currDAGListNode->next;
  }
}

// Returns the ID of the node checking it with respect to the name supplied as the parameter.
// Returns 0 if not found.
int getIDofNode(struct DAGList* nodesList, char* nodeName) {
  struct DAGListNode* currListNode =  nodesList->head;
  while (currListNode->node != NULL && currListNode->next != NULL) {
    if (strcmp(nodeName, currListNode->node->name) == 0) {
      return currListNode->node->id;
    }
    currListNode = currListNode->next;
  }
  return 0;
}

// Returns a node from the DAG based on the supplied ID parameter.
// Returns NULL if not found.
struct DAGNode* getNodeFromNodeID(struct DAGList* nodesList, int nodeID) {
  struct DAGListNode* currListNode =  nodesList->head;
  while (currListNode->node != NULL && currListNode->next != NULL) {
    if (nodeID == currListNode->node->id) {
      return currListNode->node;
    }
    currListNode = currListNode->next;
  }
  return NULL;
}

// Returns a node from the DAG based on the supplied name parameter.
// Returns NULL if not found.
struct DAGNode* getNodeFromNodeName(struct DAGList* nodesList, char* nodeName) {
  struct DAGListNode* currListNode =  nodesList->head;
  while (currListNode->node != NULL && currListNode->next != NULL) {
    if (strcmp(nodeName, currListNode->node->name) == 0) {
      return currListNode->node;
    }
    currListNode = currListNode->next;
  }
  return NULL;
}

// Returns the parent node given the ID of the child node.
// Returns NULL if not found.
struct DAGNode* getParentNodeFromChildNodeID(struct DAGList* nodesList, int childNodeID) {
  struct DAGListNode* currListNode =  nodesList->head;
  while (currListNode->node != NULL && currListNode->next != NULL) {
    if (currListNode->node->numberOfChild != 0) {
      for (int childIndex = 0; childIndex < currListNode->node->numberOfChild; childIndex++) {
        if ((currListNode->node->childNodeID)[childIndex] == childNodeID) {
          return currListNode->node;
        }
      }
    }
    currListNode = currListNode->next;
  }
  return NULL;
}

// Adds a new node to the list of nodes with the supplied name and ID
void addNodeToNodeList(struct DAGList* nodesList, int* nodeID, char* name) {
  struct DAGListNode* currListNode =  nodesList->head;
  while (currListNode->node != NULL && currListNode->next != NULL) {
    currListNode = currListNode->next;
  }
  currListNode->node = (struct DAGNode*) malloc(sizeof(struct DAGNode));
  currListNode->next = (struct DAGListNode*) malloc(sizeof(struct DAGListNode));
  currListNode->node->name = (char *) malloc(sizeof(char)*(strlen(name)+2));
  strcpy(currListNode->node->name, name);
  *nodeID += 1;
  currListNode->node->id = *nodeID;
  currListNode->node->numberOfChild = 0;
  currListNode->node->numberOfProcessedChild = 0;
  currListNode->node->visited = -1;
}

// Parses a line from the file to obtain the name of a node and add it to the node list.
void addNodeFromOneLineOfFile(FILE *readFile, struct DAGList* nodesList, int* nodeID) {
  int sizeOfLine = getLineSize(readFile);
  char* fileLine = (char *) malloc(sizeof(char)*(sizeOfLine+2));
  getline(&fileLine, &sizeOfLine, readFile);
  char **args = NULL;
  int length = makeargv(trimwhitespace(fileLine), ":", &args);
  if (length == 0) {
    fprintf(stderr, "File Line is empty! Line = %s\n", fileLine);
  } else {
    for (int i = 0; i < length; i++) {
      if (getIDofNode(nodesList, trimwhitespace(args[i])) == 0) {
        addNodeToNodeList(nodesList, nodeID, args[i]);
      }
    }
  }
  free(fileLine);
}

// Adds a node as the child of a given parent node in the DAG
void addNeighbors(struct DAGList* nodesList, struct DAGNode* parentNode, char* childNodeName) {
  parentNode->childNodeID[parentNode->numberOfChild] = getIDofNode(nodesList, childNodeName);
  parentNode->numberOfChild += 1;
}

// Parses a line from the file to obtain the name of a parent node and its children and add them to the node list.
void addNodeNeighborsFromALine(FILE *readFile, struct DAGList* nodesList) {
  int sizeOfLine = getLineSize(readFile);
  char* fileLine = (char *) malloc(sizeof(char)*(sizeOfLine+2));
  getline(&fileLine, &sizeOfLine, readFile);
  char **args = NULL;
  int length = makeargv(trimwhitespace(fileLine), ":", &args);
  if (length == 0) {
    fprintf(stderr, "File Line is empty! Line = %s\n", fileLine);
  } else if (length == 1) {
    // do nothing : no neighbors to add
  } else {
    struct DAGNode* parentNode = getNodeFromNodeName(nodesList, trimwhitespace(args[0]));
    if (parentNode != NULL) {
      parentNode->childNodeID = (int*) malloc(sizeof(int)*(length-1));
      for (int i = 1; i < length; i++) {
        addNeighbors(nodesList, parentNode, trimwhitespace(args[i]));
      }
    }
  }
  free(fileLine);
}

// Funtion that constructs the whole DAG
// Returns NULL upon encountering an error.
struct DAGList* constructDAG(char* inputFile) {
  getPermissionsForFile(inputFile);
  if (access(inputFile, F_OK) != -1) {
    FILE *readFile = fopen(inputFile, "r");
    if (readFile == NULL) {
      perror("Error when opening the readfile");
      return NULL;
    }
    int* nodeID = (int *) malloc(sizeof(int));
    *nodeID = 0;
    int numberOfLines = getNumberOfLinesInFile(readFile);
    struct DAGList* nodesList = (struct DAGList *) malloc(sizeof(struct DAGList));
    nodesList->head = (struct DAGListNode*) malloc(sizeof(struct DAGListNode));
    for (int lineNumber = 0; lineNumber < numberOfLines; lineNumber++) {
      addNodeFromOneLineOfFile(readFile, nodesList, nodeID);
    }
    fseek(readFile, 0, SEEK_SET);
    for (int lineNumber = 0; lineNumber < numberOfLines; lineNumber++) {
      addNodeNeighborsFromALine(readFile, nodesList);
    }
    if (fclose(readFile) != 0) {
      perror("Error when closing the reading file!");
    }
    free(nodeID);
    return nodesList;
  } else {
    perror("Input file specified for decrypting does not exist!");
    return NULL;
  } 
}
// ******************************************************************** //
// ******************************************************************** //
// ******************************************************************** //


// ******************************************************************** //
// Decryption Section //
// ******************************************************************** //

// Returns a decrypted variant of a supplied character by obtaining the ASCII value and casting that as a char
// ASCII value of A - 65, Z - 90, a - 97, z - 122
char decrypt(char encryptedChar) {
  unsigned int ASCIIValue = (unsigned int) encryptedChar;
  if (ASCIIValue > 64 && ASCIIValue < 91) { // Upper Case
    unsigned int decryptedASCIIValue = ((ASCIIValue - 65 + 2) % 26) + 65;
    char decryptedChar = (char) decryptedASCIIValue;
    return decryptedChar;
  } else if (ASCIIValue > 96 && ASCIIValue < 123) { // Lower Case
    unsigned int decryptedASCIIValue = ((ASCIIValue - 97 + 2) % 26) + 97;
    char decryptedChar = (char) decryptedASCIIValue;
    return decryptedChar;
  } else {
    return encryptedChar;
  }
}

// Decrypts an entire text file by calling decrypt on each individual character
void decryptFile(char* inputFile, char* outputFile) {
  getPermissionsForFile(inputFile);
  if (access(inputFile, F_OK) != -1) {
    FILE *readFile = fopen(inputFile, "r");
    if (readFile == NULL) {
      perror("Error when opening the encrypted file.");
      return;
    }
    FILE *writeFile = fopen(outputFile, "w");
    if (writeFile == NULL) {
      perror("Error when creating the decrypted file.");
      return;
    }
    int chr;
    while ((chr = fgetc(readFile)) != EOF) {
      fputc((int) decrypt((char) chr), writeFile);
    } 
    if (fclose(writeFile) != 0) {
      perror("Error when closing the writing file!");
      return;
    }
    if (fclose(readFile) != 0) {
      perror("Error when closing the reading file!");
      return;
    }
  } else {
    perror("Input file specified for decrypting does not exist!");
    return;
  } 
}
// ******************************************************************** //
// ******************************************************************** //
// ******************************************************************** //

// ******************************************************************** //
// Creating Output Dir Structure //
// ******************************************************************** //
// Attempts to create a directory given the specified path.
void constructDirectory(char* path) {
  if (mkdir(path, 0777) == 0) { //Directory creation is successful.
    return;
  } else if (errno == EEXIST) {
    // do nothing
  } else {
    fprintf(stderr, "Failed to create a directory : %s\n", path);
  }
}

// Returns a new path with the name of a new directory appended onto it.
char * makeNewPath(char* path, char* name) {
  char* newPath = (char*) malloc(sizeof(char)*(strlen(path) + strlen(name) + 5));
  strcpy(newPath, path);
  strcat(newPath, name);
  strcat(newPath, "/");
  return newPath;
}

// Creates the text file name for a given node.
void declareFileName(struct DAGNode * currNode, char* newPath) {
  currNode->nameOfFile = (char*) malloc(sizeof(char)*(strlen(newPath) + strlen(currNode->name) + 10));
  strcpy(currNode->nameOfFile, newPath);
  strcat(currNode->nameOfFile, currNode->name);
  strcat(currNode->nameOfFile, ".txt");
}

// Marks a node as currently being visited (marking it grey), and visits its children if it has any.
  // Once all of the node's children have finished being visited, DFS finishes visiting this node
  // and it is colored black to indicate this.
void DFSVisit(struct DAGList* nodesList, struct DAGNode * currNode, char* path) {
	currNode->visited = 0; //convert the white node to a grey node
	if (currNode->numberOfChild == 0) {
    char* newPath = makeNewPath(path, currNode->name);
    constructDirectory(newPath);
    declareFileName(currNode, newPath);
    free(newPath);
	} else {
    char* newPath = makeNewPath(path, currNode->name);
    constructDirectory(newPath);
    declareFileName(currNode, newPath);
		for(int i = 0; i < currNode->numberOfChild; i++) {
			if (getNodeFromNodeID(nodesList, *((currNode->childNodeID) + i))->visited == -1) {
				DFSVisit(nodesList, getNodeFromNodeID(nodesList, *((currNode->childNodeID) + i)), newPath);
			}
		}
    free(newPath);
	}
	currNode->visited = 1; //convert the grey node to a black node
}

// Utilizing Depth First Search to traverse the DAG allows us to begin at the leaf
  //nodes and work our way up the tree, aggregating results along the way.
void DFS(struct DAGList* nodesList, char* path) {
  struct DAGListNode* currListNode =  nodesList->head;
  while (currListNode->node != NULL && currListNode->next != NULL) {
    if (currListNode->node->visited == -1) {
      DFSVisit(nodesList, currListNode->node, path);
    }
    currListNode = currListNode->next;
  }
}

// Removes a given directory and its subdirectories based on a supplied path
int removeDirectories(char * path) {
  DIR *currDir = opendir(path);
	if (currDir == NULL) {
		// fprintf(stderr, "%s\n", "Cannot open current Directory.");
		return 1;
	}
	struct dirent *pDirent;
	while ((pDirent = readdir(currDir)) != NULL) {
		if (strcmp(".",pDirent->d_name) == 0 || strcmp("..",pDirent->d_name) == 0) {
			// do nothing
		} else {
      int len = strlen(path) + strlen(pDirent->d_name) + 5;
			char* newPath = (char*) malloc(sizeof(char)*len);
      snprintf(newPath, len, "%s/%s", path, pDirent->d_name);
      struct stat statbuf;
      if (!stat(newPath, &statbuf)) {
        if (S_ISDIR(statbuf.st_mode)) {
          if (removeDirectories(newPath) == 0) {
            rmdir(newPath);
          }
        } else {
          remove(newPath);
        }
      }
     free(newPath); 
		}
	}
	closedir(currDir);
  rmdir(path);
	return 0;
}

// Produces a log file which contains the name of a node, its relevant thread id, and whether the thread is starting or ending.
char* createLogTxt(char* path) {
  char* filePath = (char *) malloc(sizeof(char) * (strlen(path) + 10));
  snprintf(filePath, (strlen(path) + 10), "%s%s", path, "log.txt");
  FILE* f = fopen(filePath, "w");
  fclose(f);
  return filePath;
}
// ******************************************************************** //
// ******************************************************************** //
// ******************************************************************** //

// ******************************************************************** //
// Queue Operations //
// ******************************************************************** //
// Adds a filename to the file queue, which is used to ensure that each thread receives its own file.
void enqueue(struct Queue* fileQueue, char* name, int priority) {
  // fprintf(stderr, "Adding the node %s with priority = %d\n", name, priority);
  if (fileQueue->size <= 0) {
    fileQueue->head = (struct QNode*) malloc(sizeof(struct QNode));
    fileQueue->head->fileName = (char*) malloc(sizeof(char)*(strlen(name)+2));
    strcpy(fileQueue->head->fileName, name);
    fileQueue->head->nodePriority = priority;
    fileQueue->head->next = NULL;
    // fileQueue->head->next->fileName = NULL;
    // fileQueue->head->next->next = NULL;
    fileQueue->size = 1;
  } else {
    struct QNode* previousHead = NULL;
    struct QNode* nodePointer = fileQueue->head;
    for (int i = 0; i < fileQueue->size; i++) {
      if (nodePointer->nodePriority < priority) {
        previousHead = nodePointer;
        nodePointer = nodePointer->next;
      } else if (previousHead == NULL) {
        fileQueue->head = (struct QNode*) malloc(sizeof(struct QNode));
        fileQueue->head->fileName = (char*) malloc(sizeof(char)*(strlen(name)+2));
        strcpy(fileQueue->head->fileName, name);
        fileQueue->head->nodePriority = priority;
        fileQueue->head->next = nodePointer;
        fileQueue->size += 1;
        return;
      } else {
        previousHead->next = (struct QNode*) malloc(sizeof(struct QNode));
        previousHead->next->fileName = (char*) malloc(sizeof(char)*(strlen(name)+2));
        strcpy(previousHead->next->fileName, name);
        previousHead->next->nodePriority = priority;
        previousHead->next->next = nodePointer;
        fileQueue->size += 1;
        return;
      }
    }
    if (nodePointer == NULL) {
      previousHead->next = (struct QNode*) malloc(sizeof(struct QNode));
      previousHead->next->fileName = (char*) malloc(sizeof(char)*(strlen(name)+2));
      strcpy(previousHead->next->fileName, name);
      previousHead->next->nodePriority = priority;
      previousHead->next->next = NULL;
    } else {
      nodePointer->next = (struct QNode*) malloc(sizeof(struct QNode));
      nodePointer->next->fileName = (char*) malloc(sizeof(char)*(strlen(name)+2));
      strcpy(nodePointer->next->fileName, name);
      nodePointer->next->nodePriority = priority;
      nodePointer->next->next = NULL;
    }
    fileQueue->size += 1;
  }
}

// Returns the head of the file queue and removes it from the queue.
char* dequeue(struct Queue* fileQueue) {
  struct QNode* previousHead = fileQueue->head;
  fileQueue->head = fileQueue->head->next;
  fileQueue->size -= 1;
  return previousHead->fileName;
}
// ******************************************************************** //
// ******************************************************************** //
// ******************************************************************** //

// ******************************************************************** //
// Candidate List Stuff //
// ******************************************************************** //
// Prints off information (ID, name, number of votes) for each candidate in the candidate list
void printCandidateListNode(struct candidateList* candsList) {
  struct candidateListNode* currListNode =  candsList->head;
  while (currListNode->node != NULL && currListNode->next != NULL) {
    printf("List Node ID : %d\n", currListNode->node->id);
    printf("List Node Name : %s\n", currListNode->node->name);
    printf("List Number of Votes : %d\n", currListNode->node->numOfVotes);
    currListNode = currListNode->next;
  }
}

// Returns the ID of a candidate node in the candidate list given the name of the candidate
int getIDofCandidateListNode(struct candidateList* nodesList, char* nodeName) {
  struct candidateListNode* currListNode =  nodesList->head;
  while (currListNode != NULL && currListNode->node != NULL && currListNode->next != NULL) {
    if (currListNode->node->name != NULL && strcmp(nodeName, currListNode->node->name) == 0) {
      return currListNode->node->id;
    }
    currListNode = currListNode->next;
  }
  return 0;
}

// Returns a candidate node from the list of candidate nodes given the node ID
struct candidateNode* getCandidateNodeFromNodeID(struct candidateList* nodesList, int nodeID) {
  struct candidateListNode* currListNode =  nodesList->head;
  while (currListNode->node != NULL && currListNode->next != NULL) {
    if (nodeID == currListNode->node->id) {
      return currListNode->node;
    }
    currListNode = currListNode->next;
  }
  return NULL;
}

// Returns a candidate node from the list of candidate nodes given the name of the candidate
struct candidateNode* getCandidateNodeFromNodeName(struct candidateList* nodesList, char* nodeName) {
  struct candidateListNode* currListNode =  nodesList->head;
  while (currListNode->node != NULL && currListNode->next != NULL) {
    if (strcmp(nodeName, currListNode->node->name) == 0) {
      return currListNode->node;
    }
    currListNode = currListNode->next;
  }
  return NULL;
}
// ******************************************************************** //
// ******************************************************************** //
// ******************************************************************** //

// ******************************************************************** //
// Creating Queue from input dir //
// ******************************************************************** //
// Parses throuh the input directory and enqueues every file so that each thread may receive its own file.
void populateQueueFromInputDir(struct Queue* fileQueue, char* input_dir, int* NUM_THREADS, struct DAGList* nodesList) {
  DIR *currDir = opendir(input_dir);
	if (currDir == NULL) {
		fprintf(stderr, "%s:%s\n", "Cannot open current Directory", input_dir);
		return 1;
	}
	struct dirent *pDirent;
	while ((pDirent = readdir(currDir)) != NULL) {
		if (strcmp(".",pDirent->d_name) == 0 || strcmp("..",pDirent->d_name) == 0) {
			// do nothing
		} else {
      int len = strlen(input_dir) + strlen(pDirent->d_name)+10;
			char* newPath = (char*) malloc(sizeof(char)*len);
      snprintf(newPath, len, "%s%s", input_dir, pDirent->d_name);
      struct stat statbuf;
      if (!stat(newPath, &statbuf)) {
        if (S_ISDIR(statbuf.st_mode)) {
          // do nothing
        } else {
          if (getIDofNode(nodesList, pDirent->d_name) != 0 && getNodeFromNodeName(nodesList, pDirent->d_name)->numberOfChild == 0) {
            // Only consider the file names which exist in the DAG structure and are the leaf nodes.
            FILE* readFile = fopen(newPath, "r");
            int numOfLines = getNumberOfLinesInFile(readFile);
            if (numOfLines == 0) {
              // do nothing
            } else if (numOfLines == 1 && getLineSize(readFile) == 0) {
              // do nothing
            } else {
              int priority = INT_MIN;
              char **args = NULL;
              int length = makeargv(pDirent->d_name, "_", &args);
              if (length == 1) {
                // Do nothing
              } else {
                if (strcmp("p", args[length-2]) == 0) {
                  priority = atoi(args[length-1]);
                }
              }
              // If the file is not empty then only consider it
              enqueue(fileQueue, newPath, priority);
              *NUM_THREADS += 1;
            }
            fclose(readFile);
          }
        }
      }
		}
	}
	closedir(currDir);
}

// Writes to the log file, which contains the name of a node, its relevant thread id, and whether the thread is starting or ending.
  // This is updated at appropriate times.
void writeToLogTxtFile(char* logTxtFilePath, unsigned int threadID, char* inputFile, int flag) {
  getPermissionsForFile(logTxtFilePath);
  if (access(logTxtFilePath, F_OK) != -1) {
    FILE *writeFile = fopen(logTxtFilePath, "a");
    if (writeFile == NULL) {
      perror("Error when opening the writing file for log.txt file");
      return;
    }
    if (flag == 0) {
      fprintf(writeFile, "%s:%u:start\n", inputFile, threadID);
    } else {
      fprintf(writeFile, "%s:%u:end\n", inputFile, threadID);
    }
    if (fclose(writeFile) != 0) {
      perror("Error when closing the writing file!");
    }
  } else {
    perror("Input file specified for decrypting does not exist!");
    return;
  } 
}

// Places a single candidate node in the linked list of candidate nodes and assigns it an ID.
void addCandNodeToCandNodeList(struct candidateList* candsList, int* candNodeID, char* candName, int numOfVotes) {
  int candID = getIDofCandidateListNode(candsList, candName);
  if (candID == 0) {
    struct candidateListNode* candsListNode =  candsList->head;
    while (candsListNode->node != NULL && candsListNode->next != NULL) {
      candsListNode = candsListNode->next;
    }
    candsListNode->node = (struct candidateNode*) malloc(sizeof(struct candidateNode));
    candsListNode->next = (struct candidateListNode*) malloc(sizeof(struct candidateListNode));
    candsListNode->node->name = (char *) malloc(sizeof(char)*(strlen(candName)+2));
    strcpy(candsListNode->node->name, candName);
    *candNodeID += 1;
    candsListNode->node->id = *candNodeID;
    candsListNode->node->numOfVotes = numOfVotes;
    candsListNode->next->node = NULL;
    candsListNode->next->next = NULL;
  } else {
    if (getCandidateNodeFromNodeID(candsList, candID) == NULL) {
      fprintf(stderr, "%s\n", "The candidate not found even though the node id is not 0");
    } else {
      getCandidateNodeFromNodeID(candsList, candID)->numOfVotes += numOfVotes;
    }
  }
}

// Parses a line from a leaf file and adds a candidate node to the candidate list
  // This is more trivial than adding a node from an aggregate file, since here all that the file contains is the names of the candidates in order of the votes casted
void addCandNodeFromLeafFile(FILE *readFile, struct candidateList* candsList, int* candNodeID) {
  int sizeOfLine = getLineSize(readFile);
  char* candName = (char *) malloc(sizeof(char)*(sizeOfLine+2));
  getline(&candName, &sizeOfLine, readFile);
  trimwhitespace(candName);
  addCandNodeToCandNodeList(candsList, candNodeID, candName, 1);
  free(candName);
}

// Parses a line from an aggregate file and adds a candidate node to the candidate list
  // This is less trivial than adding a node from a leaf file, since here the file contains the candidate name as well as total num of votes
  // The list filled by this function is dynamic and is changed at each level to make sure that the votes are not counted twice.
void addCandNodeFromAggregateFile(FILE *readFile, struct candidateList* candsList, int* candNodeID) {
  int sizeOfLine = getLineSize(readFile);
  char* candName = (char *) malloc(sizeof(char)*(sizeOfLine+2));
  getline(&candName, &sizeOfLine, readFile);
  char **args = NULL;
  int length = makeargv(trimwhitespace(candName), ":", &args);
  if (length == 0) {
    fprintf(stderr, "Cand is empty! Cand Name = %s\n", candName);
  } else if (length == 2) {
    addCandNodeToCandNodeList(candsList, candNodeID, trimwhitespace(args[0]), atoi(args[1]));
  } else {
    fprintf(stderr, "%s:%s\n", "The aggregate file printed wrong result", candName);
  }
  free(candName);
}

// Parses through every line in a leaf file and individually adds each candidate to the candidate list. This function is specifically responsible to fill the
// candidate list that the thread represents which is the aggregated form of the candidates from leaf file.
void runThreadOnLeafFile(struct candidateList* candsList, struct DAGList* nodesList, struct DAGNode* currNode, int* candNodeID) {
  getPermissionsForFile(currNode->nameOfFile);
  if (access(currNode->nameOfFile, F_OK) != -1) {
    FILE *readFile = fopen(currNode->nameOfFile, "r");
    int numberOfLines = getNumberOfLinesInFile(readFile);
    for (int lineNumber = 0; lineNumber < numberOfLines; lineNumber++) {
      addCandNodeFromLeafFile(readFile, candsList, candNodeID);
    }
    if (fclose(readFile) != 0) {
      perror("Error when closing the reading file!");
    }
  }
}

// Parses through the Candidate List to obtain the name and total number of votes for each candidate
// this is then written to the supplied file in the format - "CandidateName:NumberOfVotes"
void writeCandInformationToFile(FILE* writeFile, struct candidateList* candsList, struct candidateList* argsCandsList) {
  struct candidateListNode* currListNode =  candsList->head;
  if (currListNode == NULL) {
    fprintf(stderr, "%s\n", "Original Candidate Information Node is NULL");
    return;
  }
  while (currListNode != NULL && currListNode->node != NULL && currListNode->next != NULL) {
    if (getIDofCandidateListNode(argsCandsList, currListNode->node->name) == 0) {
      // If the candidate in the candidate list obtained from the leaf file is not present in the list obtained from aggregated file then just write the information.
      fprintf(writeFile, "%s:%d\n", currListNode->node->name, currListNode->node->numOfVotes);
    }
    else {
      struct candidateNode* aggrCandNode = getCandidateNodeFromNodeName(argsCandsList, currListNode->node->name);
      if (aggrCandNode != NULL) {
        // If the candidate in the candidate list obtained from the leaf file is present in the list obtained from aggregated file then add the number of votes and write the new number of votes as the final number.
        fprintf(writeFile, "%s:%d\n", currListNode->node->name, currListNode->node->numOfVotes + aggrCandNode->numOfVotes);
      }
    }
    currListNode = currListNode->next;
  }
  currListNode =  argsCandsList->head;
  if (currListNode == NULL) {
    fprintf(stderr, "%s\n", "AggrCandidate Information Node is NULL");
    return;
  }
  while (currListNode != NULL && currListNode->node != NULL && currListNode->next != NULL) {
    if (getIDofCandidateListNode(candsList, currListNode->node->name) == 0) // If the candidate in the candidate list obtained from the aggregate file is not present in the list obtained from leaf file then write the information else not.
      fprintf(writeFile, "%s:%d\n", currListNode->node->name, currListNode->node->numOfVotes);
    currListNode = currListNode->next;
  }
}

// Similar to writeCandInformationToFile, but only writes the name for the winner following "WINNER:"
void appendWinnerInformation(FILE* writeFile, struct candidateList* candsList) {
  int candListNoOfVotes = 0;
  struct candidateNode* candListNode = NULL;

  struct candidateListNode* currListNode =  candsList->head;
  if (currListNode == NULL) {
    fprintf(stderr, "%s\n", "Original Candidate Information Node is NULL");
    return;
  }
  // Finding the winner candidate.
  while (currListNode != NULL && currListNode->node != NULL && currListNode->next != NULL) {
    if (currListNode->node->numOfVotes > candListNoOfVotes) {
      candListNoOfVotes  = currListNode->node->numOfVotes;
      candListNode = currListNode->node;
    }
    currListNode = currListNode->next;
  }
  // Writing the information about the winner cnadidate.
  fprintf(writeFile, "WINNER:%s\n", candListNode->name);
}

// This function is responsible for the thread to keep on aggregating the votes of the candidates till the thread reaches the top most region (With no parent node). This function is called recursively on the parent nodes so that the appropriate information can be written to the files of the parent nodes directory. This recursive call ends when the current thread reches the top most region. The task done accomplished by this function is as follows:
/*
* 1. Check whether a aggregated file exist in the directory corresponding to the parent node.
* If the aggregated file exist
* `2. Aggregate the information from this file about the candidateList
*  3. Colasces the information from this aggregated candidate list and the original candidate list found using the leaf file
*  4. Write the results to the same aggregated file (vote changes or some candidate is added)
*  5. Call the function recursively with the parent node. if the parent node is the top most node then append the information about the winner candidate.
* If the aggregated file does not exist
*  2. Make the aggregated file.
*  3. Fill the details about the candidates found after parsing the information from the leaf file.
*  4. Call the function recursively with the parent node. if the parent node is the top most node then append the information about the winner candidate.
*/
void runThreadOnAggregateFile(struct candidateList* candsList, struct candidateList* argsCandsList, struct decryptThreadArgs* threadArgs, struct DAGNode* currNode, int* candNodeID, int* argsCandsNodeID) {
  struct DAGNode* parentNode = getParentNodeFromChildNodeID(threadArgs->nodesList, currNode->id);
  if (parentNode != NULL) {
    if (access(parentNode->nameOfFile, F_OK) != -1) {
      getPermissionsForFile(parentNode->nameOfFile);
      int indexFlag = 0;
      if (getParentNodeFromChildNodeID(threadArgs->nodesList, parentNode->id) == NULL) {
        // required to make sure that we dont read the "WINNER:" declaration from the final aggregated file.
        // Helps in reducing the number of lines we read from the already existing file.
        indexFlag = -1;
      }
      FILE *readFile = fopen(parentNode->nameOfFile, "r");
      int numberOfLines = getNumberOfLinesInFile(readFile);
      // Construct the list of candidates from the votes of the aggregated file which already existed.
      for (int lineNumber = 0; lineNumber < numberOfLines + indexFlag; lineNumber++) {
        addCandNodeFromAggregateFile(readFile, argsCandsList, argsCandsNodeID);
      }
      if (fclose(readFile) != 0) {
        perror("Error when closing the writing file!");
      }
      FILE *writeFile = fopen(parentNode->nameOfFile, "w");
      // Write the information about all the candidates and their number of votes in the aggregated fashion by coalescing the two lists.
      writeCandInformationToFile(writeFile, candsList, argsCandsList);
      parentNode->numberOfProcessedChild += 1;
      pthread_cond_broadcast(threadArgs->cond);
      if (fclose(writeFile) != 0) {
        perror("Error when closing the writing file!");
      }
      // Renew the aggregated cands list for being able to get the information from upper aggregated file when used in the recursive call.
      argsCandsList->head->node = NULL;
      argsCandsList->head->next = NULL;
      *argsCandsNodeID = 0;
      // Check whether the current directory corresponds to the final directory (top most).
      if (getParentNodeFromChildNodeID(threadArgs->nodesList, parentNode->id) == NULL) {
        // if yes then append the winner information by first reading the data from the file (has been aggregated) and finding the particular winner candidate.
        getPermissionsForFile(parentNode->nameOfFile);
        FILE *readFile = fopen(parentNode->nameOfFile, "r");
        int numberOfLines = getNumberOfLinesInFile(readFile);
        for (int lineNumber = 0; lineNumber < numberOfLines; lineNumber++) {
          addCandNodeFromAggregateFile(readFile, argsCandsList, argsCandsNodeID);
        }
        if (fclose(readFile) != 0) {
          perror("Error when closing the writing file!");
        }
        FILE *writeFile = fopen(parentNode->nameOfFile, "w");
        candsList->head->node = NULL;
        candsList->head->next = NULL;
        // Doing this writing information again because opened the file in "w" mode.
        writeCandInformationToFile(writeFile, candsList, argsCandsList);
        // Append the information about the winner candidate. Need not use the original cands list because the argsCandsList now already contains the final information from the file
        // because the votes were read from the file again.
        appendWinnerInformation(writeFile, argsCandsList);
        if (fclose(writeFile) != 0) {
          perror("Error when closing the writing file!");
        }
      }
      runThreadOnAggregateFile(candsList, argsCandsList, threadArgs, parentNode, candNodeID, argsCandsNodeID);
    } else {
      // if the aggregated file does not exist, we can extend the same steps as used in the above implementation by skipping the part where we read from the file (no need because it doesnt exist).
      FILE *writeFile = fopen(parentNode->nameOfFile, "w");
      writeCandInformationToFile(writeFile, candsList, argsCandsList);
      parentNode->numberOfProcessedChild += 1;
      pthread_cond_broadcast(threadArgs->cond);
      if (fclose(writeFile) != 0) {
        perror("Error when closing the writing file!");
      }
      argsCandsList->head->node = NULL;
      argsCandsList->head->next = NULL;
      *argsCandsNodeID = 0;
      if (getParentNodeFromChildNodeID(threadArgs->nodesList, parentNode->id) == NULL) {
        getPermissionsForFile(parentNode->nameOfFile);
        FILE *readFile = fopen(parentNode->nameOfFile, "r");
        int numberOfLines = getNumberOfLinesInFile(readFile);
        for (int lineNumber = 0; lineNumber < numberOfLines; lineNumber++) {
          addCandNodeFromAggregateFile(readFile, argsCandsList, argsCandsNodeID);
        }
        if (fclose(readFile) != 0) {
          perror("Error when closing the writing file!");
        }
        FILE *writeFile = fopen(parentNode->nameOfFile, "w");
        candsList->head->node = NULL;
        candsList->head->next = NULL;
        writeCandInformationToFile(writeFile, candsList, argsCandsList);
        appendWinnerInformation(writeFile, argsCandsList);
        if (fclose(writeFile) != 0) {
          perror("Error when closing the writing file!");
        }
      }
      runThreadOnAggregateFile(candsList, argsCandsList, threadArgs, parentNode, candNodeID, argsCandsNodeID);
    }
  }
}

// Function which is executed by each thread. This function is responsible to actually make the thread first find the input file from the queue, then
// decrypt the contents of the input file and put them in the right file under the directory following DAG structure. Then the thread is supposed
// to read information from the leaf file and fill the candidate list  (actual list that the thread represents). Then the thread is responsible to
// start agregating the votes following the DAG structure. Also this function is responsible to implement all of this while maintining synchronization
// and correct functionaity. Also this function is responsible to write to the log txt file about the current thread and its status.
void decryptThread(void* arg) {
  struct decryptThreadArgs* threadArgs = (struct decryptThreadArgs*) arg;
  pthread_mutex_lock(threadArgs->lock);
  // get the input file from which the thread needs to decrypt information.
  char* inputFile = dequeue(threadArgs->fileQueue);
  trimwhitespace(inputFile);
  pthread_mutex_unlock(threadArgs->lock);
  pthread_mutex_lock(threadArgs->lock);
  // Get the node to which the input file corresponds.
  char **args = NULL;
  int length = makeargv(inputFile, "/", &args);
  pthread_mutex_unlock(threadArgs->lock);
  if (length > 1) {
    pthread_mutex_lock(threadArgs->lock);
    struct DAGNode* newNode = getNodeFromNodeName(threadArgs->nodesList, args[length-1]);
    pthread_mutex_unlock(threadArgs->lock);
    if (newNode == NULL) {
      fprintf(stderr, "Wrong File - %s\n", args[length-1]);
      return;
    }

    unsigned int threadID = pthread_self();
    pthread_mutex_lock(threadArgs->lock);
    // Write to the log file that the thread is starting.
    writeToLogTxtFile(threadArgs->logTxtFilePath, threadID, args[length-1], 0);
    pthread_mutex_unlock(threadArgs->lock);

    // decrypt the input file
    decryptFile(inputFile, newNode->nameOfFile);

    struct candidateList* candsList = (struct candidateList *) malloc(sizeof(struct candidateList));
    candsList->head = (struct candidateListNode*) malloc(sizeof(struct candidateListNode));
    candsList->head->node = NULL;
    candsList->head->next = NULL;
    int* candNodeID = (int *) malloc(sizeof(int));
    *candNodeID = 0;
    struct candidateList* argsCandsList = (struct candidateList *) malloc(sizeof(struct candidateList));
    argsCandsList->head = (struct candidateListNode*) malloc(sizeof(struct candidateListNode));
    argsCandsList->head->node = NULL;
    argsCandsList->head->next = NULL;
    int* argsCandsNodeID = (int *) malloc(sizeof(int));
    *argsCandsNodeID = 0;
    pthread_mutex_lock(threadArgs->lock);
    // Aggregate the votes for each candidate from the leaf file.
    runThreadOnLeafFile(candsList, threadArgs->nodesList, newNode, candNodeID);
    // pthread_mutex_unlock(threadArgs->lock);
    // pthread_mutex_lock(threadArgs->lock);
    // Run the aggregation on all the files in the hierarchical order until reached the top most directory.
    runThreadOnAggregateFile(candsList, argsCandsList, threadArgs, newNode, candNodeID, argsCandsNodeID);
    pthread_mutex_unlock(threadArgs->lock);

    threadID = pthread_self();
    pthread_mutex_lock(threadArgs->lock);
    // Write to the log file that the thread is ending.
    writeToLogTxtFile(threadArgs->logTxtFilePath, threadID, args[length-1], 1);
    pthread_mutex_unlock(threadArgs->lock);
  }
}
// ******************************************************************** //
// ******************************************************************** //
// ******************************************************************** //

int main(int argc, char **argv) {
  // THREAD_ARGS - represent the maximum number of threads that will be concurrently running.
  // Default number of THREAD_ARGS we require = 4 (if user not mentioned in the command line args)
  int THREAD_ARGS = 4;
  // check whether the number of arguments is correct.
  if (argc == 5) {
    THREAD_ARGS = atoi(argv[4]);
  } else if (argc != 4) {
		printf("Usage: %s %s %s %s\n", argv[0], "<DAG.txt>", "<input_dir>", "<output_dir>");
		return -1;
	}

  // Check if input file exists or not
  if (access(argv[1], F_OK) != -1) {
    FILE* readFile = fopen(argv[1], "r");
    int numOfLines = getNumberOfLinesInFile(readFile);
    if (numOfLines == 0) {
      printf("%s\n", "error:input file is empty");
      exit(-1);
    } else if (numOfLines == 1 && getLineSize(readFile) == 0) {
      printf("%s\n", "error:input file is empty");
      exit(-1);
    }
    fclose(readFile);
  } else {
    printf("%s\n", "error:input file not found");
    exit(-1);
  }

  char* input_dir = (char*) malloc(sizeof(char)*(strlen(argv[2])+3));
  strcpy(input_dir, argv[2]);
  char* output_dir = (char*) malloc(sizeof(char)*(strlen(argv[3])+3));
  strcpy(output_dir, argv[3]);

  // Need to add "/" at the end because of how the logic is set up for the functions.
  if (argv[2][strlen(argv[2])-1] != '/') {
    strcat(input_dir, "/");
  }
  if (argv[3][strlen(argv[3])-1] != '/') {
    strcat(output_dir, "/");
  }

  // **********************************************************
  // First construct DAG from the input.txt file.
  // **********************************************************
  struct DAGList* nodesList = constructDAG(argv[1]);
  // **********************************************************

  // **********************************************************
  // Section to remove the already existing output directory and create new one.
  // **********************************************************
  removeDirectories(output_dir);
  mkdir(output_dir, 0777);
  DFS(nodesList, output_dir);
  char* logTxtFilePath = createLogTxt(output_dir);
  // **********************************************************

  // **********************************************************
  // Making the queue of names.
  // **********************************************************
  int NUM_THREADS = 0;
  struct Queue* fileQueue = (struct Queue*) malloc(sizeof(struct Queue*));
  fileQueue->size = 0;
  populateQueueFromInputDir(fileQueue, input_dir, &NUM_THREADS, nodesList);
  if (NUM_THREADS == 0) {
    printf("%s\n", "error:input directory is empty");
    exit(-1);
  }
  // **********************************************************
  // Check for the minimum number of threads.
  THREAD_ARGS = THREAD_ARGS < NUM_THREADS ? THREAD_ARGS : NUM_THREADS;
  // **********************************************************
  // Threading to create the decrypted files.
  // **********************************************************
  struct decryptThreadArgs* args = (struct decryptThreadArgs*) malloc(sizeof(struct decryptThreadArgs));
  args->fileQueue = fileQueue;
  args->nodesList = nodesList;
  args->logTxtFilePath = logTxtFilePath;
  args->lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  args->cond = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));
  pthread_mutex_init(args->lock, NULL);
  pthread_cond_init(args->cond, NULL);
  // Run the threads until all leaf nodes are dealt with
  while (fileQueue->size > 0) {
    int threadCount = 0;
    // The number of threads working at one time should be equal to THREAD_ARGS
    pthread_t pool[THREAD_ARGS];
    // Start the threads
    while (threadCount < THREAD_ARGS && fileQueue->size > 0) {
      pthread_create(&pool[threadCount], NULL, decryptThread, (void*) args);
      threadCount++;
    }
    // Need to join all of the threads before starting to make other ones - FileQueue may not be empty.
    for (int i = 0; i < threadCount; i++) {
      pthread_join(pool[i], NULL);
    }
  }
}
