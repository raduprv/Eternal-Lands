#include <stdlib.h>
#include <string.h>
#include <SDL/SDL_keysym.h>
#include "gamewin.h"
#include "2d_objects.h"
#include "3d_objects.h"
#include "actor_scripts.h"
#include "achievements.h"
#include "asc.h"
#include "bags.h"
#include "books.h"
#include "buddy.h"
#include "chat.h"
#ifdef CLUSTER_INSIDES
#include "cluster.h"
#endif // CLUSTER_INSIDES
#include "console.h"
#include "consolewin.h"
#include "context_menu.h"
#include "cursors.h"
#include "dialogues.h"
#include "draw_scene.h"
#include "e3d.h"
#include "elconfig.h"
#include "events.h"
#include "filter.h"
#include "gl_init.h"
#include "global.h"
#include "highlight.h"
#include "hud.h"
#include "icon_window.h"
#include "init.h"
#include "interface.h"
#include "items.h"
#include "lights.h"
#include "manufacture.h"
#include "map.h"
#include "mapwin.h"
#include "multiplayer.h"
#include "particles.h"
#include "paste.h"
#include "pathfinder.h"
#include "pm_log.h"
#include "questlog.h"
#include "reflection.h"
#include "serverpopup.h"
#include "shadows.h"
#include "spells.h"
#include "storage.h"
#include "tabs.h"
#include "textures.h"
#include "tiles.h"
#include "trade.h"
#include "translate.h"
#include "url.h"
#include "weather.h"
#ifdef DEBUG
#include "sound.h"
#include "special_effects.h"
#endif
#include "special_effects.h"
#include "eye_candy_wrapper.h"
#include "minimap.h"
#ifdef PAWN
#include "pawn/elpawn.h"
#endif
#include "sky.h"
#include "missiles.h"
#ifdef ECDEBUGWIN
#include "eye_candy_debugwin.h"
#endif
#include "actor_init.h"
#include "emotes.h"

int game_root_win = -1;
int gamewin_in_id = 4442;
int use_old_clicker=0;
float fps_average = 100.0;
int include_use_cursor_on_animals = 0;
int logo_click_to_url = 1;
char LOGO_URL_LINK[128] = "http://www.eternal-lands.com";
int have_mouse = 0;
int just_released_mouse = 0;
int keep_grabbing_mouse = 0;
#ifdef NEW_CURSOR
int cursors_tex;
#endif // NEW_CURSOR
#ifdef  DEBUG
extern int e3d_count, e3d_total;    // LRNR:stats testing only
#endif  //DEBUG
int cm_banner_disabled = 0;
int ranging_lock = 0;
int auto_disable_ranging_lock = 1;

