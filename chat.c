#include <stdlib.h>
#include <string.h>
#include "global.h"

int chat_win = -1;

void remove_chat_tab (Uint8 channel);
int add_chat_tab (int nlines, Uint8 channel);
void update_chat_tab_idx (Uint8 old_ix, Uint8 new_idx);
void remove_tab_button (Uint8 channel);
int add_tab_button (Uint8 channel);
void update_tab_button_idx (Uint8 old_idx, Uint8 new_idx);
void convert_tabs (int new_wc);

Uint32 active_channels[MAX_ACTIVE_CHANNELS];
Uint8 current_channel = 0;

void add_tab (Uint8 channel)
{
	if (tab_bar_win != -1) add_tab_button (channel);
	if (chat_win != -1) add_chat_tab (0, channel);
}

void remove_tab (Uint8 channel)
{
	if (tab_bar_win != -1) remove_tab_button (channel);
	if (chat_win != -1) remove_chat_tab (channel);
}

void update_tab_idx (Uint8 old_idx, Uint8 new_idx)
{
	// XXX: CAUTION
	// Since this function simply replaces old_idx y new_idx, it could 
	// potentially cause trouble when new_idx is already in use by 
	// another tab. However, as the code is now, successive calls to 
	// update_tab_idx are in increasing order of old_idx, and new_idx
	// is lower than old_idx, so we should be safe.

	if (tab_bar_win != -1) update_tab_button_idx (old_idx, new_idx);
	if (chat_win != -1) update_chat_tab_idx (old_idx, new_idx);
}

void set_channel_tabs (const Uint32 *chans)
{
	int nmax = CHAT_CHANNEL3-CHAT_CHANNEL1+1;
	Uint32 chan;
	Uint8 chan_nr, chan_nrp;
	
	for (chan_nr = 0; chan_nr < nmax; chan_nr++)
	{
		chan = chans[chan_nr];
		if (chan == 0) continue;
		
		for (chan_nrp = 0; chan_nrp < nmax; chan_nrp++)
		{
			if (active_channels[chan_nrp] == chan) break;
		}
		
		if (chan_nrp >= nmax)
		{
			// we left this channel
			remove_tab (chan_nr+CHAT_CHANNEL1);
		}
		else
		{
			update_tab_idx (chan_nr+CHAT_CHANNEL1, chan_nrp+CHAT_CHANNEL1);
		}
	}

	for (chan_nrp = 0; chan_nrp < nmax; chan_nrp++)
	{
		chan = active_channels[chan_nrp];

		if (chan == 0) continue;
		
		for (chan_nr = 0; chan_nr < nmax; chan_nr++)
		{
			if (chans[chan_nr] == chan) break;
		}
		
		if (chan_nr >= nmax)
		{
			// we have a new channel
			add_tab (chan_nrp+CHAT_CHANNEL1);
		}
	}
}

void set_active_channels (Uint8 active, const Uint32 *channels, int nchan)
{
	Uint32 tmp[MAX_ACTIVE_CHANNELS];
	int i;
	
	for (i = 0; i < MAX_ACTIVE_CHANNELS; i++)
		tmp[i] = active_channels[i];

	for (i = 0; i < nchan; i++)
		active_channels[i] = SDL_SwapLE32(channels[i]);
	for ( ; i < MAX_ACTIVE_CHANNELS; i++)
		active_channels[i] = 0;

	set_channel_tabs (tmp);

	current_channel = active;
}

void send_active_channel (Uint8 chan)
{
	Uint8 msg[2];

	if (chan >= CHAT_CHANNEL1 && chan <= CHAT_CHANNEL3)
	{
		msg[0] = SET_ACTIVE_CHANNEL;
		msg[1] = chan;
		my_tcp_send (my_socket, msg, 2);
		
		current_channel = chan - CHAT_CHANNEL1;
	}
}

Uint32 get_active_channel (Uint8 idx)
{
	if (idx >= CHAT_CHANNEL1 && idx <= CHAT_CHANNEL3)
		return active_channels[idx-CHAT_CHANNEL1];
	return 0;
}

#define CHAT_WIN_SPACE		4
#define CHAT_WIN_TAG_HEIGHT	20
#define CHAT_WIN_TAG_SPACE	3
#define CHAT_WIN_TEXT_WIDTH  	500
#define CHAT_OUT_TEXT_HEIGHT 	(18*8)
#define CHAT_IN_TEXT_HEIGHT 	(18*3)
#define CHAT_WIN_SCROLL_WIDTH	20

int local_chat_separate = 0;
int personal_chat_separate = 0;
int guild_chat_separate = 1;
int server_chat_separate = 0;
int mod_chat_separate = 0;

/* 
 * use_windowed_chat == 0: old behaviour, all text is printed
 * use_windowed_chat == 1: channel selection bar
 * use_windowed_chat == 2: chat window
 */
int use_windowed_chat = 1;
int highlight_tab_on_nick = 1;

////////////////////////////////////////////////////////////////////////
// Chat window variables

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

chat_channel channels[MAX_CHAT_TABS];
int active_tab = -1;

const char *tab_label (Uint8 chan);//Forward declaration

