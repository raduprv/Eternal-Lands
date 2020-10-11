#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "items.h"
#include "asc.h"
#include "colors.h"
#include "client_serv.h"
#include "cursors.h"
#include "context_menu.h"
#include "text.h"
#include "elconfig.h"
#include "elwindows.h"
#include "errors.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud.h"
#include "init.h"
#include "interface.h"
#include "item_info.h"
#include "item_lists.h"
#include "manufacture.h"
#include "misc.h"
#include "multiplayer.h"
#include "platform.h"
#include "sound.h"
#include "storage.h"
#include "textures.h"
#include "translate.h"
#include "counters.h"
#include "widgets.h"
#include "spells.h"

item item_list[ITEM_NUM_ITEMS];
item_extra item_list_extra[ITEM_NUM_ITEMS];

struct quantities quantities = {
	0,
	{
		{1, 1, "1"},
		{5, 1, "5"},
		{10, 2, "10"},
		{20, 2, "20"},
		{50, 2, "50"},
		{100, 3, "100"},
		{1, 1, "1"}, // item list preview quantity - not editable or displayed with others
	}
};
int edit_quantity=-1;

static int item_action_mode = ACTION_WALK;

int item_dragged=-1;
int item_quantity=1;
int use_item=-1;
int item_uid_enabled = 0;
const Uint16 unset_item_uid = (Uint16)-1;
int items_text[MAX_ITEMS_TEXTURES];
int independant_inventory_action_modes = 0;

/* title menu options */
int allow_equip_swap=0;
int use_small_items_window = 0;
int manual_size_items_window = 0;
int items_mod_click_any_cursor = 1;
int items_disable_text_block = 0;
int items_buttons_on_left = 0;
int items_equip_grid_on_left = 0;

/* button options */
int items_mix_but_all = 0;
int items_stoall_nofirstrow = 0;
int items_stoall_nolastrow = 0;
int items_dropall_nofirstrow = 0;
int items_dropall_nolastrow = 0;
int items_list_on_left = 0;

/* use standard way to size and position the window components */
struct win_component {int pos_x; int pos_y; int len_x; int len_y; int mouse_over; int rows; int cols; int width; int height; };
static struct win_component items_grid = {0,0,0,0,-1,0,0,0,0};
static struct win_component equip_grid = {0,0,0,0,-1,0,0,0,0};
static struct win_component buttons_grid = {0,0,0,0,-1,0,0,0,0};
static struct win_component text_arrow = {0,0,0,0,-1,0,0,0,0};
static struct win_component unequip_arrow = {0,0,0,0,-1,0,0,0,0};
static struct win_component message_box = {0,0,0,0,-1,0,0,0,0};
static struct win_component labels_box = {0,0,0,0,-1,0,0,0,0};
static struct win_component quantity_grid = {0,0,0,0,-1,0,0,0,0};

static char items_string[350] = {0};
static size_t last_items_string_id = 0;
static const char *item_help_str = NULL;
static const char *item_desc_str = NULL;
static float button_text_zoom = 0.0f;

#define NUMBUT 5
static size_t buttons_cm_id[NUMBUT] = {CM_INIT_VALUE, CM_INIT_VALUE, CM_INIT_VALUE, CM_INIT_VALUE, CM_INIT_VALUE};
enum { BUT_STORE, BUT_GET, BUT_DROP, BUT_MIX, BUT_ITEM_LIST };

/* variables to enable enhanced equipment swapping */
static struct { int last_dest; int move_to; int move_from; size_t string_id; Uint32 start_time; } swap_complete = {-1, -1, -1, 0, 0};

static int show_items_handler(window_info * win);
static void drop_all_handler(void);

// Limit external setting of the action mode: Called due to an action keypress or action icon in the icon window.
void set_items_action_mode(int new_mode)
{
	// Only change the action mode if is one used by the window.
	if (!independant_inventory_action_modes && ((new_mode == ACTION_WALK) || (new_mode == ACTION_LOOK) || (new_mode == ACTION_USE) || (new_mode == ACTION_USE_WITEM)))
		item_action_mode = new_mode;
}

void set_shown_string(char colour_code, const char *the_text)
{
	if (strlen(the_text) == 0)
	{
		inventory_item_string[0] = '\0';
		inventory_item_string_id++;
		return;
	}
	inventory_item_string[0] = to_color_char(colour_code);
	safe_strncpy2(inventory_item_string+1, the_text, sizeof(inventory_item_string)-2, strlen(the_text));
	inventory_item_string[sizeof(inventory_item_string)-1] = 0;
	inventory_item_string_id++;
}


/* return index of button or -1 if mouse not over a button */
static int over_button(window_info *win, int mx, int my)
{
	if ((mx > buttons_grid.pos_x) && (mx < buttons_grid.pos_x + buttons_grid.len_x) &&
		(my > buttons_grid.pos_y) && (my < buttons_grid.pos_y + buttons_grid.len_y)) {
		return (my - buttons_grid.pos_y) / buttons_grid.height;
	}
	return -1;
}


void gray_out(int x_start, int y_start, int gridsize){

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	//glBlendFunc(GL_DST_COLOR, GL_ONE); //this brightens up
	glBlendFunc(GL_ZERO, GL_SRC_COLOR); //this brightens down
	glColor3f(0.4f, 0.2f, 0.2f);
	glBegin(GL_QUADS);
		glVertex3i(x_start,y_start,0);
		glVertex3i(x_start+gridsize,y_start,0);
		glVertex3i(x_start+gridsize,y_start+gridsize,0);
		glVertex3i(x_start,y_start+gridsize,0);
	glEnd();
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);
}


void rendergrid(int columns, int rows, int left, int top, int width, int height)
{
	int x, y;
	int temp;

	glBegin(GL_LINES);

	for(y=0; y<=rows; y++){
		temp = top + y * height;
		glVertex2i(left, temp);
		glVertex2i(left + width*columns, temp);
	}

	for(x=0; x<columns+1; x++){
		temp = left + x * width;
		glVertex2i(temp, top);
		glVertex2i(temp, top + height*rows);
	}

	glEnd();
}

int get_mouse_pos_in_grid(int mx, int my, int columns, int rows, int left, int top, int width, int height)
{
	int x, y, i;

	mx -= left;
	my -= top;

	i = 0;
	for (y = 0; y < rows; y++)
	{
		for (x = 0; x < columns; x++, i++)
		{
			if (mx >= x*width && mx <= (x+1)*width && my >= y*height && my <= (y+1)*height)
				return i;
		}
	}

	return -1;
}

void reset_quantity (int pos)
{
	int val;

	switch(pos)
	{
		case 0:
			val = 1;
			break;
		case 1:
			val = 5;
			break;
		case 2:
			val = 10;
			break;
		case 3:
			val = 20;
			break;
		case 4:
			val = 50;
			break;
		case 5:
			val = 100;
			break;
		default:
			LOG_ERROR ("Trying to reset invalid element of quantities, pos = %d", pos);
			return;
	}

	safe_snprintf (quantities.quantity[pos].str, sizeof(quantities.quantity[pos].str), "%d", val);
	quantities.quantity[pos].len = strlen (quantities.quantity[pos].str);
	quantities.quantity[pos].val = val;
}

void get_item_uv(const Uint32 item, float* u_start, float* v_start,
	float* u_end, float* v_end)
{
	*u_start = (50.0f/256.0f) * (item % 5) + 0.5f / 256.0f;
	*u_end = *u_start + (50.0f/256.0f);
	*v_start = (50.0f/256.0f) * (item / 5) + 0.5f / 256.0f;
	*v_end = *v_start + (50.0f/256.0f);
}

void drag_item(int item, int storage, int mini)
{
	float u_start,v_start,u_end,v_end;
	int cur_item,this_texture;
	int cur_item_img;
	int my_items_grid_size = (int)(0.5 + get_global_scale() * ((use_small_items_window) ?33: 50));
	int offset = (mini) ? my_items_grid_size/3 : my_items_grid_size/2;

	int quantity=item_quantity;

	if(storage) {
		if (item < 0 || item >= STORAGE_ITEMS_SIZE)
			// oops
			return;

		cur_item=storage_items[item].image_id;
		if(!storage_items[item].quantity) {
			use_item = storage_item_dragged=-1;
			return;
		}
		if (quantity > storage_items[item].quantity)
			quantity = storage_items[item].quantity;
	} else {
		if (item < 0 || item >= ITEM_NUM_ITEMS)
			// oops
			return;

		cur_item=item_list[item].image_id;
		if(!item_list[item].quantity) {
			use_item = item_dragged=-1;
			return;
		}
		if (item >= ITEM_WEAR_START) {
			quantity = -1;
		} else if (item_list[item].is_stackable) {
			if(quantity > item_list[item].quantity)
				quantity = item_list[item].quantity;
		// The quantity for non-stackable items is misleading so don't show it unless we can reliably count how many we have.
		} else if (item_list[item].id == unset_item_uid) {
			quantity = -1;
		} else {
			size_t i;
			int count = 0;
			for (i = 0; i < ITEM_WEAR_START; i++)
				if((item_list[i].quantity > 0) && (item_list[item].image_id == item_list[i].image_id) && (item_list[item].id == item_list[i].id))
					count++;
			if (quantity > count)
				quantity = count;
		}
	}

	cur_item_img=cur_item%25;
	get_item_uv(cur_item_img, &u_start, &v_start, &u_end, &v_end);

	//get the texture this item belongs to
	this_texture=get_items_texture(cur_item/25);

	bind_texture(this_texture);
	glBegin(GL_QUADS);
	draw_2d_thing(u_start, v_start, u_end, v_end, mouse_x - offset, mouse_y - offset, mouse_x + offset, mouse_y + offset);
	glEnd();

	if (!mini && quantity != -1)
	{
		int text_height = get_line_height(UI_FONT, get_global_scale() * DEFAULT_SMALL_RATIO);
		unsigned char str[20];
		safe_snprintf((char*)str, sizeof(str), "%d", quantity);
		draw_string_small_zoomed_centered(mouse_x, mouse_y - offset - text_height,
			str, 1, get_global_scale());
	}
}

