#include <stdlib.h>
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
int console_text_width = -1;

int locked_to_console = 0;

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
		int y_line;

		if (console_text_changed)
		{
			find_line_nr (total_nr_lines, total_nr_lines - nr_console_lines - scroll_up_lines, FILTER_ALL, &msg, &offset);		
			text_field_set_buf_pos (console_root_win, console_out_id, msg, offset);
			console_text_changed = 0;
		}
		
	
		draw_console_pic (cons_text);
		if (scroll_up_lines != 0)
		{
			glColor3f (1.0, 1.0, 1.0);
			draw_string_zoomed_clipped (10, 10 + win->len_y - CONSOLE_INPUT_HEIGHT - CONSOLE_SEP_HEIGHT - hud_y, sep_string, -1, win->len_x - hud_x - 20, CONSOLE_SEP_HEIGHT, chat_zoom);
		}

		draw_hud_interface ();

		y_line = win->len_y - (17 * (4+(int)((get_string_width(input_text_line.data)*11.0f/12.0f)/(win->len_x-hud_x-20))));
		glColor3f (1.0f, 1.0f, 1.0f);
		draw_string_zoomed_width (10, y_line, input_text_line.data, win->len_x-hud_x-20, 4, chat_zoom);
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
	else if (key == K_MAP && !locked_to_console)
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

		if ((ch == '`' || key == K_CONSOLE) && !locked_to_console)
		{
			hide_window (console_root_win);
			show_window (game_root_win);
			// Undo stupid quickbar hack
			if ( !get_show_window (quickbar_win) )
				show_window (quickbar_win);
#ifdef NEW_CLIENT
			if ( !get_show_window (quickspell_win) )
				show_window (quickspell_win);
#endif
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
	widget_resize (console_root_win, console_out_id, width - hud_x - 20, height - hud_y - CONSOLE_INPUT_HEIGHT - CONSOLE_SEP_HEIGHT - 10);
	widget_resize (console_root_win, console_in_id, width - hud_x - 20, CONSOLE_INPUT_HEIGHT);
	widget_move (console_root_win, console_in_id, 10, height - hud_y - CONSOLE_INPUT_HEIGHT);
	
	nr_console_lines = (int) (height - CONSOLE_INPUT_HEIGHT - CONSOLE_SEP_HEIGHT - hud_y - 10) / (18 * chat_zoom);
	console_text_width = (int) (width - hud_x - 20);
	rewrap_messages(console_text_width);
	
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

int show_console_handler (window_info *win) {
	rewrap_messages(console_text_width);
	hide_window(book_win);
	hide_window(paper_win);
	hide_window(color_race_win);
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
		set_window_handler (console_root_win, ELW_HANDLER_SHOW, &show_console_handler);

		console_out_id = text_field_add_extended (console_root_win, console_out_id, NULL, 10, 10, width - hud_x - 20, height - CONSOLE_INPUT_HEIGHT - CONSOLE_SEP_HEIGHT - hud_y - 10, 0, chat_zoom, -1.0f, -1.0f, -1.0f, display_text_buffer, DISPLAY_TEXT_BUFFER_SIZE, FILTER_ALL, 0, 0, -1.0f, -1.0f, -1.0f);
	/*	Removed. Wytter.
	 *	// initialize the input field without the default keypress
		// handler, since that's not really applicable here
		console_in_id = text_field_add_extended (console_root_win, console_in_id, NULL, 10, height - CONSOLE_INPUT_HEIGHT - hud_y, width - hud_x - 20, CONSOLE_INPUT_HEIGHT, TEXT_FIELD_EDITABLE|TEXT_FIELD_NO_KEYPRESS, chat_zoom, -1.0f, -1.0f, -1.0f, &input_text_line, 1, FILTER_ALL, 0, 0, 1.0f, 1.0f, 1.0f);*/
		
		nr_console_lines = (int) (height - CONSOLE_INPUT_HEIGHT -  CONSOLE_SEP_HEIGHT - hud_y - 10) / (18 * chat_zoom);
		console_text_width = (int) (width - hud_x - 20);
		rewrap_messages(console_text_width);
	}
}
