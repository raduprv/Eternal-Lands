#include <string.h>
#include "global.h"

#ifndef OLD_EVENT_HANDLER

#define CONSOLE_INPUT_HEIGHT	(3*18)
#define CONSOLE_SEP_HEIGHT	18

int console_root_win = -1;

int console_out_id = 40;
int console_in_id = 41;

int nr_console_lines;
int scroll_up_lines = 0;
int console_text_changed = 0;

void update_console_win (int nlines)
{
	if (scroll_up_lines == 0)
		console_text_changed = 1;
	else
		scroll_up_lines += nlines;
}

int display_console_handler (window_info *win)
{
	static int line_start = 0;
	const char *sep_string = "^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^";
	
	// are we actively drawing things?
	if (SDL_GetAppState () & SDL_APPACTIVE)
	{
		if (console_text_changed)
		{
			int ichar;
			Uint8 ch;
		
			line_start = find_line_nr (nr_text_buffer_lines - nr_console_lines - scroll_up_lines);
			if (line_start < 0) line_start = 0;
		
			// see if this line starts with a color code
			ch = display_text_buffer[line_start];
			if (ch < 127 || ch > 127 + c_grey4)
			{
				// nope, search backwards for the last color
				// code
				for (ichar = line_start; ichar >= 0; ichar--)
				{
					ch = display_text_buffer[ichar];
					if (ch >= 127 && ch <= 127 + c_grey4)
					{
						float r, g, b;
						ch -= 127;
						r = colors_list[ch].r1 / 255.0f;
						g = colors_list[ch].g1 / 255.0f;
						b = colors_list[ch].b1 / 255.0f;
						text_field_set_text_color (console_root_win, console_out_id, r, g, b);
						break;
					}
				}
			}
		
			text_field_set_buf_offset (console_root_win, console_out_id, line_start);
			console_text_changed = 0;
		}
		
	
		draw_console_pic (cons_text);
		if (scroll_up_lines != 0)
		{
			glColor3f (1.0, 1.0, 1.0);
			draw_string_clipped (0, win->len_y - CONSOLE_INPUT_HEIGHT - CONSOLE_SEP_HEIGHT - hud_y, sep_string, win->len_x - hud_x, CONSOLE_SEP_HEIGHT);
		}
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
	else if (keysym == SDLK_UP && nr_text_buffer_lines > nr_console_lines + scroll_up_lines)
	{
		scroll_up_lines++;
		console_text_changed = 1;
	}
	else if (keysym == SDLK_DOWN  && scroll_up_lines > 0)
	{
		scroll_up_lines--;
		console_text_changed = 1;
	}
	else if (keysym == SDLK_PAGEUP && nr_text_buffer_lines > nr_console_lines + scroll_up_lines)
	{
		scroll_up_lines += nr_console_lines - 1;
		if (nr_console_lines + scroll_up_lines > nr_text_buffer_lines)
			scroll_up_lines = nr_text_buffer_lines - nr_console_lines;
		console_text_changed = 1;
	}
	else if (keysym == SDLK_PAGEDOWN && scroll_up_lines > 0)
	{
		scroll_up_lines -= nr_console_lines - 1;
		if (scroll_up_lines < 0) scroll_up_lines = 0;
		console_text_changed = 1;
	}
	else if (key == K_MAP)
	{
		if ( switch_to_game_map () )
		{
			hide_window (console_root_win);
			show_window (map_root_win);
			interface_mode = INTERFACE_MAP;
		}
	}
	else
	{
		Uint8 ch = key_to_char (unikey);

		if (ch == '`' || key == K_CONSOLE)
		{
			hide_window (console_root_win);
			show_window (game_root_win);
			interface_mode = INTERFACE_GAME;
		}
		else if (ch == SDLK_RETURN && input_text_lenght > 0)
		{
			if ( input_text_lenght == 1 || (adding_mark != 1 && input_text_line[0] != '%') )
			{
				test_for_console_command (input_text_line, input_text_lenght);
				// also clear the buffer
				input_text_lenght = 0;
				input_text_lines = 1;
				input_text_line[0] = '\0';
			}
			else if ( !text_input_handler (key, unikey) )
			{
				return 0;
			}
		}
		else if ( !text_input_handler (key, unikey) )
		{
			// nothing we can handle
			return 0;
		}
	}

	// we handled it, return 1 to let the window manager know
	return 1;	
}

int resize_console_handler (window_info *win, int width, int height)
{
	widget_resize (console_root_win, console_out_id, width - hud_x, height - hud_y - CONSOLE_INPUT_HEIGHT - CONSOLE_SEP_HEIGHT);
	widget_resize (console_root_win, console_in_id, width - hud_x, CONSOLE_INPUT_HEIGHT);
	widget_move (console_root_win, console_in_id, 0, height - hud_y - CONSOLE_INPUT_HEIGHT);
	
	nr_console_lines = (height - CONSOLE_INPUT_HEIGHT - CONSOLE_SEP_HEIGHT - hud_y) / 18;
	
	return 1;
}

void create_console_root_window (int width, int height)
{
	if (console_root_win < 0)
	{
		console_root_win = create_window ("Console", -1, -1, 0, 0, width, height, ELW_TITLE_NONE|ELW_SHOW_LAST);
		
		set_window_handler (console_root_win, ELW_HANDLER_DISPLAY, &display_console_handler);
		set_window_handler (console_root_win, ELW_HANDLER_KEYPRESS, &keypress_console_handler);
		set_window_handler (console_root_win, ELW_HANDLER_RESIZE, &resize_console_handler);
		
		console_out_id = text_field_add_extended (console_root_win, console_out_id, NULL, 0, 0, width - hud_x, height - CONSOLE_INPUT_HEIGHT - CONSOLE_SEP_HEIGHT - hud_y, 0, 1.0, -1.0f, -1.0f, -1.0f, display_text_buffer, MAX_DISPLAY_TEXT_BUFFER_LENGTH, 0, 0, -1.0f, -1.0f, -1.0f);
		console_in_id = text_field_add_extended (console_root_win, console_in_id, NULL, 0, height - CONSOLE_INPUT_HEIGHT - hud_y, width - hud_x, CONSOLE_INPUT_HEIGHT, TEXT_FIELD_EDITABLE, 1.0, -1.0f, -1.0f, -1.0f, input_text_line, sizeof (input_text_line), 0, 0, 1.0f, 1.0f, 1.0f);
		
		nr_console_lines = (height - CONSOLE_INPUT_HEIGHT -  CONSOLE_SEP_HEIGHT - hud_y) / 18;
	}
}

#endif // not def OLD_EVENT_HANDLER
