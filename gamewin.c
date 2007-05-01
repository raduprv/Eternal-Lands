#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "weather.h"
#include "draw_scene.h"
#ifdef SFX
#include "special_effects.h"
#ifdef	EYE_CANDY
#include "eye_candy_wrapper.h"
#endif	//EYE_CANDY
#endif // SFX

int game_root_win = -1;
int gamewin_in_id = 4442;
int use_old_clicker=0;
float fps_average = 100.0;
#ifdef  DEBUG
extern int e3d_count, e3d_total;    // LRNR:stats testing only
#endif  //DEBUG

// This is the main part of the old check_cursor_change ()
int mouseover_game_handler (window_info *win, int mx, int my)
{
	if(object_under_mouse == -1)
	{
		if(spell_result==2){
			elwin_mouse = CURSOR_WAND;
		} else {
			elwin_mouse = CURSOR_WALK;
		}
	}

	else if (thing_under_the_mouse==UNDER_MOUSE_3D_OBJ && objects_list[object_under_mouse])
	{
		if(action_mode==ACTION_LOOK)
		{
			elwin_mouse = CURSOR_EYE;
		}
		else if(objects_list[object_under_mouse]->flags&OBJ_3D_BAG)
		{
			elwin_mouse = CURSOR_PICK;
		}
		else if(action_mode==ACTION_USE)
		{
			elwin_mouse = CURSOR_USE;
		}
		else if(action_mode==ACTION_USE_WITEM)
		{
			elwin_mouse = CURSOR_USE_WITEM;
		}
		//see if the object is a harvestable resource.
		else if(objects_list[object_under_mouse]->flags&OBJ_3D_HARVESTABLE) 
		{
			elwin_mouse = CURSOR_HARVEST;
		}
		//see if the object is an entrable resource.
		else if(objects_list[object_under_mouse]->flags&OBJ_3D_ENTRABLE) {
			elwin_mouse = CURSOR_ENTER;
		}
		//hmm, no usefull object, so select walk....
		else  
		{				
			if(spell_result==2)
				elwin_mouse = CURSOR_WAND;
			else
				elwin_mouse = CURSOR_WALK;
		}
	}

	else if(thing_under_the_mouse==UNDER_MOUSE_NPC)
	{
		if(action_mode==ACTION_LOOK)
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
		if(action_mode==ACTION_USE)
		{
			elwin_mouse = CURSOR_USE;
		}
		else if(action_mode==ACTION_LOOK)
		{
			elwin_mouse = CURSOR_EYE;
		}
		else if(action_mode==ACTION_TRADE)
		{
			elwin_mouse = CURSOR_TRADE;
		}
		else if(alt_on || action_mode==ACTION_ATTACK)
		{
			elwin_mouse = CURSOR_ATTACK;
		}
		else if (spell_result==3 && action_mode==ACTION_WAND)
		{
			elwin_mouse = CURSOR_WAND;
		}
		else 
		{
			elwin_mouse = CURSOR_EYE;
		}
	}

	else if(thing_under_the_mouse==UNDER_MOUSE_ANIMAL)
	{
		if(action_mode==ACTION_USE)
		{
			elwin_mouse = CURSOR_USE;
		}
		else if(action_mode==ACTION_LOOK)
		{
			elwin_mouse = CURSOR_EYE;
		}
		else if(shift_on)
		{
			elwin_mouse = CURSOR_EYE;
		}
		else if(spell_result==3 && action_mode == ACTION_WAND)
		{
			elwin_mouse = CURSOR_WAND;
		}
		else if(alt_on || action_mode==ACTION_ATTACK || (actor_under_mouse && !actor_under_mouse->dead))
		{
			elwin_mouse = CURSOR_ATTACK;
		}
	}

	// when all fails - walk
	else
	{
		if(spell_result==2) {
			elwin_mouse = CURSOR_WAND;
		} else {
			elwin_mouse = CURSOR_WALK;
		}
	}

	return 1;
}

