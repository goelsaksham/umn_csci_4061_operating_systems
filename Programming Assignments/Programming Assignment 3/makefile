CC=gcc
CFLAGS=-std=c99
DBFLAGS=-g

make: votecounter.c
	$(CC) $(CFLAGS) -pthread -w -o votecounter votecounter.c

votecounter: votecounter.c
	$(CC) $(CFLAGS) -pthread -w -o votecounter votecounter.c

clean:
		rm votecounter
