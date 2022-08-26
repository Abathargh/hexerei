#ifndef LIST_HEADER_H
#define LIST_HEADER_H

#include <stdlib.h>

#define DEFAULT_CAP 8

#define LIST_FUNC_NAME(name) _LIST_FUNC_NAME(LIST_PREFIX,name)
#define _LIST_FUNC_NAME(pre, word) __LIST_FUNC_NAME(pre, word)
#define __LIST_FUNC_NAME(pre, word) pre##word

typedef enum list_err_e {
	LIST_NO_ERR,
	LIST_OUT_OF_MEM_ERR
} list_err_e;

#endif

#ifndef LIST_T
#error "LIST_T must be defined"
#endif

#ifndef LIST_NAME
#error "LIST_NAME must be defined"
#else
#define LIST_NAME_TYPE_S(n) n ## _t
#define LIST_NAME_TYPE_F(n) LIST_NAME_TYPE_S(n)
#define LIST_NAME_TYPE LIST_NAME_TYPE_F(LIST_NAME)
#endif

#define LIST_PREFIX _LIST_FUNC_NAME(LIST_NAME, _)


typedef struct LIST_NAME_TYPE {
    LIST_T *items;
    size_t length;
#ifndef LIST_NO_ALLOC
    size_t capacity;
#endif
} LIST_NAME_TYPE;

#define LIST_append LIST_FUNC_NAME(append)
#define LIST_init LIST_FUNC_NAME(init)
#define LIST_free LIST_FUNC_NAME(free)

#ifndef LIST_NO_ALLOC
static inline
LIST_NAME_TYPE *LIST_init(void)
{
	LIST_NAME_TYPE *l = malloc(sizeof(LIST_NAME_TYPE));
	if(l == NULL) return NULL;

	LIST_T* items = malloc(sizeof(LIST_T) * DEFAULT_CAP);
	if(items == NULL) {	free(l); return NULL; }

	l->length = 0;
	l->capacity = DEFAULT_CAP;
	l->items = items;
	return l;
}

static inline
void LIST_free(LIST_NAME_TYPE *list)
{
	if(list == NULL) return;
#ifdef LIST_POINTER_TYPE
	for(size_t i = 0; i < list->length; i++)
		free(list->items[i]);
#endif
	free(list->items);
	free(list);
	list = NULL;
}
#endif

static inline
list_err_e LIST_append(LIST_NAME_TYPE *list, LIST_T item)
{
#ifndef LIST_NO_ALLOC
	if(list->length == list->capacity) {
		LIST_T *nl = realloc(list->items, sizeof(LIST_T) * list->capacity*2);
		if(nl == NULL) return LIST_OUT_OF_MEM_ERR;
		list->items = nl;
		list->capacity *= 2;
	}
#endif
	list->items[list->length++] = item;
	return LIST_NO_ERR;
}



#undef LIST_T
#undef LIST_NAME
#undef LIST_NAME_TYPE
#undef LIST_init
#undef LIST_append
#undef LIST_free
#ifdef LIST_POINTER_TYPE
#undef LIST_POINTER_TYPE
#endif
#ifdef LIST_NO_ALLOC
#undef LIST_NO_ALLOC
#endif
