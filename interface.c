#include "global.h"
#include <math.h>

int check_interface_buttons()
{
	if((left_click!=1 && right_click!=1) || mouse_x>=13*32 || mouse_y>=32)return -1;//no interface buttons were selected
	if(left_click==1)
		{
			if(mouse_x>=0 && mouse_x<=31)cur_mode=mode_tile;
			if(mouse_x>=32 && mouse_x<=63)cur_mode=mode_2d;
			if(mouse_x>=64 && mouse_x<=95)cur_mode=mode_3d;
			if(mouse_x>=96 && mouse_x<=127)cur_mode=mode_particles;
			if(mouse_x>=128 && mouse_x<=159)cur_mode=mode_light;
			if(mouse_x>=160 && mouse_x<=191)cur_mode=mode_height;
			if(mouse_x>=192 && mouse_x<=223)cur_mode=mode_map;
			if(mouse_x>=224 && mouse_x<=255)cur_tool=tool_select;
			if(mouse_x>=256 && mouse_x<=287)cur_tool=tool_clone;
			if(mouse_x>=288 && mouse_x<=319)
				{
					if(cur_mode==mode_3d)
						{
							SDL_Event event;
							open_3d_obj();
							while (SDL_PollEvent (&event));	//clears all the events
							left_click=2;
						}
					if(cur_mode==mode_2d)
						{
							SDL_Event event;
							open_2d_obj();
							while (SDL_PollEvent (&event));	//clears all the events
							left_click=2;
						}
					if(cur_mode==mode_particles)
						{
							SDL_Event event;
							open_particles_obj();
							while (SDL_PollEvent (&event));	//clears all the events
							left_click=2;
						}
					if(cur_mode==mode_tile)
						{
							view_tiles_list=1;
							cur_tool=tool_select;
							selected_tile=255;
						}
					if(cur_mode==mode_height)
						{
							view_heights_list=1;
							cur_tool=tool_select;
							selected_height=-1;
						}
					if(cur_mode==mode_light)
						{
							cur_tool=tool_select;
							selected_light=add_light(scene_mouse_x,scene_mouse_y,3.0f,1.0f,1.0f,1.0f,1.0f);
						}
				}
			if(mouse_x>=320 && mouse_x<=351)cur_tool=tool_kill;
			if(mouse_x>=352 && mouse_x<=383)
				{
					SDL_Event event;
					save_map_file();
					while (SDL_PollEvent (&event));	//clears all the events
					left_click=2;
				}
			if(mouse_x>=384 && mouse_x<=415)
				{
					SDL_Event event;
					open_map_file();
					while (SDL_PollEvent (&event));	//clears all the events
					left_click=2;
				}

			if(mouse_x>=416 && mouse_x<=447)
				{
					new_map_menu=1;
				}

		}
	else if(right_click==1)
			{
				if(mouse_x>=0 && mouse_x<=31)view_tile=!view_tile;
				if(mouse_x>=32 && mouse_x<=63)view_2d=!view_2d;
				if(mouse_x>=64 && mouse_x<=95)view_3d=!view_3d;
				if(mouse_x>=96 && mouse_x<=127)view_particles=!view_particles;
				if(mouse_x>=128 && mouse_x<=159)view_light=!view_light;
				if(mouse_x>=160 && mouse_x<=191)view_height=!view_height;
			}
	return 1;

}


void get_world_x_y()
{
	float window_ratio;
	float x,y,x1,y1,a,t;
	window_ratio=(GLfloat)window_width/(GLfloat)window_height;

	//x=(float)(mouse_x*3.0*zoom_level/window_width)-(9.0*zoom_level/6.0f);
	x=(float)((mouse_x)*2.8f*zoom_level/window_width)-(2.8*zoom_level/2.0f);
	y=(float)((window_height-mouse_y)*2.0f*zoom_level/window_height)-(2.0*zoom_level/2.0f);

	a=(rz)*3.1415926/180;
	t=(rx)*3.1415926/180;

	y=y/cos(t);

	x1=x*cos(a)+y*sin(a);
	y1=y*cos(a)-x*sin(a);

	scene_mouse_x=-cx+x1;
	scene_mouse_y=-cy+y1;
}

