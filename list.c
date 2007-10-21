#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include "elmemory.h"
#include "list.h"

/* Doubly linked list implementation */

list_node_t *list_push(list_node_t **head, void *data)
{
	list_node_t *new_node = (list_node_t *)malloc(sizeof(*new_node));

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
	list_destroy_with_func( head, &free );
}

void list_destroy_with_func(list_node_t *head, list_free_func_t free_func)
{
	while(head != NULL) {
		free_func(list_pop(&head));
	}
}

void list_remove_node(list_node_t **head, list_node_t *node)
{
	if (node->next) {
		node->next->prev = node->prev;
	}
	if (node->prev) {
		node->prev->next = node->next;
	} else {
		*head = node->next;
	}

    free(node);
}
void list_remove_node_and_free_data(list_node_t **head, list_node_t *node, list_free_func_t free_function)
{
	free_function(node->data);
	list_remove_node(head, node);
}

list_node_t *list_append(list_node_t **head, void *data)
{
	list_node_t *last_node = *head;
	list_node_t *new_node = (list_node_t *)malloc(sizeof(*new_node));

	while (last_node && last_node->next) {
		last_node = last_node->next;
	}

	new_node->next = NULL;
	new_node->prev = last_node;
	new_node->data = data;
	if (*head == NULL) {
		(*head) = new_node;
	} else {
        last_node->next = new_node;
	}
	return new_node;
}
