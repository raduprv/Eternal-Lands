#include <string.h>
#include "global.h"

#define CHAT_WIN_MAX_TABS	5
#define CHAT_WIN_SPACE		4
#define CHAT_WIN_TAG_HEIGHT	20
#define CHAT_WIN_TAG_SPACE	3
#define CHAT_WIN_TEXT_WIDTH  	500
#define CHAT_OUT_TEXT_HEIGHT 	(18*8)
#define CHAT_IN_TEXT_HEIGHT 	(18*3)
#define CHAT_WIN_SCROLL_WIDTH	20

typedef struct
{
	int tab_id;
	int out_id;
	int chan_nr;
	int nr_lines;
	char open, new;
} chat_channel;

int use_windowed_chat = 0;

int chat_win = -1;
int chat_scroll_id = 15;
int chat_in_id = 19;
int chat_tabcollection_id = 20;
int chat_out_start_id = 21;

int chat_win_x = 0; // upper left corner by default
int chat_win_y = 0;

int chat_win_text_width = CHAT_WIN_TEXT_WIDTH;
int chat_out_text_height = CHAT_OUT_TEXT_HEIGHT;

int current_line = 0;
int text_changed = 1;
int nr_displayed_lines;

chat_channel channels[CHAT_WIN_MAX_TABS];
int active_tab = -1;

void init_chat_channels ()
{
	int itab;
	
	for (itab = 0; itab < CHAT_WIN_MAX_TABS; itab++)
	{
		channels[itab].tab_id = -1;
		channels[itab].out_id = chat_out_start_id + itab;
		channels[itab].chan_nr = CHANNEL_LOCAL;
		channels[itab].nr_lines = 0;
		channels[itab].open = 0;
		channels[itab].new = 0;
	}
}

void sync_chat_and_console ()
{
	widget_list *cons = widget_find (console_root_win, console_in_id);
	widget_list *chat = widget_find (chat_win, chat_in_id);
	text_field *cons_tf = (text_field *) cons->widget_info;
	text_field *chat_tf = (text_field *) chat->widget_info;

	cons_tf->msg = chat_tf->msg;
	cons_tf->offset = chat_tf->offset;
	cons_tf->buf_fill = chat_tf->buf_fill;
	cons_tf->cursor = chat_tf->cursor;
}

void clear_input_line ()
{
	widget_list *cons = widget_find (console_root_win, console_in_id);
	text_field *cons_tf = (text_field *) cons->widget_info;
	
	input_text_line.data[0] = '\0';
	input_text_line.len = 0;
	cons_tf->cursor = 0;
	if (use_windowed_chat)
	{
		widget_list *chat = widget_find (chat_win, chat_in_id);
		text_field *chat_tf = (text_field *) chat->widget_info;
		chat_tf->cursor = 0;
	}
}

int close_channel (window_info *win)
{
	int id = win->window_id;
	int ichan;
	
	for (ichan = 0; ichan < CHAT_WIN_MAX_TABS; ichan++)
	{
		if (channels[ichan].tab_id == id)
		{
			channels[ichan].tab_id = -1;
			channels[ichan].chan_nr = CHANNEL_LOCAL;
			channels[ichan].nr_lines = 0;
			channels[ichan].open = 0;
			channels[ichan].new = 0;
			return 1;
		}
	}
	
	// we shouldn't get here
	LOG_ERROR ("Trying to close non-existant channel\n");
	return 0;
}

