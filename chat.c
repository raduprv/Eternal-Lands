#include <string.h>
#include "global.h"

#ifndef OLD_EVENT_HANDLER

/*! \name chat text constants
 * @{ */
#define CHAT_WIN_MAX_TABS	5	/*< the maximum number of channels */
#define CHAT_WIN_SPACE		4	/*< spacing between widget and window borders */
#define CHAT_WIN_TAG_HEIGHT	20	/*!< height of a tab */
#define CHAT_WIN_TAG_SPACE	3	/*!< spacing between tabs */
#define CHAT_WIN_TEXT_WIDTH  	500	/*!< width of the text */
#define CHAT_OUT_TEXT_HEIGHT 	(18*8)	/*!< height of the output text: 8 lines in normal font size */
#define CHAT_IN_TEXT_HEIGHT 	(18*3)	/*!< height of the input text: 3 lines in normal font size */
#define CHAT_WIN_SCROLL_WIDTH	20	/*!< width of the scrollbar for the chat window */
/*! @} */

int use_windowed_chat = 0;

int chat_win = -1;
int chat_scroll_id = 15;
int chat_in_id = 19;
int chat_tabcollection_id = 20;
int chat_out_start_id = 21;
int chat_tab_ids[CHAT_WIN_MAX_TABS];
int chat_out_ids[CHAT_WIN_MAX_TABS];

int chat_win_x = 0; // upper left corner by default
int chat_win_y = 0;

int chat_win_text_width = CHAT_WIN_TEXT_WIDTH;
int chat_out_text_height = CHAT_OUT_TEXT_HEIGHT;

int current_line = 0;
int text_changed = 1;
int nr_displayed_lines;

void sync_chat_and_console ()
{
	widget_list *cons = widget_find (console_root_win, console_in_id);
	widget_list *chat = widget_find (chat_win, chat_in_id);
	text_field *cons_tf = (text_field *) cons->widget_info;
	text_field *chat_tf = (text_field *) chat->widget_info;

	cons_tf->buf_fill = chat_tf->buf_fill;
	cons_tf->cursor = chat_tf->cursor;
}

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
					text_field_set_text_color (chat_tab_ids[0], chat_out_ids[0], r, g, b);
					break;
				}
			}
		}

		text_field_set_buf_offset (chat_tab_ids[0], chat_out_ids[0], line_start);
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
	int itab;
	int scroll_x = width - CHAT_WIN_SCROLL_WIDTH;
	int scroll_height = height - ELW_BOX_SIZE;
	int inout_width = width - CHAT_WIN_SCROLL_WIDTH - 2 * CHAT_WIN_SPACE;
	int input_height = CHAT_IN_TEXT_HEIGHT + 2 * CHAT_WIN_SPACE;
	int input_y = height - input_height - CHAT_WIN_SPACE;
	int tabcol_height = input_y - 2 * CHAT_WIN_SPACE;
	int output_height = tabcol_height - CHAT_WIN_TAG_HEIGHT;
	int diff = (int) 18*chat_zoom;
	
	if (output_height < 5*18*chat_zoom + 2 * CHAT_WIN_SPACE && input_height > 3*diff)
	{
		input_height -= 2*diff;
		input_y += 2*diff;
		output_height += 2*diff;
		tabcol_height += 2*diff;
	}
	else if (output_height < 8*18*chat_zoom + 2 * CHAT_WIN_SPACE && input_height > 2*diff)
	{
		input_height -= diff;
		input_y += diff;
		output_height += diff;
		tabcol_height += diff;
	}
	
	chat_win_text_width = inout_width - 2 * CHAT_WIN_SPACE;
	chat_out_text_height = output_height - 2 * CHAT_WIN_SPACE;
	
	widget_resize (chat_win, chat_scroll_id, CHAT_WIN_SCROLL_WIDTH, scroll_height);
	widget_move (chat_win, chat_scroll_id, scroll_x, 0);
	
	widget_resize (chat_win, chat_tabcollection_id, inout_width, tabcol_height);
	
	for (itab = 0; itab < CHAT_WIN_MAX_TABS; itab++)
		if (chat_tab_ids[itab] >= 0)
			widget_resize (chat_tab_ids[itab], chat_out_ids[itab], inout_width, output_height);
	
	widget_resize (chat_win, chat_in_id, inout_width, input_height);
	widget_move (chat_win, chat_in_id, CHAT_WIN_SPACE, input_y);
	
	reset_soft_breaks (display_text_buffer, chat_zoom, chat_win_text_width);
	reset_soft_breaks (input_text_line, chat_zoom, chat_win_text_width);
	
	nr_displayed_lines = (int) (chat_out_text_height / (18.0f * chat_zoom));
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
	else if (keysym == SDLK_HOME)
	{
		tf->cursor = 0;
	}
	else if (keysym == SDLK_END)
	{
		tf->cursor = tf->buf_fill;
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
				send_input_text_line (input_text_line, tf->buf_fill);
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
	
	sync_chat_and_console ();
	return 1;
}