void Enter2DMode()
{
		glPushAttrib(GL_LIGHTING_BIT|GL_DEPTH_BUFFER_BIT);
		glDisable(GL_LIGHTING);
   		glDisable(GL_DEPTH_TEST);

		//glEnable(GL_BLEND);
		//glBlendFunc(GL_DST_ALPHA,GL_SRC_ALPHA);

    	glViewport(0, 0, window_width, window_height);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0.0, (GLdouble)window_width, (GLdouble)window_height, 0.0, 0.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
}

void Leave2DMode()
{
	//glDisable(GL_BLEND);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopAttrib();
}



void draw_2d_thing(float u_start,float v_start,float u_end,float v_end,int x_start,int y_start,int x_end,int y_end)
		{

			glTexCoord2f(u_start,v_end);
			glVertex3i(x_start,y_end,0);

			glTexCoord2f(u_start,v_start);
			glVertex3i(x_start,y_start,0);

			glTexCoord2f(u_end,v_start);
			glVertex3i(x_end,y_start,0);

			glTexCoord2f(u_end,v_end);
			glVertex3i(x_end,y_end,0);

		}

void draw_toolbar()
{

	get_and_set_texture_id(buttons_text);

	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_QUADS);

	if(cur_mode!=mode_tile && view_tile)
	glColor3f(1.0f,1.0f,1.0f);
	else if(!view_tile && cur_mode!=mode_tile)glColor3f(0.3f,0.3f,0.3f);
	else glColor3f(0.0f,1.0f,1.0f);
	draw_2d_thing((float)64/255,1.0f,(float)96/255,1.0f-(float)32/255, 0,0,32,32);

	if(cur_mode!=mode_2d && view_2d)
	glColor3f(1.0f,1.0f,1.0f);
	else if(!view_2d && cur_mode!=mode_2d)glColor3f(0.3f,0.3f,0.3f);
	else glColor3f(0.0f,1.0f,1.0f);
	draw_2d_thing((float)32/255,1.0f,(float)64/255,1.0f-(float)32/255, 32,0,64,32);

	if(cur_mode!=mode_3d && view_3d)
	glColor3f(1.0f,1.0f,1.0f);
	else if(!view_3d && cur_mode!=mode_3d)glColor3f(0.3f,0.3f,0.3f);
	else glColor3f(0.0f,1.0f,1.0f);
	draw_2d_thing(0,1.0f,(float)32/255,1.0f-(float)32/255, 64,0,96,32);

	if(cur_mode!=mode_particles && view_particles)
	glColor3f(1.0f,1.0f,1.0f);
	else if(!view_particles && cur_mode!=mode_particles)glColor3f(0.3f,0.3f,0.3f);
	else glColor3f(0.0f,1.0f,1.0f);
	draw_2d_thing((float)192/255,1.0f-(float)32/255,(float)224/255,1.0f-(float)64/255, 96,0,128,32);

	if(cur_mode!=mode_light && view_light)
	glColor3f(1.0f,1.0f,1.0f);
	else if(!view_light && cur_mode!=mode_light)glColor3f(0.3f,0.3f,0.3f);
	else glColor3f(0.0f,1.0f,1.0f);
	draw_2d_thing((float)96/255,1.0f,(float)128/255,1.0f-(float)32/255, 128,0,160,32);

	if(cur_mode!=mode_height && view_height)
	glColor3f(1.0f,1.0f,1.0f);
	else if(!view_height && cur_mode!=mode_height)glColor3f(0.3f,0.3f,0.3f);
	else glColor3f(0.0f,1.0f,1.0f);
	draw_2d_thing((float)160/255,1.0f-(float)32/255,(float)192/255,1.0f-(float)64/255, 160,0,192,32);


	if(cur_mode!=mode_map)
	glColor3f(1.0f,1.0f,1.0f);
	else glColor3f(0.0f,1.0f,1.0f);
	draw_2d_thing((float)128/255,1.0f,(float)160/255,1.0f-(float)32/255, 192,0,224,32);


	if(cur_tool!=tool_select)
	glColor3f(1.0f,1.0f,1.0f);
	else glColor3f(0.0f,1.0f,1.0f);
	draw_2d_thing(0,1.0f-(float)32/255,(float)32/255,1.0f-(float)64/255, 224,0,256,32);

	if(cur_tool!=tool_clone)
	glColor3f(1.0f,1.0f,1.0f);
	else glColor3f(0.0f,1.0f,1.0f);
	draw_2d_thing((float)32/255,1.0f-(float)32/255,(float)64/255,1.0f-(float)64/255, 256,0,288,32);

	if(cur_tool!=tool_new)
	glColor3f(1.0f,1.0f,1.0f);
	else glColor3f(0.0f,1.0f,1.0f);
	draw_2d_thing((float)224/255,1.0f,(float)256/255,1.0f-(float)32/255, 288,0,320,32);

	if(cur_tool!=tool_kill)
	glColor3f(1.0f,1.0f,1.0f);
	else glColor3f(0.0f,1.0f,1.0f);
	draw_2d_thing((float)192/255,1.0f,(float)224/255,1.0f-(float)32/255, 320,0,352,32);

	glColor3f(1.0f,1.0f,1.0f);
	//save
	draw_2d_thing((float)64/255,1.0f-(float)32/255,(float)96/255,1.0f-(float)64/255, 352,0,384,32);

	//open
	draw_2d_thing((float)96/255,1.0f-(float)32/255,(float)128/255,1.0f-(float)64/255, 384,0,416,32);

	//new
	draw_2d_thing((float)128/255,1.0f-(float)32/255,(float)160/255,1.0f-(float)64/255, 416,0,448,32);

	glEnd();
}