void update_chat_window (int nlines, int channel)
{
	int ichan, len;
	
	// don't bother if there's no chat window
	if (chat_win < 0) return;
	
	// first check if we need to display in all open channels
	if (channel == CHANNEL_ALL)
	{
		for (ichan = 0; ichan < CHAT_WIN_MAX_TABS; ichan++)
		{
			if (channels[ichan].open)
				channels[ichan].nr_lines += nlines;
			channels[ichan].new = 1;
			// don't set label color in active tab, since the user
			// will already see the message appear
			if (ichan != active_tab)
				tab_set_label_color_by_id (chat_win, chat_tabcollection_id, channels[ichan].tab_id, 1.0, 1.0, 0.0);
		}
		
		len = channels[active_tab].nr_lines - nr_displayed_lines;
		if (len < 0) len = 0;
		vscrollbar_set_bar_len (chat_win, chat_scroll_id, len);
		vscrollbar_set_pos (chat_win, chat_scroll_id, len);

		current_line = channels[ichan].nr_lines;
		text_changed = 1;
		return;
	}
	
	// not for all channels, see if this channel is already open
	for (ichan = 0; ichan < CHAT_WIN_MAX_TABS; ichan++)
	{
		if (channels[ichan].open && channels[ichan].chan_nr == channel)
		{
			channels[ichan].nr_lines += nlines;
			channels[ichan].new = 1;

			if (ichan == active_tab)
			{
				len = channels[ichan].nr_lines - nr_displayed_lines;
				if (len < 0) len = 0;
					
				vscrollbar_set_bar_len (chat_win, chat_scroll_id, len);
				vscrollbar_set_pos (chat_win, chat_scroll_id, len);
				current_line = channels[ichan].nr_lines;
				text_changed = 1;
			}
			else
			{
				tab_set_label_color_by_id (chat_win, chat_tabcollection_id, channels[ichan].tab_id, 1.0, 1.0, 0.0);
			}
			return;
		}
	}
	
	// channel not found, try to create a new one
	for (ichan = 0; ichan < CHAT_WIN_MAX_TABS; ichan++)
	{
		if (!channels[ichan].open)
		{
			// yay, found an empty slot
			char title[64];
			int inout_width = chat_win_text_width + 2 * CHAT_WIN_SPACE;
			int output_height = chat_out_text_height + 2 * CHAT_WIN_SPACE;

			channels[ichan].chan_nr = channel;
			channels[ichan].nr_lines = nlines;
			channels[ichan].open = 1;
			channels[ichan].new = 1;
			
			switch (channel)
			{
				case CHANNEL_LOCAL:
					my_strncp (title, tab_local, sizeof (title) );
					break;
				case CHANNEL_GM:
					my_strncp (title, tab_guild, sizeof (title) );
					break;
				default:
					my_strncp (title, tab_channel, sizeof (title) );
			}

			channels[ichan].tab_id = tab_add (chat_win, chat_tabcollection_id, title, 0, 1);
				
			set_window_min_size (channels[ichan].tab_id, 0, 0);
			channels[ichan].out_id = text_field_add_extended (channels[ichan].tab_id, channels[ichan].out_id, NULL, 0, 0, inout_width, output_height, 0, chat_zoom, 0.77f, 0.57f, 0.39f, display_text_buffer, DISPLAY_TEXT_BUFFER_SIZE, channel, CHAT_WIN_SPACE, CHAT_WIN_SPACE, -1.0, -1.0, -1.0);

			set_window_handler (channels[ichan].tab_id, ELW_HANDLER_DESTROY, close_channel);
				
			tab_set_label_color_by_id (chat_win, chat_tabcollection_id, channels[ichan].tab_id, 1.0, 1.0, 0.0);
			
			return;
		}
	}
	
	// uh oh, no empty slot found. this shouldn't really be happening...
	// log as local
	channels[0].nr_lines += nlines;	
	channels[0].new = 1;
	if (0 == active_tab)
	{
		len = channels[0].nr_lines - nr_displayed_lines;
		if (len < 0) len = 0;
	
		vscrollbar_set_bar_len (chat_win, chat_scroll_id, len);
		vscrollbar_set_pos (chat_win, chat_scroll_id, len);
		current_line = channels[ichan].nr_lines;
		text_changed = 1;
	}
	else
	{
		tab_set_label_color_by_id (chat_win, chat_tabcollection_id, channels[0].tab_id, 1.0, 1.0, 0.0);
	}
}

int display_chat_handler (window_info *win)
{
	static int msg_start = 0, offset_start = 0;
	if (text_changed)
	{
		int line = vscrollbar_get_pos (chat_win, chat_scroll_id);

		find_line_nr (channels[active_tab].nr_lines, line, channels[active_tab].chan_nr, &msg_start, &offset_start);
		text_field_set_buf_pos (channels[active_tab].tab_id, channels[active_tab].out_id, msg_start, offset_start);
		text_changed = 0;
	}

	return 1;
}

int chat_tabs_click (widget_list *widget, int mx, int my, Uint32 flags)
{
	int id;
	
	id = tab_collection_get_tab_id (chat_win, widget->id);
	
	// reset the tab's label color to default
	tab_set_label_color_by_id (chat_win, widget->id, id, -1.0, -1.0, -1.0);
	
	if (id != channels[active_tab].tab_id)
	{
		for (active_tab = 0; active_tab < CHAT_WIN_MAX_TABS; active_tab++)
		{
			if (channels[active_tab].tab_id == id && channels[active_tab].open)
				break;
		}
		if (active_tab >= CHAT_WIN_MAX_TABS)
		{
			// This shouldn't be happening
			LOG_ERROR ("Trying to switch to non-existant channel");
			active_tab = 0;
		}
		current_line = channels[active_tab].nr_lines - nr_displayed_lines;
		if (current_line < 0) current_line = 0;
		vscrollbar_set_bar_len (chat_win, chat_scroll_id, current_line);
		vscrollbar_set_pos (chat_win, chat_scroll_id, current_line);
		text_changed = 1;
	}
	
        return 1;
}

