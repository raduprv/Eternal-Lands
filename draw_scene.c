#include "global.h"
#include <math.h>

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

int normal_animation_timer=0;

float scene_mouse_x;
float scene_mouse_y;

int last_texture=-2;
int font_text;
int cons_text;
int icons_text;
int open_text;
int login_text;
int ground_detail_text;

int times_FPS_below_3=0;
int main_count=0;
int fps_average=0;
int old_fps_average=0;

float clouds_movement_u=-8;
float clouds_movement_v=-3;
Uint32 last_clear_clouds=0;

GLenum base_unit=GL_TEXTURE0_ARB,detail_unit=GL_TEXTURE1_ARB,shadow_unit=GL_TEXTURE2_ARB;

void draw_scene()
{
	unsigned char str [180];
	int fps;
	int y_line,i;
	int any_reflection=0;
	int mouse_rate;

	check_gl_errors();

	//clear the clouds cache too...
	if(last_clear_clouds+10000<cur_time)clear_clouds_cache();

	if(!shadows_on || !have_stencil)glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	else glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
	
	if(interface_mode!=interface_game)
		{
			if(quickbar_relocatable && quickbar_win)//Hack 
				{
					if(windows_list.window[quickbar_win].cur_x<window_width-hud_x && window_height - windows_list.window[quickbar_win].cur_y>hud_y) windows_list.window[quickbar_win].displayed=0;
				}
			if(interface_mode==interface_console)
				{
					// are we actively drawing things?
					if(SDL_GetAppState()&SDL_APPACTIVE)
						{
							Enter2DMode();
							draw_console_pic(cons_text);
							display_console_text();
							draw_hud_interface();
							SDL_GL_SwapBuffers();
							Leave2DMode();
							check_gl_errors();
						}
					SDL_Delay(20);
					return;
				}
			if(interface_mode==interface_opening)
				{
					Enter2DMode();
					draw_console_pic(cons_text);
					display_console_text();
					SDL_Delay(20);
					SDL_GL_SwapBuffers();
					Leave2DMode();
					check_gl_errors();
					return;
				}
		
			if(interface_mode==interface_log_in)
				{
					Enter2DMode();
					draw_login_screen();
					SDL_Delay(20);
					SDL_GL_SwapBuffers();
					Leave2DMode();
					check_gl_errors();
					return;
				}

			if(interface_mode==interface_new_char)
				{
					Enter2DMode();
					draw_new_char_screen();
					SDL_Delay(20);
					SDL_GL_SwapBuffers();
					Leave2DMode();
					check_gl_errors();
					return;
				}

			if(interface_mode==interface_map)
				{
					// are we actively drawing things?
					if(SDL_GetAppState()&SDL_APPACTIVE)
						{
							Enter2DMode();
							draw_hud_interface();
							Leave2DMode();
							draw_game_map();
							SDL_GL_SwapBuffers();
							check_gl_errors();
						}
					SDL_Delay(20);
					return;			
					check_gl_errors();
				}
		}
	
	if(!have_a_map)return;
	if(yourself==-1)return;//we don't have ourselves
	for(i=0; i<max_actors; i++)
		{
        	if(actors_list[i] && actors_list[i]->actor_id==yourself) break;
		}
	if(i > max_actors) return;//we still don't have ourselves
	main_count++;
	
	if(quickbar_win>0)windows_list.window[quickbar_win].displayed=1;

	if(old_fps_average<5)
		{
			mouse_rate=1;
			read_mouse_now=1;
		}
	else if(old_fps_average<10)
		{
			mouse_rate=3;
		}
	else if(old_fps_average<20)
		{
			mouse_rate=6;
		}
	else if(old_fps_average<30)
		{
			mouse_rate=10;
		}
	else if(old_fps_average<40)
		{
			mouse_rate=15;
		}
	else
		{
			mouse_rate=20;
		}
	if(mouse_rate > mouse_limit)mouse_rate=mouse_limit;
	if(!(main_count%mouse_rate))read_mouse_now=1;
	else read_mouse_now=0;
	reset_under_the_mouse();

	if(new_zoom_level != zoom_level) {
		zoom_level=new_zoom_level;
		resize_window();
	}
	glLoadIdentity();					// Reset The Matrix
	Move();
	save_scene_matrix();

	CalculateFrustum();
	any_reflection=find_reflection();
	check_gl_errors();

	// are we actively drawing things?
	if(SDL_GetAppState()&SDL_APPACTIVE){

		//now, determine the current weather light level
		get_weather_light_level();

		if(!dungeon)draw_global_light();
		else draw_dungeon_light();
		update_scene_lights();
		draw_lights();
		check_gl_errors();

		if(!dungeon && shadows_on && is_day)render_light_view();
		check_gl_errors();

		//check for network data
		get_message_from_server();

		glEnable(GL_FOG);
		if(any_reflection)
			{
			  	if(!dungeon)draw_sky_background();
			  	else draw_dungeon_sky_background();
				check_gl_errors();
				if(show_reflection)display_3d_reflection();
			}
		check_gl_errors();

		//check for network data - reduces resyncs
		get_message_from_server();

		if(!dungeon && shadows_on && is_day)draw_sun_shadowed_scene(any_reflection);
		else {
			glNormal3f(0.0f,0.0f,1.0f);
			if(any_reflection)draw_lake_tiles();
			draw_tile_map();
			check_gl_errors();
			display_2d_objects();
			check_gl_errors();
			anything_under_the_mouse(0, UNDER_MOUSE_NOTHING);
			display_objects();
			display_actors();
		}
		glDisable(GL_FOG);
		check_gl_errors();

		//check for network data - reduces resyncs
		get_message_from_server();
	}	// end of active display check
	else display_actors();	// we need to 'touch' all the actors even if not drawing to avoid problems

	check_gl_errors();

	//check for network data - reduces resyncs
	get_message_from_server();

	// if not active, dont bother drawing any more
	if(!(SDL_GetAppState()&SDL_APPACTIVE))
		{
			SDL_Delay(20);
			return;
		}

	//particles should be last, we have no Z writting
	display_particles();
	check_gl_errors();
	if(is_raining)render_rain();
	check_gl_errors();
	//we do this because we don't want the rain/particles to mess with our cursor

	Enter2DMode();
	//get the FPS, etc
	if((cur_time-last_time))fps=1000/(cur_time-last_time);
	else fps=1000;

	if(main_count%10)fps_average+=fps;
	else
		{
			old_fps_average=fps_average/10;
			fps_average=0;
		}
	if(!no_adjust_shadows)
		{
			if(fps<5)
				{
					times_FPS_below_3++;
					if(times_FPS_below_3>4 && shadows_on)
						{
							shadows_on=0;
							put_colored_text_in_buffer(c_red1,low_framerate_str,-1,0);
							times_FPS_below_3=0;
						}
				}
			else times_FPS_below_3=0;
		}
	if(show_fps)
		{
			sprintf(str, "FPS: %i",old_fps_average);
			glColor3f(1.0f,1.0f,1.0f);
			draw_string(10,0,str,1);
		}

	check_gl_errors();
	if(find_last_lines_time())
		{
			set_font(chat_font);	// switch to the chat font
        	draw_string_zoomed(10,20,&display_text_buffer[display_text_buffer_first],max_lines_no,chat_zoom);
			set_font(0);	// switch to fixed
		}
	anything_under_the_mouse(0, UNDER_MOUSE_NO_CHANGE);
	check_gl_errors();

	draw_ingame_interface();
	check_gl_errors();
	//print the text line we are currently writting (if any)
	y_line=window_height-(17*(4+input_text_lines));
	glColor3f(1.0f,1.0f,1.0f);
	draw_string(10,y_line,input_text_line,input_text_lines);

	Leave2DMode();
	glEnable(GL_LIGHTING);

	check_cursor_change();

	SDL_GL_SwapBuffers();
	check_gl_errors();

}

