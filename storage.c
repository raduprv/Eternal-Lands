#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "storage.h"

#define STORAGE_CATEGORIES_SIZE 50
#define STORAGE_CATEGORIES_DISPLAY 13
#define STORAGE_SCROLLBAR_CATEGORIES 1200
#define STORAGE_SCROLLBAR_ITEMS 1201

struct storage_category {
	char name[25];
	int id;
	int color;
} storage_categories[STORAGE_CATEGORIES_SIZE];

int no_storage_categories=0;
int selected_category=-1;

int active_storage_item=-1;

ground_item storage_items[STORAGE_ITEMS_SIZE]={{0,0,0}};
int no_storage;

char storage_text[202]={0};

void get_storage_text (const Uint8 *in_data, int len)
{
	safe_snprintf(storage_text, sizeof(storage_text), "%.*s", len, in_data);
}

void get_storage_categories (const char *in_data, int len)
{
	int i;
	int idx, idxp;

	idx = 1;
	for (i = 0; i < in_data[0] && i < STORAGE_CATEGORIES_SIZE && idx < len; i++)
	{
		storage_categories[i].id = (Uint8)in_data[idx++];
		storage_categories[i].name[0] = 127 + c_orange1;
		idxp = 1;
		while (idx < len && idxp < sizeof (storage_categories[i].name) - 1 && in_data[idx] != '\0')
		{
			storage_categories[i].name[idxp++] = in_data[idx++];
		}
		// always make sure the string is terminated
		storage_categories[i].name[idxp] = '\0';

		// was the string too long?
		if (idxp >= sizeof (storage_categories[i].name) - 1)
		{
			// skip rest of string
			while (idx < len && in_data[idx] != '\0') idx++;
		}
		idx++;
	}
	for (i = in_data[0]; i < STORAGE_CATEGORIES_SIZE; i++)
	{
		storage_categories[i].id = -1;
		storage_categories[i].name[0] = '0';
	}

	no_storage_categories = in_data[0];
	if (storage_win > 0) vscrollbar_set_bar_len(storage_win, STORAGE_SCROLLBAR_CATEGORIES, ( no_storage_categories - STORAGE_CATEGORIES_DISPLAY ) > 1 ? (no_storage_categories - STORAGE_CATEGORIES_DISPLAY) : 1);

	selected_category=-1;
	active_storage_item=-1;

	display_storage_menu();
	display_items_menu();
}

int find_category(int id)
{
	int i;

	for(i=0;i<no_storage_categories;i++){
		if(storage_categories[i].id==id) return i;
	}

	return -1;
}

void move_to_category(int cat)
{
	Uint8 str[4];
	
	if(cat<0||cat>=no_storage_categories) return;
	storage_categories[cat].name[0]=127+c_red3;
	if(selected_category!=-1 && cat!=selected_category) storage_categories[selected_category].name[0]=127+c_orange1;
	safe_snprintf(windows_list.window[storage_win].window_name, sizeof(windows_list.window[storage_win].window_name), "%s - %s", win_storage, storage_categories[cat].name+1);

	str[0]=GET_STORAGE_CATEGORY;
	*((Uint8 *)(str+1))=storage_categories[cat].id;

	my_tcp_send(my_socket, str, 2);
}

