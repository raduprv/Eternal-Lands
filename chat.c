#include <string.h>
#include "global.h"

#ifndef OLD_EVENT_HANDLER

/*! \name chat text constants
 * @{ */
#define CHAT_WIN_SPACE		4
#define CHAT_WIN_TEXT_WIDTH  	500	/*!< width of the text */
#define CHAT_OUT_TEXT_HEIGHT 	(18*10)	/*!< height of the text: 10 lines in normal font size */
#define CHAT_IN_TEXT_HEIGHT 	(18*3)	/*!< height of the text: 3 lines in normal font size */
#define CHAT_WIN_SCROLL_WIDTH	20	/*!< width of the scrollbar for the chat window */
/*! @} */

int use_windowed_chat = 0;

int chat_win = -1;
int chat_scroll_id = 15;
int chat_out_id = 18;
int chat_in_id = 19;

int chat_win_x = 0; // upper left corner by default
int chat_win_y = 0;
int chat_win_len_x = CHAT_WIN_TEXT_WIDTH + 4 * CHAT_WIN_SPACE + CHAT_WIN_SCROLL_WIDTH;
int chat_win_len_y = CHAT_OUT_TEXT_HEIGHT + CHAT_IN_TEXT_HEIGHT + 7 * CHAT_WIN_SPACE;

int chat_win_text_width = CHAT_WIN_TEXT_WIDTH;
int chat_out_text_height = CHAT_OUT_TEXT_HEIGHT;

int current_line = 0;
int text_changed = 1;
int nr_displayed_lines;

void update_chat_scrollbar ()
{
	if (chat_win >= 0)
	{
		int len = nr_text_buffer_lines >= nr_displayed_lines ? nr_text_buffer_lines-nr_displayed_lines : 0;
		vscrollbar_set_bar_len (chat_win, chat_scroll_id, len);
		vscrollbar_set_pos (chat_win, chat_scroll_id, len);
		current_line = len;
		text_changed = 1;
	}
}

int display_chat_handler (window_info *win)
{
	static int line_start = 0;
	if (text_changed)
	{
		int line = vscrollbar_get_pos (chat_win, chat_scroll_id);
		int ichar;
		unsigned char ch;
		line_start = find_line_nr (line);

		// see if this line starts with a color code
		ch = display_text_buffer[line_start];
		if (ch < 127 || ch > 127 + c_grey4)
		{
			// nope, search backwards for the last color code
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
					text_field_set_text_color (chat_win, chat_out_id, r, g, b);
					break;
				}
			}
		}

		text_field_set_buf_offset (chat_win, chat_out_id, line_start);
		text_changed = 0;
	}

	return 1;
}

int click_chat_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int line = vscrollbar_get_pos (chat_win, chat_scroll_id);
	if (line != current_line)
	{
		text_changed = 1;
		current_line = line;
	}
        return 0;
}

int drag_chat_handler(window_info *win, int mx, int my, Uint32 flags, int dx, int dy)
{
	int line = vscrollbar_get_pos (chat_win, chat_scroll_id);
	if (line != current_line)
	{
		text_changed = 1;
		current_line = line;
	}
        return 0;
}

int resize_chat_handler(window_info *win, int width, int height)
{
	chat_win_text_width = width - 4 * CHAT_WIN_SPACE - CHAT_WIN_SCROLL_WIDTH;
	chat_out_text_height = height - 7 * CHAT_WIN_SPACE - CHAT_IN_TEXT_HEIGHT;
	
	widget_move (chat_win, chat_scroll_id, width - CHAT_WIN_SCROLL_WIDTH, 0);
	widget_resize (chat_win, chat_scroll_id, CHAT_WIN_SCROLL_WIDTH, height - ELW_BOX_SIZE);
	widget_resize (chat_win, chat_out_id, chat_win_text_width + 2 * CHAT_WIN_SPACE, chat_out_text_height + 2 * CHAT_WIN_SPACE);
	widget_resize (chat_win, chat_in_id, chat_win_text_width + 2 * CHAT_WIN_SPACE, CHAT_IN_TEXT_HEIGHT + 2 * CHAT_WIN_SPACE);
	widget_move (chat_win, chat_in_id, CHAT_WIN_SPACE, chat_out_text_height + 4 * CHAT_WIN_SPACE);
	
	reset_soft_breaks (display_text_buffer, chat_zoom, chat_win_text_width);
	reset_soft_breaks (input_text_line, chat_zoom, chat_win_text_width);
	
	nr_displayed_lines = (int) (chat_out_text_height / (18.0 * chat_zoom));
	update_chat_scrollbar ();
	return 0;
}