void Move()
{
    int i;
	for(i=0;i<max_actors;i++)
		{
			if(actors_list[i] && actors_list[i]->actor_id==yourself)
				{
					float x=actors_list[i]->x_pos;
					float y=actors_list[i]->y_pos;
					float z=-2.2f+height_map[actors_list[i]->y_tile_pos*tile_map_size_x*6+actors_list[i]->x_tile_pos]*0.2f;
					//move near the actor, but smoothly
					camera_x_speed=(x-(-cx))/16.0;
					camera_x_frames=16;
					camera_y_speed=(y-(-cy))/16.0;
					camera_y_frames=16;
					camera_z_speed=(z-(-cz))/16.0;
					camera_z_frames=16;
					break;
				}
		}
    //check to see if we are out of the map
    if(cx>-7.5f)cx=-7.5f;
    if(cy>-7.5f)cy=-7.5f;
    if(cx<-(tile_map_size_x*3-7.9))cx=(float)-(tile_map_size_x*3-7.9);
    if(cy<-(tile_map_size_x*3-7.9))cy=(float)-(tile_map_size_x*3-7.9);

	glRotatef(rx, 1.0f, 0.0f, 0.0f);
	glRotatef(rz, 0.0f, 0.0f, 1.0f);
	glTranslatef(cx, cy, cz);

	//test only
	update_position();

}

