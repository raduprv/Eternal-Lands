#include "global.h"

int view_o3dow=0;
int o3dow_x=20;
int o3dow_y=100;
int o3dow_x_len=500;
int o3dow_y_len=330;
int o3dow_dragged=0;
int o3dow_win=0;

//checkboxes
int c1=0,c1_x1=185,c1_x2=199,c1_y1=33,c1_y2=47;
int c2=0,c2_x1=330,c2_x2=344,c2_y1=13,c2_y2=27;
int c3=0,c3_x1=330,c3_x2=344,c3_y1=33,c3_y2=47;
int c4=0,c4_x1=330,c4_x2=344,c4_y1=53,c4_y2=67;
int c5_x1=170,c5_x2=184,c5_y1=103,c5_y2=117;
int c6_x1=185,c6_x2=199,c6_y1=173,c6_y2=187;
int c7_x1=185,c7_x2=199,c7_y1=233,c7_y2=247;
int c8_x1=185,c8_x2=199,c8_y1=283,c8_y2=297;

//buttons
int minay=0,minaz=0,maxay=360,maxaz=360,minax=0,maxax=360,minh=0,maxh=0,randomheight=0, randomanglex=0, randomangley=0, randomanglez=0;
int b1_x1=438,b1_x2=452,b1_y1=93,b1_y2=107;
int b2_x1=460,b2_x2=474,b2_y1=93,b2_y2=107;
int b3_x1=438,b3_x2=452,b3_y1=113,b3_y2=127;
int b4_x1=460,b4_x2=474,b4_y1=113,b4_y2=127;

int b5_x1=438,b5_x2=452,b5_y1=163,b5_y2=177;
int b6_x1=460,b6_x2=474,b6_y1=163,b6_y2=177;
int b7_x1=438,b7_x2=452,b7_y1=183,b7_y2=197;
int b8_x1=460,b8_x2=474,b8_y1=183,b8_y2=197;

int b9_x1=438,b9_x2=452,b9_y1=223,b9_y2=237;
int b10_x1=460,b10_x2=474,b10_y1=223,b10_y2=237;
int b11_x1=438,b11_x2=452,b11_y1=243,b11_y2=257;
int b12_x1=460,b12_x2=474,b12_y1=243,b12_y2=257;

int b13_x1=438,b13_x2=452,b13_y1=273,b13_y2=287;
int b14_x1=460,b14_x2=474,b14_y1=273,b14_y2=287;
int b15_x1=438,b15_x2=452,b15_y1=293,b15_y2=307;
int b16_x1=460,b16_x2=474,b16_y1=293,b16_y2=307;

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

int display_o3dow_handler(window_info *win)
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
	
	glBegin(randomanglex ? GL_QUADS: GL_LINE_LOOP);
	glVertex3i(c6_x1,c6_y1,0);
	glVertex3i(c6_x2,c6_y1,0);
	glVertex3i(c6_x2,c6_y2,0);
	glVertex3i(c6_x1,c6_y2,0);
	glEnd();

	glBegin(randomangley ? GL_QUADS: GL_LINE_LOOP);
	glVertex3i(c7_x1,c7_y1,0);
	glVertex3i(c7_x2,c7_y1,0);
	glVertex3i(c7_x2,c7_y2,0);
	glVertex3i(c7_x1,c7_y2,0);
	glEnd();

	glBegin(randomanglez ? GL_QUADS: GL_LINE_LOOP);
	glVertex3i(c8_x1,c8_y1,0);
	glVertex3i(c8_x2,c8_y1,0);
	glVertex3i(c8_x2,c8_y2,0);
	glVertex3i(c8_x1,c8_y2,0);
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
	glBegin(GL_LINE_LOOP);
	glVertex3i(b9_x1,b9_y1,0);
	glVertex3i(b9_x2,b9_y1,0);
	glVertex3i(b9_x2,b9_y2,0);
	glVertex3i(b9_x1,b9_y2,0);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3i(b10_x1,b10_y1,0);
	glVertex3i(b10_x2,b10_y1,0);
	glVertex3i(b10_x2,b10_y2,0);
	glVertex3i(b10_x1,b10_y2,0);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3i(b11_x1,b11_y1,0);
	glVertex3i(b11_x2,b11_y1,0);
	glVertex3i(b11_x2,b11_y2,0);
	glVertex3i(b11_x1,b11_y2,0);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3i(b12_x1,b12_y1,0);
	glVertex3i(b12_x2,b12_y1,0);
	glVertex3i(b12_x2,b12_y2,0);
	glVertex3i(b12_x1,b12_y2,0);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3i(b13_x1,b13_y1,0);
	glVertex3i(b13_x2,b13_y1,0);
	glVertex3i(b13_x2,b13_y2,0);
	glVertex3i(b13_x1,b13_y2,0);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3i(b14_x1,b14_y1,0);
	glVertex3i(b14_x2,b14_y1,0);
	glVertex3i(b14_x2,b14_y2,0);
	glVertex3i(b14_x1,b14_y2,0);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3i(b15_x1,b15_y1,0);
	glVertex3i(b15_x2,b15_y1,0);
	glVertex3i(b15_x2,b15_y2,0);
	glVertex3i(b15_x1,b15_y2,0);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3i(b16_x1,b16_y1,0);
	glVertex3i(b16_x2,b16_y1,0);
	glVertex3i(b16_x2,b16_y2,0);
	glVertex3i(b16_x1,b16_y2,0);
	glEnd();


	glEnable(GL_TEXTURE_2D);
   
	// The X
	draw_string(0+win->len_x-16,0+2,(unsigned char *)"X",1);

	draw_string(10,30,"Random Rotation: ",1);
	draw_string(250,10,"X axis: ",1);
	draw_string(250,30,"Y axis: ",1);
	draw_string(250,50,"Z axis: ",1);

	draw_string(10,100,"Random height: ",1);
	sprintf(temp,"Min height: %d",minh);
	draw_string(250,90,temp,1);
	draw_string(440,90,"+ -",1);
	sprintf(temp,"Max height: %d",maxh);
	draw_string(250,110,temp,1);
	draw_string(440,110,"+ -",1);
	
	draw_string(10,170,"Random angle X: ",1);
	sprintf(temp,"Min angle X: %d",minax);
	draw_string(250,160,temp,1);
	draw_string(440,160,"+ -",1);
	sprintf(temp,"Max angle X: %d",maxax);
	draw_string(250,180,temp,1);
	draw_string(440,180,"+ -",1);

	draw_string(10,230,"Random angle Y: ",1);
	sprintf(temp,"Min angle Y: %d",minay);
	draw_string(250,220,temp,1);
	draw_string(440,220,"+ -",1);
	sprintf(temp,"Max angle Y: %d",maxay);
	draw_string(250,240,temp,1);
	draw_string(440,240,"+ -",1);
	
	draw_string(10,280,"Random angle Z: ",1);
	sprintf(temp,"Min angle Z: %d",minaz);
	draw_string(250,270,temp,1);
	draw_string(440,270,"+ -",1);
	sprintf(temp,"Max angle Z: %d",maxaz);
	draw_string(250,290,temp,1);
	draw_string(440,290,"+ -",1);
	return 1;
}


