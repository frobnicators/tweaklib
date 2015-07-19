#ifndef TWEAKLIB_VARS_H
#define TWEAKLIB_VARS_H

#include "list.h"

typedef enum {
	DATATYPE_INTEGER,
} datatype_t;

struct var {
	char* name;
	size_t size;
	void* ptr;
	datatype_t datatype;
};

extern list_t vars;

#endif /* TWEAKLIB_VARS_H */
