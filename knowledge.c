#include <string.h>
#include "global.h"
#include "elwindows.h"

int knowledge_win= 0;
int knowledge_menu_x=100;
int knowledge_menu_y=20;
int knowledge_menu_x_len=500;
int knowledge_menu_y_len=350;
//int knowledge_menu_dragged=0;
//int knowledge_scroll_dragged=0;

knowledge knowledge_list[300];

int knowledge_page_start=0;

char knowledge_string[400]="";

int display_knowledge_handler(window_info *win)
{
	int i,x=2,y=2;
	int progress = (125*your_info.research_completed+1)/(your_info.research_total+1);
	int scroll = (120*knowledge_page_start)/(300-38);
	char points_string[16];
	char *research_string;
	
	if(your_info.research_total && 
	   (your_info.research_completed==your_info.research_total))
		strncpy(points_string,completed_research,10);
	else
		sprintf(points_string,"%4i/%-4i",your_info.research_completed,your_info.research_total);
	if(your_info.researching<300)
		research_string=knowledge_list[your_info.researching].name;
	else
		{
			research_string=not_researching_anything;
			points_string[0]='\0';
			progress=1;
		}

	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);
	glBegin(GL_LINES);
	// window separators
	glVertex3i(0,200,0);
	glVertex3i(win->len_x,200,0);
	glVertex3i(0,300,0);
	glVertex3i(win->len_x,300,0);
	//scroll bar
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
	//progress bar
	glVertex3i(330,315,0);
	glVertex3i(455,315,0);
	glVertex3i(330,335,0);
	glVertex3i(455,335,0);
	glVertex3i(330,315,0);
	glVertex3i(330,335,0);
	glVertex3i(455,315,0);
	glVertex3i(455,335,0);
	glEnd();
	glBegin(GL_QUADS);
	//scroll bar
	glVertex3i(win->len_x-13,35+scroll,0);
	glVertex3i(win->len_x-7,35+scroll,0);
	glVertex3i(win->len_x-7,55+scroll,0);
	glVertex3i(win->len_x-13,55+scroll,0);
	//progress bar
	glColor3f(0.40f,0.40f,1.00f);
	glVertex3i(331,316,0);
	glVertex3i(330+progress,316,0);
	glColor3f(0.10f,0.10f,0.80f);
	glVertex3i(330+progress,334,0);
	glVertex3i(331,334,0);
	glColor3f(0.77f,0.57f,0.39f);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	
	//draw text
	draw_string_small(4,210,knowledge_string,4);
	glColor3f(1.0f,1.0f,1.0f);
	draw_string_small(10,320,researching_str,1);
	draw_string_small(120,320,research_string,1);
	draw_string_small(355,320,points_string,1);
	// Draw knowledges
	for(i=knowledge_page_start;i<knowledge_page_start+38;i++){
		knowledge_list[i].mouse_over ? glColor3f(0.1f,0.1f,0.9f) : glColor3f(0.9f,0.9f,0.9f);
		if(knowledge_list[i].present)
			draw_string_zoomed(x,y,knowledge_list[i].name,1,0.7);
		x+=240;
		if(i%2==1){y+=10;x=2;}
	}
	return 1;
}

int mouseover_knowledge_handler(window_info *win, int mx, int my)
{
	int	i;

	for(i=0;i<200;i++)knowledge_list[i].mouse_over=0;
	if(mx>win->len_x-20)
		return 0;
	if(my>192)
		return 0;
	mx/=240;
	my/=10;
	knowledge_list[knowledge_page_start+my*2+mx].mouse_over=1;
	return 0;
}


int click_knowledge_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int x,y,idx;
	char str[3];

	x= mx;
	y= my;
	if(x > win->len_x-16 && y > 18 && y < 18+16)
		{
			knowledge_page_start -= 16;
			if(knowledge_page_start < 0)
				knowledge_page_start = 0;
			return 1;
		}
	if(x > win->len_x-16 && y > 180 && y < 180+16)
		{
			knowledge_page_start += 16;
			if(knowledge_page_start > 262)
				knowledge_page_start = 262;
			return 1;
		}
	if(x>win->len_x-20)
		return 0;
	if(y>192)
		return 0;
	x/=240;
	y/=10;
	idx = knowledge_page_start+y*2+x;
	if(idx < 200 && knowledge_list[idx].present)
		{
			str[0] = GET_KNOWLEDGE_INFO;
			*(Uint16 *)(str+1) = idx;
			my_tcp_send(my_socket,str,3);
		}
	return 1;
} 


int drag_knowledge_handler(window_info *win, int mx, int my, Uint32 flags, int dx, int dy)
{
	if(win->drag_in || (mx>win->len_x-20 && my>35+(120*knowledge_page_start)/(300-38) && my<55+(120*knowledge_page_start)/(300-38))) {
		win->drag_in= 1;
		//if(left_click>1)
		knowledge_page_start+=((300-38)*dy)/120;
		// bounds checking
		if(knowledge_page_start%2==1) 
			knowledge_page_start--;
		if(knowledge_page_start < 0) knowledge_page_start= 0;
		if(knowledge_page_start > 300-38) knowledge_page_start= 300-38;
		return 1;
	}
	return 0;
}


void get_knowledge_list(Uint16 size, char *list)
{
	int i;
	for(i=0;i<size;i++)
		{
			knowledge_list[i*8+0].present=list[i] & 0x01;
			knowledge_list[i*8+1].present=list[i] & 0x02;
			knowledge_list[i*8+2].present=list[i] & 0x04;
			knowledge_list[i*8+3].present=list[i] & 0x08;
			knowledge_list[i*8+4].present=list[i] & 0x10;
			knowledge_list[i*8+5].present=list[i] & 0x20;
			knowledge_list[i*8+6].present=list[i] & 0x40;
			knowledge_list[i*8+7].present=list[i] & 0x80;
		}
}

void get_new_knowledge(Uint16 idx)
{
	knowledge_list[idx].present=1;
}

void display_knowledge()
{
	if(knowledge_win <= 0){
		knowledge_win= create_window("Knowledge", 0, 0, knowledge_menu_x, knowledge_menu_y, knowledge_menu_x_len, knowledge_menu_y_len, ELW_WIN_DEFAULT);

		set_window_handler(knowledge_win, ELW_HANDLER_DISPLAY, &display_knowledge_handler );
		set_window_handler(knowledge_win, ELW_HANDLER_CLICK, &click_knowledge_handler );
		set_window_handler(knowledge_win, ELW_HANDLER_DRAG, &drag_knowledge_handler );
		set_window_handler(knowledge_win, ELW_HANDLER_MOUSEOVER, &mouseover_knowledge_handler );
	} else {
		show_window(knowledge_win);
		select_window(knowledge_win);
	}
	display_window(knowledge_win);
}