// this is the main part of the old check_mouse_click ()
int click_game_handler (window_info *win, int mx, int my, Uint32 flags)
{
	int flag_ctrl = flags & ELW_CTRL;
	int flag_right = flags & ELW_RIGHT_MOUSE;
	int force_walk = (flag_ctrl && flag_right);
	
	if (flags & ELW_WHEEL_UP)
	{
		if (camera_zoom_dir == -1)
			camera_zoom_frames += 5;
		else
			camera_zoom_frames = 5;
		camera_zoom_dir = -1;
		return 1;
	}
	
	if (flags & ELW_WHEEL_DOWN)
	{
		if (camera_zoom_dir == 1)
			camera_zoom_frames += 5;
		else
			camera_zoom_frames = 5;
		camera_zoom_dir = 1;
		return 1;
	}
	
	if (!force_walk)
	{
		if (flag_right) 
		{
			if (item_dragged != -1 || use_item != -1 || object_under_mouse == -1 
					|| storage_item_dragged != -1
					)
			{
				use_item = -1;
				item_dragged = -1;
				storage_item_dragged = -1;
				action_mode = ACTION_WALK;
				return 1;
			}
			switch (current_cursor) 
			{
				case CURSOR_EYE:
					if (thing_under_the_mouse == UNDER_MOUSE_ANIMAL && spell_result==3)
						action_mode = ACTION_WAND;
					else if (thing_under_the_mouse == UNDER_MOUSE_PLAYER)
						action_mode = ACTION_TRADE;
					else if (thing_under_the_mouse == UNDER_MOUSE_3D_OBJ){
						action_mode = ACTION_USE;
					}
					else
						action_mode = ACTION_WALK;
					break;
				case CURSOR_HARVEST:
					action_mode = ACTION_LOOK;
					break;
				case CURSOR_TRADE:
					if(spell_result==3)action_mode=ACTION_WAND;
					else action_mode = ACTION_ATTACK;
					break;
				case CURSOR_USE_WITEM:
					if(use_item != -1)
						use_item = -1;
					else
						action_mode = ACTION_WALK;
					break;
				case CURSOR_WAND:
					if(thing_under_the_mouse == UNDER_MOUSE_ANIMAL||thing_under_the_mouse == UNDER_MOUSE_PLAYER)
						action_mode = ACTION_ATTACK;
					else if(thing_under_the_mouse == UNDER_MOUSE_3D_OBJ)
						action_mode = ACTION_LOOK;
					break;
				case CURSOR_ATTACK:
					if(thing_under_the_mouse == UNDER_MOUSE_ANIMAL)
						action_mode = ACTION_LOOK;
					else
						action_mode = ACTION_WALK;
					break;
				case CURSOR_ENTER:
				case CURSOR_PICK:
				case CURSOR_WALK:
					if(thing_under_the_mouse == UNDER_MOUSE_3D_OBJ)
						action_mode = ACTION_LOOK;
					else
						action_mode = ACTION_WALK;
					break;
				case CURSOR_USE:
				case CURSOR_TALK:
				case CURSOR_ARROW:
				default:
					action_mode = ACTION_WALK;
					break;
			}
			return 1;
		}
	}

	// after we test for interface clicks
	// alternative drop method...
	if (item_dragged >= 0 && item_dragged < ITEM_NUM_ITEMS)
	{
		Uint8 str[10];

		if (flag_right)
		{
			item_dragged = -1;
			return 1;
		}

		str[0] = DROP_ITEM;
		str[1] = item_list[item_dragged].pos;
		*((Uint32 *) (str + 2)) = SDL_SwapLE32(item_quantity);
		my_tcp_send(my_socket, str, 6);
		return 1;
	}

	if (storage_item_dragged != -1)
	{
		//TODO: Withdraw from storage, drop on ground...
	}

	// if we're following a path, stop now
	if (pf_follow_path) 
	{
		pf_destroy_path();
	}

	if (force_walk)
	{
		short x,y;
		
		get_old_world_x_y ();
		x = scene_mouse_x / 0.5f;
		y = scene_mouse_y / 0.5f;
		// check to see if the coordinates are OUTSIDE the map
		if (y < 0 || x < 0 || x >= tile_map_size_x*6 || y >= tile_map_size_y*6)
			return 1;
		
		add_highlight(x, y, HIGHLIGHT_TYPE_WALKING_DESTINATION);
		
		move_to (x, y);
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
#ifdef DEBUG
				char log[100];
				
				safe_snprintf(log,sizeof(log),"Actor id: %d",object_under_mouse);
				LOG_TO_CONSOLE(c_green1, log);
#endif
				str[0] = GET_PLAYER_INFO;
				*((int *)(str+1)) = SDL_SwapLE32((int)object_under_mouse);
				my_tcp_send (my_socket, str, 5);
				return 1;
			}
			else if (thing_under_the_mouse == UNDER_MOUSE_3D_OBJ)
			{
#ifdef DEBUG
				char log[100];
				
				safe_snprintf(log,sizeof(log),"Object id: %d",object_under_mouse);
				LOG_TO_CONSOLE(c_green1, log);
#endif
				str[0] = LOOK_AT_MAP_OBJECT;
				*((int *)(str+1)) = SDL_SwapLE32((int)object_under_mouse);
				my_tcp_send (my_socket, str, 5);
				return 1;
			}

			break;
		}

		case CURSOR_WAND:
		{
			if(spell_result==2){
				Uint8 str[10];
				short x, y;
		
				if(use_old_clicker)
					get_old_world_x_y();
				else
					get_world_x_y ();

				x = scene_mouse_x / 0.5f;
				y = scene_mouse_y / 0.5f;
				// check to see if the coordinates are OUTSIDE the map
				if (y < 0 || x < 0 || x >= tile_map_size_x*6 || y >= tile_map_size_y*6)
					return 1;
			
				str[0] = MOVE_TO;
				*((short *)(str+1)) = SDL_SwapLE16((short)x);
				*((short *)(str+3)) = SDL_SwapLE16((short)y);
				
				my_tcp_send(my_socket,str,5);
				return 1;
			} else if(spell_result==3){
				Uint8 str[10];
				
				str[0] = TOUCH_PLAYER;
				*((int *)(str+1)) = SDL_SwapLE32((int)object_under_mouse);
				my_tcp_send (my_socket, str, 5);
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
			*((int *)(str+1)) = SDL_SwapLE32((int)object_under_mouse);
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
				*((int *)(str+1)) = SDL_SwapLE32((int)object_under_mouse);
				my_tcp_send (my_socket, str, 5);
				return 1;
			}

			break;
		}

		case CURSOR_ENTER:
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
				*((int *)(str+1)) = SDL_SwapLE32((int)object_under_mouse);
				my_tcp_send (my_socket, str, 5);

				// clear the previous dialogue entries, so we won't have a left over from some other NPC
				for (i=0; i<MAX_RESPONSES; i++)
					dialogue_responces[i].in_use = 0;
				return 1;
			}
			
			str[0] = USE_MAP_OBJECT;
			*((int *)(str+1)) = SDL_SwapLE32((int)object_under_mouse);
			if (use_item != -1 && current_cursor == CURSOR_USE_WITEM)
			{
				*((int *)(str+5)) = SDL_SwapLE32((int)item_list[use_item].pos);
				use_item = -1;
				action_mode = ACTION_WALK;
			} 
			else
			{
				*((int *)(str+5)) = SDL_SwapLE32((int)-1);
			}

			my_tcp_send (my_socket, str, 9);
			return 1;

			break;
		}

		case CURSOR_PICK:
			if (object_under_mouse == -1)
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
			*((Uint16 *)(str+1)) = SDL_SwapLE16((Uint16)object_under_mouse);
			my_tcp_send (my_socket, str, 3);
			return 1;
			break;
		}

		case CURSOR_WALK:
		default:
		{
			short x, y;
		
			if (you_sit && sit_lock && !flag_ctrl)
				return 1;

			if(use_old_clicker)
				get_old_world_x_y();
			else
				get_world_x_y ();

			x = scene_mouse_x / 0.5f;
			y = scene_mouse_y / 0.5f;
			// check to see if the coordinates are OUTSIDE the map
			if (y < 0 || x < 0 || x >= tile_map_size_x*6 || y >= tile_map_size_y*6)
				return 1;
			
			add_highlight(x, y, HIGHLIGHT_TYPE_WALKING_DESTINATION);
		
			move_to (x, y);
			return 1;
		}
	}

	left_click = 2;
	right_click = 2;
	return 1;
}

