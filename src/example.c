#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tweak/tweak.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

static int foo = 7;
static float bar = 12;
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

static void output(const char* msg){
	fprintf(stderr, "tweaklib: %s", msg);
}

static void update(tweak_handle handle){
	fprintf(stderr, "The variable \"%s\" (%d) was updated.\n", tweak_get_name(handle), handle);
}

int main(int argc, const char* argv[]){
	tweak_output(output);
	tweak_init(8080, 0);

	/* just a plain variable */
	tweak_int("foo", &foo);

	/* extra properties */
	tweak_handle tl_bar = tweak_float("bar", &bar);
	tweak_description(tl_bar, "Just some dummy value");
	tweak_trigger(tl_bar, update);
	tweak_options(tl_bar, "{\"min\": 5, \"max\": 35, \"step\": 0.1}"); /* json */

	signal(SIGINT, sighandler);

	while (running) {
		printf("foo: %d bar: %.1f\n", foo, bar);
		sleep(1);
	}

	tweak_cleanup();

	return 0;
}
