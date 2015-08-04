#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tweak/tweak.h"
#include "server.h"
#include "list.h"
#include "log.h"
#include "vars.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <json.h>

list_t vars = NULL;
static unsigned int var_index = 0;
static unsigned int* var_table = NULL;
static unsigned int var_table_size = 0;
static const unsigned int var_invalid = UINT_MAX;
static pthread_mutex_t tweak_mutex = PTHREAD_MUTEX_INITIALIZER;

static void var_free(struct var* var){
	if ( var->ownership ){
		free(var->ptr);
	}
	free(var->name);
	free(var->description);
	free(var->options);
	free(var);
}

void tweak_init(int port, const char* addr){
	vars = list_alloc(sizeof(struct var), 100);
	list_destructor(vars, (list_destructor_callback)var_free);

	server_init(port, addr ? addr : "127.0.0.1");
}

void tweak_init_args(int port, const char* addr, int argc, char* argv[]){

}

void tweak_cleanup(){
	server_cleanup();
	list_free(vars);

	free(var_table);
	var_table = NULL;
	var_table_size = 0;
}

void tweak_output(tweak_output_func callback){
	log_output(callback);
}

void default_trigger(tweak_handle handle){
	/* do nothing */
}

void tweak_trigger(tweak_handle handle, tweak_callback callback){
	struct var* var = var_from_handle(handle);
	if ( var ){
		var->update = callback ? callback : default_trigger;
	}
}

void tweak_description(tweak_handle handle, const char* description){
	struct var* var = var_from_handle(handle);
	if ( var ){
		free(var->description);
		var->description = strdup(description);
	}
}

void tweak_options(tweak_handle handle, const char* data){
	struct var* var = var_from_handle(handle);
	if ( var ){
		free(var->options);
		var->options = NULL;

		struct json_object* json = json_tokener_parse(data);
		if ( !json ){
			logmsg("Failed to parse options\n");
			return;
		}
		json_object_put(json);

		var->options = strdup(data);
	}
}

const char* tweak_get_name(tweak_handle handle){
	struct var* var = var_from_handle(handle);
	return var ? var->name : NULL;
}

void tweak_lock(){
	pthread_mutex_lock(&tweak_mutex);
}

void tweak_unlock(){
	pthread_mutex_unlock(&tweak_mutex);
}

void tweak_refresh(){
	const size_t n = list_size(vars);
	const size_t bytes = sizeof(void*) * n;
	struct var** set = malloc(bytes);
	size_t i;
	void** it;

	/* fill set with all variables */
	for ( i = 0, it = list_begin(vars); it != list_end(vars); i++, it++ ){
		set[i] = *(struct var**)it;
	}

	server_refresh(set, bytes);
	free(set);
}

void tweak_refresh_vars(tweak_set begin, size_t size){
	const size_t n = size / sizeof(tweak_handle);
	const size_t bytes = sizeof(void*) * n;
	struct var** set = malloc(bytes);
	size_t i;

	/* fill set with selected variables */
	tweak_handle* cur = begin;
	tweak_handle* end = begin + n;
	for ( i = 0; cur < end; i++, cur++ ){
		set[i] = var_from_handle(*cur);
	}

	server_refresh(set, bytes);
	free(set);
}

tweak_handle var_add(struct var* var){
	int index = list_push(vars, var);

	if ( var_index >= var_table_size ){
		var_table_size += 100;
		var_table = realloc(var_table, sizeof(unsigned int) * var_table_size + 100);
	}

	var_table[var_index++] = index;

	/* varindex is now +1 so user will see 1 as the first index (on purpose) */
	var->handle = var_index;
	return var_index;
}

struct var* var_from_handle(tweak_handle handle){
	if ( handle == 0 ) return NULL;

	const unsigned int index = var_table[handle - 1];
	if ( index != var_invalid ){
		return (struct var*)list_get(vars, index);
	} else {
		return NULL;
	}
}

struct var* var_create(const char* name, size_t size, void* ptr, datatype_t datatype){
	struct var* var = malloc(sizeof(struct var));
	var->name =  strdup(name);
	var->description = NULL;
	var->options = NULL;
	var->size = size;
	var->ptr = ptr;
	var->ownership = 0;
	var->datatype = datatype;
	var->update = default_trigger;
	return var;
}
