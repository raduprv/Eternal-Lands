#include "global.h"

int view_o3dow=0;
int o3dow_x=100;
int o3dow_y=400;
int o3dow_x_len=150;
int o3dow_y_len=100;
int o3dow_dragged=0;
int o3dow_win=0;

int c1=0,c1_x1=100,c1_x2=114,c1_y1=13,c1_y2=27;
int c2=0,c2_x1=100,c2_x2=114,c2_y1=33,c2_y2=47;
int c3=0,c3_x1=100,c3_x2=114,c3_y1=53,c3_y2=67;
int c4=0,c4_x1=100,c4_x2=114,c4_y1=73,c4_y2=87;

void display_o3dow()
{
	if(o3dow_win <= 0){
		o3dow_win= create_window("o3dow", 0, 0, o3dow_x, o3dow_y, o3dow_x_len, o3dow_y_len, ELW_WIN_DEFAULT);

		set_window_handler(o3dow_win, ELW_HANDLER_DISPLAY, &display_o3dow_handler );
		set_window_handler(o3dow_win, ELW_HANDLER_CLICK, &check_o3dow_interface );
		
	} else {
		show_window(o3dow_win);
		select_window(o3dow_win);
	}
	display_window(o3dow_win);
}

int display_o3dow_handler()
{

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glColor3f(0.77f,0.57f,0.39f);
	
	// Checkboxes
	glBegin(c1 ? GL_QUADS: GL_LINE_LOOP);
	glVertex3i(c1_x1,c1_y1,0);
	glVertex3i(c1_x2,c1_y1,0);
	glVertex3i(c1_x2,c1_y2,0);
	glVertex3i(c1_x1,c1_y2,0);
	glEnd();
	
	glBegin(c2 ? GL_QUADS: GL_LINE_LOOP);
	glVertex3i(c2_x1,c2_y1,0);
	glVertex3i(c2_x2,c2_y1,0);
	glVertex3i(c2_x2,c2_y2,0);
	glVertex3i(c2_x1,c2_y2,0);
	glEnd();
	
	glBegin(c3 ? GL_QUADS: GL_LINE_LOOP);
	glVertex3i(c3_x1,c3_y1,0);
	glVertex3i(c3_x2,c3_y1,0);
	glVertex3i(c3_x2,c3_y2,0);
	glVertex3i(c3_x1,c3_y2,0);
	glEnd();

	glBegin(c4 ? GL_QUADS: GL_LINE_LOOP);
	glVertex3i(c4_x1,c4_y1,0);
	glVertex3i(c4_x2,c4_y1,0);
	glVertex3i(c4_x2,c4_y2,0);
	glVertex3i(c4_x1,c4_y2,0);
	glEnd();

	glEnable(GL_TEXTURE_2D);
   
	// The X
	draw_string(0+o3dow_x_len-16,0+2,(unsigned char *)"X",1);

	draw_string(10,10,(unsigned char *)"Enable: ",1);
	draw_string(10,30,(unsigned char *)"X axis: ",1);
	draw_string(10,50,(unsigned char *)"Y axis: ",1);
	draw_string(10,70,(unsigned char *)"Z axis: ",1);

	return 1;
}


int check_o3dow_interface()
{
   int x,y;
   if(!view_o3dow || mouse_x>o3dow_x+o3dow_x_len || mouse_x<o3dow_x
      || mouse_y<o3dow_y || mouse_y>o3dow_y+o3dow_y_len)return 0;

   	if(view_o3dow && mouse_x>(o3dow_x+o3dow_x_len-20) && mouse_x<=(o3dow_x+o3dow_x_len)
	&& mouse_y>o3dow_y && mouse_y<=o3dow_y+20)
	{
		view_o3dow=0;
		return 1;
	}

   x=mouse_x-o3dow_x;
   y=mouse_y-o3dow_y;
   
	if(x>c1_x1 && x<=c1_x2 && y>c1_y1 && y<=c1_y2)c1=!c1;
	if(x>c2_x1 && x<=c2_x2 && y>c2_y1 && y<=c2_y2)c2=!c2;
	if(x>c3_x1 && x<=c3_x2 && y>c3_y1 && y<=c3_y2)c3=!c3;
	if(x>c4_x1 && x<=c4_x2 && y>c4_y1 && y<=c4_y2)c4=!c4;

   return 1;
}