void get_storage_items (const Uint8 *in_data, int len)
{
	int i;
	int cat, pos;
	int idx;

	if (in_data[0] == 255)
	{
		//It's just an update - make sure we're in the right category
		idx = 2;
		active_storage_item = (char)in_data[idx+6];
		
		for (i = 0; i < STORAGE_ITEMS_SIZE; i++)
		{
			if (storage_items[i].pos == in_data[idx+6])
			{
				storage_items[i].image_id = SDL_SwapLE16 (*((Uint16*)(&in_data[idx])));
				storage_items[i].quantity = SDL_SwapLE32 (*((Uint32*)(&in_data[idx+2])));
				return;
			}
		}

		for (i = 0; i < STORAGE_ITEMS_SIZE; i++)
		{
			if (storage_items[i].quantity == 0)
			{
				storage_items[i].pos = in_data[idx+6];
				storage_items[i].image_id = SDL_SwapLE16 (*((Uint16*)(&in_data[idx])));
				storage_items[i].quantity = SDL_SwapLE32 (*((Uint32*)(&in_data[idx+2])));
				no_storage++;
				return;
			}
		}
	}
	
	no_storage=0;

	no_storage=in_data[0];
	
	cat=find_category(in_data[1]);
	if (cat >= 0)
	{
		storage_categories[cat].name[0] = 127+c_red3;
		if (selected_category != -1 && cat != selected_category)
			storage_categories[selected_category].name[0] = 127+c_orange1;
		sprintf (windows_list.window[storage_win].window_name, "%s - %s", win_storage, storage_categories[cat].name+1);
		selected_category = cat;
	}

	idx = 2;
	for (i = 0; i < no_storage && i < STORAGE_ITEMS_SIZE; i++, idx += 7)
	{
		storage_items[i].image_id = SDL_SwapLE16 (*((Uint16*)(&in_data[idx])));
		storage_items[i].quantity = SDL_SwapLE32 (*((Uint32*)(&in_data[idx+2])));
		storage_items[i].pos = in_data[idx+6];
	}
	
	for ( ; i < STORAGE_ITEMS_SIZE; i++)
	{
		storage_items[i].quantity=0;
	}
	
	vscrollbar_set_pos(storage_win, STORAGE_SCROLLBAR_ITEMS, 0);
	pos = vscrollbar_get_pos(storage_win, STORAGE_SCROLLBAR_CATEGORIES);
	if (cat < pos) {
		vscrollbar_set_pos(storage_win, STORAGE_SCROLLBAR_CATEGORIES, cat);
	} else	if (cat >= pos + STORAGE_CATEGORIES_DISPLAY) {
		vscrollbar_set_pos(storage_win, STORAGE_SCROLLBAR_CATEGORIES, cat - STORAGE_CATEGORIES_DISPLAY + 1);
	}
}

int storage_win=-1;
int storage_win_x=100;
int storage_win_y=100;
int storage_win_x_len=400;
int storage_win_y_len=272;

int cur_item_over=-1;
int storage_item_dragged=-1;

