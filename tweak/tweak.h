#ifndef TWEAKLIB_H
#define TWEAKLIB_H

#ifdef TWEAKLIB_EXPORT
#pragma GCC visibility push(default)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int tweak_handle;
typedef void(*tweak_callback)(const char* name, void* ptr);
typedef void(*tweak_output_func)(const char* str);

void tweak_init(int port, const char* addr);

void tweak_init_args(int port, const char* addr, int argc, char* argv[]);

void tweak_cleanup();

/**
 * Callback for writing debug messages.
 */
void tweak_output(tweak_output_func callback);

tweak_handle tweak_int(const char* name, int* ptr);

void tweak_trigger(tweak_handle handle, tweak_callback callback);

void tweak_description(tweak_handle handle, const char* description);

void tweak_options(tweak_handle handle, const char* json);

#ifdef __cplusplus
}
#endif

#ifdef TWEAKLIB_EXPORT
#pragma GCC visibility pop
#endif

#endif /* TWEAKLIB_H */
