#include "tiles.h"
#include "global.h"
#include "eye_candy_window.h"
#include <math.h>

int show_position_on_minimap=0;

int check_interface_buttons()
{
#ifdef EYE_CANDY
	if ((minimap_on) && (cur_mode==mode_eye_candy) && (eye_candy_confirmed))
	{
		if (left_click)
		  add_eye_candy_point();
		else
		  last_ec_index = -2;
	}
#endif
	if((left_click!=1 && right_click!=1) || mouse_x>=15*32 || mouse_y>=32)return -1;//no interface buttons were selected
	if(left_click==1)
		{
#ifdef EYE_CANDY
			eye_candy_done_adding_effect();
#endif
			if(mouse_x>=0 && mouse_x<=31)cur_mode=mode_tile;
			if(mouse_x>=32 && mouse_x<=63)cur_mode=mode_2d;
			if(mouse_x>=64 && mouse_x<=95)cur_mode=mode_3d;
			if(mouse_x>=96 && mouse_x<=127)cur_mode=mode_particles;
#ifdef	EYE_CANDY
			if(mouse_x>=128 && mouse_x<=160)cur_mode=mode_eye_candy;
#endif	//EYE_CANDY
			if(mouse_x>=160 && mouse_x<=191)cur_mode=mode_light;
			if(mouse_x>=192 && mouse_x<=223)cur_mode=mode_height;
			if(mouse_x>=224 && mouse_x<=255)cur_mode=mode_map;
			if(mouse_x>=256 && mouse_x<=287)cur_tool=tool_select;
			if(mouse_x>=288 && mouse_x<=319)cur_tool=tool_clone;
			if(mouse_x>=320 && mouse_x<=351)
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
#ifdef	EYE_CANDY
					if(cur_mode==mode_eye_candy)
						{
							SDL_Event event;
//#ifdef LINUX
							open_eye_candy_obj();
//#endif
							while (SDL_PollEvent (&event));	//clears all the events
							left_click=2;
						}
#endif	//EYE_CANDY
					if(cur_mode==mode_tile)
						{
							view_tiles_list=1;
							display_tiles_list();
							cur_tool=tool_select;
							selected_tile=255;
						}
					if(cur_mode==mode_height)
						{
							display_heights_list();
							cur_tool=tool_select;
							selected_height=-1;
						}
					if(cur_mode==mode_light)
						{
							cur_tool=tool_select;
							selected_light=add_light(scene_mouse_x,scene_mouse_y,3.0f,1.0f,1.0f,1.0f,1.0f,0);
						}
				}
			if(mouse_x>=352 && mouse_x<=383)cur_tool=tool_kill;
			if(mouse_x>=384 && mouse_x<=415)
				{
					SDL_Event event;
					save_map_file();
					while (SDL_PollEvent (&event));	//clears all the events
					left_click=2;
				}
			if(mouse_x>=416 && mouse_x<=447)
				{
					SDL_Event event;
					open_map_file();
					while (SDL_PollEvent (&event));	//clears all the events
					left_click=2;
				}

			if(mouse_x>=448 && mouse_x<=479)
				{
					display_new_map_menu ();
				}

		}
	else if(right_click==1)
			{
				if(mouse_x>=0 && mouse_x<=31)view_tile=!view_tile;
				if(mouse_x>=32 && mouse_x<=63)view_2d=!view_2d;
				if(mouse_x>=64 && mouse_x<=95)view_3d=!view_3d;
				if(mouse_x>=96 && mouse_x<=127) {
				  if(shift_on)view_particle_handles=!view_particle_handles;
				  else view_particles=!view_particles;
				}
				if(mouse_x>=160 && mouse_x<=191)view_light=!view_light;
				if(mouse_x>=192 && mouse_x<=223)view_height=!view_height;
			}
	return 1;

}


