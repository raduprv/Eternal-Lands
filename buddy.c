#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "elwindows.h"

int buddy_win=0;
int buddy_menu_x=150;
int buddy_menu_y=70;
int buddy_menu_x_len=150;
int buddy_menu_y_len=200;
_buddy buddy_list[MAX_BUDDY];
int buddy_page_start = 0;	// first buddy number to show
int buddy_page_pos = 0;		// where the scrollbar is


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
	int c=0,i=0,x=2,y=2;
	int scroll = buddy_page_pos;

	//glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);
	//scroll bar
	glBegin(GL_LINES);
	glVertex3i(win->len_x-20,20,0);
	glVertex3i(win->len_x-20,200,0);
	glVertex3i(win->len_x-15,30,0);
	glVertex3i(win->len_x-10,25,0);
	glVertex3i(win->len_x-10,25,0);
	glVertex3i(win->len_x-5,30,0);
	glVertex3i(win->len_x-15,185,0);
	glVertex3i(win->len_x-10,190,0);
	glVertex3i(win->len_x-10,190,0);
	glVertex3i(win->len_x-5,185,0);
	glEnd();
	glBegin(GL_QUADS);
	//scroll bar
	glVertex3i(win->len_x-13,35+scroll,0);
	glVertex3i(win->len_x-7,35+scroll,0);
	glVertex3i(win->len_x-7,55+scroll,0);
	glVertex3i(win->len_x-13,55+scroll,0);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	
	// Draw budies
	qsort(buddy_list,MAX_BUDDY,sizeof(_buddy),compare2);
	i= buddy_page_start;
	// TODO: less code?
	glColor3f(1.0,1.0,1.0);
	while(c==buddy_list[i].type){
		if(i-buddy_page_start>18)	return 1;
		draw_string_zoomed(x,y,buddy_list[i].name,1,0.7);
		y+=10;
		i++;
	}
	c++;

	glColor3f(1.0,0,0);
	while(c==buddy_list[i].type){
		if(i-buddy_page_start>18)	return 1;
		draw_string_zoomed(x,y,buddy_list[i].name,1,0.7);
		y+=10;
		i++;
	}
	c++;

	glColor3f(0,1.0,0);
	while(c==buddy_list[i].type){
		if(i-buddy_page_start>18)	return 1;
		draw_string_zoomed(x,y,buddy_list[i].name,1,0.7);
		y+=10;
		i++;
	}
	c++;
	glColor3f(0,0,1.0);
	while(c==buddy_list[i].type){
		if(i-buddy_page_start>18)	return 1;
		draw_string_zoomed(x,y,buddy_list[i].name,1,0.7);
		y+=10;
		i++;
	}
	c++;
	glColor3f(1.0,1.0,0);
	while(c==buddy_list[i].type){
		if(i-buddy_page_start>18)	return 1;
		draw_string_zoomed(x,y,buddy_list[i].name,1,0.7);
		y+=10;
		i++;
	}

	return 1;
}

int click_buddy_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int x,y;

	x= mx;
	y= my;

	if(x > win->len_x-16 && y > 18 && y < 18+16)
		{
			if(buddy_page_start > 0)
				{
					buddy_page_start--;
					// calc new pos for scroll bar
					buddy_page_pos= ((win->len_y-20-30-20)*buddy_page_start)/MAX_BUDDY;
				}
			return 1;
		}
	if(x > win->len_x-16 && y > 180 && y < 180+16)
		{
			if(buddy_page_start < MAX_BUDDY-19)
				{
					buddy_page_start++;
					// calc new pos for scroll bar
					buddy_page_pos= ((win->len_y-20-30-20)*buddy_page_start)/MAX_BUDDY;
				}
			return 1;
		}
	if(x>win->len_x-20)
		return 0;
	
	// clicked on a buddies name, start apm to them
	y/= 10;
	y+= buddy_page_start;
	sprintf(input_text_line,"/%s ",buddy_list[y].name);
	input_text_lenght= strlen(input_text_line);
	return 1;
}

int drag_buddy_handler(window_info *win, int mx, int my, Uint32 flags, int dx, int dy)
{
	int scroll_area= win->len_y-20-30-20;	// account for the X ^ V and bar height

	if(win->drag_in || (mx>win->len_x-20 && my>35+buddy_page_pos && my<55+buddy_page_pos)) {
		win->drag_in= 1;
		//if(left_click>1)
		buddy_page_pos+= dy;
		// bounds checking
		if(buddy_page_pos < 0) buddy_page_pos= 0;
		if(buddy_page_pos >= scroll_area) buddy_page_pos= scroll_area-1;
		//and set which item to list first
		buddy_page_start=(MAX_BUDDY*buddy_page_pos)/scroll_area;
		return 1;
	}
	return 0;
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
	if(buddy_win <= 0)
		{
			//buddy_win = AddXMLWindow("buddy.xml");
			buddy_win = create_window("Buddy", 0, 0, buddy_menu_x, buddy_menu_y, buddy_menu_x_len, buddy_menu_y_len, ELW_WIN_DEFAULT);
			set_window_handler(buddy_win, ELW_HANDLER_DISPLAY, &display_buddy_handler );
			set_window_handler(buddy_win, ELW_HANDLER_CLICK, &click_buddy_handler );
			set_window_handler(buddy_win, ELW_HANDLER_DRAG, &drag_buddy_handler );
		/*	
			label_add(buddy_win,NULL,"Hello!",0,0);
			widget_set_size(buddy_win,0,2.0);
			widget_set_color(buddy_win,0,0.5,0.5,0.5);
			widget_move(buddy_win,0,20,20);
			label_set_text(buddy_win,0,"Bye!");
			widget_set_OnClick(buddy_win,0,clika);
			widget_set_OnMouseover(buddy_win,0,clika);
			image_add(buddy_win,NULL,load_texture_cache("./textures/sky.bmp",70),0,30,64,64,0.0,0.0,1.0,1.0);
			widget_set_OnClick(buddy_win,1,clika);
			widget_set_OnMouseover(buddy_win,1,clika);
			widget_resize(buddy_win,1,128,128);
			checkbox_add(buddy_win, NULL, 2, 100,20,20,0);
			button_add(buddy_win,NULL,"Hello!",0,150);
			widget_set_color(buddy_win,3,1.5,0.0,0.5);
			widget_set_OnClick(buddy_win,3,clika);
			progressbar_add(buddy_win,NULL,50,50,20,200);
			progressbar_set_progress(buddy_win,4,25);
			widget_set_OnClick(buddy_win,4,clikaa);
			vscrollbar_add(buddy_win,NULL,10,10,20,100,100,10);
		*/
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

