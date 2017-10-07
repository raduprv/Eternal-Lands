#ifndef __BAGS_H__
#define __BAGS_H__

#include <SDL_types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	int pos;
	int image_id;
	int quantity;
	Uint16 id; //server id
} ground_item;

/*!
 * \name windows handlers
 */
/*! @{ */
extern int ground_items_win; /*!< ground items windows handler */
extern int ground_items_menu_x; /*!< ground items windows x position */
extern int ground_items_menu_y; /*!< ground items windows y position */
extern int ground_items_visible_grid_cols; /*!< ground items window number of columns visible */
extern int ground_items_visible_grid_rows; /*!< ground items window number of rows visible */
extern int view_ground_items; /*!< flag that indicates whether we should display ground items or not */
extern int items_auto_get_all; /*!< if set, bags opened from the invenriy window butto will be automatically picked up */
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
void remove_all_bags(void);

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
 * \brief   Handle the inventroy Get All button.
 *
 *      Handle the inventroy Get All button.
 *
 * \param x ground x position
 * \param y ground y position
 *
*/
void items_get_bag(int x, int y);

/*!
 * \ingroup item
 * \brief   Common funciton for button strings.
 *
 *      Common funciton for buttons to split a six character string into two lines.
 *
 * \param in input string
 * \param out output string
 *
 */
void strap_word(char * in, char * out);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
