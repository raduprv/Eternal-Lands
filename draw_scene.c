#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "draw_scene.h"
#include "bbox_tree.h"
#include "cal.h"
#include "cursors.h"
#include "elwindows.h"
#include "gamewin.h"
#include "gl_init.h"
#include "global.h"
#include "hud.h"
#include "interface.h"
#include "items.h"
#include "map.h"
#include "multiplayer.h"
#include "new_actors.h"
#include "new_character.h"
#include "shadows.h"
#include "skeletons.h"
#include "sound.h"
#include "storage.h"
#include "tiles.h"
#include "weather.h"

static char have_display = 0;

float camera_x=0;
float camera_y=0;
float camera_z=0;
float rx=-60;
float ry=0;
float rz=45;
float terrain_scale=2.0f;
float zoom_level=3.0f;
float name_zoom=1.0f;
#ifdef SKY_FPV
#define MAX(a,b) ( ((a)>(b)) ? (a):(b))
//First Person Camera mode state
int first_person = 0;
float old_rx=-60;
float old_rz=45;
float old_zoom_level=3.0;
#endif // SKY_FPV

float fine_camera_rotation_speed;
float normal_camera_rotation_speed;

#ifndef NEW_CAMERA
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
#else // NEW_CAMERA
float camera_rotation_speed;
int camera_rotation_duration;

float camera_tilt_speed;
int camera_tilt_duration;

double camera_x_speed;
int camera_x_duration;

double camera_y_speed;
int camera_y_duration;

double camera_z_speed;
int camera_z_duration;

int camera_zoom_dir;
int camera_zoom_duration=0;
#endif
float new_zoom_level=3.0f;
float camera_distance = 2.5f;

#ifdef NEW_ACTOR_MOVEMENT
int reset_camera_at_next_update = 1;
#endif // NEW_ACTOR_MOVEMENT

#ifdef SKY_FPV
//Follow camera state stuff
int fol_cam = 1;	//On or off...
float camera_kludge = 0.0f;	//Stores direction player is facing
float last_kludge=0;	//Stores how far the camera deviated from camera_kludge
float fol_strn = 0.03f;
float fol_con,fol_lin,fol_quad;
int ext_cam = 1;	//Extended camera state
int auto_camera_zoom = 0;
//Check that this is still used, I think it's not.
float hold_camera=180;

