CC=gcc
CFLAGS=-std=c99
DBFLAGS=-g

make: intro.c 
	$(CC) $(CFLAGS) -o comp.out intro.c 

debug: intro.c
	$(CC) $(CFLAGS) $(DBFLAGS) -o comp.out intro.c

clean: 
	rm comp.out
