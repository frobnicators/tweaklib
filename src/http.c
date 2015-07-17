#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "http.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

static const int HTTP_OK = 1;

static void header_alloc(struct header_list* hdr, size_t elem){
	hdr->num_alloc = elem;
	hdr->kv = realloc(hdr->kv, sizeof(struct header) * elem);
}

void header_add(struct header_list* hdr, const char* key, const char* value){
	if ( hdr->num_elem >= hdr->num_alloc ){
		header_alloc(hdr, hdr->num_alloc + 25);
	}
	hdr->kv[hdr->num_elem].key = strdup(key);
	hdr->kv[hdr->num_elem].value = strdup(value);
	hdr->num_elem++;
}

static int http_valid_protocol(http_request_t req, const char* protocol){
	/** @todo validate protocol version */
	return 1;
}

static int http_parse_method(http_request_t req, const char* method){
	if ( strcasecmp(method, "GET") == 0 ){
		req->method = HTTP_GET;
		return 1;
	} else if ( strcasecmp(method, "POST") == 0 ){
		req->method = HTTP_POST;
		return 1;
	}

	return 0;
}

static int http_parse_request(http_request_t req, char* line){
	if ( !line ) return 0;

	char* c = NULL;
	char* method = strtok_r(line, " ", &c);
	char* url = strtok_r(NULL, " ", &c);
	char* protocol = strtok_r(NULL, "\r\n", &c);

	if ( !http_valid_protocol(req, protocol) ){
		return 0;
	}

	if ( !http_parse_method(req, method) ){
		return 0;
	}

	req->url = strdup(url);

	logmsg("method: %s\n", method);
	logmsg("url: %s\n", url);
	logmsg("protocol: %s\n", protocol);
	return 1;
}

static int http_parse_header(http_request_t req, char* line){
	if ( !line ) return 0;
	logmsg("header: %s\n", line);

	char* c = NULL;
	char* key = strtok_r(line, ": ", &c);
	char* value = strtok_r(line, "\r\n", &c);
	if ( key && value ){
		header_add(&req->header, key, value);
	}

	return 1;
}

void http_request_init(http_request_t req){
	memset(req, 0, sizeof(struct http_request));
	header_alloc(&req->header, 25); /* default to fit 25 headers */
}

void http_request_free(http_request_t req){
	free(req->url);
	free(req->header.kv);
}

int http_request_read(http_request_t req, char* buf, size_t bytes){
	char* c = NULL;

	/* first line is always the actual request */
	http_parse_request(req, strtok_r(buf, "\r\n", &c));

	/* parse and store each header */
	while ( http_parse_header(req, strtok_r(NULL, "\r\n", &c)) ){}

	return HTTP_OK;
}

void http_response_init(http_response_t resp){
	memset(resp, 0, sizeof(struct http_response));
	header_alloc(&resp->header, 25); /* default to fit 25 headers */

	/* default headers */
	header_add(&resp->header, "Server", PACKAGE_STRING);
	header_add(&resp->header, "Connection", "keep-alive");
	header_add(&resp->header, "Transfer-Encoding", "chunked");
}

void http_response_free(http_response_t resp){
	free(resp->header.kv);
	free(resp->body);
}

void http_response_set_body(http_response_t resp, const char* body){
	resp->body = strdup(body);
}

void http_response_write_header(int sd, http_response_t resp){
	send(sd, resp->status, strlen(resp->status), MSG_MORE);
	send(sd, "\r\n", 2, MSG_MORE);

	for ( unsigned int i = 0; i < resp->header.num_elem; i++ ){
		send(sd, resp->header.kv[i].key, strlen(resp->header.kv[i].key), MSG_MORE);
		send(sd, ": ", 2, MSG_MORE);
		send(sd, resp->header.kv[i].value, strlen(resp->header.kv[i].value), MSG_MORE);
		send(sd, "\r\n", 2, MSG_MORE);
	}

	send(sd, "\r\n", 2, MSG_MORE);
}

void http_response_write_chunk(int sd, const char* data, size_t bytes){
	char len[32];
	snprintf(len, sizeof(len), "%zx\r\n", bytes);
	send(sd, len, strlen(len), MSG_MORE);

	/* if this is the last chunk: flush buffer */
	if ( bytes == 0 ){
		send(sd, "\r\n", 2, 0);
		return;
	}

	send(sd, data, bytes, MSG_MORE);
	send(sd, "\r\n", 2, MSG_MORE);
}