void get_world_x_y()
{
	float window_ratio;
	float x,y,x1,y1,a,t;
	window_ratio=(GLfloat)window_width/(GLfloat)window_height;

	//x=(float)((mouse_x)*2.8f*(float)zoom_level/(float)window_width)-(2.8f*zoom_level/2.0f);
	x=(float)((mouse_x)*window_ratio*2.0*(float)zoom_level/(float)window_width)-(window_ratio*zoom_level);
	y=(float)((window_height-mouse_y)*2.0f*zoom_level/window_height)-(2.0*zoom_level/2.0f);

	a=(rz)*3.1415926/180;
	t=(rx)*3.1415926/180;

	y=(float)y/(float)cos(t);

	x1=x*cos(a)+y*sin(a);
	y1=y*cos(a)-x*sin(a);

	scene_mouse_x=-camera_x+x1;
	scene_mouse_y=-camera_y+y1;

#ifdef EYE_CANDY
	if ((cur_mode == mode_eye_candy) && (!minimap_on))
	{
//		const float z = -2.2f + height_map[(int)(scene_mouse_y*2)*tile_map_size_x*6+(int)(scene_mouse_x*2)] * 0.2;
		update_eye_candy_position(scene_mouse_x, scene_mouse_y);
	}
#endif
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
#ifdef	NEW_TEXTURES
	bind_texture(buttons_text);
#else	/* NEW_TEXTURES */
	get_and_set_texture_id(buttons_text);
#endif	/* NEW_TEXTURES */

	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_QUADS);

	if(cur_mode!=mode_tile && view_tile)
	glColor3f(1.0f,1.0f,1.0f);
	else if(!view_tile && cur_mode!=mode_tile)glColor3f(0.3f,0.3f,0.3f);
	else glColor3f(0.0f,1.0f,1.0f);
#ifdef	NEW_TEXTURES
	draw_2d_thing((float)64/255, 0.0f, (float)96/255, (float)32/255, 0,0,32,32);
#else	/* NEW_TEXTURES */
	draw_2d_thing((float)64/255,1.0f,(float)96/255,1.0f-(float)32/255, 0,0,32,32);
#endif	/* NEW_TEXTURES */

	if(cur_mode!=mode_2d && view_2d)
	glColor3f(1.0f,1.0f,1.0f);
	else if(!view_2d && cur_mode!=mode_2d)glColor3f(0.3f,0.3f,0.3f);
	else glColor3f(0.0f,1.0f,1.0f);
#ifdef	NEW_TEXTURES
	draw_2d_thing((float)32/255, 0.0f,(float)64/255, (float)32/255, 32,0,64,32);
#else	/* NEW_TEXTURES */
	draw_2d_thing((float)32/255,1.0f,(float)64/255,1.0f-(float)32/255, 32,0,64,32);
#endif	/* NEW_TEXTURES */

	if(cur_mode!=mode_3d && view_3d)
	glColor3f(1.0f,1.0f,1.0f);
	else if(!view_3d && cur_mode!=mode_3d)glColor3f(0.3f,0.3f,0.3f);
	else glColor3f(0.0f,1.0f,1.0f);
#ifdef	NEW_TEXTURES
	draw_2d_thing(0, 0.0f, (float)32/255, (float)32/255, 64,0,96,32);
#else	/* NEW_TEXTURES */
	draw_2d_thing(0,1.0f,(float)32/255,1.0f-(float)32/255, 64,0,96,32);
#endif	/* NEW_TEXTURES */

	if(cur_mode!=mode_particles && view_particles)
	glColor3f(1.0f,1.0f,1.0f);
	else if(!view_particles && cur_mode!=mode_particles)glColor3f(0.3f,0.3f,0.3f);
	else glColor3f(0.0f,1.0f,1.0f);
#ifdef	NEW_TEXTURES
	draw_2d_thing((float)192/255, (float)32/255, (float)224/255, (float)64/255, 96,0,128,32);