Uint32 next_fps_time = 0;	// made global so the other modes can keep it from goin stale
int last_count = 0;
int display_game_handler (window_info *win)
{
	static int main_count = 0;
	static int times_FPS_below_3 = 0;
	static int fps[5]={100};
	static int shadows_were_disabled=0;
#ifdef EYE_CANDY
	static int eye_candy_was_disabled=0;
#endif //EYE_CANDY
	unsigned char str[180];
	int i;
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
	last_count++;
	
	//if (quickbar_win>0) windows_list.window[quickbar_win].displayed=1;

	if (fps[0] < 5)
	{
		mouse_rate = 1;
		read_mouse_now = 1;
	}
	else if (fps[0] < 10)
	{
		mouse_rate = 3;
	}
	else if (fps[0] < 20)
	{
		mouse_rate = 6;
	}
	else if (fps[0] < 30)
	{
		mouse_rate = 10;
	}
	else if (fps[0] < 40)
	{
		mouse_rate = 15;
	}
	else
	{
		mouse_rate = 20;
	}
	if(mouse_rate > mouse_limit) {
		mouse_rate = mouse_limit;
	}
	if (!(main_count%mouse_rate)) {
		read_mouse_now = 1;
	} else {
		read_mouse_now = 0;
	}
	reset_under_the_mouse();
	
	// This window is a bit special since it's not fully 2D
	Leave2DMode ();
	glPushMatrix ();

	if (new_zoom_level != zoom_level)
	{
		if (new_zoom_level > zoom_level)
#ifdef	NEW_FRUSTUM
			set_all_intersect_update_needed(main_bbox_tree);
#else
			regenerate_near_objects = regenerate_near_2d_objects = 1;
#endif
		zoom_level = new_zoom_level;
		resize_root_window ();
	}
	
	move_camera ();
	save_scene_matrix ();

	CalculateFrustum ();
#ifdef	NEW_FRUSTUM
	set_click_line();
#endif
	any_reflection = find_reflection ();
	CHECK_GL_ERRORS ();

	// are we actively drawing things?
	if (SDL_GetAppState() & SDL_APPACTIVE)
	{
		get_tmp_actor_data();
	
#ifndef NEW_WEATHER
		//now, determine the current weather light level
		get_weather_light_level ();
#endif

		if (!dungeon){
			draw_global_light ();
		} else {
			draw_dungeon_light ();
		}
		// only draw scene lights if inside or it is night
#ifdef NEW_LIGHTING
		if (
		    ( (use_new_lighting) && (dungeon || !(game_minute >= 5 && game_minute < 235))) ||
		    ((!use_new_lighting) && (dungeon || !is_day))
		   )
#else
		if (dungeon || !is_day)
#endif
		{
			update_scene_lights ();
			draw_lights ();
		}
		CHECK_GL_ERRORS ();

#ifdef NEW_LIGHTING
		if (
		    ( (use_new_lighting) && (!dungeon && shadows_on && (game_minute >= 0 && game_minute < 240))) ||
		    ((!use_new_lighting) && (!dungeon && shadows_on && is_day))
		   )
#else
		if (!dungeon && shadows_on && is_day)
#endif
		{
			render_light_view();
			CHECK_GL_ERRORS ();
		}
		if (weather_use_fog()) render_fog();

		if (any_reflection > 1)
		{
		  	if (!dungeon)
				draw_sky_background ();
		  	else 
				draw_dungeon_sky_background ();
			CHECK_GL_ERRORS ();
			if (show_reflection) display_3d_reflection ();
		}
		CHECK_GL_ERRORS ();
#ifdef ATI_9200_FIX
		glClear(GL_DEPTH_BUFFER_BIT);
#endif

#ifdef NEW_LIGHTING
		if (
		    ( (use_new_lighting) && (!dungeon && shadows_on && (game_minute >= 5 && game_minute < 235))) ||
		    ((!use_new_lighting) && (!dungeon && shadows_on && is_day))
		   )
#else
		if (!dungeon && shadows_on && is_day)
#endif
		{
			glNormal3f(0.0f,0.0f,1.0f);
			if (weather_use_fog() && any_reflection) blend_reflection_fog();
			draw_sun_shadowed_scene (any_reflection);
		}
		else 
		{
			glNormal3f (0.0f,0.0f,1.0f);
			if (any_reflection) {
				blend_reflection_fog();
				draw_lake_tiles ();
			}

			draw_tile_map();
			CHECK_GL_ERRORS ();
			display_2d_objects();
			CHECK_GL_ERRORS();
			anything_under_the_mouse(0, UNDER_MOUSE_NOTHING);
			display_objects();
			display_ground_objects();
			display_actors(1, 0);
			display_alpha_objects();
			display_blended_objects();
		}
		CHECK_GL_ERRORS ();
	}	// end of active display check
	else 
	{
		display_actors (1, 0);	// we need to 'touch' all the actors even if not drawing to avoid problems
	}
	CHECK_GL_ERRORS ();

#if defined(NEW_LIGHTING) || defined(DEBUG_TIME)
	light_idle();
#endif // NEW_LIGHTING

#ifdef	EYE_CANDY
	ec_idle();
#endif
	// if not active, dont bother drawing any more
	if (!(SDL_GetAppState () & SDL_APPACTIVE))
	{
		// remember the time stamp to improve FPS quality when switching modes
		next_fps_time=cur_time+1000;
		last_count=0;
		draw_delay = 20;
		// Return to 2D mode to draw the other windows
		glPopMatrix (); // restore the state
		Enter2DMode ();
		return 1;
	}

	render_weather();	// draw weather effects

	CHECK_GL_ERRORS ();
	//particles should be last, we have no Z writting
	display_particles ();

	CHECK_GL_ERRORS ();
	//we do this because we don't want the rain/particles to mess with our cursor

#ifdef	EYE_CANDY
	ec_draw();
#endif	//EYE_CANDY


	last_texture = -1;

	CHECK_GL_ERRORS ();

	Enter2DMode ();
	//get the FPS, etc

	if (next_fps_time<cur_time){
		fps[4]=fps[3];
		fps[3]=fps[2];
		fps[2]=fps[1];
		fps[1]=fps[0];
		fps[0]=last_count;
		last_count=0;
		next_fps_time=cur_time+1000;
		fps_average=(fps[0]+fps[1]+fps[2]+fps[3]+fps[4])/5.0f;
	}

	if (!no_adjust_shadows)
	{
		if (fps_average < 5.0f)
		{
			times_FPS_below_3++;
			if (times_FPS_below_3 > 10 && (shadows_on
#ifdef EYE_CANDY
					|| use_eye_candy
#endif //EYE_CANDY
					)){
				put_colored_text_in_buffer (c_red1, CHAT_SERVER, (unsigned char*)low_framerate_str, -1);
				times_FPS_below_3 = 0;
				if (shadows_on)
				{
					shadows_on = 0;
					shadows_were_disabled=1;
				}
#ifdef EYE_CANDY
				if (use_eye_candy)
				{
					use_eye_candy = 0;
					eye_candy_was_disabled = 1;
				}
#endif //EYE_CANDY
			}
		}
		else 
		{
			times_FPS_below_3 = 0;
			
			if(shadows_were_disabled){
				shadows_on = 1;
				shadows_were_disabled=0;
			}

#ifdef EYE_CANDY
			if (eye_candy_was_disabled){
				use_eye_candy = 1;
				eye_candy_was_disabled = 0;
			}
#endif //EYE_CANDY
		}
	}
	if (show_fps)
	{
#ifdef	DEBUG
		actor * me = pf_get_our_actor();

		glColor3f (1.0f, 1.0f, 1.0f);
		if(me){
 			safe_snprintf(str,sizeof(str),"Busy: %i",me->busy);
	 		draw_string (400, 4, str, 1);
			safe_snprintf(str,sizeof(str),"Command: %i",me->last_command);
 			draw_string (400, 20, str, 1);
			safe_snprintf(str,sizeof(str),"Coords: %-3i %3i",me->x_tile_pos, me->y_tile_pos);
 			draw_string (550, 4, str, 1);
			safe_snprintf(str,sizeof(str),"Coords: %.3g %.3g",me->x_pos, me->y_pos);
 			draw_string (550, 20, str, 1);
		}
		safe_snprintf (str, sizeof(str),"Lights: %i", show_lights);
		draw_string (win->len_x-hud_x-105, 32, str, 1);

#ifndef NEW_WEATHER
		safe_snprintf(str, sizeof(str), "rain: %d start in: %d stop in: %d drops: %d strength: %1.2f alpha: %1.2f fog alpha: %1.2f", 
				is_raining, seconds_till_rain_starts, seconds_till_rain_stops, num_rain_drops,
				rain_strength_bias, rain_color[3], fogAlpha);
		draw_string (0, win->len_y - hud_y - 40, str, 1);
#endif
#else	//DEBUG
		glColor3f (1.0f, 1.0f, 1.0f);
#endif	//DEBUG
		safe_snprintf ((char*)str, sizeof(str), "FPS: %i", fps[0]);
		draw_string (win->len_x-hud_x-95, 4, str, 1);
#ifdef DEBUG
		//LRNR: stats testing
		safe_snprintf(str, sizeof(str), "E3D:%3d TOT:%3d", e3d_count, e3d_total);
		draw_string (win->len_x-hud_x-183, 19, str, 1);
		e3d_count= e3d_total= 0;
#endif //DEBUG
	}

	CHECK_GL_ERRORS ();
	/* Draw the chat text */
	if (use_windowed_chat != 2)
	{
		int msg, offset, ytext, htext, filter;
		
		ytext = use_windowed_chat == 1 ? 25 : 20;
		htext = (int) (1 + lines_to_show * 18 * chat_zoom);
		filter = use_windowed_chat == 1 ? current_filter : FILTER_ALL;
		if (find_last_lines_time (&msg, &offset, filter, console_text_width))
		{
			set_font(chat_font);	// switch to the chat font
			draw_messages (10, ytext, display_text_buffer, DISPLAY_TEXT_BUFFER_SIZE, filter, msg, offset, -1, console_text_width, htext, chat_zoom);
			set_font (0);	// switch to fixed
		}
	}
	
	anything_under_the_mouse (0, UNDER_MOUSE_NO_CHANGE);
	CHECK_GL_ERRORS ();

	draw_ingame_interface ();
	
	CHECK_GL_ERRORS ();

	Leave2DMode ();


#ifdef SFX
	if(special_effects){
		display_special_effects(1);
	}
#endif //SFX
	display_highlight_markers();
	
	glEnable (GL_LIGHTING);

	// Return to 2D mode to draw the other windows
	glPopMatrix ();	// restore the state
	Enter2DMode ();

	return 1;
}

