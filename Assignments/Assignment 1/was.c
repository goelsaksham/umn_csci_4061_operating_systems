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


void tee(char * path, char * executable, char * filename) {
  if (chmod(filename, 0777) == -1) {
    return -1;
  } else {
    int op = open(filename, O_WRONLY);
    pid_t pid = fork();
    if (pid > 0) {
      wait(NULL);
      dup2(op, 1);
      pid_t nextpid = fork();
      if (nextpid > 0) {
        wait(NULL);
        return 0;
      } else {
        execl(path, executable, NULL);
      }
    } else {
      execl(path, executable, NULL);
    }
  }
}

int  main(int argc, char *argv[]) {
  tee("/bin/ls", "ls", "foo");
}