#else	/* NEW_TEXTURES */
	draw_2d_thing((float)192/255,1.0f-(float)32/255,(float)224/255,1.0f-(float)64/255, 96,0,128,32);
#endif	/* NEW_TEXTURES */

#ifdef	EYE_CANDY
	if(cur_mode!=mode_eye_candy && view_eye_candy)
	glColor3f(1.0f,1.0f,1.0f);
	else if(!view_eye_candy && cur_mode!=mode_eye_candy)glColor3f(0.3f,0.3f,0.3f);
	else glColor3f(0.0f,1.0f,1.0f);
#ifdef	NEW_TEXTURES
	draw_2d_thing((float)224/255, (float)32/255, (float)255/255, (float)64/255, 128,0,160,32);
#else	/* NEW_TEXTURES */
	draw_2d_thing((float)224/255,1.0f-(float)32/255,(float)255/255,1.0f-(float)64/255, 128,0,160,32);
#endif	/* NEW_TEXTURES */
#endif	//EYE_CANDY

	if(cur_mode!=mode_light && view_light)
	glColor3f(1.0f,1.0f,1.0f);
	else if(!view_light && cur_mode!=mode_light)glColor3f(0.3f,0.3f,0.3f);
	else glColor3f(0.0f,1.0f,1.0f);
#ifdef	NEW_TEXTURES
	draw_2d_thing((float)96/255, 0.0f,(float)128/255, (float)32/255, 160,0,192,32);
#else	/* NEW_TEXTURES */
	draw_2d_thing((float)96/255,1.0f,(float)128/255,1.0f-(float)32/255, 160,0,192,32);
#endif	/* NEW_TEXTURES */

	if(cur_mode!=mode_height && view_height)
	glColor3f(1.0f,1.0f,1.0f);
	else if(!view_height && cur_mode!=mode_height)glColor3f(0.3f,0.3f,0.3f);
	else glColor3f(0.0f,1.0f,1.0f);
#ifdef	NEW_TEXTURES
	draw_2d_thing((float)160/255, (float)32/255, (float)192/255, (float)64/255, 192,0,224,32);
#else	/* NEW_TEXTURES */
	draw_2d_thing((float)160/255,1.0f-(float)32/255,(float)192/255,1.0f-(float)64/255, 192,0,224,32);
#endif	/* NEW_TEXTURES */


	if(cur_mode!=mode_map)
	glColor3f(1.0f,1.0f,1.0f);
	else glColor3f(0.0f,1.0f,1.0f);
#ifdef	NEW_TEXTURES
	draw_2d_thing((float)128/255, 0.0f, (float)160/255, (float)32/255, 224,0,256,32);
#else	/* NEW_TEXTURES */
	draw_2d_thing((float)128/255,1.0f,(float)160/255,1.0f-(float)32/255, 224,0,256,32);
#endif	/* NEW_TEXTURES */


	if(cur_tool!=tool_select)
	glColor3f(1.0f,1.0f,1.0f);
	else glColor3f(0.0f,1.0f,1.0f);
#ifdef	NEW_TEXTURES
	draw_2d_thing(0, (float)32/255, (float)32/255, (float)64/255, 256,0,288,32);
#else	/* NEW_TEXTURES */
	draw_2d_thing(0,1.0f-(float)32/255,(float)32/255,1.0f-(float)64/255, 256,0,288,32);
#endif	/* NEW_TEXTURES */

	if(cur_tool!=tool_clone)
	glColor3f(1.0f,1.0f,1.0f);
	else glColor3f(0.0f,1.0f,1.0f);
#ifdef	NEW_TEXTURES
	draw_2d_thing((float)32/255, (float)32/255, (float)64/255, (float)64/255, 288,0,320,32);
#else	/* NEW_TEXTURES */
	draw_2d_thing((float)32/255,1.0f-(float)32/255,(float)64/255,1.0f-(float)64/255, 288,0,320,32);