int check_quit_or_fullscreen (Uint32 key)
{
	int alt_on = key & ELW_ALT;
	Uint16 keysym = key & 0xffff;
#ifndef WINDOWS
	int ctrl_on = key & ELW_CTRL;

	// first, try to see if we pressed Alt+x or Ctrl+q, to quit.
	if ( (keysym == SDLK_x && alt_on) || (keysym == SDLK_q && ctrl_on && !alt_on) )
#else
	// Windows SDL reports [Alt Gr] as [Ctrl], which hinders German users typing '@'
	if ( (keysym == SDLK_x && alt_on) )
#endif
	{
		exit_now = 1;
	}
	else if (keysym == SDLK_RETURN && alt_on)
	{
		toggle_full_screen ();
	}
	else
	{
		return 0;
	}

	return 1;
}

Uint8 key_to_char (Uint32 unikey)
{
	if ( (unikey >= 256 && unikey <= 267) || unikey==271)
	{
		switch (unikey)
		{
			case 266:
				return 46;
			case 267:
				return 47;
			case 271:
				return 13;
			default:
				return (unikey-208)&0xff;
		}
	}
	
	return unikey & 0xff;
}

// keypress handler common to all in-game root windows (game_root_win, 
// console_root_win, and map_root_win)
int keypress_root_common (Uint32 key, Uint32 unikey)
{
	static Uint32 last_turn_around = 0;
	int alt_on = key & ELW_ALT;
	int ctrl_on = key & ELW_CTRL;
	int shift_on = key & ELW_SHIFT;
	Uint16 keysym = key & 0xffff;
#ifdef DEBUG
	int i;
	Uint32 _cur_time= SDL_GetTicks();
#endif
	
	if(check_quit_or_fullscreen(key))
	{
		return 1;
	}
	else if((keysym == SDLK_UP || keysym == SDLK_DOWN) && ctrl_on)
	{
		char *line;

		if(keysym == SDLK_UP)
		{
			line = history_get_line_up();
		}
		else
		{
			line= history_get_line_down();
		}
		if(line != NULL)
		{
			put_string_in_input_field((unsigned char*)line);
		}
	}
	else if(disconnected && !alt_on && !ctrl_on && !locked_to_console)
	{
		connect_to_server();
	}
	else if((keysym == SDLK_v && ctrl_on) || (keysym == SDLK_INSERT && shift_on))
	{
#ifndef WINDOWS
		startpaste ();
#else
		windows_paste ();
#endif
	}
#ifdef DEBUG
	else if((keysym == SDLK_LEFT) && shift_on && ctrl_on && !alt_on)
	{
		for (i=0; i<ITEM_WEAR_START;i++) {
			item_list[i].cooldown_rate = 60000;
			item_list[i].cooldown_time = _cur_time + 60000;
		}
	}
	else if((keysym == SDLK_DOWN) && shift_on && ctrl_on && !alt_on)
	{
		for(i=0; i<ITEM_WEAR_START;i++) {
			item_list[i].cooldown_time -= 1000;
		}
	}
	else if((keysym == SDLK_UP) && shift_on && ctrl_on && !alt_on)
	{
		for(i=0; i<ITEM_WEAR_START;i++) {
			item_list[i].cooldown_time += 1000;
		}
	}
#ifndef NEW_WEATHER
	else if((keysym == SDLK_UP) && shift_on && ctrl_on && !alt_on)
	{
		rain_color[3] += 0.01f;
	}
	else if((keysym == SDLK_DOWN) && shift_on && ctrl_on && !alt_on)
	{
		rain_color[3] -= 0.01f;
	}
#endif
	else if((keysym == SDLK_t) && shift_on && ctrl_on && !alt_on)
	{
		add_thunder(rand()%5,1+rand()%5);
	}
	else if((keysym == SDLK_w) && shift_on && ctrl_on && !alt_on)
	{
		if(game_minute >= 355) game_minute -=355; else game_minute +=  5;
		new_minute();
	}
	else if((keysym == SDLK_q) && shift_on && ctrl_on && !alt_on)
	{
		if(game_minute <  5) game_minute +=355; else game_minute -=  5;
		new_minute();
	}
#ifndef NEW_WEATHER
	else if((keysym == SDLK_PAGEUP) && shift_on && ctrl_on && !alt_on)
	{
		rain_strength_bias += 0.1f;
	}
	else if((keysym == SDLK_PAGEDOWN) && shift_on && ctrl_on && !alt_on)
	{
		rain_strength_bias -= 0.1f;
	}
#endif
	else if((keysym == SDLK_HOME) && shift_on && ctrl_on && !alt_on)
	{
#ifdef NEW_WEATHER
		start_weather(30, 1.0f);
#else
		if(is_raining) {
			seconds_till_rain_stops = 2;
			seconds_till_rain_starts = -1;
		} else {
			seconds_till_rain_stops = -1;
			seconds_till_rain_starts = 2;
		}
#endif
	}
	else if ((keysym == SDLK_END) && shift_on && ctrl_on && !alt_on)
	{
#ifdef NEW_WEATHER
		stop_weather(30, 1.0f);
#else
		if (is_raining) {
			seconds_till_rain_stops = 90;
			seconds_till_rain_starts = -1;
		} else {
			seconds_till_rain_stops = -1;
			seconds_till_rain_starts = 90;
		}
#endif
	}
#endif
	// use quickbar items
	else if (key == K_ITEM1)
	{
		quick_use (0);
	}
	else if (key == K_ITEM2)
	{
		quick_use (1);
	}
	else if (key == K_ITEM3)
	{
		quick_use (2);
	}
	else if (key == K_ITEM4)
	{
		quick_use (3);
	}
	else if (key == K_ITEM5)
	{
		quick_use (4);
	}
	else if (key == K_ITEM6)
	{
		quick_use(5);
	}
	else if (key == K_SPELL1)
	{
		if(mqb_data[1] && mqb_data[1]->spell_str[0]) {
			my_tcp_send(my_socket, mqb_data[1]->spell_str, 13);
		}
	}
	else if (key == K_SPELL2)
	{
		if(mqb_data[2] && mqb_data[2]->spell_str[0]) {
			my_tcp_send(my_socket, mqb_data[2]->spell_str, 13);
		}
	}
	else if (key == K_SPELL3)
	{
		if(mqb_data[3] && mqb_data[3]->spell_str[0]) {
			my_tcp_send(my_socket, mqb_data[3]->spell_str, 13);
		}
	}
	else if (key == K_SPELL4)
	{
		if(mqb_data[4] && mqb_data[4]->spell_str[0]) {
			my_tcp_send(my_socket, mqb_data[4]->spell_str, 13);
		}
	}
	else if (key == K_SPELL5)
	{
		if(mqb_data[5] && mqb_data[5]->spell_str[0]) {
			my_tcp_send(my_socket, mqb_data[5]->spell_str, 13);
		}
	}
	else if (key == K_SPELL6)
	{
		if(mqb_data[6] && mqb_data[6]->spell_str[0]) {
			my_tcp_send(my_socket, mqb_data[6]->spell_str, 13);
		}
	}
	// Okay, let's move, even when in console or map mode
	else if (key == K_TURNLEFT)
	{
		if (!last_turn_around || last_turn_around + 500 < cur_time)
		{
			Uint8 str[2];
			last_turn_around = cur_time;
			str[0] = TURN_LEFT;
			my_tcp_send (my_socket, str, 1);
		}
	}
	else if (key == K_TURNRIGHT)
	{
		if (!last_turn_around || last_turn_around + 500 < cur_time)
		{
			Uint8 str[2];
			last_turn_around = cur_time;
			str[0] = TURN_RIGHT;
			my_tcp_send (my_socket, str, 1);
		}
	}
	else if (key==K_ADVANCE)
	{
		move_self_forward();
	}
	// hide all windows
	else if(key==K_HIDEWINS)
	{
		if (ground_items_win >= 0)
			hide_window (ground_items_win);
		if (items_win >= 0)
			hide_window (items_win);
		if (buddy_win >= 0)
			hide_window (buddy_win);
		if (manufacture_win >= 0)
			hide_window (manufacture_win);
		if (elconfig_win >= 0)
			hide_window (elconfig_win);
		if (sigil_win >= 0)
			hide_window (sigil_win);
		if (tab_stats_win >= 0)
			hide_window (tab_stats_win);
		if (tab_help_win >= 0)
			hide_window (tab_help_win);
		if (storage_win >= 0)
			hide_window (storage_win);
		if (dialogue_win >= 0)
			hide_window (dialogue_win);
		if (server_popup_win >= 0)
			hide_window (server_popup_win );
#ifdef NOTEPAD
		if (notepad_win >= 0)
			hide_window (notepad_win);
#endif
	}
	// toggle options
	else if (key == K_HEALTHBAR)
	{
		view_health_bar = !view_health_bar;
	}
	else if (key == K_VIEWTEXTASOVERTEXT)
	{
		view_chat_text_as_overtext = !view_chat_text_as_overtext;
	}
	else if (key == K_VIEWNAMES)
	{
		view_names = !view_names;
	}
	else if (key == K_VIEWHP)
	{
		view_hp = !view_hp;
	}
	else if (key == K_SHADOWS)
	{
		clouds_shadows = !clouds_shadows;
	}
	// open or close windows
	else if (key == K_STATS)
	{
		view_tab (&tab_stats_win, &tab_stats_collection_id, STATS_TAB_STATS);
	}
	else if (key == K_OPTIONS)
	{
		view_window (&elconfig_win, 0);
	}
	else if (key == K_KNOWLEDGE)
	{
		view_tab (&tab_stats_win, &tab_stats_collection_id, STATS_TAB_KNOWLEDGE);
	}
	else if (key == K_ENCYCLOPEDIA)
	{
		view_tab (&tab_help_win, &tab_help_collection_id, HELP_TAB_ENCYCLOPEDIA);
	}
	else if (key == K_HELP)
	{
		view_tab(&tab_help_win, &tab_help_collection_id, HELP_TAB_HELP);
	}
	else if (key == K_RULES)
	{
		view_tab(&tab_help_win, &tab_help_collection_id, HELP_TAB_RULES);
	}
#ifdef NOTEPAD
	else if (key == K_NOTEPAD)
	{
		view_window (&notepad_win, 0);
	}
#endif
#ifdef  MINIMAP
	else if(key == K_MINIMAP)
	{
		view_window (&minimap_win, 0);
	}
#endif  //MINIMAP
	else if (key == K_SIGILS)
	{
		view_window (&sigil_win, -1);
	}
	else if (key == K_MANUFACTURE)
	{
		view_window (&manufacture_win, -1);
	}
	else if(key == K_ITEMS)
	{
		view_window (&items_win, -1);
	}
	else if(key == K_BUDDY)
	{
		display_buddy();
	}
	// set action modes
	else if (key == K_WALK)
	{
		item_action_mode = qb_action_mode = action_mode = ACTION_WALK;
	}
	else if (key == K_LOOK)
	{
		item_action_mode = qb_action_mode = action_mode = ACTION_LOOK;
	}
	else if (key == K_USE)
	{
		item_action_mode = qb_action_mode = action_mode = ACTION_USE;
	}
	// Roja likes to rotate the camera while in console mode :)
	else if (key == K_ROTATELEFT)
	{
		camera_rotation_speed = normal_camera_rotation_speed / 40;
		camera_rotation_frames = 40;
	}
	else if (key == K_FROTATELEFT)
	{
		camera_rotation_speed = fine_camera_rotation_speed / 10;
		camera_rotation_frames = 10;
	}
	else if (key == K_ROTATERIGHT)
	{
		camera_rotation_speed = -normal_camera_rotation_speed / 40;
		camera_rotation_frames = 40;
	}
	else if (key == K_FROTATERIGHT)
	{
		camera_rotation_speed = -fine_camera_rotation_speed / 10;
		camera_rotation_frames = 10;
	}
	else if (key == K_AFK)
	{
		if (!afk) 
		{
			go_afk ();
			last_action_time = cur_time - afk_time;
		}
		else
		{
			go_ifk ();
		}
	}
	else if (key == K_SIT)
	{
		sit_button_pressed (NULL, 0);
	}
	else if (key == K_BROWSER)
	{
		if (have_url)
		{
			open_web_link(current_url);
		}
	}
	else if (keysym == SDLK_ESCAPE)
	{
		root_key_to_input_field (key, unikey);
	}
	else if(key == K_NEXT_CHAT_TAB)
	{
		int next_tab;
		widget_list *widget;
		tab_collection *collection;
		switch(use_windowed_chat)
		{
			case 1: //Tabs
				if(current_tab == tabs_in_use-1)
				{
					next_tab = 2;
				}
				else
				{
					next_tab = current_tab + 1;
				}
				switch_to_tab(next_tab);
			break;
			case 2: //Window
				widget = widget_find(chat_win, chat_tabcollection_id);
				collection = widget->widget_info;
				if(active_tab == collection->nr_tabs - 1)
				{
					next_tab = 2;
				}
				else
				{
					next_tab = active_tab + 1;
				}
				switch_to_chat_tab(channels[next_tab].tab_id, 0);
			break;
			default:
				return 0;
			break;
		}
	}
	else if(key == K_PREV_CHAT_TAB)
	{
		int next_tab;
		widget_list *widget;
		tab_collection *collection;
		switch(use_windowed_chat)
		{
			case 1: //Tab
				if(current_tab == 2)
				{
					next_tab = tabs_in_use-1;
				}
				else
				{
					next_tab = current_tab-1;
				}
				switch_to_tab(next_tab);
				break;
			case 2: //Window
				widget = widget_find(chat_win, chat_tabcollection_id);
				collection = widget->widget_info;
				if(active_tab == 2)
				{
					next_tab = collection->nr_tabs - 1;
				}
				else
				{
					next_tab = active_tab - 1;
				}
				switch_to_chat_tab(channels[next_tab].tab_id, 0);
			break;
			default:
				return 0;
			break;
		}
	}
	else if (key == K_WINDOWS_ON_TOP)
	{
		change_windows_on_top(&windows_on_top);
	}
#ifdef PNG_SCREENSHOT
	else if (key == K_SCREENSHOT)
	{
		makeScreenShot();
	}
#endif
	else
	{
		return 0; // nothing we can handle
	}

	return 1; // we handled it
}

