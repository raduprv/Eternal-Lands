#include "global.h"
#include <string.h>

_knowledge knowledge_list[200];

char research_name[16]="none";
short research_points[2]={0,0};
char points_string[16]="   0/0   ";
int page_start = 0;

void display_knowledge()
{
	int i,x=knowledge_menu_x+2,y=knowledge_menu_y+2;
	int progress = (125*research_points[0]+1)/(research_points[1]+1);
	int scroll = (130*page_start)/(200-76);
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
	glVertex3i(knowledge_menu_x+250,knowledge_menu_y+315,0);
	glVertex3i(knowledge_menu_x+375,knowledge_menu_y+315,0);
	glVertex3i(knowledge_menu_x+250,knowledge_menu_y+335,0);
	glVertex3i(knowledge_menu_x+375,knowledge_menu_y+335,0);
	glVertex3i(knowledge_menu_x+250,knowledge_menu_y+315,0);
	glVertex3i(knowledge_menu_x+250,knowledge_menu_y+335,0);
	glVertex3i(knowledge_menu_x+375,knowledge_menu_y+315,0);
	glVertex3i(knowledge_menu_x+375,knowledge_menu_y+335,0);
	glEnd();
	glBegin(GL_QUADS);
	//scroll bar
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len-13,knowledge_menu_y+35+scroll,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len-7,knowledge_menu_y+35+scroll,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len-7,knowledge_menu_y+55+scroll,0);
	glVertex3i(knowledge_menu_x+knowledge_menu_x_len-13,knowledge_menu_y+55+scroll,0);
	//progress bar
	glColor3f(0.40f,0.40f,1.00f);
	glVertex3i(knowledge_menu_x+251,knowledge_menu_y+316,0);
	glVertex3i(knowledge_menu_x+250+progress,knowledge_menu_y+316,0);
	glColor3f(0.10f,0.10f,0.80f);
	glVertex3i(knowledge_menu_x+250+progress,knowledge_menu_y+334,0);
	glVertex3i(knowledge_menu_x+251,knowledge_menu_y+334,0);
	glColor3f(0.77f,0.57f,0.39f);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	
	draw_string(knowledge_menu_x+knowledge_menu_x_len-16,knowledge_menu_y+2,"X",1);
	//draw text
	draw_string_small(knowledge_menu_x+4,knowledge_menu_y+215,items_string,4);
	glColor3f(1.0f,1.0f,1.0f);
	draw_string_small(knowledge_menu_x+10,knowledge_menu_y+320,"Researching:",1);
	draw_string_small(knowledge_menu_x+120,knowledge_menu_y+320,research_name,1);
	draw_string_small(knowledge_menu_x+275,knowledge_menu_y+320,points_string,1);
	// Draw knowledges
	for(i=page_start;i<page_start+76;i++){
		knowledge_list[i].mouse_over ? glColor3f(0.1f,0.1f,0.9f) : glColor3f(0.9f,0.9f,0.9f);
		draw_string_zoomed(x,y,knowledge_list[i].name,1,0.7);
		x+=99;
		if(i%4==3){y+=10;x=knowledge_menu_x+2;}
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
	x/=99;
	y/=10;
	knowledge_list[page_start+y*4+x].mouse_over=1;
	return 1;
}


int check_knowledge_interface()
{
	int x,y,len,idx;
	char str[100];
	if(!view_knowledge || mouse_x>knowledge_menu_x+knowledge_menu_x_len || mouse_x<knowledge_menu_x
	   || mouse_y<knowledge_menu_y || mouse_y>knowledge_menu_y+knowledge_menu_y_len)return 0;

	x=mouse_x-knowledge_menu_x;
	y=mouse_y-knowledge_menu_y;
	if(x > knowledge_menu_x_len-16 && x < knowledge_menu_x_len &&
	   y > 18 && y < 18+16)
		{
			if(page_start > 0)
				page_start -= 4;
			return 1;
		}
	if(x > knowledge_menu_x_len-16 && x < knowledge_menu_x_len &&
	   y > 180 && y < 180+16)
		{
			if(page_start < 200-76-4)
				page_start += 4;
			return 1;
		}
	if(x>knowledge_menu_x_len-20)
		return 1;
	x/=99;
	y/=10;
	idx = page_start+y*4+x;
	if(idx < 200 && *(knowledge_list[idx].name))
		{
			str[0] = RAW_TEXT;
			len = strlen(knowledge_list[idx].name) + 5;
			strcpy(str+1,"#ki ");
			strcpy(str+5,knowledge_list[idx].name);
			str[len+1]=0;
			my_tcp_send(my_socket,str,len);
		}
	return 1;
} 

void get_knowledge_list(char *list)
{
	int i=0,j=0,k;
	for(k=0;k<200;k++)
		{
			knowledge_list[k].name[0]='\0';
		}
	while(*list)
		{
			if(*list == ',')
				{
					knowledge_list[i].name[j]='\0';
					j=0;
					i++;
				}
			else
				{
					if(j<15)
						{
							knowledge_list[i].name[j]=*list;
							j++;
						}
				}
			list++;
		}
}

void get_research_info(Uint16 points_remain, Uint16 points_total, char *name)
{
	research_points[0]=points_remain;
	research_points[1]=points_total;
	sprintf(points_string,"%4i/%-4i",points_remain,points_total);
	strncpy(research_name,name,15);
	research_name[15]='\0';
}
