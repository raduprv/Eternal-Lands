#include <stdlib.h>
#include <string.h>

#include "bags.h"
#include "3d_objects.h"
#include "asc.h"
#include "cursors.h"
#include "elconfig.h"
#include "elwindows.h"
#include "errors.h"
#include "gamewin.h"
#include "hud.h"
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif
#include "init.h"
#include "interface.h"
#include "items.h"
#include "item_info.h"
#include "md5.h"
#include "multiplayer.h"
#include "particles.h"
#include "textures.h"
#include "eye_candy_wrapper.h"
#include "sound.h"

#define NUM_BAGS 200
#define ITEMS_PER_BAG 50

typedef struct
{
	int x;
	int y;
	int obj_3d_id;
#ifdef ONGOING_BAG_EFFECT
	ec_reference ongoing_bag_effect_reference;
#endif // ONGOING_BAG_EFFECT
} bag;

int ground_items_visible_grid_rows = 10;
int ground_items_visible_grid_cols = 5;
int items_auto_get_all = 0;
int view_ground_items=0;

static ground_item ground_item_list[ITEMS_PER_BAG];
static bag bag_list[NUM_BAGS];
static int GRIDSIZE = 33;
static int ground_items_grid_rows = 10;
static int ground_items_grid_cols = 5;
static int grid_sep_x = 0;
static int grid_sep_y = 0;
static const int min_grid_rows = 4;
static const int min_grid_cols = 2;
static const char *item_desc_str = NULL;
static int mouseover_ground_item_pos = -1;
static Uint32 ground_items_empty_next_bag=0;


// forward declarations
static void draw_pick_up_menu(void);
// float get_bag_offset_x(float pos_x, float pos_y, int bag_id, int map_x, int map_y);
// float get_bag_offset_y(float pos_x, float pos_y, int bag_id, int map_x, int map_y);
static float get_bag_rotation(float pos_x, float pos_y, int bag_id, int map_x, int map_y);
static float get_bag_tilt(float pos_x, float pos_y, int bag_id, int map_x, int map_y);

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

static float get_bag_rotation(float pos_x, float pos_y, int bag_id, int map_x, int map_y)
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

