CC=gcc
CFLAGS=-std=c99
DBFLAGS=-g

make: Leaf_Counter.c Aggregate_Votes.c Vote_Counter.c
	$(CC) $(CFLAGS) -w -o Leaf_Counter Leaf_Counter.c
	$(CC) $(CFLAGS) -w -o Aggregate_Votes Aggregate_Votes.c
	$(CC) $(CFLAGS) -w -o Vote_Counter Vote_Counter.c

Leaf_Counter: Leaf_Counter.c
	$(CC) $(CFLAGS) -w -o Leaf_Counter Leaf_Counter.c


Aggregate_Votes: Aggregate_Votes.c
	$(CC) $(CFLAGS) -w -o Aggregate_Votes Aggregate_Votes.c


Vote_Counter: Vote_Counter.c
	$(CC) $(CFLAGS) -w -o Vote_Counter Vote_Counter.c

clean:
		rm Leaf_Counter
		rm Aggregate_Votes
		rm Vote_Counter
