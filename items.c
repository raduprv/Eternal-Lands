#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "items.h"
#include "asc.h"
#include "cursors.h"
#include "elwindows.h"
#include "errors.h"
#include "gamewin.h"
#include "gl_init.h"
#include "global.h"
#include "hud.h"
#include "init.h"
#include "interface.h"
#include "manufacture.h"
#include "misc.h"
#include "multiplayer.h"
#include "platform.h"
#ifdef NEW_SOUND
#include "sound.h"
#endif // NEW_SOUND
#include "storage.h"
#include "textures.h"
#include "translate.h"
#ifdef COUNTERS
#include "counters.h"
#endif

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
	}
};
int edit_quantity=-1;

int item_action_mode=ACTION_WALK;

int items_win= -1;
int items_menu_x=10;
int items_menu_y=20;
int items_grid_size=51;//Changes depending on the size of the root window (is 51 > 640x480 and 33 in 640x480).
int items_menu_x_len=6*51+110;
int items_menu_y_len=6*51+90;

int items_text[MAX_ITEMS_TEXTURES];

char items_string[300]={0};
int item_dragged=-1;
int item_quantity=1;

int use_item=-1;

int wear_items_x_offset=6*51+20;
int wear_items_y_offset=30;

int quantity_x_offset=6*51+20;
int quantity_y_offset=185;

__inline__ GLuint get_items_texture(int no)
{
	return items_text[no];
}