void get_your_items (const Uint8 *data)
{
	int i,total_items,pos,len;
	Uint8 flags;

	if (item_uid_enabled)
		len=10;
	else
		len=8;

	//data[0] -> num_items
	//data[1] -> image_id
	//data[3] -> quantity
	//data[7] -> pos
	//data[8] -> flags
	//data[9] -> id


	total_items=data[0];

	//clear the items first
	for(i=0;i<ITEM_NUM_ITEMS;i++){
		item_list[i].quantity=0;
		item_list_extra[i].slot_busy_start = 0;
	}

	for(i=0;i<total_items;i++){
		pos=data[i*len+1+6];
		// try not to wipe out cooldown information if no real change
		if(item_list[pos].image_id != SDL_SwapLE16(*((Uint16 *)(data+i*len+1))) ){
			item_list[pos].cooldown_time = 0;
			item_list[pos].cooldown_rate = 1;
		}
		item_list[pos].image_id=SDL_SwapLE16(*((Uint16 *)(data+i*len+1)));
		item_list[pos].quantity=SDL_SwapLE32(*((Uint32 *)(data+i*len+1+2)));
		item_list[pos].pos=pos;
#ifdef NEW_SOUND
		item_list[pos].action = ITEM_NO_ACTION;
		item_list[pos].action_time = 0;
#endif // NEW_SOUND
		flags=data[i*len+1+7];
		if (item_uid_enabled)
			item_list[pos].id=SDL_SwapLE16(*((Uint16 *)(data+i*len+1+8)));
		else
			item_list[pos].id=unset_item_uid;
		item_list[pos].is_resource=((flags&ITEM_RESOURCE)>0);
		item_list[pos].is_reagent=((flags&ITEM_REAGENT)>0);
		item_list[pos].use_with_inventory=((flags&ITEM_INVENTORY_USABLE)>0);
		item_list[pos].is_stackable=((flags&ITEM_STACKABLE)>0);
	}

	build_manufacture_list();
	check_castability();
}

#ifdef NEW_SOUND
void check_for_item_sound(int pos)
{
	int i, snd = -1;

#ifdef _EXTRA_SOUND_DEBUG
//	printf("Used item: %d, Image ID: %d, Action: %d\n", pos, item_list[pos].image_id, item_list[pos].action);
#endif // _EXTRA_SOUND_DEBUG
	if (item_list[pos].action != ITEM_NO_ACTION)
	{
		// Play the sound that goes with this action
		switch (item_list[pos].action)
		{
			case USE_INVENTORY_ITEM:
				snd = get_index_for_inv_use_item_sound(item_list[pos].image_id);
				break;
			case ITEM_ON_ITEM:
				// Find the second item (being used with)
				for (i = 0; i < ITEM_NUM_ITEMS; i++)
				{
					if (i != pos && item_list[i].action == ITEM_ON_ITEM)
					{
						snd = get_index_for_inv_usewith_item_sound(item_list[pos].image_id, item_list[i].action);
						break;
					}
				}
				break;
		}
		if (snd > -1)
			add_sound_object(snd, 0, 0, 1);
		// Reset the action
		item_list[pos].action = ITEM_NO_ACTION;
		item_list[pos].action_time = 0;
	}
}

void update_item_sound(int interval)
{
	int i;
	// Iterate through the list of items, checking for out of date item actions (> 2 sec old)
	for (i = 0; i < ITEM_NUM_ITEMS; i++)
	{
		if (item_list[i].action != ITEM_NO_ACTION)
		{
			item_list[i].action_time += interval;
			if (item_list[i].action_time >= 2000)
			{
				// Item action state is out of date so reset it
				item_list[i].action = ITEM_NO_ACTION;
				item_list[i].action_time = 0;
			}
		}
	}
}
#endif // NEW_SOUND

static int used_item_counter_initialised = 0;
static int used_item_counter_queue[ITEM_NUM_ITEMS];
static Uint32 used_item_counter_queue_time[ITEM_NUM_ITEMS];

void used_item_counter_init(void)
{
	size_t i;
	for (i=0; i<ITEM_NUM_ITEMS; i++)
	{
		used_item_counter_queue[i] = 0;
		used_item_counter_queue_time[i] = 0;
	}
	used_item_counter_initialised = 1;
	//printf("Used item counter code initialised\n");
}

void used_item_counter_timer(void)
{
	if (!used_item_counter_initialised)
		used_item_counter_init();
	else
	{
		size_t i;
		for (i=0; i<ITEM_NUM_ITEMS; i++)
		{
			/* The timeout value is need in the absence of a server change that would give feedback to
			use actions. Too short, and we will miss a valid counter event due to server lag. Too
			long, and we will interpret a single dropped item as use of an item that has actually
			failed. Hopefully, the server lag will not be as high as the timeout, and a player will
			not switch from failed ACTION_USE to dropping an item in less than the timeout. */
			if (used_item_counter_queue_time[i] && (SDL_GetTicks() - used_item_counter_queue_time[i]) > 1000u)
			{
				used_item_counter_queue[i] = 0;
				used_item_counter_queue_time[i] = 0;
				//printf("Used item counter pos %lu, confirmation timed out\n", i);
			}
		}
	}
}

void used_item_counter_action_use(int pos)
{
	if (!used_item_counter_initialised)
		used_item_counter_init();
	if ((pos < 0) || !(pos < ITEM_NUM_ITEMS))
		return;
	used_item_counter_queue[pos]++;
	used_item_counter_queue_time[pos] = SDL_GetTicks();
	//printf("Used item counter pos %d, waiting for confirmation pending %d\n", pos, used_item_counter_queue[pos]);
}


static void used_item_counter_check_confirmation(int pos, int new_quanity)
{
	if (!used_item_counter_initialised)
		used_item_counter_init();
	if ((pos < 0) || !(pos < ITEM_NUM_ITEMS))
		return;
	if (used_item_counter_queue[pos])
	{
		if ((item_list[pos].quantity > 0) && (new_quanity < item_list[pos].quantity))
		{
			static int sent_unique_message = 0;
			static int sent_get_failed_message = 0;
			int item_count = get_item_count(item_list[pos].id, item_list[pos].image_id);
			used_item_counter_queue[pos]--;
			if (!used_item_counter_queue[pos])
				used_item_counter_queue_time[pos] = 0;
			if (item_count == 1)
				increment_used_item_counter(get_basic_item_description(item_list[pos].id, item_list[pos].image_id), item_list[pos].quantity - new_quanity);
			else if ((item_count > 1) && !sent_unique_message)
			{
				LOG_TO_CONSOLE(c_red1, item_use_not_unique_str);
				LOG_TO_CONSOLE(c_red1, item_uid_help_str);
				sent_unique_message = 1;
			}
			else if ((item_count < 1) && !sent_get_failed_message)
			{
				char str[256];
				safe_snprintf(str, sizeof(str), "%s id=%d image_id=%d", item_use_get_failed_str, item_list[pos].id, item_list[pos].image_id);
				LOG_TO_CONSOLE(c_red1, str);
				sent_get_failed_message = 1;
			}
			//printf("Used item counter confirmed pos %d - item removed [%s] quanity %d\n", pos, get_item_description(item_list[pos].id, item_list[pos].image_id), item_list[pos].quantity - new_quanity);
		}
		//else
		//	printf("Used item counter pos %d issue with quanity\n", pos);
	}
	//else
	//	printf("Used item counter pos %d - not due to use action\n", pos);
}

void remove_item_from_inventory(int pos)
{
	used_item_counter_check_confirmation(pos, 0);
	item_list[pos].quantity=0;

	if (pos == swap_complete.move_from)
		swap_complete.move_to = swap_complete.move_from = -1;

#ifdef NEW_SOUND
	check_for_item_sound(pos);
#endif // NEW_SOUND

	build_manufacture_list();
	check_castability();
}

void get_new_inventory_item (const Uint8 *data)
{
	int pos;
	Uint8 flags;
	int quantity;
	int image_id;
	Uint16 id;

	if (item_uid_enabled)
		id=SDL_SwapLE16(*((Uint16 *)(data+8)));
	else
		id=unset_item_uid;

	pos= data[6];
	flags= data[7];
	image_id=SDL_SwapLE16(*((Uint16 *)(data)));
	quantity=SDL_SwapLE32(*((Uint32 *)(data+2)));

	used_item_counter_check_confirmation(pos, quantity);
	if (now_harvesting() && (quantity >= item_list[pos].quantity) ) {	//some harvests, eg hydrogenium and wolfram, also decrease an item number. only count what goes up
		increment_harvest_counter(item_list[pos].quantity > 0 ? quantity - item_list[pos].quantity : quantity);
	}

	// don't touch cool down when it's already active
	if(item_list[pos].quantity == 0 || item_list[pos].image_id != image_id){
		item_list[pos].cooldown_time = 0;
		item_list[pos].cooldown_rate = 1;
	}
	item_list[pos].quantity=quantity;
	item_list[pos].image_id=image_id;
	item_list[pos].pos=pos;
	item_list[pos].id=id;
	item_list[pos].is_resource=((flags&ITEM_RESOURCE)>0);
	item_list[pos].is_reagent=((flags&ITEM_REAGENT)>0);
	item_list[pos].use_with_inventory=((flags&ITEM_INVENTORY_USABLE)>0);
	item_list[pos].is_stackable=((flags&ITEM_STACKABLE)>0);

#ifdef NEW_SOUND
	check_for_item_sound(pos);
#endif // NEW_SOUND

	build_manufacture_list();
	check_castability();

	// if we can, complete a swap of equipment by moving the removed item to the slot left by the new
	if (pos == swap_complete.move_to)
	{
		if (item_list[swap_complete.move_from].quantity)
		{
			if (!move_item(swap_complete.move_from, swap_complete.move_to, -1))
				swap_complete.move_from = swap_complete.move_to = -1;
		}
		else
			swap_complete.move_from = swap_complete.move_to = -1;
	}
}