void draw_3d_obj_info()
{
	unsigned char str[128];
	int x_menu,y_menu;
	if(cur_mode!=mode_3d || selected_3d_object==-1)return;

	x_menu=0;
	y_menu=window_height-72;
	//draw a black rectangle
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,0.5f);
	glVertex3i(x_menu,y_menu+70,0);
	glVertex3i(x_menu,y_menu,0);
	glVertex3i(x_menu+600,y_menu,0);
	glVertex3i(x_menu+600,y_menu+70,0);
	glColor3f(1.0f,1.0f,1.0f);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);


	x_menu+=2;
	y_menu+=2;

	sprintf((char *)str, "X Pos: %03.2f",objects_list[selected_3d_object]->x_pos);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Y Pos: %03.2f",objects_list[selected_3d_object]->y_pos);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Z Pos: %03.2f",objects_list[selected_3d_object]->z_pos);
	draw_string(x_menu,y_menu,str,1);
/////////////////////////////////////////////////
	x_menu+=12*12;
	y_menu-=17*2;

	sprintf((char *)str, "X Rot: %03.2f",objects_list[selected_3d_object]->x_rot);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Z Rot: %03.2f",objects_list[selected_3d_object]->z_rot);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Y Rot: %03.2f",objects_list[selected_3d_object]->y_rot);
	draw_string(x_menu,y_menu,str,1);
/////////////////////////////////////////////////
	x_menu+=12*12;
	y_menu-=17*2;

	sprintf((char *)str, "Red  : %03.2f",objects_list[selected_3d_object]->r);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Green: %03.2f",objects_list[selected_3d_object]->g);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Blue : %03.2f",objects_list[selected_3d_object]->b);
	draw_string(x_menu,y_menu,str,1);
///////////////////////////////////////////////////////////////////////
	x_menu+=12*12;
	y_menu-=17*2;

	sprintf((char *)str, "[B]lended : %s",objects_list[selected_3d_object]->blended ? "Yes" : "No");
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Self [L]it: %s",objects_list[selected_3d_object]->self_lit ? "Yes" : "No");
	draw_string(x_menu,y_menu,str,1);


	y_menu+=17;
	sprintf((char *)str, "Object ID: %i",selected_3d_object);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	x_menu-=12*12*3;
	sprintf((char *)str, "FN: %s",objects_list[selected_3d_object]->file_name);
	draw_string(x_menu,y_menu,str,1);

}