void rendergrid(int columns, int rows, int left, int top, int width, int height)
{
	int x, y;
	int temp;

	glBegin(GL_LINES);

	for(y=0; y<=rows; y++){
		temp = top + y * height;
		glVertex2i(left,         temp);
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

void drag_item(int item, int storage, int mini)
{
	float u_start,v_start,u_end,v_end;
	int cur_item,this_texture;
	int cur_item_img;

	int quantity=item_quantity;
	char str[20];
	
	if(storage) {
		if (item < 0 || item >= STORAGE_ITEMS_SIZE)
			// oops
			return;
		
		cur_item=storage_items[item].image_id;
		if(!storage_items[item].quantity) {
			storage_item_dragged=-1;
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
			item_dragged=-1;
			return;
		}
		if(item_list[item].is_stackable){
			if(quantity>item_list[item].quantity)quantity=item_list[item].quantity;
		} else quantity=-1;//The quantity for non-stackable items is misleading so don't show it...
	}

	cur_item_img=cur_item%25;
	u_start=0.2f*(cur_item_img%5);
	u_end=u_start+(float)50/256;
	v_start=(1.0f+((float)50/256)/256.0f)-((float)50/256*(cur_item_img/5));
	v_end=v_start-(float)50/256;

	//get the texture this item belongs to
	this_texture=get_items_texture(cur_item/25);

	get_and_set_texture_id(this_texture);
	glBegin(GL_QUADS);
	if(mini)
		draw_2d_thing(u_start,v_start,u_end,v_end,mouse_x-16,mouse_y-16,mouse_x+16,mouse_y+16);
	else
		draw_2d_thing(u_start,v_start,u_end,v_end,mouse_x-25,mouse_y-25,mouse_x+25,mouse_y+25);
	glEnd();
	
	if(!mini && quantity!=-1){
		safe_snprintf(str,sizeof(str),"%i",quantity);
		draw_string_small(mouse_x-25, mouse_y+10, (unsigned char*)str, 1);
	}
}

void get_your_items (const Uint8 *data)
{
	int i,total_items,pos;
	Uint8 flags;

	total_items=data[0];

	//clear the item string we might have left from a previous session
	items_string[0]=0;

	//clear the items first
	for(i=0;i<ITEM_NUM_ITEMS;i++){
		item_list[i].quantity=0;
	}
	
	for(i=0;i<total_items;i++){
		pos=data[i*8+1+6];
		// try not to wipe out cooldown information if no real change
		if(item_list[pos].image_id != SDL_SwapLE16(*((Uint16 *)(data+i*8+1))) ){
			item_list[pos].cooldown_time = 0;
			item_list[pos].cooldown_rate = 1;
		}
		item_list[pos].image_id=SDL_SwapLE16(*((Uint16 *)(data+i*8+1)));
		item_list[pos].quantity=SDL_SwapLE32(*((Uint32 *)(data+i*8+1+2)));
		item_list[pos].pos=pos;
#ifdef NEW_SOUND
		item_list[pos].action = ITEM_NO_ACTION;
#endif // NEW_SOUND
		flags=data[i*8+1+7];

		item_list[pos].is_resource=((flags&ITEM_RESOURCE)>0);
		item_list[pos].is_reagent=((flags&ITEM_REAGENT)>0);
		item_list[pos].use_with_inventory=((flags&ITEM_INVENTORY_USABLE)>0);
		item_list[pos].is_stackable=((flags&ITEM_STACKABLE)>0);

	}

	build_manufacture_list();
}

#ifdef NEW_SOUND
void check_for_item_sound(int pos)
{
	int i, snd = -1;
	
#ifdef _EXTRA_SOUND_DEBUG
	printf("Used item: %d, Image ID: %d, Action: %d\n", pos, item_list[pos].image_id, item_list[pos].action);
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
			add_sound_object(snd, your_actor->x_pos * 2, your_actor->y_pos * 2, 1);
		// Reset the action
		item_list[pos].action = ITEM_NO_ACTION;
	}
}
#endif // NEW_SOUND

void remove_item_from_inventory(int pos)
{
	item_list[pos].quantity=0;
	
#ifdef NEW_SOUND
	check_for_item_sound(pos);
#endif // NEW_SOUND
	
	build_manufacture_list();
}

void get_new_inventory_item (const Uint8 *data)
{
	int pos;
	Uint8 flags;
	int quantity;
	int image_id;

	pos= data[6];
	flags= data[7];
	image_id=SDL_SwapLE16(*((Uint16 *)(data)));
	quantity=SDL_SwapLE32(*((Uint32 *)(data+2)));

#ifdef COUNTERS
	if (harvesting && (quantity >= item_list[pos].quantity) ) {	//some harvests, eg hydrogenium and wolfram, also decrease an item number. only count what goes up
		increment_harvest_counter(item_list[pos].quantity > 0 ? quantity - item_list[pos].quantity : quantity);
	}
#endif

	// don't touch cool down when it's already active
	if(item_list[pos].quantity == 0 || item_list[pos].image_id != image_id){
		item_list[pos].cooldown_time = 0;
		item_list[pos].cooldown_rate = 1;
	}
	item_list[pos].quantity=quantity;
	item_list[pos].image_id=image_id;
	item_list[pos].pos=pos;

	item_list[pos].is_resource=((flags&ITEM_RESOURCE)>0);
	item_list[pos].is_reagent=((flags&ITEM_REAGENT)>0);
	item_list[pos].use_with_inventory=((flags&ITEM_INVENTORY_USABLE)>0);
	item_list[pos].is_stackable=((flags&ITEM_STACKABLE)>0);
	
#ifdef NEW_SOUND
	check_for_item_sound(pos);
#endif // NEW_SOUND
	
	build_manufacture_list();
}

int display_items_handler(window_info *win)
{
	char str[80];
	int x,y,i;
	int item_is_weared=0;
	Uint32 _cur_time = SDL_GetTicks(); /* grab a snapshot of current time */

	glEnable(GL_TEXTURE_2D);

	x=quantity_x_offset+(video_mode>4?69:51)/2;
	y=quantity_y_offset+3;
	glColor3f(0.3f,0.5f,1.0f);
	for(i=0;i<6;x+=(video_mode>4?69:51),++i){
		if(i==edit_quantity){
			glColor3f(1.0f, 0.0f, 0.3f);
			draw_string_small(x-strlen(quantities.quantity[i].str)*4, y, (unsigned char*)quantities.quantity[i].str, 1);
			glColor3f(0.3f, 0.5f, 1.0f);
		} else if(i==quantities.selected){
			glColor3f(0.0f, 1.0f, 0.3f);
			draw_string_small(x-strlen(quantities.quantity[i].str)*4, y, (unsigned char*)quantities.quantity[i].str, 1);
			glColor3f(0.3f, 0.5f, 1.0f);
		} else  draw_string_small(x-strlen(quantities.quantity[i].str)*4, y, (unsigned char*)quantities.quantity[i].str, 1);
	}
	draw_string_small(win->len_x-strlen(quantity_str)*8-5, quantity_y_offset-19, (unsigned char*)quantity_str, 1);

	glColor3f(1.0f,1.0f,1.0f);
	//ok, now let's draw the objects...
	for(i=0;i<ITEM_NUM_ITEMS;i++){
		if(item_list[i].quantity){
			float u_start,v_start,u_end,v_end;
			int this_texture,cur_item,cur_pos;
			int x_start,x_end,y_start,y_end;

			//get the UV coordinates.
			cur_item=item_list[i].image_id%25;
			u_start=0.2f*(cur_item%5);
			u_end=u_start+(float)50/256;
			v_start=(1.0f+((float)50/256)/256.0f)-((float)50/256*(cur_item/5));
			v_end=v_start-(float)50/256;

			//get the x and y
			cur_pos=i;
			if(cur_pos>=ITEM_WEAR_START){//the items we 'wear' are smaller
				cur_pos-=ITEM_WEAR_START;
				item_is_weared=1;
				x_start=wear_items_x_offset+33*(cur_pos%2)+1;
				x_end=x_start+32-1;
				y_start=wear_items_y_offset+33*(cur_pos/2);
				y_end=y_start+32-1;
			} else {
				item_is_weared=0;
				x_start=items_grid_size*(cur_pos%6)+1;
				x_end=x_start+items_grid_size-1;
				y_start=items_grid_size*(cur_pos/6);
				y_end=y_start+items_grid_size-1;
			}

			//get the texture this item belongs to
			this_texture=get_items_texture(item_list[i].image_id/25);
			
			get_and_set_texture_id(this_texture);
			glBegin(GL_QUADS);
				draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
			glEnd();

			if (item_list[i].cooldown_time > _cur_time)
			{
				float cooldown = ((float)(item_list[i].cooldown_time - _cur_time)) / ((float)item_list[i].cooldown_rate);
				float x_center = (x_start + x_end)*0.5f;
				float y_center = (y_start + y_end)*0.5f;
				float flash_effect_offset = 0.0f;
				const float flash_delay = 600.0f; // larger values --> larger delay

				if (cooldown < 0.0f)
					cooldown = 0.0f;
				else if (cooldown > 1.0f)
					cooldown = 1.0f;

				glDisable(GL_TEXTURE_2D);
				glEnable(GL_BLEND);

				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glBegin(GL_TRIANGLE_FAN);
					//glColor4f(0.14f, 0.35f, 0.82f, 0.50f); 
					flash_effect_offset = sin((float)SDL_GetTicks()/(flash_delay * min2f(0.75f, 0.625f+cooldown/2)));
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
				glColor3f(1.0f, 1.0f, 1.0f);
			}
			
			if(!item_is_weared){
				safe_snprintf(str, sizeof(str), "%i", item_list[i].quantity);
				draw_string_small(x_start, y_end-15, (unsigned char*)str, 1);
			}
		}
	}
	
	//draw the load string
	safe_snprintf(str,sizeof(str),"%s: %i/%i",attributes.carry_capacity.shortname,your_info.carry_capacity.cur,your_info.carry_capacity.base);
	draw_string_small ((video_mode>4?win->len_x-8*strlen (str)-10:2), (video_mode>4?win->len_y-107:quantity_y_offset-19), (unsigned char*)str, 1);

	glColor3f(0.57f,0.67f,0.49f);
	safe_snprintf(str,sizeof(str),equip_str);
	draw_string_small (wear_items_x_offset + 33 - (8 * strlen(str))/2, wear_items_y_offset-18, (unsigned char*)str, 1);
	glColor3f(1.0f,1.0f,1.0f);
	
	//now, draw the inventory text, if any.
	draw_string_small(4, win->len_y - (video_mode>4?85:105), (unsigned char*)items_string, 4);
	
	// Render the grid *after* the images. It seems impossible to code
	// it such that images are rendered exactly within the boxes on all 
	// cards
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);

	//draw the grids
	rendergrid(6, 6, 0, 0, items_grid_size, items_grid_size);
	
	glColor3f(0.57f,0.67f,0.49f);
	rendergrid(2, 4, wear_items_x_offset, wear_items_y_offset, 33, 33);
	
	//now, draw the quantity boxes
	glColor3f(0.3f,0.5f,1.0f);
	rendergrid(6, 1, quantity_x_offset, quantity_y_offset, (video_mode>4?69:51), 20);
	glEnable(GL_TEXTURE_2D);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 1;
}