void draw_item(int id, int x_start, int y_start, int gridsize){
	float u_start,v_start,u_end,v_end;
	int cur_item;
	int this_texture;

	//get the UV coordinates.
	cur_item=id%25;
	get_item_uv(cur_item, &u_start, &v_start, &u_end, &v_end);

	//get the texture this item belongs to
	this_texture=get_items_texture(id/25);

	bind_texture(this_texture);
	glBegin(GL_QUADS);
		draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_start+gridsize-1,y_start+gridsize-1);
	glEnd();
}

static int display_items_handler(window_info *win)
{
	char str[80];
	char my_str[10];
	int x,y,i;
	int item_is_weared=0;
	Uint32 _cur_time = SDL_GetTicks(); /* grab a snapshot of current time */
	char *but_labels[NUMBUT] = { sto_all_str, get_all_str, drp_all_str, NULL, itm_lst_str };

	glEnable(GL_TEXTURE_2D);

	check_for_swap_completion();

	/*
	* Labrat: I never realised that a store all patch had been posted to Berlios by Awn in February '07
	* Thanks to Awn for his earlier efforts (but this is not a derivative of his earlier work)
	*
	*My next step will be to code an #ifdef STORE_ALL section to save the 0-35 loop in the click handler for future proofing
	*  ready for server side implementation
	*/
	// draw the button labels
	but_labels[BUT_MIX] = (items_mix_but_all) ?mix_all_str :mix_one_str;
	for (i=0; i<NUMBUT; i++) {
		strap_word(but_labels[i],my_str);
		glColor3f(0.77f,0.57f,0.39f);
		draw_text(buttons_grid.pos_x + buttons_grid.width/2,
			buttons_grid.pos_y + buttons_grid.height * i + buttons_grid.height/2,
			(const unsigned char*)my_str, strlen(my_str), win->font_category,
			TDO_ZOOM, button_text_zoom, TDO_ALIGNMENT, CENTER, TDO_VERTICAL_ALIGNMENT, CENTER_LINE,
			TDO_END);
	}

	x = quantity_grid.pos_x + quantity_grid.width / 2;
	y = quantity_grid.pos_y + quantity_grid.height / 2;
	for(i = 0; i < ITEM_EDIT_QUANT; x += quantity_grid.width, ++i){
		if(i==edit_quantity){
			glColor3f(1.0f, 0.0f, 0.3f);
		} else if(i==quantities.selected){
			glColor3f(0.0f, 1.0f, 0.3f);
		} else {
			glColor3f(0.3f,0.5f,1.0f);
		}
		draw_text(x, y, (const unsigned char*)quantities.quantity[i].str,
			strlen(quantities.quantity[i].str), win->font_category, TDO_ZOOM, win->current_scale_small,
			TDO_ALIGNMENT, CENTER, TDO_VERTICAL_ALIGNMENT, CENTER_DIGITS, TDO_END);
	}
	glColor3f(0.3f,0.5f,1.0f);
	draw_string_small_zoomed_right(labels_box.pos_x + labels_box.len_x, labels_box.pos_y,
		(const unsigned char*)quantity_str, 1, win->current_scale);

	glColor3f(0.57f,0.67f,0.49f);
	draw_text(equip_grid.pos_x + equip_grid.len_x / 2, equip_grid.pos_y, (const unsigned char*)equip_str,
		strlen(equip_str), win->font_category, TDO_MAX_WIDTH, equip_grid.len_x,
		TDO_ZOOM, win->current_scale_small, TDO_SHRINK_TO_FIT, 1, TDO_ALIGNMENT, CENTER,
		TDO_VERTICAL_ALIGNMENT, BOTTOM_LINE, TDO_END);

	glColor3f(1.0f,1.0f,1.0f);
	//ok, now let's draw the objects...
	for(i=ITEM_NUM_ITEMS-1;i>=0;i--){
		if(item_list[i].quantity){
			int cur_pos;
			int x_start,x_end,y_start,y_end;

			// don't display an item that is in the proces of being moved after equipment swap
			if (item_swap_in_progress(i))
				continue;

			//get the x and y
			cur_pos=i;
			if(cur_pos>=ITEM_WEAR_START){//the items we 'wear' are smaller
				cur_pos-=ITEM_WEAR_START;
				item_is_weared=1;
				x_start = equip_grid.pos_x + equip_grid.width * (cur_pos % equip_grid.cols) + 1;
				x_end = x_start + equip_grid.width - 1;
				y_start = equip_grid.pos_y + equip_grid.height * (cur_pos / equip_grid.cols);
				y_end = y_start + equip_grid.height - 1;
				draw_item(item_list[i].image_id, x_start, y_start, equip_grid.width - 1);
			} else {
				item_is_weared=0;
				x_start = items_grid.pos_x + items_grid.width * (cur_pos % items_grid.cols) +1;
				x_end = x_start + items_grid.width - 1;
				y_start = items_grid.pos_y + items_grid.height * (cur_pos / items_grid.cols);
				y_end = y_start + items_grid.height - 1;
				draw_item(item_list[i].image_id, x_start, y_start, items_grid.width - 1);
			}
			if ((_cur_time - item_list_extra[i].slot_busy_start) < 250)
				gray_out(x_start, y_start, items_grid.width);

			if (item_list[i].cooldown_time > _cur_time)
			{
				float cooldown = ((float)(item_list[i].cooldown_time - _cur_time)) / ((float)item_list[i].cooldown_rate);
				float x_center = (x_start + x_end)*0.5f;
				float y_center = (y_start + y_end)*0.5f;
				float flash_effect_offset = 0.0f;

				if (cooldown < 0.0f)
					cooldown = 0.0f;
				else if (cooldown > 1.0f)
					cooldown = 1.0f;

				glDisable(GL_TEXTURE_2D);
				glEnable(GL_BLEND);

				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glBegin(GL_TRIANGLE_FAN);
					if (cooldown < 1.0f)
						flash_effect_offset = sin(pow(1.0f - cooldown, 4.0f) * 2.0f * M_PI * 30.0);
					glColor4f(0.14f - flash_effect_offset / 20.0f, 0.35f - flash_effect_offset / 20.0f, 0.82f + flash_effect_offset / 8.0f, 0.48f + flash_effect_offset / 15.0f);

					glVertex2f(x_center, y_center);

					if (cooldown >= 0.875f) {
						float t = tan(2.0f*M_PI*(1.0f - cooldown));
						glVertex2f(t*x_end + (1.0f - t)*x_center, y_start);
						glVertex2f(x_end, y_start);
						glVertex2f(x_end, y_end);
						glVertex2f(x_start, y_end);
						glVertex2f(x_start, y_start);
					} else if (cooldown >= 0.625f) {
						float t = 0.5f + 0.5f*tan(2.0f*M_PI*(0.75f - cooldown));
						glVertex2f(x_end, t*y_end + (1.0f - t)*y_start);
						glVertex2f(x_end, y_end);
						glVertex2f(x_start, y_end);
						glVertex2f(x_start, y_start);
					} else if (cooldown >= 0.375f) {
						float t = 0.5f + 0.5f*tan(2.0f*M_PI*(0.5f - cooldown));
						glVertex2f(t*x_start + (1.0f - t)*x_end, y_end);
						glVertex2f(x_start, y_end);
						glVertex2f(x_start, y_start);
					} else if (cooldown >= 0.125f) {
						float t = 0.5f + 0.5f*tan(2.0f*M_PI*(0.25f - cooldown));
						glVertex2f(x_start, t*y_start + (1.0f - t)*y_end);
						glVertex2f(x_start, y_start);
					} else {
						float t = tan(2.0f*M_PI*(cooldown));
						glVertex2f(t*x_start + (1.0f - t)*x_center, y_start);
					}

					glVertex2f(x_center, y_start);
				glEnd();

				glDisable(GL_BLEND);
				glEnable(GL_TEXTURE_2D);
			}

			if(!item_is_weared){
				int use_large = (items_grid.mouse_over == i) && enlarge_text();
				safe_snprintf(str, sizeof(str), "%i", item_list[i].quantity);
				y_end -= (i & 1) ?items_grid.height - 1 : ((use_large) ?win->default_font_len_y :win->small_font_len_y);
				if (use_large)
					draw_string_shadowed_zoomed(x_start, y_end, (unsigned char*)str, 1,1.0f,1.0f,1.0f, 0.0f, 0.0f, 0.0f, win->current_scale);
				else
					draw_string_small_shadowed_zoomed(x_start, y_end, (unsigned char*)str, 1,1.0f,1.0f,1.0f, 0.0f, 0.0f, 0.0f, win->current_scale);
			}
		}
	}
	items_grid.mouse_over = -1;

	glColor3f(1.0f,1.0f,1.0f);

	//draw the load string
	safe_snprintf(str, sizeof(str), "%s: %i/%i", attributes.carry_capacity.shortname, your_info.carry_capacity.cur, your_info.carry_capacity.base);
	draw_string_small_zoomed(labels_box.pos_x, labels_box.pos_y, (unsigned char*)str, 1, win->current_scale);

	//now, draw the inventory text, if any.
	if (!items_disable_text_block)
	{
		if (last_items_string_id != inventory_item_string_id)
		{
			put_small_text_in_box_zoomed((const unsigned char*)inventory_item_string, strlen(inventory_item_string), message_box.len_x,
			(unsigned char*)items_string, win->current_scale);
			last_items_string_id = inventory_item_string_id;
		}
		draw_string_small_zoomed(message_box.pos_x, message_box.pos_y, (unsigned char*)items_string, message_box.rows, win->current_scale);
	}

	glDisable(GL_TEXTURE_2D);

	if (text_arrow.mouse_over != -1)
		glColor3f(0.99f,0.77f,0.55f);
	else
		glColor3f(0.77f,0.57f,0.39f);
	text_arrow.mouse_over = -1;
	if (items_disable_text_block)
	{
		glBegin(GL_LINES); /* Dn arrow */
			glVertex3i(text_arrow.pos_x, text_arrow.pos_y - text_arrow.len_y, 0);
			glVertex3i(text_arrow.pos_x + text_arrow.len_x/2, text_arrow.pos_y, 0);
			glVertex3i(text_arrow.pos_x + text_arrow.len_x/2, text_arrow.pos_y, 0);
			glVertex3i(text_arrow.pos_x + text_arrow.len_x, text_arrow.pos_y - text_arrow.len_y, 0);
		glEnd();
		// if we have a coloured message, draw a small dot at the top of the arrow to indicate so, using the colour of the message
		if ((strlen(inventory_item_string) > 0) && is_color(inventory_item_string[0]))
		{
			int colour = from_color_char (inventory_item_string[0]);
			if ((colour >= c_lbound) && (colour <= c_ubound))
			{
				glColor4f((float) colors_list[colour].r1 / 255.0f, (float) colors_list[colour].g1 / 255.0f, (float) colors_list[colour].b1 / 255.0f, 1.0f);
				glEnable( GL_POINT_SMOOTH );
				glEnable( GL_BLEND );
				glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
				glPointSize(text_arrow.len_x/3);
				glBegin(GL_POINTS);
				glVertex2f(text_arrow.pos_x + text_arrow.len_x/2, text_arrow.pos_y - text_arrow.len_y + text_arrow.len_y/6);
				glEnd();
				glDisable(GL_BLEND);
				glDisable(GL_POINT_SMOOTH);
			}
		}
	}
	else
	{
		glBegin(GL_LINES); /* Up arrow */
			glVertex3i(text_arrow.pos_x, text_arrow.pos_y, 0);
			glVertex3i(text_arrow.pos_x + text_arrow.len_x/2, text_arrow.pos_y - text_arrow.len_y, 0);
			glVertex3i(text_arrow.pos_x + text_arrow.len_x/2, text_arrow.pos_y - text_arrow.len_y, 0);
			glVertex3i(text_arrow.pos_x + text_arrow.len_x, text_arrow.pos_y, 0);
		glEnd();
	}

	if (unequip_arrow.mouse_over != -1)
		glColor3f(0.99f,0.77f,0.55f);
	else
		glColor3f(0.77f,0.57f,0.39f);
	unequip_arrow.mouse_over = -1;
	if (items_equip_grid_on_left)
	{
		int arrow_width = (disable_double_click) ?unequip_arrow.len_x : unequip_arrow.len_x/2;
		glBegin(GL_LINES); /* right arrow */
			glVertex3i(unequip_arrow.pos_x, unequip_arrow.pos_y, 0);
			glVertex3i(unequip_arrow.pos_x + arrow_width, unequip_arrow.pos_y + unequip_arrow.len_y/2, 0);
			glVertex3i(unequip_arrow.pos_x + arrow_width, unequip_arrow.pos_y + unequip_arrow.len_y/2, 0);
			glVertex3i(unequip_arrow.pos_x, unequip_arrow.pos_y + unequip_arrow.len_y, 0);
			if (!disable_double_click)
			{
				glVertex3i(unequip_arrow.pos_x + arrow_width, unequip_arrow.pos_y, 0);
				glVertex3i(unequip_arrow.pos_x + unequip_arrow.len_x, unequip_arrow.pos_y + unequip_arrow.len_y/2, 0);
				glVertex3i(unequip_arrow.pos_x + unequip_arrow.len_x, unequip_arrow.pos_y + unequip_arrow.len_y/2, 0);
				glVertex3i(unequip_arrow.pos_x + arrow_width, unequip_arrow.pos_y + unequip_arrow.len_y, 0);
			}
		glEnd();
	}
	else
	{
		int arrow_width = (disable_double_click) ?unequip_arrow.len_x : unequip_arrow.len_x/2;
		glBegin(GL_LINES); /* left arrow */
			glVertex3i(unequip_arrow.pos_x + arrow_width, unequip_arrow.pos_y, 0);
			glVertex3i(unequip_arrow.pos_x, unequip_arrow.pos_y + unequip_arrow.len_y/2, 0);
			glVertex3i(unequip_arrow.pos_x, unequip_arrow.pos_y + unequip_arrow.len_y/2, 0);
			glVertex3i(unequip_arrow.pos_x + arrow_width, unequip_arrow.pos_y + unequip_arrow.len_y, 0);
			if (!disable_double_click)
			{
				glVertex3i(unequip_arrow.pos_x + unequip_arrow.len_x, unequip_arrow.pos_y, 0);
				glVertex3i(unequip_arrow.pos_x + arrow_width, unequip_arrow.pos_y + unequip_arrow.len_y/2, 0);
				glVertex3i(unequip_arrow.pos_x + arrow_width, unequip_arrow.pos_y + unequip_arrow.len_y/2, 0);
				glVertex3i(unequip_arrow.pos_x + unequip_arrow.len_x, unequip_arrow.pos_y + unequip_arrow.len_y, 0);
			}
		glEnd();
	}

	// Render the grid *after* the images. It seems impossible to code
	// it such that images are rendered exactly within the boxes on all
	// cards
	glColor3f(0.77f,0.57f,0.39f);

	//draw the grids
	rendergrid(items_grid.cols, items_grid.rows, items_grid.pos_x, items_grid.pos_y, items_grid.width, items_grid.height);

	glColor3f(0.57f,0.67f,0.49f);
	rendergrid(equip_grid.cols, equip_grid.rows, equip_grid.pos_x, equip_grid.pos_y, equip_grid.width, equip_grid.height);

	// draw the button boxes
	glColor3f(0.77f,0.57f,0.39f);
	for (i=0; i<NUMBUT; i++) {
		glBegin(GL_LINE_LOOP);
			glVertex3i(buttons_grid.pos_x + buttons_grid.width, buttons_grid.pos_y + buttons_grid.height * i, 0);
			glVertex3i(buttons_grid.pos_x, buttons_grid.pos_y + buttons_grid.height * i, 0);
			glVertex3i(buttons_grid.pos_x, buttons_grid.pos_y + buttons_grid.height * i + buttons_grid.height, 0);
			glVertex3i(buttons_grid.pos_x + buttons_grid.width, buttons_grid.pos_y + buttons_grid.height * i + buttons_grid.height, 0);
		glEnd();
	}

	// highlight a button with the mouse over
	if (buttons_grid.mouse_over != -1)
	{
		glColor3f(0.99f,0.77f,0.55f);
		glBegin(GL_LINE_LOOP);
			glVertex3i(buttons_grid.pos_x + buttons_grid.width + 1, buttons_grid.pos_y + buttons_grid.height * buttons_grid.mouse_over - 1, 0);
			glVertex3i(buttons_grid.pos_x - 1, buttons_grid.pos_y + buttons_grid.height * buttons_grid.mouse_over - 1, 0);
			glVertex3i(buttons_grid.pos_x - 1, buttons_grid.pos_y + buttons_grid.height * buttons_grid.mouse_over + buttons_grid.height + 1, 0);
			glVertex3i(buttons_grid.pos_x + buttons_grid.width + 1, buttons_grid.pos_y + buttons_grid.height * buttons_grid.mouse_over + buttons_grid.height + 1, 0);
		glEnd();
	}

	//now, draw the quantity boxes
	glColor3f(0.3f,0.5f,1.0f);
	rendergrid(quantity_grid.cols, quantity_grid.rows, quantity_grid.pos_x, quantity_grid.pos_y, quantity_grid.width, quantity_grid.height);

	glEnable(GL_TEXTURE_2D);

	// display help text for button if mouse over one
	if ((buttons_grid.mouse_over != -1) && show_help_text) {
		char *helpstr[NUMBUT] = { stoall_help_str, getall_help_str, ((disable_double_click) ?drpall_help_str :dcdrpall_help_str), mixoneall_help_str, itmlst_help_str };
		show_help(helpstr[buttons_grid.mouse_over], 0, win->len_y+10, win->current_scale);
		show_help(cm_help_options_str, 0, win->len_y+10+win->small_font_len_y, win->current_scale);
	}
	// show help set in the mouse_over handler
	else {
		int offset = 10;
		if (show_help_text && (item_help_str != NULL)) {
			show_help(item_help_str, 0, win->len_y+offset, win->current_scale);
			offset += win->small_font_len_y;
		}
		if (item_desc_str != NULL)
			show_help(item_desc_str, 0, win->len_y+offset, win->current_scale);
		item_help_str = NULL;
		item_desc_str = NULL;
	}

	buttons_grid.mouse_over = -1;

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 1;
}


