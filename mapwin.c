#include <string.h>
#include "global.h"

int map_win = -1;
int showing_continent = 0;

int mouse_over_minimap = 0;

#ifndef OLD_EVENT_HANDLER

int click_map_handler (window_info *win, int mx, int my, Uint32 flags)
{
	Uint32 ctrl_on = flags & ELW_CTRL;
	float scale = (float) (win->len_x-hud_x) / 300.0f;

	if (left_click == 1 && mx > 0 && mx < 50*scale && my > 0 && my < 55*scale)
	{
		showing_continent = !showing_continent;
	} else if (!showing_continent)
	{
		if (left_click==1)
		{
			pf_move_to_mouse_position ();
		}
		else if (right_click == 1)
		{
			if (!ctrl_on)
				put_mark_on_map_on_mouse_position ();
			else
				delete_mark_on_map_on_mouse_position ();
		}
	}
	
	return 1;
}

int display_map_handler ()
{
	// are we actively drawing things?
	if (SDL_GetAppState () & SDL_APPACTIVE)
	{
		draw_hud_interface ();
		Leave2DMode ();
		draw_game_map (!showing_continent, mouse_over_minimap);
		Enter2DMode ();
		check_gl_errors ();
	}	

	draw_delay = 20;
	return 1;
}

int mouseover_map_handler (window_info *win, int mx, int my)
{
	float scale = (float) (win->len_x-hud_x) / 300.0f;
	
	if (mx > 0 && mx < 50*scale && my > 0 && my < 55*scale)
		mouse_over_minimap = 1;
	else
		mouse_over_minimap = 0;
	
	return 1;
}	

int keypress_map_handler (window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
	// first try the keypress handler for all root windows
	if ( keypress_root_common (key, unikey) )
	{
		return 1;
	}
	else if (key == K_MAP)
	{
		switch_from_game_map ();
		hide_window (map_win);
		show_window (game_win);
		interface_mode = interface_game;
	}
	else
	{
		Uint8 ch = key_to_char (unikey);

		if (ch == '`' || key == K_CONSOLE)
		{
			switch_from_game_map ();
			hide_window (map_win);
			show_window (console_win);
			interface_mode = interface_console;
		}
		else if (ch == SDLK_RETURN && !adding_mark && input_text_lenght > 0 && input_text_line[0] == '#')
		{
			test_for_console_command ();
			// also clear the buffer
			input_text_lenght = 0;
			input_text_lines = 1;
			input_text_line[0] = '\0';
		}
		else if ( text_input_handler (ch) )
		{
			return 1;
		}
		else
		{
			// nothing we can handle
			return 0;
		}
	}
	
	// we handled it, return 1 to let the window manager know
	return 1;
}

void create_map_window ()
{
	if (map_win < 0)
	{
		map_win = create_window ("Map", -1, -1, 0, 0, window_width, window_height, ELW_TITLE_NONE|ELW_SHOW_LAST);
	
		set_window_handler (map_win, ELW_HANDLER_DISPLAY, &display_map_handler);
		set_window_handler (map_win, ELW_HANDLER_KEYPRESS, &keypress_map_handler);
		set_window_handler (map_win, ELW_HANDLER_CLICK, &click_map_handler);
		set_window_handler (map_win, ELW_HANDLER_MOUSEOVER, &mouseover_map_handler);
	}
}

#endif
