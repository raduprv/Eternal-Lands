#ifndef LIST_H__
#define LIST_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct list_node {
	struct list_node *next;
	struct list_node *prev;
	void *data;
} list_node_t;

typedef void (*list_free_func_t)(void*);

#define list_for_each_node( node, list ) \
    for ( node = list; node; node=node->next )

list_node_t *list_push(list_node_t **head, void *data);
void *list_pop(list_node_t **head);
void list_destroy(list_node_t *head);
/*!
 * \ingroup misc
 * \brief Destroy a linked list with a custom function.
 *
 * Destroy the list, using a custom function to free the data.
 * The list node data will be passed to the free function for each node on the list
 *
 * \param head The list doublepointer.
 * \param free_function The function to be called to free data
 * \returns nothing
 */
void list_destroy_with_func(list_node_t *head, list_free_func_t free_function);
/*!
 * \ingroup misc
 * \brief Remove a node from a linked list
 *
 * Remove the specified node from the linked list, *without* freeing the data.
 * If you wish to free node data, use list_remove_node_and_free_data() instead.
 *
 * \param head The list doublepointer.
 * \param node The node we wish to remove
 * \returns nothing
 */
void list_remove_node(list_node_t **head, list_node_t *node);
/*!
 * \ingroup misc
 * \brief Remove a node from a linked list and free associated data
 *
 * Remove the specified node from the linked list, and free the data using
 * the specified function.
 *
 * \param head The list doublepointer.
 * \param node The node we wish to remove
 * \param free_function The function to be called to free data.
 * \returns nothing
 */

void list_remove_node_and_free_data(list_node_t **head, list_node_t *node, list_free_func_t free_function);

/*!
 * \ingroup misc
 * \brief Append to a linked list
 *
 * Append data to a linked list
 *
 * \param head The list doublepointer.
 * \param data The data you wish to append
 * \returns The node appended
 */
list_node_t *list_append(list_node_t **head, void *data);

#ifdef __cplusplus
} // extern "C"
#endif

#endif //LIST_H__