int text_input_handler (Uint32 key, Uint32 unikey)
{
	Uint8 ch = key_to_char (unikey);

	if (root_key_to_input_field(key, unikey))
	{
		return 1;
	}
	else if (IS_PRINT(ch) && input_text_line.len < MAX_TEXT_MESSAGE_LENGTH)
	{
		if(put_char_in_buffer (&input_text_line, ch, input_text_line.len)) {
			if(input_widget) {
				text_field *tf = input_widget->widget_info;
				tf->cursor = tf->buffer->len;
				if(input_widget->window_id == game_root_win) {
					widget_unset_flag(input_widget->window_id, input_widget->id, WIDGET_DISABLED);
				}
			}
		}
	}
#ifndef OSX
	else if (ch == SDLK_BACKSPACE && input_text_line.len > 0)
#else
	else if (((ch == SDLK_BACKSPACE) || (ch == 127)) && input_text_line.len > 0)
#endif
	{
		input_text_line.len--;
		if (input_text_line.data[input_text_line.len] == '\n'
		|| input_text_line.data[input_text_line.len] == '\r') {
			input_text_line.len--;
		}
		input_text_line.data[input_text_line.len] = '\0';
	}
	else if (ch == SDLK_RETURN && input_text_line.len > 0)
	{
		if (input_text_line.data[0] == '%' && input_text_line.len > 1) 
		{
			if ( (check_var (&(input_text_line.data[1]), IN_GAME_VAR) ) < 0)
				send_input_text_line (input_text_line.data, input_text_line.len);
		}
		else if (input_text_line.len > 5 && input_text_line.data[0] == '@' && input_text_line.data[1] == '@' && input_text_line.data[2] != ' ')
		{
			chan_target_name(input_text_line.data, input_text_line.len);
		}
		else if ( input_text_line.data[0] == '#' || input_text_line.data[0] == char_cmd_str[0] )
		{
			test_for_console_command (input_text_line.data, input_text_line.len);
		}
		else
		{
			if(input_text_line.data[0] == char_at_str[0])
				input_text_line.data[0]='@';
			send_input_text_line (input_text_line.data, input_text_line.len);
		}
		add_line_to_history(input_text_line.data, input_text_line.len);
		// also clear the buffer
		clear_input_line();
	}
	else
	{
		// no clue what to do with this character
		return 0;
	}
	return 1;
}

