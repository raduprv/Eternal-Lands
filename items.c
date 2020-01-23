#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "items.h"
#include "asc.h"
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

int item_action_mode=ACTION_WALK;

int items_win= -1;
int items_menu_x=10;
int items_menu_y=20;
static int items_grid_size=0;

int items_text[MAX_ITEMS_TEXTURES];

static char items_string[350]={0};
static size_t last_items_string_id = 0;
int item_dragged=-1;
int item_quantity=1;
static int quantity_width=0;
static int quantity_height = 0;
static int text_y_offset = 0;
int allow_equip_swap=0;
int use_item=-1;

static int wear_grid_size = 0;
static int wear_items_x_offset=0;
static int wear_items_y_offset=0;

static int quantity_x_offset=0;
static int quantity_y_offset=0;

int use_small_items_window = 0;
int manual_size_items_window = 0;
int items_mod_click_any_cursor = 1;

int item_uid_enabled = 0;
const Uint16 unset_item_uid = (Uint16)-1;

#define NUMBUT 5
static int but_len_x = 0;
static int but_len_y = 0;
static int but_x_offset = 0;
static int but_y_off[NUMBUT] = { 0, 0, 0, 0, 0 };
enum { BUT_STORE, BUT_GET, BUT_DROP, BUT_MIX, BUT_ITEM_LIST };
int items_mix_but_all = 0;
int items_stoall_nofirstrow = 0;
int items_stoall_nolastrow = 0;
int items_dropall_nofirstrow = 0;
int items_dropall_nolastrow = 0;
int items_list_on_left = 0;
static const char *item_help_str = NULL;
static const char *item_desc_str = NULL;
static int mouse_over_but = -1;
static size_t cm_stoall_but = CM_INIT_VALUE;
static size_t cm_dropall_but = CM_INIT_VALUE;
static size_t cm_mix_but = CM_INIT_VALUE;
static size_t cm_getall_but = CM_INIT_VALUE;
static size_t cm_itemlist_but = CM_INIT_VALUE;
static int mouseover_item_pos = -1;
int items_disable_text_block = 0;
static int text_arrow_x = 0;
static int text_arrow_y = 0;
static int text_arrow_width = 0;
static int text_arrow_height = 0;
static int mouse_over_text_arrow = 0;
static struct { int last_dest; int move_to; int move_from; size_t string_id; Uint32 start_time; } swap_complete = {-1, -1, -1, 0, 0};

