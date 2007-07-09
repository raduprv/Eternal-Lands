#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "global.h"
#include "draw_scene.h"
#include "weather.h"
#include "elwindows.h"

GLuint paper1_text;

char have_display = 0;
float camera_x=0;
float camera_y=0;
float camera_z=0;
float old_camera_x=0;
float old_camera_y=0;
float old_camera_z=0;
float c_delta= 0.1f;
float rx=-60;
float ry=0;
float rz=45;
float terrain_scale=2.0f;
float zoom_level=3.0f;
float name_zoom=1.0f;
#ifdef SKY_FPV_CURSOR
int cam_turn=0;

//Assists for doing gluProject()
//Store view and project matrix changes here
//modl ensures that there's a place to store current model state. 
//Maybe modl should be removed.
double project[16], modl[16];
int view[4];

//First Person Camera mode state
int first_person = 0;
#endif /* SKY_FPV_CURSOR */

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

#ifdef SKY_FPV_CURSOR
//Follow camera state stuff
int fol_cam = 1;	//On or off...
float camera_kludge = 0.0f;	//Stores direction player is facing
float last_kludge=0;	//Stores how far the camera deviated from camera_kludge
float fol_strn = 0.03f;
float fol_con,fol_lin,fol_quad;
int ext_cam = 1;	//Extended camera state
//Check that this is still used, I think it's not.
float hold_camera=180;

#endif /* SKY_FPV_CURSOR */
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
#ifdef SKY_FPV_CURSOR
	draw_cursor();
#endif /* SKY_FPV_CURSOR */

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
#ifdef SKY_FPV_CURSOR
	float hx, hy, hz, follow_speed;
#endif /* SKY_FPV_CURSOR */
	static int lagged=1;
	actor * me=pf_get_our_actor();
	
	if(!me || !me->tmp.have_tmp){
		lagged=1;
		return;
	}

	x = (float)me->tmp.x_pos+0.25f;
	y = (float)me->tmp.y_pos+0.25f;

#ifndef SKY_FPV_CURSOR
	z=-2.2f+height_map[me->tmp.y_tile_pos*tile_map_size_x*6+me->tmp.x_tile_pos]*0.2f+sitting;
#else /* SKY_FPV_CURSOR */
	if(!cal_get_head(me, &hx, &hy, &hz) || hz < 0.1f){
		//There was an error. We can try approximately correct numbers here.
		hz = 1.0f;
	}

	if (!ext_cam){
		z=-2.2f+height_map[me->tmp.y_tile_pos*tile_map_size_x*6+me->tmp.x_tile_pos]*0.2f+sitting;
	} else if (first_person){
		z=-2.1f+height_map[me->tmp.y_tile_pos*tile_map_size_x*6+me->tmp.x_tile_pos]*0.2f+hz;
	} else {
		z=-1.6f+height_map[me->tmp.y_tile_pos*tile_map_size_x*6+me->tmp.x_tile_pos]*0.2f+hz;
	}

	if(first_person||ext_cam){
		follow_speed = 8.0f;
	} else {
		follow_speed = 16.0f;
	}
#endif /* SKY_FPV_CURSOR */
	if(lagged){
		camera_x = -x;
		camera_y = -y;
		camera_z = -z;
		camera_x_frames=0;
		camera_y_frames=0;
		camera_z_frames=0;
		lagged=0;
		set_all_intersect_update_needed(main_bbox_tree);
	} else {
		//move near the actor, but smoothly
#ifndef SKY_FPV_CURSOR
		camera_x_speed=(x-(-camera_x))/16.0;
		camera_x_frames=16;
		camera_y_speed=(y-(-camera_y))/16.0;
		camera_y_frames=16;
		camera_z_speed=(z-(-camera_z))/16.0;
		camera_z_frames=16;
	}

	glTranslatef(0.0f, 0.0f, -zoom_level*camera_distance);
	glRotatef(rx, 1.0f, 0.0f, 0.0f);
	glRotatef(rz, 0.0f, 0.0f, 1.0f);
	glTranslatef(camera_x,camera_y, camera_z);

#else /* SKY_FPV_CURSOR */
		camera_x_speed=(x+camera_x)/follow_speed;
		camera_x_frames=follow_speed;
		camera_y_speed=(y+camera_y)/follow_speed;
		camera_y_frames=follow_speed;
		camera_z_speed=(z+camera_z)/follow_speed;
		camera_z_frames=follow_speed;
	}


	if (first_person){
		glTranslatef(hx,hy,0);
	} else {
		glTranslatef(0.0f, 0.0f, -zoom_level*camera_distance);
	}

	glRotatef(rx-(first_person?35:0), 1.0f, 0.0f, 0.0f);
	glRotatef(rz, 0.0f, 0.0f, 1.0f);
	glTranslatef(camera_x, camera_y, camera_z);
#endif /* SKY_FPV_CURSORS */

#ifndef NEW_SOUND	//test only
	update_position();
#endif	//NEW_SOUND
}

#ifdef SKY_FPV_CURSOR
int adjust_view;
#endif /* SKY_FPV_CURSOR */

