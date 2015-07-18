#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "server.h"
#include "log.h"
#include "http.h"
#include "static.h"

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
static const size_t buffer_size = 16384;

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

	logmsg("Tweaklib server listening on %s:%d\n", "127.0.0.1", port); /* @todo listen_addr */
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

static void handle_get(int sd, const http_request_t req, http_response_t resp){
	/* handle static files */
	struct file_entry* entry = file_table;
	while ( entry->filename ){
		if ( strcmp(entry->filename, req->url) != 0 ){
			entry++;
			continue;
		}

		/* static file found, send it */
		resp->status = "HTTP/1.1 200 OK";
		header_add(&resp->header, "Content-Type", entry->mime);
		http_response_write_header(sd, req, resp);
		http_response_write_chunk(sd, entry->data, entry->bytes);
		http_response_write_chunk(sd, NULL, 0);
		return;
	}

	/* nothing found, 404 */
	const char* error = "<h1>404: Not Found</h1>";
	resp->status = "HTTP/1.1 404 Not Found";
	header_add(&resp->header, "Content-Type", "text/html");
	http_response_write_header(sd, req, resp);
	http_response_write_chunk(sd, error, strlen(error));
	http_response_write_chunk(sd, NULL, 0);
}

static void handle_post(int sd, const http_request_t req, http_response_t resp){

}

void* client_loop(void* ptr){
	struct client* client = (struct client*)ptr;
	char* buf = malloc(buffer_size);

	for (;;){
		/* wait for next request */
		ssize_t bytes = recv(client->sd, buf, buffer_size-1, 0); /* -1 so null terminator will fit */
		if ( bytes == -1 ){
			logmsg("recv() failed: %s\n", strerror(errno));
			break;
		} else if ( bytes == 0 ){
			logmsg("connection closed\n");
			break;
		}

		/** @todo handle when request is larger than buffer_size */

		/* force null-terminator */
		buf[bytes] = 0;

		/* parse request */
		struct http_request req;
		http_request_init(&req);
		if ( !http_request_read(&req, buf, bytes) ){
			logmsg("Malformed request ignored.\n");
			http_request_free(&req);
			continue;
		}

		/* generate response */
		struct http_response resp;
		http_response_init(&resp);
		switch ( req.method ){
		case HTTP_GET:
			handle_get(client->sd, &req, &resp);
			break;

		case HTTP_POST:
			handle_post(client->sd, &req, &resp);
			break;
		}

		/* free resources allocated for this request */
		http_response_free(&resp);
		http_request_free(&req);
	}

	/* close client connection */
	shutdown(client->sd, SHUT_RDWR);
	free(client);
	return NULL;
}
