#include "global.h"

int HandleEvent(SDL_Event *event)
{
	int done=0;
	Uint8 ch;

	    mod_key_status=SDL_GetModState();
		if(mod_key_status&KMOD_SHIFT)shift_on=1;
		else shift_on=0;

	    mod_key_status=SDL_GetModState();
		if(mod_key_status&KMOD_CTRL)ctrl_on=1;
		else ctrl_on=0;

	    mod_key_status=SDL_GetModState();
		if(mod_key_status&KMOD_ALT)alt_on=1;
		else alt_on=0;

	switch( event->type ) {

	    case SDL_KEYDOWN:

		if (event->key.keysym.sym == SDLK_TAB)heights_3d=!heights_3d;
		if (event->key.keysym.sym == SDLK_g)view_grid=!view_grid;

		if ( event->key.keysym.sym == SDLK_ESCAPE )
		{
			done = 1;
		}
		if ( event->key.keysym.sym == SDLK_LEFT )
		{
			if(ctrl_on && cur_mode==mode_3d && selected_3d_object!=-1)
			if(shift_on)objects_list[selected_3d_object]->z_rot-=1.0f;
			else objects_list[selected_3d_object]->z_rot-=10.0f;

			else
			if(ctrl_on && cur_mode==mode_2d && selected_2d_object!=-1)
			if(shift_on)obj_2d_list[selected_2d_object]->z_rot-=1.01f;
			else obj_2d_list[selected_2d_object]->z_rot-=10.0f;

			else
				{
   					cx += sin((rz+90)*3.1415926/180);
					cy += cos((rz+90)*3.1415926/180);
				}
		}
		if ( event->key.keysym.sym == SDLK_RIGHT )
		{
			if(ctrl_on && cur_mode==mode_3d && selected_3d_object!=-1)
			if(shift_on)objects_list[selected_3d_object]->z_rot+=1.0f;
			else objects_list[selected_3d_object]->z_rot+=10.0f;

			else
			if(ctrl_on && cur_mode==mode_2d && selected_2d_object!=-1)
			if(shift_on)obj_2d_list[selected_2d_object]->z_rot+=1.01f;
			else obj_2d_list[selected_2d_object]->z_rot+=10.0f;

			else
				{
   					cx -= sin((rz+90)*3.1415926/180);
					cy -= cos((rz+90)*3.1415926/180);
				}
		}

		if ( event->key.keysym.sym == SDLK_UP)
		{
			if(ctrl_on && cur_mode==mode_3d && selected_3d_object!=-1)
			if(shift_on)objects_list[selected_3d_object]->x_rot-=1.0f;
			else objects_list[selected_3d_object]->x_rot-=10.0f;

			else
			if(ctrl_on && cur_mode==mode_2d && selected_2d_object!=-1)
			if(shift_on)obj_2d_list[selected_2d_object]->x_rot-=1.01f;
			else obj_2d_list[selected_2d_object]->x_rot-=10.0f;

			else
				{
	   				cx -= sin(rz*3.1415926/180);
					cy -= cos(rz*3.1415926/180);
				}

		}

		if ( event->key.keysym.sym == SDLK_DOWN )
		{
			if(ctrl_on && cur_mode==mode_3d && selected_3d_object!=-1)
			if(shift_on)objects_list[selected_3d_object]->x_rot+=1.0f;
			else objects_list[selected_3d_object]->x_rot+=10.0f;

			else
			if(ctrl_on && cur_mode==mode_2d && selected_2d_object!=-1)
			if(shift_on)obj_2d_list[selected_2d_object]->x_rot+=1.01f;
			else obj_2d_list[selected_2d_object]->x_rot+=10.0f;

			else
				{
   					cx += sin(rz*3.1415926/180);
					cy += cos(rz*3.1415926/180);
				}

		}

		if ( event->key.keysym.sym == SDLK_HOME )
		{
   				if(shift_on)rz +=1.0f;
   				else rz +=20.0f;
		}
		if ( event->key.keysym.sym == SDLK_END )
		{
   				if(shift_on)rz -=1.0f;
   				else rz -=20.0f;
		}


		if ( event->key.keysym.sym == SDLK_PAGEUP )
		{
			if(ctrl_on)
				if(zoom_level>2.0f)
					{
						zoom_level-=0.25f;
						resize_window();
					}
				else
					{
						if(cur_mode==mode_3d && selected_3d_object!=-1)
						if(shift_on)objects_list[selected_3d_object]->y_rot-=1.0f;
						else objects_list[selected_3d_object]->y_rot-=10.0f;
					}
		}
		if ( event->key.keysym.sym == SDLK_PAGEDOWN )
		{
			if(ctrl_on)
				if(zoom_level<3.75f)
					{
						zoom_level+=0.25f;
						resize_window();
					}
				else 
					{
						if(cur_mode==mode_3d && selected_3d_object!=-1)
						if(shift_on)objects_list[selected_3d_object]->y_rot+=1.0f;
						else objects_list[selected_3d_object]->y_rot+=10.0f;
					}

		}


		if ( event->key.keysym.sym == SDLK_INSERT )
		{
			if(cur_mode==mode_3d && selected_3d_object!=-1)
			if(shift_on)objects_list[selected_3d_object]->z_pos+=0.01f;
			else objects_list[selected_3d_object]->z_pos+=0.1f;

			if(cur_mode==mode_2d && selected_2d_object!=-1)
			if(shift_on)obj_2d_list[selected_2d_object]->z_pos+=0.01f;
			else obj_2d_list[selected_2d_object]->z_pos+=0.1f;

			if(cur_mode==mode_light && selected_light!=-1)
			if(shift_on)lights_list[selected_light]->pos_z+=0.01f;
			else lights_list[selected_light]->pos_z+=0.1f;

			if(cur_mode==mode_height && selected_height!=-1)
			if(selected_height<31)selected_height++;

		}

		if ( event->key.keysym.sym == SDLK_DELETE )
		{
			if(cur_mode==mode_3d && selected_3d_object!=-1)
			if(shift_on)objects_list[selected_3d_object]->z_pos-=0.01f;
			else objects_list[selected_3d_object]->z_pos-=0.1f;

			if(cur_mode==mode_2d && selected_2d_object!=-1)
			if(shift_on)obj_2d_list[selected_2d_object]->z_pos-=0.01f;
			else obj_2d_list[selected_2d_object]->z_pos-=0.1f;

			if(cur_mode==mode_light && selected_light!=-1)
			if(shift_on)lights_list[selected_light]->pos_z-=0.01f;
			else lights_list[selected_light]->pos_z-=0.1f;

			if(cur_mode==mode_height && selected_height!=-1)
			if(selected_height>0)selected_height--;


		}

		if ( event->key.keysym.sym == SDLK_F1 )
		{
			game_minute++;
			if(game_minute>60*5)game_minute=0;
		}

		if ( event->key.keysym.sym == SDLK_F2 )
		{
			if(game_minute==0)game_minute=60*5;
			else
			game_minute--;
		}

        //see if we get any text
        if ((event->key.keysym.unicode & 0xFF80)==0)
  		ch = event->key.keysym.unicode & 0x7F;
  		//check wehter we should switch shadows on/off
  		if((ch=='s' || ch=='S') && alt_on)shadows_on=!shadows_on;

  		//do we want to toggle the transparency of a 3d object?
  		if(ch=='b' && selected_3d_object!=-1 && cur_mode==mode_3d)
  		objects_list[selected_3d_object]->blended=!objects_list[selected_3d_object]->blended;
  		if(ch=='l' && selected_3d_object!=-1 && cur_mode==mode_3d)
  		objects_list[selected_3d_object]->self_lit=!objects_list[selected_3d_object]->self_lit;
  		//do the lightening stuff
  		if(ch=='1' && selected_3d_object!=-1 && cur_mode==mode_3d && !alt_on)
  		if(objects_list[selected_3d_object]->r<1.0f)objects_list[selected_3d_object]->r+=0.05f;
  		if(ch=='1' && selected_3d_object!=-1 && cur_mode==mode_3d && alt_on)
  		if(objects_list[selected_3d_object]->r>0.0f)objects_list[selected_3d_object]->r-=0.05f;
  		if(ch=='2' && selected_3d_object!=-1 && cur_mode==mode_3d && !alt_on)
  		if(objects_list[selected_3d_object]->g<1.0f)objects_list[selected_3d_object]->g+=0.05f;
  		if(ch=='2' && selected_3d_object!=-1 && cur_mode==mode_3d && alt_on)
  		if(objects_list[selected_3d_object]->g>0.0f)objects_list[selected_3d_object]->g-=0.05f;
  		if(ch=='3' && selected_3d_object!=-1 && cur_mode==mode_3d && !alt_on)
  		if(objects_list[selected_3d_object]->b<1.0f)objects_list[selected_3d_object]->b+=0.05f;
  		if(ch=='3' && selected_3d_object!=-1 && cur_mode==mode_3d && alt_on)
  		if(objects_list[selected_3d_object]->b>0.0f)objects_list[selected_3d_object]->b-=0.05f;
		//for lights now
  		if(ch=='1' && selected_light!=-1 && cur_mode==mode_light && !alt_on)
  		if(lights_list[selected_light]->r<5.0f)lights_list[selected_light]->r+=0.02f;
  		if(ch=='1' && selected_light!=-1 && cur_mode==mode_light && alt_on)
  		if(lights_list[selected_light]->r>0.0f)lights_list[selected_light]->r-=0.02f;
  		if(ch=='2' && selected_light!=-1 && cur_mode==mode_light && !alt_on)
  		if(lights_list[selected_light]->g<5.0f)lights_list[selected_light]->g+=0.02f;
  		if(ch=='2' && selected_light!=-1 && cur_mode==mode_light && alt_on)
  		if(lights_list[selected_light]->g>0.0f)lights_list[selected_light]->g-=0.02f;
  		if(ch=='3' && selected_light!=-1 && cur_mode==mode_light && !alt_on)
  		if(lights_list[selected_light]->b<5.0f)lights_list[selected_light]->b+=0.02f;
  		if(ch=='3' && selected_light!=-1 && cur_mode==mode_light && alt_on)
  		if(lights_list[selected_light]->b>0.0f)lights_list[selected_light]->b-=0.02f;
		//for ambient light
  		if(ch=='1' && cur_mode==mode_map && !alt_on)
  		if(ambient_r<1.0f)ambient_r+=0.02f;
  		if(ch=='1' && cur_mode==mode_map && alt_on)
  		if(ambient_r>-0.05f)ambient_r-=0.02f;
  		if(ch=='2' && cur_mode==mode_map && !alt_on)
  		if(ambient_g<1.0f)ambient_g+=0.02f;
  		if(ch=='2' && cur_mode==mode_map && alt_on)
  		if(ambient_g>-0.05f)ambient_g-=0.02f;
  		if(ch=='3' && cur_mode==mode_map && !alt_on)
  		if(ambient_b<1.0f)ambient_b+=0.02f;
  		if(ch=='3' && cur_mode==mode_map && alt_on)
  		if(ambient_b>-0.05f)ambient_b-=0.02f;
  		if((ch=='d' || ch=='D') && cur_mode==mode_map)
			{
					if(dungeon)
						{
							//ok, change the water to normal
							glDeleteTextures(1,&texture_cache[sky_text_1].texture_id);
							//also destroy the name of that texture, since it is not in cache anymore
							texture_cache[sky_text_1].file_name[0]=0;
							sky_text_1=load_texture_cache("./textures/sky.bmp",70);

						}
					if(!dungeon)
						{
							//ok, change the water to dungeon water
							glDeleteTextures(1,&texture_cache[sky_text_1].texture_id);
							//also destroy the name of that texture, since it is not in cache anymore
							texture_cache[sky_text_1].file_name[0]=0;
							sky_text_1=load_texture_cache("./textures/water2.bmp",70);

						}
  				dungeon=!dungeon;
			}


  		if(ch=='m')minimap_on=!minimap_on;


		break;

		case SDL_VIDEORESIZE:
	    {
	      window_width = event->resize.w;
	      window_height = event->resize.h;
		  resize_window();
	    }
	    break;

	    case SDL_QUIT:
		done = 1;
		break;
	}

	if(event->type==SDL_MOUSEMOTION || event->type==SDL_MOUSEBUTTONDOWN || event->type==SDL_MOUSEBUTTONUP)
 		   {
			 char tool_bar_click=0;
 		     mouse_x=event->motion.x;
 		     mouse_y=event->motion.y;

 		           //get the buttons state
			       if (SDL_GetMouseState (NULL, NULL) & SDL_BUTTON (SDL_BUTTON_LEFT))
				   left_click++;
			       else
			 	   left_click = 0;

			       if (SDL_GetMouseState (NULL, NULL) & SDL_BUTTON (SDL_BUTTON_RIGHT))
			 	   right_click++;
			       else
				   right_click= 0;

			if(minimap_on && left_click==1)
				{
					check_mouse_minimap();
					return(done);
				}
			else
			if(minimap_on && right_click && cur_mode==mode_tile)
				{
					draw_mouse_minimap();
					return(done);
				}

			if(check_interface_buttons()==1)tool_bar_click=1;
			if(right_click==1 && cur_tool==tool_select && selected_tile!=255 && cur_mode==mode_tile)selected_tile=255;
			if(right_click==1 && cur_tool==tool_select && selected_height!=-1 && cur_mode==mode_height)selected_height=-1;
			if(right_click==1 && cur_tool==tool_select && selected_2d_object!=-1 && cur_mode==mode_2d)kill_2d_object(selected_2d_object);
			if(right_click==1 && cur_tool==tool_select && selected_3d_object!=-1 && cur_mode==mode_3d)kill_3d_object(selected_3d_object);
			if(right_click==1 && cur_mode==mode_tile && view_tiles_list)
				{
					view_tiles_list=0;
					cur_tool=tool_select;
					selected_tile=0;
				}
			get_world_x_y();
			if(!tool_bar_click)
				{
					if(left_click==1)
						{
							if(cur_mode==mode_3d)
							{
								if(cur_tool==tool_kill)
									{
										get_3d_object_under_mouse();
										if(selected_3d_object!=-1)kill_3d_object(selected_3d_object);
										return(done);
									}
								if(cur_tool==tool_clone)
									{
										get_3d_object_under_mouse();
										if(selected_3d_object!=-1)clone_3d_object(selected_3d_object);
										return(done);
									}

								//if we have an object attached to us, drop it
								if(cur_tool==tool_select && selected_3d_object!=-1)selected_3d_object=-1;
								else
								{
									get_3d_object_under_mouse();
								}
							}
							//2D objects/////////////////
							if(cur_mode==mode_2d)
							{
								if(cur_tool==tool_kill)
									{
										get_2d_object_under_mouse();
										if(selected_2d_object!=-1)kill_2d_object(selected_2d_object);
										return(done);
									}
								if(cur_tool==tool_clone)
									{
										get_2d_object_under_mouse();
										if(selected_2d_object!=-1)clone_2d_object(selected_2d_object);
										return(done);
									}

								//if we have an object attached to us, drop it

								if(cur_tool==tool_select && selected_2d_object!=-1)clone_2d_object(selected_2d_object);
								else
								{
									get_2d_object_under_mouse();
								}

							}
							//Lights/////////////////
							if(cur_mode==mode_light)
							{
								if(cur_tool==tool_kill)
									{
										get_light_under_mouse();
										if(selected_light!=-1)kill_light(selected_light);
										return(done);
									}
								if(cur_tool==tool_clone)
									{
										get_light_under_mouse();
										if(selected_light!=-1)clone_light(selected_light);
										return(done);
									}

								//if we have a light attached to us, drop it
								if(cur_tool==tool_select && selected_light!=-1)selected_light=-1;
								else
								{
									get_light_under_mouse();
								}
							}
							// tiles /////////////////
							if(cur_mode==mode_tile)
							{
								if(cur_tool==tool_kill && scene_mouse_y>0 && scene_mouse_x>0 && scene_mouse_y<tile_map_size_y*3 && scene_mouse_x<tile_map_size_x*3)
									{
										tile_map[(int)scene_mouse_y/3*tile_map_size_x+(int)scene_mouse_x/3]=255;
										return(done);
									}

								if(cur_tool==tool_clone  && scene_mouse_y>0 && scene_mouse_x>0 && scene_mouse_y<tile_map_size_y*3 && scene_mouse_x<tile_map_size_x*3)
									{
										selected_tile=tile_map[(int)scene_mouse_y/3*tile_map_size_x+(int)scene_mouse_x/3];
										cur_tool=tool_select;
										return(done);
									}

								//if we have a tile attached to us, drop it
								if(cur_tool==tool_select && selected_tile!=255  && scene_mouse_y>0 && scene_mouse_x>0 && scene_mouse_y<tile_map_size_y*3 && scene_mouse_x<tile_map_size_x*3)
									{
										tile_map[(int)scene_mouse_y/3*tile_map_size_x+(int)scene_mouse_x/3]=selected_tile;
									}
								if(view_tiles_list)get_tile_under_mouse_from_list();
							}
							// heights /////////////////
							if(cur_mode==mode_height)
							{
								if(cur_tool==tool_kill  && scene_mouse_y>0 && scene_mouse_x>0 && scene_mouse_y<tile_map_size_y*3*6 && scene_mouse_x<tile_map_size_x*3*6)
									{
										height_map[(int)(scene_mouse_y/0.5f)*tile_map_size_x*6+(int)(scene_mouse_x/0.5f)]=11;
										return(done);
									}

								if(cur_tool==tool_clone && scene_mouse_y>0 && scene_mouse_x>0 && scene_mouse_y<tile_map_size_y*3*6 && scene_mouse_x<tile_map_size_x*3*6)
									{
										selected_height=height_map[(int)(scene_mouse_y/0.5f)*tile_map_size_x*6+(int)(scene_mouse_x/0.5f)];
										cur_tool=tool_select;
										return(done);
									}

								//if we have a height attached to us, drop it
								if(cur_tool==tool_select && selected_height!=-1  && scene_mouse_y>0 && scene_mouse_x>0 && scene_mouse_y<tile_map_size_y*3*6 && scene_mouse_x<tile_map_size_x*3*6)
									{
										if(alt_on)draw_big_height_tile(1);
										else
										if(ctrl_on)draw_big_height_tile(0);
										else
										height_map[(int)(scene_mouse_y/0.5f)*tile_map_size_x*6+(int)(scene_mouse_x/0.5f)]=selected_height;
									}
								if(view_heights_list)get_height_under_mouse_from_list();
							}

						}
						//no left click==1
						else
						if(cur_mode==mode_3d && cur_tool==tool_select && selected_3d_object!=-1)move_3d_object(selected_3d_object);
						else
						if(cur_mode==mode_2d && cur_tool==tool_select && selected_2d_object!=-1)move_2d_object(selected_2d_object);
						else
						if(cur_mode==mode_light && cur_tool==tool_select && selected_light!=-1)move_light(selected_light);
						else
						if(cur_mode==mode_tile && cur_tool==tool_select && selected_tile!=255)move_tile_a_tile=1;
						else move_tile_a_tile=0;
						if(cur_mode==mode_height && cur_tool==tool_select && selected_height!=-1)move_tile_a_height=1;
						else move_tile_a_height=0;

				}
 		   }

	return(done);
}
