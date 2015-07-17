#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"

#include <stdio.h>
#include <stdarg.h>

static tweak_output_func output = 0;
static char buffer[4096];

void logmsg(const char* fmt, ...){
	if ( !output ) return;

	/** @todo lock output, e.g mutex */

	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	output(buffer);
}

void log_output(tweak_output_func callback){
	output = callback;
}
