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

} item;

typedef struct
{
	int pos;
	int image_id;
	int quantity;
} ground_item;

typedef struct
{
	int x;
	int y;
	int obj_3d_id;
} bag;


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

#define	ITEM_WEAR_START	36
#define	ITEM_NUM_WEAR	8
#define	ITEM_NUM_ITEMS	(ITEM_WEAR_START+ITEM_NUM_WEAR)

extern item item_list[ITEM_NUM_ITEMS];
extern item manufacture_list[ITEM_NUM_ITEMS];
extern ground_item ground_item_list[50];
extern bag bag_list[200];

extern item inventory_trade_list[ITEM_WEAR_START];
extern item your_trade_list[24];
extern item others_trade_list[24];
extern int trade_you_accepted;
extern int trade_other_accepted;
extern char other_player_trade_name[20];

extern int view_ground_items;
extern int no_view_my_items;

extern int items_win;
extern int items_menu_x;
extern int items_menu_y;
extern int items_menu_x_len;
extern int items_menu_y_len;
//extern int items_menu_dragged;

extern int ground_items_win;
extern int ground_items_menu_x;
extern int ground_items_menu_y;
extern int ground_items_menu_x_len;
extern int ground_items_menu_y_len;
//extern int ground_items_menu_dragged;

extern int manufacture_menu_x;
extern int manufacture_menu_y;
extern int manufacture_menu_x_len;
extern int manufacture_menu_y_len;
//extern int manufacture_menu_dragged;

extern int trade_menu_x;
extern int trade_menu_y;
extern int trade_menu_x_len;
extern int trade_menu_y_len;
//extern int trade_menu_dragged;

extern int items_text_1;
extern int items_text_2;
extern int items_text_3;
extern int items_text_4;
extern int items_text_5;
extern int items_text_6;
extern int items_text_7;
extern int items_text_8;
extern int items_text_9;

extern int item_dragged;

extern char items_string[300];

extern int item_quantity;

extern int click_speed;

void display_items_menu();
void get_your_items(Uint8 *data);
void drag_item();
void remove_item_from_inventory(int pos);
void remove_item_from_ground(Uint8 pos);
void get_new_inventory_item(Uint8 *data);
void draw_pick_up_menu();
void get_bag_item(Uint8 *data);
void get_bags_items_list(Uint8 *data);
void put_bag_on_ground(int bag_x,int bag_y,int bag_id);
void add_bags_from_list(Uint8 *data);
void remove_bag(int which_bag);
void open_bag(int object_id);
#endif

