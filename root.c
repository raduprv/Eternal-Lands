#include "global.h"

#ifdef WINDOW_CHAT

int root_win = -1;

// This is the main part of the old check_cursor_change ()
int mouseover_root_handler (window_info *win, int mx, int my)
{
	if(object_under_mouse == -1)
	{
		elwin_mouse = CURSOR_WALK;
	}

	else if (thing_under_the_mouse==UNDER_MOUSE_3D_OBJ && objects_list[object_under_mouse])
	{
		int i;
		
		if(action_mode==action_look)
		{
			elwin_mouse = CURSOR_EYE;
		}

		//see if it is a bag. (bag1.e3d)
		else if(get_string_occurance("bag1.e3d",objects_list[object_under_mouse]->file_name, 80,0)!=-1)
		{
			elwin_mouse = CURSOR_PICK;
		}

		else if(action_mode==action_use)
		{
			elwin_mouse = CURSOR_USE;
		}
		
		else
		{
			//see if the object is a harvestable resource.
			for(i=0;i<100;i++)
			{
				if(!harvestable_objects[i].name[0])break;//end of the objects
				if(get_string_occurance(harvestable_objects[i].name,objects_list[object_under_mouse]->file_name, 80,0)!=-1)
				{
					elwin_mouse = CURSOR_HARVEST;
					return 1;
				}
			}
			
			//see if the object is an entrable resource.
			for(i=0;i<100;i++)
			{
				if(!entrable_objects[i].name[0])break;//end of the objects
				if(get_string_occurance(entrable_objects[i].name,objects_list[object_under_mouse]->file_name, 80,0)!=-1)
				{
					elwin_mouse = CURSOR_ENTER;
					return 1;
				}
			}
		
			if(action_mode==action_use_witem)
			{
				elwin_mouse = CURSOR_USE_WITEM;
			}

			//hmm, no usefull object, so select walk....
			else 
			{
				elwin_mouse = CURSOR_WALK;
			}
		}
	}

	else if(thing_under_the_mouse==UNDER_MOUSE_NPC)
	{
		if(action_mode==action_look)
		{
			elwin_mouse = CURSOR_EYE;
		}
		
		else 
		{
			elwin_mouse = CURSOR_TALK;
		}
	}

	else if(thing_under_the_mouse==UNDER_MOUSE_PLAYER)
	{
		if(action_mode==action_use)
		{
			elwin_mouse = CURSOR_USE;
		}      

		else if(action_mode==action_look)
		{
			elwin_mouse = CURSOR_EYE;
		}

		else if(action_mode==action_trade)
		{
			elwin_mouse = CURSOR_TRADE;
		}

		else if(alt_on || action_mode==action_attack)
		{
			elwin_mouse = CURSOR_ATTACK;
		}

		else
		{
			elwin_mouse = CURSOR_EYE;
		}
	}

	else if(thing_under_the_mouse==UNDER_MOUSE_ANIMAL)
	{
		if(action_mode==action_use)
		{
			elwin_mouse = CURSOR_USE;
		}      
		
		else if(action_mode==action_look)
		{
			elwin_mouse = CURSOR_EYE;
		}

		else if(shift_on)
		{
			elwin_mouse = CURSOR_EYE;
		}

		else if(alt_on || action_mode==action_attack || (actor_under_mouse && !actor_under_mouse->dead))
		{
			elwin_mouse = CURSOR_ATTACK;
		}
	}

	// when all fails - walk
	else
	{
		elwin_mouse = CURSOR_WALK;
	}

	return 1;
}

