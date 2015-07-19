#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "http.h"
#include "server.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

static const int HTTP_OK = 1;

static struct header* header_find_int(const struct header_list* hdrlist, const char* key);

static void header_alloc(struct header_list* hdr, size_t elem){
	hdr->num_alloc = elem;
	hdr->kv = realloc(hdr->kv, sizeof(struct header) * elem);
}

void header_add(struct header_list* hdr, const char* key, const char* value){
	/* overwrite header if one exists already */
	struct header* existing = header_find_int(hdr, key);
	if ( existing ){
		free(existing->value);
		existing->value = strdup(value);
		return;
	}

	if ( hdr->num_elem >= hdr->num_alloc ){
		header_alloc(hdr, hdr->num_alloc + 25);
	}
	hdr->kv[hdr->num_elem].key = strdup(key);
	hdr->kv[hdr->num_elem].value = strdup(value);
	hdr->num_elem++;
}

void header_del(struct header_list* hdr, const char* key){
	struct header* existing = header_find_int(hdr, key);
	if ( !existing ){
		return;
	}

	/* release resources for this header */
	free(existing->key);
	free(existing->value);

	/* move the last header to this location */
	struct header* last = &hdr->kv[hdr->num_elem-1];
	existing->key = last->key;
	existing->value = last->value;
	hdr->num_elem--;
}

const struct header* header_begin(const struct header_list* hdr){
	if ( hdr->num_elem == 0 ) return NULL;
	return &hdr->kv[0];
}

const struct header* header_end(const struct header_list* hdr){
	if ( hdr->num_elem == 0 ) return NULL;
	return &hdr->kv[hdr->num_elem];
}

static struct header* header_find_int(const struct header_list* hdrlist, const char* key){
	/* hack to cast away const without warnings so the internal version can return
	 * a non-const pointer but the exposed one is still const */
	union { const struct header* ro; struct header* rw; } hdr;

	for ( hdr.ro = header_begin(hdrlist); hdr.ro != header_end(hdrlist); hdr.ro++ ){
		if ( strcmp(hdr.ro->key, key) == 0 ){
			return hdr.rw;
		}
	}
	return NULL;
}

const char* header_find(const struct header_list* hdrlist, const char* key){
	const struct header* hdr = header_find_int(hdrlist, key);
	return hdr ? hdr->value : NULL;
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
	return 1;
}

static int http_parse_header(http_request_t req, char* line){
	if ( !line ) return 0;

	char* c = NULL;
	char* key = strtok_r(line, ": ", &c);
	char* value = strtok_r(NULL, "\r\n", &c);
	while ( value[0] == 32 ) value++; /* trim leading spaces */
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
	free(resp->statusline);
}

const char* method_str(enum http_method method){
	switch ( method ){
	case HTTP_GET: return "GET";
	case HTTP_POST: return "POST";
	}
	return "UNKNOWN";
}

const char* http_status_description(int code){
	switch ( code ){
	case 101: return "Switching Protocols";
	case 400: return "Bad Request";
	case 403: return "Forbidden";
	case 404: return "Not Found";
	case 500: return "Internal Server Error";
	default: return "Invalid status";
	}
}

void http_response_status(http_response_t resp, int code, const char* msg){
	resp->statuscode = code;
	if ( asprintf(&resp->statusline, "HTTP/1.1 %d %s", code, msg) == -1 ){
		resp->statusline = strdup("HTTP/1.1 500 Internal Server Error");
	}
}

void http_response_write_header(int sd, http_request_t req, http_response_t resp, int only_header){
	char buf[PEER_ADDR_LEN];
	logmsg("%s - %s %s -> %d\n", peer_addr(sd, buf), method_str(req->method), req->url, resp->statuscode);
	req->status = resp->statuscode;

	send(sd, resp->statusline, strlen(resp->statusline), MSG_MORE);
	send(sd, "\r\n", 2, MSG_MORE);

	for ( unsigned int i = 0; i < resp->header.num_elem; i++ ){
		send(sd, resp->header.kv[i].key, strlen(resp->header.kv[i].key), MSG_MORE);
		send(sd, ": ", 2, MSG_MORE);
		send(sd, resp->header.kv[i].value, strlen(resp->header.kv[i].value), MSG_MORE);
		send(sd, "\r\n", 2, MSG_MORE);
	}

	send(sd, "\r\n", 2, only_header ? 0 : MSG_MORE);
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