int display_storage_handler(window_info * win)
{
	int i;
	int n=0;
	int pos;

	glColor3f(0.77f, 0.57f, 0.39f);
	glEnable(GL_TEXTURE_2D);
	
	for(i=pos=vscrollbar_get_pos(storage_win,STORAGE_SCROLLBAR_CATEGORIES); i<no_storage_categories && storage_categories[i].id!=-1 && i<pos+STORAGE_CATEGORIES_DISPLAY; i++,n++){
		draw_string_small(20, 20+n*13, (unsigned char*)storage_categories[i].name,1);
	}
	if(storage_text[0]){
		draw_string_small(18, 220, (unsigned char*)storage_text, 1);
	}
	
	glColor3f(1.0f,1.0f,1.0f);
	
	for(i=pos=6*vscrollbar_get_pos(storage_win, STORAGE_SCROLLBAR_ITEMS); i<pos+36 && i<no_storage;i++){
		GLfloat u_start, v_start, u_end, v_end;
		int x_start, x_end, y_start, y_end;
		int cur_item;
		GLuint this_texture;

		if(!storage_items[i].quantity)continue;
		cur_item=storage_items[i].image_id%25;
		u_start=0.2f*(cur_item%5);
		u_end=u_start+(float)50/255;
		v_start=(1.0f+((float)50/255)/255.0f)-((float)50/255*(cur_item/5));
		v_end=v_start-(float)50/255;
		
		this_texture=get_items_texture(storage_items[i].image_id/25);

		if(this_texture!=-1) get_and_set_texture_id(this_texture);

		x_start=(i%6)*32+161;
		x_end=x_start+31;
		y_start=((i-pos)/6)*32+10;
		y_end=y_start+31;

		glBegin(GL_QUADS);
		draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
		glEnd();
	}
	if(active_storage_item >= 0) {
		/* Draw the active item's quantity on top of everything else. */
		for(i = pos = 6*vscrollbar_get_pos(storage_win, STORAGE_SCROLLBAR_ITEMS); i < pos+36 && i < no_storage; i++) {
			if(storage_items[i].pos == active_storage_item) {
				char str[20];
				int x = (i%6)*32+161+16;
				int len;

				safe_snprintf(str, sizeof(str), "%d", storage_items[i].quantity);
				len = strlen(str) * 8;
				if(x - len > 161) {
					x -= len;
				} else if(x + len > 161+6*32) {
					x = 161+5*32+16;
				}
				show_help(str, x, ((i-pos)/6)*32+10+8);
			}
		}
	}

	if(cur_item_over!=-1 && mouse_in_window(win->window_id, mouse_x, mouse_y) == 1 && active_storage_item!=storage_items[cur_item_over].pos){
		char str[20];

		safe_snprintf(str, sizeof(str), "%d",storage_items[cur_item_over].quantity);

		show_help(str,mouse_x-win->pos_x-(strlen(str)/2)*8,mouse_y-win->pos_y-14);
	}

	// Render the grid *after* the images. It seems impossible to code
	// it such that images are rendered exactly within the boxes on all 
	// cards
	glDisable(GL_TEXTURE_2D);
	
	glColor3f(0.77f, 0.57f, 0.39f);
	
	glBegin(GL_LINE_LOOP);
		glVertex2i(10,  10);
		glVertex2i(10,  202);
		glVertex2i(130, 202);
		glVertex2i(130, 10);
	glEnd();

	glBegin(GL_LINE_LOOP);
		glVertex2i(10, 212);
		glVertex2i(10, 262);
		glVertex2i(392, 262);
		glVertex2i(392, 212);
	glEnd();

	rendergrid(6, 6, 160, 10, 32, 32);
	glEnable(GL_TEXTURE_2D);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 1;
}

int click_storage_handler(window_info * win, int mx, int my, Uint32 flags)
{
	if(flags&ELW_WHEEL_UP) {
		if(mx>10 && mx<130) {
			vscrollbar_scroll_up(storage_win, STORAGE_SCROLLBAR_CATEGORIES);
		} else if(mx>150 && mx<352){
			vscrollbar_scroll_up(storage_win, STORAGE_SCROLLBAR_ITEMS);
		}
	} else if(flags&ELW_WHEEL_DOWN) {
		if(mx>10 && mx<130) {
			vscrollbar_scroll_down(storage_win, STORAGE_SCROLLBAR_CATEGORIES);
		} else if(mx>150 && mx<352){
			vscrollbar_scroll_down(storage_win, STORAGE_SCROLLBAR_ITEMS);
		}
	}
	else if ( (flags & ELW_MOUSE_BUTTON) == 0) {
		return 0;
	}
	else {
		if(my>10 && my<202){
			if(mx>10 && mx<130){
				int cat=-1;
		
				cat=(my-20)/13 + vscrollbar_get_pos(storage_win, STORAGE_SCROLLBAR_CATEGORIES);
				move_to_category(cat);
			} else if(mx>150 && mx<352){
				if(item_dragged!=-1 && left_click){
					Uint8 str[6];

					str[0]=DEPOSITE_ITEM;
					str[1]=item_list[item_dragged].pos;
					*((Uint32*)(str+2))=SDL_SwapLE32(item_quantity);

					my_tcp_send(my_socket, str, 6);
	
					if(item_list[item_dragged].quantity<=item_quantity) item_dragged=-1;//Stop dragging this item...
				} else if(right_click){
					storage_item_dragged=-1;
					item_dragged=-1;

					if(cur_item_over!=-1) {
						Uint8 str[2];

						str[0]=LOOK_AT_STORAGE_ITEM;
						str[1]=storage_items[cur_item_over].pos;
	
						my_tcp_send(my_socket, str, 2);
	
						active_storage_item=storage_items[cur_item_over].pos;
					}
				} else if(cur_item_over!=-1){
					storage_item_dragged=cur_item_over;
					active_storage_item=storage_items[cur_item_over].pos;
				}
			}
		}
	}

	return 1;
}

