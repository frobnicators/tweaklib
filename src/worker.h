#ifndef TWEAKLIB_WORKER_H
#define TWEAKLIB_WORKER_H

#include <pthread.h>

struct worker {
	pthread_t thread;
	unsigned int id;
	int pipe[2];
	int sd;
	int slot;
	int running;
	char* peeraddr;
	int heap;
};

/**
 * For statically initializing a worker.
 */
#define WORKER_INITIALIZER {0, 0, {-1, -1}, -1, -1, 1, NULL, 0}

enum {
	READ_FD = 0,
	WRITE_FD = 1,
};

struct worker* worker_new();
void worker_free(struct worker* worker);

#endif /* TWEAKLIB_WORKER_H */