void init_chat_channels ()
{
	int itab;
	
	for (itab = 0; itab < MAX_CHAT_TABS; itab++)
	{
		channels[itab].tab_id = -1;
		channels[itab].out_id = chat_out_start_id + itab;
		channels[itab].chan_nr = CHAT_ALL;
		channels[itab].nr_lines = 0;
		channels[itab].open = 0;
		channels[itab].newchan = 0;
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
	// XXX FIXME (Grum): when the console input field is reinstated, perhaps
	// uncomment these lines again
	//widget_list *cons = widget_find (console_root_win, console_in_id);
	//text_field *cons_tf = (text_field *) cons->widget_info;
	
	input_text_line.data[0] = '\0';
	input_text_line.len = 0;
	//cons_tf->cursor = 0;
	if (use_windowed_chat == 2)
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
	char str[256];
	
	for (ichan = 0; ichan < MAX_CHAT_TABS; ichan++)
	{
		if (channels[ichan].tab_id == id)
		{
			snprintf(str, sizeof(str), "%c#lc %d", RAW_TEXT, channels[ichan].chan_nr);
			my_tcp_send(my_socket, str, strlen(str+1)+1);

			// Safe to remove?
			if (tab_bar_win != -1) remove_tab_button(channels[ichan].chan_nr);

			return 1;
		}
	}
	
	// we shouldn't get here
	LOG_ERROR ("Trying to close non-existant channel\n");
	return 0;
}

void remove_chat_tab (Uint8 channel)
{
	int ichan;

	for (ichan = 0; ichan < MAX_CHAT_TABS; ichan++)
	{
		if (channels[ichan].chan_nr == channel && channels[ichan].open)
		{
			int nr = tab_collection_get_tab_nr (chat_win, chat_tabcollection_id, channels[ichan].tab_id);
			tab_collection_close_tab (chat_win, chat_tabcollection_id, nr);
			
			channels[ichan].tab_id = -1;
			channels[ichan].chan_nr = CHAT_ALL;
			channels[ichan].nr_lines = 0;
			channels[ichan].open = 0;
			channels[ichan].newchan = 0;
			channels[ichan].highlighted = 0;

			return;
		}
	}
}

int add_chat_tab(int nlines, Uint8 channel)
{
	int ichan;

	for (ichan = 0; ichan < MAX_CHAT_TABS; ichan++)
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
			channels[ichan].newchan = 1;
			channels[ichan].highlighted = 0;

			my_strncp(title,tab_label (channel), sizeof(title));
			
			channels[ichan].tab_id = tab_add (chat_win, chat_tabcollection_id, title, 0, 1);
			set_window_flag (channels[ichan].tab_id, ELW_CLICK_TRANSPARENT);
				
			set_window_min_size (channels[ichan].tab_id, 0, 0);
			channels[ichan].out_id = text_field_add_extended (channels[ichan].tab_id, channels[ichan].out_id, NULL, 0, 0, inout_width, output_height, 0, chat_zoom, 0.77f, 0.57f, 0.39f, display_text_buffer, DISPLAY_TEXT_BUFFER_SIZE, channel, CHAT_WIN_SPACE, CHAT_WIN_SPACE, -1.0, -1.0, -1.0);

			set_window_handler (channels[ichan].tab_id, ELW_HANDLER_DESTROY, close_channel);
				
			if(!channels[ichan].highlighted && channels[active_tab].chan_nr != CHAT_ALL)
			{
				tab_set_label_color_by_id (chat_win, chat_tabcollection_id, channels[ichan].tab_id, 1.0, 1.0, 0.0);
			}
			
			return ichan;
		}
	}
	//no empty slot found
	return -1;
}

void update_chat_tab_idx (Uint8 old_idx, Uint8 new_idx)
{
	int itab;
	
	if (old_idx == new_idx) return;
	
	for (itab = 0; itab < MAX_CHAT_TABS; itab++)
	{
		if (channels[itab].chan_nr == old_idx && channels[itab].open)
		{
			channels[itab].chan_nr = new_idx;
			return;
		}
	}
}

Uint8 get_tab_channel (Uint8 channel)
{
	switch (channel)
	{
		case CHAT_LOCAL:
			if (!local_chat_separate) return CHAT_ALL;
			break;
		case CHAT_PERSONAL:
			if (!personal_chat_separate) return CHAT_ALL;
			break;
		case CHAT_GM:
			if (!guild_chat_separate) return CHAT_ALL;
			break;
		case CHAT_SERVER:
			if (!server_chat_separate) return CHAT_ALL;
			break;
		case CHAT_MOD:
			if (!mod_chat_separate) return CHAT_ALL;
			break;
		case CHAT_MODPM:
			// always display moderator PMs in all tabs
			return CHAT_ALL;
	}
	
	return channel;
}

