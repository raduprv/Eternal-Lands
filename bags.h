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
extern int ground_items_visible_grid_cols; /*!< ground items window number of columns visible */
extern int ground_items_visible_grid_rows; /*!< ground items window number of rows visible */
extern int view_ground_items; /*!< flag that indicates whether we should display ground items or not */
extern int items_auto_get_all; /*!< if set, bags opened from the invenriy window butto will be automatically picked up */
/*! @} */

/*!
 * \ingroup item
 * \brief Remove objects from a bag
 *
 * Removes all items at position \a pos in the currently opened bag, from the bag.
 *
 * \param pos   the position from which the items are removed
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
 * \param bag_id    identifier for the bag
 *
 * \callgraph
 */
void put_bag_on_ground(int bag_x,int bag_y,int bag_id);

/*!
 * \ingroup item
 * \brief   Adds the bags given in \a data.
 *
 * Adds the bags that are given in \a data.
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
 * \brief   Remove a bag
 *
 * Remove the bag with identifier \a which_bag from the list of known bags.
 *
 * \param which_bag the identifier for the bag to remove
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
 * \brief Inspect the contents of a bag.
 *
 * Open a bag, by sending an \ref INSPECT_BAG message for the given \a object_id
 * to the server.
 *
 * \param object_id the object id of the bag to open
 * \note The \a object_id parameter is a 3D object identifier obtained from handling
 * a mouse click in the game window, and is different from the bag identifiers
 * used by e.g. \ref put_bag_on_ground() or \ref remove_bag().
*/
void open_bag(int object_id);

/*!
 * \ingroup item
 * \brief   The server has asked to close the bag.
 *
 * \callgraph
 */
void server_close_bag(void);

/*!
 * \ingroup item
 * \brief   The client is closing the bag.
 *
 * \callgraph
 */
void client_close_bag(void);

/*!
 * \ingroup item
 * \brief   Handle the inventory Get All button.
 *
 *      Handle the inventory Get All button.
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

/*!
 * \ingroup item
 * \brief   Open close clicked bag .
 *
 * \param tile_x the x coord of the clicked tile
 * \param tile_y the y coord of the clicked tile
 * \param max_distance the maximum distance between the clicked coord and the bag
 *
 * \callgraph
 * \retval int 0 for off, non-zero on
 *
 */
int find_and_open_closest_bag(int tile_x, int tile_y, float max_distance);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