/* return 1 if sent the move command */
int move_item(int item_pos_to_mov, int destination_pos, int avoid_pos)
{
	int drop_on_stack = 0;
	swap_complete.last_dest = -1;
	/* if the dragged item is equipped and the destintion is occupied, try to find another slot */
	if ((item_pos_to_mov >= ITEM_WEAR_START) && (item_list[destination_pos].quantity)){
		int i;
		int have_free_pos = 0;
		/* find first free slot, use a free slot in preference to a stack as the server does the stacking */
		for (i = 0; i < ITEM_WEAR_START; i++){
			if (!item_list[i].quantity && i != avoid_pos){
				destination_pos = i;
				have_free_pos = 1;
				break;
			}
		}
		/* if no free slot, try to find an existing stack.  But be careful of dupe image ids */
		if (!have_free_pos && item_list[item_pos_to_mov].is_stackable){
			int num_stacks_found = 0;
			int proposed_slot = -1;
			for (i = 0; i < ITEM_WEAR_START; i++){
				if (item_list[i].is_stackable && (item_list[i].image_id == item_list[item_pos_to_mov].image_id)){
					num_stacks_found++;
					proposed_slot = i;
				}
			}
			/* only use the stack if we're sure there are no other possibilities */
			if (num_stacks_found == 1){
				destination_pos = proposed_slot;
				drop_on_stack = 1;
			}
			else
				set_shown_string(c_red2, items_stack_str);
			/*  This still leaves one possibility for the dreaded server accusation.
				If we have no free inventory slots, one or more stackable items
				unequipped, and a single, different equipped item with the same id as
				the aforementioned stack.  When we try to unequip the single item, the
				client tries to place it on that stack. This may mean we have to
				abandon this feature; i.e. allowing a stackable item to be unequipping
				when there are no free slots. (pjbroad/bluap) */
		}
	}
	/* move item */
	if(drop_on_stack || !item_list[destination_pos].quantity){
		Uint8 str[20];
		//send the drop info to the server
		str[0]=MOVE_INVENTORY_ITEM;
		str[1]=item_list[item_pos_to_mov].pos;
		str[2]=destination_pos;
		my_tcp_send(my_socket,str,3);
		swap_complete.last_dest = destination_pos;
		return 1;
	}
	else
		return 0;
}

