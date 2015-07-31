#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "websocket.h"
#include "list.h"
#include "log.h"
#include "vars.h"
#include "utils/base64.h"
#include "utils/sha1.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <json.h>

static const size_t buffer_size = 16384;
extern list_t vars;

struct frame_header {
#if __BYTE_ORDER == __LITTLE_ENDIAN
	uint8_t opcode:4;
	uint8_t res:3;
	uint8_t fin:1;
	uint8_t plen1:7;
	uint8_t mask:1;
#elif __BYTE_ORDER == __BIG_ENDIAN
	uint8_t fin:1;                                       /* final fragment: if 0 the frame is fragmented */
	uint8_t res:3;                                       /* reserved */
	uint8_t opcode:4;                                    /* control codes, see enum */
	uint8_t mask:1;                                      /* payload masked, if 1 the masking key is present and payload must be decoded */
	uint8_t plen1:7;                                     /* payload length (for small sizes) */
#endif
} __attribute__((packed));

enum {
	OPCODE_CONTINUATION = 0,
	OPCODE_TEXT = 1,
	OPCODE_BINARY = 2,
	OPCODE_CLOSE = 8,
	OPCODE_PING = 9,
	OPCODE_PONG = 10,
};

static char* read16(int sd, char* buf, uint16_t* dst){
	recv(sd, buf, sizeof(uint16_t), 0);
	*dst = be16toh(*(const uint16_t*)buf);
	return buf + sizeof(uint16_t);
}

static char* read32_n(int sd, char* buf, uint32_t* dst){
	recv(sd, buf, sizeof(uint32_t), 0);
	*dst = *(const uint32_t*)buf;
	return buf + sizeof(uint32_t);
}

static char* read64(int sd, char* buf, uint64_t* dst){
	recv(sd, buf, sizeof(uint64_t), 0);
	*dst = be64toh(*(const uint64_t*)buf);
	return buf + sizeof(uint64_t);
}

static char* websocket_payload_size(int sd, char* ptr, const struct frame_header* header, size_t* size){
	*size = header->plen1;

	if ( header->plen1 == 126 ){
		uint16_t tmp;
		ptr = read16(sd, ptr, &tmp);
		*size = tmp;
	}

	if ( header->plen1 == 127 ){
		uint64_t tmp;
		ptr = read64(sd, ptr, &tmp);
		*size = tmp;
	}

	return ptr;
}

static char* websocket_frame_masking_key(int sd, char* ptr, const struct frame_header* header, uint32_t* key){
	if ( header->mask ){
		return read32_n(sd, ptr, key);
	} else {
		*key = 0;
		return ptr;
	}
}

static char* websocket_frame_payload(int sd, char* ptr, size_t left, uint32_t masking_key){
	char* begin = ptr;

	/** @todo validate buffer size vs payload size */
	while ( left > 0 ){
		ssize_t bytes = recv(sd, ptr, left, 0);
		if ( bytes <= 0 ) return NULL;
		left -= bytes;
		ptr += bytes;
	}

	/* unmask if needed */
	if ( masking_key ){
		uint32_t* cur = (uint32_t*)(begin);
		const uint32_t* end = (uint32_t*)(ptr + 4); /** @todo detect align to 4 */
		while ( cur < end ){
			*cur++ ^= masking_key;
			//cur++;
		}
	}

	return ptr;
}

static int min(int a, int b){
	return (a<b) ? a : b;
}

static void websocket_send(struct worker* client, const char* buffer, size_t len){
	/* setup frame */
	struct frame_header frame;
	frame.fin = 1;
	frame.res = 0;
	frame.opcode = OPCODE_TEXT;
	frame.mask = 0;
	frame.plen1 = min(len,126);
	uint16_t plen = htobe16(len);

	/* send frame */
	send(client->sd, &frame, sizeof(struct frame_header), MSG_MORE);
	if ( len >= 126 ){
		send(client->sd, &plen, sizeof(uint16_t), MSG_MORE);
	}
	send(client->sd, buffer, len, 0);
}

