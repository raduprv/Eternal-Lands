#include <string.h>
#include "global.h"

int console_win = -1;

#ifdef WINDOW_CHAT

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
	Uint8 ch = unikey & 0xff;
	
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
		if ( switch_to_game_map () )
		{
			hide_window (console_win);
			show_window (map_win);
			interface_mode = interface_map;
		}
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
			hide_window (console_win);
			show_window (game_win);
			interface_mode = interface_game;
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
				// XXX FIXME (Grum): this should probably only happen in map mode,
				// and shouldn't occur here. Leave it for now.
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
			else
			{
				test_for_console_command();
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
