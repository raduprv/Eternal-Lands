#include "global.h"
#include <math.h>
void Move();

int times_FPS_below_3=0;
int main_count=0;
int fps_average=0;
int old_fps_average=0;

void draw_scene()
{
        unsigned char str [180];
        int fps;
        int y_line,i;
        Uint8 status;
        int any_reflection=0;

        cur_time = SDL_GetTicks();
        get_message_from_server();

        //debug
        triangles_normal=0;

	//should we send the heart beat?
	if(last_heart_beat+25000<cur_time)
		{
			Uint8 command;
			last_heart_beat=cur_time;
			command=HEART_BEAT;
			my_tcp_send(my_socket,&command,1);
		}
	//clear the clouds cache too...
	if(last_clear_clouds+10000<cur_time)clear_clouds_cache();


        if(!shadows_on || !have_stencil)glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
        else glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

		if(interface_mode==interface_console)
		      {
		        Enter2DMode();
                glClear(GL_COLOR_BUFFER_BIT);
                draw_console_pic(cons_text);
                display_console_text();
                SDL_Delay(20);
                SDL_GL_SwapBuffers();
                Leave2DMode();
                return;
              }
		if(interface_mode==interface_opening)
		      {
		        Enter2DMode();
                glClear(GL_COLOR_BUFFER_BIT);
				draw_console_pic(cons_text);
    			display_console_text();
                SDL_Delay(20);
                SDL_GL_SwapBuffers();
                Leave2DMode();
                return;
              }

		if(interface_mode==interface_log_in)
		      {
		        Enter2DMode();
                glClear(GL_COLOR_BUFFER_BIT);
                draw_login_screen();
                SDL_Delay(20);
                SDL_GL_SwapBuffers();
                Leave2DMode();
                return;
              }

		if(interface_mode==interface_new_char)
		      {
		        Enter2DMode();
                glClear(GL_COLOR_BUFFER_BIT);
                draw_new_char_screen();
                SDL_Delay(20);
                SDL_GL_SwapBuffers();
                Leave2DMode();
                return;
              }


		if(interface_mode==interface_map)
		      {
                glClear(GL_COLOR_BUFFER_BIT);
                draw_game_map();
                SDL_Delay(20);
                SDL_GL_SwapBuffers();
                return;
              }


        if(!have_a_map)return;
        if(yourself==-1)return;//we don't have ourselves
        for(i=0;i<500;i++)
        if(actors_list[i])
        if(actors_list[i]->actor_id==yourself)break;
        if(i>499)return;//we still don't have ourselves
        main_count++;

        if(fps<5)
        	{
        		read_mouse_now=1;
			}
        else if(fps<10)
        	{
        		if(!(main_count%2))read_mouse_now=1;
        		else read_mouse_now=0;
			}
        else if(fps<20)
        	{
        		if(!(main_count%4))read_mouse_now=1;
        		else read_mouse_now=0;
			}
        else if(fps<30)
        	{
        		if(!(main_count%8))read_mouse_now=1;
        		else read_mouse_now=0;
			}
        else if(fps<40)
        	{
        		if(!(main_count%12))read_mouse_now=1;
        		else read_mouse_now=0;
			}
        else
        	{
        		if(!(main_count%15))read_mouse_now=1;
        		else read_mouse_now=0;
			}
        reset_under_the_mouse();

        glLoadIdentity();					// Reset The Matrix
        Move();

        CalculateFrustum();
        any_reflection=find_reflection();

		//now, determine the current weather light level
		get_weather_light_level();

		//if(shadows_on && day_shadows_on)make_shadows_texture();

        if(!dungeon)draw_global_light();
        else draw_dungeon_light();
        update_scene_lights();
        draw_lights();


          if(any_reflection)
            {
                if(!dungeon)draw_sky_background();
                else draw_dungeon_sky_background();
                //render_ligtning();
                glNormal3f(0.0f,0.0f,1.0f);//the normal for ground objects and such points up
                draw_tile_map();
                display_2d_objects();
                if(!poor_man)display_3d_reflection();
                glNormal3f(0.0f,0.0f,1.0f);
                draw_lake_tiles();
            }
           else
           {
            glNormal3f(0.0f,0.0f,1.0f);//the normal for ground objects and such points up
            draw_tile_map();
            display_2d_objects();
           }

          anything_under_the_mouse(0, UNDER_MOUSE_NOTHING);

          if(!dungeon)
            {
              if(shadows_on && have_stencil)if(day_shadows_on)draw_sun_shadowed_scene();
              //if(shadows_on && have_stencil)if(night_shadows_on)draw_night_shadowed_scene();
            }
          if(dungeon)
            {
              //if(shadows_on && have_stencil)draw_night_shadowed_scene();
            }

        if(!shadows_on || !have_stencil || night_shadows_on)display_objects();

          display_actors();

          //particles should be last, we have no Z writting
          display_particles();
          //we do this because we don't want the rain/particles to mess with our cursor
          anything_under_the_mouse(0, UNDER_MOUSE_NO_CHANGE);
          if(is_raining)render_rain();
          //we do this because we don't want the rain/particles to mess with our cursor
          anything_under_the_mouse(0, UNDER_MOUSE_NO_CHANGE);

          //we also need no lightening for our interface
          Enter2DMode();
          //cur_time = SDL_GetTicks();
          //get the FPS, etc
          if((cur_time-last_time))fps=1000/(cur_time-last_time);
          else fps=1000;

          if(main_count%10)fps_average+=fps;
          else
          	{
				old_fps_average=fps_average/10;
          		fps_average=fps;
			}



		  if(!no_adjust_shadows)
          if(fps<5)
          	{
          		times_FPS_below_3++;
          		if(times_FPS_below_3>4 && shadows_on)
          		    {
                        shadows_on=0;
                        str[0]=127+c_red1;
                        my_strcp(&str[1],"Low framerate detected, shadows disabled!");
                        put_text_in_buffer(str,strlen(str),0);
                        times_FPS_below_3=0;
                    }
			}
		  else times_FPS_below_3=0;

          sprintf(str, "FPS: %i",old_fps_average);
          glColor3f(1.0f,1.0f,1.0f);
          draw_string(10,0,str,1);

          //we do this because we don't want the rain/particles to mess with our cursor
          anything_under_the_mouse(0, UNDER_MOUSE_NO_CHANGE);

          if(find_last_lines_time())
          draw_string(10,20,&display_text_buffer[display_text_buffer_first],max_lines_no);
          //we do this because we don't want the rain/particles to mess with our cursor
          anything_under_the_mouse(0, UNDER_MOUSE_NO_CHANGE);

          draw_ingame_interface();
          anything_under_the_mouse(0, UNDER_MOUSE_MENU);
          //print the text line we are currently writting (if any)
          y_line=window_height-(17*6);
          glColor3f(1.0f,1.0f,1.0f);
          draw_string(10,y_line,input_text_line,2);

          Leave2DMode();
          glEnable(GL_LIGHTING);

          check_cursor_change();

          SDL_GL_SwapBuffers();
}