void update_chat_window (text_message * msg, char highlight)
{
	int ichan, len, nlines, width, channel;
	char found;

	// don't bother if there's no chat window
	if (chat_win < 0) return;

	// rewrap message to get correct # of lines
	width = windows_list.window[chat_win].len_x;
	nlines = rewrap_message(msg, chat_zoom, width, NULL);
	
	// first check if we need to display in all open channels
	channel = get_tab_channel (msg->chan_idx);

	if (channel == CHAT_ALL || channel == CHAT_MODPM)
	{
		for (ichan = 0; ichan < MAX_CHAT_TABS; ichan++)
		{
			if (channels[ichan].open) {
				if (msg->deleted) {
					channels[ichan].nr_lines -= nlines;
				} else {
					channels[ichan].nr_lines += nlines;
				}
			}
		}
		
		len = channels[active_tab].nr_lines - nr_displayed_lines;
		if (len < 0) len = 0;
		vscrollbar_set_bar_len (chat_win, chat_scroll_id, len);
		vscrollbar_set_pos (chat_win, chat_scroll_id, len);

		current_line = channels[active_tab].nr_lines;
		text_changed = 1;
		return;
	}

	// message not for all channels, see if this channel is already open
	found = 0;
	for (ichan = 0; ichan < MAX_CHAT_TABS; ichan++)
	{
		if (channels[ichan].open && (channels[ichan].chan_nr == channel || channels[ichan].chan_nr == CHAT_ALL))
		{
			if (msg->deleted) {
				channels[ichan].nr_lines -= nlines;
			} else {
				channels[ichan].nr_lines += nlines;
			}
			channels[ichan].newchan = 1;

			if (ichan == active_tab)
			{
				len = channels[ichan].nr_lines - nr_displayed_lines;
				if (len < 0) len = 0;
					
				vscrollbar_set_bar_len (chat_win, chat_scroll_id, len);
				vscrollbar_set_pos (chat_win, chat_scroll_id, len);
				current_line = channels[ichan].nr_lines;
				text_changed = 1;
			}
			else if (highlight && !channels[ichan].highlighted && channels[active_tab].chan_nr != CHAT_ALL && channels[ichan].chan_nr != CHAT_ALL && !get_show_window(console_root_win)) //Make sure we don't change the color of a highlighted tab
			{
				tab_set_label_color_by_id (chat_win, chat_tabcollection_id, channels[ichan].tab_id, 1.0, 1.0, 0.0);
			}
			if (found) return; // we found the respective tab and the "all" tab now
			found++;
		}
	}

	// nothing to delete from
	if (msg->deleted) return;

	// channel not found, try to create a new one
	if(add_chat_tab(nlines, channel) == -1)
	{
		// uh oh, no empty slot found. this shouldn't really be happening...
		// log in general channel
		channels[0].nr_lines += nlines;	
		channels[0].newchan = 1;
		if (0 == active_tab)
		{
			len = channels[0].nr_lines - nr_displayed_lines;
			if (len < 0) len = 0;
		
			vscrollbar_set_bar_len (chat_win, chat_scroll_id, len);
			vscrollbar_set_pos (chat_win, chat_scroll_id, len);
			current_line = channels[active_tab].nr_lines;
			text_changed = 1;
		}
		else if (highlight && !channels[0].highlighted) //Make sure we don't change the color of a highlighted tab
		{
			tab_set_label_color_by_id (chat_win, chat_tabcollection_id, channels[0].tab_id, 1.0, 1.0, 0.0);
		}
	}
}

int display_chat_handler (window_info *win)
{
	static int msg_start = 0, offset_start = 0;
	if (text_changed)
	{
		int line = vscrollbar_get_pos (chat_win, chat_scroll_id);

		find_line_nr (channels[active_tab].nr_lines, line, channels[active_tab].chan_nr, &msg_start, &offset_start, chat_zoom, chat_win_text_width);
		text_field_set_buf_pos (channels[active_tab].tab_id, channels[active_tab].out_id, msg_start, offset_start);
		text_changed = 0;
	}

	return 1;
}

void switch_to_chat_tab(int id, char click)
{
	if(!click)
	{
		int itab;
		//Do what a mouse click would do
		widget_list *widget = widget_find(chat_win, chat_tabcollection_id);
		tab_collection *collection = widget->widget_info;

		for(itab = 0; itab < collection->nr_tabs; itab++)
		{
			if(collection->tabs[itab].content_id == id)
			{
				break;
			}
		}
		tab_collection_select_tab(chat_win, chat_tabcollection_id, itab);
	}
	tab_set_label_color_by_id (chat_win, chat_tabcollection_id, id, -1.0, -1.0, -1.0);

	//set active_tab
	for (active_tab = 0; active_tab < MAX_CHAT_TABS; active_tab++)
	{
		if (channels[active_tab].tab_id == id && channels[active_tab].open)
		{
			break;
		}
	}
	if (active_tab >= MAX_CHAT_TABS)
	{
		// This shouldn't be happening
		LOG_ERROR ("Trying to switch to non-existant channel");
		active_tab = 0;
	}
	current_line = channels[active_tab].nr_lines - nr_displayed_lines;
	if (current_line < 0)
	{
		current_line = 0;
	}
	vscrollbar_set_bar_len(chat_win, chat_scroll_id, current_line);
	vscrollbar_set_pos(chat_win, chat_scroll_id, current_line);
	text_changed = 1;
	channels[active_tab].highlighted = 0;

	if (channels[active_tab].chan_nr >= CHAT_CHANNEL1 && channels[active_tab].chan_nr <= CHAT_CHANNEL3)
		send_active_channel (channels[active_tab].chan_nr);
}

