#include <string.h>
#include "global.h"
#include "elwindows.h"

typedef struct
{
	int x;
	int y;
	int obj_3d_id;
} bag;

item item_list[ITEM_NUM_ITEMS];
item manufacture_list[ITEM_NUM_ITEMS];
ground_item ground_item_list[50];
bag bag_list[200];

item inventory_trade_list[ITEM_WEAR_START];
item your_trade_list[24];
item others_trade_list[24];
int trade_you_accepted=0;
int trade_other_accepted=0;
char other_player_trade_name[20];

void draw_pick_up_menu();   /* forward declaration */

void strap_word(char * in, char * out)
{
	int i = 3;
	while(i--) *out++=*in++;
	while(*in==' ')in++;
	*out++='\n';
	i=3;
	while(i--) *out++=*in++;
	*out=0;
}

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

int view_ground_items=0;
int no_view_my_items=0;

int items_win= -1;
int items_menu_x=10;
int items_menu_y=20;
int items_menu_x_len=6*51+110;
int items_menu_y_len=6*51+60;
//int items_menu_dragged=0;

int ground_items_win= -1;
int ground_items_menu_x=6*51+100+20;
int ground_items_menu_y=20;
int ground_items_menu_x_len=6*33;
int ground_items_menu_y_len=10*33;
//int ground_items_menu_dragged=0;

int manufacture_menu_x=10;
int manufacture_menu_y=20;
int manufacture_menu_x_len=12*33+20;
int manufacture_menu_y_len=6*33;
//int manufacture_menu_dragged=0;

int trade_menu_x=10;
int trade_menu_y=20;
int trade_menu_x_len=13*33;
int trade_menu_y_len=11*33;
//int trade_menu_dragged=0;

int options_menu_x=220;
int options_menu_y=50;
int options_menu_x_len=390;
int options_menu_y_len=300;
//int options_menu_dragged=0;

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
	}

	return retval;
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

