	#include <stdlib.h>
#include <string.h>
#include <SDL_keyboard.h>
#ifndef WINDOWS
#include <SDL_syswm.h>
#endif

#ifdef ANDROID
#include "asc.h"
#endif
#include "elconfig.h"
#include "events.h"
#ifdef ANDROID
#include "console.h"
#include "consolewin.h"
#include "elwindows.h"
#include "multiplayer.h"
#endif
#include "context_menu.h"
#include "gamewin.h"
#include "gl_init.h"
#include "interface.h"
#include "new_character.h"
#include "particles.h"
#include "pathfinder.h"
#include "paste.h"
#include "pm_log.h"
#include "update.h"
#ifdef PAWN
#include "pawn/elpawn.h"
#endif
#include "textures.h"

#include "actor_scripts.h"

#ifdef ANDROID
int back_on;
float long_touch_delay_s = 0.5f;
float motion_touch_delay_s = 0.15f;
int enable_keyboard_debug = 0;
#endif

#ifdef OSX
int osx_right_mouse_cam = 0;
#endif

static const int min_fps = 3;

static void enter_minimised_state(void)
{
	max_fps = min_fps;
	//printf("entered minimised\n");
}

static void leave_minimised_state(void)
{
	max_fps = limit_fps;
	if (!get_show_window(newchar_root_win))
		update_all_actors(0);
	//printf("left minimised\n");
}

