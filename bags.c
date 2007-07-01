#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "elwindows.h"
#include "eye_candy_wrapper.h"

ground_item ground_item_list[ITEMS_PER_BAG];
bag bag_list[NUM_BAGS];

int ground_items_win= -1;
int ground_items_menu_x=6*51+100+20;
int ground_items_menu_y=20;
int ground_items_menu_x_len=6*33;
int ground_items_menu_y_len=10*33;

int view_ground_items=0;

void draw_pick_up_menu();//Forward declaration
	
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

        //Launch the animation
#ifdef	EYE_CANDY
	if (use_eye_candy) ec_create_bag_drop(x, y, z, (poor_man ? 6 : 10));
#endif	//EYE_CANDY

	obj_3d_id=add_e3d("./3dobjects/misc_objects/bag1.e3d",x,y,z,0,0,0,1,0,1.0f,1.0f,1.0f, 1);
	
	//now, find a place into the bags list, so we can destroy the bag properly
	bag_list[bag_id].x=bag_x;
	bag_list[bag_id].y=bag_y;
	bag_list[bag_id].obj_3d_id=obj_3d_id;
}

void add_bags_from_list (const Uint8 *data)
{
	Uint16 bags_no;
	int i;
	int bag_x,bag_y,my_offset; //bag_type unused?
	float x,y,z;
	int obj_3d_id, bag_id;

	bags_no=data[0];

	if(bags_no > NUM_BAGS) {
		return;//something nasty happened
	}
	
	for(i=0;i<bags_no;i++) {
		my_offset=i*5+1;
		bag_x=SDL_SwapLE16(*((Uint16 *)(data+my_offset)));
		bag_y=SDL_SwapLE16(*((Uint16 *)(data+my_offset+2)));
		bag_id=*((Uint8 *)(data+my_offset+4));
		if(bag_id >= NUM_BAGS) {
			continue;
		}
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
	
		obj_3d_id=add_e3d("./3dobjects/misc_objects/bag1.e3d",x,y,z,0,0,0,1,0,1.0f,1.0f,1.0f, 1);
		//now, find a place into the bags list, so we can destroy the bag properly
	
		if (bag_list[bag_id].obj_3d_id != -1) {
			char	buf[256];

			// oops, slot already taken!
			snprintf(buf, sizeof(buf), "Oops, trying to add an existing bag! id=%d\n", bag_id);
			LOG_ERROR(buf);
			return;
		}

		bag_list[bag_id].x=bag_x;
		bag_list[bag_id].y=bag_y;
		bag_list[bag_id].obj_3d_id=obj_3d_id;
	}
}

void remove_item_from_ground(Uint8 pos)
{
	ground_item_list[pos].quantity= 0;
}

void remove_bag(int which_bag)
{
#ifdef EYE_CANDY
	float x, y, z;
#endif
	
	if (which_bag >= NUM_BAGS) return;

	if (bag_list[which_bag].obj_3d_id == -1) {
		// oops, no bag in that slot!
		LOG_ERROR("Oops, double-removal of bag!\n");
		return;
	}

 #ifdef EYE_CANDY
	x = bag_list[which_bag].x;
	y = bag_list[which_bag].y;
	z = -2.2f+height_map[bag_list[which_bag].y*tile_map_size_x*6+bag_list[which_bag].x]*0.2f;
	//convert from height values to meters
	x /= 2;
	y /= 2;
	//center the object
	x = x + 0.25f;
	y = y + 0.25f;
	if (use_eye_candy) ec_create_bag_pickup(x, y, z, (poor_man ? 6 : 10));
 #else // EYE_CANDY
  #ifdef SFX
	add_particle_sys_at_tile ("./particles/bag_out.part", bag_list[which_bag].x, bag_list[which_bag].y, 1);
  #endif
 #endif // EYE_CANDY

	destroy_3d_object(bag_list[which_bag].obj_3d_id);
	bag_list[which_bag].obj_3d_id=-1;
}

void remove_all_bags(){
	int i;

	for(i=0; i<NUM_BAGS; i++){    // clear bags list!!!!
		bag_list[i].obj_3d_id= -1;
	}
}

void open_bag(int object_id)
{
	int i;
	Uint8 str[4];
	for(i=0;i<NUM_BAGS;i++){
		if(bag_list[i].obj_3d_id==object_id){
			str[0]= INSPECT_BAG;
			str[1]= i;
			my_tcp_send(my_socket,str,2);
			return;
		}
	}
}

//do the flags later on
void get_bag_item (const Uint8 *data)
{
	int	pos;
	pos= data[6];

	if (pos >= ITEMS_PER_BAG) return;

	ground_item_list[pos].image_id= SDL_SwapLE16(*((Uint16 *)(data)));
	ground_item_list[pos].quantity= SDL_SwapLE32(*((Uint32 *)(data+2)));
	ground_item_list[pos].pos= pos;
}