void change_to_current_chat_tab(const char *input)
{
	Uint8 channel;
	int ichan;
	int itab;

	if(input[0] == '@' || input[0] == char_at_str[0])
	{
		channel = CHAT_CHANNEL1 + current_channel;
	}
	else if(my_strncompare(input, "#gm", 3) || my_strncompare(input, gm_cmd_str, strlen(gm_cmd_str)))
	{
		channel = CHAT_GM;
	}
	else if(my_strncompare(input, "#mod", 4) || my_strncompare(input, mod_cmd_str, strlen(mod_cmd_str)))
	{
		channel = CHAT_MOD;
	}
	else if(my_strncompare(input, "#bc", 3) || my_strncompare(input, bc_cmd_str, strlen(bc_cmd_str)))
	{
		channel = CHAT_SERVER;
	}
	else if(input[0] == '/' || input[0] == char_slash_str[0])
	{
		channel = CHAT_PERSONAL;
	}
	else if(input[0] == '#' || input[0] == char_cmd_str[0]) {
		//We don't want to switch tab on commands.
		channel = CHAT_ALL;
	}
	else
	{
		channel = CHAT_LOCAL;
	}
	channel = get_tab_channel (channel);

	if(channel != CHAT_ALL)
	{
		for(ichan = 0; ichan < MAX_CHAT_TABS; ichan++)
		{
			if(channels[ichan].chan_nr == channel && channels[ichan].open)
			{
				if(ichan != active_tab) //We don't want to switch to the tab we're already in
				{
					switch_to_chat_tab(channels[ichan].tab_id, 0);
				}
				return;
			}
		}
		//We didn't find any tab to switch to, create new
		itab = add_chat_tab(0, channel);
		if(itab == -1)
		{
			//Eek, it failed, switch to general
			switch_to_chat_tab(channels[0].tab_id, 0);
		}
		else
		{
			switch_to_chat_tab(channels[itab].tab_id, 0);
		}
	}
}

int chat_tabs_click (widget_list *widget, int mx, int my, Uint32 flags)
{
	int id;
	
	id = tab_collection_get_tab_id (chat_win, widget->id);
	if (id != channels[active_tab].tab_id)
	{
		//We're not looking at the tab we clicked
		switch_to_chat_tab(id, 1);
		return 1;
	}
	return 0;
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
		if (tf->cursor > 0) 
		{
			do
			{
				tf->cursor--;
			}
			while (tf->cursor > 0 && msg->data[tf->cursor] == '\r');
		}
			
		return 1;
	}
	else if (keysym == K_ROTATERIGHT)
	{
		if (tf->cursor < msg->len)
		{
			do
			{
				tf->cursor++;
			} while (tf->cursor < msg->len && msg->data[tf->cursor] == '\r');
		}
		return 1;
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
	int itab;
	int scroll_x = width - CHAT_WIN_SCROLL_WIDTH;
	int scroll_height = height - 2*ELW_BOX_SIZE;
	int inout_width = width - CHAT_WIN_SCROLL_WIDTH - 2 * CHAT_WIN_SPACE;
	int input_height = CHAT_IN_TEXT_HEIGHT + 2 * CHAT_WIN_SPACE;
	int input_y = height - input_height - CHAT_WIN_SPACE;
	int tabcol_height = input_y - 2 * CHAT_WIN_SPACE;
	int output_height = tabcol_height - CHAT_WIN_TAG_HEIGHT;
	int line_height = (int) (18.0*chat_zoom);
	
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
	widget_move (chat_win, chat_scroll_id, scroll_x, ELW_BOX_SIZE);
	
	widget_resize (chat_win, chat_tabcollection_id, inout_width, tabcol_height);
	
	for (itab = 0; itab < MAX_CHAT_TABS; itab++)
		if (channels[itab].tab_id >= 0)
			widget_resize (channels[itab].tab_id, channels[itab].out_id, inout_width, output_height);
	
	widget_resize (chat_win, chat_in_id, inout_width, input_height);
	widget_move (chat_win, chat_in_id, CHAT_WIN_SPACE, input_y);

	update_chat_win_buffers();
	
	return 0;
}


void update_chat_win_buffers()
{
	int itab, imsg;
	// recompute line breaks
	for (itab = 0; itab < MAX_CHAT_TABS; itab++)
		channels[itab].nr_lines = 0;

	imsg = buffer_full ? last_message+1 : 0;
	if (imsg >= DISPLAY_TEXT_BUFFER_SIZE) imsg = 0;
	while (1)
	{
		update_chat_window (&display_text_buffer[imsg], 0);
		if (imsg == last_message || last_message < 0) break;
		if (++imsg >= DISPLAY_TEXT_BUFFER_SIZE) imsg = 0;
	}
	
	// adjust the text position and scroll bar
	nr_displayed_lines = (int) (chat_out_text_height / (18.0f * chat_zoom));
	current_line = channels[active_tab].nr_lines - nr_displayed_lines;
	if (current_line < 0) current_line = 0;
	vscrollbar_set_bar_len (chat_win, chat_scroll_id, current_line);
	vscrollbar_set_pos (chat_win, chat_scroll_id, current_line);
	text_changed = 1;
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
//		else if ( msg->data[0] == '#' || get_show_window (console_root_win) )
		else if ( msg->data[0] == '#' || msg->data[0] == char_cmd_str[0] || get_show_window (console_root_win) )
		{
#ifdef COMMAND_BUFFER
			if(test_for_console_command (msg->data, msg->len) || msg->data[0] == '#' || msg->data[0] == char_cmd_str[0])
			{
				add_line_to_history(msg->data, msg->len);
			}
#else
			test_for_console_command (msg->data, msg->len);
#endif //COMMAND_BUFFER
		}
		else
		{
			if(msg->data[0] == char_at_str[0])msg->data[0]='@';
			send_input_text_line (msg->data, msg->len);
		}
		msg->data[0] = '\0';
		msg->len = 0;
		tf->cursor = 0;
		tf->nr_lines = 0;
	}
#ifndef OSX
	else if (ch == SDLK_BACKSPACE && tf->cursor > 0)
#else
        else if (((ch == SDLK_BACKSPACE) || (ch == 127)) && tf->cursor > 0)