// Convert from utf-8 to unicode, generated from the table here:
// https://www.utf8-chartable.de/unicode-utf8-table.pl?htmlent=1
static Uint8 utf8_to_unicode(const char text[32])
{
	if ((Uint8)text[1] == 0)
	{
		if (((Uint8)text[0] >= 0x20) && ((Uint8)text[0] < 0x7f))
			return (Uint8)text[0];
		else
			return 0;
	}
	else if ((Uint8)text[0] == 0xc2)
	{
		switch((Uint8)text[1])
		{
			case 0xa0: return 0xa0; // no-break space
			case 0xa1: return 0xa1; // inverted exclamation mark
			case 0xa2: return 0xa2; // cent sign
			case 0xa3: return 0xa3; // pound sign
			case 0xa4: return 0xa4; // currency sign
			case 0xa5: return 0xa5; // yen sign
			case 0xa6: return 0xa6; // broken bar
			case 0xa7: return 0xa7; // section sign
			case 0xa8: return 0xa8; // diaeresis
			case 0xa9: return 0xa9; // copyright sign
			case 0xaa: return 0xaa; // feminine ordinal indicator
			case 0xab: return 0xab; // left-pointing double angle quotation mark
			case 0xac: return 0xac; // not sign
			case 0xad: return 0xad; // soft hyphen
			case 0xae: return 0xae; // registered sign
			case 0xaf: return 0xaf; // macron
			case 0xb0: return 0xb0; // degree sign
			case 0xb1: return 0xb1; // plus-minus sign
			case 0xb2: return 0xb2; // superscript two
			case 0xb3: return 0xb3; // superscript three
			case 0xb4: return 0xb4; // acute accent
			case 0xb5: return 0xb5; // micro sign
			case 0xb6: return 0xb6; // pilcrow sign
			case 0xb7: return 0xb7; // middle dot
			case 0xb8: return 0xb8; // cedilla
			case 0xb9: return 0xb9; // superscript one
			case 0xba: return 0xba; // masculine ordinal indicator
			case 0xbb: return 0xbb; // right-pointing double angle quotation mark
			case 0xbc: return 0xbc; // vulgar fraction one quarter
			case 0xbd: return 0xbd; // vulgar fraction one half
			case 0xbe: return 0xbe; // vulgar fraction three quarters
			case 0xbf: return 0xbf; // inverted question mark
			default: return 0;
		}
	}
	else if ((Uint8)text[0] == 0xc3)
	{
		switch((Uint8)text[1])
		{
			case 0x80: return 0xc0; // latin capital letter a with grave
			case 0x81: return 0xc1; // latin capital letter a with acute
			case 0x82: return 0xc2; // latin capital letter a with circumflex
			case 0x83: return 0xc3; // latin capital letter a with tilde
			case 0x84: return 0xc4; // latin capital letter a with diaeresis
			case 0x85: return 0xc5; // latin capital letter a with ring above
			case 0x86: return 0xc6; // latin capital letter ae
			case 0x87: return 0xc7; // latin capital letter c with cedilla
			case 0x88: return 0xc8; // latin capital letter e with grave
			case 0x89: return 0xc9; // latin capital letter e with acute
			case 0x8a: return 0xca; // latin capital letter e with circumflex
			case 0x8b: return 0xcb; // latin capital letter e with diaeresis
			case 0x8c: return 0xcc; // latin capital letter i with grave
			case 0x8d: return 0xcd; // latin capital letter i with acute
			case 0x8e: return 0xce; // latin capital letter i with circumflex
			case 0x8f: return 0xcf; // latin capital letter i with diaeresis
			case 0x90: return 0xd0; // latin capital letter eth
			case 0x91: return 0xd1; // latin capital letter n with tilde
			case 0x92: return 0xd2; // latin capital letter o with grave
			case 0x93: return 0xd3; // latin capital letter o with acute
			case 0x94: return 0xd4; // latin capital letter o with circumflex
			case 0x95: return 0xd5; // latin capital letter o with tilde
			case 0x96: return 0xd6; // latin capital letter o with diaeresis
			case 0x97: return 0xd7; // multiplication sign
			case 0x98: return 0xd8; // latin capital letter o with stroke
			case 0x99: return 0xd9; // latin capital letter u with grave
			case 0x9a: return 0xda; // latin capital letter u with acute
			case 0x9b: return 0xdb; // latin capital letter u with circumflex
			case 0x9c: return 0xdc; // latin capital letter u with diaeresis
			case 0x9d: return 0xdd; // latin capital letter y with acute
			case 0x9e: return 0xde; // latin capital letter thorn
			case 0x9f: return 0xdf; // latin small letter sharp s
			case 0xa0: return 0xe0; // latin small letter a with grave
			case 0xa1: return 0xe1; // latin small letter a with acute
			case 0xa2: return 0xe2; // latin small letter a with circumflex
			case 0xa3: return 0xe3; // latin small letter a with tilde
			case 0xa4: return 0xe4; // latin small letter a with diaeresis
			case 0xa5: return 0xe5; // latin small letter a with ring above
			case 0xa6: return 0xe6; // latin small letter ae
			case 0xa7: return 0xe7; // latin small letter c with cedilla
			case 0xa8: return 0xe8; // latin small letter e with grave
			case 0xa9: return 0xe9; // latin small letter e with acute
			case 0xaa: return 0xea; // latin small letter e with circumflex
			case 0xab: return 0xeb; // latin small letter e with diaeresis
			case 0xac: return 0xec; // latin small letter i with grave
			case 0xad: return 0xed; // latin small letter i with acute
			case 0xae: return 0xee; // latin small letter i with circumflex
			case 0xaf: return 0xef; // latin small letter i with diaeresis
			case 0xb0: return 0xf0; // latin small letter eth
			case 0xb1: return 0xf1; // latin small letter n with tilde
			case 0xb2: return 0xf2; // latin small letter o with grave
			case 0xb3: return 0xf3; // latin small letter o with acute
			case 0xb4: return 0xf4; // latin small letter o with circumflex
			case 0xb5: return 0xf5; // latin small letter o with tilde
			case 0xb6: return 0xf6; // latin small letter o with diaeresis
			case 0xb7: return 0xf7; // division sign
			case 0xb8: return 0xf8; // latin small letter o with stroke
			case 0xb9: return 0xf9; // latin small letter u with grave
			case 0xba: return 0xfa; // latin small letter u with acute
			case 0xbb: return 0xfb; // latin small letter u with circumflex
			case 0xbc: return 0xfc; // latin small letter u with diaeresis
			case 0xbd: return 0xfd; // latin small letter y with acute
			case 0xbe: return 0xfe; // latin small letter thorn
			case 0xbf: return 0xff; // latin small letter y with diaeresis
			default: return 0;
		}
	}
	else
		return 0;
}

// Make sure minimised and restored window state is noticed
// On windows as least, the minimise event is sometimes not seen
// Called from the 500 ms timer in draw_scene()
void check_minimised_or_restore_window(void)
{
	Uint32 flags = SDL_GetWindowFlags(el_gl_window);
	if (flags & (SDL_WINDOW_HIDDEN | SDL_WINDOW_MINIMIZED))
	{
		if (max_fps != min_fps)
			enter_minimised_state();
	}
	else if (flags & (SDL_WINDOW_SHOWN | SDL_WINDOW_MAXIMIZED))
	{
		if (max_fps != limit_fps)
			leave_minimised_state();
	}
}

