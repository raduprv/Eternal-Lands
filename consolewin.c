#include <stdlib.h>
#include <string.h>
#include "global.h"

int console_root_win = -1;

int console_out_id = 40;
int console_in_id = 41;

int nr_console_lines;
int scroll_up_lines = 0;
int console_text_changed = 0;
int console_text_width = -1;

int locked_to_console = 0;

void update_console_win (text_message * msg)
{
	int nlines = rewrap_message(msg, chat_zoom, console_text_width, NULL);
	if (msg->deleted) {
		if (scroll_up_lines > nlines) {
			scroll_up_lines -= nlines;
		} else {
			scroll_up_lines = 0;
			console_text_changed = 1;
		}
	} else {
		if (scroll_up_lines == 0) {
			console_text_changed = 1;
		} else {
			scroll_up_lines += nlines;
		}
	}
}

int display_console_handler (window_info *win)
{
	static int msg = 0, offset = 0;
	const char *sep_string = "^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^ ^^";
	
	// are we actively drawing things?
	if (SDL_GetAppState () & SDL_APPACTIVE)
	{
		set_font(chat_font);	// switch to the chat font
		if (console_text_changed)
		{
			find_line_nr (total_nr_lines, total_nr_lines - nr_console_lines - scroll_up_lines, FILTER_ALL, &msg, &offset, chat_zoom, console_text_width);		
			text_field_set_buf_pos (console_root_win, console_out_id, msg, offset);
			console_text_changed = 0;
		}

		draw_console_pic (cons_text);
		if (scroll_up_lines != 0)
		{
			glColor3f (1.0, 1.0, 1.0);
			draw_string_zoomed_clipped (10, 10 + win->len_y - input_widget->len_y - CONSOLE_SEP_HEIGHT - hud_y, sep_string, -1, win->len_x - hud_x - 20, CONSOLE_SEP_HEIGHT, chat_zoom);
		}
		//ttlanhil: disabled, until the scrolling in console is adusted to work with filtering properly
		//if the users prefer that console not be filtered, the following line can be removed.
		//if they want it filtered, then more work can be done until it works properly
		//((text_field*)((widget_find(console_root_win, console_out_id))->widget_info))->chan_nr = current_filter;

		draw_hud_interface ();
		set_font (0);	// switch to fixed
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
#ifdef COMMAND_BUFFER
	else if(keysym == SDLK_UP)
 	{
		if(input_text_line.len > 0 && (*input_text_line.data == '#' || *input_text_line.data == *char_cmd_str))
		{
			char *line = history_get_line_up();
			if(line != NULL) {
				input_text_line.len = snprintf(input_text_line.data, input_text_line.size, "%s", line);
				if(input_widget && input_widget->widget_info) {
					text_field *tf = input_widget->widget_info;
					tf->cursor = tf->buffer->len;
				}
			}
		}
		else if (total_nr_lines > nr_console_lines + scroll_up_lines)
		{
			scroll_up_lines++;
			console_text_changed = 1;
		}
 	}
	else if (keysym == SDLK_DOWN)
 	{
		if(input_text_line.len > 0 && (*input_text_line.data == '#' || *input_text_line.data == *char_cmd_str))
		{
			char *line = history_get_line_down();
			if(line != NULL) {
				input_text_line.len = snprintf(input_text_line.data, input_text_line.size, "%s", line);
				if(input_widget && input_widget->widget_info) {
					text_field *tf = input_widget->widget_info;
					tf->cursor = tf->buffer->len;
				}
			}
		}
		else if(scroll_up_lines > 0)
		{
			scroll_up_lines--;
			console_text_changed = 1;
		}
	}
	else if(key == K_TABCOMPLETE && input_text_line.len > 0 && (*input_text_line.data == '#' || *input_text_line.data == *char_cmd_str || *input_text_line.data == '/' || *input_text_line.data == *char_slash_str))
	{
		do_tab_complete(&input_text_line);
	}
#else
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
#endif //COMMAND_BUFFER
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
		if (scroll_up_lines < 0)
			scroll_up_lines = 0;
		console_text_changed = 1;
	}
	else if (key == K_MAP)
	{
		if (!locked_to_console && switch_to_game_map())
		{
			hide_window (console_root_win);
			show_window (map_root_win);
		}
	}
	else
	{
		Uint8 ch = key_to_char (unikey);

#ifdef COMMAND_BUFFER
		reset_tab_completer();
#endif //COMMAND_BUFFER
		if ((ch == '`' || key == K_CONSOLE) && !locked_to_console)
		{
			hide_window (console_root_win);
			show_window (game_root_win);
			// Undo stupid quickbar hack
			if ( !get_show_window (quickbar_win) )
				show_window (quickbar_win);
			if ( !get_show_window (quickspell_win) )
				show_window (quickspell_win);
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
	widget_resize (console_root_win, console_out_id, width - hud_x - 20, height - hud_y - input_widget->len_y - CONSOLE_SEP_HEIGHT - 10);
	widget_resize (console_root_win, input_widget->id, width - hud_x, INPUT_HEIGHT);
	widget_move (console_root_win, input_widget->id, 0, height-INPUT_HEIGHT-hud_y);

	nr_console_lines = (int) (height - input_widget->len_y - CONSOLE_SEP_HEIGHT - hud_y - 10) / (18 * chat_zoom);
	console_text_width = (int) (width - hud_x - 20);
	
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
	int i;

	for (i=0; i < MAX_CHAT_TABS; i++) {
		if (channels[i].open) {
			tab_set_label_color_by_id (chat_win, chat_tabcollection_id, channels[i].tab_id, -1.0f, -1.0f, -1.0f);
		}
	}
			
	hide_window(book_win);
	hide_window(paper_win);
	hide_window(color_race_win);

	if (use_windowed_chat == 1) {
		display_tab_bar ();
	}
	widget_move_win(input_widget->window_id, input_widget->id, console_root_win);
	widget_resize (input_widget->window_id, input_widget->id, window_width-hud_x, INPUT_HEIGHT);
	widget_move (input_widget->window_id, input_widget->id, 0, window_height-INPUT_HEIGHT-hud_y);
	widget_set_flags(input_widget->window_id, input_widget->id, (TEXT_FIELD_BORDER|INPUT_DEFAULT_FLAGS)^WIDGET_CLICK_TRANSPARENT);
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

		console_out_id = text_field_add_extended (console_root_win, console_out_id, NULL, 10, 25, width - hud_x - 20, height - INPUT_HEIGHT - CONSOLE_SEP_HEIGHT - hud_y - 10, 0, chat_zoom, -1.0f, -1.0f, -1.0f, display_text_buffer, DISPLAY_TEXT_BUFFER_SIZE, CHAT_ALL, 0, 0, -1.0f, -1.0f, -1.0f);
		if(input_widget == NULL) {
			Uint32 id;
			id = text_field_add_extended(console_root_win, 0, NULL, 0, height-INPUT_HEIGHT-hud_y, width-hud_x, INPUT_HEIGHT, (INPUT_DEFAULT_FLAGS|TEXT_FIELD_BORDER)^WIDGET_CLICK_TRANSPARENT, chat_zoom, 0.77f, 0.57f, 0.39f, &input_text_line, 1, FILTER_ALL, 4, 4, 1.0, 1.0, 1.0);
			input_widget = widget_find(console_root_win, id);
		} else {
			widget_move_win(input_widget->window_id, input_widget->id, console_root_win);
			widget_resize (input_widget->window_id, input_widget->id, window_width-hud_x, INPUT_HEIGHT);
			widget_move (input_widget->window_id, input_widget->id, 0, window_height-INPUT_HEIGHT-hud_y);
		}
		widget_set_OnKey(input_widget->window_id, input_widget->id, chat_input_key);
		widget_set_flags(input_widget->window_id, input_widget->id, (TEXT_FIELD_BORDER|INPUT_DEFAULT_FLAGS)^WIDGET_CLICK_TRANSPARENT);

		nr_console_lines = (int) (height - input_widget->len_y -  CONSOLE_SEP_HEIGHT - hud_y - 10) / (18 * chat_zoom);
		console_text_width = (int) (width - hud_x - 20);
	}
}
