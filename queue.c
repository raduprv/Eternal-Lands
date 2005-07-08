#ifdef NETWORK_THREAD
#include <stdlib.h>
#include <stdio.h>
#include <SDL_thread.h>
#include "queue.h"

int queue_initialise (queue_t **queue)
{
	(*queue) = malloc(sizeof(queue_t));
	if (((*queue)->front = malloc (sizeof(node_t))) == NULL) {
		fprintf(stderr, "%s:%i: Failed to allocate memory\n", __FILE__, __LINE__);
		return 0;
	}
	(*queue)->rear = (*queue)->front;
	(*queue)->front->next = NULL;
	(*queue)->mutex = SDL_CreateMutex();
	(*queue)->elements = 0;
	return 1;
}

int queue_push (queue_t *queue, void *item)
{
	node_t *newnode;

	SDL_LockMutex(queue->mutex);
	if (queue == NULL || (newnode = malloc (sizeof *newnode)) == NULL) {
		fprintf(stderr, "%s:%i: Failed to allocate memory\n", __FILE__, __LINE__);
		SDL_UnlockMutex(queue->mutex);
		return 0;
	} else {
		newnode->data = item;
		newnode->next = NULL;
		/* Add to the end of the queue */
		queue->rear->next = newnode;
		queue->rear = newnode;
		queue->elements++;
		SDL_UnlockMutex(queue->mutex);
		return 1;
	}
}

void *queue_pop (queue_t *queue)
{
	node_t *oldnode;
	void *item;

	SDL_LockMutex(queue->mutex);
	if (queue == NULL || queue_isempty(queue)) {
		SDL_UnlockMutex(queue->mutex);
		return NULL;
	} else {
		oldnode = queue->front->next;
		item = oldnode->data;
		/* Check if removing the last node from the queue */
		if (queue->front->next->next == NULL) {
			queue->rear = queue->front;
		} else {
			queue->front->next = queue->front->next->next;
		}
		free(oldnode);
		queue->elements--;
		SDL_UnlockMutex(queue->mutex);
		return item;
	}
}

int queue_isempty(const queue_t *queue)
{
	int return_value;

	SDL_LockMutex(queue->mutex);
	if (queue == NULL) {
		return_value = 1;
	} else {
		return_value = (queue->front == queue->rear);
	}
	SDL_UnlockMutex(queue->mutex);
	return return_value;
}

void *queue_peek(const queue_t *queue)
{
	if (queue == NULL || queue_isempty(queue)) {
		return NULL;
	} else {
		return queue->front->next->data;
	}
}

int queue_destroy (queue_t *queue)
{
	void *tmp;
	
	SDL_LockMutex(queue->mutex);
	while (!queue_isempty (queue)) {
		if ((tmp = queue_pop(queue)) == NULL) {
			break;
		} else {
			free(tmp);
		}
	}
	free (queue);
	SDL_UnlockMutex(queue->mutex);
	SDL_DestroyMutex(queue->mutex);
	return 0;
}
#endif //NETWORK_THREAD