static void equip_item(int item_pos_to_equip, int destination_pos)
{
	Uint8 str[20];
	//send the drop info to the server
	str[0]=MOVE_INVENTORY_ITEM;
	str[1]=item_list[item_pos_to_equip].pos;
	str[2]=destination_pos;
	my_tcp_send(my_socket,str,3);
}

static void prep_move_to_vacated_slot(int destination)
{
	swap_complete.move_from = swap_complete.last_dest;
	swap_complete.move_to = destination;
	swap_complete.string_id = inventory_item_string_id;
	swap_complete.start_time = SDL_GetTicks();
}

// stop expecting swap completion if we get a message, or we timeout (catch all)
void check_for_swap_completion(void)
{
	if (swap_complete.move_from != -1)
	{
		if (swap_complete.string_id != inventory_item_string_id)
			swap_complete.move_from = swap_complete.move_to = -1;
		if (SDL_GetTicks() > swap_complete.start_time + 2000)
			swap_complete.move_from = swap_complete.move_to = -1;
	}
}

int item_swap_in_progress(int item_pos)
{
	return (item_pos == swap_complete.move_from) ?1: 0;
}

// If we double-click an equipable item, and one of that type is already equipped, swap them.
//
static int swap_equivalent_equipped_item(int from_pos)
{
	size_t i;
	enum EQUIP_TYPE from_equip_type = get_item_equip_type(item_list[from_pos].id, item_list[from_pos].image_id);
	int same_pos = -1, left_hand_pos = -1, right_hand_pos = -1, both_hands_pos = -1;
	int item_to_swap = -1, extra_item = -1;

	// stop now if we don't know about the item type
	if (from_equip_type == EQUIP_NONE)
		return 0;

	// clear any previous "avoid destination" there may have been
	swap_complete.last_dest = -1;

	// there are several rules about swapping left and right handed items with both handed items
	// find each of the types
	for(i = ITEM_WEAR_START; i<ITEM_WEAR_START + ITEM_NUM_WEAR; i++)
		if (item_list[i].quantity > 0)
		{
			enum EQUIP_TYPE the_type = get_item_equip_type(item_list[i].id, item_list[i].image_id);
			if (the_type == from_equip_type)
				same_pos = i;
			else if (the_type == EQUIP_LEFT_HAND)
				left_hand_pos = i;
			else if (the_type == EQUIP_RIGHT_HAND)
				right_hand_pos = i;
			else if (the_type == EQUIP_BOTH_HANDS)
				both_hands_pos = i;
		}

	// if a simple, same item type swap
	if (same_pos != -1)
		item_to_swap = same_pos;
	// if equipping a both hands item, swap a right hand item first, any left hand as the extra
	else if (from_equip_type == EQUIP_BOTH_HANDS && right_hand_pos != -1)
	{
		item_to_swap = right_hand_pos;
		if (left_hand_pos != -1)
			extra_item = left_hand_pos;
	}
	// equipping a both hands item, no right hand but swap any left hand item
	else if (from_equip_type == EQUIP_BOTH_HANDS && left_hand_pos != -1)
		item_to_swap = left_hand_pos;
	// if theres an equipped both hands item, allow a right or left hand item to be swapped with it
	else if (both_hands_pos != -1 && (from_equip_type == EQUIP_RIGHT_HAND || from_equip_type == EQUIP_LEFT_HAND))
		item_to_swap = both_hands_pos;

	// remove any extra item, swap_complete.last_dest will be set the the new slot for the remove item
	if ((extra_item != -1) && !move_item(extra_item, 0, -1))
	{
		do_alert1_sound();
		set_shown_string(c_red2, items_cannot_equip_str);
		return 1;
	}

	// do the swap if any
	if (item_to_swap != -1)
	{
		// avoid moving to where we put any removed extra item
		if (move_item(item_to_swap, 0, swap_complete.last_dest))
		{
			equip_item(from_pos, item_to_swap);
			prep_move_to_vacated_slot(from_pos);
			do_get_item_sound();
			item_dragged = -1;
		}
		else
		{
			do_alert1_sound();
			set_shown_string(c_red2, items_cannot_equip_str);
		}
		return 1; // there was something to swap, and we tried, don't try other stuff
	}

	return 0; // there was nothing to swap, try other stuff
}

// common function for aut equip used by inventory and quickbar
void try_auto_equip(int from_item)
{
	if (allow_equip_swap && swap_equivalent_equipped_item(from_item)) {
		// all done, don't try direct equip
	} else {
		size_t i;
		for(i = ITEM_WEAR_START; i < ITEM_WEAR_START + ITEM_NUM_WEAR; i++) {
			if(item_list[i].quantity<1) {
				if (!move_item(from_item, i, -1))
				{
					do_alert1_sound();
					set_shown_string(c_red2, items_cannot_equip_str);
				}
				item_dragged=-1;
				break;
			}
		}
	}
}

