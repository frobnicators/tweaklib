#ifndef TWEAKLIB_STATIC_H
#define TWEAKLIB_STATIC_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct file_entry {
	const char* filename;                           /* filename entry (virtaul path) */
	const char* original;                           /* original filanem (on disk, relative to build directory) */
	const char* mime;                               /* mimetype */
	const char* data;                               /* raw data */
	size_t bytes;                                   /* file size */
};

extern struct file_entry file_table[];

#ifdef __cplusplus
}
#endif

#endif /* TWEAKLIB_STATIC_H */
