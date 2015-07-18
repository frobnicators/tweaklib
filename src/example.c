#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tweak/tweak.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

static int foo = 0;
static int running = 1;

void sighandler(int signum){
	if ( running ){
		running = 0;
		fprintf(stderr, "\rterminating example application\n");
	} else {
		fprintf(stderr, "\rterminating request received twice, aborting\n");
		abort();
	}
}

void output(const char* msg){
	fprintf(stderr, "tweaklib: %s", msg);
}

int main(int argc, const char* argv[]){
	tweak_output(output);
	tweak_init(8080, 0);
	tweak_int("foo", &foo);

	malloc(50);

	signal(SIGINT, sighandler);

	while (running) {
		printf("foo: %d\n", foo);
		sleep(1);
	}

	tweak_cleanup();

	return 0;
}