int chat_scroll_drag (widget_list *widget, int mx, int my, Uint32 flags, int dx, int dy)
{
	int line = vscrollbar_get_pos (chat_win, widget->id);
	if (line != current_line)
	{
		text_changed = 1;
		current_line = line;
	}
        return 0;
}

int chat_scroll_click (widget_list *widget, int mx, int my, Uint32 flags)
{
	int line = vscrollbar_get_pos (chat_win, widget->id);
	if (line != current_line)
	{
		text_changed = 1;
		current_line = line;
	}
        return 0;
}

int chat_input_key (widget_list *widget, int mx, int my, Uint32 key, Uint32 unikey)
{
	Uint16 keysym = key & 0xffff;
	text_field *tf;
	text_message *msg;

	if (widget == NULL) return 0;
	tf = (text_field *) widget->widget_info;
	msg = tf->buffer;

	if (keysym == K_ROTATELEFT)
	{
		if (tf->cursor > 0) tf->cursor--;
	}
	else if (keysym == K_ROTATERIGHT)
	{
		if (tf->cursor < msg->len) tf->cursor++;
	}
	else if (keysym == SDLK_HOME)
	{
		tf->cursor = 0;
	}
	else if (keysym == SDLK_END)
	{
		tf->cursor = msg->len;
	}
	else
	{
		return 0;
	}

	return 1;
}


int resize_chat_handler(window_info *win, int width, int height)
{
	int itab, imsg, nlines, ntot;
	int scroll_x = width - CHAT_WIN_SCROLL_WIDTH;
	int scroll_height = height - ELW_BOX_SIZE;
	int inout_width = width - CHAT_WIN_SCROLL_WIDTH - 2 * CHAT_WIN_SPACE;
	int input_height = CHAT_IN_TEXT_HEIGHT + 2 * CHAT_WIN_SPACE;
	int input_y = height - input_height - CHAT_WIN_SPACE;
	int tabcol_height = input_y - 2 * CHAT_WIN_SPACE;
	int output_height = tabcol_height - CHAT_WIN_TAG_HEIGHT;
	int line_height = (int) 18*chat_zoom;
	
	if (output_height < 5*line_height + 2 * CHAT_WIN_SPACE && input_height > 3*line_height + 2 * CHAT_WIN_SPACE)
	{
		input_height -= 2*line_height;
		input_y += 2*line_height;
		output_height += 2*line_height;
		tabcol_height += 2*line_height;
	}
	else if (output_height < 8*line_height + 2 * CHAT_WIN_SPACE && input_height > 2*line_height + 2 * CHAT_WIN_SPACE)
	{
		input_height -= line_height;
		input_y += line_height;
		output_height += line_height;
		tabcol_height += line_height;
	}
	
	chat_win_text_width = inout_width - 2 * CHAT_WIN_SPACE;
	chat_out_text_height = output_height - 2 * CHAT_WIN_SPACE;
	
	widget_resize (chat_win, chat_scroll_id, CHAT_WIN_SCROLL_WIDTH, scroll_height);
	widget_move (chat_win, chat_scroll_id, scroll_x, 0);
	
	widget_resize (chat_win, chat_tabcollection_id, inout_width, tabcol_height);
	
	for (itab = 0; itab < CHAT_WIN_MAX_TABS; itab++)
		if (channels[itab].tab_id >= 0)
			widget_resize (channels[itab].tab_id, channels[itab].out_id, inout_width, output_height);
	
	widget_resize (chat_win, chat_in_id, inout_width, input_height);
	widget_move (chat_win, chat_in_id, CHAT_WIN_SPACE, input_y);

	// recompute line breaks
	for (itab = 0; itab < CHAT_WIN_MAX_TABS; itab++)
		channels[itab].nr_lines = 0;
	
	ntot = 0;
	imsg = buffer_full ? last_message+1 : 0;
	if (imsg > DISPLAY_TEXT_BUFFER_SIZE) imsg = 0;
	while (1)
	{
		nlines = reset_soft_breaks (display_text_buffer[imsg].data, display_text_buffer[imsg].len, chat_zoom, chat_win_text_width);
		update_chat_window (nlines, display_text_buffer[imsg].chan_nr);
		ntot += nlines;
		if (imsg == last_message) break;
		if (++imsg > DISPLAY_TEXT_BUFFER_SIZE) imsg = 0;
	}
	update_console_win (ntot - total_nr_lines);
	total_nr_lines = ntot;
	
	// adjust the text position and scroll bar
	nr_displayed_lines = (int) (chat_out_text_height / (18.0f * chat_zoom));
	current_line = channels[active_tab].nr_lines - nr_displayed_lines;
	if (current_line < 0) current_line = 0;
	vscrollbar_set_bar_len (chat_win, chat_scroll_id, current_line);
	vscrollbar_set_pos (chat_win, chat_scroll_id, current_line);
	text_changed = 1;

	return 0;
}