// this is the main part of the old check_mouse_click ()
int click_root_handler (window_info *win, int mx, int my, Uint32 flags)
{
	int flag_ctrl = flags & ELW_CTRL;
	int flag_right = flags & ELW_RIGHT_MOUSE;
	int force_walk = (flag_ctrl && flag_right);
	
	if (!force_walk)
	{
		if (flag_right) 
		{
			if (item_dragged != -1 || use_item != -1 || object_under_mouse == -1)
			{
				use_item = -1;
				item_dragged = -1;
				action_mode = action_walk;
				return 1;
			}
			switch (current_cursor) 
			{
				case CURSOR_EYE:
					if (thing_under_the_mouse == UNDER_MOUSE_PLAYER)
						action_mode = action_trade;
					else if (thing_under_the_mouse == UNDER_MOUSE_3D_OBJ)
						action_mode = action_use;
					else
						action_mode = action_walk;
					break;
				case CURSOR_HARVEST:
					action_mode = action_look;
					break;
				case CURSOR_TRADE:
					action_mode = action_attack;
					break;
				case CURSOR_USE_WITEM:
					if(use_item != -1)
						use_item = -1;
					else
						action_mode = action_walk;
					break;
				case CURSOR_ATTACK:
					if(thing_under_the_mouse == UNDER_MOUSE_ANIMAL)
						action_mode = action_look;
					else
						action_mode = action_walk;
					break;
				case CURSOR_ENTER:
				case CURSOR_PICK:
				case CURSOR_WALK:
					if(thing_under_the_mouse == UNDER_MOUSE_3D_OBJ)
						action_mode = action_look;
					else
						action_mode = action_walk;
					break;
				case CURSOR_USE:
				case CURSOR_TALK:
				case CURSOR_ARROW:
				default:
					action_mode = action_walk;
					break;
			}
			return 1;
		}
	}

	// after we test for interface clicks
	// alternative drop method...
	if (item_dragged != -1)
	{
		Uint8 str[10];
		int quantity = item_list[item_dragged].quantity;

		if (flag_right)
		{
			item_dragged = -1;
			return 1;
		}

		if (quantity - item_quantity > 0)
			quantity = item_quantity;
		str[0] = DROP_ITEM;
		str[1] = item_list[item_dragged].pos;
		*((Uint16 *) (str + 2)) = quantity;
		my_tcp_send(my_socket, str, 4);
		if (item_list[item_dragged].quantity - quantity <= 0)
			item_dragged = -1;
		return 1;
	}

	// if we're following a path, stop now
	if (pf_follow_path) 
	{
		pf_destroy_path();
	}

	if (force_walk)
	{
		Uint8 str[10];
		short x,y;
		
		//get_old_world_x_y ();
		get_world_x_y ();
		x = scene_mouse_x / 0.5f;
		y = scene_mouse_y / 0.5f;
		// check to see if the coordinates are OUTSIDE the map
		if (y < 0 || x < 0 || x >= tile_map_size_x*6 || y >= tile_map_size_y*6)
			return 1;
		
		str[0] = MOVE_TO;
		*((short *)(str+1)) = x;
		*((short *)(str+3)) = y;

		my_tcp_send(my_socket,str,5);
		return 1;
	}
	
	switch(current_cursor) 
	{
		case CURSOR_EYE:
		{
			Uint8 str[10];

			if (object_under_mouse == -1)
				return 1;

			if (thing_under_the_mouse == UNDER_MOUSE_PLAYER || thing_under_the_mouse == UNDER_MOUSE_NPC || thing_under_the_mouse == UNDER_MOUSE_ANIMAL)
			{
				str[0] = GET_PLAYER_INFO;
				*((int *)(str+1)) = object_under_mouse;
				my_tcp_send (my_socket, str, 5);
				return 1;
			}
			else if (thing_under_the_mouse == UNDER_MOUSE_3D_OBJ)
			{
				str[0] = LOOK_AT_MAP_OBJECT;
				*((int *)(str+1)) = object_under_mouse;
				my_tcp_send (my_socket, str, 5);
				return 1;
			}

			break;
		}

		case CURSOR_TRADE:
		{
			Uint8 str[10];

			if (object_under_mouse == -1)
				return 1;
			if (thing_under_the_mouse != UNDER_MOUSE_PLAYER)
				return 1;
			str[0] = TRADE_WITH;
			*((int *)(str+1)) = object_under_mouse;
			my_tcp_send (my_socket, str, 5);
			return 1;
			
			break;
		}

		case CURSOR_ATTACK:
		{
			Uint8 str[10];

			if (object_under_mouse == -1)
				return 1;
			if (you_sit && sit_lock && !flag_ctrl)
				return 1;
			if (thing_under_the_mouse == UNDER_MOUSE_PLAYER || thing_under_the_mouse == UNDER_MOUSE_NPC || thing_under_the_mouse == UNDER_MOUSE_ANIMAL)
			{
				str[0] = ATTACK_SOMEONE;
				*((int *)(str+1)) = object_under_mouse;
				my_tcp_send (my_socket, str, 5);
				return 1;
			}

			break;
		}

		case CURSOR_ENTER:
			if(you_sit && sit_lock && !flag_ctrl)
				return 1;
		case CURSOR_USE:
		case CURSOR_USE_WITEM:
		case CURSOR_TALK:
		{
			Uint8 str[10];

			if (object_under_mouse == -1)
				return 1;
			if (thing_under_the_mouse == UNDER_MOUSE_PLAYER || thing_under_the_mouse == UNDER_MOUSE_NPC || thing_under_the_mouse == UNDER_MOUSE_ANIMAL)
			{
				int i;
				str[0] = TOUCH_PLAYER;
				*((int *)(str+1)) = object_under_mouse;
				my_tcp_send (my_socket, str, 5);

				// clear the previous dialogue entries, so we won't have a left over from some other NPC
				for (i=0; i<20; i++)
					dialogue_responces[i].in_use = 0;
				return 1;
			}
			
			str[0] = USE_MAP_OBJECT;
			*((int *)(str+1)) = object_under_mouse;
			if (use_item != -1 && current_cursor == CURSOR_USE_WITEM)
			{
				*((int *)(str+5)) = item_list[use_item].pos;
				use_item = -1;
				action_mode = action_walk;
			} 
			else
			{
				*((int *)(str+5)) = -1;
			}

			my_tcp_send (my_socket, str, 9);
			return 1;

			break;
		}

		case CURSOR_PICK:
			if (object_under_mouse == -1)
				return 1;
			if (you_sit && sit_lock && !flag_ctrl)
				return 1;
			open_bag (object_under_mouse);
			return 1;
			break;

		case CURSOR_HARVEST:
		{
			Uint8 str[10];

			if (object_under_mouse == -1)
				return 1;
			str[0] = HARVEST;
			*((short *)(str+1)) = object_under_mouse;
			my_tcp_send (my_socket, str, 3);
			return 1;
			break;
		}

		case CURSOR_WALK:
		default:
		{
			Uint8 str[10];
			short x, y;
		
			if (you_sit && sit_lock && !flag_ctrl)
				return 1;
			get_world_x_y ();
			x = scene_mouse_x / 0.5f;
			y = scene_mouse_y / 0.5f;
			// check to see if the coordinates are OUTSIDE the map
			if (y < 0 || x < 0 || x >= tile_map_size_x*6 || y >= tile_map_size_y*6)
				return 1;
			
			str[0] = MOVE_TO;
			*((short *)(str+1)) = x;
			*((short *)(str+3)) = y;
	
			my_tcp_send (my_socket, str, 5);
			return 1;
		}
	}

	left_click = 2;
	right_click = 2;
	return 1;
}