int check_o3dow_interface(window_info *win, int mx, int my, Uint32 flags)
{
	int x,y;
	if(mouse_x>win->pos_x+win->len_x || mouse_x<win->pos_x
      || mouse_y<win->pos_y || mouse_y>win->pos_y+win->len_y)return 0;

   	if(view_o3dow && mouse_x>(win->pos_x+win->len_x-20) && mouse_x<=(win->pos_x+win->len_x)
	&& mouse_y>win->pos_y && mouse_y<=win->pos_y+20)
	{
		view_o3dow=0;
		return 1;
	}

   x=mouse_x-win->pos_x;
   y=mouse_y-win->pos_y;
   
	if(x>c1_x1 && x<=c1_x2 && y>c1_y1 && y<=c1_y2)c1=!c1;
	if(x>c2_x1 && x<=c2_x2 && y>c2_y1 && y<=c2_y2)c2=!c2;
	if(x>c3_x1 && x<=c3_x2 && y>c3_y1 && y<=c3_y2)c3=!c3;
	if(x>c4_x1 && x<=c4_x2 && y>c4_y1 && y<=c4_y2)c4=!c4;
	if(x>c5_x1 && x<=c5_x2 && y>c5_y1 && y<=c5_y2)randomheight=!randomheight;
	if(x>c6_x1 && x<=c6_x2 && y>c6_y1 && y<=c6_y2)randomanglex=!randomanglex;
	if(x>c7_x1 && x<=c7_x2 && y>c7_y1 && y<=c7_y2)randomangley=!randomangley;
	if(x>c8_x1 && x<=c8_x2 && y>c8_y1 && y<=c8_y2)randomanglez=!randomanglez;


	if(x>b1_x1 && x<=b1_x2 && y>b1_y1 && y<=b1_y2)minh+=ctrl_on?100:alt_on?10:1;
	if(x>b2_x1 && x<=b2_x2 && y>b2_y1 && y<=b2_y2)minh-=ctrl_on?100:alt_on?10:1;
	if(x>b3_x1 && x<=b3_x2 && y>b3_y1 && y<=b3_y2)maxh+=ctrl_on?100:alt_on?10:1;
	if(x>b4_x1 && x<=b4_x2 && y>b4_y1 && y<=b4_y2)maxh-=ctrl_on?100:alt_on?10:1;

	if(x>b5_x1 && x<=b5_x2 && y>b5_y1 && y<=b5_y2)minax+=ctrl_on?100:alt_on?10:1;
	if(x>b6_x1 && x<=b6_x2 && y>b6_y1 && y<=b6_y2)minax-=ctrl_on?100:alt_on?10:1;
	if(x>b7_x1 && x<=b7_x2 && y>b7_y1 && y<=b7_y2)maxax+=ctrl_on?100:alt_on?10:1;
	if(x>b8_x1 && x<=b8_x2 && y>b8_y1 && y<=b8_y2)maxax-=ctrl_on?100:alt_on?10:1;

	if(x>b9_x1 && x<=b9_x2 && y>b9_y1 && y<=b9_y2)minay+=ctrl_on?100:alt_on?10:1;

	if(x>b10_x1 && x<=b10_x2 && y>b10_y1 && y<=b10_y2)minay-=ctrl_on?100:alt_on?10:1;
	if(x>b11_x1 && x<=b11_x2 && y>b11_y1 && y<=b11_y2)maxay+=ctrl_on?100:alt_on?10:1;
	if(x>b12_x1 && x<=b12_x2 && y>b12_y1 && y<=b12_y2)maxay-=ctrl_on?100:alt_on?10:1;

	if(x>b13_x1 && x<=b13_x2 && y>b13_y1 && y<=b13_y2)minaz+=ctrl_on?100:alt_on?10:1;
	if(x>b14_x1 && x<=b14_x2 && y>b14_y1 && y<=b14_y2)minaz-=ctrl_on?100:alt_on?10:1;
	if(x>b15_x1 && x<=b15_x2 && y>b15_y1 && y<=b15_y2)maxaz+=ctrl_on?100:alt_on?10:1;
	if(x>b16_x1 && x<=b16_x2 && y>b16_y1 && y<=b16_y2)maxaz-=ctrl_on?100:alt_on?10:1;


   return 1;
}
