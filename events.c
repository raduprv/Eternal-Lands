#include	<stdlib.h>
#include	<string.h>
#include	"global.h"
#include	"elwindows.h"
#include	"keys.h"

#ifndef WINDOWS
#include <SDL_syswm.h>
#endif

int adding_mark = 0;
int mark_x , mark_y;
int max_mark = 0;
marking marks[200];

int mod_key_status;
//Uint32 last_turn_around=0;

int shift_on;
int alt_on;
int ctrl_on;

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
				break;
		}
	}
}

int HandleEvent (SDL_Event *event)
{
	int done = 0;
	Uint32 key = 0;
	Uint32 flags = 0;

	if (event->type == SDL_NOEVENT) return 0;

	mod_key_status = SDL_GetModState();

	if (mod_key_status & KMOD_SHIFT) shift_on = 1;
	else shift_on = 0;

	if (mod_key_status & KMOD_LALT) alt_on = 1;
	else alt_on = 0;

	if (mod_key_status & KMOD_CTRL) ctrl_on = 1;
	else ctrl_on = 0;
	
	switch( event->type )
	{

#if !defined(WINDOWS) && !defined(OSX)
		case SDL_SYSWMEVENT:
		
			if(event->syswm.msg->event.xevent.type == SelectionNotify)
				finishpaste(event->syswm.msg->event.xevent.xselection);
			break;
#endif

		case SDL_KEYDOWN:
			key=(Uint16)event->key.keysym.sym;
			
			if (shift_on) key |= ELW_SHIFT;
			if (ctrl_on) key |= ELW_CTRL;
			if (alt_on) key |= ELW_ALT;

			if (afk_time) 
				last_action_time = cur_time;	// Set the latest event... Don't let the modifiers ALT, CTRL and SHIFT change the state

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

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			// make sure the mouse button is our window, or else we ignore it
			if(event->button.x >= window_width || event->button.y >= window_height)
			{
				break;
			}
		
			if (afk_time) 
				last_action_time = cur_time;	// Set the latest events - don't make mousemotion set the afk_time... (if you prefer that mouse motion sets/resets the afk_time, then move this one step below...
		case SDL_MOUSEMOTION:
			if(event->type==SDL_MOUSEMOTION)
			{
#ifdef OSX
				// Some idiot decided to invert the y axis on mac os x sdl. 
				// This will no-doubt need to be changed when they fix it in the next version
				mouse_x= event->motion.x;
				mouse_y= window_height-event->motion.y;

				mouse_delta_x= event->motion.xrel;
				mouse_delta_y= -event->motion.yrel;
#else
				mouse_x= event->motion.x;
				mouse_y= event->motion.y;

				mouse_delta_x= event->motion.xrel;
				mouse_delta_y= event->motion.yrel;
#endif
			}
			else
			{
#ifdef OSX
				// See above comment
				mouse_x= event->button.x;
				mouse_y= window_height-event->button.y;
				mouse_delta_x= mouse_delta_y= 0;
#else
				mouse_x= event->button.x;
				mouse_y= event->button.y;
				mouse_delta_x= mouse_delta_y= 0;
#endif
			}

			if (event->type == SDL_MOUSEBUTTONDOWN)
			{
				if(event->button.button==SDL_BUTTON_LEFT)
					left_click++;
			}
			else if (event->type == SDL_MOUSEMOTION && (event->motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)))
				left_click++;
			else
			{
				if (left_click) end_drag_windows();
				left_click = 0;
			}

			if (event->type == SDL_MOUSEBUTTONDOWN)
			{
				if (event->button.button == SDL_BUTTON_RIGHT)
					right_click++;
			}
			else if (event->type == SDL_MOUSEMOTION && (event->motion.state & SDL_BUTTON(SDL_BUTTON_RIGHT)))
				right_click++;
			else
				right_click= 0;

			if (event->type == SDL_MOUSEBUTTONDOWN) 
			{
				if (event->button.button == SDL_BUTTON_MIDDLE)
					middle_click++;
			}
			else if (event->type == SDL_MOUSEMOTION && (event->motion.state & SDL_BUTTON(SDL_BUTTON_MIDDLE)))
				middle_click++;
			else
				middle_click= 0;

			if ( SDL_GetMouseState (NULL, NULL) & SDL_BUTTON(2) )
			{
				camera_rotation_speed = normal_camera_rotation_speed * mouse_delta_x / 220;
				camera_rotation_frames = 40;
				camera_tilt_speed = normal_camera_rotation_speed * mouse_delta_y / 220;
				camera_tilt_frames = 40;
			}

			if (shift_on) flags |= ELW_SHIFT;
			if (alt_on) flags |= ELW_ALT;
			if (ctrl_on) flags |= ELW_CTRL;
			if (left_click) flags |= ELW_LEFT_MOUSE;
			if (middle_click) flags |= ELW_MID_MOUSE;
			if (right_click) flags |= ELW_RIGHT_MOUSE;

			if (event->type == SDL_MOUSEBUTTONDOWN)
			{
				if (event->button.button == SDL_BUTTON_WHEELUP)
					flags |= ELW_WHEEL_UP;
				else if (event->button.button == SDL_BUTTON_WHEELDOWN)
					flags |= ELW_WHEEL_DOWN;
			}

			if (left_click >= 1)
			{
				if (drag_windows (mouse_x, mouse_y, mouse_delta_x, mouse_delta_y) >= 0)
					return done;
				if (drag_in_windows (mouse_x, mouse_y, flags, mouse_delta_x, mouse_delta_y) >= 0)
					return done;
			}

			if ( left_click==1 || right_click==1 || (flags & (ELW_WHEEL_UP | ELW_WHEEL_DOWN) ) )
				click_in_windows (mouse_x, mouse_y, flags);

			break;
			
		case SDL_USEREVENT:
			if (event->user.code == EVENT_MOVEMENT_TIMER)
			{
				pf_move();
			}
			else if (event->user.code == EVENT_UPDATE_CAMERA)
			{
				update_camera();
			}
			else if (event->user.code == EVENT_ANIMATE_ACTORS)
			{
				animate_actors();
			}
	}

	return(done);
}

