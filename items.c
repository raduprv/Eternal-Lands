#include <string.h>
#include "global.h"
#include "elwindows.h"

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
int show_quantity_help=0;

int item_action_mode=ACTION_WALK;

int items_win= -1;
int items_menu_x=10;
int items_menu_y=20;
int items_menu_x_len=6*51+110;
int items_menu_y_len=6*51+90;

int items_text_1;
int items_text_2;
int items_text_3;
int items_text_4;
int items_text_5;
int items_text_6;
int items_text_7;
int items_text_8;
int items_text_9;
int items_text_10;

char items_string[300];
int item_dragged=-1;
int item_quantity=1;

int use_item=-1;

int wear_items_x_offset=6*51+20;
int wear_items_y_offset=30;

int quantity_x_offset=6*51+20;
int quantity_y_offset=185;

inline GLuint get_items_texture(int no)
{
	GLuint retval=-1;
	
	switch(no){
		case 0:
			retval=items_text_1;
			break;
		case 1:
			retval=items_text_2;
			break;
		case 2:
			retval=items_text_3;
			break;
		case 3:
			retval=items_text_4;
			break;
		case 4:
			retval=items_text_5;
			break;
		case 5:
			retval=items_text_6;
			break;
		case 6:
			retval=items_text_7;
			break;
		case 7:
			retval=items_text_8;
			break;
		case 8:
			retval=items_text_9;
			break;
		case 9:
			retval=items_text_10;
			break;
	}

	return retval;
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
	int x, y, i=0;

	mx-=left;
	my-=top;
	columns*=width;
	rows*=height;

	for(y=0; y<=rows; y+=height, i--){
		for(x=0; x<=columns; x+=width, i++){
			if(mx>=x && mx<=x+width && my>=y && my<=y+height)
				return i;
		}
	}

	return -1;
}

void reset_quantity(int pos)
{
	int val;
					
	switch(pos){
		case 1:
			val=5;
			break;
		case 2:
			val=10;
			break;
		case 3:
			val=20;
			break;
		case 4:
			val=50;
			break;
		case 5:
			val=100;
			break;
		case 0:
		default:
			val=1;
			break;
	}

	sprintf(quantities.quantity[pos].str,"%d",val);
	quantities.quantity[pos].len=strlen(quantities.quantity[pos].str);
	quantities.quantity[pos].val=val;
}

void drag_item(int item, int storage, int mini)
{
	float u_start,v_start,u_end,v_end;
	int cur_item,this_texture;
	int cur_item_img;

#ifdef NEW_CLIENT
	int quantity=item_quantity;
	char str[20];
	
	if(storage) {
		cur_item=storage_items[item].image_id;
		if(!storage_items[item].quantity) {
			storage_item_dragged=-1;
			return;
		}
		if(quantity>storage_items[item].quantity)quantity=storage_items[item].quantity;
	} else {
		cur_item=item_list[item].image_id;
		if(!item_list[item].quantity) {
			item_dragged=-1;
			return;
		}
		if(item_list[item].is_stackable){
			if(quantity>item_list[item].quantity)quantity=item_list[item].quantity;
		} else quantity=-1;//The quantity for non-stackable items is misleading so don't show it...
	}
#else
	cur_item=item_list[item].image_id;
#endif

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
		draw_2d_thing(u_start,v_start,u_end,v_end,mouse_x,mouse_y,mouse_x+32,mouse_y+32);
	else
		draw_2d_thing(u_start,v_start,u_end,v_end,mouse_x-25,mouse_y-25,mouse_x+25,mouse_y+25);
	glEnd();
	
#ifdef NEW_CLIENT
	if(!mini && quantity!=-1){
		sprintf(str,"%i",quantity);
		draw_string_small(mouse_x-25,mouse_y+10,str,1);
	}
#endif
}

void get_your_items(Uint8 *data)
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
		item_list[pos].image_id=SDL_SwapLE16(*((Uint16 *)(data+i*8+1)));
		item_list[pos].quantity=SDL_SwapLE32(*((Uint32 *)(data+i*8+1+2)));
		item_list[pos].pos=pos;
		flags=data[i*8+1+7];

		item_list[pos].is_resource=((flags&ITEM_RESOURCE)>0);
		item_list[pos].is_reagent=((flags&ITEM_REAGENT)>0);
		item_list[pos].use_with_inventory=((flags&ITEM_INVENTORY_USABLE)>0);
		item_list[pos].is_stackable=((flags&ITEM_STACKABLE)>0);
	}

	build_manufacture_list();
}

void remove_item_from_inventory(int pos)
{
	item_list[pos].quantity=0;
	
	build_manufacture_list();
}

