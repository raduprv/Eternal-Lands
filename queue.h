#ifndef QUEUE_H_
#define QUEUE_H_
#include <stddef.h>
#include <SDL_types.h>
#include <SDL_thread.h>

#ifdef __cplusplus
extern "C" {
#endif

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
#ifdef	NEW_TEXTURES
	SDL_cond* condition;
#endif	/* NEW_TEXTURES */
	int nodes; /* Node counter */
} queue_t;

int queue_initialise (queue_t **queue);
void queue_destroy (queue_t *queue);
int queue_push (queue_t *queue, void *item);
void *queue_pop (queue_t *queue);
void *queue_delete_node(queue_t *queue, node_t *node);
int queue_isempty (const queue_t *queue);
node_t *queue_front_node(const queue_t *queue);

#ifdef	NEW_TEXTURES
int queue_push_signal(queue_t *queue, void *item);
void *queue_pop_blocking(queue_t *queue);
#endif	/* NEW_TEXTURES */

#ifdef __cplusplus
} // extern "C"
#endif

#endif //QUEUE_H_
