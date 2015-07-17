#ifndef TWEAKLIB_INT_LOG_H
#define TWEAKLIB_INT_LOG_H

#include "tweak/tweak.h"

void __attribute__((format(printf,1,2))) logmsg(const char* fmt, ...);
void log_output(tweak_output_func callback);

#endif /* TWEAKLIB_INT_LOG_H */