void get_new_inventory_item(Uint8 *data)
{
	int pos;
	Uint8 flags;
	int quantity;
	int image_id;

	pos= data[6];
	flags= data[7];
	image_id=SDL_SwapLE16(*((Uint16 *)(data)));
	quantity=SDL_SwapLE32(*((Uint32 *)(data+2)));

	item_list[pos].quantity=quantity;
	item_list[pos].image_id=image_id;
	item_list[pos].pos=pos;

	item_list[pos].is_resource=((flags&ITEM_RESOURCE)>0);
	item_list[pos].is_reagent=((flags&ITEM_REAGENT)>0);
	item_list[pos].use_with_inventory=((flags&ITEM_INVENTORY_USABLE)>0);
	item_list[pos].is_stackable=((flags&ITEM_STACKABLE)>0);
	
	build_manufacture_list();
}

int display_items_handler(window_info *win)
{
	Uint8 str[80];
	int x,y,i;
	int item_is_weared=0;

	glEnable(GL_TEXTURE_2D);

	x=quantity_x_offset+33;
	y=quantity_y_offset+3;
	glColor3f(0.3f,0.5f,1.0f);
	for(i=0;i<6;i++,y+=20){
		if(i==edit_quantity){
			glColor3f(1.0f, 0.0f, 0.3f);
			draw_string_small(x-strlen(quantities.quantity[i].str)*4, y, quantities.quantity[i].str,1);
			glColor3f(0.3f, 0.5f, 1.0f);
		} else if(i==quantities.selected){
			glColor3f(0.0f, 1.0f, 0.3f);
			draw_string_small(x-strlen(quantities.quantity[i].str)*4, y, quantities.quantity[i].str,1);
			glColor3f(0.3f, 0.5f, 1.0f);
		} else  draw_string_small(x-strlen(quantities.quantity[i].str)*4, y, quantities.quantity[i].str,1);
	}
	
	draw_string_small(x-strlen(quantity_str)*4, quantity_y_offset-17, quantity_str, 1);

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
				x_end=x_start+32;
				y_start=wear_items_y_offset+33*(cur_pos/2);
				y_end=y_start+32;
			} else {
				item_is_weared=0;
				x_start=51*(cur_pos%6)+1;
				x_end=x_start+50;
				y_start=51*(cur_pos/6);
				y_end=y_start+50;
			}

			//get the texture this item belongs to
			this_texture=get_items_texture(item_list[i].image_id/25);
			
			get_and_set_texture_id(this_texture);
			glBegin(GL_QUADS);
				draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
			glEnd();
			
			if(!item_is_weared){
				sprintf(str,"%i",item_list[i].quantity);
				draw_string_small(x_start,y_end-15,str,1);
			}
		}
	}
	
	//draw the load string
	sprintf(str,"%s: %i/%i",attributes.carry_capacity.shortname,your_info.carry_capacity.cur,your_info.carry_capacity.base);
	draw_string_small (win->len_x -  8 * strlen (str) - 4, 6*51+10, str, 1);
	
	//now, draw the inventory text, if any.
	draw_string_small(4,win->len_y-59,items_string,4);

	if(show_quantity_help){
		show_help(quantity_edit_str, quantity_x_offset+70-strlen(quantity_edit_str)*8, quantity_y_offset+125);
	}
	
	// Render the grid *after* the images. It seems impossible to code
	// it such that images are rendered exactly within the boxes on all 
	// cards
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);

	//draw the grids
	rendergrid(6, 6, 0, 0, 51, 51);
	
	glColor3f(0.57f,0.67f,0.49f);
	rendergrid(2, 4, wear_items_x_offset, wear_items_y_offset, 33, 33);
	
	//now, draw the quantity boxes
	glColor3f(0.3f,0.5f,1.0f);
	rendergrid(1, 6, quantity_x_offset, quantity_y_offset, 66, 20);
	glEnable(GL_TEXTURE_2D);

	return 1;
}