int mouseover_storage_handler(window_info *win, int mx, int my)
{
	static int last_pos;
	
	cur_item_over=-1;
	
	if(my>10 && my<202){
		if(mx>10 && mx<130){
			int i;
			int pos=last_pos=(my-20)/13;
			int p;

			for(i=p=vscrollbar_get_pos(storage_win,STORAGE_SCROLLBAR_CATEGORIES);i<no_storage_categories;i++){
				if(i==selected_category) {
				} else if(i!=p+pos) {
					storage_categories[i].name[0]=127+c_orange1;
				} else storage_categories[i].name[0]=127+c_green2;
			}
			
			return 0;
		} else if (mx>150 && mx<352){
			cur_item_over = get_mouse_pos_in_grid(mx, my, 6, 6, 160, 10, 32, 32)+vscrollbar_get_pos(storage_win, STORAGE_SCROLLBAR_ITEMS)*6;
			if(cur_item_over>=no_storage||cur_item_over<0||!storage_items[cur_item_over].quantity) cur_item_over=-1;
		}
	}
	
	if(last_pos>=0 && last_pos<13){
		storage_categories[last_pos+vscrollbar_get_pos(storage_win,STORAGE_SCROLLBAR_CATEGORIES)].name[0]=127+c_orange1;
		last_pos=-1;
	}
	
	return 0;
}

void display_storage_menu()
{
	if(storage_win<=0){
		int our_root_win = -1;
		if (!windows_on_top) {
			our_root_win = game_root_win;
		}
		storage_win=create_window(win_storage, our_root_win, 0, storage_win_x, storage_win_y, storage_win_x_len, storage_win_y_len, ELW_WIN_DEFAULT|ELW_TITLE_NAME);
		set_window_handler(storage_win, ELW_HANDLER_DISPLAY, &display_storage_handler);
		set_window_handler(storage_win, ELW_HANDLER_CLICK, &click_storage_handler);
		set_window_handler(storage_win, ELW_HANDLER_MOUSEOVER, &mouseover_storage_handler);

		vscrollbar_add_extended(storage_win, STORAGE_SCROLLBAR_CATEGORIES, NULL, 130, 10, 20, 192, 0, 1.0, 0.77f, 0.57f, 0.39f, 0, 1, 
				max2i(no_storage_categories - STORAGE_CATEGORIES_DISPLAY, 0));
		vscrollbar_add_extended(storage_win, STORAGE_SCROLLBAR_ITEMS, NULL, 352, 10, 20, 192, 0, 1.0, 0.77f, 0.57f, 0.39f, 0, 1, 28);
	} else {
		int i;

		safe_snprintf(windows_list.window[storage_win].window_name, sizeof(windows_list.window[storage_win].window_name), win_storage);
		no_storage=0;
		
		for(i=0;i<no_storage_categories;i++)storage_categories[i].name[0]=127+c_orange1;

		show_window(storage_win);
		select_window(storage_win);

		vscrollbar_set_pos(storage_win, STORAGE_SCROLLBAR_CATEGORIES, 0);
		vscrollbar_set_pos(storage_win, STORAGE_SCROLLBAR_ITEMS, 0);
	}
}
void close_storagewin()
{
	if(storage_win >= 0) {
		hide_window(storage_win);
	}
}
