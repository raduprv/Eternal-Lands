#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "draw_scene.h"
#include "bbox_tree.h"
#include "cal.h"
#include "console.h"
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
#include "pm_log.h"
#include "shadows.h"
#include "skeletons.h"
#include "sky.h"
#include "sound.h"
#include "storage.h"
#include "text.h"
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
#define MAX(a,b) ( ((a)>(b)) ? (a):(b))
//First Person Camera mode state
int first_person = 0;
float old_rx=-60;
float old_rz=45;
float old_zoom_level=3.0;
float max_zoom_level=4.0f;

float fine_camera_rotation_speed;
float normal_camera_rotation_speed;

float camera_rotation_speed;
int camera_rotation_duration;

float camera_tilt_speed;
int camera_tilt_duration;

float normal_camera_deceleration = 0.2;
float camera_rotation_deceleration;
float camera_tilt_deceleration;

double camera_x_speed;
int camera_x_duration;

double camera_y_speed;
int camera_y_duration;

double camera_z_speed;
int camera_z_duration;

int camera_zoom_dir;
int camera_zoom_duration=0;
int camera_zoom_speed=1;

float new_zoom_level=3.0f;
float camera_distance = 2.5f;

int reset_camera_at_next_update = 1;

//Follow camera state stuff
int fol_cam = 1;	       // follow camera state (on/off)
int fol_cam_behind = 0;    // keep the camera behind the char
float camera_kludge = 0.0; // the direction player is facing
float last_kludge = 0.0;   // how far the camera deviated from camera_kludge
float fol_strn = 0.1;      // follow camera response strength
float fol_con = 7.0;       // follow camera constant speed
float fol_lin = 1.0;       // follow camera linear deceleration
float fol_quad = 1.0;      // follow camera quadratic deceleration
int ext_cam = 1;	       // extended camera state (on/off)
int ext_cam_auto_zoom = 0; // auto zooming state for extended camera (on/off)
float min_tilt_angle = 30.0; // minimum tilt angle for the extended camera
float max_tilt_angle = 90.0; // maximum tilt angle for the extended camera
float hold_camera = 0.0;   // backup of the rz value before kludge is applied

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

	glClearColor(skybox_fog_color[0], skybox_fog_color[1], skybox_fog_color[2], 0.0);

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
	draw_special_cursors();

	Leave2DMode ();

	if(elwin_mouse >= 0)
	{
		if (current_cursor != elwin_mouse) change_cursor(elwin_mouse);
		elwin_mouse = -1;
	}
	
	SDL_GL_SwapBuffers();
	CHECK_GL_ERRORS();

	/* stuff to do not every frame, twice a second is fine */
	{
		static Uint32 last_half_second_timer = 0;
		static int first_time = 1;
		Uint32 current_time = SDL_GetTicks();
		if (first_time)
		{
			last_half_second_timer = current_time;
			first_time = 0;
		}
		if ((current_time - last_half_second_timer) > 500u)
		{
			/* start or stop the harvesting effect depending on harvesting state */
			check_harvesting_effect();
			/* check for and possibly do auto save */
			auto_save_local_and_server();
			/* action on afk state changes */
			check_afk_state();
			/* the timer in the hud */
			update_hud_timer();
			/* until next time */
			last_half_second_timer = current_time;
		}
	}

	if (draw_delay > 0)
	{
		SDL_Delay (draw_delay);
		draw_delay = 0;
	}
}

