#ifndef TWEAKLIB_VARS_H
#define TWEAKLIB_VARS_H

#include "list.h"
#include "tweak/tweak.h"

typedef enum {
	DATATYPE_INTEGER,
} datatype_t;

struct var {
	tweak_handle handle;
	char* name;
	char* description;
	size_t size;
	void* ptr;
	datatype_t datatype;

	struct json_object* (*store)(struct var*);
	void (*load)(struct var*, struct json_object*);
};

extern list_t vars;

struct var* var_create(const char* name, size_t size, void* ptr, datatype_t datatype);

#endif /* TWEAKLIB_VARS_H */