static float get_bag_tilt(float pos_x, float pos_y, int bag_id, int map_x, int map_y)
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
	else
		add_particle_sys_at_tile("./particles/bag_in.part", bag_x, bag_y, 1);
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

	obj_3d_id=add_e3d("./3dobjects/bag1.e3d", x, y, z,
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

		obj_3d_id = add_e3d("./3dobjects/bag1.e3d", x, y, z,
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
	if (pos < ITEMS_PER_BAG)
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
	else
		add_particle_sys_at_tile ("./particles/bag_out.part", bag_list[bag_id].x, bag_list[bag_id].y, 1);
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

void remove_all_bags(void){
	int i;

	for(i=0; i<NUM_BAGS; i++){    // clear bags list!!!!
		bag_list[i].obj_3d_id= -1;
	}
}


static int clear_groundlist(void)
{
	int i;
	for(i = 0; i < ITEMS_PER_BAG; i++) {
		ground_item_list[i].quantity = 0;
	}
	return 1;
}

int find_and_open_closest_bag(int tile_x, int tile_y, float max_distance)
{
	int i;
	int found_bag = -1;
	float x = tile_x;
	float y = tile_y;

	float distance;
	float min_distance_found = 50;

	for(i=0; i<NUM_BAGS; i++)
		if(bag_list[i].obj_3d_id != -1)
		{
			distance = sqrt(((float)bag_list[i].x - x) * ((float)bag_list[i].x - x) + ((float)bag_list[i].y - y) * ((float)bag_list[i].y - y));
			if (distance < max_distance)
				if (distance < min_distance_found)
				{
					found_bag = i;
					min_distance_found = distance;
				}
		}

	if (found_bag != -1)
	{
		Uint8 str[4];
		str[0]= INSPECT_BAG;
		str[1]= found_bag;
		my_tcp_send(my_socket,str, 2);
		return 1;
	}

	return 0;
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


static void pick_up_all_items(void)
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

void items_get_bag(int x, int y)
{
	int pos;
	for(pos=0;pos<NUM_BAGS;pos++)
	{
		if(bag_list[pos].x != 0 && bag_list[pos].y != 0 &&
			bag_list[pos].x == x && bag_list[pos].y == y)
		{
			if(get_show_window_MW(MW_BAGS))
				pick_up_all_items();
			else
			{
				// if auto empty bags enable, set the open timer
				if (items_auto_get_all)
					ground_items_empty_next_bag = SDL_GetTicks();
				else
					ground_items_empty_next_bag = 0;
				open_bag(bag_list[pos].obj_3d_id);
			}
			break; //we should only stand on one bag
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
		if ((SDL_GetTicks() - ground_items_empty_next_bag) < 1000)
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

static int pre_display_ground_items_handler(window_info *win)
{
	glEnable(GL_TEXTURE_2D);
	if (item_desc_str != NULL)
	{
		show_help(item_desc_str, 0, win->len_y+10, win->current_scale);
		item_desc_str = NULL;
	}
	return 1;
}

static int display_ground_items_handler(window_info *win)
{
	char str[80];
	char my_str[10];
	int i;
	static Uint8 resizing = 0;
	int yoffset = get_window_scroll_pos(win->window_id);
	int but_text_y;

	/* if resizing wait until we stop */
	if (win->resized)
		resizing = 1;
	/* once we stop, snap the window to the new grid size */
	else if (resizing)
	{
		int new_width = grid_sep_x + (ground_items_grid_cols + 1) * GRIDSIZE;
		int new_rows = (win->len_y - 2 * grid_sep_y + GRIDSIZE / 2) / GRIDSIZE;
		int max_rows = (ITEMS_PER_BAG + ground_items_grid_cols - 1) / ground_items_grid_cols;
		resizing = 0;
		resize_window (win->window_id, new_width, 2 * grid_sep_y + ((new_rows > max_rows) ?max_rows :new_rows) * GRIDSIZE);
		yoffset = get_window_scroll_pos(win->window_id);
	}

	glEnable(GL_TEXTURE_2D);

	// write "get all" in the "get all" box :)
	strap_word(get_all_str,my_str);
	glColor3f(0.77f,0.57f,0.39f);
	but_text_y = (int)(0.5 + ((GRIDSIZE - (float)(2 * win->small_font_len_y)) / 2.0));
	draw_string_small_zoomed_centered(win->len_x-GRIDSIZE/2, win->box_size+but_text_y+yoffset, (unsigned char*)my_str, 2, win->current_scale);

	glColor3f(1.0f,1.0f,1.0f);
	//ok, now let's draw the objects...
	for(i=ITEMS_PER_BAG-1; i>=0; --i) {
		if(ground_item_list[i].quantity > 0) {
			float u_start,v_start,u_end,v_end;
			int this_texture,cur_item,cur_pos;
			int x_start,x_end,y_start,y_end;
			int use_large = (mouseover_ground_item_pos == i) && enlarge_text();

			//get the UV coordinates.
			cur_item=ground_item_list[i].image_id%25;
			get_item_uv(cur_item, &u_start, &v_start, &u_end, &v_end);

			//get the x and y
			cur_pos=i;
			x_start = grid_sep_x + GRIDSIZE * (cur_pos % ground_items_grid_cols) + 1;
			x_end=x_start+GRIDSIZE-1;
			y_start = grid_sep_y +  + GRIDSIZE * (cur_pos / ground_items_grid_cols);
			y_end=y_start+GRIDSIZE-1;

			//get the texture this item belongs to
			this_texture=get_items_texture(ground_item_list[i].image_id/25);
			bind_texture(this_texture);

			glBegin(GL_QUADS);
				draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
			glEnd();

			safe_snprintf(str,sizeof(str),"%i",ground_item_list[i].quantity);
			y_end -= ((i & 1) ?GRIDSIZE-1 : ((use_large) ?win->default_font_len_y :win->small_font_len_y));
			if (use_large)
				draw_string_shadowed_zoomed(x_start,y_end,(unsigned char*)str,1,1.0f,1.0f,1.0f,0.0f,0.0f,0.0f, win->current_scale);
			else
				draw_string_small_shadowed_zoomed(x_start,y_end,(unsigned char*)str,1,1.0f,1.0f,1.0f,0.0f,0.0f,0.0f, win->current_scale);
		}
	}
	mouseover_ground_item_pos = -1;

	// Render the grid *after* the images. It seems impossible to code
	// it such that images are rendered exactly within the boxes on all
	// cards
	glDisable(GL_TEXTURE_2D);

	glColor3f(0.77f,0.57f,0.39f);
	/* if a full grid render in one go */
	if (ground_items_grid_cols*ground_items_grid_rows == ITEMS_PER_BAG)
		rendergrid(ground_items_grid_cols, ground_items_grid_rows, grid_sep_x, grid_sep_y, GRIDSIZE, GRIDSIZE);
	/* otherwise don't render the extra slots */
	else
	{
		int remainder = ITEMS_PER_BAG - (ground_items_grid_cols*(ground_items_grid_rows-1));
		rendergrid(ground_items_grid_cols, ground_items_grid_rows - 1, grid_sep_x, grid_sep_y, GRIDSIZE, GRIDSIZE);
		rendergrid(remainder, 1, grid_sep_x, grid_sep_y + GRIDSIZE*(ground_items_grid_rows-1), GRIDSIZE, GRIDSIZE);
	}

	glBegin(GL_LINE_LOOP);

		// draw the "get all" box
		glVertex2i(win->len_x, win->box_size + yoffset);
		glVertex2i(win->len_x - GRIDSIZE, win->box_size + yoffset);
		glVertex2i(win->len_x - GRIDSIZE, win->box_size + GRIDSIZE + yoffset);
		glVertex2i(win->len_x, win->box_size + GRIDSIZE + yoffset);

	glEnd();
	glEnable(GL_TEXTURE_2D);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;
}


static int click_ground_items_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int pos;
	Uint8 str[10];
	int right_click = flags & ELW_RIGHT_MOUSE;
	int ctrl_on = flags & KMOD_CTRL;
	int yoffset = get_window_scroll_pos(win->window_id);

	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) {
		return 0;
	}

	if(right_click) {
		if(item_dragged != -1) {
			item_dragged = -1;
			return 1;
		} else if(is_gamewin_look_action()) {
			clear_gamewin_look_action();
			return 1;
		}
	}

	// see if we clicked on the "Get All" box
	if(mx>(win->len_x-GRIDSIZE) && mx<win->len_x && my>win->box_size && my<GRIDSIZE+win->box_size){
		pick_up_all_items();
		do_get_item_sound();
		return 1;
	}

	pos = (my<0) ?-1 :get_mouse_pos_in_grid(mx, my + yoffset + 1,
		ground_items_grid_cols, ground_items_grid_rows, grid_sep_x, grid_sep_y, GRIDSIZE, GRIDSIZE);

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
	} else if(right_click || is_gamewin_look_action()) {
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


static int mouseover_ground_items_handler(window_info *win, int mx, int my) {
	int yoffset = get_window_scroll_pos(win->window_id);
	int pos = (yoffset>my) ?-1 :get_mouse_pos_in_grid(mx, my + 1, ground_items_grid_cols,
		ground_items_grid_rows, grid_sep_x, grid_sep_y, GRIDSIZE, GRIDSIZE);

	if(pos!=-1 && pos<ITEMS_PER_BAG && ground_item_list[pos].quantity) {
		Uint16 item_id = ground_item_list[pos].id;
		int image_id = ground_item_list[pos].image_id;
		if (show_item_desc_text && item_info_available() && (get_item_count(item_id, image_id) == 1))
			item_desc_str = get_item_description(item_id, image_id);
		if(is_gamewin_look_action()) {
			elwin_mouse=CURSOR_EYE;
		} else {
			elwin_mouse=CURSOR_PICK;
		}
		mouseover_ground_item_pos = pos;
		return 1;
	}

	return 0;
}

/* dynamically adjust the grid and the scroll bar when the window resizes */
static int resize_ground_items_handler(window_info *win, int width, int height)
{
	ground_items_visible_grid_cols = ((win->len_x - grid_sep_x) / GRIDSIZE) - 1;
	ground_items_visible_grid_rows = ((win->len_y - 2 * grid_sep_y) / GRIDSIZE);

	/* let the width lead */
	ground_items_grid_cols = ground_items_visible_grid_cols;
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

	set_window_scroll_len(win->window_id, ground_items_grid_rows * GRIDSIZE + 2 * grid_sep_y - win->len_y);
	return 0;
}

static int ui_scale_ground_items_handler(window_info * win)
{
	int current_scroll_pos = get_window_scroll_pos(win->window_id) / (GRIDSIZE/3);
	if (ground_items_visible_grid_cols < min_grid_cols || ground_items_visible_grid_cols >= ITEMS_PER_BAG ||
		ground_items_visible_grid_rows < min_grid_rows || ground_items_visible_grid_rows >= ITEMS_PER_BAG)
	{
		ground_items_visible_grid_cols = 5;
		ground_items_visible_grid_rows = 10;
	}
	GRIDSIZE = (int)(0.5 + 33 * win->current_scale);
	grid_sep_x = (int)(0.5 + 2 * win->current_scale);
	grid_sep_y = (int)(0.5 + 2 * win->current_scale);
	set_window_min_size(win->window_id, grid_sep_x + (min_grid_cols + 1) * GRIDSIZE, 2 * grid_sep_y + min_grid_rows * GRIDSIZE);
	set_window_scroll_inc(win->window_id, GRIDSIZE/3);
	set_window_scroll_yoffset(win->window_id, GRIDSIZE);
	set_window_scroll_pos(win->window_id, current_scroll_pos * GRIDSIZE/3);
	resize_window(win->window_id, grid_sep_x + (ground_items_visible_grid_cols + 1) * GRIDSIZE,
		2 * grid_sep_y + ground_items_visible_grid_rows * GRIDSIZE);
	return 1;
}

void server_close_bag(void)
{
	hide_window_MW(MW_BAGS);
	clear_groundlist();
	clear_was_open_MW(MW_BAGS);
}

void client_close_bag(void)
{
	const unsigned char protocol_close_bag = S_CLOSE_BAG;
	my_tcp_send(my_socket, &protocol_close_bag, 1);
	server_close_bag();
}

static int close_handler(window_info *win)
{
	client_close_bag();
	return 1;
}

static void draw_pick_up_menu(void)
{
	int ground_items_win = get_id_MW(MW_BAGS);

	if(ground_items_win < 0){
		ground_items_win = create_window(win_bag, (not_on_top_now(MW_BAGS) ?game_root_win : -1), 0, get_pos_x_MW(MW_BAGS), get_pos_y_MW(MW_BAGS),
			0, 0, ELW_USE_UISCALE|ELW_SCROLLABLE|ELW_RESIZEABLE|ELW_WIN_DEFAULT);
		set_id_MW(MW_BAGS, ground_items_win);

		set_window_custom_scale(ground_items_win, MW_BAGS);
		set_window_handler(ground_items_win, ELW_HANDLER_DISPLAY, &display_ground_items_handler );
		set_window_handler(ground_items_win, ELW_HANDLER_PRE_DISPLAY, &pre_display_ground_items_handler );
		set_window_handler(ground_items_win, ELW_HANDLER_CLICK, &click_ground_items_handler );
		set_window_handler(ground_items_win, ELW_HANDLER_MOUSEOVER, &mouseover_ground_items_handler );
		set_window_handler(ground_items_win, ELW_HANDLER_RESIZE, &resize_ground_items_handler );
		set_window_handler(ground_items_win, ELW_HANDLER_CLOSE, &close_handler );
		set_window_handler(ground_items_win, ELW_HANDLER_UI_SCALE, &ui_scale_ground_items_handler );

		if (ground_items_win >=0 && ground_items_win < windows_list.num_windows)
			ui_scale_ground_items_handler(&windows_list.window[ground_items_win]);
		else
			return;
		check_proportional_move(MW_BAGS);

	} else {
		show_window(ground_items_win);
		select_window(ground_items_win);
	}
	set_window_scroll_pos(ground_items_win, 0);
}