void draw_special_cursors()
{
	const float RET_WID = 4.0f;
	const float RET_LEN = 10.0f;
	float ret_x = 0.0, ret_y = 0.0;

	float ret_spin,ret_zoom, ret_alpha=0.5f;
	float ret_color[4];
	float ret_out = 7.0;

#ifdef NEW_CURSOR
	if (!have_mouse && sdl_cursors) return;
#else // NEW_CURSOR
	if (!have_mouse) return;
#endif // NEW_CURSOR

	if(!(SDL_GetAppState() & SDL_APPMOUSEFOCUS)) return;

	switch (current_cursor){
	case (CURSOR_ATTACK):
		ret_zoom = 2.0f;
		ret_spin = (cur_time%2000)*360.0f/2000.0f;
		ret_color[0]=1.0f;
		ret_color[1]=0.0f;
		ret_color[2]=0.0f;
		ret_color[3]=ret_alpha;
		break;
	case (CURSOR_WAND):
		ret_spin = 0.0f;
		ret_zoom = (sin((cur_time%1000)*3.1415/1000.0)+1.0)*6.0;
		ret_color[0]=0.0f;
		ret_color[1]=0.0f;
		ret_color[2]=1.0f;
		ret_color[3]=ret_alpha;
		ret_out=15.0f;
		break;
	default:
		ret_spin = 45.0f;
		ret_zoom = 3.0f;
		ret_color[0]=0.0f;
		ret_color[1]=1.0f;
		ret_color[2]=0.0f;
		ret_color[3]=ret_alpha;
	}
	
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glPushMatrix();
	glTranslatef(mouse_x, mouse_y, 0.0);
#ifdef NEW_CURSOR
	glScalef(pointer_size, pointer_size, 1.0);
#endif // NEW_CURSOR

	//printf("mouse_x=%d mouse_y=%d\n", mouse_x, mouse_y);

	if(have_mouse)
	{
#ifdef NEW_CURSOR
		glColor4f(1,1,1,1);
		get_and_set_texture_id(cursors_tex);
		if (big_cursors /* && !sdl_cursors */) {
			float x = (current_cursor%8)/8.0;
			float y = (1-current_cursor/8 + 5)/8.0;
			glBegin(GL_QUADS);
			glTexCoord2f(x,y);					glVertex2f(0,32);
			glTexCoord2f(x+0.125f,y);			glVertex2f(32,32);
			glTexCoord2f(x+0.125f,y+0.125f);	glVertex2f(32,0);
			glTexCoord2f(x,y+0.125f);			glVertex2f(0,0);
			glEnd();
		} else /* if (!sdl_cursors) */ {
			glBegin(GL_QUADS);
			glTexCoord2f(current_cursor/16.0,15.0/16.0);		glVertex2f(10,26);
			glTexCoord2f((current_cursor+1.0)/16.0,15.0/16.0);	glVertex2f(26,26);
			glTexCoord2f((current_cursor+1.0)/16.0,16.0/16.0);	glVertex2f(26,10);
			glTexCoord2f(current_cursor/16.0,16.0/16.0);		glVertex2f(10,10);
			glEnd();
		}
#endif // NEW_CURSOR

		glRotatef(ret_spin, 0.0, 0.0, 1.0);
		glColor4fv(ret_color);
		glDisable(GL_TEXTURE_2D);

		ret_x += ret_zoom;
		glBegin(GL_TRIANGLES);
		
		glVertex2f(ret_x,ret_y);
		glVertex2f(ret_x+RET_LEN,ret_y-RET_WID);
		glVertex2f(ret_x+ret_out,ret_y);
		
		glVertex2f(ret_x,ret_y);
		glVertex2f(ret_x+RET_LEN,ret_y+RET_WID);
		glVertex2f(ret_x+ret_out,ret_y);
		
		ret_x -= ret_zoom*2;
		
		glVertex2f(ret_x,ret_y);
		glVertex2f(ret_x-RET_LEN,ret_y-RET_WID);
		glVertex2f(ret_x-ret_out,ret_y);
		
		glVertex2f(ret_x,ret_y);
		glVertex2f(ret_x-RET_LEN,ret_y+RET_WID);
		glVertex2f(ret_x-ret_out,ret_y);
		
		ret_x += ret_zoom;
		ret_y -= ret_zoom;
		
		glVertex2f(ret_x,ret_y);
		glVertex2f(ret_x-RET_WID,ret_y-RET_LEN);
		glVertex2f(ret_x,ret_y-ret_out);
		
		glVertex2f(ret_x,ret_y);
		glVertex2f(ret_x+RET_WID,ret_y-RET_LEN);
		glVertex2f(ret_x,ret_y-ret_out);
		
		ret_y += ret_zoom*2;
		
		glVertex2f(ret_x,ret_y);
		glVertex2f(ret_x-RET_WID,ret_y+RET_LEN);
		glVertex2f(ret_x,ret_y+ret_out);
		
		glVertex2f(ret_x,ret_y);
		glVertex2f(ret_x+RET_WID,ret_y+RET_LEN);
		glVertex2f(ret_x,ret_y+ret_out);
		
		glEnd();
		ret_y -= ret_zoom;
		
		glColor4f(0.0,0.0,0.0,ret_alpha);
		
		ret_x += ret_zoom;
		glBegin(GL_LINE_LOOP);       
		glVertex2f(ret_x,ret_y);
		glVertex2f(ret_x+RET_LEN,ret_y-RET_WID);
		glVertex2f(ret_x+ret_out,ret_y);
		glVertex2f(ret_x+RET_LEN,ret_y+RET_WID);
		glEnd();
		ret_x -= ret_zoom*2;
		glBegin(GL_LINE_LOOP);
		glVertex2f(ret_x,ret_y);
		glVertex2f(ret_x-RET_LEN,ret_y-RET_WID);
		glVertex2f(ret_x-ret_out,ret_y);
		glVertex2f(ret_x-RET_LEN,ret_y+RET_WID);
		glEnd();
		
		ret_x += ret_zoom;
		ret_y -= ret_zoom;
		glBegin(GL_LINE_LOOP);
		glVertex2f(ret_x,ret_y);
		glVertex2f(ret_x-RET_WID,ret_y-RET_LEN);
		glVertex2f(ret_x,ret_y-ret_out);
		glVertex2f(ret_x+RET_WID,ret_y-RET_LEN);
		glEnd();
		
		ret_y += ret_zoom*2;
		glBegin(GL_LINE_LOOP);       
		glVertex2f(ret_x,ret_y);
		glVertex2f(ret_x-RET_WID,ret_y+RET_LEN);
		glVertex2f(ret_x,ret_y+7);
		glVertex2f(ret_x+RET_WID,ret_y+RET_LEN);
		glEnd();
		ret_y -= ret_zoom;
	}
#ifdef NEW_CURSOR
	else
#endif // NEW_CURSOR
#ifdef NEW_CURSOR
	{
		if (current_cursor != CURSOR_ARROW){
			glColor4fv(ret_color);
			glRotatef(-135.0,0,0,1);
			glDisable(GL_TEXTURE_2D);
			glBegin(GL_TRIANGLES);
			glVertex2f(ret_x,ret_y);
			glVertex2f(ret_x-RET_LEN,ret_y-RET_WID);
			glVertex2f(ret_x-ret_out,ret_y);
			
			glVertex2f(ret_x,ret_y);
			glVertex2f(ret_x-RET_LEN,ret_y+RET_WID);
			glVertex2f(ret_x-ret_out,ret_y);
			glEnd();
			
			glColor4f(0.0,0.0,0.0,ret_alpha);
			
			glBegin(GL_LINE_LOOP);
			glVertex2f(ret_x,ret_y);
			glVertex2f(ret_x-RET_LEN,ret_y-RET_WID);
			glVertex2f(ret_x-ret_out,ret_y);
			glVertex2f(ret_x-RET_LEN,ret_y+RET_WID);
			glEnd();
			glRotatef(135.0,0,0,1);
			glEnable(GL_TEXTURE_2D);
		}

		glColor4f(1,1,1,1);
		get_and_set_texture_id(cursors_tex);
		if(big_cursors){
			float x = (current_cursor%8)/8.0;
			float y = (1-current_cursor/8 + 5)/8.0;
			glBegin(GL_QUADS);
			glTexCoord2f(x,y);					glVertex2f(0,32);
			glTexCoord2f(x+0.125f,y);			glVertex2f(32,32);
			glTexCoord2f(x+0.125f,y+0.125f);	glVertex2f(32,0);
			glTexCoord2f(x,y+0.125f);			glVertex2f(0,0);
			glEnd();
		} else {
			glBegin(GL_QUADS);
			glTexCoord2f(current_cursor/16.0,14.0/16.0);		glVertex2f(0,16);
			glTexCoord2f((current_cursor+1.0)/16.0,14.0/16.0);	glVertex2f(16,16);
			glTexCoord2f((current_cursor+1.0)/16.0,15.0/16.0);	glVertex2f(16,0);
			glTexCoord2f(current_cursor/16.0,15.0/16.0);		glVertex2f(0,0);
			glEnd();
		}
	}
#endif // NEW_CURSOR

	glPopMatrix();
	glPopAttrib();
	reset_material();
}

void toggle_have_mouse()
{
	have_mouse = !have_mouse;
	if(have_mouse){
		SDL_WM_GrabInput(SDL_GRAB_ON);
#ifdef NEW_CURSOR
		if (sdl_cursors)
#endif // NEW_CURSOR
			SDL_ShowCursor(0);
		if (fol_cam) toggle_follow_cam(&fol_cam);
		LOG_TO_CONSOLE (c_red1, "Grab mode: press alt+g again to enter Normal mode.");
	} else {
		SDL_WM_GrabInput(SDL_GRAB_OFF);
#ifdef NEW_CURSOR
		if (sdl_cursors)
#endif // NEW_CURSOR
			SDL_ShowCursor(1);
		LOG_TO_CONSOLE (c_red1, "Normal mode: press alt+g again to enter Grab mode.");
	}
}