void draw_2d_obj_info()
{
	unsigned char str[128];
	int x_menu,y_menu;
	if(cur_mode!=mode_2d || selected_2d_object==-1)return;

	x_menu=0;
	y_menu=window_height-72;
	//draw a black rectangle
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,0.5f);
	glVertex3i(x_menu,y_menu+70,0);
	glVertex3i(x_menu,y_menu,0);
	glVertex3i(x_menu+600,y_menu,0);
	glVertex3i(x_menu+600,y_menu+70,0);
	glColor3f(1.0f,1.0f,1.0f);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);


	x_menu+=2;
	y_menu+=2;

	sprintf((char *)str, "X Pos: %03.2f",obj_2d_list[selected_2d_object]->x_pos);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Y Pos: %03.2f",obj_2d_list[selected_2d_object]->y_pos);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Z Pos: %03.2f",obj_2d_list[selected_2d_object]->z_pos);
	draw_string(x_menu,y_menu,str,1);
/////////////////////////////////////////////////
	x_menu+=12*12;
	y_menu-=17*2;

	sprintf((char *)str, "X Rot: %03.2f",obj_2d_list[selected_2d_object]->x_rot);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Z Rot: %03.2f",obj_2d_list[selected_2d_object]->z_rot);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Object ID: %i",selected_2d_object);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	x_menu-=12*12;
	sprintf((char *)str, "FN: %s",obj_2d_list[selected_2d_object]->file_name);
	draw_string(x_menu,y_menu,str,1);

}

void draw_light_info()
{
	unsigned char str[128];
	int x_menu,y_menu;
	if(cur_mode!=mode_light || selected_light==-1)return;

	x_menu=0;
	y_menu=window_height-72;
	//draw a black rectangle
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,0.5f);
	glVertex3i(x_menu,y_menu+70,0);
	glVertex3i(x_menu,y_menu,0);
	glVertex3i(x_menu+400,y_menu,0);
	glVertex3i(x_menu+400,y_menu+70,0);
	glColor3f(1.0f,1.0f,1.0f);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);


	x_menu+=2;
	y_menu+=2;

	sprintf((char *)str, "X Pos: %03.2f",lights_list[selected_light]->pos_x);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Y Pos: %03.2f",lights_list[selected_light]->pos_y);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Z Pos: %03.2f",lights_list[selected_light]->pos_z);
	draw_string(x_menu,y_menu,str,1);
/////////////////////////////////////////////////
	x_menu+=12*12;
	y_menu-=17*2;

	sprintf((char *)str, "R: %03.2f",lights_list[selected_light]->r);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "G: %03.2f",lights_list[selected_light]->g);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "B: %03.2f",lights_list[selected_light]->b);
	draw_string(x_menu,y_menu,str,1);

	x_menu+=12*8;
	y_menu-=17*2;

	sprintf((char *)str, "Light ID: %i",selected_light);
	draw_string(x_menu,y_menu,str,1);
}



void draw_height_info()
{
	unsigned char str[128];
	int x_menu,y_menu;
	if(cur_mode!=mode_height || selected_height==-1)return;

	x_menu=0;
	y_menu=window_height-62;
	//draw a black rectangle
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,0.5f);
	glVertex3i(x_menu,y_menu+60,0);
	glVertex3i(x_menu,y_menu,0);
	glVertex3i(x_menu+200,y_menu,0);
	glVertex3i(x_menu+200,y_menu+60,0);
	glColor3f(1.0f,1.0f,1.0f);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);


	x_menu+=2;
	y_menu+=2;

	sprintf((char *)str, "X Pos: %i",(int)(scene_mouse_x*2.0f));
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Y Pos: %i",(int)(scene_mouse_y*2.0f));
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Z Pos: %03.2f",(float)selected_height*0.2f-11.0f*0.2f);
	draw_string(x_menu,y_menu,str,1);
}