#endif	/* NEW_TEXTURES */

	if(cur_tool!=tool_new)
	glColor3f(1.0f,1.0f,1.0f);
	else glColor3f(0.0f,1.0f,1.0f);
#ifdef	NEW_TEXTURES
	draw_2d_thing((float)224/255, 0.0f, (float)256/255, (float)32/255, 320,0,352,32);
#else	/* NEW_TEXTURES */
	draw_2d_thing((float)224/255,1.0f,(float)256/255,1.0f-(float)32/255, 320,0,352,32);
#endif	/* NEW_TEXTURES */

	if(cur_tool!=tool_kill)
	glColor3f(1.0f,1.0f,1.0f);
	else glColor3f(0.0f,1.0f,1.0f);
#ifdef	NEW_TEXTURES
	draw_2d_thing((float)192/255, 0.0f, (float)224/255, (float)32/255, 352,0,384,32);
#else	/* NEW_TEXTURES */
	draw_2d_thing((float)192/255,1.0f,(float)224/255,1.0f-(float)32/255, 352,0,384,32);
#endif	/* NEW_TEXTURES */

	glColor3f(1.0f,1.0f,1.0f);
	//save
#ifdef	NEW_TEXTURES
	draw_2d_thing((float)64/255, (float)32/255, (float)96/255, (float)64/255, 384,0,416,32);
#else	/* NEW_TEXTURES */
	draw_2d_thing((float)64/255,1.0f-(float)32/255,(float)96/255,1.0f-(float)64/255, 384,0,416,32);
#endif	/* NEW_TEXTURES */

	//open
#ifdef	NEW_TEXTURES
	draw_2d_thing((float)96/255, (float)32/255, (float)128/255, (float)64/255, 416,0,448,32);
#else	/* NEW_TEXTURES */
	draw_2d_thing((float)96/255,1.0f-(float)32/255,(float)128/255,1.0f-(float)64/255, 416,0,448,32);
#endif	/* NEW_TEXTURES */

	//new
#ifdef	NEW_TEXTURES
	draw_2d_thing((float)128/255, (float)32/255, (float)160/255, (float)64/255, 448,0,480,32);
#else	/* NEW_TEXTURES */
	draw_2d_thing((float)128/255,1.0f-(float)32/255,(float)160/255,1.0f-(float)64/255, 448,0,480,32);
#endif	/* NEW_TEXTURES */

	glEnd();
}

void draw_3d_obj_info()
{
	unsigned char str[128];
	int x_menu,y_menu;
	if (cur_mode!=mode_3d || selected_3d_object==-1 || objects_list[selected_3d_object] == NULL)
		return;
		
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

	sprintf((char *)str, "Red  : %03.2f",objects_list[selected_3d_object]->color[0]);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Green: %03.2f",objects_list[selected_3d_object]->color[1]);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Blue : %03.2f",objects_list[selected_3d_object]->color[2]);
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
	if(cur_mode!=mode_2d || selected_2d_object==-1||obj_2d_list[selected_2d_object]==NULL)return;

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
	if(cur_mode!=mode_light || selected_light==-1 || lights_list[selected_light] == NULL)return;

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



int tiles_win = 0;

int display_tiles_handler(window_info *win)
{

	int i,j;
	int x_start,y_start,x_end,y_end;

	//draw a black rectangle
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor3f(0.0f,0.0f,0.0f);
	glVertex3i(0,win->len_y-1,0);
	glVertex3i(0,20,0);
	glVertex3i(win->len_x-1,20,0);
	glVertex3i(win->len_x-1,win->len_y-1,0);
	glColor3f(1.0f,1.0f,1.0f);
	glEnd();
	glEnable(GL_TEXTURE_2D);

	for(i=tile_offset,j=0;i<tiles_no && j<64 ;i++,j++)
		{

			x_start=48*(j%8);
			x_end=x_start+47;

			y_start=48*(j/8);
			y_start+=20;
			y_end=y_start+47;

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
#ifdef	NEW_TEXTURES
				bind_texture(tile_list[i]);
#else	/* NEW_TEXTURES */
				get_and_set_texture_id(tile_list[i]);
#endif	/* NEW_TEXTURES */

				glBegin(GL_QUADS);
				glTexCoord2f(0,0.0f);
				glVertex3i(x_start,y_end,0);

				glTexCoord2f(0,1.0f);
				glVertex3i(x_start,y_start,0);

				glTexCoord2f(1.0f,1.0f);
				glVertex3i(x_end,y_start,0);

				glTexCoord2f(1.0f,0.0f);
				glVertex3i(x_end,y_end,0);

				glEnd();
			}
		}

	return 1;
}

