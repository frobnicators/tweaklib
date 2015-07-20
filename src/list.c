#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "list.h"
#include <stdlib.h>

static const size_t list_grow = 25;

struct list {
	void** storage;                            /* raw storage */
	size_t element_size;                       /* size of one element */
	size_t num_allocated;                      /* number of elements allocated */
	size_t num_elements;                       /* number of elements used */
	list_destructor_callback destructor;       /* custom element destructor */
};

static void list_realloc(list_t list, size_t n){
	list->storage = realloc(list->storage, list->element_size * n);
	list->num_allocated = n;
}

list_t list_alloc(size_t element_size, size_t prealloc){
	list_t list = malloc(sizeof(struct list));
	list->storage = NULL;
	list->element_size = element_size;
	list->num_allocated = 0;
	list->num_elements = 0;
	list->destructor = NULL;
	if ( prealloc > 0 ){
		list_realloc(list, prealloc);
	}
	return list;
}

void list_destructor(list_t list, list_destructor_callback callback){
	list->destructor = callback;
}

void list_free(list_t list){
	while ( list_size(list) > 0 ){
		list_erase(list, 0);
	}
	free(list->storage);
	free(list);
}

int list_push(list_t list, void* elem){
	if ( list->num_elements >= list->num_allocated ){
		list->num_allocated += list_grow;
		list_realloc(list, sizeof(void*) * list->num_allocated);
	}

	const int index = list->num_elements;
	list->storage[index] = elem;
	list->num_elements++;

	return index;
}

int list_erase(list_t list, int index){
	const int last_index = list->num_elements - 1;
	void* last = list->storage[last_index];
	void* elem = list->storage[index];

	if ( list->destructor ){
		list->destructor(elem);
	} else {
		free(elem);
	}
	list->storage[index] = last;
	list->num_elements--;

	return last_index;
}

void* list_get(list_t list, int index){
	return list->storage[index];
}

void** list_begin(list_t list){
	return &list->storage[0];
}

void** list_end(list_t list){
	return &list->storage[list->num_elements];

}

size_t list_size(list_t list){
	return list->num_elements;
}
