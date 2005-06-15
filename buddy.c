#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "elwindows.h"

#define	MAX_BUDDY	100

int buddy_win=-1;
int buddy_menu_x=150;
int buddy_menu_y=70;
int buddy_menu_x_len=150;
int buddy_menu_y_len=200;
_buddy buddy_list[MAX_BUDDY];


int compare2( const void *arg1, const void *arg2)
{
	_buddy *b1=(_buddy*)arg1, *b2=(_buddy*)arg2;
	if(b1->type==b2->type)
		return strcmp(b1->name,b2->name);
	else
		return b1->type>b2->type ? 1: -1;
}

int display_buddy_handler(window_info *win)
{
	int i=0,x=2,y=2;
	glEnable(GL_TEXTURE_2D);
	// Draw budies
	qsort(buddy_list,MAX_BUDDY,sizeof(_buddy),compare2);
	for(i=vscrollbar_get_pos(buddy_win,12);i<(vscrollbar_get_pos(buddy_win,12)+19);i++){
		switch(buddy_list[i].type){
			case 0:glColor3f(1.0,1.0,1.0);break;
			case 1:glColor3f(1.0,0,0);break;
			case 2:glColor3f(0,1.0,0);break;
			case 3:glColor3f(0,0,1.0);break;
			case 4:glColor3f(1.0,1.0,0);break;
		}
		draw_string_zoomed(x,y,buddy_list[i].name,1,0.7);
		y+=10;
	}
	return 1;
}

int click_buddy_handler (window_info *win, int mx, int my, Uint32 flags)
{
	int x=mx,y=my;

	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	if(x>win->len_x-20)
		return 0;
	// clicked on a buddies name, start apm to them
	y /= 10;
	y += vscrollbar_get_pos(buddy_win,12);
	put_char_in_buffer (&input_text_line, '/', 0);
	put_string_in_buffer (&input_text_line, buddy_list[y].name, 1);
	put_char_in_buffer ( &input_text_line, ' ', 1+strlen (buddy_list[y].name) );
	return 1;
}

void init_buddy()
{
	int i;

	for(i=0; i<MAX_BUDDY; i++)
		{
			buddy_list[i].type= 0xff;
		}
}

/*
int clika(widget_list *w){
	w->pos_x+=10;
	return 0;
}
int clikaa(widget_list *w){
	progressbar *b = (progressbar *)w->widget_info;
	b->progress++;
	return 0;
}
*/
void display_buddy()
{
	if(buddy_win < 0)
		{
			//buddy_win = AddXMLWindow("buddy.xml");
			buddy_win = create_window("Buddy", game_root_win, 0, buddy_menu_x, buddy_menu_y, buddy_menu_x_len, buddy_menu_y_len, ELW_WIN_DEFAULT);

			set_window_handler(buddy_win, ELW_HANDLER_DISPLAY, &display_buddy_handler );
			set_window_handler(buddy_win, ELW_HANDLER_CLICK, &click_buddy_handler );

			vscrollbar_add_extended (buddy_win, 12, NULL, 130, 20, 20, 180, 0, 1.0, 0.77f, 0.57f, 0.39f, 0, 1, MAX_BUDDY-19);
		
		}
	else
		{
			show_window(buddy_win);
			select_window(buddy_win);
		}
}

void add_buddy(char *n, int t, int len)
{
	int i;

	//find empty space
	for(i=0; i<MAX_BUDDY; i++){
		if(buddy_list[i].type == 0xff){//found then add buddy
			buddy_list[i].type= t;
			strncpy(buddy_list[i].name,n,len);
			buddy_list[i].name[len]= '\0';
			break;
		}
	}
}

void del_buddy(char *n, int len)
{
	int i;

	//find buddy
	for(i=0; i<MAX_BUDDY; i++){
		if(!strncmp(n,buddy_list[i].name, len)){
			buddy_list[i].type= 0xff;
			break;
		}
		
	}
}

void clear_buddy()
{
	int i;
	for(i=0; i<MAX_BUDDY; i++)
			buddy_list[i].type= 0xff;
}
