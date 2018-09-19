#define _BSD_SOURCE
#define NUM_ARGS 0
#define NUM_THRD 1000

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
#include <semaphore.h>

// DO NOT ALTER!
struct philosopher {

	int food;
	pthread_mutex_t* left;
	pthread_mutex_t* right;
};

// DO NOT ALTER!
void eat(struct philosopher* p) {

	--(p->food);

	usleep(rand() % 10);
}

void dine(struct philosopher* p) {

	// Grab the utensils.
	pthread_mutex_lock(p->left);
	if (pthread_mutex_trylock(p->right) == 0) {
		eat(p);
		pthread_mutex_unlock(p->right);
	}
	// Put them down.
	pthread_mutex_unlock(p->left);

}

// DO NOT ALTER!
void threadFun(void* arg) {

	usleep(rand() % 10);

	struct philosopher* p = (struct philosopher*) arg;

	while (p->food > 0) dine(p);
}

// DO NOT ALTER!
int main(int argc, char** argv) {

	if (argc != NUM_ARGS + 1) {

		printf("Wrong number of args, expected %d, given %d\n", NUM_ARGS, argc - 1);
		exit(1);
	}

	// Seed the random generator.
	srand(time(NULL));

	// Create threads.
	pthread_t pool[NUM_THRD];
	struct philosopher* p[NUM_THRD];

	// Create the utensils (mutexes).
	pthread_mutex_t* locks[NUM_THRD];
	for (int i=0; i < NUM_THRD; ++i) {

		locks[i] = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(locks[i], NULL);
	}

	// Create the philosophers.
	for (int i=0; i < NUM_THRD; ++i) {

		p[i] = (struct philosopher*) malloc(sizeof(struct philosopher));
		p[i]->food = 100;
		p[i]->left = locks[i];
		p[i]->right = locks[(i+1) % NUM_THRD];
	}

	struct timeval start;
	struct timeval end;

	gettimeofday(&start, NULL);

	for (int i=0; i < NUM_THRD; ++i) pthread_create(&pool[i], NULL, threadFun, (void*) p[i]);
	for (int i=0; i < NUM_THRD; ++i) pthread_join(pool[i], NULL);

	gettimeofday(&end, NULL);

	printf("Time (in us) to complete = %d\n", ((end.tv_sec - start.tv_sec) * 1000000) + (end.tv_usec - start.tv_usec));
}
