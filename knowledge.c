#include "global.h"
#include <string.h>

int view_knowledge=0;
int knowledge_menu_x=100;
int knowledge_menu_y=20;
int knowledge_menu_x_len=500;
int knowledge_menu_y_len=350;
int knowledge_menu_dragged=0;
int knowledge_scroll_dragged=0;

knowledge knowledge_list[300];

int knowledge_page_start=0;

char knowledge_string[400]="";
char *none="nothing";

void display_knowledge()
{
	int i,x=knowledge_menu_x+2,y=knowledge_menu_y+2;
	int progress = (125*your_info.research_completed+1)/(your_info.research_total+1);
	int scroll = (120*knowledge_page_start)/(300-38);
	char points_string[16];
	char *research_string;
	if(your_info.research_total && 
	   (your_info.research_completed==your_info.research_total))
		strcpy(points_string,"COMPLETE");
	else
		sprintf(points_string,"%4i/%-4i",your_info.research_completed,your_info.research_total);
	if(your_info.researching<300)
		research_string=knowledge_list[your_info.researching].name;
	else
		{
			research_string=none;
			points_string[0]='\0';
			progress=1;
		}

	//title bar
	draw_menu_title_bar(knowledge_menu_x,knowledge_menu_y-16,knowledge_menu_x_len);
	// window drawing
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,0.5f);
	glVertex3i(knowledge_menu_x,knowledge_menu_y+knowledge_menu_y_len,0);
	glVertex3i(knowledge_menu_x,knowledge_menu_y,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len,knowledge_menu_y,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len,knowledge_menu_y+knowledge_menu_y_len,0);
	glEnd();
	glDisable(GL_BLEND);
	glColor3f(0.77f,0.57f,0.39f);
	glBegin(GL_LINES);
	glVertex3i(knowledge_menu_x,knowledge_menu_y,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len,knowledge_menu_y,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len,knowledge_menu_y,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len,knowledge_menu_y+knowledge_menu_y_len,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len,knowledge_menu_y+knowledge_menu_y_len,0);
	glVertex3i(knowledge_menu_x,knowledge_menu_y+knowledge_menu_y_len,0);
	glVertex3i(knowledge_menu_x,knowledge_menu_y+knowledge_menu_y_len,0);
	glVertex3i(knowledge_menu_x,knowledge_menu_y,0);
	// window separators
	glVertex3i(knowledge_menu_x,knowledge_menu_y+200,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len,knowledge_menu_y+200,0);
	glVertex3i(knowledge_menu_x,knowledge_menu_y+300,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len,knowledge_menu_y+300,0);
	// X corner
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len,knowledge_menu_y+20,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len-20,knowledge_menu_y+20,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len-20,knowledge_menu_y+20,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len-20,knowledge_menu_y,0);
	//scroll bar
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len-20,knowledge_menu_y+20,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len-20,knowledge_menu_y+200,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len-15,knowledge_menu_y+30,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len-10,knowledge_menu_y+25,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len-10,knowledge_menu_y+25,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len-5,knowledge_menu_y+30,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len-15,knowledge_menu_y+185,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len-10,knowledge_menu_y+190,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len-10,knowledge_menu_y+190,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len-5,knowledge_menu_y+185,0);
	//progress bar
	glVertex3i(knowledge_menu_x+330,knowledge_menu_y+315,0);
	glVertex3i(knowledge_menu_x+455,knowledge_menu_y+315,0);
	glVertex3i(knowledge_menu_x+330,knowledge_menu_y+335,0);
	glVertex3i(knowledge_menu_x+455,knowledge_menu_y+335,0);
	glVertex3i(knowledge_menu_x+330,knowledge_menu_y+315,0);
	glVertex3i(knowledge_menu_x+330,knowledge_menu_y+335,0);
	glVertex3i(knowledge_menu_x+455,knowledge_menu_y+315,0);
	glVertex3i(knowledge_menu_x+455,knowledge_menu_y+335,0);
	glEnd();
	glBegin(GL_QUADS);
	//scroll bar
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len-13,knowledge_menu_y+35+scroll,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len-7,knowledge_menu_y+35+scroll,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len-7,knowledge_menu_y+55+scroll,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len-13,knowledge_menu_y+55+scroll,0);
	//progress bar
	glColor3f(0.40f,0.40f,1.00f);
	glVertex3i(knowledge_menu_x+331,knowledge_menu_y+316,0);
	glVertex3i(knowledge_menu_x+330+progress,knowledge_menu_y+316,0);
	glColor3f(0.10f,0.10f,0.80f);
	glVertex3i(knowledge_menu_x+330+progress,knowledge_menu_y+334,0);
	glVertex3i(knowledge_menu_x+331,knowledge_menu_y+334,0);
	glColor3f(0.77f,0.57f,0.39f);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	
	draw_string(knowledge_menu_x+knowledge_menu_x_len-16,knowledge_menu_y+2,"X",1);
	//draw text
	draw_string_small(knowledge_menu_x+4,knowledge_menu_y+210,knowledge_string,4);
	glColor3f(1.0f,1.0f,1.0f);
	draw_string_small(knowledge_menu_x+10,knowledge_menu_y+320,"Researching:",1);
	draw_string_small(knowledge_menu_x+120,knowledge_menu_y+320,research_string,1);
	draw_string_small(knowledge_menu_x+355,knowledge_menu_y+320,points_string,1);
	// Draw knowledges
	for(i=knowledge_page_start;i<knowledge_page_start+38;i++){
		knowledge_list[i].mouse_over ? glColor3f(0.1f,0.1f,0.9f) : glColor3f(0.9f,0.9f,0.9f);
		if(knowledge_list[i].present)
			draw_string_zoomed(x,y,knowledge_list[i].name,1,0.7);
		x+=240;
		if(i%2==1){y+=10;x=knowledge_menu_x+2;}
	}
	return;
}

int knowledge_mouse_over()
{
	int x,y;
	if(!view_knowledge || mouse_x>knowledge_menu_x+knowledge_menu_x_len || mouse_x<knowledge_menu_x
	   || mouse_y<knowledge_menu_y || mouse_y>knowledge_menu_y+knowledge_menu_y_len)return 0;
	for(x=0;x<200;x++)knowledge_list[x].mouse_over=0;
	x=mouse_x-knowledge_menu_x;
	y=mouse_y-knowledge_menu_y;
	if(x>knowledge_menu_x_len-20)
		return 1;
	if(y>192)
		return 1;
	x/=240;
	y/=10;
	knowledge_list[knowledge_page_start+y*2+x].mouse_over=1;
	return 1;
}


int check_knowledge_interface()
{
	int x,y,idx;
	char str[3];
	if(!view_knowledge || mouse_x>knowledge_menu_x+knowledge_menu_x_len || mouse_x<knowledge_menu_x
	   || mouse_y<knowledge_menu_y || mouse_y>knowledge_menu_y+knowledge_menu_y_len)return 0;

	x=mouse_x-knowledge_menu_x;
	y=mouse_y-knowledge_menu_y;
	if(x > knowledge_menu_x_len-16 && x < knowledge_menu_x_len &&
	   y > 18 && y < 18+16)
		{
			if(knowledge_page_start > 0)
				knowledge_page_start -= 16;
			return 1;
		}
	if(x > knowledge_menu_x_len-16 && x < knowledge_menu_x_len &&
	   y > 180 && y < 180+16)
		{
			if(knowledge_page_start < 300-38-16)
				knowledge_page_start += 16;
			return 1;
		}
	if(x>knowledge_menu_x_len-20)
		return 1;
	if(y>192)
		return 1;
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