int chat_in_key_handler (widget_list *w, int x, int y, Uint32 key, Uint32 unikey)
{
	Uint16 keysym = key & 0xffff;
	Uint8 ch = key_to_char (unikey);
	text_field *tf;
	int alt_on = key & ELW_ALT, ctrl_on = key & ELW_CTRL;
	
	if (w == NULL) return 0;
	if ( (w->Flags & TEXT_FIELD_EDITABLE) == 0) return 0;
	
	tf = (text_field *) w->widget_info;
	
	if (keysym == K_ROTATELEFT)
	{
		if (tf->cursor > 0) tf->cursor--;
	}
	else if (keysym == K_ROTATERIGHT)
	{
		if (tf->cursor < tf->buf_fill) tf->cursor++;
	}
	else if (keysym == SDLK_ESCAPE)
	{
		tf->buffer[0] = '\0';
		tf->cursor = 0;
		tf->buf_fill = 0;
		input_text_lenght = 0;
		input_text_lines = 1;
	}
	else if (ch == SDLK_RETURN && tf->buf_fill > 0)
	{
		if (tf->buffer[0] == '%' && tf->buf_fill > 1) 
		{
			if ( (check_var (&(tf->buffer[1]), IN_GAME_VAR) ) < 0)
			{
				send_input_text_line (input_text_line, input_text_lenght);
			}
		}
		else if ( tf->buffer[0] == '#' || get_show_window (console_root_win) )
		{
			test_for_console_command (tf->buffer, tf->buf_fill);
		}
		else
		{
			send_input_text_line (tf->buffer, tf->buf_fill);
		}
		tf->buffer[0] = '\0';
		tf->cursor = 0;
		tf->buf_fill = 0;
		input_text_lenght = 0;
		input_text_lines = 1;
	}
	else if (ch == SDLK_BACKSPACE && tf->cursor > 0)
	{
		int i;
		for (i = tf->cursor; i <= tf->buf_fill; i++)
			input_text_line[i-1] = input_text_line[i];
		tf->cursor--;
		tf->buf_fill--;
		reset_soft_breaks (tf->buffer, w->size, w->len_x - 2 * CHAT_WIN_SPACE);
	}
	else if (ch == SDLK_DELETE && tf->cursor < tf->buf_fill)
	{
		int i;
		for (i = tf->cursor+1; i <= tf->buf_fill; i++)
			input_text_line[i-1] = input_text_line[i];
		tf->buf_fill--;
		reset_soft_breaks (tf->buffer, w->size, w->len_x - 2 * CHAT_WIN_SPACE);
	}
	else if ( !alt_on && !ctrl_on && ( (ch >= 32 && ch <= 126) || (ch > 127 + c_grey4) ) && ch != '`' )
	{
		// watch for the '//' shortcut
		if (tf->buf_fill == 1 && ch == '/' && tf->buffer[0] == '/' && last_pm_from[0])
		{
			int i;
			int l = strlen (last_pm_from);
			for (i = 0; i < l; i++) 
				tf->buffer[i+1] = last_pm_from[i];
			tf->buffer[l+1] = ' ';
			tf->buffer[l+2] = '\0';
			tf->buf_fill = l+2;
			tf->cursor = l+2;
		}
		else if (tf->buf_fill < tf->buf_size - 1)
		{
			int i;
			for (i = tf->buf_fill+1; i > tf->cursor; i--)
				tf->buffer[i] = tf->buffer[i-1];
			tf->buffer[tf->cursor] = ch;
			tf->cursor++;
			tf->buf_fill++;
			reset_soft_breaks (tf->buffer, w->size, w->len_x - 2 * CHAT_WIN_SPACE);
		}
	}
	else
	{
		return 0;
	}
	
	return 1;
}


