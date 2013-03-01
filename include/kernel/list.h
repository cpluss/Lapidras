#ifndef LIST_H
#define LIST_H
#include "types.h"

typedef struct node
{
	struct node *next;
	struct node *prev;
	void *value;
} __attribute__((packed)) node_t;

typedef struct list
{
	node_t *head;
	node_t *tail;
	uint length;
} __attribute__((packed)) list_t;


void list_append(list_t *list, node_t *item);
void list_insert(list_t *list, void *item);

list_t *list_create();
void list_destroy(list_t *list);
void list_clean(list_t *list);

node_t *list_find(list_t *list, void *value);
void list_remove(list_t *list, uint index);
void list_delete(list_t *list, node_t *node);

node_t *list_pop(list_t *list);
node_t *list_dequeue(list_t *list);
list_t *list_copy(list_t *orig);
void list_merge(list_t *target, list_t *source);

#define foreach(i, list) node_t *i;for (i = list->head; i != 0; i = i->next)

#endif
