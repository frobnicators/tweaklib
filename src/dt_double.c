#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tweak/tweak.h"
#include "log.h"
#include "vars.h"
#include <string.h>
#include <json.h>

static struct json_object* store_double(struct var* var){
	return json_object_new_double(*(double*)var->ptr);
}

static void load_double(struct var* var, struct json_object* obj){
	const enum json_type type = json_object_get_type(obj);
	if ( !(type == json_type_double || type == json_type_int) ){
		logmsg("variable \"%s\" expected double value but got %s, update ignored.\n", var->name, json_type_to_name(json_object_get_type(obj)));
	}
	*(double*)var->ptr = json_object_get_double(obj);
}

tweak_handle tweak_double(const char* name, double* ptr){
	struct var* var = var_create(name, sizeof(double), ptr, DATATYPE_DOUBLE);
	var->store = store_double;
	var->load = load_double;
	return var_add(var);
}