int check_tiles_interface (window_info *win, int _x, int _y)
{
	int tile_id;

	if (_x > win->len_x - 20 && mouse_y < 20)
	{
		view_tiles_list = 0;
		return 0;
	}
	
	if (_y < 20) return 0; 

	tile_id = 8 * ( (_y-20) / 48 ) + _x / 48;	
	tile_id+=tile_offset;
	if(tile_id>tiles_no)return 0;//check to see if we clicked on an empty tile

	cur_tool=tool_select;
	selected_tile=tile_id;

	return 1;
}

void display_tiles_list()
{
	if(tiles_win <= 0){
		tiles_win = create_window("tiles", 0, 0, 64, 128, 48*8, 48*8+20, ELW_WIN_DEFAULT);

		set_window_handler(tiles_win, ELW_HANDLER_DISPLAY, &display_tiles_handler );
		set_window_handler(tiles_win, ELW_HANDLER_CLICK, &check_tiles_interface );
		
	} else {
		show_window(tiles_win);
		select_window(tiles_win);
	}
	display_window(tiles_win);
}



int height_win = 0;

int display_heights_handler(window_info *win)
{

	int i;
	int x_start,y_start,x_end,y_end;

	//draw a black rectangle
	//n_tile_menu_offset isn't needed in a window...
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor3f(0.0f,0.0f,0.0f);
	glVertex3i(0,win->len_y-1,0);
	glVertex3i(0,20,0);
	glVertex3i(win->len_x-1,20,0);
	glVertex3i(win->len_x-1,win->len_y-1,0);
	glColor3f(1.0f,1.0f,1.0f);
	glEnd();
	
	for(i=0;i<32;i++)
		{

			x_start=32*(i%8);
			x_end=x_start+31;

			y_start=32*(i/8);
			y_start+=20;
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
	
	return 1;
}

int check_height_interface (window_info *win, int _x, int _y)
{
	int height_id;
	
	if (_y < 20) return 0; 
	//check to see if we clicked outside our rectangle

	height_id = 8 * ( (_y - 20) / 32 ) + _x / 32;
	cur_tool=tool_select;
	selected_height=height_id;
	
	return 1;                 // Disable clickthroughs?
}

void display_heights_list()
{
	if(height_win <= 0){
		height_win = create_window("heights", 0, 0, 64, 128, 32*8, 32*4+20, ELW_WIN_DEFAULT);

		set_window_handler(height_win, ELW_HANDLER_DISPLAY, &display_heights_handler );
		set_window_handler(height_win, ELW_HANDLER_CLICK, &check_height_interface );
		
	} else {
		show_window(height_win);
		select_window(height_win);
	}
	display_window(height_win);
}


void check_mouse_minimap()
{
	int minimap_x_start=window_width/2-128;
	int minimap_y_start;
	int x_map_pos;
	int y_map_pos;
	int scale;

	if(window_width<window_height) scale=window_width/256;
	else scale=window_height/256;

	minimap_x_start/=scale;
	minimap_y_start=10*scale;
	
	if(mouse_x<minimap_x_start || mouse_y<minimap_y_start
	|| mouse_x>minimap_x_start+256*scale || mouse_y>minimap_y_start+256*scale) return;

	x_map_pos=((float)(mouse_x-minimap_x_start)/(float)scale)*tile_map_size_x/256;
	y_map_pos=tile_map_size_y-(((mouse_y-minimap_y_start))/(float)scale)*tile_map_size_y/256;
	mx=x_map_pos*3;
	my=y_map_pos*3;
	minimap_on=0;
}

void draw_mouse_minimap()
{
	int minimap_x_start=window_width/2-128;
	int minimap_y_start;
	int x_map_pos;
	int y_map_pos;
	int x,y, scale;

	if(window_width<window_height) scale=window_width/256;
	else scale=window_height/256;

	minimap_x_start/=scale;
	minimap_y_start=10*scale;
	
	if(mouse_x<minimap_x_start || mouse_y<minimap_y_start
	|| mouse_x>minimap_x_start+256*scale || mouse_y>minimap_y_start+256*scale)return;

	x_map_pos=((float)(mouse_x-minimap_x_start)/(float)scale)*tile_map_size_x/256;
	y_map_pos=tile_map_size_y-(((mouse_y-minimap_y_start))/(float)scale)*tile_map_size_y/256;
	mx=-x_map_pos*3;
	my=-y_map_pos*3;

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

	map_has_changed=1;

}

//Generates a minimap and returns the texture's integer value
GLuint generate_minimap()
{
        int x=0,y=0,i,j;
        float scale=(float)256/tile_map_size_x;//Set the scale...
        //img_struct * cur_img;
#ifdef	NEW_TEXTURES
	image_t* cur_img;
	Uint32* ptr;
	Sint32 s;
#else	/* NEW_TEXTURES */
        texture_struct * cur_img;
#endif	/* NEW_TEXTURES */
        GLuint texture;

        char map[256*256*4]={0};

#ifdef	NEW_TEXTURES
	if (scale >= 1.0f)
	{
		s = scale;

		for (y = 0; y < tile_map_size_y; y++)
		{
			for (x = 0; x < tile_map_size_x; x++)
			{
				//Scale up
				for (i = 0; i < scale; i++)
				{
					for (j = 0; j < scale; j++)
					{
						cur_img = &map_tiles[tile_map[x * tile_map_size_y + y]];

						ptr = (Uint32*)(map + ((x * s + j) + (i + y * s) * 256) * 4);

						if (cur_img->image == NULL)
						{
							*ptr = 0x00000000;
						}
						else
						{
							*ptr = *((Uint32 *)(cur_img->image + (((i+y*(int)scale)&(cur_img->height-1))*cur_img->width+((j+x*(int)scale)&(cur_img->width-1)))*4));
						}
					}
				}
			}
		}
	}
#else	/* NEW_TEXTURES */
        if(scale>=1)
                {
                        for(y=0;y<tile_map_size_y;y++)
                                {
                                for(x=0;x<tile_map_size_x;x++)
                                        {
                                        //Scale up
                                        for(i=0;i<scale;i++)
                                                {
                                                for(j=0;j<scale;j++)
                                                        {
                                                                if((cur_img=&map_tiles[tile_map[x*tile_map_size_y+y]])==NULL||
                                                                    cur_img->texture==NULL)
                                                                	*((Uint32 *)(map+(((x*(int)scale+j)+(i+y*(int)scale)*256)*4)))=0x00000000;
								else
                                                                	*((Uint32 *)(map+(((x*(int)scale+j)+(i+y*(int)scale)*256)*4)))=*((Uint32 *)(cur_img->texture+(((i+y*(int)scale)&(cur_img->y_size-1))*cur_img->x_size+((j+x*(int)scale)&(cur_img->x_size-1)))*4));
                                                        }
                                                }
                                        }
                                }
                }
        else
                {
                        //Maps are not available at 256x256 yet... should they be supported?
                        /*
                        scale=(float)1/scale;
                        for(y=0;y<tile_map_size_y;y++)
                                {
                                        for(x=0;x<tile_map_size_x;x+=scale)
                                                {
                                                        cur_img=&map_tiles[tile_map[x*tile_map_size_y+y]];
                                                        *((Uint32*)(map+(((x*(int)scale+j)+(i+y*(int)scale)*256)*4))=*((Uint32 *)(cur_img->img+(((i+y*(int)scale)&(cur_img->y-1))*cur_img->x+((j+x*(int)scale)&(cur_img->x-1)))*4));;
                                                }

                                }*/
                }
#endif	/* NEW_TEXTURES */

	//OK, now check the 3d objects... we want them all to show up as red dots...
	scale=(float)3/scale;//Change the scale for 3d objects...
	for (i = 0; i < MAX_OBJ_3D; i++)
		{
			if(objects_list[i] && objects_list[i]->blended!=20)
				{
					x=(float)objects_list[i]->x_pos/scale;
					y=(float)objects_list[i]->y_pos/scale;

					x&=255;//Just in case...
					y&=255;
					
					*((Uint32*)(map+4*(x*256+y)))=0xFF0000F1;
				}
		}
        
	glGenTextures(1, &texture);

        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        if(have_arb_compression)
                {
                        if(have_s3_compression)
                                glTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGBA_S3TC_DXT5_EXT, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, map);
                        else
                                glTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGBA_ARB, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, map);
                }
        else glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, map);

        return texture;
}


