#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include "elmemory.h"
#include "list.h"

/* Doubly linked list implementation */

list_node_t *list_push(list_node_t **head, void *data)
{
	list_node_t *new_node = malloc(sizeof(*new_node));

	new_node->next = (*head);
	new_node->prev = NULL;
	new_node->data = data;
	if(*head != NULL) {
		(*head)->prev = new_node;
	}
	(*head) = new_node;
	return new_node;
}

void *list_pop(list_node_t **head)
{
	list_node_t *old_node = *head;
	void *data;

	if(*head != NULL) {
		data = (*head)->data;
		*head = (*head)->next;
		if(*head != NULL) {
			(*head)->prev = NULL;
		}
		free(old_node);
	} else {
		data = NULL;
	}

	return data;
}

void list_destroy(list_node_t *head)
{
	void *data;

	while(head != NULL) {
		data = list_pop(&head);
		free(data);
	}
}
