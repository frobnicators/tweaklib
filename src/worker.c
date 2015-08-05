#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "worker.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/socket.h>

static void worker_reset(struct worker* worker){
	assert(worker);
	memset(worker, 0, sizeof(struct worker));
	worker->pipe[0] = -1;
	worker->pipe[0] = -1;
	worker->sd = -1;
	worker->slot = -1;
}

struct worker* worker_new(){
	struct worker* worker = malloc(sizeof(struct worker));
	if ( worker ){
		worker_reset(worker);
		worker->heap = 1;
	}
	return worker;
}

void worker_free(struct worker* worker){
	/* release resources */
	if ( worker->pipe[0] >= 0 ) close(worker->pipe[0]);
	if ( worker->pipe[1] >= 0 ) close(worker->pipe[1]);
	if ( worker->sd >= 0 ) shutdown(worker->sd, SHUT_RDWR);
	free(worker->peeraddr);

	/* reset memory in case someone tries to access it again */
	worker_reset(worker);

	/* if this worker was allocated using worker_new() heap will be true, if it
	 * was allocated manually the user should either set this flag or free
	 * manually. */
	if ( worker->heap ){
		free(worker);
	}
}
