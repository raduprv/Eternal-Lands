#include <stdlib.h>
#include <string.h>
#include "events.h"
#include "actor_scripts.h"
#include "client_serv.h"
#include "context_menu.h"
#include "draw_scene.h"
#include "elconfig.h"
#include "elwindows.h"
#include "gamewin.h"
#include "gl_init.h"
#include "global.h"
#include "interface.h"
#include "items.h"
#include "keys.h"
#include "mapwin.h"
#include "multiplayer.h"
#include "particles.h"
#include "pathfinder.h"
#include "paste.h"
#include "pm_log.h"
#include "update.h"
#ifdef PAWN
#include "pawn/elpawn.h"
#endif
#ifdef	NEW_TEXTURES
#include "textures.h"
#endif	/* NEW_TEXTURES */	    

#ifndef WINDOWS
#include <SDL_syswm.h>
#endif

int adding_mark = 0;
int mark_x , mark_y;
int max_mark = 0;
marking marks[MAX_MARKINGS];

SDLMod  mod_key_status;
//Uint32 last_turn_around=0;

int shift_on;
int alt_on;
int ctrl_on;
int meta_on;
#ifdef OSX
int osx_right_mouse_cam = 0;
#endif

void	quick_use(int use_id)
{
	Uint8 quick_use_str[3];
	int	i;

	for(i=0; i<ITEM_NUM_ITEMS; i++){
		if(item_list[i].pos==use_id &&
			item_list[i].quantity &&
			item_list[i].use_with_inventory){
				quick_use_str[0]= USE_INVENTORY_ITEM;
				quick_use_str[1]= use_id;
				quick_use_str[2]= i;
				my_tcp_send(my_socket,quick_use_str,2);
#ifdef NEW_SOUND
				item_list[i].action = USE_INVENTORY_ITEM;
#endif // NEW_SOUND
				break;
		}
	}
}