static int click_items_handler(window_info *win, int mx, int my, Uint32 flags)
{
	Uint8 str[100];
	int right_click = flags & ELW_RIGHT_MOUSE;
	int ctrl_on = flags & KMOD_CTRL;
	int shift_on = flags & KMOD_SHIFT;
	int alt_on = flags & KMOD_ALT;
	int pos;
	actor *me;

	// only handle mouse button clicks, not scroll wheels moves (unless its the mix button)
	if (((flags & ELW_MOUSE_BUTTON) == 0) && (over_button(win, mx, my) != BUT_MIX)) return 0;

	// ignore middle mouse button presses
	if ((flags & ELW_MID_MOUSE) != 0) return 0;

	if (!right_click && over_button(win, mx, my) != -1)
		do_click_sound();

	if ((flags & ELW_LEFT_MOUSE) && (mx > text_arrow.pos_x) && (mx < text_arrow.pos_x + text_arrow.len_x) &&
			(my < text_arrow.pos_y) && (my > text_arrow.pos_y - text_arrow.len_y))
	{
		items_disable_text_block ^= 1;
		show_items_handler(win);
		do_click_sound();
		return 1;
	}

	if ((flags & ELW_LEFT_MOUSE) && (mx > unequip_arrow.pos_x) && (mx < unequip_arrow.pos_x + unequip_arrow.len_x) &&
		(my > unequip_arrow.pos_y) && (my < unequip_arrow.pos_y + unequip_arrow.len_y))
	{
		size_t i;
		int last_pos = 0;
		int success = 1;
		static Uint32 last_click = 0;
		if (!safe_button_click(&last_click))
			return 1;
		for(i = ITEM_WEAR_START; i < ITEM_WEAR_START + ITEM_NUM_WEAR; i++)
		{
			if(item_list[i].quantity>0)
			{
				size_t j;
				int found_slot = 0;
				if (item_list[i].is_stackable)
				{
					for (j=0; j<ITEM_WEAR_START; j++)
						if ((item_list[j].quantity>0) && (item_list[i].id == item_list[j].id) && (item_list[i].image_id == item_list[j].image_id))
						{
							if (move_item(i, j, -1))
								found_slot = 1;
							break;
						}
				}
				if (!found_slot)
					for (j=last_pos; j<ITEM_WEAR_START; j++)
					{
						if (item_list[j].quantity<1)
						{
							if (move_item(i, j, -1))
								found_slot = 1;
							last_pos = j + 1;
							break;
						}
					}
				if (!found_slot)
					success = 0;
			}
		}
		if (success)
			do_get_item_sound();
		else
			do_alert1_sound();
		item_dragged=-1;
		return 1;
	}

	if(right_click) {
		if(item_dragged!=-1 || use_item!=-1 || storage_item_dragged!=-1){
			use_item=-1;
			item_dragged=-1;
			storage_item_dragged=-1;
			item_action_mode=ACTION_WALK;
			return 1;
		}

		if((mx >= equip_grid.pos_x) && (mx < equip_grid.pos_x + equip_grid.len_x) &&
				(my >= equip_grid.pos_y) && (my < equip_grid.pos_y + equip_grid.len_y + 1)) {
			switch(item_action_mode){
				case ACTION_WALK:
					item_action_mode=ACTION_LOOK;
					break;
				case ACTION_LOOK:
				default:
					item_action_mode=ACTION_WALK;
			}
			return 1;
		} else if((mx >= quantity_grid.pos_x) && (mx < quantity_grid.pos_x + quantity_grid.len_x) &&
				(my >= quantity_grid.pos_y) && (my < quantity_grid.pos_y +  + quantity_grid.len_y)){
			//fall through...
		} else {
			switch(item_action_mode) {
			case ACTION_WALK:
				item_action_mode=ACTION_LOOK;
				break;
			case ACTION_LOOK:
				item_action_mode=ACTION_USE;
				break;
			case ACTION_USE:
				item_action_mode=ACTION_USE_WITEM;
				break;
			case ACTION_USE_WITEM:
				item_action_mode=ACTION_WALK;
				break;
			default:
				item_action_mode=ACTION_WALK;
			}
			return 1;
		}
	}

	if (item_action_mode == ACTION_USE_WITEM)
		set_gamewin_usewith_action();

	//see if we changed the quantity
	if((mx >= quantity_grid.pos_x) && (mx < quantity_grid.pos_x + quantity_grid.len_x) &&
			(my >= quantity_grid.pos_y) && (my < quantity_grid.pos_y + quantity_grid.len_y)) {
		int pos = get_mouse_pos_in_grid(mx, my, quantity_grid.cols, quantity_grid.rows, quantity_grid.pos_x, quantity_grid.pos_y, quantity_grid.width, quantity_grid.height);

		if(pos==-1){
		} else if(flags & ELW_LEFT_MOUSE){
			if(edit_quantity!=-1){
				if(!quantities.quantity[edit_quantity].len){
					//Reset the quantity
					reset_quantity(edit_quantity);
				}
				edit_quantity=-1;
			}

			item_quantity=quantities.quantity[pos].val;
			quantities.selected=pos;
		} else if(right_click){
			//Edit the given quantity
			edit_quantity=pos;
		}

		return 1;
	}

	if(edit_quantity!=-1){
		if(!quantities.quantity[edit_quantity].len)reset_quantity(edit_quantity);
		item_quantity=quantities.quantity[edit_quantity].val;
		quantities.selected=edit_quantity;
		edit_quantity=-1;
	}

	//see if we clicked on any item in the main category
	else if(mx>items_grid.pos_x && (mx < items_grid.pos_x + items_grid.len_x) &&
				my>0 && my < items_grid.len_y) {
		int pos=get_mouse_pos_in_grid(mx, my, items_grid.cols, items_grid.rows, items_grid.pos_x, items_grid.pos_y, items_grid.width, items_grid.height);

#ifdef NEW_SOUND
		if(pos>-1) {
			item_list[pos].action = ITEM_NO_ACTION;
			item_list[pos].action_time = 0;
		}
#endif // NEW_SOUND
		if(pos==-1) {
		} else if(item_dragged!=-1){
			if(item_dragged == pos){
				try_auto_equip(item_dragged);
			} else {
				if (move_item(item_dragged, pos, -1)){
					do_drop_item_sound();
				}
				else {
					do_alert1_sound();
				}
				item_dragged=-1;
			}

		}
		else if(storage_item_dragged!=-1){
			str[0]=WITHDRAW_ITEM;
			*((Uint16*)(str+1))=SDL_SwapLE16(storage_items[storage_item_dragged].pos);
			*((Uint32*)(str+3))=SDL_SwapLE32(item_quantity);
			my_tcp_send(my_socket, str, 6);
			do_drop_item_sound();
			if(storage_items[storage_item_dragged].quantity<=item_quantity) storage_item_dragged=-1;
		}
		else if(item_list[pos].quantity){
			if (ctrl_on && (items_mod_click_any_cursor || (item_action_mode==ACTION_WALK))) {
				str[0]=DROP_ITEM;
				str[1]=item_list[pos].pos;
				if(item_list[pos].is_stackable)
					*((Uint32 *)(str+2))=SDL_SwapLE32(item_list[pos].quantity);
				else
					*((Uint32 *)(str+2))=SDL_SwapLE32(36);//Drop all
				my_tcp_send(my_socket, str, 6);
				do_drop_item_sound();
			} else if (alt_on && (items_mod_click_any_cursor || (item_action_mode==ACTION_WALK))) {
				if ((get_id_MW(MW_STORAGE) >= 0) && (get_show_window_MW(MW_STORAGE)) && (view_only_storage == 0)) {
					str[0]=DEPOSITE_ITEM;
					str[1]=item_list[pos].pos;
					*((Uint32*)(str+2))=SDL_SwapLE32(INT_MAX);
					my_tcp_send(my_socket, str, 6);
					do_drop_item_sound();
				} else {
					drop_fail_time = SDL_GetTicks();
					do_alert1_sound();
				}
			} else if(item_action_mode==ACTION_LOOK) {
				click_time=cur_time;
				str[0]=LOOK_AT_INVENTORY_ITEM;
				str[1]=item_list[pos].pos;
				my_tcp_send(my_socket,str,2);
			} else if(item_action_mode==ACTION_USE) {
				if(item_list[pos].use_with_inventory){
					str[0]=USE_INVENTORY_ITEM;
					str[1]=item_list[pos].pos;
					my_tcp_send(my_socket,str,2);
					used_item_counter_action_use(pos);
#ifdef NEW_SOUND
					item_list[pos].action = USE_INVENTORY_ITEM;
#ifdef _EXTRA_SOUND_DEBUG
//					printf("Using item: %d, inv pos: %d, Image ID: %d\n", item_list[pos].pos, pos, item_list[pos].image_id);
#endif // _EXTRA_SOUND_DEBUG
#endif // NEW_SOUND
				}
			} else if(item_action_mode==ACTION_USE_WITEM) {
				if(use_item!=-1) {
					str[0]=ITEM_ON_ITEM;
					str[1]=item_list[use_item].pos;
					str[2]=item_list[pos].pos;
					my_tcp_send(my_socket,str,3);
					used_item_counter_action_use(use_item);
#ifdef NEW_SOUND
					item_list[use_item].action = ITEM_ON_ITEM;
					item_list[pos].action = ITEM_ON_ITEM;
#ifdef _EXTRA_SOUND_DEBUG
//					printf("Using item: %d on item: %d, Image ID: %d\n", pos, use_item, item_list[pos].image_id);
#endif // _EXTRA_SOUND_DEBUG
#endif // NEW_SOUND
					if (!shift_on)
						use_item=-1;
				} else {
					use_item=pos;
				}
			} else {
				item_dragged=pos;
				do_drag_item_sound();
			}
		}
	}

	// Get All button
	else if(over_button(win, mx, my)==BUT_GET){
		int x,y;
		me = get_our_actor ();
		if(!me)return(1);
		x=me->x_tile_pos;
		y=me->y_tile_pos;
		items_get_bag(x,y);
	}

	// Sto All button
	else if(over_button(win, mx, my)==BUT_STORE && get_id_MW(MW_STORAGE) >= 0 && view_only_storage == 0 && get_show_window_MW(MW_STORAGE) /*thanks alberich*/){
#ifdef STORE_ALL
		/*
		* Future code to save server load by having one byte to represent the 36 slot inventory loop. Will need server support.
		*/
		str[0]=DEPOSITE_ITEM;
		str[1]=STORE_ALL;
		my_tcp_send(my_socket, str, 2);
#else
		for(pos=((items_stoall_nofirstrow)?6:0);pos<((items_stoall_nolastrow)?30:36);pos++){
			if(item_list[pos].quantity>0){
				str[0]=DEPOSITE_ITEM;
				str[1]=item_list[pos].pos;
				*((Uint32*)(str+2))=SDL_SwapLE32(item_list[pos].quantity);
				my_tcp_send(my_socket, str, 6);
			}
		}
#endif
	}

	// Drop All button
	else if(over_button(win, mx, my)==BUT_DROP){
		drop_all_handler();
	}

	// Mix One/All button
	else if(over_button(win, mx, my)==BUT_MIX){
		if (items_mix_but_all)
			mix_handler(255, mixbut_empty_str);
		else
			mix_handler(1, mixbut_empty_str);
	}

	// Item List button
	else if (over_button(win, mx, my)==BUT_ITEM_LIST)
		toggle_items_list_window(win);

	//see if we clicked on any item in the wear category
	else if((mx > equip_grid.pos_x) && (mx < equip_grid.pos_x + equip_grid.len_x) &&
				(my > equip_grid.pos_y) && (my < equip_grid.pos_y + equip_grid.len_y)){
		int pos = ITEM_WEAR_START + get_mouse_pos_in_grid(mx, my, equip_grid.cols, equip_grid.rows, equip_grid.pos_x, equip_grid.pos_y, equip_grid.width, equip_grid.height);

		if(pos < ITEM_WEAR_START) {
		} else if(item_list[pos].quantity){
			if(item_action_mode == ACTION_LOOK) {
				str[0]=LOOK_AT_INVENTORY_ITEM;
				str[1]=item_list[pos].pos;
				my_tcp_send(my_socket, str, 2);
			} else if(item_dragged==-1 && left_click) {
				item_dragged=pos;
				do_drag_item_sound();
			}
			else if(item_dragged!=-1 && left_click) {
				int can_move = (item_dragged == pos) || allow_equip_swap;
				if (can_move && move_item(pos, 0, -1)) {
					equip_item(item_dragged, pos);
					if (item_dragged != pos)
						prep_move_to_vacated_slot(item_dragged);
					do_get_item_sound();
				}
				else {
					do_alert1_sound();
					set_shown_string(c_red2, items_cannot_equip_str);
				}
				item_dragged=-1;
			}
		} else if(item_dragged!=-1){
			equip_item(item_dragged, pos);
			item_dragged=-1;
			do_drop_item_sound();
		}
	}

	// clear the message area if double-clicked
	else if (!items_disable_text_block && (mx > message_box.pos_x) && (mx < message_box.pos_x + message_box.len_x) &&
			(my > message_box.pos_y) && (my < message_box.pos_y + message_box.len_y)) {
		static Uint32 last_click = 0;
		if (safe_button_click(&last_click)) {
			set_shown_string(0,"");
			return 1;
		}
	}

	return 1;
}