int click_items_handler(window_info *win, int mx, int my, Uint32 flags)
{
	Uint8 str[100];
	int right_click = flags & ELW_RIGHT_MOUSE;
	
	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	if(right_click) {
#ifdef NEW_CLIENT 
		if(item_dragged!=-1 || use_item!=-1 || storage_item_dragged!=-1){
#else
		if(item_dragged!=-1 || use_item!=-1){
#endif 
			use_item=-1;
			item_dragged=-1;
#ifdef NEW_CLIENT
			storage_item_dragged=-1;
#endif
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
		} else if(mx>=quantity_x_offset && mx<quantity_x_offset+66 && my>=quantity_y_offset && my<quantity_y_offset+120){
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
	if(mx>quantity_x_offset && mx<quantity_x_offset+66 &&
	   my>quantity_y_offset && my<quantity_y_offset+120) {
		int pos=get_mouse_pos_in_grid(mx, my, 1, 6, quantity_x_offset, quantity_y_offset, 70, 20);

		if(flags & ELW_LEFT_MOUSE){
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
	else if(mx>0 && mx < 306 &&
	   my>0 && my < 306) {
		int pos=get_mouse_pos_in_grid(mx, my, 6, 6, 0, 0, 51, 51);
		
		if(item_dragged!=-1){
			if(!item_list[pos].quantity){
				//send the drop info to the server
				str[0]=MOVE_INVENTORY_ITEM;
				str[1]=item_list[item_dragged].pos;
				str[2]=pos;
				my_tcp_send(my_socket,str,3);
			}
			
			item_dragged=-1;
		}
#ifdef NEW_CLIENT
		else if(storage_item_dragged!=-1){
			str[0]=WITHDRAW_ITEM;
			str[1]=storage_items[storage_item_dragged].pos;
			*((Uint32*)(str+2))=SDL_SwapLE32(item_quantity);
			my_tcp_send(my_socket, str, 6);
			
			if(storage_items[storage_item_dragged].quantity<=item_quantity) storage_item_dragged=-1;
		}
#endif
		else if(item_list[pos].quantity){
			if(ctrl_on){
				str[0]=DROP_ITEM;
				str[1]=item_list[pos].pos;
				*((Uint16 *)(str+2))=SDL_SwapLE16((short)item_list[pos].quantity);
				my_tcp_send(my_socket, str, 4);
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
				}
			} else if(item_action_mode==ACTION_USE_WITEM) {
				if(use_item!=-1) {
					str[0]=ITEM_ON_ITEM;
					str[1]=item_list[use_item].pos;
					str[2]=item_list[pos].pos;
					my_tcp_send(my_socket,str,3);
					use_item=-1;
				} else {
					use_item=pos;
				}
			} else {
				item_dragged=pos;
			}
		}
	} 
	
	//see if we clicked on any item in the wear category
	else if(mx>wear_items_x_offset && mx<wear_items_x_offset+2*33 &&
	   my>wear_items_y_offset && my<wear_items_y_offset+4*33){
		int pos=36+get_mouse_pos_in_grid(mx, my, 2, 4, wear_items_x_offset, wear_items_y_offset, 32, 32);
		
		if(item_list[pos].quantity){
			if(item_action_mode == ACTION_LOOK) {
				str[0]=LOOK_AT_INVENTORY_ITEM;
				str[1]=item_list[pos].pos;
				my_tcp_send(my_socket, str, 2);
			} else if(item_dragged==-1 && left_click) item_dragged=pos;
		} else if(item_dragged!=-1){
			Uint8 str[20];
			
			//send the drop info to the server
			str[0]=MOVE_INVENTORY_ITEM;
			str[1]=item_list[item_dragged].pos;
			str[2]=pos;
			my_tcp_send(my_socket,str,3);
			item_dragged=-1;
		} 
	}
	
	return 1;
}

int mouseover_items_handler(window_info *win, int mx, int my) {
	int pos;
	
	show_quantity_help=0;
	
	if(mx>0&&mx<306&&my>0&&my<306){
		pos=get_mouse_pos_in_grid(mx, my, 6, 6, 0, 0, 51, 51);

		if(item_list[pos].quantity){
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
		
		if(item_list[pos].quantity){
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
	} else if(show_help_text && mx>quantity_x_offset && mx<quantity_x_offset+66 &&
	          my>quantity_y_offset && my<quantity_y_offset+120){
		show_quantity_help=1;
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
		} else if(key==SDLK_RETURN){
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

int drop_all_handler ()
{
	Uint8 str[4] = {0};
#ifndef SERVER_DROP_ALL
	int i;

	for(i = 0; i < ITEM_NUM_ITEMS; i++)
	{
		if (item_list[i].quantity != 0 && item_list[i].pos < ITEM_WEAR_START) // only drop stuff that we're not wearing
		{
			str[0] = DROP_ITEM;
			str[1] = item_list[i].pos;
			*((Uint16 *)(str+2)) = item_list[i].quantity;
			my_tcp_send (my_socket, str, 4);
		}
	}
#else
	str[0] = DROP_ALL;
	my_tcp_send (my_socket, str, 1); // this may need to be altered depending on server implementation
#endif

	return 1;
}

void display_items_menu()
{
	int drop_button_id = 0;

	if(items_win < 0){
		items_win= create_window("Inventory", game_root_win, 0, items_menu_x, items_menu_y, items_menu_x_len, items_menu_y_len, ELW_WIN_DEFAULT);

		set_window_handler(items_win, ELW_HANDLER_DISPLAY, &display_items_handler );
		set_window_handler(items_win, ELW_HANDLER_CLICK, &click_items_handler );
		set_window_handler(items_win, ELW_HANDLER_MOUSEOVER, &mouseover_items_handler );
		set_window_handler(items_win, ELW_HANDLER_KEYPRESS, &keypress_items_handler );
		
		drop_button_id = button_add_extended (items_win, drop_button_id,  NULL, 0, 6*51+10, 0, 0, 0, 0.8f, 0.77f, 0.57f, 0.39f, "Drop All");
		widget_set_OnClick (items_win, drop_button_id, drop_all_handler);
	} else {
		show_window(items_win);
		select_window(items_win);
	}
}
