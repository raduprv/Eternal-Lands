#include <string.h>
#include "global.h"

int console_win = -1;

#ifndef OLD_EVENT_HANDLER

int display_console_handler (window_info *win)
{
	// are we actively drawing things?
	if (SDL_GetAppState () & SDL_APPACTIVE)
	{
		draw_console_pic (cons_text);
		display_console_text ();
		draw_hud_interface ();
	}
	
	draw_delay = 20;
	return 1;
}

int keypress_console_handler (window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
	Uint16 keysym = key & 0xffff;
	
	// first try the keypress handler for all root windows
	if ( keypress_root_common (key, unikey) )
	{
		return 1;
	}
	else if (keysym == SDLK_UP)
	{
		console_move_up ();
	}
	else if (keysym == SDLK_DOWN)
	{
		console_move_down ();
	}
	else if (keysym == SDLK_PAGEDOWN)
	{
		console_move_page_down ();
	}
	else if (keysym == SDLK_PAGEUP)
	{
		console_move_page_up();
	}
	else if (key == K_MAP)
	{
		if ( switch_to_game_map () )
		{
			hide_window (console_win);
			show_window (map_win);
			interface_mode = INTERFACE_MAP;
		}
	}
	else
	{
		Uint8 ch = key_to_char (unikey);

		if (ch == '`' || key == K_CONSOLE)
		{
			hide_window (console_win);
			show_window (game_win);
			interface_mode = INTERFACE_GAME;
		}
		else if (ch == SDLK_RETURN && input_text_lenght > 0)
		{
			if ( input_text_lenght == 1 || (adding_mark != 1 && input_text_line[0] != '%') )
			{
				test_for_console_command ();
				// also clear the buffer
				input_text_lenght = 0;
				input_text_lines = 1;
				input_text_line[0] = '\0';
			}
			else if ( !text_input_handler (ch) )
			{
				return 0;
			}
		}
		else if ( !text_input_handler (ch) )
		{
			// nothing we can handle
			return 0;
		}
	}

	// we handled it, return 1 to let the window manager know
	return 1;	
}

void create_console_window ()
{
	if (console_win < 0)
	{
		console_win = create_window ("Console", -1, -1, 0, 0, window_width, window_height, ELW_TITLE_NONE|ELW_SHOW_LAST);
		
		set_window_handler (console_win, ELW_HANDLER_DISPLAY, &display_console_handler);
		set_window_handler (console_win, ELW_HANDLER_KEYPRESS, &keypress_console_handler);
	}
}

#endif