void display_tiles_list()
{

	int i,j;
	int x_start,y_start,x_end,y_end;

	//draw a black rectangle
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor3f(0.0f,0.0f,0.0f);
	glVertex3i((int)(x_tile_menu_offset),(int)(y_tile_menu_offset)+64*4,0);
	glVertex3i((int)(x_tile_menu_offset),(int)(y_tile_menu_offset),0);
	glVertex3i((int)(x_tile_menu_offset)+64*8,(int)(y_tile_menu_offset),0);
	glVertex3i((int)(x_tile_menu_offset)+64*8,(int)(y_tile_menu_offset)+64*4,0);
	glColor3f(1.0f,1.0f,1.0f);
	glEnd();
	glEnable(GL_TEXTURE_2D);

	for(i=tile_offset,j=0;i<tiles_no;i++,j++)
		{

			x_start=64*(j%8);
			x_start+=(int)x_tile_menu_offset;
			x_end=x_start+63;

			y_start=64*(j/8);
			y_start+=(int)y_tile_menu_offset;
			y_end=y_start+63;

			if(!i)//we have a lake tile
			{
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_LIGHTING);
				glColor3f(0,0.6f,1.0f);
				glBegin(GL_QUADS);
				glVertex3i(x_start,y_end,0);
				glVertex3i(x_start,y_start,0);
				glVertex3i(x_end,y_start,0);
				glVertex3i(x_end,y_end,0);
				glEnd();
				glEnable(GL_LIGHTING);
				glEnable(GL_TEXTURE_2D);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, get_texture_id(tile_list[i]));

				glBegin(GL_QUADS);
				glTexCoord2f(0,1.0f);
				glVertex3i(x_start,y_end,0);

				glTexCoord2f(0,0);
				glVertex3i(x_start,y_start,0);

				glTexCoord2f(1.0f,0);
				glVertex3i(x_end,y_start,0);

				glTexCoord2f(1.0f,1.0f);
				glVertex3i(x_end,y_end,0);

				glEnd();
			}
		}
}


void display_heights_list()
{

	int i;
	int x_start,y_start,x_end,y_end;

	//draw a black rectangle
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor3f(0.0f,0.0f,0.0f);
	glVertex3i((int)(x_tile_menu_offset),(int)(y_tile_menu_offset)+32*4,0);
	glVertex3i((int)(x_tile_menu_offset),(int)(y_tile_menu_offset),0);
	glVertex3i((int)(x_tile_menu_offset)+32*8,(int)(y_tile_menu_offset),0);
	glVertex3i((int)(x_tile_menu_offset)+32*8,(int)(y_tile_menu_offset)+32*4,0);
	glColor3f(1.0f,1.0f,1.0f);
	glEnd();

	for(i=0;i<32;i++)
		{

			x_start=32*(i%8);
			x_start+=(int)x_tile_menu_offset;
			x_end=x_start+31;

			y_start=32*(i/8);
			y_start+=(int)y_tile_menu_offset;
			y_end=y_start+31;

			change_color_height(i);
			glBegin(GL_QUADS);

			glVertex3i(x_start,y_end,0);
			glVertex3i(x_start,y_start,0);
			glVertex3i(x_end,y_start,0);
			glVertex3i(x_end,y_end,0);

			glEnd();
		}
	glEnable(GL_TEXTURE_2D);
}

void check_mouse_minimap()
{
	int minimap_x_start;
	int minimap_y_start;
	int x_map_pos;
	int y_map_pos;

	minimap_x_start=(window_width-tile_map_size_x*2)/2;
	minimap_y_start=(window_height-tile_map_size_y*2)/2;

	if(mouse_x<minimap_x_start || mouse_y<minimap_y_start
	|| mouse_x>minimap_x_start+tile_map_size_x*2 || mouse_y>minimap_y_start+tile_map_size_y*2)return;

	x_map_pos=((mouse_x-minimap_x_start)/2)*3;
	y_map_pos=(tile_map_size_y-((mouse_y-minimap_y_start)/2))*3;
	cx=-x_map_pos;
	cy=-y_map_pos;
	minimap_on=0;

}

void draw_mouse_minimap()
{
	int minimap_x_start;
	int minimap_y_start;
	int x_map_pos;
	int y_map_pos;
	int x,y;


	minimap_x_start=(window_width-tile_map_size_x*2)/2;
	minimap_y_start=(window_height-tile_map_size_y*2)/2;

	if(mouse_x<minimap_x_start || mouse_y<minimap_y_start
	|| mouse_x>minimap_x_start+tile_map_size_x*2 || mouse_y>minimap_y_start+tile_map_size_y*2)return;

	x_map_pos=((mouse_x-minimap_x_start)/2);
	y_map_pos=(tile_map_size_y-((mouse_y-minimap_y_start)/2));
	cx=-x_map_pos;
	cy=-y_map_pos;

	for(x=-2;x!=2;x++){
	  for(y=-2;y!=2;y++){
	    if(y_map_pos+y>=0 && y_map_pos+y<tile_map_size_y && x_map_pos+x>=0 && x_map_pos+x<tile_map_size_x){
	      tile_map[(int)(y_map_pos+y)*tile_map_size_x+(int)x_map_pos+x]=((cur_tool==tool_kill)?255:selected_tile);
	      if(cur_tool==tool_kill || selected_tile == 0 || selected_tile == 20 || selected_tile == 21){
		kill_height_map_at_texture_tile((int)(y_map_pos+y)*tile_map_size_x+(int)x_map_pos+x);
	      }
	    }
	  }
	}	

}

