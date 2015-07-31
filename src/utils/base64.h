#ifndef TWEAKLIB_UTILS_BASE64_H
#define TWEAKLIB_UTILS_BASE64_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int base64encode(const void* src, size_t bytes, char* dst, size_t bufsize);

#ifdef __cplusplus
}
#endif

#endif /* TWEAKLIB_UTILS_BASE64_H */
