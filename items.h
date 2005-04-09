/*!
 * \file
 * \ingroup item
 * \brief item handling and storing
 */
#ifndef __ITEMS_H__
#define __ITEMS_H__

/*!
 * any item in EL has assigned an item struct
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

/*!
 * \name windows handlers
 */
/*! @{ */
extern int items_win; /*!< inventory windows handler */
/*! @} */

extern int items_menu_x;
extern int items_menu_y;

/*!
 * \name windows handlers
 */
/*! @{ */
extern int ground_items_win; /*!< ground items windows handler */
/*! @} */

extern int ground_items_menu_x;
extern int ground_items_menu_y;

extern int manufacture_menu_x;
extern int manufacture_menu_y;
extern int manufacture_menu_x_len;
extern int manufacture_menu_y_len;
//extern int manufacture_menu_dragged; // has been commented before my cleanup. Think it can get removed too.

extern int trade_menu_x;
extern int trade_menu_y;

/*! \name text fields for items 
 * @{ */
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
 * \ingroup items_win
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void display_items_menu();

/*!
 * \ingroup item
 * \brief
 *
 *      Detail
 *
 * \param data
 *
 * \callgraph
 */
void get_your_items(Uint8 *data);

/*!
 * \ingroup item
 * \brief
 *
 *      Detail
 *
 * \param item
 * \param mini
 *
 * \callgraph
 */
void drag_item(int item, int mini);

/*!
 * \ingroup item
 * \brief
 *
 *      Detail
 *
 * \param pos
 *
 * \callgraph
 */
void remove_item_from_inventory(int pos);

/*!
 * \ingroup item
 * \brief
 *
 *      Detail
 *
 * \param pos
 *
 * \sa process_message_from_server
 */
void remove_item_from_ground(Uint8 pos);

/*!
 * \ingroup item
 * \brief
 *
 *      Detail
 *
 * \param data
 *
 * \callgraph
 */
void get_new_inventory_item(Uint8 *data);

/*!
 * \ingroup item
 * \brief
 *
 *      Detail
 *
 * \param data
 *
 * \sa process_message_from_server
 */
void get_bag_item(Uint8 *data);

/*!
 * \ingroup item
 * \brief
 *
 *      Detail
 *
 * \param data
 *
 * \callgraph
 */
void get_bags_items_list(Uint8 *data);

/*!
 * \ingroup item
 * \brief
 *
 *      Detail
 *
 * \param bag_x
 * \param bag_y
 * \param bag_id
 *
 * \callgraph
 */
void put_bag_on_ground(int bag_x,int bag_y,int bag_id);

/*!
 * \ingroup item
 * \brief
 *
 *      Detail
 *
 * \param data
 *
 * \callgraph
 */
void add_bags_from_list(Uint8 *data);

/*!
 * \ingroup item
 * \brief
 *
 *      Detail
 *
 * \param which_bag
 *
 * \callgraph
 */
void remove_bag(int which_bag);

/*!
 * \ingroup item
 * \brief
 *
 *      Detail
 *
 * \param object_id
 */
void open_bag(int object_id);
#endif
