CC=gcc
CFLAGS=-std=c99
DBFLAGS=-g

make: btree.c
	$(CC) $(CFLAGS) -o btree btree.c

debug: btree.c
	$(CC) $(CFLAGS) $(DBFLAGS) -o btree btree.c

clean:
	rm btree