void move_camera ()
{
	float x, y, z;
	// float head_pos[3];
	float follow_speed;
	actor *me = get_our_actor ();

    if(!me){
		return;
	}

	x = (float)me->x_pos+0.25f;
	y = (float)me->y_pos+0.25f;

    // cal_get_actor_bone_local_position(me, get_actor_bone_id(me, head_bone), NULL, head_pos);

    /* Schmurk: I've commented this out because I don't see why the position of
     * the camera should be different from the head position in ext cam and fpv */
/* 	if (first_person){ */
/* 		z = (ext_cam?-1.7f:-2.1f) + height_map[me->y_tile_pos*tile_map_size_x*6+me->x_tile_pos]*0.2f + head_pos[2]; */
/* 	} else if (ext_cam){ */
/* 		z = -1.6f + height_map[me->y_tile_pos*tile_map_size_x*6+me->x_tile_pos]*0.2f + head_pos[2]; */
	if (first_person || ext_cam) {
        // the camera position corresponds to the head position
		z = get_tile_height(me->x_tile_pos, me->y_tile_pos);
		// z += (head_pos[2]+0.1)*get_actor_scale(me);
		
		//attachment_props *att_props = get_attachment_props_if_held(me);
		//z += (me->sitting ? 0.7 : 1.5) * get_actor_scale(me);
		if (me->attached_actor>=0) z+=me->z_pos + me->attachment_shift[Z]+2.0*get_actor_scale(me);
		else z += (me->sitting ? 0.7 : 1.5) * get_actor_scale(me);
	} else {
		z = get_tile_height(me->x_tile_pos, me->y_tile_pos) + sitting;
	}

	if(first_person||ext_cam){
		follow_speed = 150.0f;
	} else {
		follow_speed = 300.0f;
	}
	if(reset_camera_at_next_update){
		camera_x = -x;
		camera_y = -y;
		camera_z = -z;
		camera_x_duration=0;
		camera_y_duration=0;
		camera_z_duration=0;
        reset_camera_at_next_update = 0;
		set_all_intersect_update_needed(main_bbox_tree);
	} else {
		//move near the actor, but smoothly
		camera_x_speed=(x+camera_x)/follow_speed;
		camera_x_duration=follow_speed;
		camera_y_speed=(y+camera_y)/follow_speed;
		camera_y_duration=follow_speed;
		camera_z_speed=(z+camera_z)/follow_speed;
		camera_z_duration=follow_speed;		
	}


	if (first_person){
		// glTranslatef(head_pos[0], head_pos[1], 0.0);
	} else {
		glTranslatef(0.0f, 0.0f, -zoom_level*camera_distance);
	}

	glRotatef(rx, 1.0f, 0.0f, 0.0f);
	glRotatef(rz, 0.0f, 0.0f, 1.0f);
	glTranslatef(camera_x, camera_y, camera_z);
}



void clamp_camera(void)
{
		if(first_person){
			if(rx < -170){
				rx = -170;
				camera_tilt_duration=0;
				camera_tilt_speed = 0.0;
			} else if(rx > -30){
				rx = -30;
				camera_tilt_duration=0;
				camera_tilt_speed = 0.0;
			}
		} else if(ext_cam){
			if(rx < -max_tilt_angle){
				rx = -max_tilt_angle;
				camera_tilt_duration=0;
				camera_tilt_speed = 0.0;
			} else if(rx > -min_tilt_angle){
				rx = -min_tilt_angle;
				camera_tilt_duration=0;
				camera_tilt_speed = 0.0;
			}			
	} else {
		if(rx < -60){
			rx = -60;
			camera_tilt_duration=0;
			camera_tilt_speed = 0.0;
		} else if(rx > -45){
			rx = -45;
			camera_tilt_duration=0;
			camera_tilt_speed = 0.0;
		}
	}
	if (have_mouse){
		camera_rotation_duration = 0;
		camera_tilt_duration=0;
		camera_rotation_speed = 0.0;
		camera_tilt_speed = 0.0;
	}
	if(rz > 360) {
		rz -= 360;
	} else if (rz < 0) {
		rz += 360;
	}
	if(new_zoom_level > max_zoom_level){
		new_zoom_level = max_zoom_level;
		camera_zoom_duration = 0;
	} else if(new_zoom_level < 0.5f) {
		new_zoom_level = 0.5f;
		camera_zoom_duration = 0;
	}
}



int adjust_view;

