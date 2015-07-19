#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "server.h"
#include "log.h"
#include "http.h"
#include "static.h"
#include "websocket.h"
#include "worker.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

enum {
	READ_FD = 0,
	WRITE_FD = 1,
};

enum IPC {
	IPC_SHUTDOWN = 1,
};

static struct worker server = {0, {0,0}, -1};
static const size_t buffer_size = 16384;

static void* server_loop(void*);
static void* client_loop(void*);

void server_init(int port, const char* listen_addr){
	/* make sure server isn't initailzed twice */
	if ( server.sd != -1 ){
		logmsg("cannot start multiple server instances\n");
		return;
	}

	/* IPC pipe */
	if ( pipe2(server.pipe, O_NONBLOCK) != 0 ){
		logmsg("pipe2() failed: %s\n", strerror(errno));
		return;
	}

	/* open socket */
	if ( (server.sd=socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
		logmsg("Failed to open socket: %s\n", strerror(errno));
		return;
	}

	/* enable reusing address, in many cases when tweaklib is used the user
	 * application is still restarted frequently. */
	int on = 1;
	if ( setsockopt(server.sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) != 0 ){
		logmsg("Failed to set SO_REUSEADDR: %s\n", strerror(errno));
	}

	/* bind to listening address */
	struct sockaddr_in addr = {0,};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if ( inet_pton(AF_INET, listen_addr, &addr.sin_addr.s_addr) != 1 ){
		logmsg("inet_pton() failed: invalid address\n");
		goto error;
	}
	if ( bind(server.sd, (struct sockaddr *)&addr, sizeof(addr)) < 0 ){
		logmsg("bind() failed: %s\n", strerror(errno));
		goto error;
	}

	/* allow incoming connections */
	if ( listen(server.sd, 5) != 0 ){
		logmsg("listen() failed: %s\n", strerror(errno));
		goto error;
	}

	/* create listening thread */
	int error;
	if ( (error=pthread_create(&server.thread, NULL, server_loop, NULL)) != 0 ){
		logmsg("pthread_create() failed: %s\n", strerror(error));
		goto error;
	}

	logmsg("Tweaklib server listening on %s:%d\n", listen_addr, port);
	return;

  error:
	close(server.sd);
	server.sd = -1;
}

void server_cleanup(){
	const enum IPC command = IPC_SHUTDOWN;
	if ( write(server.pipe[WRITE_FD], &command, sizeof(command)) == -1 ){
		logmsg("write() failed: %s\n", strerror(errno));
	}

	/* wait for server shutdown */
	pthread_join(server.thread, NULL);
}

static int max(int a, int b){
	return (a>b) ? a : b;
}

static void* server_loop(void* arg){
	const int max_fd = max(server.sd, server.pipe[0])+1;
	int running = 1;

	while (running) {
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(server.sd, &fds);
		FD_SET(server.pipe[0], &fds);

		if ( select(max_fd, &fds, NULL, NULL, NULL) == -1 ){
			logmsg("select() failed: %s\n", strerror(errno));
			continue;
		}

		/* handle IPC */
		if ( FD_ISSET(server.pipe[READ_FD], &fds) ){
			enum IPC command;
			if ( read(server.pipe[READ_FD], &command, sizeof(command)) == -1 ){
				logmsg("read() failed: %s\n", strerror(errno));
				continue;
			}

			logmsg("IPC command %d received\n", command);

			switch ( command ){
			case IPC_SHUTDOWN:
				running = 0;
				break;

			default:
				logmsg("Unknown IPC command %d ignored.\n", command);
			}

			continue;
		}

		/* wait for a client to connect */
		int cd;
		if ( (cd=accept(server.sd, NULL, NULL)) == -1 ){
			logmsg("accept() failed: %s\n", strerror(errno));
			break;
		}

		logmsg("client connected\n");

		/* allocate state, freed by client loop */
		struct worker* client = (struct worker*)malloc(sizeof(struct worker));
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
	};

	/* close socket */
	shutdown(server.sd, SHUT_RDWR);
	server.sd = -1;

	return NULL;
}

static void write_error(int sd, http_request_t req, http_response_t resp, int code, const char* details){
	const char* message = http_status_description(code);
	char* html = NULL;
	if ( asprintf(&html, "<h1>%d: %s</h1><p>%s</p>", code, message, details ? details : message) == -1 ){
		html = NULL;
	}

	header_add(&resp->header, "Content-Type", "text/html");
	http_response_status(resp, 404, message);
	http_response_write_header(sd, req, resp, 0);
	if ( html ){
		http_response_write_chunk(sd, html, strlen(html));
	}
	http_response_write_chunk(sd, NULL, 0);
}

static const char* websocket_derive_key(const char* key){
	static unsigned char hash[SHA_DIGEST_LENGTH];
	static char hex[512] = {0,};
	static char magic[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

	/* concaternate and hash */
	SHA_CTX ctx;
	SHA1_Init(&ctx);
	SHA1_Update(&ctx, (const unsigned char*)key, strlen(key));
	SHA1_Update(&ctx, (const unsigned char*)magic, strlen(magic));
	SHA1_Final((unsigned char*)hash, &ctx);

	/* encode hash as base64 */
	BIO *b64 = BIO_new(BIO_f_base64()); // create BIO to perform base64
	BIO *mem = BIO_new(BIO_s_mem()); // create BIO that holds the result
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	BIO_push(b64, mem);
	BIO_write(b64, hash, SHA_DIGEST_LENGTH);
	BIO_flush(b64);

	/* write copy into the static buffer */
	unsigned char* output;
	long bytes = BIO_get_mem_data(mem, &output);
	snprintf(hex, sizeof(hex), "%.*s", (int)bytes, output);

	BIO_free_all(b64);
	return hex;
}

static void handle_websocket(struct worker* client, const http_request_t req, http_response_t resp){
	const char* upgrade = header_find(&req->header, "Upgrade");
	const char* key = header_find(&req->header, "Sec-WebSocket-Key");
	int version = atoi(header_find(&req->header, "Sec-WebSocket-Version") ?: "0");

	/* validate that this request is actually requesting a websocket */
	if ( !upgrade || strcmp(upgrade, "websocket") != 0 ){
		write_error(client->sd, req, resp, 400, "Only websockets supported");
		return;
	}

	/* for simplicity only version 13 (current) is supported */
	if ( version != 13 ){
		write_error(client->sd, req, resp, 400, "Only websocket v13 supported");
		return;
	}

	/* upgrade this socket to a websocket */
	header_add(&resp->header, "Upgrade", "websocket");
	header_add(&resp->header, "Connection", "Upgrade");
	header_add(&resp->header, "Sec-WebSocket-Accept", websocket_derive_key(key));
	header_add(&resp->header, "Sec-WebSocket-Protocol", "v1.tweaklib.sidvind.com");
	header_del(&resp->header, "Transfer-Encoding");
	http_response_status(resp, 101, http_status_description(101));
	http_response_write_header(client->sd, req, resp, 1);

	/* retain this connection in a new loop */
	websocket_loop(client);
}

static void handle_get(struct worker* client, const http_request_t req, http_response_t resp){
	/* handle actual websocket */
	if ( strcmp(req->url, "/socket") == 0 ){
		handle_websocket(client, req, resp);
		return;
	}

	/* handle static files */
	struct file_entry* entry = file_table;
	while ( entry->filename ){
		if ( strcmp(entry->filename, req->url) != 0 ){
			entry++;
			continue;
		}

		/* static file found, send it */
		header_add(&resp->header, "Content-Type", entry->mime);
		http_response_status(resp, 200, "OK");
		http_response_write_header(client->sd, req, resp, 0);

		/* check if a local (non-builtin) file is present. */
		FILE* fp = fopen(entry->original, "r");
		if ( fp ){
			/* local file found, using it instead of builtin (helps during development) */
			char buf[4096];
			size_t bytes;
			while ( (bytes=fread(buf, 1, sizeof(buf), fp)) > 0 ){
				http_response_write_chunk(client->sd, buf, bytes);
			}
		} else {
			/* no local file, use builtin version */
			http_response_write_chunk(client->sd, entry->data, entry->bytes);
		}

		http_response_write_chunk(client->sd, NULL, 0);
		return;
	}

	/* nothing found, 404 */
	write_error(client->sd, req, resp, 404, NULL);
}

static void handle_post(struct worker* client, const http_request_t req, http_response_t resp){

}

void* client_loop(void* ptr){
	struct worker* client = (struct worker*)ptr;
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
			handle_get(client, &req, &resp);
			break;

		case HTTP_POST:
			handle_post(client, &req, &resp);
			break;
		}

		/* ensure request was handled in some way */
		if ( req.status == 0 ){
			logmsg("Unhandled request\n");
			write_error(client->sd, &req, &resp, 404, "No handler available for this request");
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
