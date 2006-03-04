#include <stdlib.h>
#include <string.h>
#include "global.h"

int map_root_win = -1;
int showing_continent = 0;

int mouse_over_minimap = 0;

int click_map_handler (window_info *win, int mx, int my, Uint32 flags)
{
	Uint32 ctrl_on = flags & ELW_CTRL;
	Uint32 left_click = flags & ELW_LEFT_MOUSE;
	Uint32 right_click = flags & ELW_RIGHT_MOUSE;
	float scale = (float) (win->len_x-hud_x) / 300.0f;

	if (left_click && mx > 0 && mx < 50*scale && my > 0 && my < 55*scale)
	{
		showing_continent = !showing_continent;
	} else if (!showing_continent)
	{
		if (left_click)
		{
			pf_move_to_mouse_position ();
		}
		else if (right_click)
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
		CHECK_GL_ERRORS ();
	}	

	draw_delay = 20;
	return 1;
}

int mouseover_map_handler (window_info *win, int mx, int my)
{
	float scale = (float) (win->len_x-hud_x) / 300.0f;

	if (mx > 0 && mx < 50*scale && my > 0 && my < 55*scale)
	{
		mouse_over_minimap = 1;
	}
	else
	{
		mouse_over_minimap = 0;
	}
	return mouse_over_minimap;
}	

int keypress_map_handler (window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
	Uint8 ch = key_to_char (unikey);

	if (ch == SDLK_RETURN && adding_mark && input_text_line.len > 0)
	{
		int i;

		// if text wrapping just keep the text until the wrap.
		for (i = 0; i < input_text_line.len; i++) 
		{
			if (input_text_line.data[i] == '\n') 
			{
				input_text_line.data[i] = '\0';
				break;
			}
		}
						    
		marks[max_mark].x = mark_x;
		marks[max_mark].y = mark_y;
		memset ( marks[max_mark].text, 0, sizeof (marks[max_mark].text) );
						  
		my_strncp ( marks[max_mark].text, input_text_line.data, sizeof (marks[max_mark].text) );
		max_mark++;
		save_markings ();
		adding_mark = 0;
		clear_input_line ();
	}
	// does the user want to cancel a mapmark?
	else if (ch == SDLK_ESCAPE && adding_mark)
	{
		input_text_line.len=0;
		input_text_line.data[0] = '\0';
		adding_mark = 0;
		clear_input_line ();
	}
	// now try the keypress handler for all root windows
	else if ( keypress_root_common (key, unikey) )
	{
		return 1;
	}
	else if (key == K_MAP)
	{
		switch_from_game_map ();
		hide_window (map_root_win);
		show_window (game_root_win);
		// Undo stupid quickbar hack
		if ( !get_show_window (quickbar_win) )
			show_window (quickbar_win);
	}
	else
	{
#ifdef COMMAND_BUFFER
		reset_tab_completer();
#endif //COMMAND_BUFFER
		if (ch == '`' || key == K_CONSOLE)
		{
			switch_from_game_map ();
			hide_window (map_root_win);
			show_window (console_root_win);
		}
		else if (ch == SDLK_RETURN && input_text_line.len > 0 && (input_text_line.data[0] == char_cmd_str[0] || input_text_line.data[0] == '#'))
		{
#ifdef COMMAND_BUFFER
			if(test_for_console_command (input_text_line.data, input_text_line.len) || (input_text_line.data[0] == char_cmd_str[0] || input_text_line.data[0] == '#')) {
				add_line_to_history(input_text_line.data, input_text_line.len);
			}
#else
			test_for_console_command (input_text_line.data, input_text_line.len);
#endif //COMMAND_BUFFER
			// also clear the buffer
			clear_input_line (); 
		}
		else if ( text_input_handler (key, unikey) )
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

int show_map_handler (window_info *win)
{
	hide_window(book_win);
	hide_window(paper_win);
	hide_window(color_race_win);
	hide_window(tab_bar_win);
	return 1;
}

void create_map_root_window (int width, int height)
{
	if (map_root_win < 0)
	{
		map_root_win = create_window ("Map", -1, -1, 0, 0, width, height, ELW_TITLE_NONE|ELW_SHOW_LAST);
	
		set_window_handler (map_root_win, ELW_HANDLER_DISPLAY, &display_map_handler);
		set_window_handler (map_root_win, ELW_HANDLER_KEYPRESS, &keypress_map_handler);
		set_window_handler (map_root_win, ELW_HANDLER_CLICK, &click_map_handler);
		set_window_handler (map_root_win, ELW_HANDLER_MOUSEOVER, &mouseover_map_handler);
		set_window_handler (map_root_win, ELW_HANDLER_SHOW, &show_map_handler);
	}
}