static int show_items_handler(window_info * win);
static void drop_all_handler(void);

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
	if (mx>(but_x_offset) && mx<win->len_x-3 && my>wear_items_y_offset
		&& my<wear_items_y_offset+but_y_off[NUMBUT-1]+but_len_y) {
		return (my - wear_items_y_offset) / but_len_y;
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
	char str[20];
	
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
		if(item_list[item].is_stackable){
			if(quantity>item_list[item].quantity)quantity=item_list[item].quantity;
		} else quantity=-1;//The quantity for non-stackable items is misleading so don't show it...
	}

	cur_item_img=cur_item%25;
	get_item_uv(cur_item_img, &u_start, &v_start, &u_end, &v_end);

	//get the texture this item belongs to
	this_texture=get_items_texture(cur_item/25);

	bind_texture(this_texture);
	glBegin(GL_QUADS);
	draw_2d_thing(u_start, v_start, u_end, v_end, mouse_x - offset, mouse_y - offset, mouse_x + offset, mouse_y + offset);
	glEnd();
	
	if(!mini && quantity!=-1){
		safe_snprintf(str,sizeof(str),"%i",quantity);
		draw_string_small_zoomed(mouse_x-offset, mouse_y + offset - get_global_scale() * SMALL_FONT_Y_LEN, (unsigned char*)str, 1, get_global_scale());
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
			used_item_counter_queue[pos]--;
			if (!used_item_counter_queue[pos])
				used_item_counter_queue_time[pos] = 0;
			increment_used_item_counter(get_item_description(item_list[pos].id, item_list[pos].image_id), item_list[pos].quantity - new_quanity);
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
			if (!move_item(swap_complete.move_from, swap_complete.move_to))
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


int display_items_handler(window_info *win)
{
	char str[80];
	char my_str[10];
	int x,y,i;
	int item_is_weared=0;
	Uint32 _cur_time = SDL_GetTicks(); /* grab a snapshot of current time */
	char *but_labels[NUMBUT] = { sto_all_str, get_all_str, drp_all_str, NULL, itm_lst_str };

	glEnable(GL_TEXTURE_2D);

	// stop expecting swap completion if we get a message, or we timeout (catch all)
	if (swap_complete.move_from != -1)
	{
		if (swap_complete.string_id != inventory_item_string_id)
			swap_complete.move_from = swap_complete.move_to = -1;
		if (SDL_GetTicks() > swap_complete.start_time + 2000)
			swap_complete.move_from = swap_complete.move_to = -1;
	}

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
		int text_x_offset = (int)(0.5 + ((but_len_x - (float)(3 * win->small_font_len_x)) / 2.0)) + gx_adjust - 1;
		int text_y_offset = (int)(0.5 + ((but_len_y - (float)(2 * win->small_font_len_y)) / 2.0)) + gy_adjust;
		strap_word(but_labels[i],my_str);
		glColor3f(0.77f,0.57f,0.39f);
		draw_string_small_zoomed(but_x_offset+text_x_offset, wear_items_y_offset+but_y_off[i]+text_y_offset, (unsigned char*)my_str, 2, win->current_scale);
	}

	x=quantity_x_offset+quantity_width/2;
	y=quantity_y_offset + (quantity_height - win->small_font_len_y) / 2;
	glColor3f(0.3f,0.5f,1.0f);
	for(i=0;i<ITEM_EDIT_QUANT;x+=quantity_width,++i){
		if(i==edit_quantity){
			glColor3f(1.0f, 0.0f, 0.3f);
			draw_string_small_zoomed(1+gx_adjust+x-strlen(quantities.quantity[i].str)*win->small_font_len_x/2, y+gy_adjust, (unsigned char*)quantities.quantity[i].str, 1, win->current_scale);
			glColor3f(0.3f, 0.5f, 1.0f);
		} else if(i==quantities.selected){
			glColor3f(0.0f, 1.0f, 0.3f);
			draw_string_small_zoomed(1+gx_adjust+x-strlen(quantities.quantity[i].str)*win->small_font_len_x/2, y+gy_adjust, (unsigned char*)quantities.quantity[i].str, 1, win->current_scale);
			glColor3f(0.3f, 0.5f, 1.0f);
		} else draw_string_small_zoomed(1+gx_adjust+x-strlen(quantities.quantity[i].str)*win->small_font_len_x/2, y+gy_adjust, (unsigned char*)quantities.quantity[i].str, 1, win->current_scale);
	}
	draw_string_small_zoomed(win->len_x-strlen(quantity_str)*win->small_font_len_x-5, quantity_y_offset-quantity_height-1, (unsigned char*)quantity_str, 1, win->current_scale);

	glColor3f(0.57f,0.67f,0.49f);
	draw_string_small_zoomed (wear_items_x_offset + wear_grid_size - (win->small_font_len_x * strlen(equip_str))/2, wear_items_y_offset-win->small_font_len_y-3, (unsigned char*)equip_str, 1, win->current_scale);

	glColor3f(1.0f,1.0f,1.0f);
	//ok, now let's draw the objects...
	for(i=ITEM_NUM_ITEMS-1;i>=0;i--){
		if(item_list[i].quantity){
			int cur_pos;
			int x_start,x_end,y_start,y_end;

			// don't display an item that is in the proces of being moved after equipment swap
			if (i == swap_complete.move_from)
				continue;

			//get the x and y
			cur_pos=i;
			if(cur_pos>=ITEM_WEAR_START){//the items we 'wear' are smaller
				cur_pos-=ITEM_WEAR_START;
				item_is_weared=1;
				x_start=wear_items_x_offset+wear_grid_size*(cur_pos%2)+1;
				x_end=x_start+wear_grid_size-1;
				y_start=wear_items_y_offset+wear_grid_size*(cur_pos/2);
				y_end=y_start+wear_grid_size-1;
				draw_item(item_list[i].image_id,x_start,y_start,wear_grid_size-1);
			} else {
				item_is_weared=0;
				x_start=items_grid_size*(cur_pos%6)+1;
				x_end=x_start+items_grid_size-1;
				y_start=items_grid_size*(cur_pos/6);
				y_end=y_start+items_grid_size-1;
				draw_item(item_list[i].image_id,x_start,y_start,items_grid_size - 1);
			}


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
				int use_large = (mouseover_item_pos == i) && enlarge_text();
				safe_snprintf(str, sizeof(str), "%i", item_list[i].quantity);
				y_end -= (i&1) ?items_grid_size-1 : ((use_large) ?win->default_font_len_y :win->small_font_len_y);
				if (use_large)
					draw_string_shadowed_zoomed(x_start, y_end, (unsigned char*)str, 1,1.0f,1.0f,1.0f, 0.0f, 0.0f, 0.0f, win->current_scale);
				else
					draw_string_small_shadowed_zoomed(x_start, y_end, (unsigned char*)str, 1,1.0f,1.0f,1.0f, 0.0f, 0.0f, 0.0f, win->current_scale);
			}
		}
	}
	mouseover_item_pos = -1;

	glColor3f(1.0f,1.0f,1.0f);

	//draw the load string
	safe_snprintf(str, sizeof(str), "%s: %i/%i", attributes.carry_capacity.shortname, your_info.carry_capacity.cur, your_info.carry_capacity.base);
	draw_string_small_zoomed(2, quantity_y_offset-quantity_height-1, (unsigned char*)str, 1, win->current_scale);

	//now, draw the inventory text, if any.
	if (!items_disable_text_block)
	{
		if (last_items_string_id != inventory_item_string_id)
		{
			put_small_text_in_box_zoomed((unsigned char*)inventory_item_string, strlen(inventory_item_string), win->len_x-8, items_string, win->current_scale);
			last_items_string_id = inventory_item_string_id;
		}
		draw_string_small_zoomed(4, win->len_y - text_y_offset, (unsigned char*)items_string, 4, win->current_scale);
	}

	glDisable(GL_TEXTURE_2D);

	if (mouse_over_text_arrow)
		glColor3f(0.99f,0.77f,0.55f);
	else
		glColor3f(0.77f,0.57f,0.39f);
	mouse_over_text_arrow = 0;
	if (items_disable_text_block)
	{
		glBegin(GL_LINES); /* Dn arrow */
			glVertex3i(text_arrow_x, text_arrow_y - text_arrow_height, 0);
			glVertex3i(text_arrow_x + text_arrow_width/2, text_arrow_y, 0);
			glVertex3i(text_arrow_x + text_arrow_width/2, text_arrow_y, 0);
			glVertex3i(text_arrow_x + text_arrow_width, text_arrow_y - text_arrow_height, 0);
		glEnd();
	}
	else
	{
		glBegin(GL_LINES); /* Up arrow */
			glVertex3i(text_arrow_x, text_arrow_y, 0);
			glVertex3i(text_arrow_x + text_arrow_width/2, text_arrow_y - text_arrow_height, 0);
			glVertex3i(text_arrow_x + text_arrow_width/2, text_arrow_y - text_arrow_height, 0);
			glVertex3i(text_arrow_x + text_arrow_width, text_arrow_y, 0);
		glEnd();
	}

	// Render the grid *after* the images. It seems impossible to code
	// it such that images are rendered exactly within the boxes on all 
	// cards
	glColor3f(0.77f,0.57f,0.39f);

	//draw the grids
	rendergrid(6, 6, 0, 0, items_grid_size, items_grid_size);
	
	glColor3f(0.57f,0.67f,0.49f);
	rendergrid(2, 4, wear_items_x_offset, wear_items_y_offset, wear_grid_size, wear_grid_size);
	
	// draw the button boxes
	glColor3f(0.77f,0.57f,0.39f);
	for (i=0; i<NUMBUT; i++) {
		glBegin(GL_LINE_LOOP);
			glVertex3i(win->len_x-3, wear_items_y_offset+but_y_off[i],0);
			glVertex3i(but_x_offset, wear_items_y_offset+but_y_off[i],0);
			glVertex3i(but_x_offset, wear_items_y_offset+but_y_off[i]+but_len_y,0);
			glVertex3i(win->len_x-3, wear_items_y_offset+but_y_off[i]+but_len_y,0);
		glEnd();
	}
	
	// highlight a button with the mouse over
	if (mouse_over_but != -1)
	{
		glColor3f(0.99f,0.77f,0.55f);
		glBegin(GL_LINE_LOOP);
			glVertex3i(win->len_x-2, wear_items_y_offset+but_y_off[mouse_over_but]-1,0);
			glVertex3i(but_x_offset-1, wear_items_y_offset+but_y_off[mouse_over_but]-1,0);
			glVertex3i(but_x_offset-1, wear_items_y_offset+but_y_off[mouse_over_but]+but_len_y+1,0);
			glVertex3i(win->len_x-2, wear_items_y_offset+but_y_off[mouse_over_but]+but_len_y+1,0);
		glEnd();
	}

	//now, draw the quantity boxes
	glColor3f(0.3f,0.5f,1.0f);
	rendergrid(ITEM_EDIT_QUANT, 1, quantity_x_offset, quantity_y_offset, quantity_width, quantity_height);
	
	glEnable(GL_TEXTURE_2D);
	
	// display help text for button if mouse over one
	if ((mouse_over_but != -1) && show_help_text) {
		char *helpstr[NUMBUT] = { stoall_help_str, getall_help_str, ((disable_double_click) ?drpall_help_str :dcdrpall_help_str), mixoneall_help_str, itmlst_help_str };
		show_help(helpstr[mouse_over_but], 0, win->len_y+10, win->current_scale);
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

	mouse_over_but = -1;
	
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 1;
}


