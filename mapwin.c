#include <string.h>
#include "global.h"

int map_win = -1;
int showing_continent = 0;

int mouse_over_minimap = 0;

#ifdef WINDOW_CHAT

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
		Enter2DMode ();
		draw_hud_interface ();
		Leave2DMode ();
		draw_game_map (!showing_continent, mouse_over_minimap);
		check_gl_errors ();
	}
	SDL_Delay (20);
	Enter2DMode ();

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
	Uint8 ch = unikey & 0xff;
	
	// first try the keypress handler for all root windows
	if ( keypress_root_common (key, unikey) )
	{
		return 1;
	}
        // handle K_STATS?
        // handle K_OPTIONS?
        // handle K_KNOWLEDGE?
        // handle K_ENCYCLOPEDIA?
        // handle K_HELP?
        // handle K_SIGILS
        // handle K_MANUFACTURE
        // handle K_ITEMS
	else if (key == K_MAP)
	{
		switch_from_game_map ();
		hide_window (map_win);
		show_window (game_win);
		interface_mode = interface_game;
	}
	else
	{
		if ( (key >= 256 && key <= 267) || key==271)
		{
			switch (key)
			{
				case 266:
					ch = 46;
					break;
				case 267:
					ch = 47;
					break;
				case 271:
					ch = 13;
					break;
				default:
					ch = key-208;
					break;
			}
		}

		if (ch == '`' || key == K_CONSOLE)
		{
			switch_from_game_map ();
			hide_window (map_win);
			show_window (console_win);
			interface_mode = interface_console;
		}
		else if (key == K_SHADOWS)
		{
			clouds_shadows = !clouds_shadows;
		}
		else if ( ( (ch >= 32 && ch <= 126) || (ch > 127 + c_grey4) ) && input_text_lenght < 160)
		{
			// watch for the '//' shortcut
			if (input_text_lenght == 1 && ch== '/' && input_text_line[0] == '/' && last_pm_from[0])
			{
				int i;
				int l = strlen (last_pm_from);
				for (i=0; i<l; i++) input_text_line[input_text_lenght++] = last_pm_from[i];
				put_char_in_buffer (' ');
			}
			else
			{
				// not the shortcut, add the character to the buffer
				put_char_in_buffer(ch);
			}
		}
		else if (ch == SDLK_BACKSPACE && input_text_lenght > 0)
		{
			input_text_lenght--;
			if (input_text_line[input_text_lenght] == 0x0a)
			{
				input_text_lenght--;
				if (input_text_lines > 1) input_text_lines--;
				input_text_line[input_text_lenght] = '_';
				input_text_line[input_text_lenght+1] = 0;
			}
			input_text_line[input_text_lenght] = '_';
			input_text_line[input_text_lenght+1] = 0;
		}
		else if (ch == SDLK_RETURN && input_text_lenght > 0)
		{
			if ( (adding_mark == 1) && (input_text_lenght > 1) )
			{
				int i;
				
				// if text wrapping just keep the text until the wrap.
				for (i = 0; i < strlen (input_text_line); i++) 
					if (input_text_line[i] == 0x0a) 
						input_text_line[i] = 0;
							    
				marks[max_mark].x = mark_x;
				marks[max_mark].y = mark_y;
				memset(marks[max_mark].text,0,500);
						  
				my_strncp (marks[max_mark].text, input_text_line, 500);
				marks[max_mark].text[strlen(marks[max_mark].text)-1] = 0;
				max_mark++;
				save_markings ();
				adding_mark = 0;
				input_text_lenght = 0;
				input_text_lines = 1;
				input_text_line[0] = 0;
			}
			else if (*input_text_line == '%' && input_text_lenght > 1) 
			{
				input_text_line[input_text_lenght] = 0;
				if ( (check_var (input_text_line + 1, 1) ) < 0)
					send_input_text_line();
			}
			else if (*input_text_line == '#')
			{
				test_for_console_command();
			}
			else
			{
				send_input_text_line();
			}
			//also clear the buffer
			input_text_lenght = 0;
			input_text_lines = 1;
			input_text_line[0] = 0;
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
	map_win = create_window ("Map", -1, -1, 0, 0, window_width, window_height, ELW_WIN_INVISIBLE|ELW_SHOW_LAST);
	
	set_window_handler (map_win, ELW_HANDLER_DISPLAY, &display_map_handler);
	set_window_handler (map_win, ELW_HANDLER_KEYPRESS, &keypress_map_handler);
	set_window_handler (map_win, ELW_HANDLER_CLICK, &click_map_handler);
	set_window_handler (map_win, ELW_HANDLER_MOUSEOVER, &mouseover_map_handler);
	
	hide_window (map_win);
}

#endif
