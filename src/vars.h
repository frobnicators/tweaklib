#ifndef TWEAKLIB_VARS_H
#define TWEAKLIB_VARS_H

#include "list.h"
#include "tweak/tweak.h"

struct var;

typedef enum {
	DATATYPE_INTEGER,
} datatype_t;

typedef void(*update_callback)(tweak_handle handle);
typedef struct json_object* (*store_callback)(struct var*);
typedef void (*load_callback)(struct var*, struct json_object*);

struct var {
	tweak_handle handle;
	char* name;
	char* description;
	size_t size;
	void* ptr;
	datatype_t datatype;

	store_callback store;
	load_callback load;
	update_callback update;
};

extern list_t vars;

struct var* var_create(const char* name, size_t size, void* ptr, datatype_t datatype);
tweak_handle var_add(struct var* var);
struct var* var_from_handle(tweak_handle handle);

void default_trigger(tweak_handle handle);

#endif /* TWEAKLIB_VARS_H */