/* return 1 if sent the move command */
int move_item(int item_pos_to_mov, int destination_pos)
{
	int drop_on_stack = 0;
	swap_complete.last_dest = -1;
	/* if the dragged item is equipped and the destintion is occupied, try to find another slot */
	if ((item_pos_to_mov >= ITEM_WEAR_START) && (item_list[destination_pos].quantity)){
		int i;
		int have_free_pos = 0;
		/* find first free slot, use a free slot in preference to a stack as the server does the stacking */
		for (i = 0; i < ITEM_WEAR_START; i++){
			if (!item_list[i].quantity){
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


int click_items_handler(window_info *win, int mx, int my, Uint32 flags)
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

	if ((flags & ELW_LEFT_MOUSE) && (mx > text_arrow_x) && (mx < text_arrow_x + text_arrow_width) &&
			(my < text_arrow_y) && (my > text_arrow_y - text_arrow_height))
	{
		items_disable_text_block ^= 1;
		show_items_handler(win);
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
		
		if(mx>=wear_items_x_offset && mx<wear_items_x_offset+wear_grid_size*2 && my>=wear_items_y_offset && my<wear_items_y_offset+wear_grid_size*4+1) {
			switch(item_action_mode){
				case ACTION_WALK:
					item_action_mode=ACTION_LOOK;
					break;
				case ACTION_LOOK:
				default:
					item_action_mode=ACTION_WALK;
			}
			return 1;
		} else if(mx>=quantity_x_offset && mx<quantity_x_offset+ITEM_EDIT_QUANT*quantity_width && my>=quantity_y_offset && my<quantity_y_offset+quantity_height){
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
	
	if(item_action_mode==ACTION_USE_WITEM)	action_mode=ACTION_USE_WITEM;

	//see if we changed the quantity
	if(mx>=quantity_x_offset && mx<quantity_x_offset+ITEM_EDIT_QUANT*quantity_width
			&& my>=quantity_y_offset && my<quantity_y_offset+quantity_height) {
		int pos=get_mouse_pos_in_grid(mx, my, ITEM_EDIT_QUANT, 1, quantity_x_offset, quantity_y_offset, quantity_width, quantity_height);

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
	else if(mx>0 && mx < 6*items_grid_size &&
				my>0 && my < 6*items_grid_size) {
		int pos=get_mouse_pos_in_grid(mx, my, 6, 6, 0, 0, items_grid_size, items_grid_size);
		
#ifdef NEW_SOUND
		if(pos>-1) {
			item_list[pos].action = ITEM_NO_ACTION;
			item_list[pos].action_time = 0;
		}
#endif // NEW_SOUND
		if(pos==-1) {
		} else if(item_dragged!=-1){
			if(item_dragged == pos){ //let's try auto equip
				int i;
				for(i = ITEM_WEAR_START; i<ITEM_WEAR_START+8;i++) {
					if(item_list[i].quantity<1) {
						move_item(pos,i);
						item_dragged=-1;
						break;
					}
				}
			} else {
				if (move_item(item_dragged, pos)){
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
				if ((storage_win >= 0) && (get_show_window(storage_win)) && (view_only_storage == 0)) {
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
	else if(over_button(win, mx, my)==BUT_STORE && storage_win >= 0 && view_only_storage == 0 && get_show_window(storage_win) /*thanks alberich*/){
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
	else if(mx>wear_items_x_offset && mx<wear_items_x_offset+2*wear_grid_size &&
				my>wear_items_y_offset && my<wear_items_y_offset+4*wear_grid_size){
		int pos=36+get_mouse_pos_in_grid(mx, my, 2, 4, wear_items_x_offset, wear_items_y_offset, wear_grid_size, wear_grid_size);
		
		if(pos<36) {
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
				if (can_move && move_item(pos, 0)) {
					equip_item(item_dragged, pos);
					if (item_dragged != pos) // start to process to move removed item to the vacated slot
					{
						swap_complete.move_from = swap_complete.last_dest;
						swap_complete.move_to = item_dragged;
						swap_complete.string_id = inventory_item_string_id;
						swap_complete.start_time = SDL_GetTicks();
					}
					do_get_item_sound();
				}
				else {
					do_alert1_sound();
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
	else if (my > (win->len_y - text_y_offset)) {
		static Uint32 last_click = 0;
		if (safe_button_click(&last_click)) {
			set_shown_string(0,"");
			return 1;
		}
	}
	
	return 1;
}

void set_description_help(int pos)
{
	Uint16 item_id = item_list[pos].id;
	int image_id = item_list[pos].image_id;
	if (show_item_desc_text && item_info_available() && (get_item_count(item_id, image_id) == 1))
		item_desc_str = get_item_description(item_id, image_id);
}

int mouseover_items_handler(window_info *win, int mx, int my) {
	int pos;
	
	// check and record if mouse if over a button
	if ((mouse_over_but = over_button(win, mx, my)) != -1)
		return 0; // keep standard cursor

	if ((mx > text_arrow_x) && (mx < text_arrow_x + text_arrow_width) &&
			(my < text_arrow_y) && (my > text_arrow_y - text_arrow_height))
	{
		item_help_str = items_text_toggle_help_str;
		mouse_over_text_arrow = 1;
		return 0;
	}


	if(mx>0&&mx<6*items_grid_size&&my>0&&my<6*items_grid_size){
		pos=get_mouse_pos_in_grid(mx, my, 6, 6, 0, 0, items_grid_size, items_grid_size);

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
			mouseover_item_pos = pos;
			
			return 1;
		}
	} else if(mx>wear_items_x_offset && mx<wear_items_x_offset+2*wear_grid_size &&
				my>wear_items_y_offset && my<wear_items_y_offset+4*wear_grid_size) {
		pos=36+get_mouse_pos_in_grid(mx, my, 2, 4, wear_items_x_offset, wear_items_y_offset, wear_grid_size, wear_grid_size);
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
	} else if(show_help_text && mx>quantity_x_offset && mx<quantity_x_offset+ITEM_EDIT_QUANT*quantity_width &&
			my>quantity_y_offset && my<quantity_y_offset+6*quantity_height){
		item_help_str = quantity_edit_str;
	} else if (show_help_text && *inventory_item_string && (my > (win->len_y - text_y_offset))) {
		item_help_str = (disable_double_click)?click_clear_str :double_click_clear_str;
	}
	
	return 0;
}

int keypress_items_handler(window_info * win, int x, int y, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
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

	items_grid_size = (int)(0.5 + win->current_scale * ((use_small_items_window) ?33: 50));
	wear_grid_size = (int)(0.5 + win->current_scale * 33);

	wear_items_y_offset = items_grid_size;
	wear_items_x_offset = items_grid_size * 6 + seperator;

	text_arrow_width = wear_grid_size * 0.4;
	text_arrow_height = wear_grid_size * 0.4;
	text_arrow_x = wear_items_x_offset;
	text_arrow_y = items_grid_size * 6;

	but_len_x = (int)(0.5 + win->current_scale * 31);
	but_x_offset = wear_items_x_offset + 2 * wear_grid_size + seperator;

	win_x_len = but_x_offset + but_len_x;

	text_y_offset = (int)(0.5 + ((items_disable_text_block) ?3 : 7) * win->small_font_len_y);

	win_y_len = 6 * items_grid_size + text_y_offset + seperator;

	quantity_width = (int)((win_x_len - 2) / 6.0);
	quantity_height = win->small_font_len_y + seperator;
	quantity_y_offset = win_y_len - quantity_height - 1;
	quantity_x_offset = 1;
	item_quantity = quantities.quantity[quantities.selected].val;

	but_len_y = wear_grid_size;
	for (i=0; i<NUMBUT; i++)
		but_y_off[i] = but_len_y*i;

	resize_window(win->window_id, win_x_len, win_y_len);

	cm_remove_regions(items_win);
	cm_add_region(cm_stoall_but, items_win, but_x_offset, wear_items_y_offset+but_y_off[0], but_len_x, but_len_y);
	cm_add_region(cm_getall_but, items_win, but_x_offset, wear_items_y_offset+but_y_off[1], but_len_x, but_len_y);
	cm_add_region(cm_dropall_but, items_win, but_x_offset, wear_items_y_offset+but_y_off[2], but_len_x, but_len_y);
	cm_add_region(cm_mix_but, items_win, but_x_offset, wear_items_y_offset+but_y_off[3], but_len_x, but_len_y);
	cm_add_region(cm_itemlist_but, items_win, but_x_offset, wear_items_y_offset+but_y_off[4], but_len_x, but_len_y);

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
		case ELW_CM_MENU_LEN+2: show_items_handler(win); break;
		case ELW_CM_MENU_LEN+7: send_input_text_line("#sto", 4); break;
	}
	return 1;
}

void display_items_menu()
{
	if(items_win < 0){
		int our_root_win = -1;
		if (!windows_on_top) {
			our_root_win = game_root_win;
		}
		items_win= create_window(win_inventory, our_root_win, 0, items_menu_x, items_menu_y, 0, 0, ELW_USE_UISCALE|ELW_WIN_DEFAULT);

		set_window_custom_scale(items_win, &custom_scale_factors.items);
		set_window_handler(items_win, ELW_HANDLER_DISPLAY, &display_items_handler );
		set_window_handler(items_win, ELW_HANDLER_CLICK, &click_items_handler );
		set_window_handler(items_win, ELW_HANDLER_MOUSEOVER, &mouseover_items_handler );
		set_window_handler(items_win, ELW_HANDLER_KEYPRESS, (int (*)())&keypress_items_handler );
		set_window_handler(items_win, ELW_HANDLER_SHOW, &show_items_handler );
		set_window_handler(items_win, ELW_HANDLER_UI_SCALE, &show_items_handler );
		
		cm_add(windows_list.window[items_win].cm_id, cm_items_menu_str, context_items_handler);
		cm_bool_line(windows_list.window[items_win].cm_id, ELW_CM_MENU_LEN+1, &use_small_items_window, NULL);
		cm_bool_line(windows_list.window[items_win].cm_id, ELW_CM_MENU_LEN+2, &manual_size_items_window, NULL);
		cm_bool_line(windows_list.window[items_win].cm_id, ELW_CM_MENU_LEN+3, &item_window_on_drop, "item_window_on_drop");
		cm_bool_line(windows_list.window[items_win].cm_id, ELW_CM_MENU_LEN+4, &allow_equip_swap, NULL);
		cm_bool_line(windows_list.window[items_win].cm_id, ELW_CM_MENU_LEN+5, &items_mod_click_any_cursor, NULL);
				
		cm_stoall_but = cm_create(inv_keeprow_str, NULL);
		cm_bool_line(cm_stoall_but, 0, &items_stoall_nofirstrow, NULL);
		cm_bool_line(cm_stoall_but, 1, &items_stoall_nolastrow, NULL);
		
		cm_dropall_but = cm_create(inv_keeprow_str, NULL);
		cm_bool_line(cm_dropall_but, 0, &items_dropall_nofirstrow, NULL);
		cm_bool_line(cm_dropall_but, 1, &items_dropall_nolastrow, NULL);
		
		cm_mix_but = cm_create(mix_all_str, NULL);
		cm_bool_line(cm_mix_but, 0, &items_mix_but_all, NULL);

		cm_getall_but = cm_create(auto_get_all_str, NULL);
		cm_bool_line(cm_getall_but, 0, &items_auto_get_all, NULL);

		cm_itemlist_but = cm_create(item_list_but_str, NULL);
		cm_bool_line(cm_itemlist_but, 0, &items_list_on_left, NULL);

		show_items_handler(&windows_list.window[items_win]);

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
