#ifndef TWEAKLIB_UTILS_SHA1_H
#define TWEAKLIB_UTILS_SHA1_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sha1* sha1_t;
typedef char sha1_hash[41];

sha1_t sha1_new();
void sha1_reset(sha1_t sha);
void sha1_free(sha1_t sha);
void sha1_update(sha1_t sha, const void* ptr, size_t n);
uint8_t* sha1_hash_bytes(sha1_t sha);
const char* sha1_hash_hex(sha1_t sha, sha1_hash buffer);

#ifdef __cplusplus
}
#endif

#endif /* TWEAKLIB_UTILS_SHA1_H */