void draw_minimap()
{
	int minimap_x_start;
	int minimap_y_start;

	//now, draw all the tiles, on the screen...
	int x,y;
	int sx;
	int sy=tile_map_size_y;
	int x_scaled,y_scaled;
	int cur_texture;
	int i;

	minimap_x_start=(window_width-tile_map_size_x*2)/2;
	minimap_y_start=(window_height-tile_map_size_y*2)/2;


	//we have the axes inverted, btw they go from 0 to -255
	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_QUADS);
	for(y=0;y<tile_map_size_y;y++)
		{
			sy--;
			y_scaled=(int)(sy*2.0);
			sx=0;
			for(x=0;x<tile_map_size_x;x++)
				{
					sx++;
					x_scaled=(int)(sx*2.0);
					if(!tile_map[y*tile_map_size_x+x])//water
						{
							glColor3f(0.0f,0.5f,1.0f);
							glEnd();

							glDisable(GL_TEXTURE_2D);
							glBegin(GL_QUADS);

	 						glVertex3i(minimap_x_start+x_scaled,minimap_y_start+y_scaled+2, 0);
							glVertex3i(minimap_x_start+x_scaled,minimap_y_start+y_scaled, 0);
							glVertex3i(minimap_x_start+x_scaled+2, minimap_y_start+y_scaled,0);
							glVertex3i(minimap_x_start+x_scaled+2, minimap_y_start+y_scaled+2,0);

							glEnd();
							glEnable(GL_TEXTURE_2D);

							glBegin(GL_QUADS);
							glColor3f(1.0f,1.0f,1.0f);
							continue;
						}
					if(tile_map[y*tile_map_size_x+x]==255)continue;//null, skip
					cur_texture=get_texture_id(tile_list[tile_map[y*tile_map_size_x+x]]);
					if(last_texture!=cur_texture)
						{
							glEnd();
							glBindTexture(GL_TEXTURE_2D, cur_texture);
							glBegin(GL_QUADS);
							last_texture=cur_texture;
						}

 					glTexCoord2f(0, 1.0f);
	 				glVertex3i(minimap_x_start+x_scaled,minimap_y_start+y_scaled+2, 0);
					glTexCoord2f(0, 0);
					glVertex3i(minimap_x_start+x_scaled,minimap_y_start+y_scaled, 0);
					glTexCoord2f(1.0f, 0);
					glVertex3i(minimap_x_start+x_scaled+2, minimap_y_start+y_scaled,0);
					glTexCoord2f(1.0f, 1.0f);
					glVertex3i(minimap_x_start+x_scaled+2, minimap_y_start+y_scaled+2,0);
				}
		}
	glEnd();

	//draw the objects as red points
	glBegin(GL_POINTS);
	glColor3f(1.0f,0.0f,0.0f);
	for(i=0;i<max_obj_3d;i++)
		{
			if(objects_list[i])
			     {
			         int x;
			         int y;

			         x=(int)objects_list[i]->x_pos/3;
			         y=(int)objects_list[i]->y_pos/3;
			         glVertex3i(minimap_x_start+x*2, minimap_y_start+(tile_map_size_y-y)*2,0);
                 }
		}
	glEnd();


}

int map_size=0;