static void set_description_help(int pos)
{
	Uint16 item_id = item_list[pos].id;
	int image_id = item_list[pos].image_id;
	if (show_item_desc_text && item_info_available() && (get_item_count(item_id, image_id) == 1))
		item_desc_str = get_item_description(item_id, image_id);
}

static int mouseover_items_handler(window_info *win, int mx, int my) {
	int pos;

	// check and record if mouse if over a button
	if ((buttons_grid.mouse_over = over_button(win, mx, my)) != -1)
		return 0; // keep standard cursor

	if ((mx > text_arrow.pos_x) && (mx < text_arrow.pos_x + text_arrow.len_x) &&
			(my < text_arrow.pos_y) && (my > text_arrow.pos_y - text_arrow.len_y))
	{
		item_help_str = items_text_toggle_help_str;
		text_arrow.mouse_over = 1;
		return 0;
	}

	if ((mx > unequip_arrow.pos_x) && (mx < unequip_arrow.pos_x + unequip_arrow.len_x) &&
			(my > unequip_arrow.pos_y) && (my < unequip_arrow.pos_y + unequip_arrow.len_y))
	{
		item_help_str = (disable_double_click) ?items_unequip_all_help_str :items_doubleclick_unequip_all_help_str;
		unequip_arrow.mouse_over = 1;
		return 0;
	}

	if((mx > items_grid.pos_x) && (mx < items_grid.pos_x + items_grid.len_x) && (my > 0) && (my < items_grid.len_y)) {
		pos = get_mouse_pos_in_grid(mx, my, items_grid.cols, items_grid.rows, items_grid.pos_x, items_grid.pos_y, items_grid.width, items_grid.height);

		if(pos==-1) {
		} else if(item_list[pos].quantity){
			set_description_help(pos);
			if ((item_dragged == -1) && (items_mod_click_any_cursor || (item_action_mode==ACTION_WALK)))
					item_help_str = mod_click_item_help_str;
			if(item_action_mode==ACTION_LOOK) {
				elwin_mouse=CURSOR_EYE;
			} else if(item_action_mode==ACTION_USE) {
				elwin_mouse=CURSOR_USE;
			} else if(item_action_mode==ACTION_USE_WITEM) {
				elwin_mouse=CURSOR_USE_WITEM;
				if (use_item!=-1)
					item_help_str = multiuse_item_help_str;
			} else {
				elwin_mouse=CURSOR_PICK;
			}
			items_grid.mouse_over = pos;

			return 1;
		}
	} else if((mx > equip_grid.pos_x) && (mx < equip_grid.pos_x + equip_grid.len_x) &&
			(my > equip_grid.pos_y) && (my < equip_grid.pos_y + equip_grid.len_y)) {
		pos = ITEM_WEAR_START + get_mouse_pos_in_grid(mx, my, equip_grid.cols, equip_grid.rows, equip_grid.pos_x, equip_grid.pos_y, equip_grid.width, equip_grid.height);
		item_help_str = equip_here_str;
		if(pos==-1) {
		} else if(item_list[pos].quantity){
			set_description_help(pos);
			if(item_action_mode==ACTION_LOOK) {
				elwin_mouse=CURSOR_EYE;
			} else if(item_action_mode==ACTION_USE) {
				elwin_mouse=CURSOR_USE;
			} else if(item_action_mode==ACTION_USE_WITEM) {
				elwin_mouse=CURSOR_USE_WITEM;
			} else {
				elwin_mouse=CURSOR_PICK;
			}
			return 1;
		}
	} else if(show_help_text && (mx > quantity_grid.pos_x) && (mx < quantity_grid.pos_x +  quantity_grid.len_x) &&
			(my > quantity_grid.pos_y) && (my < quantity_grid.pos_y + quantity_grid.len_y)){
		item_help_str = quantity_edit_str;
	} else if (show_help_text && *inventory_item_string && !items_disable_text_block &&
			(mx > message_box.pos_x) && (mx < message_box.pos_x + message_box.len_x) &&
			(my > message_box.pos_y) && (my < message_box.pos_y + message_box.len_y)) {
		item_help_str = (disable_double_click)?click_clear_str :double_click_clear_str;
	}

	return 0;
}

static int keypress_items_handler(window_info * win, int x, int y, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	if(edit_quantity!=-1){
		char * str=quantities.quantity[edit_quantity].str;
		int * len=&quantities.quantity[edit_quantity].len;
		int * val=&quantities.quantity[edit_quantity].val;

		if(key_code == SDLK_DELETE){
			reset_quantity(edit_quantity);
			edit_quantity=-1;
			return 1;
		} else if(key_code == SDLK_BACKSPACE){
			if(*len>0){
				(*len)--;
				str[*len]=0;
				*val=atoi(str);
			}
			return 1;
		} else if(key_code == SDLK_RETURN || key_code == SDLK_KP_ENTER){
			if(!*val){
				reset_quantity(edit_quantity);
			}
			item_quantity=*val;
			quantities.selected=edit_quantity;
			edit_quantity=-1;
			return 1;
		} else if(key_code == SDLK_ESCAPE){
			reset_quantity(edit_quantity);
			edit_quantity=-1;
			return 1;
		} else if(key_unicode >= '0' && key_unicode <= '9'){
			if (*len<5)
			{
				str[*len]=key_unicode;
				(*len)++;
				str[*len]=0;
				*val=atoi(str);
			}
			return 1;
		}
	}

	return 0;
}

static void drop_all_handler (void)
{
	Uint8 str[6] = {0};
	int i;
	int dropped_something = 0;
	static Uint32 last_click = 0;

	/* provide some protection for inadvertent pressing (double click that can be disabled) */
	if (safe_button_click(&last_click))
	{
		for(i = 0; i < ITEM_NUM_ITEMS; i++)
		{
			if (item_list[i].quantity != 0 &&  // only drop stuff that we're not wearing or not excluded
				item_list[i].pos >= ((items_dropall_nofirstrow)?6:0) &&
				item_list[i].pos < ((items_dropall_nolastrow)?30:ITEM_WEAR_START))
			{
				str[0] = DROP_ITEM;
				str[1] = item_list[i].pos;
				*((Uint32 *)(str+2)) = SDL_SwapLE32(item_list[i].quantity);
				my_tcp_send (my_socket, str, 6);
				dropped_something = 1;
			}
		}
		if (dropped_something)
			do_drop_item_sound();
	}
}