int keypress_game_handler (window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
	Uint16 keysym = key & 0xffff;

	// first try the keypress handler for all root windows
	if ( keypress_root_common (key, unikey) )
	{
		return 1;
	}
	else if (key == K_TABCOMPLETE && input_text_line.len > 0)
	{
		do_tab_complete(&input_text_line);
	}
	else if (key == K_CAMERAUP)
	{
		if (rx > -60) rx -= 1.0f;
	}
	else if (key == K_CAMERADOWN)
	{
		if (rx < -45) rx += 1.0f;
	}
	else if (key == K_ZOOMIN)
	{
		new_zoom_level= new_zoom_level - 0.25;
		if (new_zoom_level < 1.00f) new_zoom_level= 1.00f;
	}
	else if (key == K_ZOOMOUT)
	{
		new_zoom_level= new_zoom_level + 0.25;
		if (new_zoom_level > 3.75f) new_zoom_level= 3.75f;
	}
	else if (key == K_REPEATSPELL)	// REPEAT spell command
	{
		if ( get_show_window (sigil_win) && !get_show_window (trade_win) )
		{
			repeat_spell();
		}
	}
	else if ((key == K_MAP) || (key == K_MARKFILTER))
 	{
		// if K_MARKFILTER pressed, open the map window with the filter active
		if (key == K_MARKFILTER)
			mark_filter_active = 1;
		if ( switch_to_game_map () )
		{
			hide_window (game_root_win);
			show_window (map_root_win);
		}
	}
	// TEST REMOVE LATER!!!!!!!!!!!!!!!!!!!!!!
	else if (keysym == SDLK_F7)
	{
		if (ctrl_on) 
			read_local_book ("books/abc.xml\0", 22);
		else if (shift_on)
			read_local_book ("books/sediculos.xml\0", 22);
	}
	else if (keysym == SDLK_F9)
	{
		actor *me = get_actor_ptr_from_id (yourself);
#ifdef	NEW_FRUSTUM
 #ifdef EYE_CANDY
		ec_create_campfire(me->x_pos + 0.25f, me->y_pos + 0.25f, -2.3f + height_map[me->y_tile_pos*tile_map_size_x*6+me->x_tile_pos]*0.2f + 0.1f, (poor_man ? 6 : 10), 0.7);
 #else // EYE_CANDY
  #ifdef SFX
		add_particle_sys ("./particles/fire_small.part", me->x_pos + 0.25f, me->y_pos + 0.25f, -2.2f + height_map[me->y_tile_pos*tile_map_size_x*6+me->x_tile_pos]*0.2f + 0.1f, 1);
  #endif // SFX
 #endif // EYE_CANDY
#else // NEW_FRUSTUM
 #ifdef EYE_CANDY
		ec_create_campfire(me->x_pos + 0.25f, me->y_pos + 0.25f, -2.3f + height_map[me->y_tile_pos*tile_map_size_x*6+me->x_tile_pos]*0.2f + 0.1f, (poor_man ? 6 : 10), 0.7);
 #else // EYE_CANDY
  #ifdef SFX
		add_particle_sys ("./particles/fire_small.part", me->x_pos + 0.25f, me->y_pos + 0.25f, -2.2f + height_map[me->y_tile_pos*tile_map_size_x*6+me->x_tile_pos]*0.2f + 0.1f);
  #endif // SFX
 #endif // EYE_CANDY
#endif // NEW_FRUSTUM
	}
	else if (keysym == SDLK_F6)
	{
		if(!hud_x)
		{
			hud_x = HUD_MARGIN_X;
			hud_y = HUD_MARGIN_Y;
		}
		else
		{
			hud_x=0;
			hud_y=0;
		}
		resize_root_window ();
	}
#ifdef DEBUG
	else if (keysym == SDLK_F10)
	{
		if (key & ELW_SHIFT)
		{
#ifndef NEW_SOUND
			print_sound_objects ();
#endif //!NEW_SOUND
		}
		else if (key & ELW_ALT)
		{
			print_filter_list ();
		}
		else
		{
			int iwin;
			widget_list *l;
			for (iwin = 0; iwin < windows_list.num_windows; iwin++)
			{
				printf ("%s: id = %d, order = %d, parent = %d, pos = (%d, %d), cur_pos = (%d, %d), displayed = %d\n", windows_list.window[iwin].window_name, windows_list.window[iwin].window_id, windows_list.window[iwin].order, windows_list.window[iwin].pos_id, windows_list.window[iwin].pos_x, windows_list.window[iwin].pos_y, windows_list.window[iwin].cur_x, windows_list.window[iwin].cur_y, windows_list.window[iwin].displayed);
				for (l = windows_list.window[iwin].widgetlist; l; l = l->next)
					printf ("\t%d\n", l->id);
			}
		}
	}
#endif //DEBUG
	// END OF TEST!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	else
	{
		Uint8 ch = key_to_char (unikey);

		reset_tab_completer();
		if (ch == '`' || key == K_CONSOLE)
		{
			hide_window (game_root_win);
			show_window (console_root_win);
		}
		// see if the common text handler can deal with it
		else if ( !text_input_handler (key, unikey) )
		{
			// nothing we can handle
			return 0;
		}
	}
	
	// we handled it, return 1 to let the window manager know
	return 1;
}

