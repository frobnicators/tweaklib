#ifndef TWEAKLIB_HTTP_H
#define TWEAKLIB_HTTP_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum http_method {
	HTTP_GET,
	HTTP_POST,
};

struct header {
	char* key;
	char* value;
};

struct header_list {
	size_t num_alloc;                     /* number of allocated entries */
	size_t num_elem;                      /* number of actual headers */
	struct header* kv;
};

struct http_request {
	enum http_method method;              /* HTTP method */
	char* url;                            /* Request URL (can be NULL)*/
	struct header_list header;            /* Request headers */
	int status;                           /* If server has handled this request it is set to the reply status code */
};

struct http_response {
	int statuscode;
	char* statusline;
	struct header_list header;
	char* body;
};

typedef struct http_request* http_request_t;
typedef struct http_response* http_response_t;

void header_add(struct header_list* hdr, const char* key, const char* value);

void http_request_init(http_request_t req);
void http_request_free(http_request_t req);
int http_request_read(http_request_t req, char* buf, size_t bytes);


void http_response_init(http_response_t resp);
void http_response_free(http_response_t resp);

void http_response_status(http_response_t resp, int code, const char* msg);
void http_response_write_header(int sd, http_request_t req, http_response_t resp);
void http_response_write_chunk(int sd, const char* data, size_t bytes);

/**
 * Returns textual description of a HTTP status code, e.g. 404 -> "Not Found"
 * Return value is a pointer to static memory.
 */
const char* http_status_description(int code);

#ifdef __cplusplus
}
#endif

#endif /* TWEAKLIB_HTTP_H */