#ifdef ANDROID
typedef struct
{
	float tfinger_x;
	float tfinger_y;
} LONG_TOUCH_STRUCT;

Uint32 gen_EVENT_LONG_TOUCH_callback(Uint32 interval, void *param)
{
	SDL_Event event;
	SDL_UserEvent userevent;
	userevent.type = SDL_USEREVENT;
	userevent.code = EVENT_LONG_TOUCH;
	userevent.data1 = param;
	userevent.data2 = NULL;
	event.type = SDL_USEREVENT;
	event.user = userevent;
	SDL_PushEvent(&event);
	return 0;
}
#endif

int HandleEvent (SDL_Event *event)
{
	int done = 0;
	int mouse_delta_x = 0;
	int mouse_delta_y = 0;
	Uint32 flags = KMOD_NONE;
	SDL_Keymod  mod_key_status = 0;
	Uint8 unicode = '\0';
#ifdef ANDROID
	static int last_mouse_x;
	static int last_mouse_y;
	static int last_mouse_flags;
	static SDL_bool have_finger_motion = SDL_FALSE;
	static Uint32 last_finger_down_start = 0;
	static SDL_bool have_usable_finger = SDL_FALSE;
	static Uint32 last_gesture_time = 0;
	static SDL_TimerID long_touch_timer_id = 0;
	static LONG_TOUCH_STRUCT long_touch_info = {0.0f, 0.0f};
#endif
	static Uint32 last_loss = 0;
	static Uint32 last_gain = 0;
	static Uint32 last_SDL_KEYDOWN_timestamp = 0;
	static Uint32 last_SDL_KEYDOWN_return_value = 0;
	static int el_input_focus = 1;

	if (event->type == SDL_FIRSTEVENT) return 0;

	mod_key_status = SDL_GetModState();
	flags |= (mod_key_status & KMOD_SHIFT) | (mod_key_status & KMOD_ALT) | (mod_key_status & KMOD_CTRL);

	switch( event->type )
	{
#ifdef ANDROID
		case SDL_APP_DIDENTERFOREGROUND:
			SDL_Log("App returned to forground");
			if (is_disconnected() && !locked_to_console)
			{
				SDL_Log("Reconnectecing after return to forground");
				connect_to_server();
			}
			break;

		case SDL_RENDER_DEVICE_RESET:
			SDL_Log("SDL_RENDER_DEVICE_RESET saving and exiting");
			save_local_data();
			exit(1);
			break;

		case SDL_APP_TERMINATING:
			SDL_Log("OS is terminating us...");
			// ANDROID_TODO radu removed the save in the latest version - "might cause problems"
			save_local_data();
			exit(1);
			break;

		case SDL_APP_WILLENTERBACKGROUND:
			SDL_Log("App entered background");
			save_local_data();
			break;
#endif

#if !defined(WINDOWS) && !defined(OSX) && !defined(ANDROID)
		case SDL_SYSWMEVENT:
			if (event->syswm.msg->msg.x11.event.type == SelectionNotify)
				finishpaste(event->syswm.msg->msg.x11.event.xselection);
			else if (event->syswm.msg->msg.x11.event.type == SelectionRequest)
				process_copy(&event->syswm.msg->msg.x11.event.xselectionrequest);
			break;
#endif

		case SDL_WINDOWEVENT:
			switch (event->window.event) {
				case SDL_WINDOWEVENT_HIDDEN:
				case SDL_WINDOWEVENT_MINIMIZED:
					if (clear_mod_keys_on_focus)
						last_loss = SDL_GetTicks();
					if (max_fps != min_fps)
						enter_minimised_state();
					break;
				case SDL_WINDOWEVENT_SHOWN:
				case SDL_WINDOWEVENT_EXPOSED:
				case SDL_WINDOWEVENT_MAXIMIZED:
				case SDL_WINDOWEVENT_RESTORED:
					if (last_loss && ((SDL_GetTicks() - last_loss) > 250))
					{
						last_loss = 0;
						SDL_SetModState(KMOD_NONE);
					}
					last_gain = SDL_GetTicks();
					if (max_fps != limit_fps)
						leave_minimised_state();
					break;
				case SDL_WINDOWEVENT_LEAVE:
				case SDL_WINDOWEVENT_FOCUS_LOST:
#ifndef ANDROID
					if (clear_mod_keys_on_focus)
						last_loss = SDL_GetTicks();
					el_input_focus = 0;
#endif
					break;
				case SDL_WINDOWEVENT_ENTER:
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					if (last_loss && ((SDL_GetTicks() - last_loss) > 250))
					{
						last_loss = 0;
						SDL_SetModState(KMOD_NONE);
					}
					last_gain = SDL_GetTicks();
					el_input_focus = 1;
					break;
				case SDL_WINDOWEVENT_SIZE_CHANGED:
				{
					Uint32 old_window_width = window_width, old_window_height = window_height;
					//printf("\nSDL_WINDOWEVENT_SIZE_CHANGED old=%dx%d new=%dx%d\n", old_window_width, old_window_height, event->window.data1, event->window.data2);
					update_window_size_and_scale();
					resize_all_root_windows(old_window_width, window_width, old_window_height, window_height);
					break;
				}
				default:
					//printf("untrapped SDL_WINDOWEVENT %x\n", event->window.event);
					break;
					
			}
			break;

		case SDL_TEXTEDITING:
			//printf("SDL_TEXTEDITING text=[%s] start=%d len=%d\n", (unsigned char *)event->edit.text, event->edit.start, event->edit.length);
			break;

		case SDL_TEXTINPUT:
			if (afk_time) // if enabled...
				last_action_time = cur_time;  // reset the AFK timer
			cm_post_show_check(1); // forces any context menu to close
			unicode = utf8_to_unicode(event->text.text);
#ifdef ANDROID
			if (enable_keyboard_debug)
			{
				char str[200];
				safe_snprintf(str, sizeof(str), "SDL_TEXTINPUT text=[%s] len=%" PRI_SIZET ",%" PRI_SIZET " timestamp=%u", (unsigned char *)event->text.text, sizeof(event->text.text), strlen(event->text.text), event->key.timestamp);
				LOG_TO_CONSOLE(c_green1, str);
				safe_snprintf(str, sizeof(str), "SDL_TEXTINPUT UTF-8 udf8=(%x,%x) unicode=%x", event->text.text[0], event->text.text[1], unicode);
				LOG_TO_CONSOLE(c_green1, str);
			}
#endif
			//printf("SDL_TEXTINPUT text=[%s] len=%lu,%lu timestamp=%u\n", (unsigned char *)event->text.text, sizeof(event->text.text), strlen(event->text.text), event->key.timestamp);
			//printf("UTF-8 udf8=(%x,%x) unicode=%x\n", event->text.text[0], event->text.text[1], unicode);
			if (unicode)
			{
				if (((event->key.timestamp - last_SDL_KEYDOWN_timestamp) > 10) || (last_SDL_KEYDOWN_return_value == -1))
					keypress_in_windows (mouse_x, mouse_y, SDLK_UNKNOWN, unicode, KMOD_NONE);
			}
			break;
#ifdef ANDROID

		case SDL_KEYUP:
			if (event->key.keysym.sym == SDLK_AC_BACK)
				back_on = 0;
			break;
#endif

		case SDL_KEYDOWN:
			// Don't let the modifiers GUI, ALT, CTRL and SHIFT change the state if only the key pressed
			if ((event->key.keysym.sym != SDLK_LSHIFT) && (event->key.keysym.sym != SDLK_RSHIFT) &&
				(event->key.keysym.sym != SDLK_LCTRL) && (event->key.keysym.sym != SDLK_RCTRL) &&
				(event->key.keysym.sym != SDLK_LALT) && (event->key.keysym.sym != SDLK_RALT) &&
				(event->key.keysym.sym != SDLK_LGUI) && (event->key.keysym.sym != SDLK_RGUI))
			{
				if (afk_time) // if enabled...
					last_action_time = cur_time;  // reset the AFK timer
				cm_post_show_check(1); // forces any context menu to close
			}
			// Don't use a TAB key dangling from system window switching.  By default this would toggle the map window.
			if (last_gain && (event->key.keysym.sym == SDLK_TAB) && ((SDL_GetTicks() - last_gain) < 50))
				break;
			last_gain = 0;
#ifdef ANDROID
			if (enable_keyboard_debug)
			{
				char str[200];
				safe_snprintf(str, sizeof(str), "SDL_KEYDOWN keycode=%u,[%s] mod=%u timestamp=%u", event->key.keysym.sym, SDL_GetKeyName(event->key.keysym.sym), event->key.keysym.mod, event->key.timestamp);
				LOG_TO_CONSOLE(c_green1, str);
			}
#endif
			//printf("SDL_KEYDOWN keycode=%u,[%s] mod=%u timestamp=%u\n", event->key.keysym.sym, SDL_GetKeyName(event->key.keysym.sym), event->key.keysym.mod, event->key.timestamp);
			last_SDL_KEYDOWN_timestamp = event->key.timestamp;
#ifdef ANDROID
			if(event->key.keysym.sym == SDLK_AC_BACK)
			{
				last_SDL_KEYDOWN_return_value = 1;
				close_last_window();
				back_on = 1;
				break;
			}
			// ANDROID_TODO - the hardware keyboard on Android does not produce SDL_TEXTINPUT for SDLK_SPACE, so fix here
			last_SDL_KEYDOWN_return_value = keypress_in_windows (mouse_x, mouse_y, event->key.keysym.sym,
				(event->key.keysym.sym == SDLK_SPACE ?SDLK_SPACE :0), event->key.keysym.mod);
#else
			last_SDL_KEYDOWN_return_value = keypress_in_windows (mouse_x, mouse_y, event->key.keysym.sym, 0, event->key.keysym.mod);
#endif
			//printf("SDL_KEYDOWN result=%d\n", last_SDL_KEYDOWN_return_value);
			break;

		case SDL_QUIT:
		case SDL_WINDOWEVENT_CLOSE:
			//printf("SDL_QUIT\n");
			done = 1;
			break;

#ifdef ANDROID
		case SDL_MULTIGESTURE:
			SDL_RemoveTimer(long_touch_timer_id);
			have_finger_motion = SDL_FALSE;
			last_finger_down_start = 0;
			last_gesture_time = SDL_GetTicks();
			multi_gesture_in_windows(event->mgesture.timestamp, event->mgesture.x, event->mgesture.y, event->mgesture.dDist, event->mgesture.dTheta);
			break;

		case SDL_FINGERMOTION:
			if (((SDL_GetTicks() - last_gesture_time) > 100) &&
				((SDL_GetTicks() - last_finger_down_start) > (motion_touch_delay_s * 1000)))
			{
				int drag_x = (int)(event->tfinger.x * window_width + 0.5);
				int drag_y = (int)(event->tfinger.y * window_height + 0.5);
				int drag_dx = (int)(event->tfinger.dx * window_width + 0.5);
				int drag_dy = (int)(event->tfinger.dy * window_height + 0.5);

				if ((abs(drag_dx) > 0) || (abs(drag_dy) > 0))
				{
					if (drag_windows (drag_x, drag_y, drag_dx, drag_dy) >= 0)
					{
						SDL_RemoveTimer(long_touch_timer_id);
						last_finger_down_start = 0;
						have_finger_motion = SDL_TRUE;
						return done;
					}

					if (drag_in_windows (drag_x, drag_y, ELW_LEFT_MOUSE, drag_dx, drag_dy) >= 0)
					{
						SDL_RemoveTimer(long_touch_timer_id);
						last_finger_down_start = 0;
						have_finger_motion = SDL_TRUE;
						return done;
					}
				}

				if ((abs(drag_dx) > 3) || (abs(drag_dy) > 3))
				{
					SDL_RemoveTimer(long_touch_timer_id);
					last_finger_down_start = 0;
					have_finger_motion = SDL_TRUE;
					finger_motion_in_windows(event->tfinger.timestamp, event->tfinger.x, event->tfinger.y, event->tfinger.dx, event->tfinger.dy);
				}
			}
			break;

		case SDL_FINGERDOWN:
			last_finger_down_start = SDL_GetTicks();
			have_usable_finger = SDL_FALSE;
			have_finger_motion = SDL_FALSE;
			SDL_RemoveTimer(long_touch_timer_id);
			long_touch_info.tfinger_x = event->tfinger.x; long_touch_info.tfinger_y = event->tfinger.y;
			long_touch_timer_id = SDL_AddTimer((Uint32)(1000 * long_touch_delay_s), gen_EVENT_LONG_TOUCH_callback, &long_touch_info);
			break;

		case SDL_FINGERUP:
			SDL_RemoveTimer(long_touch_timer_id);
			if (have_finger_motion)
			{
				have_finger_motion = SDL_FALSE;
				end_drag_windows();
			}
			if (last_finger_down_start == 0)
				break;
			if (afk_time)
				last_action_time = cur_time;	// Set the latest events - don't make mousemotion set the afk_time... (if you prefer that mouse motion sets/resets the afk_time, then move this one step below...
			have_usable_finger = SDL_TRUE;
			last_finger_down_start = 0;
			flags |= ELW_LEFT_MOUSE;
			mouse_x = (int)(0.5 + window_width * event->tfinger.x);
			mouse_y = (int)(0.5 + window_height * event->tfinger.y);
			last_mouse_x = mouse_x;
			last_mouse_y = mouse_y;
			last_mouse_flags = flags;
			click_in_windows (mouse_x, mouse_y, flags);
			break;

#else // ANDROID
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			// make sure the mouse button is our window, or else we ignore it
			//Checking if we have keyboard focus for a mouse click is wrong, but SDL doesn't care to tell us we have mouse focus when someone alt-tabs back in and the mouse was within bounds of both other and EL windows. Blech.
			if(event->button.x >= window_width || event->button.y >= window_height || !((SDL_GetWindowFlags(el_gl_window) & SDL_WINDOW_MOUSE_FOCUS) || el_input_focus))
			{
				break;
			}

			if (afk_time && event->type == SDL_MOUSEBUTTONDOWN)
				last_action_time = cur_time;	// Set the latest events - don't make mousemotion set the afk_time... (if you prefer that mouse motion sets/resets the afk_time, then move this one step below...

			// fallthrough
		case SDL_MOUSEMOTION:
		case SDL_MOUSEWHEEL:
			if (have_mouse)
			{
				mouse_x = window_width/2;
				mouse_y = window_height/2;
				highdpi_scale(&mouse_x, &mouse_y);

				mouse_delta_x= event->motion.xrel;
				mouse_delta_y= event->motion.yrel;
				highdpi_scale(&mouse_delta_x, &mouse_delta_y);
			}
			else if(event->type==SDL_MOUSEMOTION)
			{
				mouse_x = event->motion.x;
				mouse_y = event->motion.y;
				highdpi_scale(&mouse_x, &mouse_y);

				mouse_delta_x = event->motion.xrel;
				mouse_delta_y = event->motion.yrel;
				highdpi_scale(&mouse_delta_x, &mouse_delta_y);
			}
			else if(event->type==SDL_MOUSEWHEEL)
			{
				SDL_GetMouseState(&mouse_x, &mouse_y);
				highdpi_scale(&mouse_x, &mouse_y);
			}
			else
			{
#ifdef NEW_CURSOR
				if (sdl_cursors)
				{
#endif // NEW_CURSOR
					mouse_x= event->button.x;
					mouse_y= event->button.y;
					highdpi_scale(&mouse_x, &mouse_y);
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

				if (event->type == SDL_MOUSEMOTION && ((event->motion.state & SDL_BUTTON_MMASK) || (mod_key_status & KMOD_GUI)))
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

			if (left_click) flags |= ELW_LEFT_MOUSE;
			if (middle_click || (mod_key_status & KMOD_GUI)) flags |= ELW_MID_MOUSE;
			if (right_click) flags |= ELW_RIGHT_MOUSE;
			if (event->type == SDL_MOUSEWHEEL)
			{
				if (event->wheel.y > 0)
					flags |= ELW_WHEEL_UP;
				else if (event->wheel.y < 0)
					flags |= ELW_WHEEL_DOWN;
			}

			if ( left_click == 1 || right_click == 1 || middle_click == 1 || (flags & (ELW_WHEEL_UP | ELW_WHEEL_DOWN) ) )
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
#endif // ANDROID

		case SDL_USEREVENT:
			switch(event->user.code){
			case	EVENT_MOVEMENT_TIMER:
				pf_move();
				break;
#ifdef ANDROID
			case	EVENT_CURSOR_CALCULATION_COMPLETE:
				if (have_usable_finger)
					click_in_windows (last_mouse_x, last_mouse_y, last_mouse_flags);
				break;
			case	EVENT_LONG_TOUCH:
			{
				LONG_TOUCH_STRUCT *info = (LONG_TOUCH_STRUCT *)event->user.data1;
				if (info != NULL)
				{
					last_mouse_x = mouse_x = (int)(0.5 + window_width * info->tfinger_x);
					last_mouse_y = mouse_y = (int)(0.5 + window_height * info->tfinger_y);
					last_mouse_flags = flags |= ELW_RIGHT_MOUSE;
					have_usable_finger = SDL_TRUE;
					last_finger_down_start = 0;
					click_in_windows (mouse_x, mouse_y, flags);
				}
				break;
			}
#endif
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
				unload_actor_texture_cache();
				break;
#endif	/* CUSTOM_UPDATE */
			}
			break;

		default:
			//printf("untrapped event %x\n", event->type);
			break;

	}

	return(done);
}