int drop_all_handler ()
{
	Uint8 str[4] = {0};
#ifndef SERVER_DROP_ALL
	int i;
#endif

#ifndef SERVER_DROP_ALL
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


int display_items_handler(window_info *win)
{
	Uint8 str[80];
	int x,y,i;
	int item_is_weared=0;

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

	x=quantity_x_offset+33;
	y=quantity_y_offset+3;
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
	draw_string_small (items_menu_x_len -  8 * strlen (str) - 4, items_menu_y_len - 18, str, 1);
	
	//now, draw the inventory text, if any.
	draw_string_small(4,win->len_y-59,items_string,4);

	if(show_quantity_help){
		show_help(quantity_edit_str, quantity_x_offset+70-strlen(quantity_edit_str)*8, quantity_y_offset+125);
	}
	
	return 1;
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


int click_items_handler(window_info *win, int mx, int my, Uint32 flags)
{
	Uint8 str[100];
	int right_click = flags & ELW_RIGHT_MOUSE;
	
	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	if(right_click) {
#ifdef STORAGE 
		if(item_dragged!=-1 || use_item!=-1 || storage_item_dragged!=-1){
#else
		if(item_dragged!=-1 || use_item!=-1){
#endif 
			use_item=-1;
			item_dragged=-1;
#ifdef STORAGE
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
#ifdef STORAGE
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



void drag_item(int item, int storage, int mini)
{
	float u_start,v_start,u_end,v_end;
	int cur_item,this_texture;
	int cur_item_img;

#ifdef STORAGE
	int quantity=item_quantity;
	char str[20];
	
	if(storage) {
		cur_item=storage_items[item].image_id;
		if(quantity>storage_items[item].quantity)quantity=storage_items[item].quantity;
	} else {
		cur_item=item_list[item].image_id;
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
	
#ifdef STORAGE
	if(!mini && quantity!=-1){
		sprintf(str,"%i",quantity);
		draw_string_small(mouse_x-25,mouse_y+10,str,1);
	}
#endif
}


void remove_item_from_inventory(int pos)
{
	item_list[pos].quantity=0;
	
	build_manufacture_list();
}

void remove_item_from_ground(Uint8 pos)
{
	ground_item_list[pos].quantity= 0;
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

int display_ground_items_handler(window_info *win)
{
	Uint8 str[80];
	Uint8 my_str[10];
	int i;

	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);

	rendergrid(5,10,0,0,33,33);
	
	glBegin(GL_LINE_LOOP);
	
		// draw the "get all" box
		glVertex3i(win->len_x, 20,0);
		glVertex3i(win->len_x-33, 20,0);
		glVertex3i(win->len_x-33, 53,0);
		glVertex3i(win->len_x, 53,0);

	glEnd();
	glEnable(GL_TEXTURE_2D);

	// write "get all" in the "get all" box :)
	strap_word(get_all_str,my_str);
	draw_string_small(win->len_x-28, 23, my_str, 2);

	glColor3f(1.0f,1.0f,1.0f);
	//ok, now let's draw the objects...
	for(i=0;i<50;i++) {
		if(ground_item_list[i].quantity > 0) {
			float u_start,v_start,u_end,v_end;
			int this_texture,cur_item,cur_pos;
			int x_start,x_end,y_start,y_end;

			//get the UV coordinates.
			cur_item=ground_item_list[i].image_id%25;
			u_start=0.2f*(cur_item%5);
			u_end=u_start+0.2f;
			v_start=(1.0f+2.0f/256.0f)-(0.2f*(cur_item/5));
			v_end=v_start-0.2f;

			//get the x and y
			cur_pos=i;
			x_start=33*(cur_pos%5)+1;
			x_end=x_start+32;
			y_start=33*(cur_pos/5);
			y_end=y_start+32;

			//get the texture this item belongs to
			this_texture=get_items_texture(ground_item_list[i].image_id/25);

			get_and_set_texture_id(this_texture);
					
			glBegin(GL_QUADS);
				draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
			glEnd();
					
			sprintf(str,"%i",ground_item_list[i].quantity);
			draw_string_small(x_start,y_end-15,str,1);
		}
	}
	
	return 1;
}


//do the flags later on
void get_bag_item(Uint8 *data)
{
	int	pos;
	pos= data[6];

	ground_item_list[pos].image_id= SDL_SwapLE16(*((Uint16 *)(data)));
	ground_item_list[pos].quantity= SDL_SwapLE32(*((Uint32 *)(data+2)));
	ground_item_list[pos].pos= pos;
}

//put the flags later on
void get_bags_items_list(Uint8 *data)
{
	Uint16 items_no;
	int i;
	int pos;
	int my_offset;

	view_ground_items=1;
	draw_pick_up_menu();
	if(item_window_on_drop)
		display_items_menu();
	
	//clear the list
	for(i=0;i<50;i++) ground_item_list[i].quantity=0;

	items_no=data[0];
	for(i=0;i<items_no;i++) {
		my_offset= i*7+1;
		pos= data[my_offset+6];
		ground_item_list[pos].image_id= SDL_SwapLE16(*((Uint16 *)(data+my_offset)));
		ground_item_list[pos].quantity= SDL_SwapLE32(*((Uint32 *)(data+my_offset+2)));
		ground_item_list[pos].pos= pos;
	}
}

void put_bag_on_ground(int bag_x,int bag_y,int bag_id)
{
	float x,y,z;
	int obj_3d_id;

	//now, get the Z position
	if(bag_y*tile_map_size_x*6+bag_x>tile_map_size_x*tile_map_size_y*6*6) {
		//Warn about this error:
		log_error("A bag was placed OUTSIDE the map!\n");
		return;
	}
	
	z=-2.2f+height_map[bag_y*tile_map_size_x*6+bag_x]*0.2f;
	//convert from height values to meters
	x=(float)bag_x/2;
	y=(float)bag_y/2;
	//center the object
	x=x+0.25f;
	y=y+0.25f;
	obj_3d_id=add_e3d("./3dobjects/misc_objects/bag1.e3d",x,y,z,0,0,0,1,0,1.0f,1.0f,1.0f);

	//now, find a place into the bags list, so we can destroy the bag properly
	bag_list[bag_id].x=bag_x;
	bag_list[bag_id].y=bag_y;
	bag_list[bag_id].obj_3d_id=obj_3d_id;
	sector_add_3do(obj_3d_id);
}

void add_bags_from_list(Uint8 *data)
{
	Uint16 bags_no;
	int i;
	int bag_x,bag_y,my_offset; //bag_type unused?
	float x,y,z;
	int obj_3d_id, bag_id;

	bags_no=data[0];
	for(i=0;i<bags_no;i++) {
		my_offset=i*5+1;
		bag_x=SDL_SwapLE16(*((Uint16 *)(data+my_offset)));
		bag_y=SDL_SwapLE16(*((Uint16 *)(data+my_offset+2)));
		bag_id=*((Uint8 *)(data+my_offset+4));
		if(bag_id>=200)continue;
		//now, get the Z position
		if(bag_y*tile_map_size_x*6+bag_x>tile_map_size_x*tile_map_size_y*6*6)  {
			//Warn about this error!
			log_error("A bag was located OUTSIDE the map!\n");
			continue;
		}
		
		z=-2.2f+height_map[bag_y*tile_map_size_x*6+bag_x]*0.2f;
		//convert from height values to meters
		x=(float)bag_x/2;
		y=(float)bag_y/2;
		//center the object
		x=x+0.25f;
		y=y+0.25f;
	
		obj_3d_id=add_e3d("./3dobjects/misc_objects/bag1.e3d",x,y,z,0,0,0,1,0,1.0f,1.0f,1.0f);
		//now, find a place into the bags list, so we can destroy the bag properly
	
		bag_list[bag_id].x=bag_x;
		bag_list[bag_id].y=bag_y;
		bag_list[bag_id].obj_3d_id=obj_3d_id;
		sector_add_3do(obj_3d_id);
	}
}

void remove_bag(int which_bag)
{
	int sector, i, j=MAX_3D_OBJECTS-1, k=-1;

#ifdef PARTICLE_SYS_SOUND
	add_particle_sys_at_tile ("./particles/bag_out.part", bag_list[which_bag].x, bag_list[which_bag].y);
#else
	add_particle_sys_at_tile("./particles/bag_out.part",bag_list[which_bag].x,bag_list[which_bag].y, -1, 0, 0);
#endif
	sector=SECTOR_GET(objects_list[bag_list[which_bag].obj_3d_id]->x_pos, objects_list[bag_list[which_bag].obj_3d_id]->y_pos);
	for(i=0;i<MAX_3D_OBJECTS;i++){
		if(k!=-1 && sectors[sector].e3d_local[i]==-1){
			j=i-1;
			break;
		}
		else if(k==-1 && sectors[sector].e3d_local[i]==bag_list[which_bag].obj_3d_id)
			k=i;
	}

	sectors[sector].e3d_local[k]=sectors[sector].e3d_local[j];
	sectors[sector].e3d_local[j]=-1;

	destroy_3d_object(bag_list[which_bag].obj_3d_id);
	bag_list[which_bag].obj_3d_id=-1;
}

int click_ground_items_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int pos;
	Uint8 str[10];
	int right_click = flags & ELW_RIGHT_MOUSE;
	
	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	if(right_click) {
		if(item_dragged!=-1) item_dragged=-1;
		else if(item_action_mode==ACTION_LOOK)
			item_action_mode=ACTION_WALK;
		else
			item_action_mode=ACTION_LOOK;
		return 1;
	}

	// see if we clicked on the "Get All" box
	if(mx>(win->len_x-33) && mx<win->len_x && my>20 && my<53){
		for(pos = 0; pos < 50; pos++){
			if(ground_item_list[pos].quantity){
				str[0]=PICK_UP_ITEM;
				str[1]=pos;
				*((Uint16 *)(str+2))=SDL_SwapLE16((short)ground_item_list[pos].quantity);
				my_tcp_send(my_socket,str,4);
			}
		}
		return 1;
	}

	pos=get_mouse_pos_in_grid(mx,my,5,10,0,0,33,33);

	if(!ground_item_list[pos].quantity) {
		if (item_dragged != -1){
			Uint8 str[10];
			int quantity = item_list[item_dragged].quantity;
			if (quantity > item_quantity)
				quantity = item_quantity;
			str[0] = DROP_ITEM;
			str[1] = item_dragged;
			*((Uint16 *) (str + 2)) = SDL_SwapLE16((short)quantity);
			my_tcp_send(my_socket, str, 4);
			if (item_list[item_dragged].quantity - quantity <= 0)
				item_dragged = -1;
		}
	} else if(item_action_mode==ACTION_LOOK) {
		str[0]= LOOK_AT_GROUND_ITEM;
		str[1]= ground_item_list[pos].pos;
		my_tcp_send(my_socket,str,2);
	} else {
		int quantity;
		quantity= ground_item_list[pos].quantity;
		if(quantity > item_quantity && !ctrl_on) quantity= item_quantity;

		str[0]= PICK_UP_ITEM;
		str[1]= ground_item_list[pos].pos;
		*((Uint16 *)(str+2))= SDL_SwapLE16((short)quantity);
		my_tcp_send(my_socket,str,4);
	}
		
	return 1;
}


int mouseover_ground_items_handler(window_info *win, int mx, int my) {
	int pos=get_mouse_pos_in_grid(mx, my, 5, 10, 0, 0, 33, 33);
	
	if(ground_item_list[pos].quantity) {
		if(item_action_mode==ACTION_LOOK) {
			elwin_mouse=CURSOR_EYE;
		} else {
			elwin_mouse=CURSOR_PICK;
		}
		return 1;
	}
	
	return 0;
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

void open_bag(int object_id)
{
	int i;
	Uint8 str[4];
	for(i=0;i<200;i++){
		if(bag_list[i].obj_3d_id==object_id){
			str[0]= INSPECT_BAG;
			str[1]= i;
			my_tcp_send(my_socket,str,2);
			return;
		}
	}
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

void display_items_menu()
{
	int drop_button_id = 0;

	if(items_win < 0){
		items_win= create_window("Inventory", game_root_win, 0, items_menu_x, items_menu_y, items_menu_x_len, items_menu_y_len, ELW_WIN_DEFAULT);

		set_window_handler(items_win, ELW_HANDLER_DISPLAY, &display_items_handler );
		set_window_handler(items_win, ELW_HANDLER_CLICK, &click_items_handler );
		set_window_handler(items_win, ELW_HANDLER_MOUSEOVER, &mouseover_items_handler );
		set_window_handler(items_win, ELW_HANDLER_KEYPRESS, &keypress_items_handler );
		
		drop_button_id = button_add_extended (items_win, drop_button_id,  NULL, 5, items_menu_y_len-25, 0, 0, 0, 0.8f, 0.77f, 0.57f, 0.39f, "Drop All");
		widget_set_OnClick (items_win, drop_button_id, drop_all_handler);
	} else {
		show_window(items_win);
		select_window(items_win);
	}
}


void draw_pick_up_menu()
{
	if(ground_items_win < 0){
		ground_items_win= create_window("Bag", game_root_win, 0, ground_items_menu_x, ground_items_menu_y, ground_items_menu_x_len, ground_items_menu_y_len, ELW_WIN_DEFAULT|ELW_DROPSHADOWS);

		set_window_handler(ground_items_win, ELW_HANDLER_DISPLAY, &display_ground_items_handler );
		set_window_handler(ground_items_win, ELW_HANDLER_CLICK, &click_ground_items_handler );
		set_window_handler(ground_items_win, ELW_HANDLER_MOUSEOVER, &mouseover_ground_items_handler );
	} else {
		show_window(ground_items_win);
		select_window(ground_items_win);
	}
}