int display_root_handler (window_info *win)
{
	static int main_count = 0;
	static int old_fps_average = 0;
	static int fps_average = 0;
	static int times_FPS_below_3 = 0;
	unsigned char str[180];
	int fps;
	int y_line,i;
	int any_reflection = 0;
	int mouse_rate;
	
	if (!have_a_map) return 1;
	if (yourself==-1) return 1; //we don't have ourselves
	
	for(i=0; i<max_actors; i++)
	{
        	if(actors_list[i] && actors_list[i]->actor_id == yourself) 
			break;
	}
	if(i > max_actors) return 1;//we still don't have ourselves
	
	main_count++;
	
	//if (quickbar_win>0) windows_list.window[quickbar_win].displayed=1;

	if (old_fps_average < 5)
	{
		mouse_rate = 1;
		read_mouse_now = 1;
	}
	else if (old_fps_average<10)
	{
		mouse_rate = 3;
	}
	else if (old_fps_average < 20)
	{
		mouse_rate = 6;
	}
	else if (old_fps_average < 30)
	{
		mouse_rate = 10;
	}
	else if (old_fps_average < 40)
	{
		mouse_rate = 15;
	}
	else
	{
		mouse_rate = 20;
	}
	if(mouse_rate > mouse_limit) mouse_rate = mouse_limit;
	if (!(main_count%mouse_rate)) 
		read_mouse_now = 1;
	else 
		read_mouse_now=0;
	reset_under_the_mouse();

	if (new_zoom_level != zoom_level)
	{
		zoom_level = new_zoom_level;
		resize_root_window ();
	}
	
	// This window is a bit special since it's not fully 2D
	glPushMatrix (); // save the current state
	Leave2DMode ();
		
	glLoadIdentity ();				// Reset The Matrix
	Move ();
	save_scene_matrix ();

	CalculateFrustum ();
	any_reflection = find_reflection ();
	check_gl_errors ();

	// are we actively drawing things?
	if (SDL_GetAppState() & SDL_APPACTIVE)
	{
		//now, determine the current weather light level
		get_weather_light_level ();

		if (!dungeon) 
			draw_global_light ();
		else 
			draw_dungeon_light ();
		update_scene_lights ();
		draw_lights ();
		check_gl_errors ();

		if (!dungeon && shadows_on && is_day) 
			render_light_view();
		check_gl_errors ();

		//check for network data
		get_message_from_server ();

		glEnable (GL_FOG);
		if (any_reflection > 1)
		{
		  	if (!dungeon)
				draw_sky_background ();
		  	else 
				draw_dungeon_sky_background ();
			check_gl_errors ();
			if (show_reflection) display_3d_reflection ();
		}
		check_gl_errors ();

		//check for network data - reduces resyncs
		get_message_from_server ();

		if (!dungeon && shadows_on && is_day)
		{
			draw_sun_shadowed_scene (any_reflection);
		}
		else 
		{
			glNormal3f (0.0f,0.0f,1.0f);
			if (any_reflection) draw_lake_tiles ();
			draw_tile_map ();
			check_gl_errors ();
			display_2d_objects ();
			check_gl_errors ();
			anything_under_the_mouse (0, UNDER_MOUSE_NOTHING);
			display_objects ();
			display_actors ();
		}
		glDisable (GL_FOG);
		check_gl_errors ();

		//check for network data - reduces resyncs
		get_message_from_server ();
	}	// end of active display check
	else 
	{
		display_actors ();	// we need to 'touch' all the actors even if not drawing to avoid problems
	}

	check_gl_errors ();

	//check for network data - reduces resyncs
	get_message_from_server ();

	// if not active, dont bother drawing any more
	if (!(SDL_GetAppState () & SDL_APPACTIVE))
	{
		SDL_Delay (20);
		// Return to 2D mode to draw the other windows
		glPopMatrix (); // restore the state
		Enter2DMode ();
		return 1;
	}

	//particles should be last, we have no Z writting
	display_particles ();
	check_gl_errors ();
	if (is_raining) render_rain ();
	check_gl_errors ();
	//we do this because we don't want the rain/particles to mess with our cursor

	Enter2DMode ();
	//get the FPS, etc
	if ((cur_time-last_time)) 
		fps = 1000 / (cur_time-last_time);
	else 
		fps = 1000;

	if (main_count%10)
	{
		fps_average += fps;
	}
	else
	{
		old_fps_average = fps_average / 10;
		fps_average = 0;
	}
	if (!no_adjust_shadows)
	{
		if (fps < 5)
		{
			times_FPS_below_3++;
			if (times_FPS_below_3 > 4 && shadows_on)
			{
				shadows_on = 0;
				put_colored_text_in_buffer (c_red1,low_framerate_str, -1, 0);
				times_FPS_below_3 = 0;
			}
		}
		else 
		{
			times_FPS_below_3 = 0;
		}
	}
	if (show_fps)
	{
		sprintf (str, "FPS: %i", old_fps_average);
		glColor3f (1.0f, 1.0f, 1.0f);
		draw_string (10, 0, str, 1);
	}

	check_gl_errors ();
	if (!use_windowed_chat && find_last_lines_time ())
	{
		set_font(chat_font);	// switch to the chat font
        	draw_string_zoomed (10, 20, &display_text_buffer[display_text_buffer_first], max_lines_no, chat_zoom);
		set_font (0);	// switch to fixed
	}
	
	anything_under_the_mouse (0, UNDER_MOUSE_NO_CHANGE);
	check_gl_errors ();

	draw_ingame_interface ();
	
	check_gl_errors ();
	// print the text line we are currently writting (if any)
	y_line = window_height - (17 * (4+input_text_lines));
	switch(map_type)
	{
		case 2:
			glColor3f (0.6f, 1.0f, 1.0f);
			break;
		case 1:
		default:
			glColor3f (1.0f, 1.0f, 1.0f);
	}
	draw_string (10, y_line, input_text_line, input_text_lines);

	Leave2DMode ();
	glEnable (GL_LIGHTING);

	// Return to 2D mode to draw the other windows
	glPopMatrix ();	// restore the state
	Enter2DMode ();

	return 1;
}

void display_root ()
{
	if (root_win < 0)
	{
		root_win = create_window ("root", -1, -1, 0, 0, window_width, window_height, ELW_WIN_INVISIBLE|ELW_SHOW_LAST);
		
        	set_window_handler (root_win, ELW_HANDLER_DISPLAY, &display_root_handler);
        	set_window_handler (root_win, ELW_HANDLER_CLICK, &click_root_handler);
        	set_window_handler (root_win, ELW_HANDLER_MOUSEOVER, &mouseover_root_handler);
		
		resize_root_window();
	}
}

#endif
