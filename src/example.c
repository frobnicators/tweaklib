#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tweak/tweak.h"

#include <stdio.h>
#include <unistd.h>

static int foo = 0;

void output(const char* msg){
	fprintf(stderr, "tweaklib: %s", msg);
}

int main(int argc, const char* argv[]){
	tweak_output(output);
	tweak_init(8080, 0);
	tweak_int("foo", &foo);

	for (;;){
		printf("foo: %d\n", foo);
		sleep(1);
	}

	return 0;
}
