#include <string.h>
#include "global.h"

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
	static int msg = 0, offset = 0;
	const char *sep_string = "^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^";
	
	// are we actively drawing things?
	if (SDL_GetAppState () & SDL_APPACTIVE)
	{
		if (console_text_changed)
		{
			find_line_nr (total_nr_lines, total_nr_lines - nr_console_lines - scroll_up_lines, CHANNEL_ALL, &msg, &offset);		
			text_field_set_buf_pos (console_root_win, console_out_id, msg, offset);
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
	else if (keysym == SDLK_UP && total_nr_lines > nr_console_lines + scroll_up_lines)
	{
		scroll_up_lines++;
		console_text_changed = 1;
	}
	else if (keysym == SDLK_DOWN && scroll_up_lines > 0)
	{
		scroll_up_lines--;
		console_text_changed = 1;
	}
	else if (keysym == SDLK_PAGEUP && total_nr_lines > nr_console_lines + scroll_up_lines)
	{
		scroll_up_lines += nr_console_lines - 1;
		if (nr_console_lines + scroll_up_lines > total_nr_lines)
			scroll_up_lines = total_nr_lines - nr_console_lines;
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
		}
	}
	else
	{
		Uint8 ch = key_to_char (unikey);

		if (ch == '`' || key == K_CONSOLE)
		{
			hide_window (console_root_win);
			show_window (game_root_win);
			// Undo stupid quickbar hack
			if ( !get_show_window (quickbar_win) )
				show_window (quickbar_win);
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

int click_console_handler (window_info *win, int mx, int my, Uint32 flags)
{
	if ( (flags & ELW_WHEEL_UP) && total_nr_lines > nr_console_lines + scroll_up_lines )
	{
		scroll_up_lines++;
		console_text_changed = 1;
	}
	else if ( (flags & ELW_WHEEL_DOWN) && scroll_up_lines > 0 )
	{
		scroll_up_lines--;
		console_text_changed = 1;
	}
	else
	{
		return 0; // we didn't handle it
	}
	
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
		set_window_handler (console_root_win, ELW_HANDLER_CLICK, &click_console_handler);
		
		console_out_id = text_field_add_extended (console_root_win, console_out_id, NULL, 0, 0, width - hud_x, height - CONSOLE_INPUT_HEIGHT - CONSOLE_SEP_HEIGHT - hud_y, 0, 1.0, -1.0f, -1.0f, -1.0f, display_text_buffer, DISPLAY_TEXT_BUFFER_SIZE, CHANNEL_ALL, 0, 0, -1.0f, -1.0f, -1.0f);
		// initialize the input field without the default keypress
		// handler, since that's not really applicable here
		console_in_id = text_field_add_extended (console_root_win, console_in_id, NULL, 0, height - CONSOLE_INPUT_HEIGHT - hud_y, width - hud_x, CONSOLE_INPUT_HEIGHT, TEXT_FIELD_EDITABLE|TEXT_FIELD_NOKEYPRESS, 1.0, -1.0f, -1.0f, -1.0f, &input_text_line, 1, CHANNEL_ALL, 0, 0, 1.0f, 1.0f, 1.0f);
		
		nr_console_lines = (height - CONSOLE_INPUT_HEIGHT -  CONSOLE_SEP_HEIGHT - hud_y) / 18;
	}
}