GLuint minimap_tex;
int map_has_changed=1;

void draw_minimap()
{
	int minimap_x_start=window_width/2-128;
	int minimap_y_end;
	int scale;//looks ugly if it's accurate :)...
	float x_map_pos, y_map_pos;
	
	if(map_has_changed)
	        {
		        if(minimap_tex>0) glDeleteTextures(1,&minimap_tex);
			minimap_tex=generate_minimap();
		}
	map_has_changed=0;

	//We have the map, display the texture
	
	if((Uint32)last_texture!=minimap_tex)
		{
			glBindTexture(GL_TEXTURE_2D, minimap_tex);
			last_texture=minimap_tex;
		}
	
	if(window_width<window_height) scale=window_width/256;
	else scale=window_height/256;

        x_map_pos=(float)mx/(float)(tile_map_size_x*3.0f)*256.0f*scale;
	y_map_pos=(float)my/(float)(tile_map_size_y*3.0f)*256.0f*scale;
	
	minimap_x_start/=scale*scale;

	glPushMatrix();
	glScalef(scale,scale,scale);
	
	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 0.0f); glVertex3i(minimap_x_start,10+256,0);
	glTexCoord2f(1.0f, 0.0f); glVertex3i(minimap_x_start,10,0);
	glTexCoord2f(1.0f, 1.0f); glVertex3i(minimap_x_start+256,10,0);
	glTexCoord2f(0.0f, 1.0f); glVertex3i(minimap_x_start+256,10+256,0);
	
	glEnd();

	glPopMatrix();
	
	if(show_position_on_minimap)
		{
			glDisable(GL_TEXTURE_2D);
			glColor3f(1.0f,0.0f,0.0f);
	
			minimap_x_start*=scale;
			minimap_y_end=(10+256)*scale;
	
			glBegin(GL_LINES);
			glVertex2i(minimap_x_start+x_map_pos-7,minimap_y_end-y_map_pos+7);
			glVertex2i(minimap_x_start+x_map_pos+7,minimap_y_end-y_map_pos-7);
	
			glVertex2i(minimap_x_start+x_map_pos+7,minimap_y_end-y_map_pos+7);
			glVertex2i(minimap_x_start+x_map_pos-7,minimap_y_end-y_map_pos-7);
			glEnd();
	
			glColor3f(1.0f,1.0f,1.0f);
			glEnable(GL_TEXTURE_2D);
		}