int show_game_handler (window_info *win) {
	text_field *tf = input_widget->widget_info;
	init_hud_interface(1);
	show_hud_windows();

	if (use_windowed_chat == 2) {
		window_info *win;
		/* Put the input widget back into the chat window */
		widget_move_win(input_widget->window_id, input_widget->id, chat_win);
		win = &windows_list.window[chat_win];
		resize_chat_handler(win, win->len_x, win->len_y);
	} else {
		if (use_windowed_chat == 1) {
			display_tab_bar();
		}
		widget_move_win(input_widget->window_id, input_widget->id, game_root_win);
		widget_resize (input_widget->window_id, input_widget->id, win->len_x-HUD_MARGIN_X, input_widget->len_y);
		input_widget->OnResize = input_field_resize;
		widget_move (input_widget->window_id, input_widget->id, 0, win->len_y-input_widget->len_y-HUD_MARGIN_Y);
		widget_set_flags(input_widget->window_id, input_widget->id, INPUT_DEFAULT_FLAGS);
	}
	if(input_widget->window_id == game_root_win) {
		if(tf->buffer->len > 0) {
			widget_unset_flag(input_widget->window_id, input_widget->id, WIDGET_DISABLED);
		} else {
			widget_set_flags(input_widget->window_id, input_widget->id, input_widget->Flags|WIDGET_DISABLED);
		}
	}
	return 1;
}

