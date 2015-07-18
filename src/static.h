#ifndef TWEAKLIB_STATIC_H
#define TWEAKLIB_STATIC_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct file_entry {
	const char* filename;
	const char* mime;
	const char* data;
	size_t bytes;
};

extern struct file_entry file_table[];

#ifdef __cplusplus
}
#endif

#endif /* TWEAKLIB_STATIC_H */
