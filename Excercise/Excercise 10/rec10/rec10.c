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
#include <semaphore.h>

long condTotal = 0;
long semTotal = 0;

struct queue {

	int vals[100];
	int index;
};

struct condQueue {

	struct queue* q;
	pthread_cond_t* cond;
	pthread_mutex_t* mutex;
};

struct semQueue {

	struct queue* q;
	sem_t* sem;
	pthread_mutex_t* mutex;
};

void enqueue(struct queue* q, int val) {

	q->vals[q->index] = val;
	++q->index;
}

int dequeue(struct queue* q) {

	--q->index;
	int val = q->vals[q->index];
	return val;
}

// TODO: Insert code to use a condition variable.
void condProducer(void* arg) {

	// Random delay. DO NOT REMOVE!
	usleep(rand() % 1000);

	struct condQueue* cq = (struct condQueue*) arg;
	// Counter.
	static int in = 0;
	++in;

	pthread_mutex_lock(cq->mutex);
	// Add an element to the queue.
	// while (cq->q->index >= 100) {
	// 	pthread_cond_wait(cq->cond, cq->mutex);
	// }
	enqueue(cq->q, in);
	pthread_cond_signal(cq->cond);
	pthread_mutex_unlock(cq->mutex);
}

// TODO: Insert code to use a condition variable.
void condConsumer(void* arg) {

	// Random delay. DO NOT REMOVE!
	usleep(rand() % 1000);

	struct condQueue* cq = (struct condQueue*) arg;

	pthread_mutex_lock(cq->mutex);
	while (cq->q->index <= 0) {
		pthread_cond_wait(cq->cond, cq->mutex);
	}
	// Remove an element from the queue.
	condTotal += dequeue(cq->q);
	// pthread_cond_signal(cq->cond);
	pthread_mutex_unlock(cq->mutex);
}

// TODO: Insert code to use a semaphore.
void semProducer(void* arg) {

	// Random delay. DO NOT REMOVE!
	usleep(rand() % 1000);

	struct semQueue* sq = (struct semQueue*) arg;

	static int in = 0;
	++in;

	pthread_mutex_lock(sq->mutex);
	// Add an element to the queue.
	// while (sq->q->index >= 100) {
	// 	sem_wait(sq->sem);
	// }
	enqueue(sq->q, in);
	sem_post(sq->sem);
	pthread_mutex_unlock(sq->mutex);
}

// TODO: Insert code to use a semaphore.
void semConsumer(void* arg) {

	// Random delay. DO NOT REMOVE!
	usleep(rand() % 1000);

	struct semQueue* sq = (struct semQueue*) arg;
	pthread_mutex_lock(sq->mutex);
	while (sq->q->index <= 0) {
		sem_wait(sq->sem);
	}
	// Reove an element from the queue.
	semTotal += dequeue(sq->q);
	// sem_post(sq->sem);
	pthread_mutex_unlock(sq->mutex);
}

int main(int argc, char** argv) {

	if (argc != NUM_ARGS + 1) {

		printf("Wrong number of args, expected %d, given %d\n", NUM_ARGS, argc - 1);
		exit(1);
	}

	// Seed the random generator.
	srand(time(NULL));

	// Create threads.
	pthread_t condPool[100];

	struct timeval start;
	gettimeofday(&start, NULL);

	// Create the cond variable controlled task queue.
	struct condQueue* cq = (struct condQueue*) malloc(sizeof(struct condQueue));
	cq->q = (struct queue*) malloc(sizeof(struct queue));
	cq->cond = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));
	cq->mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
	pthread_cond_init(cq->cond, NULL);
	pthread_mutex_init(cq->mutex, NULL);

	// Launch them.
	for (int i=0; i < 50; ++i) {

		pthread_create(&condPool[i], NULL, condProducer, (void*) cq);
		pthread_create(&condPool[50 + i], NULL, condConsumer, (void*) cq);
	}

	for (int i=0; i < 100; ++i) pthread_join(condPool[i], NULL);

	struct timeval end;
	gettimeofday(&end, NULL);

	printf("Cond Test: \nTotal of queue = %d\n", condTotal);
	printf("Time (in us) to run = %d\n\n", ((end.tv_sec - start.tv_sec) * 1000000) + (end.tv_usec - start.tv_usec));

	// Create threads.
	pthread_t semPool[100];

	gettimeofday(&start, NULL);

	// Create task queue.
	struct semQueue* sq = (struct semQueue*) malloc(sizeof(struct semQueue));
	sq->q = (struct queue*) malloc(sizeof(struct queue));
	sq->sem = (sem_t*) malloc(sizeof(sem_t));
	sq->mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
	sem_init(sq->sem, 0, 0);
	pthread_mutex_init(sq->mutex, NULL);

	for (int i=0; i < 50; ++i) {

		pthread_create(&semPool[i], NULL, semProducer, (void*) sq);
		pthread_create(&semPool[50 + i], NULL, semConsumer, (void*) sq);
	}

	for (int i=0; i < 100; ++i) pthread_join(semPool[i], NULL);

	gettimeofday(&end, NULL);
	printf("Sem Test: \nTotal of queue = %d\n", semTotal);
	printf("Time (in us) to complete = %d\n", ((end.tv_sec - start.tv_sec) * 1000000) + (end.tv_usec - start.tv_usec));
}