int root_key_to_input_field (Uint32 key, Uint32 unikey)
{
	widget_list *w = widget_find (chat_win, chat_in_id);
	Uint16 keysym = key & 0xffff;
	Uint8 ch = key_to_char (unikey);
	text_field *tf;
	text_message *msg;
	int alt_on = key & ELW_ALT, ctrl_on = key & ELW_CTRL;
	
	if (w == NULL) return 0;
	if ( (w->Flags & TEXT_FIELD_EDITABLE) == 0) return 0;
	
	tf = (text_field *) w->widget_info;
	msg = tf->buffer;
	
	if (keysym == SDLK_ESCAPE)
	{
		msg->data[0] = '\0';
		msg->len = 0;
		tf->cursor = 0;
	}
	else if (ch == SDLK_RETURN && msg->len > 0)
	{	
		if (msg->data[0] == '%' && msg->len > 1) 
		{
			if ( (check_var (&(msg->data[1]), IN_GAME_VAR) ) < 0)
			{
				send_input_text_line (msg->data, msg->len);
			}
		}
		else if ( msg->data[0] == '#' || get_show_window (console_root_win) )
		{
			test_for_console_command (msg->data, msg->len);
		}
		else
		{
			send_input_text_line (msg->data, msg->len);
		}
		msg->data[0] = '\0';
		msg->len = 0;
		tf->cursor = 0;
	}
	else if (ch == SDLK_BACKSPACE && tf->cursor > 0)
	{
		int i;
		for (i = tf->cursor; i <= msg->len; i++)
			msg->data[i-1] = msg->data[i];
		tf->cursor--;
		msg->len--;
		reset_soft_breaks (msg->data, msg->len, w->size, w->len_x - 2 * CHAT_WIN_SPACE);
	}
	else if (ch == SDLK_DELETE && tf->cursor < msg->len)
	{
		int i;
		for (i = tf->cursor+1; i <= msg->len; i++)
			msg->data[i-1] = msg->data[i];
		msg->len--;
		reset_soft_breaks (msg->data, msg->len, w->size, w->len_x - 2 * CHAT_WIN_SPACE);
	}
	else if ( !alt_on && !ctrl_on && ( (ch >= 32 && ch <= 126) || (ch > 127 + c_grey4) ) && ch != '`' )
	{
		// watch for the '//' shortcut
		if (tf->cursor == 1 && ch == '/' && msg->data[0] == '/' && last_pm_from[0])
		{
			tf->cursor += put_string_in_buffer (msg, last_pm_from, 1);
			tf->cursor += put_char_in_buffer (msg, ' ', tf->cursor);
		}
		else if (msg->len < msg->size - 1)
		{
			tf->cursor += put_char_in_buffer (msg, ch, tf->cursor);
		}
		reset_soft_breaks (msg->data, msg->len, w->size, w->len_x - 2 * CHAT_WIN_SPACE);
	}

	else
	{
		return 0;
	}
	
	sync_chat_and_console ();
	return 1;
}

void paste_in_input_field (const Uint8 *text)
{
	widget_list *w = widget_find (chat_win, chat_in_id);
	text_field *tf;
	
	if (w == NULL) return;
	tf = (text_field *) w->widget_info;
	
	put_string_in_buffer (&input_text_line, text, tf->cursor);
	reset_soft_breaks (tf->buffer->data, tf->buffer->len, w->size, w->len_x - 2 * CHAT_WIN_SPACE);
}

