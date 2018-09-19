#define _BSD_SOURCE
#define NUM_ARGS 0

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
#include <pthread.h>

struct account {

	char* name;
	double balance;
	pthread_mutex_t* lock;
};

struct xfer {

	struct account* out;
	struct account* in;
	double amt;
};

struct depositStruct {

	struct account* in;
	double amt;
};

struct withdrawStruct {

	struct account* out;
	double amt;
};

void deposit(struct account* act, double amt) {
	pthread_mutex_lock(act->lock);
	act->balance += amt;
	pthread_mutex_unlock(act->lock);
}

void withdraw(struct account* act, double amt) {
	pthread_mutex_lock(act->lock);
	act->balance -= amt;
	pthread_mutex_unlock(act->lock);
}

void transfer(struct account* out, struct account* in, double amt) {
	pthread_mutex_lock(out->lock);
	withdraw(out, amt);
	pthread_mutex_lock(out->lock);
	pthread_mutex_lock(in->lock);
	deposit(in, amt);
	pthread_mutex_unlock(in->lock);
}

void depositTask(void* arg) {

	struct depositStruct* dep = arg;
	pthread_mutex_lock(dep->in->lock);
	// Randome delay. DO NOT ALTER!
	usleep(rand() % 100000);
	// Deposit to account.
	deposit(dep->in, dep->amt);

	pthread_mutex_unlock(dep->in->lock);
}

void withdrawTask(void* arg) {

	struct withdrawStruct* wit = arg;
	pthread_mutex_lock(wit->out->lock);
	// Random delay. DO NOT ALTER!
	usleep(rand() % 100000);
	// Withdraw from account.
	withdraw(wit->out, wit->amt);

	pthread_mutex_unlock(wit->out->lock);
}

void transferTask(void* arg) {
	struct xfer* x = arg;
	// Random delay. DO NOT ALTER!
	usleep(rand() % 100000);
	// Transfer.
	transfer(x->out, x->in, x->amt);
}

int main(int argc, char** argv) {

	if (argc < NUM_ARGS + 1) {

		printf("Wrong number of args, expected %d, given %d\n", NUM_ARGS, argc - 1);
		exit(1);
	}

	// Seed the random generator.
	srand(time(NULL));

	// Initialize bank account.
	struct account* bank1 = (struct account*) malloc(sizeof(struct account));
	bank1->name = (char*) malloc(sizeof(char) * 9);
	bank1->lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
	pthread_mutexattr_init(bank1->lock);
	strcpy(bank1->name, "Account1");
	bank1->balance = 2500;

	struct account* bank2 = (struct account*) malloc(sizeof(struct account));
	bank2->name = (char*) malloc(sizeof(char) * 9);
	bank2->lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
	pthread_mutexattr_init(bank2->lock);
	strcpy(bank2->name, "Account2");
	bank2->balance = 3000;

	// Create threads.
	pthread_t pool[1000];

	struct timeval start;
	gettimeofday(&start, NULL);



	// Launch them.
	for (int i=0; i < 400; ++i) {
		struct depositStruct* dep = (struct depositStruct*) malloc(sizeof(struct depositStruct));
		dep->in = (i / 200) ? bank1 : bank2;
		dep->amt = (i * 100);
		pthread_create(&pool[i], NULL, depositTask, (void*) dep);
	}

	for (int i=400; i < 800; ++i) {
		struct withdrawStruct* wit = (struct withdrawStruct*) malloc(sizeof(struct withdrawStruct));
		wit->out = (i / 600) ? bank1 : bank2;
		wit->amt = (i * 25);
		pthread_create(&pool[i], NULL, withdrawTask, (void*) wit);
	}

	for (int i=800; i < 1000; ++i) {
		struct xfer* x = (struct xfer*) malloc(sizeof(struct xfer));
		x->out = (i / 900) ? bank1 : bank2;
		x->in = (i / 900) ? bank2 : bank1;
		x->amt = i;
		pthread_create(&pool[i], NULL, transferTask, (void*) x);
	}

	struct timeval end;
	gettimeofday(&end, NULL);
	printf("The balance of account %s = %f\n", bank1->name, bank1->balance);
	printf("The balance of account %s = %f\n", bank2->name, bank2->balance);
	printf("Time to complete = %d\n", ((end.tv_sec - start.tv_sec) * 1000000) + (end.tv_usec - start.tv_usec));
}

