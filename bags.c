#include <stdlib.h>
#include <string.h>

#include "bags.h"
#include "3d_objects.h"
#include "asc.h"
#include "cursors.h"
#include "elwindows.h"
#include "errors.h"
#include "gamewin.h"
#include "hud.h"
#include "init.h"
#include "interface.h"
#include "items.h"
#include "item_info.h"
#include "md5.h"
#include "multiplayer.h"
#include "particles.h"
#include "textures.h"
#include "tiles.h"
#include "translate.h"
#include "eye_candy_wrapper.h"
#include "gl_init.h"
#ifdef NEW_SOUND
#include "actors.h"
#endif // NEW_SOUND
#include "sound.h"

ground_item ground_item_list[ITEMS_PER_BAG];
bag bag_list[NUM_BAGS];

#define GRIDSIZE 33

int ground_items_win= -1;
int ground_items_menu_x=6*51+100+20;
int ground_items_menu_y=20;
int ground_items_menu_x_len=6*GRIDSIZE;
int ground_items_menu_y_len=10*GRIDSIZE;
Uint32 ground_items_empty_next_bag=0;

static int ground_items_grid_rows = 10;
static int ground_items_grid_cols = 5;
static const int min_grid_rows = 4;
static const int min_grid_cols = 2;
static const char *item_desc_str = NULL;

int view_ground_items=0;

// forward declarations
void draw_pick_up_menu();
// float get_bag_offset_x(float pos_x, float pos_y, int bag_id, int map_x, int map_y);
// float get_bag_offset_y(float pos_x, float pos_y, int bag_id, int map_x, int map_y);
float get_bag_rotation(float pos_x, float pos_y, int bag_id, int map_x, int map_y);
float get_bag_tilt(float pos_x, float pos_y, int bag_id, int map_x, int map_y);

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

/*float get_bag_offset_x(float pos_x, float pos_y, int bag_id, int map_x, int map_y)
{
	char str[64];
	MD5 md5;
	Uint8 digest[16];
	memset(digest, 0, 16);
	safe_snprintf(str, 40, "%f%f%f%f%f", pos_x, pos_y, (float) bag_id,
		(float) map_x, (float) map_y);
	MD5Open(&md5);
	MD5Digest(&md5, str, strlen(str));
	MD5Close(&md5, digest);
	// the sum of 5 sin/cos operations can be between -5 and +5
	// normalize them to -1 ... +1
	// then divide by 16 to normalize the return value to -0.0625 ... + 0.0625
	// (a tile is 0.5 wide)
	return (sinf(powf(digest[0], 2.0f)) + sinf(powf(digest[1], 2.0f)) + sinf(
		sqrtf(abs((float) digest[2]))) + cosf((float) digest[3]) + sinf(
		(float) digest[4])) / 80.0f * ((((int) abs(digest[5])) % 3 == 0) ? 1.0f : -1.0f);
}*/

/*float get_bag_offset_y(float pos_x, float pos_y, int bag_id, int map_x, int map_y)
{
	char str[64];
	MD5 md5;
	Uint8 digest[16];
	memset(digest, 0, 16);
	safe_snprintf(str, 40, "%f%f%f%f%f", pos_x, pos_y, (float) bag_id,
		(float) map_x, (float) map_y);
	MD5Open(&md5);
	MD5Digest(&md5, str, strlen(str));
	MD5Close(&md5, digest);
	// the sum of 5 sin/cos operations can be between -5 and +5
	// normalize them to -1 ... +1
	// then divide by 16 to normalize the return value to -0.0625 ... + 0.0625
	// (a tile is 0.5 wide)
	return (cosf(powf(digest[1], 2.0f)) + cosf(powf(digest[2], 2.0f)) + cosf(
		sqrtf(abs((float) digest[3]))) + sinf((float) digest[4]) + cosf(
		(float) digest[5])) / 80.0f * ((((int) abs(digest[6])) % 3 == 0) ? 1.0f : -1.0f);
}*/

