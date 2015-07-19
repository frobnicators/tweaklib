#ifndef TWEAKLIB_LIST_H
#define TWEAKLIB_LIST_H

/**
 * Unordered list of items.
 */

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct list* list_t;
typedef void (*list_destructor_callback)(void*);

/**
 * Create a new list.
 */
list_t list_alloc(size_t element_size, size_t prealloc);

/**
 * Assign a custom destructor callback.
 * Called on elements when they are destroyed.
 */
void list_destructor(list_t list, list_destructor_callback callback);

/**
 * Free list (and all elements).
 */
void list_free(list_t list);

/**
 * Push a new element to the end of the list.
 * The list takes ownership of the pointer and will call free() on it when erased.
 */
int list_push(list_t list, void* elem);

/**
 * Erase an element from the list. Will rearrange the order of elements and return the index of the rearranged one.
 */
int list_erase(list_t list, int index);

/**
 * Get element at index.
 */
void* list_get(list_t list, int index);

/**
 * Iteration.
 * for ( void** it = list_begin(list); it != list_end(list); it++ ){ .. }
 */
void** list_begin(list_t list);
void** list_end(list_t list);

/**
 * Tell how many elements is in the list.
 */
size_t list_size(list_t list);

#ifdef __cplusplus
}
#endif

#endif /* TWEAKLIB_LIST_H */