static int show_items_handler(window_info * win)
{
	int i, win_x_len, win_y_len;
	int seperator = (int)(0.5 + win->current_scale * 5);

	if (!manual_size_items_window)
		use_small_items_window = ((window_height<=600) || (window_width<=800));

	items_grid.cols = 6;
	items_grid.rows = 6;
	items_grid.width = (int)(0.5 + win->current_scale * ((use_small_items_window) ?33: 50));
	items_grid.height = (int)(0.5 + win->current_scale * ((use_small_items_window) ?33: 50));
	items_grid.len_x = items_grid.width * items_grid.cols;
	items_grid.len_y = items_grid.height * items_grid.rows;

	equip_grid.cols = 2;
	equip_grid.rows = 4;
	equip_grid.width = (int)(0.5 + win->current_scale * 33);
	equip_grid.height = (int)(0.5 + win->current_scale * 33);
	equip_grid.len_x = equip_grid.width * equip_grid.cols;
	equip_grid.len_y = equip_grid.height * equip_grid.rows;;

	/* the buttons */
	button_text_zoom = win->current_scale_small
		* min2f(1.0, 0.5 * equip_grid.height / win->small_font_len_y);
	buttons_grid.cols = 1;
	buttons_grid.rows = NUMBUT;
	buttons_grid.width = 4 * get_max_char_width_zoom(win->font_category, button_text_zoom);
	buttons_grid.height = equip_grid.height;
	buttons_grid.len_x = buttons_grid.width * buttons_grid.cols;
	buttons_grid.len_y = buttons_grid.height * buttons_grid.rows;

	/* we can have the eqipment grid and or the button bar on the left of right */
	if (items_buttons_on_left)
	{
		buttons_grid.pos_x = 0;
		if (items_equip_grid_on_left)
		{
			equip_grid.pos_x = buttons_grid.pos_x + buttons_grid.len_x + seperator;
			items_grid.pos_x = equip_grid.pos_x + equip_grid.len_x + seperator;
		}
		else
		{
			items_grid.pos_x = buttons_grid.pos_x + buttons_grid.len_x + seperator;
			equip_grid.pos_x = items_grid.pos_x + items_grid.len_x + seperator;
		}
		buttons_grid.pos_y = items_grid.height;
		items_grid.pos_y = 0;
		equip_grid.pos_y = items_grid.height;
	}
	else
	{
		if (items_equip_grid_on_left)
		{
			equip_grid.pos_x = 0;
			items_grid.pos_x = equip_grid.pos_x + equip_grid.len_x + seperator;
			buttons_grid.pos_x = items_grid.pos_x + items_grid.len_x + seperator;
		}
		else
		{
			items_grid.pos_x = 0;
			equip_grid.pos_x = items_grid.pos_x + items_grid.len_x + seperator;
			buttons_grid.pos_x = equip_grid.pos_x + equip_grid.len_x + seperator;
		}
		items_grid.pos_y = 0;
		equip_grid.pos_y = items_grid.height;
		buttons_grid.pos_y = items_grid.height;
	}

	text_arrow.len_x = unequip_arrow.len_x = equip_grid.width * 0.4;
	text_arrow.len_y = unequip_arrow.len_y = equip_grid.height * 0.4;
	unequip_arrow.pos_x = equip_grid.pos_x + ((items_equip_grid_on_left) ?equip_grid.width :0) + (equip_grid.width - unequip_arrow.len_x) / 2;
	unequip_arrow.pos_y = equip_grid.pos_y + equip_grid.len_y + (equip_grid.height - unequip_arrow.len_y) / 2;

	/* we can finally calculate the maximum y value so far .... */
	win_y_len = ((items_grid.pos_y + items_grid.len_y) > (equip_grid.pos_y + equip_grid.len_y + text_arrow.len_y + seperator)) ?(items_grid.pos_y + items_grid.len_y) : (equip_grid.pos_y + equip_grid.len_y + text_arrow.len_y + seperator);
	win_y_len = ((buttons_grid.pos_y + buttons_grid.len_y) > win_y_len) ?(buttons_grid.pos_y + buttons_grid.len_y) :win_y_len;

	/* then we can finally poistion the message box arrow */
	text_arrow.pos_x = equip_grid.pos_x + ((items_equip_grid_on_left) ?0 :equip_grid.width) + (equip_grid.width - text_arrow.len_x) / 2;
	text_arrow.pos_y = win_y_len;

	/* calculate the window width needed for all the components */
	win_x_len = items_grid.len_x + equip_grid.len_x + buttons_grid.len_x + 2 * seperator + ((items_buttons_on_left && items_equip_grid_on_left) ?win->box_size :0);

	/* the text message box */
	message_box.rows = 4;
	message_box.cols = 1;
	message_box.len_x = win_x_len - 8;
	message_box.len_y = win->small_font_len_y * message_box.rows;
	message_box.pos_x = 4;
	if (items_disable_text_block)
		message_box.pos_y = 0;
	else
	{
		message_box.pos_y = win_y_len + seperator;
		win_y_len += message_box.len_y;
	}

	/* the load and quanity labels */
	win_y_len += seperator;
	labels_box.rows = 1;
	labels_box.cols = 1;
	labels_box.len_x = win_x_len - 8;
	labels_box.len_y = win->small_font_len_y * labels_box.rows;
	labels_box.pos_x = 4;
	labels_box.pos_y = win_y_len;
	win_y_len += labels_box.len_y;

	/* the quanity numbers */
	item_quantity = quantities.quantity[quantities.selected].val;
	quantity_grid.cols = ITEM_EDIT_QUANT;
	quantity_grid.rows = 1;
	quantity_grid.width = (int)((float)win_x_len / (float)quantity_grid.cols);
	quantity_grid.height = win->small_font_len_y + seperator;
	quantity_grid.len_x = quantity_grid.width * quantity_grid.cols;
	quantity_grid.len_y = quantity_grid.height * quantity_grid.rows;
	quantity_grid.pos_x = (win_x_len - quantity_grid.len_x) / 2;
	quantity_grid.pos_y = win_y_len;
	win_y_len += quantity_grid.height;

	resize_window(win->window_id, win_x_len, win_y_len);

	cm_remove_regions(win->window_id);
	for (i=0; i<NUMBUT; i++)
		cm_add_region(buttons_cm_id[i], win->window_id, buttons_grid.pos_x, buttons_grid.pos_y + buttons_grid.height * i, buttons_grid.width, buttons_grid.height);

	/* make sure we redraw any string */
	last_items_string_id = 0;

	return 1;
}

static int context_items_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	if (option<ELW_CM_MENU_LEN)
		return cm_title_handler(win, widget_id, mx, my, option);
	switch (option)
	{
		case ELW_CM_MENU_LEN+1: manual_size_items_window = 1; show_items_handler(win); break;
		case ELW_CM_MENU_LEN+2: case ELW_CM_MENU_LEN+6: case ELW_CM_MENU_LEN+7: show_items_handler(win); break;
		case ELW_CM_MENU_LEN+9: send_input_text_line("#sto", 4); break;
	}
	return 1;
}

void display_items_menu()
{
	int items_win = get_id_MW(MW_ITEMS);
	if(items_win < 0){
		items_win = create_window(win_inventory, (not_on_top_now(MW_ITEMS) ?game_root_win : -1), 0, get_pos_x_MW(MW_ITEMS), get_pos_y_MW(MW_ITEMS),
			0, 0, ELW_USE_UISCALE|ELW_WIN_DEFAULT);
		set_id_MW(MW_ITEMS, items_win);

		set_window_custom_scale(items_win, MW_ITEMS);
		set_window_handler(items_win, ELW_HANDLER_DISPLAY, &display_items_handler );
		set_window_handler(items_win, ELW_HANDLER_CLICK, &click_items_handler );
		set_window_handler(items_win, ELW_HANDLER_MOUSEOVER, &mouseover_items_handler );
		set_window_handler(items_win, ELW_HANDLER_KEYPRESS, (int (*)())&keypress_items_handler );
		set_window_handler(items_win, ELW_HANDLER_SHOW, &show_items_handler );
		set_window_handler(items_win, ELW_HANDLER_UI_SCALE, &show_items_handler );
		set_window_handler(items_win, ELW_HANDLER_FONT_CHANGE, &show_items_handler);

		cm_add(windows_list.window[items_win].cm_id, cm_items_menu_str, context_items_handler);
		cm_bool_line(windows_list.window[items_win].cm_id, ELW_CM_MENU_LEN+1, &use_small_items_window, NULL);
		cm_bool_line(windows_list.window[items_win].cm_id, ELW_CM_MENU_LEN+2, &manual_size_items_window, NULL);
		cm_bool_line(windows_list.window[items_win].cm_id, ELW_CM_MENU_LEN+3, &item_window_on_drop, "item_window_on_drop");
		cm_bool_line(windows_list.window[items_win].cm_id, ELW_CM_MENU_LEN+4, &allow_equip_swap, NULL);
		cm_bool_line(windows_list.window[items_win].cm_id, ELW_CM_MENU_LEN+5, &items_mod_click_any_cursor, NULL);
		cm_bool_line(windows_list.window[items_win].cm_id, ELW_CM_MENU_LEN+6, &items_buttons_on_left, NULL);
		cm_bool_line(windows_list.window[items_win].cm_id, ELW_CM_MENU_LEN+7, &items_equip_grid_on_left, NULL);

		buttons_cm_id[BUT_STORE] = cm_create(inv_keeprow_str, NULL);
		cm_bool_line(buttons_cm_id[BUT_STORE], 0, &items_stoall_nofirstrow, NULL);
		cm_bool_line(buttons_cm_id[BUT_STORE], 1, &items_stoall_nolastrow, NULL);

		buttons_cm_id[BUT_DROP] = cm_create(inv_keeprow_str, NULL);
		cm_bool_line(buttons_cm_id[BUT_DROP], 0, &items_dropall_nofirstrow, NULL);
		cm_bool_line(buttons_cm_id[BUT_DROP], 1, &items_dropall_nolastrow, NULL);

		buttons_cm_id[BUT_MIX] = cm_create(mix_all_str, NULL);
		cm_bool_line(buttons_cm_id[BUT_MIX], 0, &items_mix_but_all, NULL);

		buttons_cm_id[BUT_GET] = cm_create(auto_get_all_str, NULL);
		cm_bool_line(buttons_cm_id[BUT_GET], 0, &items_auto_get_all, NULL);

		buttons_cm_id[BUT_ITEM_LIST] = cm_create(item_list_but_str, NULL);
		cm_bool_line(buttons_cm_id[BUT_ITEM_LIST], 0, &items_list_on_left, NULL);

		show_items_handler(&windows_list.window[items_win]);
		check_proportional_move(MW_ITEMS);

	} else {
		show_window(items_win);
		select_window(items_win);
	}
}

void get_items_cooldown (const Uint8 *data, int len)
{
	int iitem, nitems, ibyte, pos;
	Uint8 cooldown, max_cooldown;

	// reset old cooldown values
	for (iitem = 0; iitem < ITEM_NUM_ITEMS; iitem++)
	{
		item_list[iitem].cooldown_time = 0;
		item_list[iitem].cooldown_rate = 1;
	}

	nitems = len / 5;
	if (nitems <= 0) return;

	ibyte = 0;
	for (iitem = 0; iitem < nitems; iitem++)
	{
		pos = data[ibyte];
		max_cooldown = SDL_SwapLE16 (*((Uint16*)(&data[ibyte+1])));
		cooldown = SDL_SwapLE16 (*((Uint16*)(&data[ibyte+3])));
		ibyte += 5;

		item_list[pos].cooldown_rate = 1000 * (Uint32)max_cooldown;
		item_list[pos].cooldown_time = cur_time + 1000 * (Uint32)cooldown;
	}
}