float get_bag_rotation(float pos_x, float pos_y, int bag_id, int map_x, int map_y)
{
	char str[64];
	MD5 md5;
	Uint8 digest[16];
	memset(digest, 0, 16);
	safe_snprintf(str, 40, "%f%f%f%f%f", pos_x, pos_y, (float) bag_id,
		(float) map_x, (float) map_y);
	MD5Open(&md5);
	MD5Digest(&md5, str, strlen(str));
	MD5Close(&md5, digest);
	// the sum of 5 sin operations can be between -5 and +5
	// normalize them to -0.5 ... +0.5
	// multiply with 360 (-180.0 ... +180.0 degrees)
	return ((sinf(digest[2]) + sinf(digest[3]) + sinf(digest[4]) + sinf(
		digest[5]) + sinf(digest[6])) / 10.0f) * 360;
}

float get_bag_tilt(float pos_x, float pos_y, int bag_id, int map_x, int map_y)
{
	char str[64];
	MD5 md5;
	Uint8 digest[16];
	memset(digest, 0, 16);
	safe_snprintf(str, 40, "%f%f%f%f%f", pos_x, pos_y, (float) bag_id,
		(float) map_x, (float) map_y);
	MD5Open(&md5);
	MD5Digest(&md5, str, strlen(str));
	MD5Close(&md5, digest);
	// the sum of 5 cos operations can be between -5 and +5
	// normalize them to -1.0 ... +1.0
	// multiply with 15 (-15.0 ... +15.0 degrees)
	return ((cosf(digest[3]) + cosf(digest[4]) + cosf(digest[5]) + cosf(
		digest[6]) + cosf(digest[7])) / 5.0f) * 30;
}