#endif
	{
		int i = tf->cursor, n = 1;
		
		while (n < i && msg->data[i-n] == '\r') n++;
		
		for ( ; i <= msg->len; i++)
			msg->data[i-n] = msg->data[i];

		tf->cursor -= n;
		msg->len -= n;
		// set invalid width to force rewrap
		msg->wrap_width = 0;
		tf->nr_lines = rewrap_message (msg, w->size, w->len_x, &tf->cursor);
		
		return 1;
	}
	else if (ch == SDLK_DELETE && tf->cursor < msg->len)
	{
		int i = tf->cursor, n = 1;
		
		while (i+n <= msg->len && msg->data[i+n] == '\r') n++;
		
		for (i += n; i <= msg->len; i++)
			msg->data[i-n] = msg->data[i];

		msg->len -= n;
		// set invalid width to force rewrap
		msg->wrap_width = 0;
		tf->nr_lines = rewrap_message (msg, w->size, w->len_x, &tf->cursor);
		
		return 1;
	}
	else if ( !alt_on && !ctrl_on && IS_PRINT (ch) && ch != '`' )
	{
		int nr_lines;
	
		// watch for the '//' shortcut
		if (tf->cursor == 1 && (ch == '/' || ch == char_slash_str[0])
			&& (msg->data[0] == '/' || msg->data[0]== char_slash_str[0])
			&& last_pm_from[0])
		{
			tf->cursor += put_string_in_buffer (msg, last_pm_from, 1);
			tf->cursor += put_char_in_buffer (msg, ' ', tf->cursor);
		}
		else if (msg->len < msg->size - 1)
		{
			tf->cursor += put_char_in_buffer (msg, ch, tf->cursor);
		}
		// set invalid width to force rewrap
		msg->wrap_width = 0;
		nr_lines = rewrap_message (msg, w->size, w->len_x - 2 * CHAT_WIN_SPACE, &tf->cursor);
		
		if (nr_lines != tf->nr_lines)
		{
			msg->len += nr_lines - tf->nr_lines;
			tf->nr_lines = nr_lines;
		}
	}
	else
	{
		return 0;
	}
	
	// wytter removed the console text input field
	//sync_chat_and_console ();
	return 1;
}

void paste_in_input_field (const Uint8 *text)
{
	widget_list *w = widget_find (chat_win, chat_in_id);
	text_field *tf;
	
	if (w == NULL) return;
	tf = (text_field *) w->widget_info;
	
	put_string_in_buffer (&input_text_line, text, tf->cursor);

	// set invalid width to force rewrap
	tf->buffer->wrap_width = 0;
	tf->nr_lines = rewrap_message(tf->buffer, w->size, w->len_x - 2 * CHAT_WIN_SPACE, &tf->cursor);
}

int close_chat_handler (window_info *win)
{
	// revert to using the tab bar
	use_windowed_chat = 1;
	if (game_root_win >= 0) display_tab_bar ();
	convert_tabs (use_windowed_chat);
	
	return 1;
}

void create_chat_window() {
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
			
	chat_win = create_window ("Chat", game_root_win, 0, chat_win_x, chat_win_y, chat_win_width, chat_win_height, ELW_WIN_DEFAULT|ELW_RESIZEABLE|ELW_CLICK_TRANSPARENT);
	
	set_window_handler (chat_win, ELW_HANDLER_DISPLAY, &display_chat_handler);
	set_window_handler (chat_win, ELW_HANDLER_RESIZE, &resize_chat_handler);
	set_window_handler (chat_win, ELW_HANDLER_CLOSE, &close_chat_handler);

	chat_scroll_id = vscrollbar_add_extended (chat_win, chat_scroll_id, NULL, chat_win_width - CHAT_WIN_SCROLL_WIDTH, ELW_BOX_SIZE, CHAT_WIN_SCROLL_WIDTH, chat_win_height - 2*ELW_BOX_SIZE, 0, 1.0f, 0.77f, 0.57f, 0.39f, 0, 1, 0);
	widget_set_OnDrag (chat_win, chat_scroll_id, chat_scroll_drag);
	widget_set_OnClick (chat_win, chat_scroll_id, chat_scroll_click);
	
	chat_tabcollection_id = tab_collection_add_extended (chat_win, chat_tabcollection_id, NULL, CHAT_WIN_SPACE, CHAT_WIN_SPACE, inout_width, tabcol_height, 0, 0.7, 0.77f, 0.57f, 0.39f, MAX_CHAT_TABS, CHAT_WIN_TAG_HEIGHT);
	widget_set_OnClick (chat_win, chat_tabcollection_id, chat_tabs_click);
	
	channels[0].tab_id = tab_add (chat_win, chat_tabcollection_id, tab_all, 0, 0);
	set_window_flag (channels[0].tab_id, ELW_CLICK_TRANSPARENT);
	set_window_min_size (channels[0].tab_id, 0, 0);
	channels[0].out_id = text_field_add_extended (channels[0].tab_id, channels[0].out_id, NULL, 0, 0, inout_width, output_height, 0, chat_zoom, 0.77f, 0.57f, 0.39f, display_text_buffer, DISPLAY_TEXT_BUFFER_SIZE, FILTER_ALL, CHAT_WIN_SPACE, CHAT_WIN_SPACE, -1.0, -1.0, -1.0);
	channels[0].chan_nr = CHAT_ALL;
	channels[0].nr_lines = 0;
	channels[0].open = 1;
	channels[0].newchan = 0;
	active_tab = 0;		

	chat_in_id = text_field_add_extended (chat_win, chat_in_id, NULL, CHAT_WIN_SPACE, input_y, inout_width, input_height, TEXT_FIELD_BORDER|TEXT_FIELD_EDITABLE|TEXT_FIELD_NO_KEYPRESS, chat_zoom, 0.77f, 0.57f, 0.39f, &input_text_line, 1, FILTER_ALL, CHAT_WIN_SPACE, CHAT_WIN_SPACE, 1.0, 1.0, 1.0);
	widget_set_OnKey (chat_win, chat_in_id, chat_input_key);
	
	set_window_min_size (chat_win, min_width, min_height);
}

