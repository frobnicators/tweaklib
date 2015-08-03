#ifndef TWEAKLIB_H
#define TWEAKLIB_H

#ifdef TWEAKLIB_EXPORT
#pragma GCC visibility push(default)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int tweak_handle;
typedef void(*tweak_callback)(tweak_handle handle);
typedef void(*tweak_output_func)(const char* str);
typedef struct {const char* key; int value;} tweak_enum_value;

void tweak_init(int port, const char* addr);

void tweak_init_args(int port, const char* addr, int argc, char* argv[]);

void tweak_cleanup();

/**
 * Callback for writing debug messages.
 */
void tweak_output(tweak_output_func callback);

/**
 * Make variable tweakable.
 *
 * - POD variants will just overwrite the old value.
 * - Time is the same as float but uses different controls. The speed factor is
 *   a multiplier to allow time to run faster or slower (multiply dt with factor)
 * - String assumes string is allocated with malloc and will swap the pointer
 *   and free the old string. Use with caution as there will be race conditions.
 * - Vector is an array of floats.
 * - Color is same as vector but different controls. Only 3 or 4 components
 *   (RGB or RGBA). For other color formats but an intermediate variable and
 *   a trigger callback to convert to your format.
 * - Enum is similar as int but using a dropdown with a set of key/value-pairs.
 *
 * Use tweak_lock() and tweak_unlock() to prevent race conditions.
 */
tweak_handle tweak_int(const char* name, int* ptr);
tweak_handle tweak_float(const char* name, float* ptr);
tweak_handle tweak_double(const char* name, double* ptr);
tweak_handle tweak_time(const char* name, float* ptr, float* speed);
tweak_handle tweak_string(const char* name, char** ptr);
tweak_handle tweak_vector(const char* name, float*, unsigned int components);
tweak_handle tweak_color(const char* name, float*, unsigned int components);
tweak_handle tweak_enum(const char* name, int* ptr, tweak_enum_value* values, unsigned int n);

void tweak_trigger(tweak_handle handle, tweak_callback callback);

void tweak_description(tweak_handle handle, const char* description);

/**
 * Set custom options for a tweakable variable. The options should be set as
 * valid JSON and what options is allowed is different for each datatype.
 *
 * - "min" (inclusive): for numerical variables it is the lowest value allowed
 *   and for strings it is the least number of characters allowed.
 * - "max" (inclusive): for numerical variables it is the highest value allowed
 *   and for strings it is the largest number of characters allowed.
 * - "step": for numerical variables only, sets the step-size.
 */
void tweak_options(tweak_handle handle, const char* json);

/**
 * Assigns a tweakable variable to a named group. This will display all
 * variables together.
 */
void tweak_set_group(tweak_handle handle, const char* group);

const char* tweak_get_name(tweak_handle handle);

/**
 * Lock tweaklib from performing updates (i.e. mutex). Some datatypes will cause
 * race conditions (such as strings) so locking is needed.
 */
void tweak_lock();
void tweak_unlock();

/**
 * Call this after updating variables (e.g. once per frame) to send updates
 * to clients.
 */
void tweak_refresh();

#ifdef __cplusplus
}
#endif

#ifdef TWEAKLIB_EXPORT
#pragma GCC visibility pop
#endif

#endif /* TWEAKLIB_H */
