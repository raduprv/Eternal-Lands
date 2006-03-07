#include <stdlib.h>
#include <stdio.h>
#include <SDL_thread.h>
#include "queue.h"
#include "elmemory.h"

int queue_initialise (queue_t **queue)
{
	(*queue) = malloc(sizeof(queue_t));
	/* Create a dummy node that's always at the front of our queue */
	if (((*queue)->front = malloc(sizeof(node_t))) == NULL) {
		fprintf(stderr, "%s:%i: Failed to allocate memory\n", __FILE__, __LINE__);
		return 0;
	}
	(*queue)->front->data = NULL;
	(*queue)->rear = (*queue)->front;
	(*queue)->front->next = NULL;
	(*queue)->mutex = SDL_CreateMutex();
	(*queue)->nodes = 0;
	return 1;
}

int queue_push (queue_t *queue, void *item)
{
	node_t *newnode;

	if(queue == NULL || (newnode = malloc(sizeof *newnode)) == NULL) {
		fprintf(stderr, "%s:%i: Failed to allocate memory\n", __FILE__, __LINE__);
		return 0;
	} else {
		newnode->data = item;
		newnode->next = NULL;
		/* Add to the end of the queue */
		SDL_LockMutex(queue->mutex);
		queue->rear->next = newnode;
		queue->rear = newnode;
		queue->nodes++;
		SDL_UnlockMutex(queue->mutex);
		return 1;
	}
}

void *queue_pop (queue_t *queue)
{
	node_t *oldnode;
	void *item;

	if (queue == NULL || queue_isempty(queue)) {
		return NULL;
	} else {
		SDL_LockMutex(queue->mutex);
		oldnode = queue->front->next;
		item = oldnode->data;
		/* Check if removing the last node from the queue */
		if (queue->front->next->next == NULL) {
			queue->rear = queue->front;
		} else {
			queue->front->next = queue->front->next->next;
		}
		free(oldnode);
		queue->nodes--;
		SDL_UnlockMutex(queue->mutex);
		return item;
	}
}

void *queue_delete_node(queue_t *queue, node_t *node)
{
	if (queue == NULL || node == NULL || queue_isempty(queue)) {
		return NULL;
	} else {
		void *data = NULL;
		node_t *search_node;

		SDL_LockMutex(queue->mutex);
		search_node = queue->front;
		/* Find the node in the queue */
		/* Check if it's the first node. */
		if(node == queue->front) {
			/* Shouldn't really happen */
			return NULL;
		} else {
			while(search_node != NULL) {
				/* Check if the next node is what we're looking for */
				if(search_node->next == node) {
					/* Point the  node before our node's  next pointer to the node after our node  */
					search_node->next = node->next;
					if(node == queue->rear) {
						queue->rear = search_node;
					}
					/* Make sure the data isn't lost when we free the node */
					data = node->data;
					free(node);
					queue->nodes--;
					break;
				}
				search_node = search_node->next;
			}
		}
		SDL_UnlockMutex(queue->mutex);
		return data;
	}
}

int queue_isempty(const queue_t *queue)
{
	int return_value;

	if (queue == NULL) {
		return_value = 1;
	} else {
		SDL_LockMutex(queue->mutex);
		return_value = (queue->front == queue->rear);
		SDL_UnlockMutex(queue->mutex);
	}
	return return_value;
}

node_t *queue_front_node(const queue_t *queue)
{
	if(queue != NULL) {
		return queue->front->next;
	} else {
		return NULL;
	}
}

/* Unused, should be removed
void *queue_peek(const queue_t *queue)
{
	if (queue == NULL || queue_isempty(queue)) {
		return NULL;
	} else {
		return queue->front->next->data;
	}
}*/

void queue_destroy (queue_t *queue)
{
	void *tmp;

	if(queue != NULL) {
		SDL_LockMutex(queue->mutex);
		while (!queue_isempty (queue)) {
			if ((tmp = queue_pop(queue)) == NULL) {
				break;
			} else {
				free(tmp);
			}
		}
		/* Free the dummy node too */
		free(queue->front);
		SDL_UnlockMutex(queue->mutex);
		SDL_DestroyMutex(queue->mutex);
		free (queue);
	}
}