void update_camera()
{
	const float c_delta = 0.1f;

	static int last_update = 0;
	int time_diff = cur_time - last_update;

	static float old_camera_x = 0;
	static float old_camera_y = 0;
	static float old_camera_z = 0;
	float adjust;
	actor *me = get_our_actor();

	old_rx=rx;
	old_rz=rz;
	new_zoom_level=old_zoom_level=zoom_level;
	
	//printf("kludge: %f, hold: %f, rx: %f, rz %f, zoom: %f\n",camera_kludge, hold_camera,rx,rz,zoom_level);
	
	if (fol_cam && !fol_cam_behind)
		rz = hold_camera;
	if (me)
		camera_kludge = -me->z_rot;

	/* This is a BIG hack to not polluate the code but if this feature
	 * is accepted and the flag is removed, all the code that
	 * follows will have to be changed in order to get rid of
	 * camera_rotation_duration and camera_tilt_duration. */
	camera_rotation_duration = camera_rotation_speed != 0.0 ? time_diff : 0.0;
	camera_tilt_duration = camera_tilt_speed != 0.0 ? time_diff : 0.0;

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
		adjust_view++;
	}
	if(camera_zoom_duration > 0) {
		if (time_diff <= camera_zoom_duration)
			new_zoom_level += camera_zoom_speed*(camera_zoom_dir==1?0.003f:-0.003f)*time_diff;
		else
			new_zoom_level += camera_zoom_speed*(camera_zoom_dir==1?0.003f:-0.003f)*camera_zoom_duration;
		camera_zoom_duration-=time_diff;
		adjust_view++;
	}
	else
		camera_zoom_speed = 1;


	if (camera_rotation_speed > 0.0)
	{
		camera_rotation_speed -= time_diff * camera_rotation_deceleration;
		if (camera_rotation_speed < 0.0)
			camera_rotation_speed = 0.0;
	}
	else if (camera_rotation_speed < 0.0)
	{
		camera_rotation_speed += time_diff * camera_rotation_deceleration;
		if (camera_rotation_speed > 0.0)
			camera_rotation_speed = 0.0;
	}
	if (camera_tilt_speed > 0.0)
	{
		camera_tilt_speed -= time_diff * camera_tilt_deceleration;
		if (camera_tilt_speed < 0.0)
			camera_tilt_speed = 0.0;
	}
	else if (camera_tilt_speed < 0.0)
	{
		camera_tilt_speed += time_diff * camera_tilt_deceleration;
		if (camera_tilt_speed > 0.0)
			camera_tilt_speed = 0.0;
	}

	clamp_camera();

	if (ext_cam && !first_person && me &&
		rx <= -min_tilt_angle && rx >= -max_tilt_angle)
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

		if (get_tile_walkable(tx, ty))
		{
			tz = get_tile_height(tx, ty);
		}
		else
		{
			// if the tile is outside the map, we take the height at the actor position
			tz = get_tile_height(me->x_tile_pos, me->y_tile_pos);
		}
		// here we use a shift of 0.2 to avoid to be too close to the ground
		if (tz + 0.2 > dir[2] - camera_z)
		{
			if (ext_cam_auto_zoom) // new behaviour
			{
				// if the camera is under the ground, we change the zoom level
				if (fabsf(dir[2]) > 1E-4)
					new_zoom_level *= (tz + camera_z + 0.2) / dir[2];
				else
					new_zoom_level = 0.0;

				if (new_zoom_level < 1.0)
				{
					new_zoom_level = 1.0;
					camera_tilt_duration = camera_zoom_duration = 0;
					camera_tilt_speed = 0.0;
					if (fabsf(tz + camera_z + 0.2) < fabsf(vect[2]) - 0.01)
						rx = -90.0 + 180.0 * asinf((tz + camera_z + 0.2) / vect[2]) / M_PI;
				}
				else if (new_zoom_level > old_zoom_level)
				{
					new_zoom_level = old_zoom_level;
					camera_tilt_duration = camera_zoom_duration = 0;
					camera_tilt_speed = 0.0;
				}
			}
			else // old freecam behaviour
			{
				new_zoom_level = old_zoom_level;
				camera_tilt_duration = camera_zoom_duration = 0;
				camera_tilt_speed = 0.0;
				if (fabsf(tz + camera_z + 0.2) < fabsf(vect[2]) - 0.01)
					rx = -90.0 + 180.0 * asinf((tz + camera_z + 0.2) / vect[2]) / M_PI;
			}
		}
	}

	if(adjust_view){
		set_all_intersect_update_needed(main_bbox_tree);
		old_camera_x= camera_x;
		old_camera_y= camera_y;
		old_camera_z= camera_z;
	}

	
	hold_camera = rz;
	if (fol_cam) {
		static int fol_cam_stop = 0;

		if ((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(2)) || camera_rotation_speed != 0)
			fol_cam_stop = 1;
		else if (me && me->moving && fol_cam_stop)
			fol_cam_stop = 0;

		if (last_kludge != camera_kludge && !fol_cam_stop) {
			set_all_intersect_update_needed(main_bbox_tree);
			adjust = (camera_kludge-last_kludge);

			//without this the camera will zip the wrong way when camera_kludge
			//flips from 180 <-> -180
			if      (adjust >=  180) adjust -= 360.0;
			else if (adjust <= -180) adjust += 360.0;

			if (fabs(adjust) < fol_strn) {
				last_kludge=camera_kludge;
			}
			else {
				last_kludge += fol_strn*(
					adjust*(fol_quad*fol_strn + fol_lin)+
					fol_con*(adjust>0?1:-1))/
					(fol_quad+fol_lin+fol_con+.000001f);//cheap no/0
			}
		}
		if (fol_cam_behind)
        {
            if (!fol_cam_stop)
                rz = -last_kludge;
            else
                last_kludge = -rz;
        }
		else
			rz -= last_kludge;
	}

	//Make Character Turn with Camera
	if (have_mouse && !on_the_move (get_our_actor ()))
	{
		adjust = rz;
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
	last_update = cur_time;
}

int update_have_display(window_info * win)
{
	// if the calling window is shown, we have a display, else check all 3d windows
	have_display = (win->displayed || get_show_window(game_root_win) || get_show_window(newchar_root_win));
	return 0;
}
