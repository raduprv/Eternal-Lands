#ifndef __BAGS_H__
#define __BAGS_H__

#include <SDL_types.h>
#ifdef ONGOING_BAG_EFFECT
#include "eye_candy_types.h"
#endif // ONGOING_BAG_EFFECT


#ifdef __cplusplus
extern "C" {
#endif

#define NUM_BAGS 200
#define ITEMS_PER_BAG 50

typedef struct
{
	int x;
	int y;
	int obj_3d_id;
#ifdef ONGOING_BAG_EFFECT
	ec_reference ongoing_bag_effect_reference;
#endif // ONGOING_BAG_EFFECT
} bag;

typedef struct
{
	int pos;
	int image_id;
	int quantity;
	Uint16 id; //server id
} ground_item;

extern int view_ground_items; /*!< flag that indicates whether we should display ground items or not */

extern int ground_items_menu_x;
extern int ground_items_menu_y;
extern int ground_items_menu_x_len;
extern int ground_items_menu_y_len;
extern Uint32 ground_items_empty_next_bag;

/*!
 * \name windows handlers
 */
/*! @{ */
extern int ground_items_win; /*!< ground items windows handler */
/*! @} */

/*!
 * \ingroup item
 * \brief   Sets the quantity of the \ref ground_item_list at index \a pos to be 0.
 *
 *      Sets the quantity of the \ref ground_item_list at index \a pos to be 0. No sanity checks are performed.
 *
 * \param pos   the index into \ref ground_item_list that should get removed.
 *
 * \note No sanity checks whether \a pos is a valid index are performed. This is possibly a bug.
 * \bug Does not perform sanity checks for the parameter \a pos.
 */
void remove_item_from_ground(Uint8 pos);

/*!
 * \ingroup item
 * \brief   Gets a new item in a bag on the ground.
 *
 *      Gets a new item for a bag on the ground from the given \a data.
 *
 * \param data  the data for the new ground item aka bag
 *
 * \note No sanity checks for \a data are performed. This may be a possible bug.
 * \bug Doesn't perform any sanity checks on the given \a data.
 *
 */
void get_bag_item (const Uint8 *data);

/*!
 * \ingroup item
 * \brief   Gets the contents of a bag.
 *
 *      Gets the contents of a bag on the ground from the given \a data.
 *
 * \param data  the data of the bags items
 *
 * \callgraph
 *
 * \note No sanity checks for \a data are performed. This may be a possible bug.
 * \bug Doesn't perform any sanity checks on the given \a data.
 */
void get_bags_items_list (const Uint8 *data);

/*!
 * \ingroup item
 * \brief   Puts the bag \a bag_id on the ground at coordinates (\a bag_x, \a bag_y).
 *
 *      Puts the bag \a bag_id at the coordinates (\a bag_x, \a bag_y) on the ground.
 *
 * \param bag_x     x coordinate of the bags position
 * \param bag_y     y coordinate of the bags position
 * \param bag_id    index into \ref bag_list to be used for the bag
 *
 * \callgraph
 */
void put_bag_on_ground(int bag_x,int bag_y,int bag_id);

/*!
 * \ingroup item
 * \brief   Adds the bags given in \a data.
 *
 *      Adds the bags that are given in \a data.
 *
 * \param data  the data from the server for the bags to add
 *
 * \callgraph
 *
 * \note No sanity checks on \a data are performed. This may be a possible bug.
 * \bug No sanity checks on \a data are performed.
 */
void add_bags_from_list (const Uint8 *data);

/*!
 * \ingroup item
 * \brief   Removes the bag with the given index \a which_bag from the \ref bag_list.
 *
 *      Removes the bag at the given index \a which_bag from the \ref bag_list. The list of bags will be adjusted accordingly.
 *
 * \param which_bag the index into \ref bag_list for the bag to remove
 *
 * \callgraph
 */
void remove_bag(int which_bag);

/*!
 * \ingroup item
 * \brief   Removes all the bag from a map.
 *
 *      Removes all the bag on a map.
 *
 * \callgraph
 */
void remove_all_bags();

/*!
 * \ingroup item
 * \brief   Sends an \ref INSPECT_BAG message for the given \a object_id to the server.
 *
 *      Sends an \ref INSPECT_BAG message for the given \a object_id to the server.
 *
 * \param object_id the id of the bag to open
 *
 * \note Uses a fixed upper limit to search the \ref bag_list. This may be a possible bug.
 * \bug Uses a fixed upper limit to search \ref bag_list. We should use a defined constant instead.
 */
void open_bag(int object_id);

/*!
 * \ingroup item
 * \brief   Send messages to pick up all items in a ground bag.
 *
 *      Send messages to pick up all items in a ground bag
 */
void pick_up_all_items(void);


#ifdef __cplusplus
} // extern "C"
#endif

#endif
