#ifndef TWEAKLIB_WORKER_H
#define TWEAKLIB_WORKER_H

#include <pthread.h>

struct worker {
	pthread_t thread;
	unsigned int id;
	int pipe[2];
	int sd;
	char* peeraddr;
};
#endif /* TWEAKLIB_WORKER_H */
