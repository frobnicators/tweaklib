#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tweak/tweak.h"
#include "server.h"
#include "log.h"

void tweak_init(int port, const char* addr){
	server_init(port, addr ? addr : "127.0.0.1");
}

void tweak_init_args(int port, const char* addr, int argc, char* argv[]){

}

void tweak_cleanup(){

}

void tweak_output(tweak_output_func callback){
	log_output(callback);
}

tweak_handle tweak_int(const char* name, int* ptr){
	return 0;
}