void display_chat ()
{
	if (chat_win < 0)
	{
		create_chat_window ();
	}
	else
	{
		show_window (chat_win);
		select_window (chat_win);
	}
	update_chat_win_buffers();
}

void chat_win_update_zoom () {
	int itab;
	widget_set_size(chat_win, chat_in_id, chat_zoom);
	for (itab = 0; itab < MAX_CHAT_TABS; itab++) {
		if (channels[itab].open) {
			widget_set_size(channels[itab].tab_id, channels[itab].out_id, chat_zoom);
		}
	}
	text_changed = 1;
}

////////////////////////////////////////////////////////////////////////

int tab_bar_win = -1;
chat_tab tabs[MAX_CHAT_TABS];
int cur_button_id = 0;
int tabs_in_use = 0;
int current_tab = 0;

int tab_bar_width = 0;
int tab_bar_height = 18;

int highlight_tab(const Uint8 channel)
{
	int i;
	
	if(!highlight_tab_on_nick || channel == CHAT_ALL)
	{
		//We don't want to highlight
		return 0;
	}
	switch(use_windowed_chat)
	{
		case 1:
			if (tab_bar_win < 0)
			{
				//it doesn't exist
				return 0;
			}
			if(tabs[current_tab].channel != CHAT_ALL) {
				/* If we're in the All tab, we have already seen this message */
				for (i = 0; i < tabs_in_use; i++)
				{
					if (tabs[i].channel == channel)
					{
						if (current_tab != i && !tabs[i].highlighted)
						{
							widget_set_color (tab_bar_win, tabs[i].button, 1.0f, 0.0f, 0.0f);
							tabs[i].highlighted = 1;
						}
						break;
					}
				}
			}
		break;
		case 2:
			if (chat_win < 0)
			{
				//it doesn't exist
				return 0;
			}
			if(channels[active_tab].chan_nr != CHAT_ALL) {
				/* If we're in the All tab, we have already seen this message */
				for (i = 0; i < MAX_CHAT_TABS; i++)
				{
					if (channels[i].open && channels[i].chan_nr == channel)
					{
						if (i != active_tab && !channels[i].highlighted)
						{
							tab_set_label_color_by_id (chat_win, chat_tabcollection_id, channels[i].tab_id, 1.0, 0.0, 0.0);
							channels[i].highlighted = 1;
						}
						break;
					}
				}
			}
		break;
		default:
			return 0;
		break;
	}
	return 1;
}

void switch_to_tab(int id)
{
	widget_set_color (tab_bar_win, tabs[current_tab].button, 0.77f, 0.57f, 0.39f);
	current_tab = id;
	widget_set_color (tab_bar_win, tabs[current_tab].button, 0.57f, 1.0f, 0.59f);
	current_filter = tabs[current_tab].channel;
	tabs[current_tab].highlighted = 0;
	
	if (tabs[current_tab].channel >= CHAT_CHANNEL1 && tabs[current_tab].channel <= CHAT_CHANNEL3)
		send_active_channel (tabs[current_tab].channel); 
}

int tab_bar_button_click (widget_list *w, int mx, int my, Uint32 flags)
{
	int itab;
	
	for (itab = 0; itab < tabs_in_use; itab++)
	{
		if (w->id == tabs[itab].button)
			break;
	}
	
	if (itab >= tabs_in_use)
		// shouldn't happen
		return 0;
	
#ifdef WIDGETS_FIX

	// NOTE: This is an optimization, instead of redefining a "Tab/Button" type.
	//		 Further use of this would be best served be a new definition.
	// Detect clicking on 'x'
	if(tabs[itab].channel == CHAT_CHANNEL1 || tabs[itab].channel == CHAT_CHANNEL2 ||
	   tabs[itab].channel == CHAT_CHANNEL3)
	{
			int x = w->len_x - 6;
			int y = 5;
			char str[256];

			// 'x' was clicked?
			if(mx > x-4 && mx < x+3 && my > y-4 && my < y+3)
			{
				// Drop this channel via #lc
				snprintf(str, sizeof(str), "%c#lc %d", RAW_TEXT, active_channels[tabs[itab].channel-CHAT_CHANNEL1]);
				my_tcp_send(my_socket, str, strlen(str+1)+1);
				// Can I remove this?
				remove_tab(tabs[itab].channel);
			}
	}

#endif

	if (current_tab != itab)
	{
		switch_to_tab(itab);
	}
	lines_to_show = 10;
	
	return 1;
}

char tmp_tab_label[20];

const char *tab_label (Uint8 chan)
{
	int cnr;

	switch (chan)
	{
		case CHAT_ALL: return tab_all;
		case CHAT_NONE: return tab_none;
		case CHAT_LOCAL: return tab_local;
		case CHAT_PERSONAL: return tab_personal;
		case CHAT_GM: return tab_guild;
		case CHAT_SERVER: return tab_server;
		case CHAT_MOD: return tab_mod;
		case CHAT_CHANNEL1:
		case CHAT_CHANNEL2:
		case CHAT_CHANNEL3:
			cnr = active_channels[chan-CHAT_CHANNEL1];
			switch (cnr)
			{
				case 1: return tab_newbie_channel;
				case 2: return tab_help_channel;
				case 3: return tab_market_channel;
				case 4: return tab_general_channel;
				case 5: return tab_roleplaying_channel;
				case 6: return tab_contest_channel;
				case 20: return tab_offtopic_channel;
				default: 
					if (cnr >= 1000000000)
					{
						return tab_guild_channel;
					}
					snprintf (tmp_tab_label, sizeof (tmp_tab_label), tab_channel, cnr);
					return tmp_tab_label;
			}
		default:
			// shouldn't get here 
			return "";
	}
}

