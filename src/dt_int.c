#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tweak/tweak.h"
#include "log.h"
#include "vars.h"
#include <string.h>
#include <json.h>

static struct json_object* store_int(struct var* var){
	return json_object_new_int(*(int*)var->ptr);
}

static void load_int(struct var* var, struct json_object* obj){
	if ( !json_object_is_type(obj, json_type_int) ){
		logmsg("variable \"%s\" expected integer value but got %s, update ignored.\n", var->name, json_type_to_name(json_object_get_type(obj)));
	}
	*(int*)var->ptr = json_object_get_int(obj);
}

tweak_handle tweak_int(const char* name, int* ptr){
	struct var* var = var_create(name, sizeof(int), ptr, DATATYPE_INTEGER);
	var->store = store_int;
	var->load = load_int;
	list_push(vars, var);
	return 0;
}
