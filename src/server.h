#ifndef TWEAKLIB_INT_SERVER_H
#define TWEAKLIB_INT_SERVER_H

#include "tweak/tweak.h"
#include "worker.h"
#include <stddef.h>

#define PEER_ADDR_LEN 64

void server_init(int port, const char* addr);
void server_cleanup();
void server_refresh_vars();

const char* peer_addr(int sd, char buf[PEER_ADDR_LEN]);

/**
 * Fetch IPC command and try to handle it. If return value is non-zero caller should handle it.
 */
enum IPC ipc_fetch(struct worker* client);

/**
 * Send IPC command to worker.
 */
void ipc_push(struct worker* thread, enum IPC command);

#endif /* TWEAKLIB_INT_SERVER_H */