void create_game_root_window (int width, int height)
{
	if (game_root_win < 0)
	{
		game_root_win = create_window ("Game", -1, -1, 0, 0, width, height, ELW_TITLE_NONE|ELW_SHOW_LAST);
		
		set_window_handler (game_root_win, ELW_HANDLER_DISPLAY, &display_game_handler);
		set_window_handler (game_root_win, ELW_HANDLER_CLICK, &click_game_handler);
		set_window_handler (game_root_win, ELW_HANDLER_MOUSEOVER, &mouseover_game_handler);
		set_window_handler (game_root_win, ELW_HANDLER_KEYPRESS, &keypress_game_handler);
		set_window_handler (game_root_win, ELW_HANDLER_SHOW, &show_game_handler);
		set_window_handler (game_root_win, ELW_HANDLER_AFTER_SHOW, &update_have_display);
		set_window_handler (game_root_win, ELW_HANDLER_HIDE, &update_have_display);

		if(input_widget == NULL) {
			Uint32 id;
			id = text_field_add_extended(game_root_win, 42, NULL, 0, height-INPUT_HEIGHT-hud_y, width-hud_x, INPUT_HEIGHT, INPUT_DEFAULT_FLAGS, chat_zoom, 0.77f, 0.57f, 0.39f, &input_text_line, 1, FILTER_ALL, INPUT_MARGIN, INPUT_MARGIN, 1.0, 1.0, 1.0);
			input_widget = widget_find(game_root_win, id);
			input_widget->OnResize = input_field_resize;
		} else {
			widget_move_win(input_widget->window_id, input_widget->id, game_root_win);
			widget_resize (input_widget->window_id, input_widget->id, width-HUD_MARGIN_X, input_widget->len_y);
			input_widget->OnResize = input_field_resize;
			widget_move (input_widget->window_id, input_widget->id, 0, height-input_widget->len_y-HUD_MARGIN_Y);
			widget_set_flags(input_widget->window_id, input_widget->id, INPUT_DEFAULT_FLAGS);
		}
		widget_set_OnKey(input_widget->window_id, input_widget->id, chat_input_key);
		if(input_text_line.len > 0) {
			widget_unset_flag(input_widget->window_id, input_widget->id, WIDGET_DISABLED);
		}
		resize_root_window();
	}
}