int keypress_chat_handler (window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
	return 0;
}

int root_key_to_input_field (Uint32 key, Uint32 unikey)
{
	widget_list *w = widget_find (chat_win, chat_in_id);
	if (w == NULL) return 0;
	return chat_in_key_handler (w, 0, 0, key, unikey);
}

void paste_in_input_field (const Uint8 *text)
{
	widget_list *w = widget_find (chat_win, chat_in_id);
	text_field *tf;
	int nr_paste, nr_free, ib, jb, nb;
	Uint8 ch;
	
	if (w == NULL) return;
	tf = (text_field *) w->widget_info;
	
	// find out how many characters to paste
	nr_free = tf->buf_size - tf->cursor - 1;
	nr_paste = 0;
	for (ib = 0; text[ib] && nr_paste < nr_free; ib++)
	{
		ch = text[ib];
		if ( (ch >= 32 && ch <= 126) || ch > 127 + c_grey4)
			nr_paste++;
	}
	
	if (nr_paste == 0) return;

	// now move the characters right of the cursor (if any)
	nb = tf->buf_fill - tf->cursor;
	if (nb > nr_free - nr_paste) nb = nr_free - nr_paste;
	if (nb > 0)
	{
		for (ib = nb-1; ib >= 0; ib--)
			tf->buffer[tf->cursor+nr_paste+ib] = tf->buffer[tf->cursor+ib];
	}
	tf->buffer[tf->cursor+nr_paste+nb] = '\0';
	
	// insert the pasted text
	jb = 0;
	for (ib = 0; text[ib]; ib++)
	{
		ch = text[ib];
		if ( (ch >= 32 && ch <= 126) || ch > 127 + c_grey4)
		{
			tf->buffer[tf->cursor+jb] = text[ib];
			jb++;
		}
	}
	
	// update the widget information
	tf->cursor += nr_paste;
	tf->buf_fill = tf->cursor + nb;
	
	reset_soft_breaks (tf->buffer, w->size, w->len_x - 2 * CHAT_WIN_SPACE);
	
	input_text_lenght = tf->buf_fill;
}

