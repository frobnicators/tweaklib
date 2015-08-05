#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ipc.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

static const size_t max_payload_size = 256;

/**
 * Wrapper for read which handles some error conditions. If the error cannot be
 * handled by this it should be considered fatal.
 *
 * @return dst + bytes or NULL on unhandled errors
 */
static void* read_wrapper(int fd, void* dst, size_t bytes){
	void* out = dst;
	while ( bytes > 0 ){
		ssize_t result = read(fd, out, bytes);
		if ( result == -1 ){
			switch ( errno ){
			case EAGAIN:
			case EINTR:
				continue;
			default:
				return NULL;
			}
		}

		if ( result == 0 ){
			errno = EBADFD;
			return NULL;
		}

		bytes -= result;
		out += result;
	}

	return out;
}

static void ipc_fetch_payload(struct worker* client, void* dst, size_t payload_size){
	char buf[max_payload_size];

	if ( read_wrapper(client->pipe[READ_FD], dst ? dst : buf, payload_size) == NULL ){
		logmsg("ipc_fetch_payload - read() failed: %s\n", strerror(errno));
		logmsg("things will probably explode now, bye bye!\n");
	}
}

enum IPC ipc_fetch(struct worker* client, void** payload, size_t* payload_size_ptr){
	/* reset pointers */
	if ( payload_size_ptr ) *payload_size_ptr = 0;
	if ( payload ) *payload = NULL;

	/* read command */
	enum IPC command;
	if ( read_wrapper(client->pipe[READ_FD], &command, sizeof(command)) == NULL ){
		logmsg("ipc_fetch[command] - read() failed: %s\n", strerror(errno));
		return IPC_NONE;
	}

	/* read payload size */
	size_t payload_size;
	if ( read_wrapper(client->pipe[READ_FD], &payload_size, sizeof(size_t)) == NULL ){
		logmsg("ipc_fetch[payload_size] - read() failed: %s\n", strerror(errno));
		return IPC_NONE;
	}

	if ( payload_size_ptr ){
		*payload_size_ptr = payload_size;
	}

	/* allocate payload buffer */
	if ( payload && payload_size > 0 ){
		*payload = malloc(payload_size);
	}

	ipc_fetch_payload(client, payload ? *payload : NULL, payload_size);

	switch ( command ){
	case IPC_SHUTDOWN:
		client->running = 0;
		return IPC_HANDLED;
		break;

	case IPC_TESTING:
	case IPC_REFRESH:
		/* pass to caller */
		break;

	default:
		logmsg("Unknown IPC command %d ignored.\n", command);
		return IPC_NONE;
	}

	return command;
}

void ipc_push(struct worker* thread, enum IPC command, const void* payload, size_t payload_size){
	if ( !thread ) return;

	if ( payload_size > max_payload_size ){
		logmsg("too large ipc payload, ignored\n");
		return;
	}

	/* write command */
	if ( write(thread->pipe[WRITE_FD], &command, sizeof(command)) == -1 ){
		logmsg("ipc_push - write() failed: %s\n", strerror(errno));
	}

	/* write payload size */
	if ( write(thread->pipe[WRITE_FD], &payload_size, sizeof(size_t)) == -1 ){
		logmsg("ipc_push - write() failed: %s\n", strerror(errno));
	}

	/* write payload data */
	const char* ptr = (const char*)payload;
	while ( payload_size > 0 ){
		ssize_t bytes = write(thread->pipe[WRITE_FD], ptr, payload_size);
		if ( bytes == -1 ){
			logmsg("ipc_push - write() failed: %s\n", strerror(errno));
			logmsg("things will probably explode now, bye bye!\n");
			return;
		}

		/* advance in src buffer */
		payload_size -= bytes;
		ptr += bytes;
	}
}

const char* ipc_name(enum IPC command){
	switch ( command ){
	case IPC_NONE: return "<none>";
	case IPC_TESTING: return "<testing>";
	case IPC_REFRESH: return "<REFRESH>";
	case IPC_SHUTDOWN: return "<SHUTDOWN>";
	}
	return "<invalid>";
}
