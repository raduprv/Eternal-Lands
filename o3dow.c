#include "global.h"

int view_o3dow=0;
int o3dow_x=100;
int o3dow_y=200;
int o3dow_x_len=250;
int o3dow_y_len=220;
int o3dow_dragged=0;
int o3dow_win=0;

//checkboxes
int c1=0,c1_x1=100,c1_x2=114,c1_y1=13,c1_y2=27;
int c2=0,c2_x1=100,c2_x2=114,c2_y1=33,c2_y2=47;
int c3=0,c3_x1=100,c3_x2=114,c3_y1=53,c3_y2=67;
int c4=0,c4_x1=100,c4_x2=114,c4_y1=73,c4_y2=87;
int c5_x1=170,c5_x2=184,c5_y1=133,c5_y2=147;
int c6_x1=170,c6_x2=184,c6_y1=193,c6_y2=207;

//buttons
int mina=0,maxa=360,minh=0,maxh=0,randomheight=0, randomangle=0;
int b1_x1=168,b1_x2=182,b1_y1=93,b1_y2=107;
int b2_x1=190,b2_x2=204,b2_y1=93,b2_y2=107;
int b3_x1=168,b3_x2=182,b3_y1=113,b3_y2=127;
int b4_x1=190,b4_x2=204,b4_y1=113,b4_y2=127;

int b5_x1=188,b5_x2=202,b5_y1=153,b5_y2=167;
int b6_x1=210,b6_x2=224,b6_y1=153,b6_y2=167;
int b7_x1=188,b7_x2=202,b7_y1=173,b7_y2=187;
int b8_x1=210,b8_x2=224,b8_y1=173,b8_y2=187;

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
	char temp[100];

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
	
	glBegin(randomangle ? GL_QUADS: GL_LINE_LOOP);
	glVertex3i(c6_x1,c6_y1,0);
	glVertex3i(c6_x2,c6_y1,0);
	glVertex3i(c6_x2,c6_y2,0);
	glVertex3i(c6_x1,c6_y2,0);
	glEnd();

	glBegin(randomheight ? GL_QUADS: GL_LINE_LOOP);
	glVertex3i(c5_x1,c5_y1,0);
	glVertex3i(c5_x2,c5_y1,0);
	glVertex3i(c5_x2,c5_y2,0);
	glVertex3i(c5_x1,c5_y2,0);
	glEnd();

	//Buttons
	glBegin(GL_LINE_LOOP);
	glVertex3i(b1_x1,b1_y1,0);
	glVertex3i(b1_x2,b1_y1,0);
	glVertex3i(b1_x2,b1_y2,0);
	glVertex3i(b1_x1,b1_y2,0);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3i(b2_x1,b2_y1,0);
	glVertex3i(b2_x2,b2_y1,0);
	glVertex3i(b2_x2,b2_y2,0);
	glVertex3i(b2_x1,b2_y2,0);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3i(b3_x1,b3_y1,0);
	glVertex3i(b3_x2,b3_y1,0);
	glVertex3i(b3_x2,b3_y2,0);
	glVertex3i(b3_x1,b3_y2,0);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3i(b4_x1,b4_y1,0);
	glVertex3i(b4_x2,b4_y1,0);
	glVertex3i(b4_x2,b4_y2,0);
	glVertex3i(b4_x1,b4_y2,0);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3i(b5_x1,b5_y1,0);
	glVertex3i(b5_x2,b5_y1,0);
	glVertex3i(b5_x2,b5_y2,0);
	glVertex3i(b5_x1,b5_y2,0);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3i(b6_x1,b6_y1,0);
	glVertex3i(b6_x2,b6_y1,0);
	glVertex3i(b6_x2,b6_y2,0);
	glVertex3i(b6_x1,b6_y2,0);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3i(b7_x1,b7_y1,0);
	glVertex3i(b7_x2,b7_y1,0);
	glVertex3i(b7_x2,b7_y2,0);
	glVertex3i(b7_x1,b7_y2,0);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3i(b8_x1,b8_y1,0);
	glVertex3i(b8_x2,b8_y1,0);
	glVertex3i(b8_x2,b8_y2,0);
	glVertex3i(b8_x1,b8_y2,0);
	glEnd();




	glEnable(GL_TEXTURE_2D);
   
	// The X
	draw_string(0+o3dow_x_len-16,0+2,(unsigned char *)"X",1);

	draw_string(10,10,(unsigned char *)"Enable: ",1);
	draw_string(10,30,(unsigned char *)"X axis: ",1);
	draw_string(10,50,(unsigned char *)"Y axis: ",1);
	draw_string(10,70,(unsigned char *)"Z axis: ",1);
	sprintf(temp,"Min height: %d",minh);
	draw_string(10,90,temp,1);
	draw_string(170,90,"+ -",1);
	sprintf(temp,"Max height: %d",maxh);
	draw_string(10,110,temp,1);
	draw_string(170,110,"+ -",1);
	draw_string(10,130,"Random height: ",1);
	
	sprintf(temp,"Min angle: %d",mina);
	draw_string(10,150,temp,1);
	draw_string(190,150,"+ -",1);
	sprintf(temp,"Max angle: %d",maxa);
	draw_string(10,170,temp,1);
	draw_string(190,170,"+ -",1);
	draw_string(10,190,"Random angle: ",1);

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
	if(x>c5_x1 && x<=c5_x2 && y>c5_y1 && y<=c5_y2)randomheight=!randomheight;
	if(x>c6_x1 && x<=c6_x2 && y>c6_y1 && y<=c6_y2)randomangle=!randomangle;

	if(x>b1_x1 && x<=b1_x2 && y>b1_y1 && y<=b1_y2)minh++;
	if(x>b2_x1 && x<=b2_x2 && y>b2_y1 && y<=b2_y2)minh--;
	if(x>b3_x1 && x<=b3_x2 && y>b3_y1 && y<=b3_y2)maxh++;
	if(x>b4_x1 && x<=b4_x2 && y>b4_y1 && y<=b4_y2)maxh--;

	if(x>b5_x1 && x<=b5_x2 && y>b5_y1 && y<=b5_y2)mina++;
	if(x>b6_x1 && x<=b6_x2 && y>b6_y1 && y<=b6_y2)mina--;
	if(x>b7_x1 && x<=b7_x2 && y>b7_y1 && y<=b7_y2)maxa++;
	if(x>b8_x1 && x<=b8_x2 && y>b8_y1 && y<=b8_y2)maxa--;


   return 1;
}