void update_camera()
{
	if(camera_rotation_frames)
		{
			rz+=camera_rotation_speed;
			camera_rotation_frames--;
		}
	if(camera_x_frames)
		{
			if(camera_x_speed>0.005 || camera_x_speed<-0.005)
				cx-=camera_x_speed;
			camera_x_frames--;
		}
	if(camera_y_frames)
		{
			if(camera_y_speed>0.0005 || camera_y_speed<-0.005)
				cy-=camera_y_speed;
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
			} else 
				camera_zoom_frames = 0;
		} else {
			if(zoom_level>1.75f){
				new_zoom_level-=0.05f;
				camera_zoom_frames--;
			} else 
				camera_zoom_frames = 0;
		}
	}

}

#define	TIMER_RATE 20
int	my_timer_adjust=0;
int	my_timer_clock=0;
Uint32 my_timer(unsigned int some_int)
{
	int	new_time;
	SDL_Event e;

	// adjust the timer clock
	if(my_timer_clock == 0)my_timer_clock=SDL_GetTicks();
	else	my_timer_clock+=(TIMER_RATE-my_timer_adjust);
	
	e.type = SDL_USEREVENT;
	e.user.code = EVENT_UPDATE_CAMERA;
	SDL_PushEvent(&e);

	//check the thunders
	thunder_control();
	//check the rain
	rain_control();

	if(is_raining)update_rain();
	if(normal_animation_timer>2)
		{
			if(my_timer_adjust > 0)my_timer_adjust--;
			normal_animation_timer=0;
    		update_particles();
    		next_command();

		e.type = SDL_USEREVENT;
		e.user.code = EVENT_ANIMATE_ACTORS;
		SDL_PushEvent(&e);
#ifdef CAL3D
			update_cal3d_model();
#endif
    		move_to_next_frame();
    		if(lake_waves_timer>2)
    		    {
    		        lake_waves_timer=0;
    		        make_lake_water_noise();
    		    }
    		lake_waves_timer++;
    		water_movement_u+=0.0004f;
    		water_movement_v+=0.0002f;
    		if(!dungeon && 0)//we do not want clouds movement in dungeons, but we want clouds detail
    			{
    				clouds_movement_u+=0.0003f;
    				clouds_movement_v+=0.0006f;
				}
		}
	normal_animation_timer++;

	// find the new interval
	new_time=TIMER_RATE-(SDL_GetTicks()-my_timer_clock);
	if(new_time<10)	new_time=10;	//put an absoute minimume in
    return new_time;
}
