#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "elwindows.h"

int buddy_win=0;
int buddy_menu_x=150;
int buddy_menu_y=70;
int buddy_menu_x_len=150;
int buddy_menu_y_len=200;
//int buddy_menu_dragged=0;
_buddy buddy_list[100];
int bpage_start = 0;


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
	int scroll = (130*bpage_start)/(100-19);

	glDisable(GL_BLEND);
	glColor3f(0.77f,0.57f,0.39f);
	//scroll bar
	glBegin(GL_LINES);
	glVertex3i(buddy_menu_x_len-20,20,0);
	glVertex3i(buddy_menu_x_len-20,200,0);
	glVertex3i(buddy_menu_x_len-15,30,0);
	glVertex3i(buddy_menu_x_len-10,25,0);
	glVertex3i(buddy_menu_x_len-10,25,0);
	glVertex3i(buddy_menu_x_len-5,30,0);
	glVertex3i(buddy_menu_x_len-15,185,0);
	glVertex3i(buddy_menu_x_len-10,190,0);
	glVertex3i(buddy_menu_x_len-10,190,0);
	glVertex3i(buddy_menu_x_len-5,185,0);
	glEnd();
	glBegin(GL_QUADS);
	//scroll bar
	glVertex3i(buddy_menu_x_len-13,35+scroll,0);
	glVertex3i(buddy_menu_x_len-7,35+scroll,0);
	glVertex3i(buddy_menu_x_len-7,55+scroll,0);
	glVertex3i(buddy_menu_x_len-13,55+scroll,0);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	
	// Draw budies
	qsort(buddy_list,100,sizeof(_buddy),compare2);
	i=bpage_start;
	glColor3f(1.0,1.0,1.0);
	while(c==buddy_list[i].type){
		if(i-bpage_start>18)return 1;
		draw_string_zoomed(x,y,buddy_list[i].name,1,0.7);
		y+=10;
		i++;
	}
	c++;

	glColor3f(1.0,0,0);
	while(c==buddy_list[i].type){
		if(i-bpage_start>18)return 1;
		draw_string_zoomed(x,y,buddy_list[i].name,1,0.7);
		y+=10;
		i++;
	}
	c++;

	glColor3f(0,1.0,0);
	while(c==buddy_list[i].type){
		if(i-bpage_start>18)return 1;
		draw_string_zoomed(x,y,buddy_list[i].name,1,0.7);
		y+=10;
		i++;
	}
	c++;
	glColor3f(0,0,1.0);
	while(c==buddy_list[i].type){
		if(i-bpage_start>18)return 1;
		draw_string_zoomed(x,y,buddy_list[i].name,1,0.7);
		y+=10;
		i++;
	}
	c++;
	glColor3f(1.0,1.0,0);
	while(c==buddy_list[i].type){
		if(i-bpage_start>18)return 1;
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

	if(x > buddy_menu_x_len-16 && x < buddy_menu_x_len &&
		y > 18 && y < 18+16)
		{
			if(bpage_start > 0)
				bpage_start--;
			return 1;
		}
	if(x > buddy_menu_x_len-16 && x < buddy_menu_x_len &&
		y > 180 && y < 180+16)
		{
			if(bpage_start < 100-19)
				bpage_start++;
			return 1;
		}
	if(x>buddy_menu_x_len-20)
		return 0;
	
	y/= 10;
	y+= bpage_start;
	sprintf(input_text_line,"/%s ",buddy_list[y].name);
	input_text_lenght= strlen(input_text_line);
	return 1;
}

void init_buddy()
{
	int i;

	for(i=0;i<100;i++)
		{
			buddy_list[i].type=0xff;
		}
	
}

void display_buddy()
{
	if(buddy_win <= 0)
		{
			buddy_win= create_window("Buddy", 0, 0, buddy_menu_x, buddy_menu_y, buddy_menu_x_len, buddy_menu_y_len, ELW_WIN_DEFAULT);
			set_window_handler(buddy_win, ELW_HANDLER_DISPLAY, &display_buddy_handler );
			set_window_handler(buddy_win, ELW_HANDLER_CLICK, &click_buddy_handler );
		}
	else
		{
			show_window(buddy_win);
			select_window(buddy_win);
		}
}

void add_buddy(char *n, int t)
{
	int i;

	//find empty space
	for(i=0;i<100;i++){
		if(buddy_list[i].type==0xff){//found then add buddy
			buddy_list[i].type=t;
			strcpy(buddy_list[i].name,n);
			break;
		}
	}
}

void del_buddy(char *n)
{
	int i;

	//find buddy
	for(i=0;i<100;i++){
		if(!strcmp(n,buddy_list[i].name)){
			buddy_list[i].type=0xff;
			break;
		}
		
	}

}

