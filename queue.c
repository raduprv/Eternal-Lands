#include <stdlib.h>
#include <stdio.h>
#include "queue.h"
#include "elmemory.h"
#include "threads.h"
#include "errors.h"

int queue_initialise (queue_t **queue)
{
	(*queue) = malloc(sizeof(queue_t));

	if ((*queue) == 0)
	{
		LOG_ERROR("Failed to allocate memory for queue");

		return 0;
	}

	(*queue)->front = malloc(sizeof(node_t));

	/* Create a dummy node that's always at the front of our queue */
	if ((*queue)->front == 0)
	{
		LOG_ERROR("Failed to allocate memory for queue node");

		return 0;
	}
	(*queue)->front->data = 0;
	(*queue)->rear = (*queue)->front;
	(*queue)->front->next = 0;
	(*queue)->mutex = SDL_CreateMutex();
#ifdef	NEW_TEXTURES
	(*queue)->condition = SDL_CreateCond();
#endif	/* NEW_TEXTURES */
	(*queue)->nodes = 0;
	return 1;
}

int queue_push (queue_t *queue, void *item)
{
	node_t *newnode;

	if (queue == 0)
	{
		LOG_ERROR("Null pointer for queue");

		return 0;
	}

	newnode = malloc(sizeof(node_t));

	if (newnode == 0)
	{
		LOG_ERROR("Failed to allocate memory for queue node");

		return 0;
	}

	newnode->data = item;
	newnode->next = 0;
	/* Add to the end of the queue */
	CHECK_AND_LOCK_MUTEX(queue->mutex);
	queue->rear->next = newnode;
	queue->rear = newnode;
	queue->nodes++;
	CHECK_AND_UNLOCK_MUTEX(queue->mutex);

	return 1;
}

void *queue_pop (queue_t *queue)
{
	node_t *oldnode;
	void *item;

	if (queue == NULL || queue_isempty(queue))
	{
		return NULL;
	}
	else
	{
		CHECK_AND_LOCK_MUTEX(queue->mutex);
		oldnode = queue->front->next;
		item = oldnode->data;
		/* Check if removing the last node from the queue */
		if (queue->front->next->next == NULL)
		{
			queue->rear = queue->front;
		}
		else
		{
			queue->front->next = queue->front->next->next;
		}
		free(oldnode);
		queue->nodes--;
		CHECK_AND_UNLOCK_MUTEX(queue->mutex);
		return item;
	}
}

void *queue_delete_node(queue_t *queue, node_t *node)
{
	if (queue == NULL || node == NULL || queue_isempty(queue))
	{
		return NULL;
	}
	else
	{
		void *data = NULL;
		node_t *search_node;

		CHECK_AND_LOCK_MUTEX(queue->mutex);
		search_node = queue->front;
		/* Find the node in the queue */
		/* Check if it's the first node. */
		if (node == queue->front)
		{
			/* Shouldn't really happen */
			return NULL;
		}
		else
		{
			while(search_node != NULL)
			{
				/* Check if the next node is what we're looking for */
				if (search_node->next == node)
				{
					/* Point the  node before our node's  next pointer to the node after our node  */
					search_node->next = node->next;
					if (node == queue->rear)
					{
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
		CHECK_AND_UNLOCK_MUTEX(queue->mutex);
		return data;
	}
}

int queue_isempty(const queue_t *queue)
{
	int return_value;

	if (queue == NULL)
	{
		return_value = 1;
	}
	else
	{
		CHECK_AND_LOCK_MUTEX(queue->mutex);
		return_value = (queue->front == queue->rear);
		CHECK_AND_UNLOCK_MUTEX(queue->mutex);
	}
	return return_value;
}

node_t *queue_front_node(const queue_t *queue)
{
	if (queue != NULL)
	{
		return queue->front->next;
	}
	else
	{
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

	if (queue != NULL)
	{
		CHECK_AND_LOCK_MUTEX(queue->mutex);
		while (!queue_isempty (queue))
		{
			if ((tmp = queue_pop(queue)) == NULL)
			{
				break;
			}
			else
			{
				free(tmp);
			}
		}
		/* Free the dummy node too */
		free(queue->front);
		CHECK_AND_UNLOCK_MUTEX(queue->mutex);
		SDL_DestroyMutex(queue->mutex);
#ifdef	NEW_TEXTURES
		SDL_DestroyCond(queue->condition);
#endif	/* NEW_TEXTURES */
		free (queue);
	}
}

#ifdef	NEW_TEXTURES
int queue_push_signal(queue_t *queue, void *item)
{
	node_t *node;

	if (queue == 0)
	{
		return 0;
	}

	node = malloc(sizeof(node_t));

	if (node == 0)
	{
		CHECK_AND_UNLOCK_MUTEX(queue->mutex);

		LOG_ERROR("Failed to allocate memory for queue");

		return 0;
	}

	node->data = item;
	node->next = 0;

	/* Add to the end of the queue */
	CHECK_AND_LOCK_MUTEX(queue->mutex);

	queue->rear->next = node;
	queue->rear = node;
	queue->nodes++;

	CHECK_AND_UNLOCK_MUTEX(queue->mutex);

	SDL_CondSignal(queue->condition);

	return 1;
}

void *queue_pop_blocking(queue_t *queue)
{
	node_t *node;
	void *item;

	if (queue == 0)
	{
		return 0;
	}

	CHECK_AND_LOCK_MUTEX(queue->mutex);

	while (queue->front == queue->rear)
	{
		SDL_CondWait(queue->condition, queue->mutex);
		if (queue->front == queue->rear)
		{
			CHECK_AND_UNLOCK_MUTEX(queue->mutex);
			return 0;
		}
	}

	node = queue->front->next;
	item = node->data;
	/* Check if removing the last node from the queue */
	if (queue->front->next->next == 0)
	{
		queue->rear = queue->front;
	}
	else
	{
		queue->front->next = queue->front->next->next;
	}

	free(node);
	queue->nodes--;
	CHECK_AND_UNLOCK_MUTEX(queue->mutex);

	return item;
}

#endif	/* NEW_TEXTURES */