void put_bag_on_ground(int bag_x,int bag_y,int bag_id)
{
	float x,y,z;
	int obj_3d_id;
#ifdef NEW_SOUND
	int snd;
#endif // NEW_SOUND

	//now, get the Z position
	if (!get_tile_valid(bag_x, bag_y))
	{
		//Warn about this error:
		LOG_WARNING("A bag was placed OUTSIDE the map!\n");
		return;
	}

	z = get_tile_height(bag_x, bag_y);
	//convert from height values to meters
	x=(float)bag_x/2;
	y=(float)bag_y/2;
	//center the object (slightly randomized)
	x = x + 0.25f; // + get_bag_offset_x(bag_x, bag_y, bag_id, tile_map_size_x, tile_map_size_y);
	y = y + 0.25f; // + get_bag_offset_y(bag_x, bag_y, bag_id, tile_map_size_x, tile_map_size_y);

	// DEBUG
	// printf("bag <%i> (%f,%f) rot %f tilt %f\n", bag_id, x, y,
	//	get_bag_rotation(bag_x, bag_y, bag_id, tile_map_size_x, tile_map_size_y),
	//	get_bag_tilt(bag_x, bag_y, bag_id, tile_map_size_x, tile_map_size_y));

	//Launch the animation
	if (use_eye_candy) {
		ec_create_bag_drop(x, y, z, (poor_man ? 6 : 10));
#ifdef ONGOING_BAG_EFFECT
		// start an ongoing effect until the ongoing bag effect is coded
		bag_list[bag_id].ongoing_bag_effect_reference = ec_create_lamp(x, y, z, 0.0, 1.0, 0.75, (poor_man ? 6 : 10));
#endif // ONGOING_BAG_EFFECT
	}
#ifdef NEW_SOUND
	if (your_actor && bag_x == your_actor->x_pos * 2 && bag_y == your_actor->y_pos * 2)
	{
		snd = get_sound_index_for_particle_file_name("./particles/bag_in.part");
		if (snd >= 0)
		{
			add_sound_object (snd, bag_x, bag_y, 0);
		}
	}
#endif // NEW_SOUND

#ifdef OLD_MISC_OBJ_DIR
	obj_3d_id=add_e3d("./3dobjects/misc_objects/bag1.e3d", x, y, z,
#else
	obj_3d_id=add_e3d("./3dobjects/bag1.e3d", x, y, z,
#endif
		get_bag_tilt(bag_x, bag_y, bag_id, tile_map_size_x, tile_map_size_y), 0,
		get_bag_rotation(bag_x, bag_y, bag_id, tile_map_size_x, tile_map_size_y),
		1 ,0 ,1.0f ,1.0f, 1.0f, 1);

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
		if (!get_tile_valid(bag_x, bag_y))
		{
			//Warn about this error!
			LOG_WARNING("A bag was located OUTSIDE the map!\n");
			continue;
		}

		z = get_tile_height(bag_x, bag_y);
		//convert from height values to meters
		x=(float)bag_x/2;
		y=(float)bag_y/2;
		//center the object (slightly randomized)
		x = x + 0.25f; // + get_bag_offset_x(bag_x, bag_y, bag_id, tile_map_size_x, tile_map_size_y);
		y = y + 0.25f; // + get_bag_offset_y(bag_x, bag_y, bag_id, tile_map_size_x, tile_map_size_y);

		// DEBUG
		LOG_DEBUG_VERBOSE("bag <%i> (%f,%f) rot %f tilt %f\n", bag_id, x, y,
			get_bag_rotation(bag_x, bag_y, bag_id, tile_map_size_x, tile_map_size_y),
			get_bag_tilt(bag_x, bag_y, bag_id, tile_map_size_x, tile_map_size_y));

		if (use_eye_candy) {
	#ifdef ONGOING_BAG_EFFECT
			// start an ongoing effect until the ongoing bag effect is coded
			bag_list[bag_id].ongoing_bag_effect_reference = ec_create_lamp(x, y, z, 0.0, 1.0, 0.75, (poor_man ? 6 : 10));
	#endif // ONGOING_BAG_EFFECT
		}

		// Now, find a place into the bags list, so we can destroy the bag properly
		if (bag_list[bag_id].obj_3d_id != -1) {
			char buf[256];
			// oops, slot already taken!
			safe_snprintf(buf, sizeof(buf), "Oops, trying to add an existing bag! id=%d\n", bag_id);
			LOG_ERROR(buf);
			return;
		}

#ifdef OLD_MISC_OBJ_DIR
		obj_3d_id = add_e3d("./3dobjects/misc_objects/bag1.e3d", x, y, z,
#else
		obj_3d_id = add_e3d("./3dobjects/bag1.e3d", x, y, z,
#endif
			get_bag_tilt(bag_x, bag_y, bag_id, tile_map_size_x, tile_map_size_y), 0,
			get_bag_rotation(bag_x, bag_y, bag_id, tile_map_size_x, tile_map_size_y),
			1, 0, 1.0f, 1.0f, 1.0f, 1);
		bag_list[bag_id].x=bag_x;
		bag_list[bag_id].y=bag_y;
		bag_list[bag_id].obj_3d_id=obj_3d_id;
	}
}

void remove_item_from_ground(Uint8 pos)
{
	ground_item_list[pos].quantity= 0;
}

void remove_bag(int bag_id)
{
#ifdef NEW_SOUND
	int snd;
#endif // NEW_SOUND

	if (bag_id >= NUM_BAGS) return;

	if (bag_list[bag_id].obj_3d_id == -1) {
		// oops, no bag in that slot!
		LOG_ERROR("Oops, double-removal of bag!\n");
		return;
	}

	if (use_eye_candy) {
		ec_create_bag_pickup(objects_list[bag_list[bag_id].obj_3d_id]->x_pos, objects_list[bag_list[bag_id].obj_3d_id]->y_pos, objects_list[bag_list[bag_id].obj_3d_id]->z_pos, (poor_man ? 6 : 10));
#ifdef ONGOING_BAG_EFFECT
		if (bag_list[bag_id].ongoing_bag_effect_reference != NULL) {
			ec_recall_effect(bag_list[bag_id].ongoing_bag_effect_reference);
			bag_list[bag_id].ongoing_bag_effect_reference = NULL;
		}
#endif // ONGOING_BAG_EFFECT
	}
#ifdef NEW_SOUND
	if (your_actor && bag_list[bag_id].x == your_actor->x_pos * 2 && bag_list[bag_id].y == your_actor->y_pos * 2)
	{
		snd = get_sound_index_for_particle_file_name("./particles/bag_out.part");
		if (snd >= 0)
		{
			add_sound_object (snd, bag_list[bag_id].x, bag_list[bag_id].y, 0);
		}
	}
#endif // NEW_SOUND

	destroy_3d_object(bag_list[bag_id].obj_3d_id);
	bag_list[bag_id].obj_3d_id=-1;
}

void remove_all_bags(){
	int i;

	for(i=0; i<NUM_BAGS; i++){    // clear bags list!!!!
		bag_list[i].obj_3d_id= -1;
	}
}


int clear_groundlist(void)
{
	int i;
	for(i = 0; i < ITEMS_PER_BAG; i++) {
		ground_item_list[i].quantity = 0;
	}
	return 1;
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
	ground_item_list[pos].id= unset_item_uid;
	ground_item_list[pos].pos= pos;
}


void pick_up_all_items(void)
{
	Uint8 str[20];
	int itempos;
	for(itempos = 0; itempos < ITEMS_PER_BAG; itempos++){
		if(ground_item_list[itempos].quantity){
			str[0]=PICK_UP_ITEM;
			str[1]=itempos;
			*((Uint32 *)(str+2))=SDL_SwapLE32(ground_item_list[itempos].quantity);
			my_tcp_send(my_socket,str,6);
		}
	}
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
	clear_groundlist();

	items_no = data[0];
	if(items_no > ITEMS_PER_BAG) {
		return;
	}

	for(i=0;i<items_no;i++) {
		my_offset= i*7+1;
		pos= data[my_offset+6];
		ground_item_list[pos].image_id= SDL_SwapLE16(*((Uint16 *)(data+my_offset)));
		ground_item_list[pos].quantity= SDL_SwapLE32(*((Uint32 *)(data+my_offset+2)));
		ground_item_list[pos].id= unset_item_uid;
		ground_item_list[pos].pos= pos;
	}

	//	If we have auto bag empting set, only do so within 1 second of pressing getall
	if (ground_items_empty_next_bag) {
		if (abs(ground_items_empty_next_bag - SDL_GetTicks()) < 1000)
			pick_up_all_items();
		ground_items_empty_next_bag = 0;
	}

	// Open and display the bag even if we have sent the server messages to
	// empty. Because if we can't carry the load the window needs to be shown.

	draw_pick_up_menu();
	if(item_window_on_drop) {
		display_items_menu();
	}
}

int pre_display_ground_items_handler(window_info *win)
{
	glEnable(GL_TEXTURE_2D);
	if (item_desc_str != NULL)
	{
		show_help(item_desc_str, 0, win->len_y+10);
		item_desc_str = NULL;
	}
	return 1;
}

int display_ground_items_handler(window_info *win)
{
	char str[80];
	char my_str[10];
	int i;
	static Uint8 resizing = 0;
	int yoffset = get_window_scroll_pos(win->window_id);

	/* if resizing wait until we stop */
	if (win->resized)
		resizing = 1;
	/* once we stop, snap the window to the new grid size */
	else if (resizing)
	{
		int new_width = (ground_items_grid_cols+1)*GRIDSIZE;
		int new_rows = (win->len_y+GRIDSIZE/2)/GRIDSIZE;
		int max_rows = (ITEMS_PER_BAG + ground_items_grid_cols - 1) / ground_items_grid_cols;
		resizing = 0;
		resize_window (win->window_id, new_width, ((new_rows > max_rows) ?max_rows :new_rows)*GRIDSIZE);
		yoffset = get_window_scroll_pos(win->window_id);
	}

	glEnable(GL_TEXTURE_2D);

	// write "get all" in the "get all" box :)
	strap_word(get_all_str,my_str);
	glColor3f(0.77f,0.57f,0.39f);
	draw_string_small(win->len_x-(GRIDSIZE-5), ELW_BOX_SIZE+3+yoffset, (unsigned char*)my_str, 2);

	glColor3f(1.0f,1.0f,1.0f);
	//ok, now let's draw the objects...
	for(i=ITEMS_PER_BAG-1; i>=0; --i) {
		if(ground_item_list[i].quantity > 0) {
			float u_start,v_start,u_end,v_end;
			int this_texture,cur_item,cur_pos;
			int x_start,x_end,y_start,y_end;

			//get the UV coordinates.
			cur_item=ground_item_list[i].image_id%25;
#ifdef	NEW_TEXTURES
			get_item_uv(cur_item, &u_start, &v_start, &u_end,
				&v_end);
#else	/* NEW_TEXTURES */
			u_start=0.2f*(cur_item%5);
			u_end=u_start+(float)50/256;
			v_start=(1.0f+((float)50/256)/256.0f)-((float)50/256*(cur_item/5));
			v_end=v_start-(float)50/256;
#endif	/* NEW_TEXTURES */

			//get the x and y
			cur_pos=i;
			x_start=GRIDSIZE*(cur_pos%ground_items_grid_cols)+1;
			x_end=x_start+32;
			y_start=GRIDSIZE*(cur_pos/ground_items_grid_cols);
			y_end=y_start+32;

			//get the texture this item belongs to
			this_texture=get_items_texture(ground_item_list[i].image_id/25);

#ifdef	NEW_TEXTURES
			bind_texture(this_texture);
#else	/* NEW_TEXTURES */
			get_and_set_texture_id(this_texture);
#endif	/* NEW_TEXTURES */

			glBegin(GL_QUADS);
				draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
			glEnd();

			safe_snprintf(str,sizeof(str),"%i",ground_item_list[i].quantity);
			draw_string_small_shadowed(x_start,y_end-(i&1?22:12),(unsigned char*)str,1,1.0f,1.0f,1.0f,0.0f,0.0f,0.0f);
		}
	}

	// Render the grid *after* the images. It seems impossible to code
	// it such that images are rendered exactly within the boxes on all
	// cards
	glDisable(GL_TEXTURE_2D);

	glColor3f(0.77f,0.57f,0.39f);
	/* if a full grid render in one go */
	if (ground_items_grid_cols*ground_items_grid_rows == ITEMS_PER_BAG)
		rendergrid(ground_items_grid_cols,ground_items_grid_rows,0,0,GRIDSIZE,GRIDSIZE);
	/* otherwise don't render the extra slots */
	else
	{
		int remainder = ITEMS_PER_BAG - (ground_items_grid_cols*(ground_items_grid_rows-1));
		rendergrid(ground_items_grid_cols,ground_items_grid_rows-1,0,0,GRIDSIZE,GRIDSIZE);
		rendergrid(remainder, 1, 0, GRIDSIZE*(ground_items_grid_rows-1), GRIDSIZE, GRIDSIZE);
	}

	glBegin(GL_LINE_LOOP);

		// draw the "get all" box
		glVertex2i(win->len_x, ELW_BOX_SIZE+yoffset);
		glVertex2i(win->len_x-GRIDSIZE, ELW_BOX_SIZE+yoffset);
		glVertex2i(win->len_x-GRIDSIZE, ELW_BOX_SIZE+GRIDSIZE+yoffset);
		glVertex2i(win->len_x, ELW_BOX_SIZE+GRIDSIZE+yoffset);

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
	int ctrl_on = flags & ELW_CTRL;
	int yoffset = get_window_scroll_pos(win->window_id);

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
	if(mx>(win->len_x-GRIDSIZE) && mx<win->len_x && my>ELW_BOX_SIZE && my<GRIDSIZE+ELW_BOX_SIZE){
		pick_up_all_items();
		do_get_item_sound();
		return 1;
	}

	pos = (my<0) ?-1 :get_mouse_pos_in_grid(mx,my+yoffset+1,ground_items_grid_cols,ground_items_grid_rows,0,0,GRIDSIZE,GRIDSIZE);

	if(pos==-1 || pos>=ITEMS_PER_BAG){
	} else
	if(!ground_item_list[pos].quantity) {
		if (item_dragged != -1){
			str[0] = DROP_ITEM;
			str[1] = item_dragged;
			*((Uint32 *) (str + 2)) = SDL_SwapLE32(item_quantity);
			my_tcp_send(my_socket, str, 6);
			do_drop_item_sound();
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
		do_get_item_sound();
	}

	return 1;
}


int mouseover_ground_items_handler(window_info *win, int mx, int my) {
	int yoffset = get_window_scroll_pos(win->window_id);
	int pos = (yoffset>my) ?-1 :get_mouse_pos_in_grid(mx, my+1, ground_items_grid_cols, ground_items_grid_rows, 0, 0, GRIDSIZE, GRIDSIZE);

	if(pos!=-1 && pos<ITEMS_PER_BAG && ground_item_list[pos].quantity) {
		Uint16 item_id = ground_item_list[pos].id;
		int image_id = ground_item_list[pos].image_id;
		if (show_item_desc_text && item_info_available() && (get_item_count(item_id, image_id) == 1))
			item_desc_str = get_item_description(item_id, image_id);
		if(item_action_mode==ACTION_LOOK) {
			elwin_mouse=CURSOR_EYE;
		} else {
			elwin_mouse=CURSOR_PICK;
		}
		return 1;
	}

	return 0;
}

/* dynamically adjust the grid and the scroll bar when the window resizes */
static int resize_ground_items_handler(window_info *win)
{
		/* let the width lead */
		ground_items_grid_cols = (win->len_x / GRIDSIZE) - 1;
		if (ground_items_grid_cols < min_grid_cols)
			ground_items_grid_cols = min_grid_cols;

		/* but maintain a minimum height */
		ground_items_grid_rows = (ITEMS_PER_BAG + ground_items_grid_cols - 1) / ground_items_grid_cols;
		if (ground_items_grid_rows <= min_grid_rows)
		{
			ground_items_grid_rows = min_grid_rows;
			ground_items_grid_cols = min_grid_cols;
			while (ground_items_grid_cols*ground_items_grid_rows < ITEMS_PER_BAG)
				ground_items_grid_cols++;
		}

		set_window_scroll_len(win->window_id, ground_items_grid_rows*GRIDSIZE-win->len_y);
		return 0;
}

void draw_pick_up_menu()
{
	if(ground_items_win < 0){
		int our_root_win = -1;
		if (!windows_on_top) {
			our_root_win = game_root_win;
		}
		if (ground_items_menu_x_len < (min_grid_cols+1)*GRIDSIZE || ground_items_menu_x_len >= window_width ||
			ground_items_menu_y_len < min_grid_rows*GRIDSIZE || ground_items_menu_y_len >= window_height)
		{
			ground_items_menu_x_len=6*GRIDSIZE;
			ground_items_menu_y_len=10*GRIDSIZE;
		}
		ground_items_win= create_window(win_bag, our_root_win, 0, ground_items_menu_x, ground_items_menu_y,
			ground_items_menu_x_len, ground_items_menu_y_len, ELW_SCROLLABLE|ELW_RESIZEABLE|ELW_WIN_DEFAULT);

		set_window_handler(ground_items_win, ELW_HANDLER_DISPLAY, &display_ground_items_handler );
		set_window_handler(ground_items_win, ELW_HANDLER_PRE_DISPLAY, &pre_display_ground_items_handler );
		set_window_handler(ground_items_win, ELW_HANDLER_CLICK, &click_ground_items_handler );
		set_window_handler(ground_items_win, ELW_HANDLER_MOUSEOVER, &mouseover_ground_items_handler );
		set_window_handler(ground_items_win, ELW_HANDLER_RESIZE, &resize_ground_items_handler );
		set_window_handler(ground_items_win, ELW_HANDLER_CLOSE, &clear_groundlist );
		set_window_min_size(ground_items_win, (min_grid_cols+1)*GRIDSIZE, min_grid_rows*GRIDSIZE);
		set_window_scroll_inc(ground_items_win, GRIDSIZE/3);
		set_window_scroll_yoffset(ground_items_win, GRIDSIZE);
		resize_ground_items_handler(&windows_list.window[ground_items_win]);
	} else {
		show_window(ground_items_win);
		select_window(ground_items_win);
	}
	set_window_scroll_pos(ground_items_win, 0);
}
