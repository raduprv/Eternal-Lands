#ifndef __ITEMS_H__
#define __ITEMS_H__

typedef struct
	{
		int image_id;
		int pos;
		int quantity;
		int is_reagent;//can be used for spells?
		int is_resource;//does it appear on the manufacturing menu?
		int use_with_inventory;

	}item;

typedef struct
	{
		int pos;
		int image_id;
		int quantity;
	}ground_item;

typedef struct
	{
		int x;
		int y;
		int obj_3d_id;
	}bag;


typedef enum {
	ITEM_REAGENT           = 1,//can be used in magic
	ITEM_RESOURCE          = 2,//can be used to manufacture
	ITEM_STACKABLE         = 4,
	ITEM_INVENTORY_USABLE  = 8,
	ITEM_TILE_USABLE       = 16,
	ITEM_PLAYER_USABLE     = 32,
	ITEM_OBJECT_USABLE     = 64,
	ITEM_ON_OFF            = 128,
} item_definition_flags;

extern item item_list[36+6];
extern item manufacture_list[36+6];
extern ground_item ground_item_list[50];
extern bag bag_list[200];

extern item inventory_trade_list[36];
extern item your_trade_list[24];
extern item others_trade_list[24];
extern int trade_you_accepted;
extern int trade_other_accepted;
extern char other_player_trade_name[20];

extern int view_my_items;
extern int view_ground_items;
extern int view_manufacture_menu;
extern int view_trade_menu;
extern int no_view_my_items;

extern int items_menu_x;
extern int items_menu_y;
extern int items_menu_x_len;
extern int items_menu_y_len;
extern int items_menu_dragged;

extern int ground_items_menu_x;
extern int ground_items_menu_y;
extern int ground_items_menu_x_len;
extern int ground_items_menu_y_len;
extern int ground_items_menu_dragged;

extern int manufacture_menu_x;
extern int manufacture_menu_y;
extern int manufacture_menu_x_len;
extern int manufacture_menu_y_len;
extern int manufacture_menu_dragged;

extern int trade_menu_x;
extern int trade_menu_y;
extern int trade_menu_x_len;
extern int trade_menu_y_len;
extern int trade_menu_dragged;

extern int items_text_1;
extern int items_text_2;
extern int items_text_3;
extern int items_text_4;
extern int items_text_5;
extern int items_text_6;
extern int item_dragged;

extern char items_string[300];

extern int item_quantity;

#endif