int HandleEvent (SDL_Event *event)
{
	int done = 0;
	int mouse_delta_x;
	int mouse_delta_y;
	Uint32 key = 0;
	Uint32 flags = 0;

	if (event->type == SDL_NOEVENT) return 0;

	mod_key_status = SDL_GetModState();

	if (mod_key_status & KMOD_SHIFT) shift_on = 1;
	else shift_on = 0;

	//AltGR users still do not have AltGr working properly. Currently we have to accept only the left ALT key
	//if (mod_key_status & KMOD_ALT && !(mod_key_status & KMOD_MODE)) alt_on = 1;
	if (mod_key_status & KMOD_LALT) alt_on = 1;
	else alt_on = 0;

	if (mod_key_status & KMOD_CTRL) ctrl_on = 1;
	else ctrl_on = 0;

        if (mod_key_status & KMOD_META) meta_on = 1;
        else meta_on = 0;

	switch( event->type )
	{

#if !defined(WINDOWS) && !defined(OSX)
		case SDL_SYSWMEVENT:
			if (event->syswm.msg->event.xevent.type == SelectionNotify)
				finishpaste(event->syswm.msg->event.xevent.xselection);
			else if (event->syswm.msg->event.xevent.type == SelectionRequest)
				process_copy(&event->syswm.msg->event.xevent.xselectionrequest);
			break;
#endif

		case SDL_KEYDOWN:
			if(!(SDL_GetAppState() & SDL_APPINPUTFOCUS)){
				break;  //don't have focus, so we shouldn't be getting keystrokes
			}
			key=(Uint16)event->key.keysym.sym;

			//use the modifiers that were on when the key was pressed, not when we go to check
			if (event->key.keysym.mod & KMOD_SHIFT) key |= ELW_SHIFT;
			if (event->key.keysym.mod & KMOD_CTRL) key |= ELW_CTRL;
			//AltGR users still do not have AltGr working properly. Currently we have to accept only the left ALT key
			//if (event->key.keysym.mod & KMOD_ALT && !(event->key.keysym.mod & KMOD_MODE)) key |= ELW_ALT;
			if (event->key.keysym.mod & KMOD_LALT) key |= ELW_ALT;
			if (event->key.keysym.mod & KMOD_META) key |= ELW_META;

			if (afk_time) 
				last_action_time = cur_time;	// Set the latest event... Don't let the modifiers ALT, CTRL and SHIFT change the state

			/* any keypress forces any context menu to close */
			cm_post_show_check(1);
			keypress_in_windows (mouse_x, mouse_y, key, event->key.keysym.unicode);
			break;

		case SDL_VIDEORESIZE:
	    	 	window_width = event->resize.w;
	      		window_height = event->resize.h;
			resize_root_window ();
			break;

		case SDL_QUIT:
			done = 1;
			break;

		case SDL_ACTIVEEVENT:
			{
				// force ALL keys up, else you can 'catch' the alt/ctrl keys due to an SDL bug ...
				// But, compiz on Linux generates these events with every mouse click causing
				// ctrl/alt/shift states to be unreadable.  Adding loss/gain timer can exclude
				// the mouse click events while still catching genuein focus changes.
				static Uint32 last_loss = 0;
				if (event->active.gain == 0)
					last_loss = SDL_GetTicks();
				else if (last_loss && (event->active.gain == 1) && ((SDL_GetTicks() - last_loss) > 250))
				{
					last_loss = 0;
					SDL_SetModState(KMOD_NONE);
				}
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			// make sure the mouse button is our window, or else we ignore it
			//Checking if we have keyboard focus for a mouse click is wrong, but SDL doesn't care to tell us we have mouse focus when someone alt-tabs back in and the mouse was within bounds of both other and EL windows. Blech.
			if(event->button.x >= window_width || event->button.y >= window_height || !((SDL_GetAppState() & SDL_APPMOUSEFOCUS)||(SDL_GetAppState() & SDL_APPINPUTFOCUS)))
			{
				break;
			}

			if (afk_time && event->type == SDL_MOUSEBUTTONDOWN)
				last_action_time = cur_time;	// Set the latest events - don't make mousemotion set the afk_time... (if you prefer that mouse motion sets/resets the afk_time, then move this one step below...

			// fallthrough
		case SDL_MOUSEMOTION:
			if (have_mouse)
			{
				mouse_x = window_width/2;
				mouse_y = window_height/2;

				mouse_delta_x= event->motion.xrel;
				mouse_delta_y= event->motion.yrel;
			}
			else if(event->type==SDL_MOUSEMOTION)
			{
				mouse_x = event->motion.x;
				mouse_y = event->motion.y;

				mouse_delta_x = event->motion.xrel;
				mouse_delta_y = event->motion.yrel;
			}
			else
			{
#ifdef NEW_CURSOR
				if (sdl_cursors)
				{
#endif // NEW_CURSOR
					mouse_x= event->button.x;
					mouse_y= event->button.y;
#ifdef NEW_CURSOR
				}
#endif // NEW_CURSOR
				mouse_delta_x= mouse_delta_y= 0;
			}

			if (event->type == SDL_MOUSEBUTTONDOWN)
			{
				if (event->button.button == SDL_BUTTON_LEFT)
					left_click++;
				else if (event->button.button == SDL_BUTTON_RIGHT)
					right_click++;
				else if (event->button.button == SDL_BUTTON_MIDDLE)
					middle_click++;
			}
			else
			{
				if (event->type == SDL_MOUSEMOTION && (event->motion.state & SDL_BUTTON_LMASK))
				{
					left_click++;
				}
				else
				{
					if (left_click) end_drag_windows();
					left_click = 0;
				}

				if (event->type == SDL_MOUSEMOTION && (event->motion.state & SDL_BUTTON_RMASK))
				{
					right_click++;
#ifdef OSX
					if (osx_right_mouse_cam)
					{
						have_mouse = 1;
					}
#endif
				}
				else
				{
					right_click = 0;
#ifdef OSX
					if (osx_right_mouse_cam)
					{
						have_mouse = 0;
					}
#endif
				}

				if (event->type == SDL_MOUSEMOTION && ((event->motion.state & SDL_BUTTON_MMASK) || meta_on))
				{
					middle_click++;
				}
				else
				{
					middle_click = 0;
				}
			}

			if ((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_MMASK) || have_mouse)
			{
				camera_rotation_speed = camera_rotation_speed*0.5 + normal_camera_rotation_speed * mouse_delta_x*0.00025;
				camera_tilt_speed = camera_tilt_speed*0.5 + normal_camera_rotation_speed * mouse_delta_y*0.00025;
				camera_rotation_deceleration = normal_camera_deceleration*1E-3;
				camera_tilt_deceleration = normal_camera_deceleration*1E-3;

				if (camera_rotation_speed > 1.0)
					camera_rotation_speed = 1.0;
				else if (camera_rotation_speed < -1.0)
					camera_rotation_speed = -1.0;

				// the following variables have to be removed!
				camera_rotation_duration = 0;
				camera_tilt_duration = 0;
				if (fol_cam && !fol_cam_behind)
				{
					hold_camera += camera_kludge - last_kludge;
					last_kludge = camera_kludge;
				}
			}

			if (shift_on) flags |= ELW_SHIFT;
			if (alt_on) flags |= ELW_ALT;
			if (ctrl_on) flags |= ELW_CTRL;
			if (left_click) flags |= ELW_LEFT_MOUSE;
			if (middle_click || meta_on) flags |= ELW_MID_MOUSE;
			if (right_click) flags |= ELW_RIGHT_MOUSE;
			if (event->type == SDL_MOUSEBUTTONDOWN)
			{
				if (event->button.button == SDL_BUTTON_WHEELUP)
					flags |= ELW_WHEEL_UP;
				else if (event->button.button == SDL_BUTTON_WHEELDOWN)
					flags |= ELW_WHEEL_DOWN;
			}

			if ( left_click == 1 || right_click == 1
#if !defined OSX && !defined WINDOWS
#ifdef MIDDLE_MOUSE_PASTE
				|| middle_click == 1
#endif
#endif
				|| (flags & (ELW_WHEEL_UP | ELW_WHEEL_DOWN) ) )
			{
				click_in_windows (mouse_x, mouse_y, flags);
			}
			if (left_click >= 1)
			{
				if (drag_windows (mouse_x, mouse_y, mouse_delta_x, mouse_delta_y) >= 0)
				{
					/* clicking title forces any context menu to close */
					cm_post_show_check(1);
					return done;
				}
				if (drag_in_windows (mouse_x, mouse_y, flags, mouse_delta_x, mouse_delta_y) >= 0)
				{
					return done;
				}
			}
			break;

		case SDL_USEREVENT:
			switch(event->user.code){
			case	EVENT_MOVEMENT_TIMER:
				pf_move();
				break;
			case	EVENT_UPDATE_PARTICLES:
				update_particles();
				break;

			case    EVENT_UPDATES_DOWNLOADED:
				handle_update_download((struct http_get_struct *)event->user.data1);
				break;
			    
			case    EVENT_DOWNLOAD_COMPLETE:
				handle_file_download((struct http_get_struct *)event->user.data1);
				break;

#ifdef PAWN			
			case 	EVENT_PAWN_TIMER:
				handle_pawn_timers ();
				break;
#endif
#ifdef	CUSTOM_UPDATE
			case    EVENT_CUSTOM_UPDATE_COMPLETE:
#ifdef	NEW_TEXTURES
				unload_actor_texture_cache();
#endif	/* NEW_TEXTURES */	    
				break;
#endif	/* CUSTOM_UPDATE */
			}
	}

	return(done);
}