#ifdef EYE_CANDY
	draw_bounds_on_minimap();
#endif
}

int map_size=0;

int new_map_display_handler ()
{
	glColor3f (1.0f,1.0f,0.0f);
	draw_string (2, 2, (const unsigned char*) "    Map Size", 1);
		
	if (map_size == 0)
		glColor3f (0.0f, 0.5f, 1.0f);
	else 
		glColor3f (1.0f, 1.0f, 1.0f);
	draw_string (2, 2*17+2, (const unsigned char*) "Very Small [16x16]", 1);

	if (map_size == 1)
		glColor3f (0.0f, 0.5f, 1.0f);
	else
		glColor3f (1.0f, 1.0f, 1.0f);
	draw_string (2, 3*17+2, (const unsigned char*) "Small      [32x32]", 1);
		
	if (map_size == 2)
		glColor3f (0.0f, 0.5f, 1.0f);
	else 
		glColor3f (1.0f, 1.0f, 1.0f);
	draw_string (2, 4*17+2, (const unsigned char*) "Medium     [64x64]", 1);

	if (map_size == 3)
		glColor3f (0.0f, 0.5f, 1.0f);
	else 
		glColor3f (1.0f, 1.0f, 1.0f);
	draw_string (2, 5*17+2, (const unsigned char*) "Large      [128x128]", 1);

	if (map_size == 4)
		glColor3f (0.0f, 0.5f, 1.0f);
	else 
		glColor3f (1.0f, 1.0f, 1.0f);
	draw_string (2, 6*17+2, (const unsigned char*) "Huge       [256x256]", 1);

	glColor3f (1.0f, 1.0f, 1.0f);
	draw_string (2, 8*17+2, (const unsigned char*) "   [Ok]    [Cancel]", 1);
	
	return 1;
}

