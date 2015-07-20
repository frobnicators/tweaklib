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

list_t vars = NULL;

static void var_free(struct var* var){
	free(var->name);
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
}

void tweak_output(tweak_output_func callback){
	log_output(callback);
}

void tweak_trigger(tweak_handle handle, tweak_callback callback){

}

void tweak_description(tweak_handle handle, const char* description){

}

void tweak_options(tweak_handle handle, const char* json){

}

static unsigned int handle_to_index(tweak_handle handle){
	return 0;
}

struct var* var_create(const char* name, size_t size, void* ptr, datatype_t datatype){
	struct var* var = malloc(sizeof(struct var));
	var->name =  strdup(name);
	var->description = NULL;
	var->size = size;
	var->ptr = ptr;
	var->datatype = datatype;
	return var;
}