void update_tab_button_idx (Uint8 old_idx, Uint8 new_idx)
{
	int itab;
	
	if (old_idx == new_idx) return;
	
	for (itab = 0; itab < tabs_in_use; itab++)
	{
		if (tabs[itab].channel == old_idx)
		{
			tabs[itab].channel = new_idx;
			return;
		}
	}
}

#ifdef WIDGETS_FIX

// Just for drawing an 'x' on channel buttons.

int draw_tab_x (widget_list *W)
{
	int x = W->pos_x + W->len_x - 6;
	int y = W->pos_y+5;

	glColor3f(0.77f,0.57f,0.39f);

	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINES);
		glVertex2i(x-4,y-4);
		glVertex2i(x+3,y+3);

		glVertex2i(x-4,y+3);
		glVertex2i(x+3,y-4);
	glEnd();
	glEnable(GL_TEXTURE_2D);

	return 1;
}

#endif

int add_tab_button (Uint8 channel)
{
	int itab;
	const char *label;

	for (itab = 0; itab < tabs_in_use; itab++)
	{
		if (tabs[itab].channel == channel)
			// already there
			return itab;
	}
	
	if (tabs_in_use >= MAX_CHAT_TABS)
		// no more room. Shouldn't happen anyway.
		return -1;

	tabs[tabs_in_use].channel = channel;
	tabs[tabs_in_use].highlighted = 0;
	label = tab_label (channel);

	tabs[tabs_in_use].button = button_add_extended (tab_bar_win, cur_button_id++, NULL, tab_bar_width, 0, 0, tab_bar_height, 0, 0.75, 0.77f, 0.57f, 0.39f, label);
	widget_set_OnClick (tab_bar_win, tabs[itab].button, tab_bar_button_click);
#ifndef WIDGETS_FIX
	widget_set_OnDraw (tab_bar_win, tabs[itab].button, square_button_draw);

#else
	widget_set_type(tab_bar_win, tabs[itab].button, &square_button_type); 
 	// Handlers for the 'x'
 	// Make sure it's a CHANNEL first
 	if(tabs[itab].channel == CHAT_CHANNEL1 || tabs[itab].channel == CHAT_CHANNEL2 ||
 	   tabs[itab].channel == CHAT_CHANNEL3)
 	{
 		widget_set_OnDraw (tab_bar_win, tabs[itab].button, draw_tab_x);
 	}
#endif
	tab_bar_width += widget_get_width (tab_bar_win, tabs[tabs_in_use].button)+1;
	resize_window (tab_bar_win, tab_bar_width, tab_bar_height);

	tabs_in_use++;
	return tabs_in_use - 1;
}

void remove_tab_button (Uint8 channel)
{
	int itab, w;

	for (itab = 0; itab < tabs_in_use; itab++)
	{
		if (tabs[itab].channel == channel)
			break;
	}
	if (itab >= tabs_in_use) return;
	
	w = widget_get_width (tab_bar_win, tabs[itab].button)+1;
	widget_destroy (tab_bar_win, tabs[itab].button);
	for (++itab; itab < tabs_in_use; itab++)
	{
		widget_move_rel (tab_bar_win, tabs[itab].button, -w, 0);
		tabs[itab-1] = tabs[itab];
	}
	tabs_in_use--;

	tab_bar_width -= w;
	resize_window (tab_bar_win, tab_bar_width, tab_bar_height);
}

void update_tab_bar (text_message * msg)
{
	int itab, new_button;
	Uint8 channel;

	// dont need to care for deleted messages
	if (msg->deleted) return;

	// don't bother if there's no tab bar
	if (tab_bar_win < 0) return;

	// Only update specific channels
	channel = get_tab_channel (msg->chan_idx);
	if (channel == CHAT_ALL || channel == CHAT_MODPM) {
		lines_to_show += rewrap_message(msg, chat_zoom, console_text_width, NULL);
		if (lines_to_show >= 10) lines_to_show = 10;
		return;
	}

	if (tabs[current_tab].channel == CHAT_ALL) {
		lines_to_show += rewrap_message(msg, chat_zoom, console_text_width, NULL);
		if (lines_to_show >= 10) lines_to_show = 10;
	}

	for (itab = 0; itab < tabs_in_use; itab++)
	{
		if (tabs[itab].channel == channel)
		{
			if (current_tab != itab && !tabs[itab].highlighted && tabs[current_tab].channel != CHAT_ALL && !get_show_window(console_root_win))
				widget_set_color (tab_bar_win, tabs[itab].button, 1.0f, 1.0f, 0.0f);
			if (current_tab == itab) {
				lines_to_show += rewrap_message(msg, chat_zoom, console_text_width, NULL);
				if (lines_to_show >= 10) lines_to_show = 10;
			}
			return;
		}
	}
	
	// we need a new button
	new_button = add_tab_button (channel);
	if(tabs[current_tab].channel != CHAT_ALL) {
		widget_set_color (tab_bar_win, tabs[new_button].button, 1.0f, 1.0f, 0.0f);
	}
}

