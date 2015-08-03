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
};

enum {
	READ_FD = 0,
	WRITE_FD = 1,
};

#endif /* TWEAKLIB_WORKER_H */
