#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "global.h"
#include "draw_scene.h"
#include "weather.h"
#include "elwindows.h"

GLuint paper1_text;

char have_display = 0;
float cx=0;
float cy=0;
float cz=0;
float rx=-60;
float ry=0;
float rz=45;
float terrain_scale=2.0f;
float zoom_level=3.0f;
float name_zoom=1.0f;

float fine_camera_rotation_speed;
float normal_camera_rotation_speed;

float camera_rotation_speed;
int camera_rotation_frames;

float camera_tilt_speed;
int camera_tilt_frames;

double camera_x_speed;
int camera_x_frames;

double camera_y_speed;
int camera_y_frames;

double camera_z_speed;
int camera_z_frames;

int camera_zoom_dir;
int camera_zoom_frames=0;
float new_zoom_level=3.0f;
float camera_distance = 2.5f;

float scene_mouse_x;
float scene_mouse_y;

int last_texture=-2;
int font_text;
int cons_text;
int icons_text;

int login_text;
int ground_detail_text;

float clouds_movement_u=-8;
float clouds_movement_v=-3;
Uint32 last_clear_clouds=0;

GLenum base_unit=GL_TEXTURE0_ARB,detail_unit=GL_TEXTURE1_ARB,shadow_unit=GL_TEXTURE2_ARB,extra_unit=GL_TEXTURE3_ARB,normal_map_unit=GL_TEXTURE4_ARB;

Uint32 draw_delay = 0;

void draw_scene()
{
	CHECK_GL_ERRORS();

	//clear the clouds cache too...
	if(last_clear_clouds+10000<cur_time)clear_clouds_cache();

#ifndef NEW_WEATHER
	if (dungeon) {
		glClearColor(0.0, 0.0, 0.0, 0.0);
	} else {
		glClearColor(fogColor[0], fogColor[1], fogColor[2], 0.0);
	}
#endif
	if(!shadows_on || !have_stencil)glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	else glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

	if (!have_display)
	{
		new_zoom_level = zoom_level;	// No scrolling when switching modes...
		if (quickbar_relocatable && quickbar_win >= 0) // Hack 
		{
			if (get_show_window (quickbar_win) && windows_list.window[quickbar_win].cur_x < window_width - hud_x && window_height - windows_list.window[quickbar_win].cur_y > hud_y)
				hide_window (quickbar_win);
		}
	}

	glLoadIdentity ();	// Reset The Matrix
	
	Enter2DMode ();
	display_windows (1);

	// Have to draw the dragged item *after* all windows
	
	glColor3f(1.0f,1.0f,1.0f);
	if (item_dragged != -1)
		drag_item (item_dragged, 0, 0);
	else if (use_item != -1 && current_cursor == CURSOR_USE_WITEM)
		drag_item (use_item, 0, 1);
	else if (storage_item_dragged != -1) 
		drag_item (storage_item_dragged, 1, 0);

	Leave2DMode ();

	if(elwin_mouse >= 0)
	{
		if (current_cursor != elwin_mouse) change_cursor(elwin_mouse);
		elwin_mouse = -1;
	}

	SDL_GL_SwapBuffers();
	CHECK_GL_ERRORS();
	
	if (draw_delay > 0)
	{
		SDL_Delay (draw_delay);
		draw_delay = 0;
	}
}

void get_tmp_actor_data()
{
	int i;
	LOCK_ACTORS_LISTS();
	for(i=0;i<max_actors;i++)
		{
			if(actors_list[i])
				{
					actors_list[i]->tmp.x_pos=actors_list[i]->x_pos;
					actors_list[i]->tmp.y_pos=actors_list[i]->y_pos;
					actors_list[i]->tmp.z_pos=actors_list[i]->z_pos;
					
					actors_list[i]->tmp.x_tile_pos=actors_list[i]->x_tile_pos;
					actors_list[i]->tmp.y_tile_pos=actors_list[i]->y_tile_pos;
					
					actors_list[i]->tmp.x_rot=actors_list[i]->x_rot;
					actors_list[i]->tmp.y_rot=actors_list[i]->y_rot;
					actors_list[i]->tmp.z_rot=actors_list[i]->z_rot;
					
					actors_list[i]->tmp.have_tmp=1;
				}
		}
	UNLOCK_ACTORS_LISTS();
}