void Move()
{
    int i=0;
	while(i<1000)
		{
			if(actors_list[i])
			if(actors_list[i]->actor_id==yourself)
			 {
			 //the following IF is in case the camera lost the actor like after a teleporting
			    float x=actors_list[i]->x_pos;
			    float y=actors_list[i]->y_pos;
			    float z=-2.2f+height_map[actors_list[i]->y_tile_pos*tile_map_size_x*6+actors_list[i]->x_tile_pos]*0.2f;
			    /*
				if((cx-x)*(cx-x)+(cy-y)*(cy-y)>1800.00)
					{
                		cx=-actors_list[i]->x_pos;
                		cy=-actors_list[i]->y_pos;
					}
				*/
			//move near the actor, but smoothly
			camera_x_speed=(x-(-cx))/16.0;
			camera_x_frames=16;
			camera_y_speed=(y-(-cy))/16.0;
			camera_y_frames=16;
			camera_z_speed=(z-(-cz))/16.0;
			camera_z_frames=16;
                break;
             }
			i++;
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
    check_range_sounds();
	update_positional_sounds();
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

}

Uint32 my_timer(unsigned int some_int)
{
	update_camera();
	//check the thunders
	thunder_control();
	//check the rain
	rain_control();

	if(is_raining)update_rain();
	if(normal_animation_timer>2)
		{
			normal_animation_timer=0;
    		update_particles();
    		update_rain();
    		next_command();
    		animate_actors();
    		//animate_actors();
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

    return some_int;
}