void update_camera()
{
#ifndef SKY_FPV_CURSOR
	int adjust_view= 0;
#else /* SKY_FPV_CURSOR */
	float adjust;

	if (fol_cam) rz=hold_camera;
#endif /* SKY_FPV_CURSOR */

	if(camera_rotation_frames){
		rz+=camera_rotation_speed;
		if(rz > 360) {
			rz -= 360;
		} else if (rz < 0) {
			rz += 360;
		}
#ifdef SKY_FPV_CURSOR
		if (have_mouse) camera_rotation_frames = 1;
#endif /* SKY_FPV_CURSOR */
		camera_rotation_frames--;
		adjust_view++;
	}
	if(camera_x_frames){
		if(camera_x_speed>0.005 || camera_x_speed<-0.005){
			camera_x-=camera_x_speed;
			if(fabs(camera_x-old_camera_x) >= c_delta){
				adjust_view++;
			}
		}
		camera_x_frames--;
	}
	if(camera_y_frames){
		if(camera_y_speed>0.0005 || camera_y_speed<-0.005){
			camera_y-=camera_y_speed;
			if(fabs(camera_y-old_camera_y) >= c_delta){
				adjust_view++;
			}
		}
		camera_y_frames--;
	}
	if(camera_z_frames){
		if(camera_z_speed>0.0005 || camera_z_speed<-0.005){
			camera_z-=camera_z_speed;
			if(fabs(camera_z-old_camera_z) >= c_delta){
				adjust_view++;
			}
		}
		camera_z_frames--;
	}

	if(camera_tilt_frames) {
#ifndef SKY_FPV_CURSOR
		if(camera_tilt_speed<0) {
			if(rx>-60)rx+=camera_tilt_speed;
			if(rx<-60) {
				rx=-60;
				camera_tilt_frames=0;
			} else
				camera_tilt_frames--;
#else /* SKY_FPV_CURSOR */
		int rx_min = ext_cam?(first_person?-180:-100):-60;
		if(rx>rx_min){
			rx+=camera_tilt_speed;
		}
		if(rx<rx_min) {
			rx=rx_min;
			camera_tilt_frames=0;
#endif /* SKY_FPV_CURSOR */
		} else {
#ifndef SKY_FPV_CURSOR
			if(rx<-45)rx+=camera_tilt_speed;
			if(rx>-45) {
				rx=-45;
				camera_tilt_frames=0;
			} else
				camera_tilt_frames--;
#else /* SKY_FPV_CURSOR */
			if (have_mouse) camera_tilt_frames=1;
			camera_tilt_frames--;
#endif /* SKY_FPV_CURSOR */
		}
	}
	if(camera_zoom_frames) {
		if(camera_zoom_dir == 1) {
			if(new_zoom_level<3.75f){
				new_zoom_level+=0.05f;
				camera_zoom_frames--;
				adjust_view++;
			} else {
				camera_zoom_frames = 0;
			}
		} else {
			if(new_zoom_level>1.00f){
				new_zoom_level-=0.05f;
				camera_zoom_frames--;
				adjust_view++;
			} else {
				camera_zoom_frames = 0;
			}
		}
	}
#ifdef SKY_FPV_CURSOR
	if(first_person){
		if(rx <= -105){
			rx = -104;
		} else if(rx >= -15){
			rx = -16;
		}
	} else {
		if(rx <= -60){
			rx = -59;
		} else if(rx >= -45){
			rx = -46;
		}
	}

#endif /* SKY_FPV_CURSOR */
	if(adjust_view){
		set_all_intersect_update_needed(main_bbox_tree);
		old_camera_x= camera_x;
		old_camera_y= camera_y;
		old_camera_z= camera_z;
	}
	if(zoom_level<1.00f) {
		new_zoom_level=zoom_level=1.00f;
		resize_root_window();
	}
#ifdef SKY_FPV_CURSOR
	hold_camera=rz;
	if (fol_cam){
		if (last_kludge != camera_kludge) {
			set_all_intersect_update_needed(main_bbox_tree);
			adjust = (camera_kludge-last_kludge);

			//without this the camera will zip the wrong way when camera_kludge
			//flips from 180 <-> -180
			if      (adjust >=  180) adjust -= 360.0;
			else if (adjust <= -180) adjust += 360.0;

			if (fabs(adjust) < fol_strn){
				last_kludge=camera_kludge;
			} else {
				last_kludge += fol_strn*(
					adjust*(fol_quad*fol_strn + fol_lin)+
					fol_con*(adjust>0?1:-1))/
					(fol_quad+fol_lin+fol_con+.000001f);//cheap no/0
			}
		}
		rz-=last_kludge;
	}

	//Make Character Turn with Camera
	if (have_mouse)
	{
		adjust = (rz+180);
		//without this the character will turn the wrong way when camera_kludge
		//and character are in certain positions
		if      (adjust >=  180) adjust -= 360.0;
		else if (adjust <= -180) adjust += 360.0;
		adjust+=camera_kludge;
		if      (adjust >=  180) adjust -= 360.0;
		else if (adjust <= -180) adjust += 360.0;
		if (adjust > 35){
			Uint8 str[2];
			str[0] = TURN_LEFT;
			my_tcp_send (my_socket, str, 1);
		} else if (adjust < -35){
			Uint8 str[2];
			str[0] = TURN_RIGHT;
			my_tcp_send (my_socket, str, 1);
		}
	}
	adjust_view = 0;
#endif /* SKY_FPV_CURSOR */
}

int update_have_display(window_info * win) {
	// if the calling window is shown, we have a display, else check all 3d windows
	have_display = (win->displayed || get_show_window(game_root_win) || get_show_window(newchar_root_win));
	return 0;
}