void display_chat ()
{
	if (chat_win < 0)
	{
		int chat_win_width = CHAT_WIN_TEXT_WIDTH + 4 * CHAT_WIN_SPACE + CHAT_WIN_SCROLL_WIDTH;
		int chat_win_height = CHAT_OUT_TEXT_HEIGHT + CHAT_IN_TEXT_HEIGHT + 7 * CHAT_WIN_SPACE + CHAT_WIN_TAG_HEIGHT;
		int inout_width = CHAT_WIN_TEXT_WIDTH + 2 * CHAT_WIN_SPACE;
		int output_height = CHAT_OUT_TEXT_HEIGHT + 2 * CHAT_WIN_SPACE;
		int tabcol_height = output_height + CHAT_WIN_TAG_HEIGHT;
		int input_y = tabcol_height + 2 * CHAT_WIN_SPACE;
		int input_height = CHAT_IN_TEXT_HEIGHT + 2 * CHAT_WIN_SPACE;
		
		int min_width = CHAT_WIN_SCROLL_WIDTH + 2 * CHAT_WIN_SPACE + (int)(CHAT_WIN_TEXT_WIDTH * chat_zoom);
		int min_height = 7 * CHAT_WIN_SPACE + CHAT_WIN_TAG_HEIGHT + (int) ((2+5) * 18.0 * chat_zoom);
		
		int imsg, nlines, ntot;
		
		nr_displayed_lines = (int) ((CHAT_OUT_TEXT_HEIGHT-1) / (18.0 * chat_zoom));
				
		chat_win = create_window ("Chat", game_root_win, 0, chat_win_x, chat_win_y, chat_win_width, chat_win_height, (ELW_WIN_DEFAULT|ELW_RESIZEABLE) & ~ELW_CLOSE_BOX);
		
		set_window_handler (chat_win, ELW_HANDLER_DISPLAY, &display_chat_handler);
		set_window_handler (chat_win, ELW_HANDLER_RESIZE, &resize_chat_handler);

		chat_scroll_id = vscrollbar_add_extended (chat_win, chat_scroll_id, NULL, chat_win_width - CHAT_WIN_SCROLL_WIDTH, 0, CHAT_WIN_SCROLL_WIDTH, chat_win_height - ELW_BOX_SIZE, 0, 1.0f, 0.77f, 0.57f, 0.39f, 0, 1, 0);
		widget_set_OnDrag (chat_win, chat_scroll_id, chat_scroll_drag);
		widget_set_OnClick (chat_win, chat_scroll_id, chat_scroll_click);
		
		chat_tabcollection_id = tab_collection_add_extended (chat_win, chat_tabcollection_id, NULL, CHAT_WIN_SPACE, CHAT_WIN_SPACE, inout_width, tabcol_height, 0, 0.7, 0.77f, 0.57f, 0.39f, CHAT_WIN_MAX_TABS, CHAT_WIN_TAG_HEIGHT, CHAT_WIN_TAG_SPACE);
		widget_set_OnClick (chat_win, chat_tabcollection_id, chat_tabs_click);
		
		channels[0].tab_id = tab_add (chat_win, chat_tabcollection_id, tab_local, 0, 0);
		set_window_min_size (channels[0].tab_id, 0, 0);
		channels[0].out_id = text_field_add_extended (channels[0].tab_id, channels[0].out_id, NULL, 0, 0, inout_width, output_height, 0, chat_zoom, 0.77f, 0.57f, 0.39f, display_text_buffer, DISPLAY_TEXT_BUFFER_SIZE, CHANNEL_LOCAL, CHAT_WIN_SPACE, CHAT_WIN_SPACE, -1.0, -1.0, -1.0);		
		channels[0].chan_nr = CHANNEL_LOCAL;
		channels[0].nr_lines = 0;
		channels[0].open = 1;
		channels[0].new = 0;
		active_tab = 0;		

		chat_in_id = text_field_add_extended (chat_win, chat_in_id, NULL, CHAT_WIN_SPACE, input_y, inout_width, input_height, TEXT_FIELD_BORDER|TEXT_FIELD_EDITABLE|TEXT_FIELD_NO_KEYPRESS, chat_zoom, 0.77f, 0.57f, 0.39f, &input_text_line, 1, CHANNEL_ALL, CHAT_WIN_SPACE, CHAT_WIN_SPACE, 1.0, 1.0, 1.0);
		widget_set_OnKey (chat_win, chat_in_id, chat_input_key);
		
		set_window_min_size (chat_win, min_width, min_height);
		
		// update the channel information
		ntot = 0;
		imsg = buffer_full ? last_message+1 : 0;
		if (imsg > DISPLAY_TEXT_BUFFER_SIZE) imsg = 0;
		while (1)
		{
			nlines = reset_soft_breaks (display_text_buffer[imsg].data, display_text_buffer[imsg].len, chat_zoom, chat_win_text_width);
			update_chat_window (nlines, display_text_buffer[imsg].chan_nr);
			ntot += nlines;
			if (imsg == last_message) break;
			if (++imsg > DISPLAY_TEXT_BUFFER_SIZE) imsg = 0;
		}
		update_console_win (ntot - total_nr_lines);
		total_nr_lines = ntot;
	}
	else
	{
		show_window (chat_win);
		select_window (chat_win);
	}
}
