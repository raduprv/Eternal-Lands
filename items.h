/*!
 * \file
 * \ingroup item
 * \brief Item handling and storing
 */
#ifndef __ITEMS_H__
#define __ITEMS_H__

/*!
 * Any item in EL has assigned an item struct
 */
typedef struct
{
	int image_id; /*!< id of the image for this item */
	int pos;
	int quantity;
	int is_reagent; /*!< can be used for spells? */
	int is_resource; /*!< does it appear on the manufacturing menu? */
	int use_with_inventory;
} item;

typedef struct
{
	int pos;
	int image_id;
	int quantity;
} ground_item;


/*!
 * \name Item definition flags
 */
/*! @{ */
typedef enum {
	ITEM_REAGENT           = 1, /*!< can be used in magic */
	ITEM_RESOURCE          = 2, /*!< can be used to manufacture */
	ITEM_STACKABLE         = 4, /*!< the item is stackable */
	ITEM_INVENTORY_USABLE  = 8, /*!< item can be used with inventory */
	ITEM_TILE_USABLE       = 16,
	ITEM_PLAYER_USABLE     = 32, /*!< item is usable by players */
	ITEM_OBJECT_USABLE     = 64,
	ITEM_ON_OFF            = 128,
} item_definition_flags;
/*! @} */

/*!
 * \name Item constants
 */
/*! @{ */
#define	ITEM_WEAR_START	36
#define	ITEM_NUM_WEAR	8
#define	ITEM_NUM_ITEMS	(ITEM_WEAR_START+ITEM_NUM_WEAR)
/*! @} */

extern item item_list[ITEM_NUM_ITEMS]; /*!< global list of items */
extern item manufacture_list[ITEM_NUM_ITEMS]; /*!< global list of manufacturable items */

extern int trade_you_accepted; /*!< flag, indicating whether you have accepted the trade or not */
extern int trade_other_accepted; /*!< flag, indicating whether the trade partner has accepted the trade or not */

extern int item_action_mode;

extern int view_ground_items; /*!< flag that indicates whether we should display ground items or not */

/*! \name windows handlers */
/*! @{ */
extern int items_win; /*!< inventory windows handler */
extern int ground_items_win; /*!< ground items windows handler */
/*! @} */

extern int items_menu_x;
extern int items_menu_y;

extern int ground_items_menu_x;
extern int ground_items_menu_y;

extern int manufacture_menu_x;
extern int manufacture_menu_y;
extern int manufacture_menu_x_len;
extern int manufacture_menu_y_len;
//extern int manufacture_menu_dragged; // has been commented before my cleanup. Think it can get removed too.

extern int trade_menu_x;
extern int trade_menu_y;

/*! \name Text fields for items */
/*! @{ */
extern int items_text_1;
extern int items_text_2;
extern int items_text_3;
extern int items_text_4;
extern int items_text_5;
extern int items_text_6;
extern int items_text_7;
extern int items_text_8;
extern int items_text_9;
extern int items_text_10;
/*! @} */

extern int item_dragged;

extern int use_item;

extern char items_string[300];

extern int item_quantity;

/*!
 * \ingroup item
 * \brief Gets the textures associated with the item \a no.
 *
 *      Returns the texture id associated with the item given by \a no.
 *
 * \param no        id of the item
 * \retval GLuint   the texture id associated with \a no.
 */
inline GLuint get_items_texture(int no);

/*!
 * \ingroup items_window
 * \brief   Displays the items (inventory) window.
 *
 *      Displays the items (inventory) window. If the window was not displayed before, it will first created and the event handlers for the window initialized accordingly. \ref items_window recognizes the following events: \ref ELW_HANDLER_DISPLAY, \ref ELW_HANDLER_CLICK and \ref ELW_HANDLER_MOUSEOVER.
 *
 * \callgraph
 */
void display_items_menu();

/*!
 * \ingroup item
 * \brief   Gets the items for \ref item_list from the parameter \a data.
 *
 *      Initializes the \ref item_list from the \a data given. Calls \ref build_manufacture_list after the initializing is done.
 *
 * \param data  the data for the \ref item_list
 *
 * \callgraph
 */
void get_your_items(Uint8 *data);

/*!
 * \ingroup item
 * \brief   Drags the given \a item
 *
 *      Drags the given \a item. If \a mini is true, the dragged item will be drawn smaller.
 *
 * \param item  the index into array of the item being dragged
 * \param storage specifies if it's taken from the storage or the inventory items array
 * \param mini  boolean flag, indicating whether the dragged item will be drawn smaller
 *
 * \callgraph
 */
void drag_item(int item, int storage, int mini);

/*!
 * \ingroup item
 * \brief   Removes the item at the given inventory position \a pos from the items menu.
 *
 *      Removes the item at the given inventory position \a pos from the inventory. Calls \ref build_manufacture_list after the item was removed.
 *
 * \param pos   the position into the items menu
 *
 * \callgraph
 *
 */
void remove_item_from_inventory(int pos);

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
 * \brief   Gets a new item from the given \a data.
 *
 *      Gets a new inventory item from the given \a data. If we already have such an item, only the quantity will get updated. Calls \ref build_manufacture_list after the item has been updated or added.
 *
 * \param data  teh data for the new item
 *
 * \callgraph
 *
 * \note Assumes that \a data is valid and not NULL. This may be a possible bug.
 * \bug Assumes that \a data is valid and not NULL and does not perform any sanity checks.
 */
void get_new_inventory_item(Uint8 *data);

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
void get_bag_item(Uint8 *data);

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
void get_bags_items_list(Uint8 *data);

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
void add_bags_from_list(Uint8 *data);

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
#endif
