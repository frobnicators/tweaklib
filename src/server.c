#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "server.h"
#include "log.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

static int sd = -1;
static pthread_t thread;

struct client {
	pthread_t thread;
	int sd;
};

static void* server_loop(void*);
static void* client_loop(void*);

void server_init(int port, const char* listen_addr){
	/* make sure server isn't initailzed twice */
	if ( sd != -1 ){
		logmsg("cannot start multiple server instances\n");
		return;
	}

	/* open socket */
	if ( (sd=socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
		logmsg("Failed to open socket: %s\n", strerror(errno));
		return;
	}

	/* enable reusing address, in many cases when tweaklib is used the user
	 * application is still restarted frequently. */
	int on = 1;
	if ( setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) != 0 ){
		logmsg("Failed to set SO_REUSEADDR: %s\n", strerror(errno));
	}

	/* bind to listening address */
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); /** @todo listen_addr */
	if ( bind(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0 ){
		logmsg("bind() failed: %s\n", strerror(errno));
		goto error;
	}

	/* allow incoming connections */
	if ( listen(sd, 5) != 0 ){
		logmsg("listen() failed: %s\n", strerror(errno));
		goto error;
	}

	/* create listening thread */
	int error;
	if ( (error=pthread_create(&thread, NULL, server_loop, NULL)) != 0 ){
		logmsg("pthread_create() failed: %s\n", strerror(error));
		goto error;
	}

	return;

  error:
	close(sd);
	sd = -1;
}

static void* server_loop(void* arg){
	do {
		/* wait for a client to connect */
		int cd;
		if ( (cd=accept(sd, NULL, NULL)) == -1 ){
			logmsg("accept() failed: %s\n", strerror(errno));
			break;
		}

		logmsg("client connected\n");

		/* allocate state, freed by client loop */
		struct client* client = (struct client*)malloc(sizeof(struct client));
		if ( !client ){
			logmsg("malloc() failed: %s\n", strerror(errno));
			shutdown(cd, SHUT_RDWR);
			continue;
		}
		client->sd = cd;

		/* create thread for client */
		int error;
		if ( (error=pthread_create(&client->thread, NULL, client_loop, client)) != 0 ){
			logmsg("pthread_create() failed: %s\n", strerror(error));
			shutdown(cd, SHUT_RDWR);
		}
	} while (1);

	/* close socket */
	shutdown(sd, SHUT_RDWR);
	sd = -1;

	return NULL;
}

void* client_loop(void* ptr){
	struct client* client = (struct client*)ptr;

	/* close client connection */
	shutdown(client->sd, SHUT_RDWR);
	free(client);
	return NULL;
}
