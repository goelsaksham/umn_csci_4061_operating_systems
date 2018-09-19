#define _BSD_SOURCE
#define NUM_ARGS 1

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define SERVER_PORT 4061

int main(int argc, char** argv) {

	if (argc < NUM_ARGS + 1) {

		printf("Wrong number of args, expected %d, given %d\n", NUM_ARGS, argc - 1);
		exit(1);
	}

	// Create a TCP socket.
	int sock = socket(AF_INET , SOCK_STREAM , 0);

	// Specify an address to connect to (we use the local host or 'loop-back' address).
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(SERVER_PORT);
	address.sin_addr.s_addr = inet_addr("127.0.0.1");

	// Connect it.
	if (connect(sock, (struct sockaddr *) &address, sizeof(address)) == 0) {

		// Buffer for data.
		char buffer[256];

		// Open the file.
		int fd = open(argv[1], O_RDONLY);

		// TODO: Read the file and write to the socket.
		while(read(fd, buffer, 256) != 0) {
			write(sock, buffer, 256);
		}

		// Close the file.
		close(fd);
		close(sock);

	} else {

		perror("Connection failed!");
	}
}