void move_camera ()
{
	float x, y, z;
	static int lagged=1;
	actor * me=pf_get_our_actor();
	
	if(!me || !me->tmp.have_tmp){
		lagged=1;
		return;
	}

	x=me->tmp.x_pos+0.25f;
	y=me->tmp.y_pos+0.25f;
	z=-2.2f+height_map[me->tmp.y_tile_pos*tile_map_size_x*6+me->tmp.x_tile_pos]*0.2f+sitting;

	if(lagged){
		cx-=(x-(-cx));
		cy-=(y-(-cy));
		cz-=(z-(-cz));
		camera_x_frames=0;
		camera_y_frames=0;
		camera_z_frames=0;
		lagged=0;
		regenerate_near_objects=
		regenerate_near_2d_objects=1;
	} else {
		//move near the actor, but smoothly
		camera_x_speed=(x-(-cx))/16.0;
		camera_x_frames=16;
		camera_y_speed=(y-(-cy))/16.0;
		camera_y_frames=16;
		camera_z_speed=(z-(-cz))/16.0;
		camera_z_frames=16;
	}
	
	//check to see if we are out of the map
	if(cx>-7.5f)cx=-7.5f;
	if(cy>-7.5f)cy=-7.5f;
	if(cx<-(tile_map_size_x*3-7.9))cx=(float)-(tile_map_size_x*3-7.9);
	if(cy<-(tile_map_size_x*3-7.9))cy=(float)-(tile_map_size_x*3-7.9);

	glTranslatef(0.0f, 0.0f, -zoom_level*camera_distance);
	glRotatef(rx, 1.0f, 0.0f, 0.0f);
	glRotatef(rz, 0.0f, 0.0f, 1.0f);
	glTranslatef(cx,cy, cz);

	//test only
	update_position();
}

void update_camera()
{
	if(camera_rotation_frames)
		{
			rz+=camera_rotation_speed;
			if(rz > 360) {
				rz -= 360;
			} else if (rz < 0) {
				rz += 360;
			}
			camera_rotation_frames--;
			regenerate_near_objects=
			regenerate_near_2d_objects=1;
		}
	if(camera_x_frames)
		{
			if(camera_x_speed>0.005 || camera_x_speed<-0.005){
				cx-=camera_x_speed;
				regenerate_near_objects=
				regenerate_near_2d_objects=1;
			}
			camera_x_frames--;
		}
	if(camera_y_frames)
		{
			if(camera_y_speed>0.0005 || camera_y_speed<-0.005){
				cy-=camera_y_speed;
				regenerate_near_objects=
				regenerate_near_2d_objects=1;
			}
			camera_y_frames--;
		}
	if(camera_z_frames)
		{
			if(camera_z_speed>0.0005 || camera_z_speed<-0.005)
				cz-=camera_z_speed;
			camera_z_frames--;
		}
	if(camera_tilt_frames) {
		if(camera_tilt_speed<0) {
			if(rx>-60)rx+=camera_tilt_speed;
			if(rx<-60) {
				rx=-60;
				camera_tilt_frames=0;
			} else
				camera_tilt_frames--;
		} else {
			if(rx<-45)rx+=camera_tilt_speed;
			if(rx>-45) {
				rx=-45;
				camera_tilt_frames=0;
			} else
				camera_tilt_frames--;
		}
	}
	if(camera_zoom_frames) {
		if(camera_zoom_dir == 1) {
			if(zoom_level<3.75f){
				new_zoom_level+=0.05f;
				camera_zoom_frames--;
				regenerate_near_objects=
				regenerate_near_2d_objects=1;
			} else 
				camera_zoom_frames = 0;
		} else {
			if(zoom_level>sitting){
				new_zoom_level-=0.05f;
				camera_zoom_frames--;
				regenerate_near_objects=
				regenerate_near_2d_objects=1;
			} else 
				camera_zoom_frames = 0;
		}
	}
	if(zoom_level<sitting) {
		new_zoom_level=zoom_level=sitting;
		resize_root_window();
	}
}

int update_have_display(window_info * win) {
	// if the calling window is shown, we have a display, else check all 3d windows
	have_display = (win->displayed || get_show_window(game_root_win) || get_show_window(newchar_root_win));
	return 0;
}
