#include <stdio.h>
#include <string.h>
#include "global.h"
#include "storage.h"

struct storage_category {
	char name[25];
	int id;
	int color;
} storage_categories[50];

int no_storage_categories=0;
int selected_category=-1;

ground_item storage_items[200]={{0,0,0}};
int no_storage;

char storage_text[202]={0};

void get_storage_text(Uint8 * in_data, int len)
{
	if(len>200)len=200;
	strncpy(storage_text, in_data, len);
	storage_text[len]=0;
}

void get_storage_categories(char * in_data, int len)
{
	int i;
	char * ptr=in_data+1;

	for(i=0;i<in_data[0] && i<50;i++){
		char * sptr=storage_categories[i].name+1;
		
		storage_categories[i].name[0]=127+c_orange1;
		
		storage_categories[i].id=*((Uint8*)(ptr++));
		while((*sptr++=*ptr++));
	}

	storage_categories[i].id=-1;
	
	no_storage_categories=in_data[0];

	display_storage_menu();
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
	char str[4];
		
	if(cat<0||cat>=no_storage_categories) return;
	storage_categories[cat].name[0]=127+c_red3;
	if(selected_category!=-1) storage_categories[selected_category].name[0]=127+c_orange1;
	sprintf(windows_list.window[storage_win].window_name, "Storage - %s", storage_categories[cat].name+1);
	selected_category=cat;

	str[0]=GET_STORAGE_CATEGORY;
	*((Uint8 *)(str+1))=storage_categories[cat].id;

	my_tcp_send(my_socket, str, 2);
}

void get_storage_items(Uint8 * in_data, int len)
{
	int i;
	char * ptr;

	if(in_data[0]==255){
		//It's just an update - make sure we're in the right category
		ptr=in_data+2;
		if(selected_category==-1||storage_categories[selected_category].id!=in_data[1]) {
			move_to_category(find_category(in_data[1]));
			return;
		}
		for(i=0;i<200;i++){
			if(storage_items[i].pos==*((Uint8*)(ptr+6))){
				storage_items[i].image_id=SDL_SwapLE16(*((Uint16*)(ptr)));
				storage_items[i].quantity=SDL_SwapLE32(*((Uint32*)(ptr+2)));
			}
		}
		return;
	}
	
	no_storage=0;

	ptr=in_data+1;
	
	for(i=0;i<in_data[0] && no_storage<200;i++,ptr+=7){
		storage_items[no_storage].image_id=SDL_SwapLE16(*((Uint16*)(ptr)));
		storage_items[no_storage].quantity=SDL_SwapLE32(*((Uint32*)(ptr+2)));
		storage_items[no_storage].pos=*((Uint8*)(ptr+6));
		no_storage++;
	}
	
	no_storage=i;
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
	
	for(i=pos=vscrollbar_get_pos(storage_win,1200); i<200 && storage_categories[i].id!=-1 && i<pos+13; i++,n++){
		draw_string_small(20, 20+n*13, storage_categories[i].name,1);
	}

	if(storage_text[0]){
		draw_string_small(18, 220, storage_text, 1);
	}

	glColor3f(1.0f,1.0f,1.0f);
	
	for(i=pos=6*vscrollbar_get_pos(storage_win, 1201); i<no_storage && i<pos+36 && storage_items[i].quantity;i++){
		GLfloat u_start, v_start, u_end, v_end;
		int x_start, x_end, y_start, y_end;
		int cur_item;
		GLuint this_texture;

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

	if(cur_item_over!=-1){
		char str[20];

		sprintf(str,"%d",storage_items[cur_item_over].quantity);

		show_help(str,mouse_x-win->pos_x-(strlen(str)/2)*8,mouse_y-win->pos_y-14);
	}

	return 1;
}

int click_storage_handler(window_info * win, int mx, int my, Uint32 flags)
{
	if(my>10 && my<202){
		if(mx>10 && mx<130){
			int cat=-1;
	
			cat=(my-20)/13 + vscrollbar_get_pos(storage_win, 1200);
			move_to_category(cat);
		} else if(mx>150 && mx<352){
			if(item_dragged!=-1){
				char str[6];

				str[0]=DEPOSITE_ITEM;
				str[1]=item_list[item_dragged].pos;
				*((Uint32*)(str+2))=SDL_SwapLE32(item_quantity);

				my_tcp_send(my_socket, str, 6);

				item_dragged=-1;
			} else if(right_click){
				char str[2];

				str[0]=LOOK_AT_STORAGE_ITEM;
				str[1]=storage_items[cur_item_over].pos;

				my_tcp_send(my_socket, str, 2);
			} else {
				if(cur_item_over!=-1){
					storage_item_dragged=cur_item_over;
					printf("%d\n",storage_item_dragged);
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

			for(i=p=vscrollbar_get_pos(storage_win,1200);i<no_storage_categories;i++){
				if(i==selected_category) {
				} else if(i!=p+pos) {
					storage_categories[i].name[0]=127+c_orange1;
				} else storage_categories[i].name[0]=127+c_green2;
			}
			
			return 0;
		} else if (mx>150 && mx<352){
			cur_item_over = get_mouse_pos_in_grid(mx, my, 6, 6, 160, 10, 32, 32)+vscrollbar_get_pos(storage_win, 1201)*6;
			if(!storage_items[cur_item_over].quantity||cur_item_over>=no_storage) cur_item_over=-1;
		}
	}
	
	if(last_pos>=0 && last_pos<13){
		storage_categories[last_pos+vscrollbar_get_pos(storage_win,1200)].name[0]=127+c_orange1;
		last_pos=-1;
	}
	
	return 0;
}

void display_storage_menu()
{
	if(storage_win<=0){
		int sb_len=20;
		
		storage_win=create_window("Storage", game_root_win, 0, storage_win_x, storage_win_y, storage_win_x_len, storage_win_y_len, ELW_WIN_DEFAULT|ELW_TITLE_NAME);

		set_window_handler(storage_win, ELW_HANDLER_DISPLAY, &display_storage_handler);
		set_window_handler(storage_win, ELW_HANDLER_CLICK, &click_storage_handler);
		set_window_handler(storage_win, ELW_HANDLER_MOUSEOVER, &mouseover_storage_handler);

		if(no_storage_categories) {
			sb_len=no_storage_categories-13;
			if(sb_len<=0)sb_len=1;
		}

		vscrollbar_add_extended(storage_win, 1200, NULL, 130, 10, 20, 192, 0, 1.0, 0.77f, 0.57f, 0.39f, 0, 1, sb_len);
		vscrollbar_add_extended(storage_win, 1201, NULL, 352, 10, 20, 192, 0, 1.0, 0.77f, 0.57f, 0.39f, 0, 1, 28);
	} else {
		int i;

		strcpy(windows_list.window[storage_win].window_name, "Storage");
		no_storage=0;
		
		for(i=0;i<no_storage_categories;i++)storage_categories[i].name[0]=127+c_orange1;
	
		show_window(storage_win);
	//	select_window(storage_win);
	}
}
