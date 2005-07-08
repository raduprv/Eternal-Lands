#ifdef NETWORK_THREAD

#ifndef QUEUE_H_
#define QUEUE_H_
#include <stddef.h>
#include <SDL_types.h>
#include <SDL_thread.h>

//Move to multiplayer.h?
typedef struct message
{
	int length;
	unsigned char *data;
} message_t;

typedef struct node
{
	void *data;
	struct node *next;	/* Pointer to the next node */
} node_t;

typedef struct queue
{
	node_t *front;	/* Front of the queue */
	node_t *rear;	/* Back of the queue */
	SDL_mutex *mutex; /* Mutex */
	int elements; /* Element counter */
} queue_t;

int queue_initialise (queue_t **queue);
int queue_destroy (queue_t *queue);
int queue_push (queue_t *queue, void *item);
void *queue_pop (queue_t *queue);
int queue_isempty (const queue_t *const queue);
void *queue_peek (const queue_t *const queue);

#endif //QUEUE_H_
#endif //NETWORK_THREAD