int new_map_click_handler (window_info *win, int _x, int _y)
{
	if (_y > 2*17+2 && _y < 3*17+2)
	{
		map_size = 0;
	}
	else if (_y > 3*17+2 && _y < 4*17+2)
	{
		map_size = 1;
	}
	else if (_y > 4*17+2 && _y < 5*17+2)
	{
		map_size = 2;
	}
	else if (_y > 5*17+2 && _y < 6*17+2)
	{
		map_size = 3;
	}
	else if (_y > 6*17+2 && _y < 7*17+2)
	{
		map_size = 4;
	}
	else if (_y > 8*17+2 && _y < 9*17+2)
	{
		if (_x > 3*12+2 && _x < 7*12+2)
		{
			hide_window (new_map_menu);
			switch (map_size)
			{
				case 0: new_map (16, 16); break;
				case 1: new_map (32, 32); break;
				case 2: new_map (64, 64); break;
				case 3: new_map (128, 128); break;
				case 4: new_map (256, 256); break;
				default: LOG_ERROR ("Unknown map size!");
			}
		}
		else if (_x > 11*12+2 && _x < 19*12+2)
		{
			hide_window (new_map_menu);
		}
	}
	else
	{
		return 0;
	}
	
	return 1;
}

void display_new_map_menu()
{
	if (new_map_menu < 0)
	{
		int x_menu, y_menu, w_menu, h_menu;
		
		x_menu = 90;
		y_menu = 82;
		w_menu = 240;
		h_menu = 170;
		new_map_menu = create_window ("New map", -1, 0, x_menu, y_menu, w_menu, h_menu, ELW_WIN_DEFAULT);
		
		set_window_handler (new_map_menu, ELW_HANDLER_DISPLAY, new_map_display_handler);
		set_window_handler (new_map_menu, ELW_HANDLER_CLICK, new_map_click_handler);
		
		show_window (new_map_menu);select_window (new_map_menu);
	}
	else
	{
		show_window (new_map_menu);
	}
}

void display_map_settings()
{
	unsigned char str[128];
	int x_menu,y_menu;

	if (
	    (cur_mode != mode_map)
	   )
	  return;

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