void display_chat ()
{
	if (chat_win < 0)
	{
		int scroll_len, itab;
		
		int chat_win_width = CHAT_WIN_TEXT_WIDTH + 4 * CHAT_WIN_SPACE + CHAT_WIN_SCROLL_WIDTH;
		int chat_win_height = CHAT_OUT_TEXT_HEIGHT + CHAT_IN_TEXT_HEIGHT + 7 * CHAT_WIN_SPACE + CHAT_WIN_TAG_HEIGHT;
		int inout_width = CHAT_WIN_TEXT_WIDTH + 2 * CHAT_WIN_SPACE;
		int output_height = CHAT_OUT_TEXT_HEIGHT + 2 * CHAT_WIN_SPACE;
		int tabcol_height = output_height + CHAT_WIN_TAG_HEIGHT;
		int input_y = tabcol_height + 2 * CHAT_WIN_SPACE;
		int input_height = CHAT_IN_TEXT_HEIGHT + 2 * CHAT_WIN_SPACE;
		
		int min_width = CHAT_WIN_SCROLL_WIDTH + 2 * CHAT_WIN_SPACE + (int)(CHAT_WIN_TEXT_WIDTH * chat_zoom);
		int min_height = 7 * CHAT_WIN_SPACE + CHAT_WIN_TAG_HEIGHT + (int) ((2+5) * 18.0 * chat_zoom);
		
		nr_displayed_lines = (int) ((CHAT_OUT_TEXT_HEIGHT-1) / (18.0 * chat_zoom));
		scroll_len = nr_text_buffer_lines >= nr_displayed_lines ? nr_text_buffer_lines-nr_displayed_lines : 0;
		
		chat_win = create_window ("chat", game_root_win, 0, chat_win_x, chat_win_y, chat_win_width, chat_win_height, (ELW_WIN_DEFAULT|ELW_RESIZEABLE) & ~ELW_CLOSE_BOX);
		
		set_window_handler (chat_win, ELW_HANDLER_DISPLAY, &display_chat_handler);
		set_window_handler (chat_win, ELW_HANDLER_DRAG, &drag_chat_handler);
		set_window_handler (chat_win, ELW_HANDLER_CLICK, &click_chat_handler);
		set_window_handler (chat_win, ELW_HANDLER_RESIZE, &resize_chat_handler);
		set_window_handler (chat_win, ELW_HANDLER_KEYPRESS, &keypress_chat_handler);

		chat_scroll_id = vscrollbar_add_extended (chat_win, chat_scroll_id, NULL, chat_win_width - CHAT_WIN_SCROLL_WIDTH, 0, CHAT_WIN_SCROLL_WIDTH, chat_win_height - ELW_BOX_SIZE, 0, 1.0f, 0.77f, 0.57f, 0.39f, 0, 1, scroll_len);
		
		chat_tabcollection_id = tab_collection_add_extended (chat_win, chat_tabcollection_id, NULL, CHAT_WIN_SPACE, CHAT_WIN_SPACE, inout_width, tabcol_height, 0, 0.7, 0.77f, 0.57f, 0.39f, CHAT_WIN_MAX_TABS, CHAT_WIN_TAG_HEIGHT, CHAT_WIN_TAG_SPACE);
		
		for (itab = 0; itab < CHAT_WIN_MAX_TABS; itab++)
		{
			chat_tab_ids[itab] = -1;
			chat_out_ids[itab] = chat_out_start_id + itab;
		}

		chat_tab_ids[0] = tab_add (chat_win, chat_tabcollection_id, "Local", 0);
		set_window_min_size (chat_tab_ids[0], 0, 0);
		chat_out_ids[0] = text_field_add_extended (chat_tab_ids[0], chat_out_ids[0], NULL, 0, 0, inout_width, output_height, 0, chat_zoom, 0.77f, 0.57f, 0.39f, display_text_buffer, MAX_DISPLAY_TEXT_BUFFER_LENGTH, CHAT_WIN_SPACE, CHAT_WIN_SPACE, -1.0, -1.0, -1.0);
		
		chat_in_id = text_field_add_extended (chat_win, chat_in_id, NULL, CHAT_WIN_SPACE, input_y, inout_width, input_height, TEXT_FIELD_BORDER|TEXT_FIELD_EDITABLE, chat_zoom, 0.77f, 0.57f, 0.39f, input_text_line, sizeof (input_text_line), CHAT_WIN_SPACE, CHAT_WIN_SPACE, 1.0, 1.0, 1.0);
		
		set_window_min_size (chat_win, min_width, min_height);
		
		widget_set_OnKey (chat_win, chat_in_id, chat_in_key_handler);
	}
	else
	{
		show_window (chat_win);
		select_window (chat_win);
	}
}

#endif // not def OLD_EVENT_HANDLER