void display_new_map_menu()
{
	char str[128];
	int x_menu,y_menu;
	if(!new_map_menu)return;

	x_menu=90;
	y_menu=82;
	//draw a black rectangle
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,0.5f);
	glVertex3i(x_menu,y_menu+170,0);
	glVertex3i(x_menu,y_menu,0);
	glVertex3i(x_menu+240,y_menu,0);
	glVertex3i(x_menu+240,y_menu+170,0);
	glColor3f(1.0f,1.0f,1.0f);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);


	x_menu+=2;
	y_menu+=2;

	glColor3f(1.0f,1.0f,0.0f);
	draw_string(x_menu,y_menu,(unsigned char *)"    Map Size",1);

	if(mouse_x>x_menu && mouse_x<x_menu+240 && mouse_y>y_menu+17 && mouse_y<y_menu+17+17 && left_click==1)
	map_size=0;

	if(map_size==0)glColor3f(0.0f,0.5f,1.0f);
	else glColor3f(1.0f,1.0f,1.0f);
	y_menu+=17;
	draw_string(x_menu,y_menu,(unsigned char *)"Very Small [16x16]",1);

	if(mouse_x>x_menu && mouse_x<x_menu+240 && mouse_y>y_menu+17 && mouse_y<y_menu+17+17 && left_click==1)
	map_size=1;

	if(map_size==1)glColor3f(0.0f,0.5f,1.0f);
	else glColor3f(1.0f,1.0f,1.0f);
	y_menu+=17;
	draw_string(x_menu,y_menu,(unsigned char *)"Small      [32x32]",1);

	if(mouse_x>x_menu && mouse_x<x_menu+240 && mouse_y>y_menu+17 && mouse_y<y_menu+17+17 && left_click==1)
	map_size=2;

	if(map_size==2)glColor3f(0.0f,0.5f,1.0f);
	else glColor3f(1.0f,1.0f,1.0f);
	y_menu+=17;
	draw_string(x_menu,y_menu,(unsigned char *)"Medium     [64x64]",1);

	if(mouse_x>x_menu && mouse_x<x_menu+240 && mouse_y>y_menu+17 && mouse_y<y_menu+17+17 && left_click==1)
	map_size=3;


	if(map_size==3)glColor3f(0.0f,0.5f,1.0f);
	else glColor3f(1.0f,1.0f,1.0f);
	y_menu+=17;
	draw_string(x_menu,y_menu,(unsigned char *)"Large      [128x128]",1);

	if(mouse_x>x_menu && mouse_x<x_menu+240 && mouse_y>y_menu+17 && mouse_y<y_menu+17+17 && left_click==1)
	map_size=4;

	if(map_size==4)glColor3f(0.0f,0.5f,1.0f);
	else glColor3f(1.0f,1.0f,1.0f);
	y_menu+=17;
	draw_string(x_menu,y_menu,(unsigned char *)"Huge       [256x256]",1);

	y_menu+=17;
	//test for OK
	if(mouse_x>x_menu+3*12 && mouse_x<x_menu+7*12 && mouse_y>y_menu+17 && mouse_y<y_menu+17+17 && left_click==1)
	{
		if(map_size==0)new_map(16,16);
		else
		if(map_size==1)new_map(32,32);
		else
		if(map_size==2)new_map(64,64);
		else
		if(map_size==3)new_map(128,128);
		else
		if(map_size==4)new_map(256,256);

		new_map_menu=0;
	}
	//test for Cancel
	if(mouse_x>x_menu+11*12 && mouse_x<x_menu+19*12 &&
	mouse_y>y_menu+17 && mouse_y<y_menu+17+17 && left_click==1)new_map_menu=0;

	y_menu+=17;
	glColor3f(1.0f,1.0f,1.0f);
	draw_string(x_menu,y_menu,(unsigned char *)"   [Ok]    [Cancel]",1);

}

void display_map_settings()
{
	unsigned char str[128];
	int x_menu,y_menu;
	if(cur_mode!=mode_map)return;

	x_menu=0;
	y_menu=window_height-62;
	//draw a black rectangle
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,0.5f);
	glVertex3i(x_menu,y_menu+170,0);
	glVertex3i(x_menu,y_menu,0);
	glVertex3i(x_menu+250,y_menu,0);
	glVertex3i(x_menu+250,y_menu+170,0);
	glColor3f(1.0f,1.0f,1.0f);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);


	x_menu+=2;
	y_menu+=2;

	sprintf((char *)str, "R: %03.2f",ambient_r);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "G: %03.2f",ambient_g);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "B: %03.2f",ambient_b);
	draw_string(x_menu,y_menu,str,1);

	y_menu-=17*2;
	x_menu+=90;

	sprintf((char *)str, "[D]ungeon: %s",dungeon ? "Yes" : "No");
	draw_string(x_menu,y_menu,str,1);

}
