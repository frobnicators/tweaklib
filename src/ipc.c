#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ipc.h"
#include "log.h"
#include <string.h>
#include <errno.h>
#include <unistd.h>

enum IPC ipc_fetch(struct worker* client){
	enum IPC command;
	if ( read(client->pipe[READ_FD], &command, sizeof(command)) == -1 ){
		logmsg("read() failed: %s\n", strerror(errno));
		return IPC_NONE;
	}

	switch ( command ){
	case IPC_SHUTDOWN:
		client->running = 0;
		return IPC_HANDLED;
		break;

	default:
		logmsg("Unknown IPC command %d ignored.\n", command);
		return IPC_NONE;
	}

	return command;
}

void ipc_push(struct worker* thread, enum IPC command){
	if ( !thread ) return;
	if ( write(thread->pipe[WRITE_FD], &command, sizeof(command)) == -1 ){
		logmsg("ipc_command - write() failed: %s\n", strerror(errno));
	}
}