#endif // SKY_FPV
int last_texture=-2;
int cons_text;
int icons_text;

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
	if (dungeon || !use_fog) {
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
#ifdef NEW_CURSOR
	draw_cursor();
#endif // NEW_CURSOR

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

#ifndef NEW_ACTOR_MOVEMENT
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
#endif // NEW_ACTOR_MOVEMENT

void move_camera ()
{
	float x, y, z;
#ifdef SKY_FPV
	float head_pos[3], follow_speed;
#endif // SKY_FPV
#ifndef NEW_ACTOR_MOVEMENT
	static int lagged=1;
#endif // NEW_ACTOR_MOVEMENT
	actor *me = get_our_actor ();
	
#ifndef NEW_ACTOR_MOVEMENT
	if(!me || !me->tmp.have_tmp){
		lagged=1;
		return;
	}

	x = (float)me->tmp.x_pos+0.25f;
	y = (float)me->tmp.y_pos+0.25f;

#ifndef SKY_FPV
	z=-2.2f+height_map[me->tmp.y_tile_pos*tile_map_size_x*6+me->tmp.x_tile_pos]*0.2f+sitting;
#else // SKY_FPV

    cal_get_actor_bone_local_position(me, get_actor_bone_id(me, head_bone), NULL, head_pos);

    /* Schmurk: I've commented this out because I don't see why the position of
     * the camera should be different from the head position in ext cam and fpv */
/* 	if (first_person){ */
/* 		z = (ext_cam?-1.7f:-2.1f) + height_map[me->tmp.y_tile_pos*tile_map_size_x*6+me->tmp.x_tile_pos]*0.2f + head_pos[2]; */
/* 	} else if (ext_cam){ */
/* 		z = -1.6f + height_map[me->tmp.y_tile_pos*tile_map_size_x*6+me->tmp.x_tile_pos]*0.2f + head_pos[2]; */
	if (first_person || ext_cam) {
        // the camera position corresponds to the head position
		z = -2.2f + height_map[me->tmp.y_tile_pos*tile_map_size_x*6+me->tmp.x_tile_pos]*0.2f + head_pos[2];
	} else {
		z = -2.2f + height_map[me->tmp.y_tile_pos*tile_map_size_x*6+me->tmp.x_tile_pos]*0.2f + sitting;
	}
#endif // SKY_FPV
#else // NEW_ACTOR_MOVEMENT
    if(!me){
		return;
	}

	x = (float)me->x_pos+0.25f;
	y = (float)me->y_pos+0.25f;

#ifndef SKY_FPV
	z=-2.2f+height_map[me->y_tile_pos*tile_map_size_x*6+me->x_tile_pos]*0.2f+sitting;
#else // SKY_FPV

    cal_get_actor_bone_local_position(me, get_actor_bone_id(me, head_bone), NULL, head_pos);

    /* Schmurk: I've commented this out because I don't see why the position of
     * the camera should be different from the head position in ext cam and fpv */
/* 	if (first_person){ */
/* 		z = (ext_cam?-1.7f:-2.1f) + height_map[me->y_tile_pos*tile_map_size_x*6+me->x_tile_pos]*0.2f + head_pos[2]; */
/* 	} else if (ext_cam){ */
/* 		z = -1.6f + height_map[me->y_tile_pos*tile_map_size_x*6+me->x_tile_pos]*0.2f + head_pos[2]; */
	if (first_person || ext_cam) {
        // the camera position corresponds to the head position
		z = -2.2f + height_map[me->y_tile_pos*tile_map_size_x*6+me->x_tile_pos]*0.2f + head_pos[2];
	} else {
		z = -2.2f + height_map[me->y_tile_pos*tile_map_size_x*6+me->x_tile_pos]*0.2f + sitting;
	}
#endif // SKY_FPV
#endif // NEW_ACTOR_MOVEMENT

#ifdef SKY_FPV
	if(first_person||ext_cam){
#ifndef NEW_CAMERA
		follow_speed = 8.0f;
#else
		follow_speed = 150.0f;
#endif
	} else {
#ifndef NEW_CAMERA
		follow_speed = 16.0f;
#else
		follow_speed = 300.0f;
#endif
	}
#endif // SKY_FPV
#ifndef NEW_ACTOR_MOVEMENT
	if(lagged){
#else // NEW_ACTOR_MOVEMENT
	if(reset_camera_at_next_update){
#endif // NEW_ACTOR_MOVEMENT
		camera_x = -x;
		camera_y = -y;
		camera_z = -z;
#ifndef NEW_CAMERA
		camera_x_frames=0;
		camera_y_frames=0;
		camera_z_frames=0;
#else // NEW_CAMERA
		camera_x_duration=0;
		camera_y_duration=0;
		camera_z_duration=0;
#endif // NEW_CAMERA
#ifndef NEW_ACTOR_MOVEMENT
		lagged=0;
#else // NEW_ACTOR_MOVEMENT
        reset_camera_at_next_update = 0;
#endif // NEW_ACTOR_MOVEMENT
		set_all_intersect_update_needed(main_bbox_tree);
	} else {
		//move near the actor, but smoothly
#ifndef SKY_FPV
#ifndef NEW_CAMERA
		camera_x_speed=(x-(-camera_x))/16.0;
		camera_x_frames=16;
		camera_y_speed=(y-(-camera_y))/16.0;
		camera_y_frames=16;
		camera_z_speed=(z-(-camera_z))/16.0;
		camera_z_frames=16;
#else // NEW_CAMERA
		camera_x_speed=(x+camera_x)/300.0;
		camera_x_duration=300;
		camera_y_speed=(y+camera_y)/300.0;
		camera_y_duration=300;
		camera_z_speed=(z+camera_z)/300.0;
		camera_z_duration=300;
#endif // NEW_CAMERA
	}

	glTranslatef(0.0f, 0.0f, -zoom_level*camera_distance);
	glRotatef(rx, 1.0f, 0.0f, 0.0f);
	glRotatef(rz, 0.0f, 0.0f, 1.0f);
	glTranslatef(camera_x,camera_y, camera_z);

#else // SKY_FPV
#ifndef NEW_CAMERA
		camera_x_speed=(x+camera_x)/follow_speed;
		camera_x_frames=follow_speed;
		camera_y_speed=(y+camera_y)/follow_speed;
		camera_y_frames=follow_speed;
		camera_z_speed=(z+camera_z)/follow_speed;
		camera_z_frames=follow_speed;
#else //NEW_CAMERA
		camera_x_speed=(x+camera_x)/follow_speed;
		camera_x_duration=follow_speed;
		camera_y_speed=(y+camera_y)/follow_speed;
		camera_y_duration=follow_speed;
		camera_z_speed=(z+camera_z)/follow_speed;
		camera_z_duration=follow_speed;		
#endif
	}


	if (first_person){
		glTranslatef(head_pos[0], head_pos[1], 0.0);
	} else {
		glTranslatef(0.0f, 0.0f, -zoom_level*camera_distance);
	}

	glRotatef(rx, 1.0f, 0.0f, 0.0f);
	glRotatef(rz, 0.0f, 0.0f, 1.0f);
	glTranslatef(camera_x, camera_y, camera_z);
#endif // SKY_FPV

#ifndef NEW_SOUND	//test only
	update_position();
#endif	//NEW_SOUND
}



void clamp_camera(void)
{
#ifdef SKY_FPV
#ifndef NEW_CAMERA
	if(first_person){
		if(rx < -140){
			rx = -140;
			camera_tilt_frames=0;
		} else if(rx > 15){
			rx = 15;
			camera_tilt_frames=0;
		}
	} else if(ext_cam){
		if(rx < -90){
			rx = -90;
			camera_tilt_frames=0;
		} else if(rx > -15){
			rx = -15;
			camera_tilt_frames=0;
		}
#else // NEW CAMERA
		if(first_person){
			if(rx < -170){
				rx = -170;
				camera_tilt_duration=0;
			} else if(rx > -30){
				rx = -30;
				camera_tilt_duration=0;
			}
		} else if(ext_cam){
			if(rx < -150){
				rx = -150;
				camera_tilt_duration=0;
			} else if(rx > -20){
				rx = -20;
				camera_tilt_duration=0;
			}			
#endif // NEW_CAMERA
	} else {
#endif // SKY_FPV
#ifndef NEW_CAMERA
		if(rx < -60){
			rx = -60;
			camera_tilt_frames=0;
		} else if(rx > -45){
			rx = -44;
			camera_tilt_frames=0;
		}
#else // NEW_CAMERA
		if(rx < -60){
			rx = -60;
			camera_tilt_duration=0;
		} else if(rx > -45){
			rx = -45;
			camera_tilt_duration=0;
		}
#endif // NEW_CAMERA
#ifdef SKY_FPV
	}
	if (have_mouse){
#ifndef NEW_CAMERA
		camera_rotation_frames = 0;
		camera_tilt_frames=0;
#else // NEW_CAMERA
		camera_rotation_duration = 0;
		camera_tilt_duration=0;
#endif // NEW_CAMERA
	}
#endif // SKY_FPV
	if(rz > 360) {
		rz -= 360;
	} else if (rz < 0) {
		rz += 360;
	}
#ifndef NEW_CAMERA
	if(new_zoom_level > 4.0f){
		new_zoom_level = 4.0f;
		camera_zoom_frames = 0;
	} else if(new_zoom_level < 1.0f) {
		new_zoom_level = 1.0f;
		camera_zoom_frames = 0;
	}
#else // NEW_CAMERA
	if(new_zoom_level > 4.0f){
		new_zoom_level = 4.0f;
		camera_zoom_duration = 0;
	} else if(new_zoom_level < 1.0f) {
		new_zoom_level = 1.0f;
		camera_zoom_duration = 0;
	}
#endif // NEW_CAMERA
}



#ifdef SKY_FPV
int adjust_view;
#endif // SKY_FPV

void update_camera()
{
	const float c_delta = 0.1f;

#ifdef NEW_CAMERA
	static int last_update = 0;
	int time_diff = cur_time - last_update;
#endif // NEW_CAMERA

	static float old_camera_x = 0;
	static float old_camera_y = 0;
	static float old_camera_z = 0;
#ifndef SKY_FPV
	int adjust_view= 0;
#else // SKY_FPV
	float adjust;
	actor *me = get_our_actor();

	old_rx=rx;
	old_rz=rz;
	new_zoom_level=old_zoom_level=zoom_level;
	
	//printf("kludge: %f, hold: %f, rx: %f, rz %f, zoom: %f\n",camera_kludge, hold_camera,rx,rz,zoom_level);
	
	if (fol_cam) rz=hold_camera;
#ifndef NEW_ACTOR_MOVEMENT
	if (me) camera_kludge=180-me->tmp.z_rot;
#else // NEW_ACTOR_MOVEMENT
	if (me) camera_kludge=180-me->z_rot;
#endif // NEW_ACTOR_MOVEMENT
#endif // SKY_FPV

#ifndef NEW_CAMERA
	if(camera_rotation_frames){
		rz+=camera_rotation_speed;
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
		rx+=camera_tilt_speed;
		camera_tilt_frames--;
	}
	if(camera_zoom_frames) {
		new_zoom_level += (camera_zoom_dir==1?0.05f:-0.05f);
		camera_zoom_frames--;
		adjust_view++;
	}
#else // NEW_CAMERA
	if(camera_rotation_duration > 0){
		if (time_diff <= camera_rotation_duration)
			rz+=camera_rotation_speed*time_diff;
		else
			rz+=camera_rotation_speed*camera_rotation_duration;
		camera_rotation_duration-=time_diff;
		adjust_view++;
	}
	if(camera_x_duration > 0){
		if(camera_x_speed>1E-4 || camera_x_speed<-1E-4){
			if (time_diff <= camera_x_duration)
				camera_x-=camera_x_speed*time_diff;
			else
				camera_x-=camera_x_speed*camera_x_duration;
			if(fabs(camera_x-old_camera_x) >= c_delta){
				adjust_view++;
			}
		}
		camera_x_duration-=time_diff;
	}
	if(camera_y_duration > 0){
		if(camera_y_speed>1E-4 || camera_y_speed<-1E-4){
			if (time_diff <= camera_y_duration)
				camera_y-=camera_y_speed*time_diff;
			else
				camera_y-=camera_y_speed*camera_y_duration;
			if(fabs(camera_y-old_camera_y) >= c_delta){
				adjust_view++;
			}
		}
		camera_y_duration-=time_diff;
	}
	if(camera_z_duration > 0){
		if(camera_z_speed>1E-4 || camera_z_speed<-1E-4){
			if (time_diff <= camera_z_duration)
				camera_z-=camera_z_speed*time_diff;
			else
				camera_z-=camera_z_speed*camera_z_duration;
			if(fabs(camera_z-old_camera_z) >= c_delta){
				adjust_view++;
			}
		}
		camera_z_duration-=time_diff;
	}

	if(camera_tilt_duration > 0) {
		if (time_diff <= camera_tilt_duration)
			rx+=camera_tilt_speed*time_diff;
		else
			rx+=camera_tilt_speed*camera_tilt_duration;
		camera_tilt_duration-=time_diff;
	}
	if(camera_zoom_duration > 0) {
		if (time_diff <= camera_zoom_duration)
			new_zoom_level += (camera_zoom_dir==1?0.003f:-0.003f)*time_diff;
		else
			new_zoom_level += (camera_zoom_dir==1?0.003f:-0.003f)*camera_zoom_duration;
		camera_zoom_duration-=time_diff;
		adjust_view++;
	}
#endif // NEW_CAMERA

#ifdef SKY_FPV
	if (ext_cam && !first_person && me)
	{
		float rot_x[9], rot_z[9], rot[9], dir[3];
		float vect[3] = {0.0, 0.0, new_zoom_level*camera_distance};
		int tx, ty;
		float tz;

		// we compute the camera position
		MAT3_ROT_X(rot_x, -rx*M_PI/180.0);
		MAT3_ROT_Z(rot_z, -rz*M_PI/180.0);
		MAT3_MULT(rot, rot_z, rot_x);
		MAT3_VECT3_MULT(dir, rot, vect);

		// we take the tile where the camera is
		tx = (int)((dir[0] - camera_x)*2);
		ty = (int)((dir[1] - camera_y)*2);

		if (tx >= 0 && tx < tile_map_size_x*6 &&
			ty >= 0 && ty < tile_map_size_y*6)
		{
			tz = height_map[ty*tile_map_size_x*6+tx]*0.2 - 2.2;
			if (tz <= -2.2)
			{
				// if the tile is not walkable, we take the height of the actor
				tz = height_map[me->y_tile_pos*tile_map_size_x*6+me->x_tile_pos]*0.2 - 2.2;
			}
		}
		else
		{
			// if the tile is outside of the map, we take the hight of the actor
			tz = height_map[me->y_tile_pos*tile_map_size_x*6+me->x_tile_pos]*0.2 - 2.2;
		}
		// here we use a shift of 0.2 to avoid to be too close from the ground
		if (tz + 0.2 > dir[2] - camera_z)
		{
			if (auto_camera_zoom) // new behaviour
			{
				// if the camera is under the ground, we change the zoom level
				new_zoom_level *= (tz + camera_z + 0.2) / dir[2];

				if (new_zoom_level < 1.0)
				{
					new_zoom_level = 1.0;
#ifdef NEW_CAMERA
					camera_tilt_duration = camera_zoom_duration = 0;
#else // NEW_CAMERA
					camera_tilt_frames = camera_zoom_frames = 0;
#endif // NEW_CAMERA
					rx = -90.0 + 180.0 * asinf((tz + camera_z + 0.2) / vect[2]) / M_PI;
				}
				else if (new_zoom_level > 4.0)
				{
					new_zoom_level = 4.0;
#ifdef NEW_CAMERA
					camera_tilt_duration = camera_zoom_duration = 0;
#else // NEW_CAMERA
					camera_tilt_frames = camera_zoom_frames = 0;
#endif // NEW_CAMERA
				}
			}
			else // old freecam behaviour
			{
				rx = old_rx;
				new_zoom_level = old_zoom_level;
#ifdef NEW_CAMERA
				camera_tilt_duration = camera_zoom_duration = 0;
#else // NEW_CAMERA
				camera_tilt_frames = camera_zoom_frames = 0;
#endif // NEW_CAMERA
			}
		}
	}
#endif // SKY_FPV

	clamp_camera();
	if(adjust_view){
		set_all_intersect_update_needed(main_bbox_tree);
		old_camera_x= camera_x;
		old_camera_y= camera_y;
		old_camera_z= camera_z;
	}

#ifdef SKY_FPV
	
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
	if (have_mouse && !on_the_move (get_our_actor ()))
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
#endif // SKY_FPV

#ifdef NEW_CAMERA
	last_update = cur_time;
#endif // NEW_CAMERA
}

int update_have_display(window_info * win)
{
	// if the calling window is shown, we have a display, else check all 3d windows
	have_display = (win->displayed || get_show_window(game_root_win) || get_show_window(newchar_root_win));
	return 0;
}
