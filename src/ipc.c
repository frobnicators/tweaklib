#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ipc.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

static int min(int a, int b){
	return (a<b) ? a : b;
}

static void ipc_fetch_payload(struct worker* client, void* dst, size_t payload_size){
	char buf[64];

	while ( payload_size > 0 ){
		ssize_t bytes = read(client->pipe[READ_FD], dst ? dst : buf, min(sizeof(buf), payload_size));
		if ( bytes == -1 ){
			logmsg("ipc_flush - read() failed: %s\n", strerror(errno));
			logmsg("things will probably explode now, bye bye!\n");
			return;
		} else if ( bytes == 0 ){
			logmsg("ipc_flush - read() no more data\n");
			logmsg("things will probably explode now, bye bye!\n");
			return;
		}

		/* advance in dst buffer */
		payload_size -= bytes;
		if ( dst ){
			dst += bytes;
		}
	}
}

enum IPC ipc_fetch(struct worker* client, void** payload, size_t* payload_size_ptr){
	/* reset pointers */
	if ( payload_size_ptr ) *payload_size_ptr = 0;
	if ( payload ) *payload = NULL;

	/* read command */
	enum IPC command;
	if ( read(client->pipe[READ_FD], &command, sizeof(command)) == -1 ){
		logmsg("ipc_fetch - read() failed: %s\n", strerror(errno));
		return IPC_NONE;
	}

	/* read payload size */
	size_t payload_size;
	if ( read(client->pipe[READ_FD], &payload_size, sizeof(size_t)) == -1 ){
		logmsg("ipc_fetch - read() failed: %s\n", strerror(errno));
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