void create_tab_bar ()
{
	int tab_bar_x = 10;
	int tab_bar_y = 3;

	tab_bar_win = create_window ("Tab bar", game_root_win, 0, tab_bar_x, tab_bar_y, tab_bar_width < ELW_BOX_SIZE ? ELW_BOX_SIZE : tab_bar_width, tab_bar_height, ELW_USE_BACKGROUND|ELW_SHOW);
	
	add_tab_button (CHAT_ALL);
	//add_tab_button (CHAT_NONE);
	current_tab = 0;
	widget_set_color (tab_bar_win, tabs[current_tab].button, 0.57f, 1.0f, 0.59f);
	current_filter = tabs[current_tab].channel;
}

void display_tab_bar ()
{
	if (tab_bar_win < 0)
	{
		create_tab_bar ();
	}
	else
	{
		show_window (tab_bar_win);
		select_window (tab_bar_win);
	}
}

void change_to_current_tab(const char *input)
{
	Uint8 channel;
	int itab;

	if(input[0] == '@' || input[0] == char_at_str[0])
	{
		channel = CHAT_CHANNEL1 + current_channel;
	}
	else if(my_strncompare(input, "#gm", 3) || my_strncompare(input, gm_cmd_str,strlen(gm_cmd_str)))
	{
		channel = CHAT_GM;
	}
	else if(my_strncompare(input, "#mod", 4) || my_strncompare(input, mod_cmd_str, strlen(mod_cmd_str)))
	{
		channel = CHAT_MOD;
	}
	else if(my_strncompare(input, "#bc", 3) || my_strncompare(input, bc_cmd_str, strlen(bc_cmd_str)))
	{
		channel = CHAT_SERVER;
	}
	else if(input[0] == '/' || input[0]==char_slash_str[0])
	{
		channel = CHAT_PERSONAL;
	}
	else if(input[0] == '#' || input[0] == char_cmd_str[0])
	{
		//We don't want to switch tab on commands.
		channel = CHAT_ALL;
	}
	else
	{
		channel = CHAT_LOCAL;
	}
	switch (channel)
	{
		case CHAT_LOCAL:
			if (!local_chat_separate)
			{
				channel = CHAT_ALL;
			}
		break;
		case CHAT_PERSONAL:
			if (!personal_chat_separate)
			{
				channel = CHAT_ALL;
			}
		break;
		case CHAT_GM:
			if (!guild_chat_separate)
			{
				channel = CHAT_ALL;
			}
		break;
		case CHAT_SERVER:
			if (!server_chat_separate)
			{
				channel = CHAT_ALL;
			}
		break;
		case CHAT_MOD:
			if (!mod_chat_separate)
			{
				channel = CHAT_ALL;
			}
		break;
	}
	if(channel != CHAT_ALL)
	{
		for(itab = 0; itab < tabs_in_use; itab++)
		{
			if(tabs[itab].channel == channel)
			{
				if(itab != current_tab) //We don't want to switch to the tab we're already in
				{
					switch_to_tab(itab);
				}
				return;
			}
		}
		//We didn't find any tab to switch to, create new
		itab = add_tab_button (channel);
		if (itab >= 0) switch_to_tab (itab);
	}
}

void convert_tabs (int new_wc)
{
	int iwc, ibc;
	Uint8 chan;
	
	if (new_wc == 1)
	{
		// first close possible remaining tab buttons that are no 
		// longer active
		for (ibc = 0; ibc < tabs_in_use; ibc++)
		{
			chan = tabs[ibc].channel;
			for (iwc = 0; iwc < MAX_CHAT_TABS; iwc++)
			{
				if (channels[iwc].chan_nr == chan && channels[iwc].open)
					break;
			}
			
			if (iwc >= MAX_CHAT_TABS)
				remove_tab_button (chan);
		}
		
		// now add buttons for every tab that doesn't have a button yet
		for (iwc = 0; iwc < MAX_CHAT_TABS; iwc++)
		{
			if (channels[iwc].open)
			{
				chan = channels[iwc].chan_nr;
				for (ibc = 0; ibc < tabs_in_use; ibc++)
				{
					if (tabs[ibc].channel == chan)
						break;
				}
				
				if (ibc >= tabs_in_use)
				{
					add_tab_button (chan);
				}
			}
		}
	}
	else if (new_wc == 2)
	{
		// first close possible remaining tabs that are no 
		// longer active
		for (iwc = 0; iwc < MAX_CHAT_TABS; iwc++)
		{
			if (channels[iwc].open)
			{
				chan = channels[iwc].chan_nr;
				for (ibc = 0; ibc < tabs_in_use; ibc++)
				{
					if (tabs[ibc].channel == chan)
						break;
				}
				
				if (ibc >= tabs_in_use)
				{
					remove_chat_tab (chan);
				}
			}
		}

		// now add tabs for every button that doesn't have a tab yet
		for (ibc = 0; ibc < tabs_in_use; ibc++)
		{
			chan = tabs[ibc].channel;
			for (iwc = 0; iwc < MAX_CHAT_TABS; iwc++)
			{
				if (channels[iwc].chan_nr == chan && channels[iwc].open)
					break;
			}
			
			if (iwc >= MAX_CHAT_TABS)
				// unfortunately we have no clue about the 
				// number of lines written in this channel, so
				// we won't see anything until new messages
				// arrive. Oh well.
				add_chat_tab (0, chan);
		}
	}
}

