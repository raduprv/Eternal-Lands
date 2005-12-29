#ifndef LIST_H__
#define LIST_H__

#include <stddef.h>

typedef struct list_node {
	struct list_node *next;
	struct list_node *prev;
	void *data;
} list_node_t;

list_node_t *list_push(list_node_t **head, void *data);
void *list_pop(list_node_t **head);
void list_destroy(list_node_t *head);

#endif //LIST_H__
