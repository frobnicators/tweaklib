#ifndef TWEAKLIB_WEBSOCKET_H
#define TWEAKLIB_WEBSOCKET_H

#include "worker.h"

#ifdef __cplusplus
extern "C" {
#endif

void websocket_loop(struct worker* client);
const char* websocket_derive_key(const char* key);

#ifdef __cplusplus
}
#endif

#endif /* TWEAKLIB_WEBSOCKET_H */
