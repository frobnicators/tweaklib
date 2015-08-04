#ifndef TWEAKLIB_INT_IPC_H
#define TWEAKLIB_INT_IPC_H

#include "worker.h"

enum IPC {
	IPC_NONE = 0,                 /* no command (e.g. read error) */
	IPC_HANDLED = IPC_NONE,       /* command already handled internally */

	/* common */
	IPC_SHUTDOWN = 1,             /* request for shutdown */

	/* websocket */
	IPC_REFRESH = 128,            /* send updated variables to client */
};


/**
 * Fetch IPC command and try to handle it. If return value is non-zero caller should handle it.
 *
 * If payload is non-null and the command has payload attached payload and
 * payload_size will contain the additional data.
 */
enum IPC ipc_fetch(struct worker* client, void** payload, size_t* payload_size);

/**
 * Send IPC command to worker.
 */
void ipc_push(struct worker* thread, enum IPC command, const void* payload, size_t payload_size);

/**
 * Convert IPC enum to string.
 */
const char* ipc_name(enum IPC command);

#endif /* TWEAKLIB_INT_IPC_H */