void toggle_first_person()
{
	if (first_person == 0){
		//rotate camera where actor is looking at
		actor *me = get_our_actor();
		if (me)
			rz=me->z_rot;
		rx=-90;
		first_person = 1;
		fol_cam = 0;
	} else {
		first_person = 0;    
		if (rx < -90) {rx = -90;}
	}
	++adjust_view;
	resize_root_window();
	//set_all_intersect_update_needed(main_bbox_tree);
}

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
		int range_weapon_equipped;
		LOCK_ACTORS_LISTS();
		range_weapon_equipped = (your_actor &&
								 your_actor->cur_weapon >= BOW_LONG &&
								 your_actor->cur_weapon <= BOW_CROSS);
		UNLOCK_ACTORS_LISTS();
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
		// allow to shoot at 3D objects
		else if (range_weapon_equipped &&
                 (action_mode == ACTION_ATTACK || (alt_on && ctrl_on)))
		{
			elwin_mouse = CURSOR_ATTACK;
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
int click_game_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int flag_alt = flags & ELW_ALT;
	int flag_ctrl = flags & ELW_CTRL;
	int flag_right = flags & ELW_RIGHT_MOUSE;
	int force_walk = (flag_ctrl && flag_right && !flag_alt);
	int shift_on = flags & ELW_SHIFT;
	int range_weapon_equipped;

#if !defined OSX && !defined WINDOWS
#ifdef MIDDLE_MOUSE_PASTE
	if ((flags & ELW_MOUSE_BUTTON_WHEEL) == ELW_MID_MOUSE)
		// Don't handle middle button clicks
		return 0;
#endif
#endif

	if (flags & ELW_WHEEL_UP)
	{
		camera_zoom_speed = (flag_ctrl) ?10 :1;
		if (camera_zoom_dir == -1)
			camera_zoom_duration += 100;
		else
			camera_zoom_duration = 100;
		camera_zoom_dir = -1;
		return 1;
	}

	if (flags & ELW_WHEEL_DOWN)
	{
		camera_zoom_speed = (flag_ctrl) ?10 :1;
		if (camera_zoom_dir == 1)
			camera_zoom_duration += 100;
		else
			camera_zoom_duration = 100;
		camera_zoom_dir = 1;
		return 1;
	}

	if (mx > win->len_x - 64 && my < 54 ) // 10 pixels dead space to try to prevent accidental misclicks
	{
		if (logo_click_to_url)
			open_web_link(LOGO_URL_LINK);
		return 1;
	}

	LOCK_ACTORS_LISTS();
	range_weapon_equipped = (your_actor &&
							 your_actor->cur_weapon >= BOW_LONG &&
							 your_actor->cur_weapon <= BOW_CROSS);
	UNLOCK_ACTORS_LISTS();

	if (!force_walk)
	{
		if (flag_right)
		{
			if (!cm_banner_disabled)
			{
				/* show the banner control menu if right-clicked and over your actors banner */
				static Uint32 reset_cursor_time = 0;
				static int cm_activate_when_cursor_is = -1;
				extern int cm_mouse_over_banner;
				/* activate the menu once in the cursor cycle - start-cursor reset after a couple of seconds inactivity */
				if (SDL_GetTicks()-reset_cursor_time > 2000)
					cm_activate_when_cursor_is = current_cursor;
				if (cm_mouse_over_banner && (current_cursor == cm_activate_when_cursor_is))
				{
					static size_t cm_id = CM_INIT_VALUE;
					if (!cm_valid(cm_id))
					{
						/* create first time needed */
						cm_id = cm_create(cm_banner_menu_str, NULL);
						cm_bool_line(cm_id, 0, &view_names, NULL);
						cm_bool_line(cm_id, 1, &view_health_bar, NULL);
						cm_bool_line(cm_id, 2, &view_hp, NULL);
						cm_bool_line(cm_id, 3, &view_ether_bar, NULL);
						cm_bool_line(cm_id, 4, &view_ether, NULL);
						cm_bool_line(cm_id, 5, &view_mode_instance, "use_view_mode_instance");
						cm_bool_line(cm_id, 6, &view_chat_text_as_overtext, NULL);
						cm_bool_line(cm_id, 7, &use_alpha_banner, "use_alpha_banner");
						cm_bool_line(cm_id, 8, &sit_lock, "sit_lock");
						cm_bool_line(cm_id, 9, &ranging_lock, NULL);
						cm_bool_line(cm_id, 11, &cm_banner_disabled, "cm_banner_disabled");
					}
					cm_show_direct(cm_id, -1, -1);
					reset_cursor_time = SDL_GetTicks();
				}
			}
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
					else if ((thing_under_the_mouse == UNDER_MOUSE_ANIMAL) && include_use_cursor_on_animals)
						action_mode = ACTION_USE;
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
					if (range_weapon_equipped)
						action_mode = ACTION_ATTACK;
					else
						action_mode = ACTION_WALK;
					break;
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

	// if we're following a path, stop now if the click was in the main window
	if (pf_follow_path && !((mx >= window_width-hud_x) || (my >= window_height-hud_y)))
	{
		pf_destroy_path();
	}

	if (force_walk)
	{
		short x,y;

		get_old_world_x_y (&x, &y);
		// check to see if the coordinates are OUTSIDE the map
		if (y < 0 || x < 0 || x >= tile_map_size_x*6 || y >= tile_map_size_y*6)
			return 1;

		add_highlight(x, y, HIGHLIGHT_TYPE_WALKING_DESTINATION);

		move_to (x, y, 1);
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
				if (thing_under_the_mouse == UNDER_MOUSE_PLAYER)
					achievements_requested(mouse_x, mouse_y, flag_ctrl);
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
			if (spell_result==2)
			{
				short x, y;
		
				if(use_old_clicker)
					get_old_world_x_y (&x, &y);
				else
					get_world_x_y (&x, &y);

				// check to see if the coordinates are OUTSIDE the map
				if (y < 0 || x < 0 || x >= tile_map_size_x*6 || y >= tile_map_size_y*6)
					return 1;
			
				move_to(x,y,0);
				return 1;
			}
			else if (spell_result==3)
			{
				Uint8 str[10];
				
				if (object_under_mouse >= 0 &&
					(thing_under_the_mouse == UNDER_MOUSE_ANIMAL ||
					 thing_under_the_mouse == UNDER_MOUSE_PLAYER))
				{
					actor *this_actor = get_actor_ptr_from_id(object_under_mouse);
					if(this_actor != NULL)
					{
						add_highlight(this_actor->x_tile_pos,this_actor->y_tile_pos, HIGHLIGHT_TYPE_SPELL_TARGET);

						str[0] = TOUCH_PLAYER;
						*((int *)(str+1)) = SDL_SwapLE32((int)object_under_mouse);
						my_tcp_send (my_socket, str, 5);
					}
				}
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
			if (you_sit && sit_lock && !flag_ctrl){
				if(your_actor != NULL)
					add_highlight(your_actor->x_tile_pos,your_actor->y_tile_pos, HIGHLIGHT_TYPE_LOCK);
				return 1;
			}
			if (thing_under_the_mouse == UNDER_MOUSE_PLAYER || thing_under_the_mouse == UNDER_MOUSE_NPC || thing_under_the_mouse == UNDER_MOUSE_ANIMAL)
			{
				if (object_under_mouse>=0){
					actor *this_actor = get_actor_ptr_from_id(object_under_mouse);
					if(this_actor != NULL)
						add_highlight(this_actor->x_tile_pos,this_actor->y_tile_pos, HIGHLIGHT_TYPE_ATTACK_TARGET);
				}

				str[0] = ATTACK_SOMEONE;
				*((int *)(str+1)) = SDL_SwapLE32((int)object_under_mouse);
				my_tcp_send (my_socket, str, 5);
				return 1;
			}
			else if (range_weapon_equipped && thing_under_the_mouse == UNDER_MOUSE_3D_OBJ)
			{
				str[0] = FIRE_MISSILE_AT_OBJECT;
				*((int *)(str+1)) = SDL_SwapLE32((int)object_under_mouse);
				my_tcp_send(my_socket, str, 5);
			}
			break;
		}

		case CURSOR_ENTER:
		case CURSOR_USE:
		case CURSOR_USE_WITEM:
		case CURSOR_TALK:
		{
			Uint8 str[10];

			if (flag_alt && range_weapon_equipped)
				return 1;
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
				if (!shift_on)
				{
					use_item = -1;
					action_mode = ACTION_WALK;
				}
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
		{
			if (flag_alt && range_weapon_equipped)
				return 1;
			if (object_under_mouse == -1)
				return 1;
			if (thing_under_the_mouse == UNDER_MOUSE_3D_OBJ)
			{
				open_bag (object_under_mouse);
				return 1;
			}
			break;
		}

		case CURSOR_HARVEST:
		{
			Uint8 str[10];

			if (flag_alt && range_weapon_equipped)
				return 1;
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
			
			/* if outside the main window, on the hud, don't walk */
			if ((mx >= window_width-hud_x) || (my >= window_height-hud_y))
				return 1;				

			if (flag_alt && range_weapon_equipped)
				return 1;
			if (ranging_lock && range_weapon_equipped){
				if(your_actor != NULL)
					add_highlight(your_actor->x_tile_pos,your_actor->y_tile_pos, HIGHLIGHT_TYPE_LOCK);
				return 1;
			}
			if (you_sit && sit_lock && !flag_ctrl){
				if(your_actor != NULL)
					add_highlight(your_actor->x_tile_pos,your_actor->y_tile_pos, HIGHLIGHT_TYPE_LOCK);
				return 1;
			}

			if(use_old_clicker)
				get_old_world_x_y (&x, &y);
			else
				get_world_x_y (&x, &y);

			// check to see if the coordinates are OUTSIDE the map
			if (y < 0 || x < 0 || x >= tile_map_size_x*6 || y >= tile_map_size_y*6)
				return 1;
			
			add_highlight(x, y, HIGHLIGHT_TYPE_WALKING_DESTINATION);
		
#ifdef DEBUG // FOR DEBUG ONLY!
            if (enable_client_aiming) {
                if (flag_ctrl) {
                    float target[3];
                    
                    target[0] = x * 0.5 + 0.25;
                    target[1] = y * 0.5 + 0.25;
                    target[2] = get_tile_height(x, y) + 1.2f;
                    
					missiles_aim_at_xyz(yourself, target);
					add_command_to_actor(yourself, aim_mode_reload);
					missiles_fire_a_to_xyz(yourself, target);
                }
                else {
                    char in_aim_mode;
                    actor *cur_actor = get_actor_ptr_from_id(yourself);
                    LOCK_ACTORS_LISTS();
                    in_aim_mode = cur_actor->in_aim_mode;
                    UNLOCK_ACTORS_LISTS();
                    if (in_aim_mode == 1)
                        add_command_to_actor(yourself, leave_aim_mode);
                    move_to(x, y, 1);
                }
            }
            else
#endif // DEBUG
			move_to (x, y, 1);

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
	static int eye_candy_was_disabled=0;
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
	
#ifdef CLUSTER_INSIDES
	current_cluster = get_actor_cluster();
#endif // CLUSTER_INSIDES

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
	if (mouse_rate < 5)
	{
		mouse_rate = 5;
	}
	if ((main_count % mouse_rate) == 0)
	{
		read_mouse_now = 1;
	}
	else
	{
		read_mouse_now = 0;
	}
	
	// This window is a bit special since it's not fully 2D
	Leave2DMode ();
	glPushMatrix ();

    update_camera();

	if (new_zoom_level != zoom_level)
	{
		if (new_zoom_level > zoom_level)
			set_all_intersect_update_needed(main_bbox_tree);
		zoom_level = new_zoom_level;
		resize_root_window ();
	}

	move_camera ();
	save_scene_matrix ();

	CalculateFrustum ();
	set_click_line();
	any_reflection = find_reflection ();
	CHECK_GL_ERRORS ();

	reset_under_the_mouse();

	// are we actively drawing things?
	if (SDL_GetAppState() & SDL_APPACTIVE)
	{

		if (!dungeon){
			draw_global_light ();
		} else {
			draw_dungeon_light ();
		}

		if (skybox_update_delay < 1)
			skybox_update_colors();
		if (skybox_show_sky)
        {
			skybox_compute_z_position();
            glPushMatrix();
            glTranslatef(0.0, 0.0, skybox_get_z_position());
			skybox_display();
            glPopMatrix();
        }
	
		if (use_fog)
			weather_render_fog();

		// only draw scene lights if inside or it is night
		if (dungeon || !is_day)
		{
			update_scene_lights ();
			draw_lights ();
		}
		CHECK_GL_ERRORS ();

		if (!dungeon && shadows_on && (is_day || lightning_falling))
		{
			render_light_view();
			CHECK_GL_ERRORS ();
		}

		if (any_reflection > 1) // there are water tiles to display
		{
			draw_water_background();
			CHECK_GL_ERRORS ();
			if (show_reflection) display_3d_reflection ();
		}
		CHECK_GL_ERRORS ();
		glClear(GL_DEPTH_BUFFER_BIT);

		missiles_update();
	
		if (!is_day)
			weather_init_lightning_light();

		if (!dungeon && shadows_on && (is_day || lightning_falling))
		{
			glNormal3f(0.0f,0.0f,1.0f);
			if (use_fog && any_reflection) blend_reflection_fog();
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
			display_actors(1, DEFAULT_RENDER_PASS);
			display_alpha_objects();
			display_blended_objects();
		}

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadMatrixd(skybox_view);
		glMatrixMode(GL_MODELVIEW);

		weather_render_lightning();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);

		CHECK_GL_ERRORS ();
	}	// end of active display check
	else 
	{
		display_actors (1, DEFAULT_RENDER_PASS);	// we need to 'touch' all the actors even if not drawing to avoid problems
	}
	CHECK_GL_ERRORS ();
#ifdef DEBUG_TIME
	light_idle();
#endif // DEBUG_TIME

	ec_idle();

	CHECK_GL_ERRORS();
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

	if (show_weather){
		weather_render();
	}

	CHECK_GL_ERRORS ();
	//particles should be last, we have no Z writting
	display_particles ();

	CHECK_GL_ERRORS ();
	//we do this because we don't want the rain/particles to mess with our cursor

	ec_draw();
	CHECK_GL_ERRORS();

	missiles_draw();
	CHECK_GL_ERRORS();


	last_texture = -1;

	CHECK_GL_ERRORS ();

	animate_map_markers();
	display_map_markers();
	display_map_marks();

	Enter2DMode ();
	//get the FPS, etc

	if (next_fps_time<cur_time){
		fps[4]=fps[3];
		fps[3]=fps[2];
		fps[2]=fps[1];
		fps[1]=fps[0];
		fps[0]=last_count*1000/(cur_time-next_fps_time+1000);
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
					|| use_eye_candy
					)){
				put_colored_text_in_buffer (c_red1, CHAT_SERVER, (unsigned char*)low_framerate_str, -1);
				times_FPS_below_3 = 0;
				if (shadows_on)
				{
					shadows_on = 0;
					shadows_were_disabled=1;
				}
				if (use_eye_candy)
				{
					use_eye_candy = 0;
					eye_candy_was_disabled = 1;
				}
			}
		}
		else 
		{
			times_FPS_below_3 = 0;
			
			if(shadows_were_disabled){
				shadows_on = 1;
				shadows_were_disabled=0;
			}

			if (eye_candy_was_disabled){
				use_eye_candy = 1;
				eye_candy_was_disabled = 0;
			}
		}
	}
	if (show_fps)
	{
#ifdef	DEBUG
		actor *me = get_our_actor ();

		glColor3f (1.0f, 1.0f, 1.0f);
		if(me){
 			safe_snprintf((char*)str,sizeof(str),"Busy: %i",me->busy);
	 		draw_string (400, 4, str, 1);
			safe_snprintf((char*)str,sizeof(str),"Command: %i",me->last_command);
 			draw_string (400, 20, str, 1);
			safe_snprintf((char*)str,sizeof(str),"Coords: %-3i %3i",me->x_tile_pos, me->y_tile_pos);
 			draw_string (550, 4, str, 1);
			safe_snprintf((char*)str,sizeof(str),"Coords: %.3g %.3g",me->x_pos, me->y_pos);
 			draw_string (550, 20, str, 1);
		}
		safe_snprintf ((char*)str, sizeof(str),"Lights: %i", show_lights);
		draw_string (win->len_x-hud_x-105, 32, str, 1);

		safe_snprintf((char*)str, sizeof(str), "lights: ambient=(%.2f,%.2f,%.2f,%.2f) diffuse=(%.2f,%.2f,%.2f,%.2f)",
					  ambient_light[0], ambient_light[1], ambient_light[2], ambient_light[3],
					  diffuse_light[0], diffuse_light[1], diffuse_light[2], diffuse_light[3]);
		draw_string (0, win->len_y - hud_y - 65, str, 1);
		safe_snprintf((char*)str, sizeof(str), "weather: drops=%d/%d/%d/%d/%d/%d, int=%f, dty=%f, fog=%f",
					  weather_get_drops_count(1),
					  weather_get_drops_count(2),
					  weather_get_drops_count(3),
					  weather_get_drops_count(4),
					  weather_get_drops_count(5),
					  weather_get_drops_count(6),
					  weather_get_intensity(),
					  weather_get_density(),
					  skybox_fog_density);
		draw_string (0, win->len_y - hud_y - 40, str, 1);
#else	//DEBUG
		glColor3f (1.0f, 1.0f, 1.0f);
#endif	//DEBUG
		safe_snprintf ((char*)str, sizeof(str), "FPS: %i", fps[0]);
		draw_string (win->len_x-hud_x-95, 4, str, 1);
		safe_snprintf((char*)str, sizeof(str), "UVP: %d", use_animation_program);
		draw_string (win->len_x-hud_x-95, 19, str, 1);
#ifdef DEBUG
		//LRNR: stats testing
		safe_snprintf((char*)str, sizeof(str), "E3D:%3d TOT:%3d", e3d_count, e3d_total);
		draw_string (win->len_x-hud_x-183, 49, str, 1);
		e3d_count= e3d_total= 0;
#endif //DEBUG
	}
	draw_spell_icon_strings();

	CHECK_GL_ERRORS ();
	/* Draw the chat text */
	if (use_windowed_chat != 2)
	{
		int msg, offset, filter;
		filter = use_windowed_chat == 1 ? current_filter : FILTER_ALL;
		if (find_last_lines_time (&msg, &offset, filter, get_console_text_width()))
		{
			set_font(chat_font);	// switch to the chat font
			draw_messages (10, use_windowed_chat == 1 ? 25 : 20, display_text_buffer, DISPLAY_TEXT_BUFFER_SIZE, filter, msg, offset, -1, get_console_text_width(), (int) (1 + lines_to_show * 18 * chat_zoom), chat_zoom, NULL);
			set_font (0);	// switch to fixed
		}
	}
	
	anything_under_the_mouse (0, UNDER_MOUSE_NO_CHANGE);
	CHECK_GL_ERRORS ();

	draw_ingame_interface ();
	
	CHECK_GL_ERRORS ();

	Leave2DMode ();

	if(special_effects){
		display_special_effects(1);
	}
	display_highlight_markers();

	glEnable (GL_LIGHTING);

	// Return to 2D mode to draw the other windows
	glPopMatrix ();	// restore the state
	Enter2DMode ();
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	if ((input_widget!= NULL) && (input_widget->window_id != win->window_id) && !get_show_window(chat_win))
		input_widget_move_to_win(win->window_id);

	return 1;
}

int check_quit_or_fullscreen (Uint32 key)
{
	int alt_on = key & ELW_ALT;
	Uint16 keysym = key & 0xffff;

	// first, try to see if we pressed Alt+x or Ctrl+q, to quit.
	if (key == K_QUIT || key == K_QUIT_ALT) 
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
	// convert keypad values (numlock on)
	if (unikey >= SDLK_KP0 && unikey <= SDLK_KP_EQUALS)
	{
		switch (unikey)
		{
			case SDLK_KP_PERIOD:
				return SDLK_PERIOD;
			case SDLK_KP_DIVIDE:
				return SDLK_SLASH;
			case SDLK_KP_MULTIPLY:
				return SDLK_ASTERISK;
			case SDLK_KP_MINUS:
				return SDLK_MINUS;
			case SDLK_KP_PLUS:
				return SDLK_PLUS;
			case SDLK_KP_ENTER:
				return SDLK_RETURN;
			case SDLK_KP_EQUALS:
				return SDLK_EQUALS;
			default:
				return (unikey-SDLK_WORLD_48)&0xff;
		}
	}

	// catch stupid windows problem if control+enter pressed (that 10 is retuned not 13)
	if (unikey == 10)
		return SDLK_RETURN;

	return unikey & 0xff;
}

int string_input(char *text, size_t maxlen, char ch)
{
	size_t len = strlen(text);
#ifndef OSX
	if (ch == SDLK_BACKSPACE)
#else
	if ((ch == SDLK_BACKSPACE) || (ch == 127))
#endif
	{
		if (len > 0)
			text[len-1] = '\0';
		return 1;
	}
	if (is_printable (ch))
	{
		if (len < (maxlen-1))
		{
			text[len] = ch;
			text[len+1] = '\0';
		}
		return 1;
	}
	return 0;
}

void hide_all_windows(){
	/* Note: We don't watch for if a window is otherwise closed; alt+d to reopen only cares about the last
	 * time it hid windows itself. If you alt+d to reopen windows, manually close them all, and alt+d
	 * again, it'll reopen the same ones.
	 */
	static unsigned int were_open = 0;	//Currently up to 14 windows are managed by this function.
	//If you add more windows, you must ensure that the int is at least 'windows' bits long.
	if (get_show_window(ground_items_win) > 0 || get_show_window(items_win) > 0 || get_show_window(buddy_win) > 0 ||
		get_show_window(manufacture_win) > 0 || get_show_window(elconfig_win) > 0 || get_show_window(sigil_win) > 0 ||
		get_show_window(tab_stats_win) > 0 || get_show_window(tab_help_win) > 0 || get_show_window(storage_win) > 0 ||
		get_show_window(dialogue_win) > 0 || get_show_window(questlog_win) > 0 || (get_show_window(minimap_win) > 0 && !pin_minimap)
		|| get_show_window(tab_info_win) > 0 || get_show_window(emotes_win) > 0 || get_show_window(range_win) > 0
	){	//Okay, hide the open ones.
		if (get_window_showable(ground_items_win) > 0){
			unsigned char protocol_name;

			hide_window (ground_items_win);
			protocol_name= S_CLOSE_BAG;
			my_tcp_send(my_socket,&protocol_name,1);
		}
		if (get_window_showable(items_win) > 0){
			hide_window (items_win);
			were_open |= 1<<0;
		} else {
			were_open &= ~(1<<0);
		}
		if (get_window_showable(buddy_win) > 0){
			hide_window (buddy_win);
			were_open |= 1<<1;
		} else {
			were_open &= ~(1<<1);
		}
		if (get_window_showable(manufacture_win) > 0){
			hide_window (manufacture_win);
			were_open |= 1<<2;
		} else {
			were_open &= ~(1<<2);
		}
		if (get_window_showable(elconfig_win) > 0){
			hide_window (elconfig_win);
			were_open |= 1<<3;
		} else {
			were_open &= ~(1<<3);
		}
		if (get_window_showable(sigil_win) > 0){
			hide_window (sigil_win);
			were_open |= 1<<4;
		} else {
			were_open &= ~(1<<4);
		}
		if (get_window_showable(tab_stats_win) > 0){
			hide_window (tab_stats_win);
			were_open |= 1<<5;
		} else {
			were_open &= ~(1<<5);
		}
		if (get_window_showable(tab_help_win) > 0){
			hide_window (tab_help_win);
			were_open |= 1<<6;
		} else {
			were_open &= ~(1<<6);
		}
		if (get_window_showable(dialogue_win) > 0){
			hide_window (dialogue_win);
		}
		if (get_window_showable(range_win)){
			hide_window (range_win);
			were_open |= 1<<7;
		} else {
			were_open &= ~(1<<7);
		}
		if (get_window_showable(minimap_win) > 0 && !pin_minimap){
			hide_window (minimap_win);
			were_open |= 1<<8;
		} else {
			were_open &= ~(1<<8);
		}
		if (get_window_showable(tab_info_win) > 0){
			hide_window (tab_info_win);
			were_open |= 1<<9;
		} else {
			were_open &= ~(1<<9);
		}
		if (get_window_showable(storage_win) > 0){
			hide_window (storage_win);
			if (view_only_storage)
				were_open |= 1<<10;
			else
				were_open &= ~(1<<10);
		} else {
			were_open &= ~(1<<10);
		}
		if (get_window_showable(emotes_win) > 0){
			hide_window (emotes_win);
			were_open |= 1<<11;
		} else {
			were_open &= ~(1<<11);
		}
		if (get_window_showable(questlog_win) > 0){
			hide_window (questlog_win);
			were_open |= 1<<12;
		} else {
			were_open &= ~(1<<12);
		}
	} else {	//None were open, restore the ones that were open last time the key was pressed
		if (were_open & 1<<0){
			show_window (items_win);
		}
		if (were_open & 1<<1){
			show_window (buddy_win);
		}
		if (were_open & 1<<2){
			show_window (manufacture_win);
		}
		if (were_open & 1<<3){
			show_window (elconfig_win);
		}
		if (were_open & 1<<4){
			show_window (sigil_win);
		}
		if (were_open & 1<<5){
			show_window (tab_stats_win);
		}
		if (were_open & 1<<6){
			show_window (tab_help_win);
		}
		if (were_open & 1<<7){
			show_window (range_win);
		}
		if (were_open & 1<<8){
			show_window (minimap_win );
		}
		if (were_open & 1<<9){
			show_window (tab_info_win);
		}
		if (view_only_storage && (were_open & 1<<10)){
			show_window (storage_win );
		}
		if (were_open & 1<<11){
			show_window (emotes_win);
		}
		if (were_open & 1<<12){
			show_window (questlog_win);
		}
	}
}


static void toggle_sit_stand()
{
	Uint8 str[4];
	//Send message to server...	
	str[0]=SIT_DOWN;
	str[1]=!you_sit;
	my_tcp_send(my_socket,str,2);
}

// keypress handler common to all in-game root windows (game_root_win, 
// console_root_win, and map_root_win)
int keypress_root_common (Uint32 key, Uint32 unikey)
{
	int alt_on = key & ELW_ALT;
	int ctrl_on = key & ELW_CTRL;
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
	else if (key == K_PASTE)
	{
		start_paste(NULL);
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
	else if((keysym == SDLK_t) && shift_on && ctrl_on && !alt_on)
	{
		weather_add_lightning(rand()%5, -camera_x-100+rand()%200, -camera_y-100+rand()%200);
	}
	else if((keysym == SDLK_w) && shift_on && ctrl_on && alt_on)
	{
		if(real_game_minute >= 355) real_game_minute -=355; else real_game_minute +=  5;
		new_minute();
	}
	else if((keysym == SDLK_q) && shift_on && ctrl_on && alt_on)
	{
		if(real_game_minute < 5) real_game_minute +=355; else real_game_minute -=  5;
		new_minute();
	}
	else if((keysym == SDLK_w) && !shift_on && ctrl_on && alt_on)
	{
		if(real_game_minute >= 359) real_game_minute -=359; else real_game_minute +=  1;
		new_minute();
	}
	else if((keysym == SDLK_q) && !shift_on && ctrl_on && alt_on)
	{
		if(real_game_minute < 1) real_game_minute +=359; else real_game_minute -=  1;
		new_minute();
	}
	else if((keysym == SDLK_s) && shift_on && ctrl_on && alt_on)
	{
		skybox_init_defs(NULL);
	}
	else if((keysym == SDLK_f) && shift_on && ctrl_on && alt_on)
	{
		freeze_time = !freeze_time;
		if (freeze_time)
			LOG_TO_CONSOLE(c_green2, "Time freezed!");
		else
		{
			LOG_TO_CONSOLE(c_green2, "Time unfreezed!");
			new_minute();
			new_second();
		}
	}
	else if((keysym == SDLK_HOME) && shift_on && ctrl_on && !alt_on)
	{
		weather_set_area(0, -camera_x, -camera_y, 100.0, 1, 1.0, 10);
	}
	else if ((keysym == SDLK_END) && shift_on && ctrl_on && !alt_on)
	{
		weather_set_area(1, -camera_x, -camera_y, 100.0, 2, 1.0, 10);
	}
	else if((keysym == SDLK_z) && shift_on && ctrl_on && !alt_on)
	{
		ec_create_mine_detonate(your_actor->x_pos + 0.25f, your_actor->y_pos + 0.25f, 0, MINE_TYPE_SMALL_MINE, (poor_man ? 6 : 10));
	}
	else if((keysym == SDLK_x) && shift_on && ctrl_on && !alt_on)
	{
		ec_create_mine_detonate(your_actor->x_pos + 0.25f, your_actor->y_pos + 0.25f, 0, MINE_TYPE_MEDIUM_MINE, (poor_man ? 6 : 10));
	}
	else if((keysym == SDLK_c) && shift_on && ctrl_on && !alt_on)
	{
		ec_create_mine_detonate(your_actor->x_pos + 0.25f, your_actor->y_pos + 0.25f, 0, MINE_TYPE_HIGH_EXPLOSIVE_MINE, (poor_man ? 6 : 10));
	}
	else if((keysym == SDLK_v) && shift_on && ctrl_on && !alt_on)
	{
		ec_create_mine_detonate(your_actor->x_pos + 0.25f, your_actor->y_pos + 0.25f, 0, MINE_TYPE_TRAP, (poor_man ? 6 : 10));
	}
	else if((keysym == SDLK_b) && shift_on && ctrl_on && !alt_on)
	{
		ec_create_mine_detonate(your_actor->x_pos + 0.25f, your_actor->y_pos + 0.25f, 0, MINE_TYPE_CALTROP, (poor_man ? 6 : 10));
	}
	else if((keysym == SDLK_n) && shift_on && ctrl_on && !alt_on)
	{
		ec_create_mine_detonate(your_actor->x_pos + 0.25f, your_actor->y_pos + 0.25f, 0, MINE_TYPE_POISONED_CALTROP, (poor_man ? 6 : 10));
	}
	else if((keysym == SDLK_m) && shift_on && ctrl_on && !alt_on)
	{
		ec_create_mine_detonate(your_actor->x_pos + 0.25f, your_actor->y_pos + 0.25f, 0, MINE_TYPE_MANA_BURNER, (poor_man ? 6 : 10));
	}
	else if((keysym == SDLK_j) && shift_on && ctrl_on && !alt_on)
	{
		ec_create_mine_detonate(your_actor->x_pos + 0.25f, your_actor->y_pos + 0.25f, 0, MINE_TYPE_MANA_DRAINER, (poor_man ? 6 : 10));
	}
	else if((keysym == SDLK_k) && shift_on && ctrl_on && !alt_on)
	{
		ec_create_mine_detonate(your_actor->x_pos + 0.25f, your_actor->y_pos + 0.25f, 0, MINE_TYPE_UNINVIZIBILIZER, (poor_man ? 6 : 10));
	}
	else if((keysym == SDLK_l) && shift_on && ctrl_on && !alt_on)
	{
		ec_create_mine_detonate(your_actor->x_pos + 0.25f, your_actor->y_pos + 0.25f, 0, MINE_TYPE_MAGIC_IMMUNITY_REMOVAL, (poor_man ? 6 : 10));
	}
#endif
#ifdef DEBUG
    // scale the current actor
	else if((keysym == SDLK_p) && shift_on && ctrl_on && !alt_on)
	{
		get_our_actor()->scale *= 1.05;
	}
	else if((keysym == SDLK_o) && shift_on && ctrl_on && !alt_on)
	{
		get_our_actor()->scale /= 1.05;
	}
	else if((keysym == SDLK_h) && shift_on && ctrl_on && !alt_on)
	{
        if (get_our_actor())
        {
            if (get_our_actor()->attached_actor < 0)
                add_actor_attachment(get_our_actor()->actor_id, 200);
            else
                remove_actor_attachment(get_our_actor()->actor_id);
        }
	}
#endif // DEBUG
	// use quickbar items & spells
	else if (action_item_keys(key))
	{
	}
	else if (action_spell_keys(key))
	{
	}
	// hide all windows
	else if(key==K_HIDEWINS)
	{
		hide_all_windows();
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
	else if (key == K_RANGINGLOCK)
	{
		ranging_lock = !ranging_lock;
		if (ranging_lock)
			LOG_TO_CONSOLE(c_green1, ranginglock_enabled_str);
		else
			LOG_TO_CONSOLE(c_green1, ranginglock_disabled_str);
	}
	// open or close windows
	else if (key == K_STATS)
	{
		view_tab (&tab_stats_win, &tab_stats_collection_id, STATS_TAB_STATS);
	}
	else if (key == K_QUESTLOG)
	{
		view_window (&questlog_win, 0);
	}
	else if (key == K_SESSION)
	{
		view_tab (&tab_stats_win, &tab_stats_collection_id, STATS_TAB_SESSION);
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
	else if (key == K_NOTEPAD)
	{
		view_tab(&tab_info_win, &tab_info_collection_id, INFO_TAB_NOTEPAD);
	}
	else if(key == K_MINIMAP)
	{
		view_window (&minimap_win, 0);
	}
#ifdef ECDEBUGWIN
	else if(key == K_ECDEBUGWIN)
	{
		view_window (&ecdebug_win, 0);
	}
#endif // ECDEBUGWIN
	else if (key == K_SIGILS)
	{
		view_window (&sigil_win, -1);
	}
	else if (key == K_EMOTES)
	{
		view_window (&emotes_win, -1);
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
	else if(key == K_RANGINGWIN)
	{
		view_window(&range_win, -1);
	}
	else if (key == K_SIT)
	{
		toggle_sit_stand ();
	}
	else if (key == K_BROWSER)
	{
		open_last_seen_url();
	}
	else if (key == K_BROWSERWIN)
	{
		view_tab(&tab_info_win, &tab_info_collection_id, INFO_TAB_URLWIN);
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
	else if (key == K_OPAQUEWIN)
	{
		if (top_SWITCHABLE_OPAQUE_window_drawn != -1)
			windows_list.window[top_SWITCHABLE_OPAQUE_window_drawn].opaque ^= 1;
	}
	else if (key == K_REPEATSPELL)	// REPEAT spell command
	{
		if ( !get_show_window (trade_win) )
		{
			repeat_spell();
		}
	}
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
	// The following should only be reached when we hit an invalid key 
	// combo or for any reason we don't have a valid input_widget.
	else if (is_printable (ch) && input_text_line.len < MAX_TEXT_MESSAGE_LENGTH)
	{
		if(put_char_in_buffer (&input_text_line, ch, input_text_line.len)) {
			if(input_widget) {
				text_field *tf = input_widget->widget_info;
				tf->cursor = tf->buffer->len;
				if(input_widget->window_id == game_root_win) {
					widget_unset_flags (input_widget->window_id, input_widget->id, WIDGET_DISABLED);
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
		parse_input(input_text_line.data, input_text_line.len);
		add_line_to_history(input_text_line.data, input_text_line.len);
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
	else if (key == K_TURNLEFT)
	{
		//Moved delay to my_tcp_send
		Uint8 str[2];
		str[0] = TURN_LEFT;
		my_tcp_send (my_socket, str, 1);
	}
	else if (key == K_TURNRIGHT)
	{
		Uint8 str[2];
		str[0] = TURN_RIGHT;
		my_tcp_send (my_socket, str, 1);
	}
	else if (key==K_ADVANCE)
	{
		move_self_forward();
	}
	else if (key == K_ROTATELEFT)
	{
		camera_rotation_speed = (first_person?-1:1)*normal_camera_rotation_speed / 800.0;
		camera_rotation_deceleration = normal_camera_deceleration*0.5E-3;
		camera_rotation_duration = 800;
		if (fol_cam && !fol_cam_behind)
		{
			hold_camera += camera_kludge - last_kludge;
			last_kludge = camera_kludge;
		}
	}
	else if (key == K_FROTATELEFT)
	{
		camera_rotation_speed = (first_person?-1:1)*fine_camera_rotation_speed / 200.0;
		camera_rotation_speed /= 4.0;
		camera_rotation_deceleration = normal_camera_deceleration*0.5E-3;
		camera_rotation_duration = 200;
		if (fol_cam && !fol_cam_behind)
		{
			hold_camera += camera_kludge - last_kludge;
			last_kludge = camera_kludge;
		}
	}
	else if (key == K_ROTATERIGHT)
	{
		camera_rotation_speed = (first_person?1:-1)*normal_camera_rotation_speed / 800.0;
		camera_rotation_deceleration = normal_camera_deceleration*0.5E-3;
		camera_rotation_duration = 800;
		if (fol_cam && !fol_cam_behind)
		{
			hold_camera += camera_kludge - last_kludge;
			last_kludge = camera_kludge;
		}
	}
	else if (key == K_FROTATERIGHT)
	{
		camera_rotation_speed = (first_person?1:-1)*fine_camera_rotation_speed / 200.0;
		camera_rotation_speed /= 4.0;
		camera_rotation_deceleration = normal_camera_deceleration*0.5E-3;
		camera_rotation_duration = 200;
		if (fol_cam && !fol_cam_behind)
		{
			hold_camera += camera_kludge - last_kludge;
			last_kludge = camera_kludge;
		}
	}
	else if (key == K_CAMERAUP)
	{
		camera_tilt_speed = -normal_camera_rotation_speed * 0.0005;
		camera_tilt_duration += 100;
		camera_tilt_deceleration = normal_camera_deceleration*0.5E-3;
	}
	else if (key == K_CAMERADOWN)
	{
		camera_tilt_speed = normal_camera_rotation_speed * 0.0005;
		camera_tilt_duration += 100;
		camera_tilt_deceleration = normal_camera_deceleration*0.5E-3;
	}
	else if (key == K_ZOOMIN)
	{
		if (camera_zoom_dir == -1)
			camera_zoom_duration += 100;
		else
			camera_zoom_duration = 100;
		camera_zoom_dir = -1;
	}
	else if (key == K_ZOOMOUT)
	{
		if (camera_zoom_dir == 1)
			camera_zoom_duration += 100;
		else
			camera_zoom_duration = 100;
		camera_zoom_dir = 1;
	}
	else if ((key == K_MAP) || (key == K_MARKFILTER))
	{
		// if K_MARKFILTER pressed, open the map window with the filter active
		if (key == K_MARKFILTER)
			mark_filter_active = 1;
		if ( switch_to_game_map () )
		{
			if (have_mouse) {toggle_have_mouse(); keep_grabbing_mouse=1;}
			hide_window (game_root_win);
			show_window (map_root_win);
		}
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
	else if (key == K_FIRST_PERSON)
	{
		toggle_first_person();
	}
	else if (key == K_GRAB_MOUSE)
	{
		toggle_have_mouse();
	}
	else if (key == K_EXTEND_CAM)
	{
		toggle_ext_cam(&ext_cam);
	}
#ifdef PAWN
	else if (keysym == SDLK_F8)
	{
		if (object_under_mouse != -1 && thing_under_the_mouse == UNDER_MOUSE_3D_OBJ && objects_list[object_under_mouse])
		{
			run_pawn_map_function ("play_with_object_pos", "ii", object_under_mouse, key & ELW_SHIFT ? 1: 0);
		}
		else
		{
			run_pawn_server_function ("pawn_test", "s", "meep!");
		}
	}
#endif
	else if (keysym == SDLK_F9)
	{
		actor *me = get_actor_ptr_from_id (yourself);
		ec_create_campfire(me->x_pos + 0.25f, me->y_pos + 0.25f, get_tile_height(me->x_tile_pos, me->y_tile_pos), 0.0, 1.0, (poor_man ? 6 : 10), 0.7);
	}
#ifdef DEBUG
	else if (keysym == SDLK_F10)
	{
		if (key & ELW_SHIFT)
		{
#ifdef NEW_SOUND
			print_sound_types();
			print_sound_samples();
			print_sounds_list();
			print_sound_sources();
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
	else if (keysym == SDLK_F11)
	{
#ifdef	NEW_TEXTURES
		unload_texture_cache();
#else	/* NEW_TEXTURES */
		int i;

		for (i = 0; i < TEXTURE_CACHE_MAX; i++)
		{
			if(texture_cache[i].file_name[0])
			{
				glDeleteTextures(1,(GLuint*)&texture_cache[i].texture_id);
				texture_cache[i].texture_id = 0;
				CHECK_GL_ERRORS();
			}
		}
		
		//now, reload the textures
		for (i=0; i < TEXTURE_CACHE_MAX; i++)
		{
			if(texture_cache[i].file_name[0] && !texture_cache[i].load_err)
			{
				int alpha = texture_cache[i].alpha;
				if(alpha <= 0)
					texture_cache[i].texture_id = load_bmp8_color_key (&(texture_cache[i]), alpha);
				else 
					texture_cache[i].texture_id = load_bmp8_fixed_alpha (&(texture_cache[i]), alpha);
			}
		}
#endif	/* NEW_TEXTURES */
	}
#ifdef	NEW_TEXTURES
	else if (keysym == SDLK_F12)
	{
		dump_texture_cache();
	}
#endif	/* NEW_TEXTURES */
#endif	/* DEBUG */
	// END OF TEST!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	else
	{
		Uint8 ch = key_to_char (unikey);

		reset_tab_completer();
		if (ch == '`' || key == K_CONSOLE)
		{
			if (have_mouse) {toggle_have_mouse(); keep_grabbing_mouse=1;}
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
	init_hud_interface (HUD_INTERFACE_GAME);
	show_hud_windows();
	if (use_windowed_chat == 1)
		display_tab_bar();
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
			if (dark_channeltext == 1)
				set_text_message_color (&input_text_line, 0.6f, 0.6f, 0.6f);
			else if (dark_channeltext == 2)
				set_text_message_color (&input_text_line, 0.16f, 0.16f, 0.16f);
			else
				set_text_message_color (&input_text_line, 1.0f, 1.0f, 1.0f);
			id = text_field_add_extended(game_root_win, 42, NULL, 0, height-INPUT_HEIGHT-hud_y, width-hud_x, INPUT_HEIGHT, INPUT_DEFAULT_FLAGS, chat_zoom, 0.77f, 0.57f, 0.39f, &input_text_line, 1, FILTER_ALL, INPUT_MARGIN, INPUT_MARGIN);
			input_widget = widget_find(game_root_win, id);
			input_widget->OnResize = input_field_resize;
		}
		widget_set_OnKey(input_widget->window_id, input_widget->id, chat_input_key);
		if(input_text_line.len > 0) {
			widget_unset_flags (input_widget->window_id, input_widget->id, WIDGET_DISABLED);
		}
		resize_root_window();
	}
}
