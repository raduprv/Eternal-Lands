/*!
 * \file
 * \ingroup item
 * \brief Item handling and storing
 */
#ifndef __ITEMS_H__
#define __ITEMS_H__

#include "platform.h"
#include "bags.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Any item in EL has assigned an item struct
 */
typedef struct
{
	Uint16 id; /*!< server id of this item */
	int image_id; /*!< id of the image for this item */
	int pos;
	int quantity;
	int is_reagent; /*!< can be used for spells? */
	int is_resource; /*!< does it appear on the manufacturing menu? */
	int use_with_inventory;
	int is_stackable;
#ifdef NEW_SOUND
	int action;			/*!< action being done on this item, for playing sounds on server result */
	int action_time;	/*!< time this action has been active - times out after 2 seconds */
#endif // NEW_SOUND
	Uint32 cooldown_time; /*!< time when cooldown shall reach zero */
	Uint32 cooldown_rate; /*!< time that the item would need to cool down from full heat */
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
#define ITEM_NO_ACTION -1
#define ITEM_EDIT_QUANT 6
/*! @} */

/*!
 * \name The quantities are located within this struct 
*/
struct quantities {
	int selected;
	struct tmp {
		int val;
		int len;
		char str[10];
	} quantity[ITEM_EDIT_QUANT+1];
};

extern item item_list[ITEM_NUM_ITEMS]; /*!< global list of items */

extern struct quantities quantities; /*!< Quantities displayed in the items window*/

extern void strap_word(char * in, char * out);
extern ground_item ground_item_list[ITEMS_PER_BAG];
extern bag bag_list[NUM_BAGS];

extern int item_action_mode;

/*! \name windows handlers */
/*! @{ */
extern int items_win; /*!< inventory windows handler */
/*! @} */

extern int items_menu_x;
extern int items_menu_y;

/*! \name Text fields for items */
/*! @{ */
#define MAX_ITEMS_TEXTURES  32
extern int items_text[MAX_ITEMS_TEXTURES];
/*! @} */

extern int item_dragged;

extern int use_item;

extern int item_quantity;

extern int use_small_items_window;

extern int manual_size_items_window;

extern int allow_equip_swap;

extern int items_mix_but_all;

extern int items_stoall_nofirstrow;
extern int items_stoall_nolastrow;
extern int items_dropall_nofirstrow;
extern int items_dropall_nolastrow;

extern int items_auto_get_all;
extern int items_list_on_left;

extern int item_uid_enabled;
extern const Uint16 unset_item_uid;

#ifdef NEW_SOUND
void update_item_sound(int interval);
#endif // NEW_SOUND

int move_item(int item_pos_to_mov, int destination_pos);


void draw_item(int id, int x_start, int y_start, int gridsize);
void gray_out(int x_start, int y_start, int gridsize);


/*!
 * \ingroup display_utils
 * \brief   Renders the storage grid
 *
 *      Renders a storage grid with up to \a columns columns and \a rows rows. The parameters \a left and \a top indicate the starting position and \a width and \a height indicate the size of the grid window.
 *
 * \param columns   number of columns to use for the grid
 * \param rows      number of rows to use for the grid
 * \param left      x coordinate of the grid window
 * \param top       y coordinate of the grid window
 * \param width     width of the grid window
 * \param height    height of the grid window
 */
void rendergrid(int columns, int rows, int left, int top, int width, int height);

/*!
 * \ingroup display_utils
 * \brief   Gets the mouse position within the storage grid window
 *
 *      Gets the mouse position within the storage grid window.
 *
 * \param mx        x coordinate of the mouse position
 * \param my        y coordinate of the mouse position
 * \param columns   number of columns of the grid window
 * \param rows      number of rows of the grid window
 * \param left      x coordinate of the grid window
 * \param top       y coordinate of the grid window
 * \param width     width of the grid window
 * \param height    height of the grid window
 * \retval int      the grid position of the mouse, i.e. the grid number where the mouse cursor currently is, or -1 if the mouse cursor is outside the grid window
 */
int get_mouse_pos_in_grid(int mx, int my, int columns, int rows, int left, int top, int width, int height);

/*!
 * \ingroup item
 * \brief Gets the textures associated with the item \a no.
 *
 *      Returns the texture id associated with the item given by \a no.
 *
 * \param no        id of the item
 * \retval GLuint   the texture id associated with \a no.
 */
static __inline__ GLuint get_items_texture(int no)
{
	return items_text[no];
}

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
void get_your_items (const Uint8 *data);

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
void get_new_inventory_item (const Uint8 *data);

/*!
 * \ingroup item
 * \brief   Sets the cooldown values of inventory items from server data.
 *
 *      Sets the cooldown values of inventory items from server data.
 *
 * \param data		the incoming data string from the server
 * \param cooldown 	the length of the string in bytes
 *
 */
void get_items_cooldown (const Uint8 *data, int len);

/*!
 * \ingroup item
 * \brief   Updates the cooldown value of inventory items.
 *
 *      Updates the cooldown value of inventory items.
 *
 */
void update_cooldown ();

/*!
 * \ingroup item
 * \brief   Sets the displayed string for the items, manufacture and trade windows.
 *
 *      The items, manufacture and trade windows all display the same string, 
 * 		normally set in multiplayer.c.  This function can also set the string.
 *		Each window independantly wraps the string to fit it's window.
 *
 * \param  colour_code the colour code for the string
 * \param  the_text the null terminated string to display
 */
void set_shown_string(char colour_code, const char *the_text);

#ifdef	NEW_TEXTURES
void get_item_uv(const Uint32 item, float* u_start, float* v_start,
	float* u_end, float* v_end);
#endif	/* NEW_TEXTURES */

#ifdef __cplusplus
} // extern "C"
#endif

#endif