int keypress_chat_handler (window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
	return 0;
}

int handle_root_key (Uint32 key, Uint32 unikey)
{
	widget_list *w = widget_find (chat_win, chat_in_id);
	return chat_in_key_handler (w, 0, 0, key, unikey);
}

void display_chat ()
{
	if (chat_win < 0)
	{
		int scroll_len;
		
		nr_displayed_lines = (int) ((CHAT_OUT_TEXT_HEIGHT-1) / (18.0 * chat_zoom));
		scroll_len = nr_text_buffer_lines >= nr_displayed_lines ? nr_text_buffer_lines-nr_displayed_lines : 0;
		
		chat_win = create_window ("chat", game_root_win, 0, chat_win_x, chat_win_y, chat_win_len_x, chat_win_len_y, (ELW_WIN_DEFAULT|ELW_RESIZEABLE) & ~ELW_CLOSE_BOX);
		
		set_window_handler (chat_win, ELW_HANDLER_DISPLAY, &display_chat_handler);
		set_window_handler (chat_win, ELW_HANDLER_DRAG, &drag_chat_handler);
		set_window_handler (chat_win, ELW_HANDLER_CLICK, &click_chat_handler);
		set_window_handler (chat_win, ELW_HANDLER_RESIZE, &resize_chat_handler);
		set_window_handler (chat_win, ELW_HANDLER_KEYPRESS, &keypress_chat_handler);

		chat_scroll_id = vscrollbar_add_extended (chat_win, chat_scroll_id, NULL, CHAT_WIN_TEXT_WIDTH+2*CHAT_WIN_SPACE+8, 0, CHAT_WIN_SCROLL_WIDTH, chat_win_len_y-ELW_BOX_SIZE, 0, 1.0f, 0.77f, 0.57f, 0.39f, 0, 1, scroll_len);
		
		chat_out_id = text_field_add_extended (chat_win, chat_out_id, NULL, CHAT_WIN_SPACE, CHAT_WIN_SPACE, CHAT_WIN_TEXT_WIDTH + 2 * CHAT_WIN_SPACE, CHAT_OUT_TEXT_HEIGHT + 2 * CHAT_WIN_SPACE, TEXT_FIELD_BORDER, chat_zoom, 0.77f, 0.57f, 0.39f, display_text_buffer, MAX_DISPLAY_TEXT_BUFFER_LENGTH, CHAT_WIN_SPACE, CHAT_WIN_SPACE, -1.0, -1.0, -1.0);
		
		chat_in_id = text_field_add_extended (chat_win, chat_in_id, NULL, CHAT_WIN_SPACE, CHAT_OUT_TEXT_HEIGHT + 4 * CHAT_WIN_SPACE, CHAT_WIN_TEXT_WIDTH + 2 * CHAT_WIN_SPACE, CHAT_IN_TEXT_HEIGHT + 2 * CHAT_WIN_SPACE, TEXT_FIELD_BORDER|TEXT_FIELD_EDITABLE, chat_zoom, 0.77f, 0.57f, 0.39f, input_text_line, sizeof (input_text_line), CHAT_WIN_SPACE, CHAT_WIN_SPACE, 1.0, 1.0, 1.0);
		
		widget_set_OnKey (chat_win, chat_in_id, chat_in_key_handler);
	}
	else
	{
		show_window (chat_win);
		select_window (chat_win);
	}
}

#endif // not def OLD_EVENT_HANDLER
