#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tweak/tweak.h"
#include "log.h"
#include "vars.h"
#include <string.h>
#include <json.h>

static struct json_object* store_float(const struct var* var){
	return json_object_new_double(*(float*)var->ptr);
}

static void load_float(struct var* var, struct json_object* obj){
	const enum json_type type = json_object_get_type(obj);
	if ( !(type == json_type_double || type == json_type_int) ){
		logmsg("variable \"%s\" expected double value but got %s, update ignored.\n", var->name, json_type_to_name(json_object_get_type(obj)));
	}
	*(float*)var->ptr = (float)json_object_get_double(obj);
}

tweak_handle tweak_float(const char* name, float* ptr){
	struct var* var = var_create(name, sizeof(float), ptr, DATATYPE_FLOAT);
	var->store = store_float;
	var->load = load_float;
	return var_add(var);
}