int click_items_handler(window_info *win, int mx, int my, Uint32 flags)
{
	Uint8 str[100];
	int right_click = flags & ELW_RIGHT_MOUSE;
	int ctrl_on = flags & ELW_CTRL;
	int shift_on = flags & ELW_SHIFT;
	
	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	if(right_click) {
		if(item_dragged!=-1 || use_item!=-1 || storage_item_dragged!=-1){
			use_item=-1;
			item_dragged=-1;
			storage_item_dragged=-1;
			item_action_mode=ACTION_WALK;
			return 1;
		}
		
		if(mx>=wear_items_x_offset && mx<wear_items_x_offset+66 && my>=wear_items_y_offset && my<wear_items_y_offset+133) {
			switch(item_action_mode){
				case ACTION_WALK:
					item_action_mode=ACTION_LOOK;
					break;
				case ACTION_LOOK:
				default:
					item_action_mode=ACTION_WALK;
			}
			return 1;
		} else if(mx>=quantity_x_offset && mx<quantity_x_offset+6*(video_mode>4?69:51) && my>=quantity_y_offset && my<quantity_y_offset+20){
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
	if(item_action_mode==ACTION_USE)	action_mode=ACTION_USE;

	//see if we changed the quantity
	if(mx>=quantity_x_offset && mx<quantity_x_offset+6*(video_mode>4?69:51)
			&& my>=quantity_y_offset && my<quantity_y_offset+20) {
		int pos=get_mouse_pos_in_grid(mx, my, 6, 1, quantity_x_offset, quantity_y_offset, (video_mode>4?69:51), 20);

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
		item_list[pos].action = ITEM_NO_ACTION;
#endif // NEW_SOUND
		if(pos==-1) {
		} else if(item_dragged!=-1){
			if(!item_list[pos].quantity){
				//send the drop info to the server
				str[0]=MOVE_INVENTORY_ITEM;
				str[1]=item_list[item_dragged].pos;
				str[2]=pos;
				my_tcp_send(my_socket,str,3);
			}
#ifdef NEW_SOUND
			add_sound_object(get_index_for_sound_type_name("Drop Item"), your_actor->x_pos * 2, your_actor->y_pos * 2, 1);
#endif // NEW_SOUND
			
			item_dragged=-1;
		}
		else if(storage_item_dragged!=-1){
			str[0]=WITHDRAW_ITEM;
			str[1]=storage_items[storage_item_dragged].pos;
			*((Uint32*)(str+2))=SDL_SwapLE32(item_quantity);
			my_tcp_send(my_socket, str, 6);
			
#ifdef NEW_SOUND
			add_sound_object(get_index_for_sound_type_name("Drop Item"), your_actor->x_pos * 2, your_actor->y_pos * 2, 1);
#endif // NEW_SOUND
			if(storage_items[storage_item_dragged].quantity<=item_quantity) storage_item_dragged=-1;
		}
		else if(item_list[pos].quantity){
			if(ctrl_on){
				str[0]=DROP_ITEM;
				str[1]=item_list[pos].pos;
				if(item_list[pos].is_stackable)
					*((Uint32 *)(str+2))=SDL_SwapLE32(item_list[pos].quantity);
				else 
					*((Uint32 *)(str+2))=SDL_SwapLE32(36);//Drop all
				my_tcp_send(my_socket, str, 6);
#ifdef NEW_SOUND
				add_sound_object(get_index_for_sound_type_name("Drop Item"), your_actor->x_pos * 2, your_actor->y_pos * 2, 1);
#endif // NEW_SOUND
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
#ifdef NEW_SOUND
					item_list[pos].action = USE_INVENTORY_ITEM;
#ifdef _EXTRA_SOUND_DEBUG
					printf("Using item: %d, inv pos: %d, Image ID: %d\n", item_list[pos].pos, pos, item_list[pos].image_id);
#endif // _EXTRA_SOUND_DEBUG
#endif // NEW_SOUND
				}
			} else if(item_action_mode==ACTION_USE_WITEM) {
				if(use_item!=-1) {
					str[0]=ITEM_ON_ITEM;
					str[1]=item_list[use_item].pos;
					str[2]=item_list[pos].pos;
					my_tcp_send(my_socket,str,3);
					if (!shift_on)
						use_item=-1;
#ifdef NEW_SOUND
					item_list[use_item].action = ITEM_ON_ITEM;
					item_list[pos].action = ITEM_ON_ITEM;
#ifdef _EXTRA_SOUND_DEBUG
					printf("Using item: %d on item: %d, Image ID: %d\n", pos, use_item, item_list[pos].image_id);
#endif // _EXTRA_SOUND_DEBUG
#endif // NEW_SOUND
				} else {
					use_item=pos;
				}
			} else {
				item_dragged=pos;
#ifdef NEW_SOUND
				add_sound_object(get_index_for_sound_type_name("Drag Item"), your_actor->x_pos * 2, your_actor->y_pos * 2, 1);
#endif // NEW_SOUND
			}
		}
	} 
	
	//see if we clicked on any item in the wear category
	else if(mx>wear_items_x_offset && mx<wear_items_x_offset+2*33 &&
	   my>wear_items_y_offset && my<wear_items_y_offset+4*33){
		int pos=36+get_mouse_pos_in_grid(mx, my, 2, 4, wear_items_x_offset, wear_items_y_offset, 32, 32);
		
		if(pos<36) {
		} else if(item_list[pos].quantity){
			if(item_action_mode == ACTION_LOOK) {
				str[0]=LOOK_AT_INVENTORY_ITEM;
				str[1]=item_list[pos].pos;
				my_tcp_send(my_socket, str, 2);
			} else if(item_dragged==-1 && left_click) {
				item_dragged=pos;
#ifdef NEW_SOUND
				add_sound_object(get_index_for_sound_type_name("Drag Item"), your_actor->x_pos * 2, your_actor->y_pos * 2, 1);
#endif // NEW_SOUND
			}
		} else if(item_dragged!=-1){
			Uint8 str[20];
			
			//send the drop info to the server
			str[0]=MOVE_INVENTORY_ITEM;
			str[1]=item_list[item_dragged].pos;
			str[2]=pos;
			my_tcp_send(my_socket,str,3);
			item_dragged=-1;
#ifdef NEW_SOUND
				add_sound_object(get_index_for_sound_type_name("Drop Item"), your_actor->x_pos * 2, your_actor->y_pos * 2, 1);
#endif // NEW_SOUND
		}
	}
	
	return 1;
}

int mouseover_items_handler(window_info *win, int mx, int my) {
	int pos;
	
	if(mx>0&&mx<6*items_grid_size&&my>0&&my<6*items_grid_size){
		pos=get_mouse_pos_in_grid(mx, my, 6, 6, 0, 0, items_grid_size, items_grid_size);

		if(pos==-1) {
		} else if(item_list[pos].quantity){
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
	} else if(mx>wear_items_x_offset && mx<wear_items_x_offset+2*33 &&
	          my>wear_items_y_offset && my<wear_items_y_offset+4*33){
		pos=36+get_mouse_pos_in_grid(mx, my, 2, 4, wear_items_x_offset, wear_items_y_offset, 33, 33);
		if(show_help_text){
			show_help(equip_here_str, win->len_x-strlen(quantity_edit_str)*8, quantity_y_offset+30);
		}
		if(pos==-1) {
		} else if(item_list[pos].quantity){
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
	} else if(show_help_text && mx>quantity_x_offset && mx<quantity_x_offset+6*(video_mode>4?69:51) &&
			my>quantity_y_offset && my<quantity_y_offset+6*20){
		show_help(quantity_edit_str, win->len_x-strlen(quantity_edit_str)*8, quantity_y_offset+30);
	} 
	
	return 0;
}

int keypress_items_handler(window_info * win, int x, int y, Uint32 key, Uint32 keysym)
{
	if(edit_quantity!=-1){
		char * str=quantities.quantity[edit_quantity].str;
		int * len=&quantities.quantity[edit_quantity].len;
		int * val=&quantities.quantity[edit_quantity].val;

		if(key==SDLK_DELETE){
			reset_quantity(edit_quantity);
			edit_quantity=-1;
			return 1;
		} else if(key==SDLK_BACKSPACE){
			if(*len>0){
				(*len)--;
				str[*len]=0;
				*val=atoi(str);
			}
			return 1;
		} else if(keysym=='\r'){
			if(!*val){
				reset_quantity(edit_quantity);
			}
			item_quantity=*val;
			quantities.selected=edit_quantity;
			edit_quantity=-1;
			return 1;
		} else if(keysym>='0' && keysym<='9' && *len<5){
			str[*len]=keysym;
			(*len)++;
			str[*len]=0;
			
			*val=atoi(str);
			return 1;
		}
	}

	return 0;
}

int drop_all_handler (widget_list *w, int mx, int my, Uint32 flags)
{
	Uint8 str[6] = {0};
	int i;
#ifdef NEW_SOUND
	int dropped_something = 0;
#endif // NEW_SOUND

	//only drop items if it was a left click.
	//There were complaints about mouse-scrolling setting this off
	if ( ! ( flags & ~ELW_LEFT_MOUSE ) )
	{
		for(i = 0; i < ITEM_NUM_ITEMS; i++)
		{
			if (item_list[i].quantity != 0 && item_list[i].pos < ITEM_WEAR_START) // only drop stuff that we're not wearing
			{
				str[0] = DROP_ITEM;
				str[1] = item_list[i].pos;
				*((Uint32 *)(str+2)) = item_list[i].quantity;
				my_tcp_send (my_socket, str, 6);
#ifdef NEW_SOUND
				dropped_something = 1;
#endif // NEW_SOUND
			}
		}
#ifdef NEW_SOUND
		if (dropped_something)
			add_sound_object(get_index_for_sound_type_name("Drop Item"), your_actor->x_pos * 2, your_actor->y_pos * 2, 1);
#endif // NEW_SOUND
		return 1;
	} else {
		return 0;
	}
}

int drop_button_id = 0;

int show_items_handler(window_info * win)
{
	widget_list *w;
	char str[512];
	
	if(video_mode>4) {
		items_grid_size=51;
		wear_items_y_offset=50;
		win->len_y=6*items_grid_size+90;
	} else {
		items_grid_size=33;
		wear_items_y_offset=19;
		win->len_y=6*items_grid_size+110;
	}
	
	win->len_x=6*items_grid_size+110;
	quantity_y_offset=win->len_y-21;
	quantity_x_offset=1;
	wear_items_x_offset=6*items_grid_size+17;
	item_quantity=quantities.quantity[quantities.selected].val;

	w=widget_find(items_win, drop_button_id);
	if(w){
		w->pos_y=(int)((video_mode>4?5:6)*items_grid_size-w->len_y-5);
		w->pos_x=win->len_x - (strlen(drop_all_str)*11+18);
	}
	
	safe_strncpy(str,items_string,sizeof(str));
	put_small_text_in_box((unsigned char*)str, strlen(str), win->len_x-10, items_string);

	return 1;
}

void display_items_menu()
{
	if(items_win < 0){
		int our_root_win = -1;
		if (!windows_on_top) {
			our_root_win = game_root_win;
		}
		items_win= create_window(win_inventory, our_root_win, 0, items_menu_x, items_menu_y, items_menu_x_len, items_menu_y_len, ELW_WIN_DEFAULT);

		set_window_handler(items_win, ELW_HANDLER_DISPLAY, &display_items_handler );
		set_window_handler(items_win, ELW_HANDLER_CLICK, &click_items_handler );
		set_window_handler(items_win, ELW_HANDLER_MOUSEOVER, &mouseover_items_handler );
		set_window_handler(items_win, ELW_HANDLER_KEYPRESS, &keypress_items_handler );
		set_window_handler(items_win, ELW_HANDLER_SHOW, &show_items_handler );
		
		drop_button_id = button_add_extended (items_win, drop_button_id,  NULL, items_menu_x_len - (strlen(drop_all_str)*11+18), 6*(video_mode>4?69:51)+2, 0, 0, 0, 0.8f, 0.77f, 0.57f, 0.39f, drop_all_str);
		widget_set_OnClick (items_win, drop_button_id, drop_all_handler);
		
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
