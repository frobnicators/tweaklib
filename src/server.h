#ifndef TWEAKLIB_INT_SERVER_H
#define TWEAKLIB_INT_SERVER_H

#include "tweak/tweak.h"
#include "vars.h"
#include "worker.h"
#include <stddef.h>

#define PEER_ADDR_LEN 64

void server_init(int port, const char* addr);
void server_cleanup();

/**
 * Update variables on all clients.
 *
 * @param vars array with all variables to update
 * @param n number of array elements
 */
void server_refresh(struct var* vars[], size_t n);

const char* peer_addr(int sd, char buf[PEER_ADDR_LEN]);

#endif /* TWEAKLIB_INT_SERVER_H */
