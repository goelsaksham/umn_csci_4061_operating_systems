CC=gcc
CFLAGS=-std=c99
DBFLAGS=-g

make: server.c client.c
	$(CC) $(CFLAGS) -pthread -w -o server server.c
	$(CC) $(CFLAGS) -pthread -w -o client client.c

server: server.c
	$(CC) $(CFLAGS) -pthread -w -o server server.c

client: client.c
	$(CC) $(CFLAGS) -pthread -w -o client client.c

clean:
		rm server
		rm client