//put the flags later on
void get_bags_items_list (const Uint8 *data)
{
	Uint16 items_no;
	int i;
	int pos;
	int my_offset;

	view_ground_items=1;
	//clear the list
	for(i = 0; i < ITEMS_PER_BAG; i++) {
		ground_item_list[i].quantity = 0;
	}

	items_no = data[0];
	if(items_no > ITEMS_PER_BAG) {
		return;
	}
	
	for(i=0;i<items_no;i++) {
		my_offset= i*7+1;
		pos= data[my_offset+6];
		ground_item_list[pos].image_id= SDL_SwapLE16(*((Uint16 *)(data+my_offset)));
		ground_item_list[pos].quantity= SDL_SwapLE32(*((Uint32 *)(data+my_offset+2)));
		ground_item_list[pos].pos= pos;
	}
	
	draw_pick_up_menu();
	if(item_window_on_drop) {
		display_items_menu();
	}
}

int display_ground_items_handler(window_info *win)
{
	char str[80];
	char my_str[10];
	int i;

	glEnable(GL_TEXTURE_2D);

	// write "get all" in the "get all" box :)
	strap_word(get_all_str,my_str);
	glColor3f(0.77f,0.57f,0.39f);
	draw_string_small(win->len_x-28, 23, (unsigned char*)my_str, 2);

	glColor3f(1.0f,1.0f,1.0f);
	//ok, now let's draw the objects...
	for(i=ITEMS_PER_BAG; i>=0; --i) {
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
					
			safe_snprintf(str,sizeof(str),"%i",ground_item_list[i].quantity);
			draw_string_small(x_start,y_end-(i&1?22:12),(unsigned char*)str,1);
		}
	}
	
	// Render the grid *after* the images. It seems impossible to code
	// it such that images are rendered exactly within the boxes on all 
	// cards
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
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;
}


int click_ground_items_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int pos;
	Uint8 str[10];
	int right_click = flags & ELW_RIGHT_MOUSE;
	
	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) {
		return 0;
	}

	if(right_click) {
		if(item_dragged != -1) {
			item_dragged = -1;
		} else if(item_action_mode == ACTION_LOOK) {
			item_action_mode = ACTION_WALK;
		} else {
			item_action_mode = ACTION_LOOK;
		}
		return 1;
	}

	// see if we clicked on the "Get All" box
	if(mx>(win->len_x-33) && mx<win->len_x && my>20 && my<53){
		for(pos = 0; pos < ITEMS_PER_BAG; pos++){
			if(ground_item_list[pos].quantity){
				str[0]=PICK_UP_ITEM;
				str[1]=pos;
				*((Uint32 *)(str+2))=SDL_SwapLE32(ground_item_list[pos].quantity);
				my_tcp_send(my_socket,str,6);
			}
		}
		return 1;
	}

	pos=get_mouse_pos_in_grid(mx,my,5,10,0,0,33,33);

	if(pos==-1){
	} else
	if(!ground_item_list[pos].quantity) {
		if (item_dragged != -1){
			str[0] = DROP_ITEM;
			str[1] = item_dragged;
			*((Uint32 *) (str + 2)) = SDL_SwapLE32(item_quantity);
			my_tcp_send(my_socket, str, 6);
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
		*((Uint32 *)(str+2))= SDL_SwapLE32(quantity);
		my_tcp_send(my_socket,str,6);
	}
		
	return 1;
}


int mouseover_ground_items_handler(window_info *win, int mx, int my) {
	int pos=get_mouse_pos_in_grid(mx, my, 5, 10, 0, 0, 33, 33);
	
	if(pos!=-1 && ground_item_list[pos].quantity) {
		if(item_action_mode==ACTION_LOOK) {
			elwin_mouse=CURSOR_EYE;
		} else {
			elwin_mouse=CURSOR_PICK;
		}
		return 1;
	}
	
	return 0;
}


void draw_pick_up_menu()
{
	if(ground_items_win < 0){
		int our_root_win = -1;
		if (!windows_on_top) {
			our_root_win = game_root_win;
		}
		ground_items_win= create_window(win_bag, our_root_win, 0, ground_items_menu_x, ground_items_menu_y, ground_items_menu_x_len, ground_items_menu_y_len, ELW_WIN_DEFAULT);

		set_window_handler(ground_items_win, ELW_HANDLER_DISPLAY, &display_ground_items_handler );
		set_window_handler(ground_items_win, ELW_HANDLER_CLICK, &click_ground_items_handler );
		set_window_handler(ground_items_win, ELW_HANDLER_MOUSEOVER, &mouseover_ground_items_handler );
	} else {
		show_window(ground_items_win);
		select_window(ground_items_win);
	}
}