static struct json_object* serialize_vars(){
	struct json_object* json_vars = json_object_new_array();
	for ( void** it = list_begin(vars); it != list_end(vars); it++ ){
		struct var* var = *(struct var**)it;
		struct json_object* json_var = json_object_new_object();
		json_object_object_add(json_var, "name", json_object_new_string(var->name));
		json_object_object_add(json_var, "handle", json_object_new_int(var->handle));
		json_object_object_add(json_var, "description", var->description ? json_object_new_string(var->description) : NULL);
		json_object_object_add(json_var, "datatype", json_object_new_int(var->datatype));
		json_object_object_add(json_var, "value", var->store(var));
		json_object_array_add(json_vars, json_var);
	}
	return json_vars;
}

static void websocket_hello(struct worker* client){
	struct json_object* root = json_object_new_object();
	json_object_object_add(root, "vars", serialize_vars());
	json_object_object_add(root, "type", json_object_new_string("hello"));

	const char* data = json_object_to_json_string_ext(root, 0);
	websocket_send(client, data, strlen(data));
}

static void handle_update(struct json_object* json){
	struct json_object* handle;
	struct json_object* value;

	if ( !json_object_object_get_ex(json, "handle", &handle) ){
		logmsg("update missing handle\n");
		return;
	}

	if ( !json_object_object_get_ex(json, "value", &value) ){
		logmsg("update missing value\n");
		return;
	}

	struct var* var = var_from_handle(json_object_get_int(handle));
	if ( var ){
		var->load(var, value);
	}
}

static void handle_message(struct worker* client, const char* data){
	struct json_object* json = json_tokener_parse	(data);
	if ( !json ){
		logmsg("Failed to parse JSON\n");
		return;
	}

	struct json_object* type;
	if ( !json_object_object_get_ex(json, "type", &type) ){
		logmsg("message missing type\n");
		return;
	}

	const char* type_str = json_object_get_string(type);
	if ( strcmp(type_str, "update") == 0 ){
		handle_update(json);
	} else {
		logmsg("unhandled message type %s\n", type_str);
	}
}

void websocket_loop(struct worker* client){
	char* buf = malloc(buffer_size);
	int running = 1;

	logmsg("%s - websocket opened\n", client->peeraddr);

	websocket_hello(client);

	while (running){
		/* wait for next request */
		ssize_t bytes = recv(client->sd, buf, sizeof(struct frame_header), 0);
		if ( bytes == -1 ){
			logmsg("recv() failed: %s\n", strerror(errno));
			break;
		} else if ( bytes == 0 ){
			logmsg("connection closed\n");
			break;
		}

		const struct frame_header* frame = (const struct frame_header*)buf;
		char* ptr = buf + sizeof(struct frame_header);

		/* fragmentation */
		if ( !frame->fin ){
			logmsg("Fragmented frames is not supported yet.\n");
			continue;
		}

		/* read frame size */
		size_t payload_size;
		ptr = websocket_payload_size(client->sd, ptr, frame, &payload_size);

		/* read masking key */
		uint32_t masking_key;
		ptr = websocket_frame_masking_key(client->sd, ptr, frame, &masking_key);

		/* read full payload */
		char* payload = ptr;
		if ( (ptr=websocket_frame_payload(client->sd, ptr, payload_size, masking_key)) == NULL ){
			logmsg("Failed to receive payload");
			continue;
		}

		switch ( frame->opcode ){
		case OPCODE_TEXT:
			/* add null-terminator */
			payload[payload_size] = 0;

			logmsg("payload: %s\n", payload);
			handle_message(client, payload);
			break;

		case OPCODE_CLOSE:
			running = 0;
			break;

		default:
			logmsg("unhandled opcode %d\n", frame->opcode);
		}
	}

	logmsg("%s - websocket closed\n", client->peeraddr);
	free(buf);
}

const char* websocket_derive_key(const char* key){
	static char hex[512] = {0,};
	static char magic[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

	/* compute concaternated hash */
	sha1_t sha = sha1_new();
	sha1_update(sha, key, strlen(key));
	sha1_update(sha, magic, strlen(magic));

	/* encode hash as base64 */
	base64encode(sha1_hash_bytes(sha), 20, hex, sizeof(hex));

	sha1_free(sha);
	return hex;
}
