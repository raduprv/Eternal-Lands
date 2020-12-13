#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <libxml/parser.h>
#include "chat.h"
#include "asc.h"
#include "colors.h"
#include "console.h"
#include "consolewin.h"
#include "elconfig.h"
#include "errors.h"
#include "gamewin.h"
#include "hud.h"
#include "icon_window.h"
#include "init.h"
#include "interface.h"
#ifdef JSON_FILES
#include "json_io.h"
#endif
#include "loginwin.h"
#include "main.h"
#include "mapwin.h"
#include "multiplayer.h"
#include "queue.h"
#include "text.h"
#include "translate.h"
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif
#include "io/elfilewrapper.h"
#include "io/elpathwrapper.h"
#include "sound.h"

static queue_t *chan_name_queue;
widget_list *input_widget = NULL;

/*!
 * \name Tabbed and old behaviour chat
 */
/*! @{ */
max_chat_lines_def max_chat_lines = { .value = 10, .lower = 5, .upper = 100 }; /*!< the upper, lower and current max lines to display */
static int lines_to_show = 0; /*!< the number of lines currently shown */
int get_lines_to_show(void) { return lines_to_show; }
void dec_lines_to_show(void) { if (lines_to_show > 0) lines_to_show--; }
void clear_lines_to_show(void) { lines_to_show = 0; }
/*! @} */

static void remove_chat_tab (Uint8 channel);
static int add_chat_tab (int nlines, Uint8 channel);
static void update_chat_tab_idx (Uint8 old_ix, Uint8 new_idx);
static void remove_tab_button (Uint8 channel);
static int add_tab_button (Uint8 channel);
static void update_tab_button_idx (Uint8 old_idx, Uint8 new_idx);
static int resize_chat_handler(window_info *win, int width, int height);
static void change_to_current_tab(const char *input);
static int display_channel_color_win(Uint32 channel_number);

#define MAX_CHANNEL_COLORS 64
#define MAX_CHAT_TABS 12			/*!< Size of the \see channels array */
#define MAX_ACTIVE_CHANNELS	10		/*!< Maximum number of channels in use */
#define SPEC_CHANS 12				/*!< 11 are currently in use. read channels.xml for the list */

typedef struct
{
	int tab_id;
	int out_id;
	Uint8 chan_nr;
	int nr_lines;
	char open, newchan, highlighted;
} chat_channel;

typedef struct
{
	Uint8 channel;
	int button;
	char highlighted;
	char * description;
} chat_tab;

typedef struct
{
	Uint32 channel;
	char * name;
	char * description;
} chan_name;

static chat_channel channels[MAX_CHAT_TABS]; /*!< Infos about a chat window tabs  */
static channelcolor channel_colors[MAX_CHANNEL_COLORS];
static Uint32 active_channels[MAX_ACTIVE_CHANNELS];
static Uint8 current_channel = 0;
static chan_name * pseudo_chans[SPEC_CHANS];

static const int tab_control_border_sep = 2;
static const int tab_control_half_len = 5;
static const int tab_control_underline_sep = 4;

int enable_chat_show_hide = 0;
static int chat_shown = 1;

void enable_chat_shown(void)
{
	chat_shown = 1;
	switch (use_windowed_chat)
	{
		case 0: // old chat
			lines_to_show = max_chat_lines.value;
			break;
		case 1: // chat bar
			show_window(tab_bar_win);
			lines_to_show = max_chat_lines.value;
			break;
		case 2: // chat window
			show_window(get_id_MW(MW_CHAT));
			break;
	}
	set_icon_state("chat", (enable_chat_show_hide != 0));
}

int is_chat_shown(void)
{
	return chat_shown;
}

void open_chat(void)
{
	if (enable_chat_show_hide)
		toggle_chat();
}

void toggle_chat(void)
{
	if (get_window_showable(get_id_MW(MW_CONSOLE)))
		return;
	if (!enable_chat_show_hide)
		return;
	switch (use_windowed_chat)
	{
		case 0: // old chat
			if ((chat_shown = ((chat_shown) ?0 :1)))
				lines_to_show = max_chat_lines.value;
			break;
		case 1: // chat bar
			toggle_window(tab_bar_win);
			if ((chat_shown = get_window_showable(tab_bar_win)))
				lines_to_show = max_chat_lines.value;
			break;
		case 2: // chat window
			toggle_window(get_id_MW(MW_CHAT));
			chat_shown = get_window_showable(get_id_MW(MW_CHAT));
			break;
	}
}

int get_tabbed_chat_end_x(void)
{
	if ((tab_bar_win < 0) || (use_windowed_chat != 1))
		return 0;
	return windows_list.window[tab_bar_win].cur_x + windows_list.window[tab_bar_win].len_x;
}

void input_widget_move_to_win(int window_id)
{
	window_info *win = NULL;
	if ((window_id >= 0) && (window_id < windows_list.num_windows))
		win = &windows_list.window[window_id];
	if ((input_widget == NULL) || (win == NULL))
		return;

	widget_move_win(input_widget->window_id, input_widget->id, window_id);
	if(window_id == get_id_MW(MW_CHAT)) {
		widget_set_flags(input_widget->window_id, input_widget->id, TEXT_FIELD_BORDER|TEXT_FIELD_EDITABLE|TEXT_FIELD_NO_KEYPRESS);
		input_widget->OnResize = NULL;
		resize_chat_handler(win, win->len_x, win->len_y);
	} else {
		text_field *tf = input_widget->widget_info;
		int text_height = get_text_height(tf->nr_lines, CHAT_FONT, input_widget->size);
		Uint32 flags;

		input_widget->OnResize = input_field_resize;
		if(window_id == get_id_MW(MW_CONSOLE)) {
			flags = (TEXT_FIELD_BORDER|INPUT_DEFAULT_FLAGS)^WIDGET_CLICK_TRANSPARENT;
		} else if(window_id == game_root_win && input_text_line.len == 0) {
			flags = INPUT_DEFAULT_FLAGS|WIDGET_DISABLED;
		} else if(window_id == get_id_MW(MW_TABMAP)) {
			flags = INPUT_DEFAULT_FLAGS|WIDGET_INVISIBLE;
		} else {
			flags = INPUT_DEFAULT_FLAGS;
		}
		widget_set_flags(input_widget->window_id, input_widget->id, flags);
		widget_resize(input_widget->window_id, input_widget->id,
			win->len_x - HUD_MARGIN_X, 2 * tf->y_space + text_height);
		widget_move(input_widget->window_id, input_widget->id, 0, win->len_y-input_widget->len_y-HUD_MARGIN_Y);
	}
}

static void add_tab (Uint8 channel)
{
	if (tab_bar_win != -1) add_tab_button (channel);
	if (get_id_MW(MW_CHAT) != -1) add_chat_tab (0, channel);
}

static void remove_tab (Uint8 channel)
{
	recolour_messages(display_text_buffer);
	if (tab_bar_win != -1) remove_tab_button (channel);
	if (get_id_MW(MW_CHAT) != -1) remove_chat_tab (channel);
}

static void update_tab_idx (Uint8 old_idx, Uint8 new_idx)
{
	// XXX: CAUTION
	// Since this function simply replaces old_idx y new_idx, it could
	// potentially cause trouble when new_idx is already in use by
	// another tab. However, as the code is now, successive calls to
	// update_tab_idx are in increasing order of old_idx, and new_idx
	// is lower than old_idx, so we should be safe.

	if (tab_bar_win != -1) update_tab_button_idx (old_idx, new_idx);
	if (get_id_MW(MW_CHAT) != -1) update_chat_tab_idx (old_idx, new_idx);
}

static void set_channel_tabs (const Uint32 *chans)
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

static void send_active_channel (Uint8 chan)
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
#define CHAT_WIN_TAG_SPACE	3
#define CHAT_WIN_TEXT_WIDTH  	500

static int CHAT_WIN_TAG_HEIGHT = 20;
static int CHAT_WIN_SCROLL_WIDTH = ELW_BOX_SIZE;
static int CLOSE_SIZE = ELW_BOX_SIZE;

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
//int highlight_tab_on_nick = 1;

////////////////////////////////////////////////////////////////////////
// Chat window variables

static int chat_scroll_id = 15;
static int chat_tabcollection_id = 20;
static int chat_out_start_id = 21;
static int chat_win_text_width = CHAT_WIN_TEXT_WIDTH;
static int chat_out_text_height = 0;
static int current_line = 0;
static int text_changed = 1;
static int nr_displayed_lines;
static int active_tab = -1;
static chan_name *tab_label (Uint8 chan);//Forward declaration

void clear_chat_wins (void)
{
	int chat_win = get_id_MW(MW_CHAT);
	int i = 0;
	if(use_windowed_chat != 2){return;}

	for (;i < MAX_CHAT_TABS; ++i){
		channels[i].nr_lines = 0;
	}

	vscrollbar_set_bar_len (chat_win, chat_scroll_id, 0);
	vscrollbar_set_pos (chat_win, chat_scroll_id, 0);
	current_line = 0;
	text_changed = 1;
}


void init_chat_channels(void)
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

void clear_input_line(void)
{
	input_text_line.data[0] = '\0';
	input_text_line.len = 0;
	if (input_widget != NULL)
	{
		text_field *field = input_widget->widget_info;
		field->cursor = 0;
		field->cursor_line = 0;
		field->nr_lines = 1;
		if (use_windowed_chat != 2)
		{
			int line_height = get_line_height(CHAT_FONT, input_widget->size);
			widget_resize(input_widget->window_id, input_widget->id,
				input_widget->len_x, 2*field->y_space + line_height);
		}
		/* Hide the game win input widget */
		if(input_widget->window_id == game_root_win)
			widget_set_flags(game_root_win, input_widget->id, INPUT_DEFAULT_FLAGS|WIDGET_DISABLED);
	}
	history_reset();
}

static int close_channel (window_info *win)
{
	int id = win->window_id;
	int ichan;

	for (ichan = 0; ichan < MAX_CHAT_TABS; ichan++)
	{
		if (channels[ichan].tab_id == id)
		{
			int idx = channels[ichan].chan_nr - CHAT_CHANNEL1;

			if (idx >= 0 && idx < MAX_ACTIVE_CHANNELS)
			{
				char str[256];
				safe_snprintf(str, sizeof(str), "%c#lc %d", RAW_TEXT, active_channels[idx]);
				my_tcp_send(my_socket, (Uint8*)str, strlen(str+1)+1);
			}

			// Safe to remove?
			if (tab_bar_win != -1) remove_tab_button(channels[ichan].chan_nr);

			return 1;
		}
	}

	// we shouldn't get here
	LOG_ERROR ("Trying to close non-existant channel\n");
	return 0;
}

static void remove_chat_tab (Uint8 channel)
{
	int chat_win = get_id_MW(MW_CHAT);
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

static int add_chat_tab(int nlines, Uint8 channel)
{
	int chat_win = get_id_MW(MW_CHAT);
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

			safe_strncpy(title,(tab_label(channel))->name, sizeof(title));

			channels[ichan].tab_id = tab_add (chat_win, chat_tabcollection_id, title, 0, 1, 0);
			set_window_flag (channels[ichan].tab_id, ELW_CLICK_TRANSPARENT);

			set_window_min_size (channels[ichan].tab_id, 0, 0);
			channels[ichan].out_id = text_field_add_extended(channels[ichan].tab_id,
				channels[ichan].out_id, NULL, 0, 0, inout_width, output_height, 0,
				CHAT_FONT, 1.0, display_text_buffer,
				DISPLAY_TEXT_BUFFER_SIZE, channel, CHAT_WIN_SPACE, CHAT_WIN_SPACE);

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

static void update_chat_tab_idx (Uint8 old_idx, Uint8 new_idx)
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

static Uint8 get_tab_channel (Uint8 channel)
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

static void update_chat_window (text_message *msg, char highlight)
{
	int chat_win = get_id_MW(MW_CHAT);
	int ichan, len, nlines, width, channel;
	char found;

	// don't bother if there's no chat window
	if (chat_win < 0) return;

	// rewrap message to get correct # of lines
	width = windows_list.window[chat_win].len_x;
	nlines = rewrap_message(msg, CHAT_FONT, 1.0, width, NULL);

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
			//Make sure we don't change the color of a highlighted tab
			else if (highlight && !channels[ichan].highlighted && channels[active_tab].chan_nr != CHAT_ALL &&
					channels[ichan].chan_nr != CHAT_ALL && !get_show_window_MW(MW_CONSOLE))
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

static int display_chat_handler (window_info *win)
{
	static int msg_start = 0, offset_start = 0;
	if (text_changed)
	{
		int line = vscrollbar_get_pos (get_id_MW(MW_CHAT), chat_scroll_id);

		find_line_nr(channels[active_tab].nr_lines, line, channels[active_tab].chan_nr,
			&msg_start, &offset_start, CHAT_FONT, 1.0, chat_win_text_width);
		text_field_set_buf_pos (channels[active_tab].tab_id, channels[active_tab].out_id, msg_start, offset_start);
		text_changed = 0;
	}

	if ((input_widget!= NULL) && (input_widget->window_id != win->window_id))
		input_widget_move_to_win(win->window_id);

	return 1;
}

static void switch_to_chat_tab(int id, char click)
{
	int chat_win = get_id_MW(MW_CHAT);
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

	if (channels[active_tab].chan_nr >= CHAT_CHANNEL1 && channels[active_tab].chan_nr <= CHAT_CHANNEL3) {
		send_active_channel (channels[active_tab].chan_nr);
		recolour_messages(display_text_buffer);
	}
}

static void change_to_current_chat_tab(const char *input)
{
	Uint8 channel;
	int ichan;
	int itab;
	int input_len = strlen(input);

	if(input[0] == '@' || input[0] == char_at_str[0])
	{
		channel = CHAT_CHANNEL1 + current_channel;
	}
	else if(!strncasecmp(input, "#gm ", 4) || (!strncasecmp(input, gm_cmd_str, strlen(gm_cmd_str)) && input_len > strlen(gm_cmd_str)+1 && input[strlen(gm_cmd_str)] == ' '))
	{
		channel = CHAT_GM;
	}
	else if(!strncasecmp(input, "#mod ", 5) || (!strncasecmp(input, mod_cmd_str, strlen(mod_cmd_str)) && input_len > strlen(mod_cmd_str)+1 && input[strlen(mod_cmd_str)] == ' '))
	{
		channel = CHAT_MOD;
	}
	else if(!strncasecmp(input, "#bc ", 4) || (!strncasecmp(input, bc_cmd_str, strlen(bc_cmd_str)) && input_len > strlen(bc_cmd_str)+1 && input[strlen(bc_cmd_str)] == ' '))
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

static int chat_tabs_click (widget_list *widget, int mx, int my, Uint32 flags)
{
	int id;

	id = tab_collection_get_tab_id (get_id_MW(MW_CHAT), widget->id);

	if (flags&ELW_RIGHT_MOUSE)
	{
		int i;
		for(i=0; i < MAX_CHAT_TABS; i++)
		{
			if(channels[i].tab_id == id)
			{
				display_channel_color_win(get_active_channel(channels[i].chan_nr));
				return 1;
			}
		}
	}
	else
	{
		if (id != channels[active_tab].tab_id)
		{
			//We're not looking at the tab we clicked
			switch_to_chat_tab(id, 1);
			return 1;
		}
	}
	return 0;
}

static int chat_scroll_drag (widget_list *widget, int mx, int my, Uint32 flags, int dx, int dy)
{
	int line = vscrollbar_get_pos (get_id_MW(MW_CHAT), widget->id);
	if (line != current_line)
	{
		text_changed = 1;
		current_line = line;
	}
        return 0;
}

static int chat_scroll_click (widget_list *widget, int mx, int my, Uint32 flags)
{
	int line = vscrollbar_get_pos (get_id_MW(MW_CHAT), widget->id);
	if (line != current_line)
	{
		text_changed = 1;
		current_line = line;
	}
        return 0;
}

int chat_input_key (widget_list *widget, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	text_field *tf;
	text_message *msg;

	if (widget == NULL) {
		return 0;
	}
	tf = (text_field *) widget->widget_info;
	msg = tf->buffer;

	if ( (!(key_mod & KMOD_CTRL) && ( (key_code == SDLK_UP) || (key_code == SDLK_DOWN) ) ) ||
		(key_code == SDLK_LEFT) || (key_code == SDLK_RIGHT) || (key_code == SDLK_HOME) ||
		(key_code == SDLK_END) || (key_code == SDLK_DELETE && tf->cursor < msg->len) ) {
		//pass it along. the defaults are good enough
		widget->Flags &= ~TEXT_FIELD_NO_KEYPRESS;
		text_field_keypress (widget, mx, my, key_code, key_unicode, key_mod);
		widget->Flags |= TEXT_FIELD_NO_KEYPRESS;
	}
	else
	{
		return 0;
	}
	/* Key was handled, stop blinking on input */
	tf->next_blink = cur_time + TF_BLINK_DELAY;
	return 1;
}

static int resize_chat_handler(window_info *win, int width, int height)
{
	int chat_win = get_id_MW(MW_CHAT);
	int itab;
	int line_height = get_line_height(CHAT_FONT, 1.0);
	int line_skip = get_line_skip(CHAT_FONT, 1.0);
	int scroll_x = width - CHAT_WIN_SCROLL_WIDTH;
	int scroll_height = height - 2*CLOSE_SIZE;
	int inout_width = width - CHAT_WIN_SCROLL_WIDTH - 2 * CHAT_WIN_SPACE;
	int input_height = line_height + 2 * line_skip + 2 * CHAT_WIN_SPACE;
	int input_y = height - input_height - CHAT_WIN_SPACE;
	int tabcol_height = input_y - 2 * CHAT_WIN_SPACE;
	int output_height = tabcol_height - CHAT_WIN_TAG_HEIGHT;

	if (output_height < line_height + 4*line_skip + 2 * CHAT_WIN_SPACE
		&& input_height > line_height + 2*line_skip + 2 * CHAT_WIN_SPACE)
	{
		input_height -= 2*line_skip;
		input_y += 2*line_skip;
		output_height += 2*line_skip;
		tabcol_height += 2*line_skip;
	}
	else if (output_height < line_height + 7*line_skip + 2 * CHAT_WIN_SPACE
		&& input_height > line_height + line_skip + 2 * CHAT_WIN_SPACE)
	{
		input_height -= line_skip;
		input_y += line_skip;
		output_height += line_skip;
		tabcol_height += line_skip;
	}

	chat_win_text_width = inout_width - 2 * CHAT_WIN_SPACE;
	chat_out_text_height = output_height - 2 * CHAT_WIN_SPACE;

	widget_resize (chat_win, chat_scroll_id, CHAT_WIN_SCROLL_WIDTH, scroll_height);
	widget_move (chat_win, chat_scroll_id, scroll_x, CLOSE_SIZE);

	widget_resize (chat_win, chat_tabcollection_id, inout_width, tabcol_height);

	for (itab = 0; itab < MAX_CHAT_TABS; itab++)
		if (channels[itab].tab_id >= 0)
			widget_resize (channels[itab].tab_id, channels[itab].out_id, inout_width, output_height);

	widget_resize (chat_win, input_widget->id, inout_width, input_height);
	widget_move (chat_win, input_widget->id, CHAT_WIN_SPACE, input_y);

	update_chat_win_buffers();

	return 0;
}


void update_chat_win_buffers(void)
{
	int chat_win = get_id_MW(MW_CHAT);
	int itab, imsg;
	// recompute line breaks
	for (itab = 0; itab < MAX_CHAT_TABS; itab++)
		channels[itab].nr_lines = 0;

	imsg = 0;
	while (1)
	{
		update_chat_window (&display_text_buffer[imsg], 0);
		if (imsg == last_message || last_message < 0)
			break;
		if (++imsg >= DISPLAY_TEXT_BUFFER_SIZE)
			imsg = 0;
	}

	// adjust the text position and scroll bar
	nr_displayed_lines = get_max_nr_lines(chat_out_text_height, CHAT_FONT, 1.0);
	current_line = channels[active_tab].nr_lines - nr_displayed_lines;
	if (current_line < 0)
		current_line = 0;
	vscrollbar_set_bar_len (chat_win, chat_scroll_id, current_line);
	vscrollbar_set_pos (chat_win, chat_scroll_id, current_line);
	text_changed = 1;
}

void parse_input(char *data, int len)
{
	if (len > MAX_TEXT_MESSAGE_LENGTH)
	{
		LOG_TO_CONSOLE(c_red2, command_too_long_str);
		return;
	}

	if (data[0] == '%' && len > 1)
	{
		if ( (check_var ((char*)&(data[1]), IN_GAME_VAR) ) < 0)
		{
			send_input_text_line ((char*)data, len);
		}
	}
	else if ( data[0] == '#' || data[0] == char_cmd_str[0] )
	{
		test_for_console_command ((char*)data, len);
	}
	else if (data[0] == '/' && len > 1)
	{
		// Forum #58898: Please the server when sending player messages;
		// remove all but one space between name and message start.
		// do not assume data is null terminated
		size_t dx = 0;
		char *rebuf = (char *)malloc(len+1);
		for (dx=0; (dx < len) && (data[dx] != ' '); dx++)
			rebuf[dx] = data[dx];
		rebuf[dx] = '\0';
		while ((dx < len) && (data[dx] == ' '))
			dx++;
		if (dx < len)
		{
			size_t rebuf_len = 0;
			safe_strcat(rebuf, " ", len+1);
			rebuf_len = strlen(rebuf);
			safe_strncpy2(&rebuf[rebuf_len], &data[dx], len+1-rebuf_len, len-dx);
		}
		send_input_text_line (rebuf, strlen(rebuf));
		free(rebuf);
	}
	else
	{
		if(data[0] == char_at_str[0])
			data[0] = '@';
		send_input_text_line ((char*)data, len);
	}
}


int root_key_to_input_field (SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	Uint8 ch = key_to_char (key_unicode);
	text_field *tf;
	text_message *msg;
	int alt_on = key_mod & KMOD_ALT, ctrl_on = key_mod & KMOD_CTRL;
	int do_reset_tab_completer = 1;

	if(input_widget == NULL || (input_widget->Flags & TEXT_FIELD_EDITABLE) == 0) {
		return 0;
	}

	tf = input_widget->widget_info;
	msg = &(tf->buffer[tf->msg]);

	if (key_code == SDLK_ESCAPE)
	{
		clear_input_line();
	}
	else if ((key_code == SDLK_RETURN || key_code == SDLK_KP_ENTER) && msg->len > 0)
	{
		parse_input(msg->data, msg->len);
		add_line_to_history((char*)msg->data, msg->len);
		clear_input_line();
	}
	else if (tf->cursor == 1 && (ch == '/' || ch == char_slash_str[0])
	             && (msg->data[0] == '/' || msg->data[0]==char_slash_str[0])
	             && last_pm_from[0])
	{
		// watch for the '//' shortcut
		tf->cursor += put_string_in_buffer (msg, (unsigned char*)last_pm_from, 1);
		tf->cursor += put_char_in_buffer (msg, ' ', tf->cursor);

		// set invalid width to force rewrap
		msg->wrap_width = 0;
		tf->nr_lines = rewrap_message(msg, input_widget->fcat, input_widget->size,
			input_widget->len_x - 2 * tf->x_space, &tf->cursor);
	}
	else if (key_code == SDLK_BACKSPACE || key_code == SDLK_DELETE
#ifdef OSX
		|| ch == 127
#endif
		|| (!alt_on && !ctrl_on && is_printable (ch) && ch != '`'))
	{
		if (is_printable (ch) && !get_show_window_MW(MW_TABMAP)) {
			//Make sure the widget is visible.
			widget_unset_flags (input_widget->window_id, input_widget->id, WIDGET_DISABLED);
			widget_unset_flags (input_widget->window_id, input_widget->id, WIDGET_INVISIBLE);
		}
		// XXX FIXME: we've set the input widget with the
		// TEXT_FIELD_NO_KEYPRESS flag so that the default key
		// handler for a text field isn't called, but now
		// we do want to call it directly. So we clear the flag,
		// and reset it afterwards.
		input_widget->Flags &= ~TEXT_FIELD_NO_KEYPRESS;
		text_field_keypress (input_widget, 0, 0, key_code, key_unicode, key_mod);
		input_widget->Flags |= TEXT_FIELD_NO_KEYPRESS;
	}
	else if (KEY_DEF_CMP(K_TABCOMPLETE, key_code, key_mod) && input_text_line.len > 0)
	{
		do_tab_complete(&input_text_line);
		do_reset_tab_completer = 0;
	}
	else if (get_show_window_MW(MW_CONSOLE))
	{
		if (!chat_input_key (input_widget, 0, 0, key_code, key_unicode, key_mod))
			return 0;
	}
	else
	{
		return 0;
	}

	if (do_reset_tab_completer)
		reset_tab_completer();

	tf->next_blink = cur_time + TF_BLINK_DELAY;
	if (input_widget->window_id != get_id_MW(MW_CHAT)
		&& tf->nr_lines != get_max_nr_lines(input_widget->len_y-2*tf->y_space, CHAT_FONT, 1.0))
	{
		/* Resize the input widget if needed */
		widget_resize(input_widget->window_id, input_widget->id,
			input_widget->len_x, tf->y_space*2 + get_text_height(tf->nr_lines, CHAT_FONT, 1.0));
	}
	while(tf->buffer->data[tf->cursor] == '\r' && tf->cursor < tf->buffer->len)
	{
		tf->cursor++;
	}
	return 1;
}

void paste_in_input_field (const Uint8 *text)
{
	text_field *tf;
	text_message *msg;

	if (input_widget == NULL) {
		return;
	} else if (input_widget->window_id == game_root_win) {
		widget_unset_flags (game_root_win, input_widget->id, WIDGET_DISABLED);
	}

	tf = input_widget->widget_info;
	msg = &(tf->buffer[tf->msg]);

	tf->cursor += put_string_in_buffer(msg, text, tf->cursor);

	// set invalid width to force rewrap
	msg->wrap_width = 0;
	tf->nr_lines = rewrap_message(msg, input_widget->fcat, input_widget->size,
		input_widget->len_x - 2 * tf->x_space, &tf->cursor);
	if (use_windowed_chat != 2)
	{
		widget_resize(input_widget->window_id, input_widget->id,
			input_widget->len_x, tf->y_space*2 + get_text_height(tf->nr_lines, CHAT_FONT, 1.0));
	}
}

void put_string_in_input_field(const Uint8 *text)
{
	text_field *tf = input_widget->widget_info;
	text_message *msg = &(tf->buffer[tf->msg]);

	if(text != NULL) {
		tf->cursor = msg->len = safe_snprintf((char*)msg->data, msg->size, "%s", text);
		// set invalid width to force rewrap
		msg->wrap_width = 0;
		tf->nr_lines = rewrap_message(msg, input_widget->fcat, input_widget->size,
			input_widget->len_x - 2 * tf->x_space, &tf->cursor);
		if (use_windowed_chat != 2)
		{
			widget_resize(input_widget->window_id, input_widget->id,
				input_widget->len_x, tf->y_space*2 + get_text_height(tf->nr_lines, CHAT_FONT, 1.0));
		}
		if(input_widget->window_id == game_root_win) {
			widget_unset_flags (input_widget->window_id, input_widget->id, WIDGET_DISABLED);
		}
	}
}

static int close_chat_handler (window_info *win)
{
	// if show and hide enabled, just moved to the hidden state
	if (enable_chat_show_hide)
	{
		chat_shown = 0;
		return 1;
	}

	// revert to using the tab bar
	// call the config function to make sure it's done properly
	change_windowed_chat(&use_windowed_chat, 1);
	set_var_unsaved("windowed_chat", INI_FILE_VAR);

	return 1;
}

static int ui_scale_chat_handler(window_info *win)
{
	int tab_tag_height = tab_collection_calc_tab_height(win->font_category,
		win->current_scale_small);
	widget_list *w = widget_find (win->window_id, chat_tabcollection_id);

	CHAT_WIN_TAG_HEIGHT = tab_tag_height;
	CHAT_WIN_SCROLL_WIDTH = (int)(0.5 + win->current_scale * ELW_BOX_SIZE);
	CLOSE_SIZE = win->box_size;

	widget_set_size(win->window_id, chat_tabcollection_id, win->current_scale_small);

	tab_collection_resize(w, win->len_x, win->len_y);
	tab_collection_move(w, win->pos_x + CHAT_WIN_SPACE, win->pos_y + tab_tag_height + CHAT_WIN_SPACE);

	resize_window(win->window_id, win->len_x, win->len_y);

	return 1;
}

static int change_chat_font_handler(window_info* win, font_cat cat)
{
	if (cat != CHAT_FONT)
		return 0;
	text_changed = 1;
	return 1;
}

static void create_chat_window(void)
{
	int chat_win_width = (int)(CHAT_WIN_TEXT_WIDTH * font_scales[CHAT_FONT]) + 4 * CHAT_WIN_SPACE + CHAT_WIN_SCROLL_WIDTH;
	int input_height = get_text_height(3, CHAT_FONT, 1.0) + 2 * CHAT_WIN_SPACE;
	int output_height = get_text_height(8, CHAT_FONT, 1.0) + 2 * CHAT_WIN_SPACE;
	int chat_win_height = output_height + input_height + 3 * CHAT_WIN_SPACE + CHAT_WIN_TAG_HEIGHT;
	int inout_width = CHAT_WIN_TEXT_WIDTH + 2 * CHAT_WIN_SPACE;
	int tabcol_height = output_height + CHAT_WIN_TAG_HEIGHT;
	int input_y = tabcol_height + 2 * CHAT_WIN_SPACE;
	int chat_win = -1;

	int min_width = chat_win_width * 0.5;
	int min_height = 7 * CHAT_WIN_SPACE + CHAT_WIN_TAG_HEIGHT + get_text_height(2, CHAT_FONT, 1.0)
		+ get_text_height(5, CHAT_FONT, 1.0);

	nr_displayed_lines = get_max_nr_lines(output_height - 2*CHAT_WIN_SPACE, CHAT_FONT, 1.0);
	chat_out_text_height = output_height - 2 * CHAT_WIN_SPACE;

	chat_win = create_window ("Chat", game_root_win, 0, get_pos_x_MW(MW_CHAT), get_pos_y_MW(MW_CHAT), chat_win_width, chat_win_height, ELW_USE_UISCALE|ELW_WIN_DEFAULT|ELW_RESIZEABLE|ELW_CLICK_TRANSPARENT);
	if (chat_win < 0 || chat_win >= windows_list.num_windows)
		return;
	set_id_MW(MW_CHAT, chat_win);
	set_window_custom_scale(chat_win, MW_CHAT);

	set_window_handler (chat_win, ELW_HANDLER_DISPLAY, &display_chat_handler);
	set_window_handler (chat_win, ELW_HANDLER_RESIZE, &resize_chat_handler);
	set_window_handler (chat_win, ELW_HANDLER_UI_SCALE, &ui_scale_chat_handler);
	set_window_handler (chat_win, ELW_HANDLER_CLOSE, &close_chat_handler);
	set_window_handler(chat_win, ELW_HANDLER_FONT_CHANGE, &change_chat_font_handler);

	chat_scroll_id = vscrollbar_add_extended (chat_win, chat_scroll_id, NULL, chat_win_width - CHAT_WIN_SCROLL_WIDTH, CLOSE_SIZE, CHAT_WIN_SCROLL_WIDTH, chat_win_height - 2*CLOSE_SIZE, 0, 1.0f, 0, 1, 0);
	widget_set_OnDrag (chat_win, chat_scroll_id, chat_scroll_drag);
	widget_set_OnClick (chat_win, chat_scroll_id, chat_scroll_click);

	chat_tabcollection_id = tab_collection_add_extended (chat_win, chat_tabcollection_id, NULL, CHAT_WIN_SPACE, CHAT_WIN_SPACE, inout_width, tabcol_height, 0, DEFAULT_SMALL_RATIO, MAX_CHAT_TABS);
	widget_set_OnClick (chat_win, chat_tabcollection_id, chat_tabs_click);

	channels[0].tab_id = tab_add (chat_win, chat_tabcollection_id, (tab_label(CHAT_ALL))->name, 0, 0, 0);
	set_window_flag (channels[0].tab_id, ELW_CLICK_TRANSPARENT);
	set_window_min_size (channels[0].tab_id, 0, 0);
	channels[0].out_id = text_field_add_extended(channels[0].tab_id, channels[0].out_id,
		NULL, 0, 0, inout_width, output_height, 0, CHAT_FONT, 1.0,
		display_text_buffer, DISPLAY_TEXT_BUFFER_SIZE, FILTER_ALL, CHAT_WIN_SPACE, CHAT_WIN_SPACE);
	channels[0].chan_nr = CHAT_ALL;
	channels[0].nr_lines = 0;
	channels[0].open = 1;
	channels[0].newchan = 0;
	active_tab = 0;

	if(input_widget == NULL) {
		Uint32 id;
		set_text_message_color (&input_text_line, 1.0f, 1.0f, 1.0f);
		id = text_field_add_extended (chat_win, 19, NULL, CHAT_WIN_SPACE, input_y,
			inout_width, input_height, TEXT_FIELD_BORDER|TEXT_FIELD_EDITABLE|TEXT_FIELD_NO_KEYPRESS,
			CHAT_FONT, 1.0, &input_text_line, 1, FILTER_ALL,
			CHAT_WIN_SPACE, CHAT_WIN_SPACE);
		widget_set_OnKey (chat_win, id, (int (*)())chat_input_key);
		input_widget = widget_find(chat_win, id);
	}
	set_window_min_size (chat_win, min_width, min_height);
	ui_scale_chat_handler(&windows_list.window[chat_win]);

	check_proportional_move(MW_CHAT);
}

void display_chat(void)
{
	int chat_win = get_id_MW(MW_CHAT);
	if (chat_win < 0)
	{
		create_chat_window ();
	}
	else
	{
		if(input_widget != NULL) {
			input_widget->OnResize = NULL;
		}
		show_window (chat_win);
		select_window (chat_win);
	}
	update_chat_win_buffers();
}

////////////////////////////////////////////////////////////////////////

#define CS_MAX_DISPLAY_CHANS 10

int tab_bar_win = -1;
static int chan_sel_win = -1;
static int chan_sel_scroll_id = -1;
static int chan_sel_border = 5;
static int chan_sel_sep_offset = 2;
static chat_tab tabs[MAX_CHAT_TABS];
static int cur_button_id = 0;
static int tabs_in_use = 0;
static int current_tab = 0;
static int tab_bar_width = 0;
static int tab_bar_height = 0;
static float tab_bar_zoom = 0.75;

static chan_name *create_chan_name(int no, const char* name, const char* desc)
{
	chan_name *entry = malloc(sizeof(*entry));
	if (!entry)
		return NULL;
	if ( !(entry->description = strdup(desc)) )
	{
		free(entry);
		return NULL;
	}
	if ( !(entry->name = strdup(name)) )
	{
		free(entry->description);
		free(entry);
		return NULL;
	}
	entry->channel = no;
	return entry;
}

static chan_name *add_chan_name(int no, const char * name, const char * desc)
{
	chan_name *entry = create_chan_name(no, name, desc);
	int len;

	if (!entry)
	{
		LOG_ERROR("Memory allocation error reading channel list");
		return NULL;
	}
	queue_push(chan_name_queue, entry);
	len = chan_name_queue->nodes-CS_MAX_DISPLAY_CHANS;
	if(len > 0 && chan_sel_scroll_id == -1 && chan_sel_win != -1) {
		chan_sel_scroll_id = vscrollbar_add_extended (chan_sel_win, 0, NULL, 165, 20, 20, 163, 0, 1.0, 0, 1, len);
	}

	return entry;
}

static void add_spec_chan_name(int no, const char* name, const char* desc)
{
	chan_name *entry = create_chan_name(no, name, desc);
	if (entry)
		pseudo_chans[no] = entry;
	else
		LOG_ERROR("Memory allocation error reading channel list");
}

static void generic_chans(void)
{	//the channel list file is missing. We'll use hard-coded values
	//remake the queue, just in case we got half way through the file
	queue_destroy(chan_name_queue);
	queue_initialise(&chan_name_queue);
	add_spec_chan_name(0, "Channel %d", "Channel %d");
	add_spec_chan_name(1, "Guild", "Your guild's chat channel");
	add_spec_chan_name(2, "All", "Display chat in all channels");
	add_spec_chan_name(3, "None", "Messages not on any channel");
	add_spec_chan_name(4, "Options", "Select which channels to join");
	add_spec_chan_name(5, "History", "View all previous chat in all channels you have been on");
	add_spec_chan_name(6, "Local", "Chat in your local area");
	add_spec_chan_name(7, "PMs", "Private messages");
	add_spec_chan_name(8, "GMs", "Guild Messages");
	add_spec_chan_name(9, "Server", "Messages from the server");
	add_spec_chan_name(10, "Mod", "Mod chat");
	add_chan_name(1, "Newbie", "Newbie Q&A about the game");
	add_chan_name(3, "Market", "Trading, hiring, and price checks");
	add_chan_name(4, "EL Gen Chat", "Chat about EL topics");
	add_chan_name(5, "Roleplay", "Discussion about, and Roleplaying");
	add_chan_name(6, "Contests", "Contest information and sometimes chat");
}

void init_channel_names(void)
{
	char file[256];
	xmlDocPtr doc;
	xmlNodePtr cur;

	// Per-channel info
	char *channelname;
	char *channeldesc;
	int channelno;

	// Temp info
	xmlChar *attrib;

	queue_initialise(&chan_name_queue);

	// Load the file, depending on WINDOWS = def|undef
	// Then parse it. If that fails, fallback onto the english one. If that fails, use builtins.
	safe_snprintf (file, sizeof (file), "languages/%s/strings/channels.xml", lang);

	doc = xmlParseFile (file);
	if (doc == NULL ) {
		doc = xmlParseFile("languages/en/strings/channels.xml");
		if (doc == NULL) { //darn, don't have that either?
			LOG_ERROR (using_builtin_chanlist);
			generic_chans();
			return;
		}
		//well the localised version didn't load, but the 'en' version did
		LOG_ERROR (using_eng_chanlist, lang);
	}

	// Get the root element, if it exists.
	cur = xmlDocGetRootElement (doc);
	if (cur == NULL) {
		// Use generics. Defaulting to english, then using the fallbacks makes obfuscated, messy code.
		LOG_ERROR (using_builtin_chanlist);
		generic_chans();
		xmlFreeDoc(doc);
		return;
	}

	// Check the root element.
	if (xmlStrcasecmp (cur->name, (const xmlChar *) "CHANNELS")) {
		LOG_ERROR (xml_bad_root_node, file);
		xmlFreeDoc(doc);
		generic_chans();
		return;
	}

	// Load first child node
	cur = cur->xmlChildrenNode;

	// Loop while we have a node, copying ATTRIBS, etc
	while (cur != NULL)	{
		if(cur->type != XML_ELEMENT_NODE) {
			/* NO-OP. better performance to check now than later */
		} else if ((!xmlStrcmp (cur->name, (const xmlChar *)"label"))) {
			// Get the name.
			attrib = xmlGetProp (cur, (xmlChar*)"name");
			if (attrib == NULL) {
				LOG_ERROR (xml_bad_node);
				xmlFree (attrib);
				continue;
			}
			if (xmlStrlen(attrib) < 1) {
				LOG_ERROR (xml_bad_node);
				xmlFree (attrib);
				continue;
			}
			channelname = (char*)xmlStrdup(attrib);
			xmlFree (attrib);

			// Get the index number
			attrib = xmlGetProp (cur, (xmlChar*)"index");
			if (attrib == NULL) {
				LOG_ERROR (xml_bad_node);
				xmlFree (attrib);
				continue;
			}
			if (xmlStrlen(attrib) < 1) {
				LOG_ERROR (xml_bad_node);
				xmlFree (attrib);
				continue;
			}
			channelno = atoi ((char*)attrib);
			xmlFree (attrib);

			// Get the description.
			if ((cur->children == NULL) || (strlen ((char*)cur->children->content) < 1)) {
				free (channelname);
				LOG_ERROR (xml_bad_node);
				continue;
			}
			attrib = cur->children->content;
			channeldesc = (char*)xmlStrdup(attrib);

			// Add it.
			add_spec_chan_name(channelno, channelname, channeldesc);
			free(channelname);
			free(channeldesc);
		} else if ((!xmlStrcmp (cur->name, (const xmlChar *)"channel"))) {
			// Get the channel.
			attrib = xmlGetProp (cur, (xmlChar*)"number");
			if (attrib == NULL){
				LOG_ERROR (xml_bad_node);
				xmlFree (attrib);
				continue;
			}
			if (xmlStrlen(attrib) < 1){
				LOG_ERROR (xml_bad_node);
				xmlFree (attrib);
				continue;
			}
			channelno = atoi ((char*)attrib);
			xmlFree (attrib);

			// Get the name.
			attrib = xmlGetProp (cur, (xmlChar*)"name");
			if (attrib == NULL){
				LOG_ERROR (xml_bad_node);
				xmlFree (attrib);
				continue;
			}
			if (xmlStrlen(attrib) < 1){
				LOG_ERROR (xml_bad_node);
				xmlFree (attrib);
				continue;
			}
			channelname = (char*)xmlStrdup(attrib);
			xmlFree (attrib);

			// Get the description.
			if (cur->children == NULL) {
				free (channelname);
				LOG_ERROR (xml_bad_node);
				continue;
			} else if (strlen ((char*)cur->children->content) < 1) {
				free (channelname);
				LOG_ERROR (xml_bad_node);
				continue;
			}
			attrib = cur->children->content;
			channeldesc = (char*)xmlStrdup(attrib);

			// Add it.
			add_chan_name(channelno, channelname, channeldesc);
			free(channelname);
			free(channeldesc);
		} else {
			LOG_ERROR (xml_undefined_node, file, (cur->name != NULL && strlen((char*)cur->name) < 100) ? cur->name	: (const xmlChar *)"not a string");
		}
		cur = cur->next;         // Advance to the next node.
	}
	if(queue_isempty(chan_name_queue)) {
		//how did we not get any channels from it?
		LOG_ERROR(using_builtin_chanlist);
		generic_chans();
	}
	xmlFreeDoc(doc);
}

void cleanup_chan_names(void)
{
	//don't call this command unless the client is closing
	int i=0;
	node_t *temp_node, *step = queue_front_node(chan_name_queue);
	chan_name *temp_cn;
	for (;i<SPEC_CHANS;++i) {
		if(pseudo_chans[i] == NULL) {
			continue;
		}
		if(pseudo_chans[i]->name != NULL) {
			free(pseudo_chans[i]->name);
		}
		if(pseudo_chans[i]->description != NULL) {
			free(pseudo_chans[i]->description);
		}
		free(pseudo_chans[i]);
	}
	while(step != NULL) {
		temp_node = step;
		step = step->next;
		temp_cn = queue_delete_node(chan_name_queue, temp_node);
		if(temp_cn == NULL || temp_cn->name == NULL || strlen(temp_cn->name) < 1) {
			continue;
		}
		if(temp_cn->name != NULL) {
			free(temp_cn->name);
		}
		if(temp_cn->description != NULL) {
			free(temp_cn->description);
		}
		free(temp_cn);
	}
	queue_destroy(chan_name_queue);
}

// Not called from anywhere, I wonder what happened?  My may to restore one day.....
//
//int highlight_tab(const Uint8 channel)
//{
//	int i;
//
//	if(!highlight_tab_on_nick || channel == CHAT_ALL)
//	{
//		//We don't want to highlight
//		return 0;
//	}
//	switch(use_windowed_chat)
//	{
//		case 1:
//			if (tab_bar_win < 0)
//			{
//				//it doesn't exist
//				return 0;
//			}
//			if(tabs[current_tab].channel != CHAT_ALL) {
//				/* If we're in the All tab, we have already seen this message */
//				for (i = 0; i < tabs_in_use; i++)
//				{
//					if (tabs[i].channel == channel)
//					{
//						if (current_tab != i && !tabs[i].highlighted)
//						{
//							widget_set_color (tab_bar_win, tabs[i].button, 1.0f, 0.0f, 0.0f);
//							tabs[i].highlighted = 1;
//						}
//						break;
//					}
//				}
//			}
//		break;
//		case 2:
//			if (chat_win < 0)
//			{
//				//it doesn't exist
//				return 0;
//			}
//			if(channels[active_tab].chan_nr != CHAT_ALL) {
//				/* If we're in the All tab, we have already seen this message */
//				for (i = 0; i < MAX_CHAT_TABS; i++)
//				{
//					if (channels[i].open && channels[i].chan_nr == channel)
//					{
//						if (i != active_tab && !channels[i].highlighted)
//						{
//							tab_set_label_color_by_id (chat_win, chat_tabcollection_id, channels[i].tab_id, 1.0, 0.0, 0.0);
//							channels[i].highlighted = 1;
//						}
//						break;
//					}
//				}
//			}
//		break;
//		default:
//			return 0;
//		break;
//	}
//	return 1;
//}

static void switch_to_tab(int id)
{
	int i=2;
	widget_set_color (tab_bar_win, tabs[current_tab].button, gui_color[0], gui_color[1], gui_color[2]);
	widget_set_color (tab_bar_win, tabs[0].button,  0.5f, 0.75f, 1.0f);
	widget_set_color (tab_bar_win, tabs[1].button,  0.5f, 0.75f, 1.0f);
	for(;i < MAX_CHAT_TABS; ++i) {
		if(tabs[i].button <= 0) {
			continue;
		} else if(tabs[i].highlighted) {
			continue;
		}
		widget_set_color (tab_bar_win, tabs[i].button, gui_color[0], gui_color[1], gui_color[2]);
	}
	current_tab = id;
	widget_set_color (tab_bar_win, tabs[current_tab].button, 0.57f, 1.0f, 0.59f);
	current_filter = tabs[current_tab].channel;
	tabs[current_tab].highlighted = 0;
	if(tabs[current_tab].channel >= CHAT_CHANNEL1 && tabs[current_tab].channel <= CHAT_CHANNEL3) {
		send_active_channel (tabs[current_tab].channel);
		recolour_messages(display_text_buffer);
	}
}

static int tab_bar_button_click (widget_list *w, int mx, int my, Uint32 flags)
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

	if (flags&ELW_RIGHT_MOUSE)
	{
		display_channel_color_win(get_active_channel(tabs[itab].channel));
		return 1;
	}
	else
	{
		// NOTE: This is an optimization, instead of redefining a "Tab/Button" type.
		//		 Further use of this would be best served be a new definition.
		// Detect clicking on 'x'
		if(tabs[itab].channel == CHAT_CHANNEL1 || tabs[itab].channel == CHAT_CHANNEL2 ||
		   tabs[itab].channel == CHAT_CHANNEL3)
		{
			int half_len = (int)(0.5 + w->size * tab_control_half_len);
			int x = w->len_x - (half_len + (int)(0.5 + w->size * tab_control_border_sep));
			int y = (int)(0.5 + w->size * tab_control_border_sep) + half_len;
			char str[256];

			// 'x' was clicked?
			if(mx > x - half_len && mx < x + half_len && my > y - half_len && my < y + half_len)
			{
				// Drop this channel via #lc
				safe_snprintf(str, sizeof(str), "%c#lc %d", RAW_TEXT, active_channels[tabs[itab].channel-CHAT_CHANNEL1]);
				my_tcp_send(my_socket, (Uint8*)str, strlen(str+1)+1);
				// Can I remove this?
				remove_tab(tabs[itab].channel);
				if(current_tab == itab) {
					int i;
					//We're closing the current tab, switch to the all-tab
					for(i = 0; i < tabs_in_use; i++)
					{
						if(tabs[i].channel == CHAT_ALL)
						{
							switch_to_tab(i);
							break;
						}
					}
				}
				do_click_sound();
				return 1; //The click was handled, no need to continue
			}
		}


		if (current_tab != itab)
		{
			switch_to_tab(itab);
			do_click_sound();
		}
		lines_to_show = max_chat_lines.value;
	}
	return 1;
}

static chan_name *tab_label (Uint8 chan)
{
	//return pointer after stepping through chan_name_queue
	int cnr, steps=0;
	node_t *step = queue_front_node(chan_name_queue);
	char name[255];
	char desc[255];
	chan_name *res;

	switch (chan)
	{
		case CHAT_ALL:	return pseudo_chans[2];
		case CHAT_NONE:	return pseudo_chans[3];
		case CHAT_LIST:	return pseudo_chans[4];
		case CHAT_HIST:	return pseudo_chans[5];
		case CHAT_LOCAL:	return pseudo_chans[6];
		case CHAT_PERSONAL:	return pseudo_chans[7];
		case CHAT_GM:	return pseudo_chans[8];
		case CHAT_SERVER:	return pseudo_chans[9];
		case CHAT_MOD:	return pseudo_chans[10];
	}
	if(chan < CHAT_CHANNEL1 || chan > CHAT_CHANNEL3 ){
		// shouldn't get here...
		return NULL;
	}
	cnr = active_channels[chan-CHAT_CHANNEL1];
	if(cnr >= 1000000000){//ooh, guild channel!
		return pseudo_chans[1];
	}
	if(step == NULL){
		//say what? we don't know _any_ channels? something is very wrong...
		return NULL;
	}
	for (; step != NULL && step->data != NULL; step = step->next, steps++){
		if(((chan_name*)(step->data))->channel == cnr){
			return step->data;
		}
		if(step->next == NULL){
			break;
		}
	}
	//we didn't find it, so we use the generic version
	safe_snprintf(name, sizeof(name), pseudo_chans[0]->name, cnr);
	safe_snprintf(desc, sizeof(desc), pseudo_chans[0]->description, cnr);
	res = add_chan_name(cnr, name, desc);

	if (chan_sel_scroll_id >= 0 && steps > 8) {
		vscrollbar_set_bar_len(chan_sel_win, chan_sel_scroll_id, steps-8);
		//we're adding another name to the queue, so the window scrollbar needs to be adusted
	}

	return res;
}

static unsigned int chan_int_from_name(char * name, int * return_length)
{
	node_t *step = queue_front_node(chan_name_queue);
	char * cname = name;

	while(*cname && isspace(*cname)) {	//should there be a space at the front,
		cname++;						//we can handle that.
	}
	while(step->next != NULL) {
		step = step->next;
		if(!strncasecmp(((chan_name*)(step->data))->name, cname, strlen(((chan_name*)(step->data))->name))) {
			if(return_length != NULL) {
				*return_length = strlen(((chan_name*)(step->data))->name);
			}
			return ((chan_name*)(step->data))->channel;
		}
	}
	return 0;
}

static void update_tab_button_idx (Uint8 old_idx, Uint8 new_idx)
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

static int chan_tab_mouseover_handler(widget_list *widget)
{
	int itab = 0;
	if(!show_help_text){return 0;}
	for (itab = 0; itab < tabs_in_use; itab++){
		if ((tabs[itab].button) == (widget->id)){
			window_info *win = &windows_list.window[widget->window_id];
			show_help(tabs[itab].description, widget->pos_x,widget->pos_y+widget->len_y+2, win->current_scale);
			return 1;
		}
	}
	return 0;
}

static int display_chan_sel_handler(window_info *win)
{
	int i = 0, y = chan_sel_border, x = chan_sel_border, t = 0, num_lines = 0;
	int line_height;
	float local_zoom = win->current_scale_small;

	node_t *step = queue_front_node(chan_name_queue);
	if(mouse_x >= win->pos_x+win->len_x || mouse_y >= win->pos_y+win->len_y) {
		win->displayed = 0;
		return 0;//auto close when you mouseout
	}
	t = vscrollbar_get_pos(chan_sel_win, chan_sel_scroll_id);
	if(t > 0) {
		for (; i<t ; ++i) {
			if(step->next == NULL) {
				break;
			}
			step = step->next;
		}
	}
	for (i = 0; i < CS_MAX_DISPLAY_CHANS; ++i)
	{
		const unsigned char* name = (const unsigned char*)((chan_name*)(step->data))->name;
		int width = get_string_width_zoom(name, win->font_category, local_zoom);

		glColor3f(0.5f, 0.75f, 1.0f);
		draw_string_zoomed(x, y, name, 1, local_zoom);
		if (mouse_y > win->pos_y + y && mouse_y < win->pos_y + y + win->small_font_len_y
			&& mouse_x >= win->pos_x + chan_sel_border
			&& mouse_x <= win->pos_x + chan_sel_border + width)
		{
			show_help(((chan_name*)(step->data))->description, mouse_x-win->pos_x,mouse_y-win->pos_y-win->small_font_len_y, win->current_scale);
		}
		y += win->default_font_len_y;
		step = step->next;
		if(step == NULL) {
			y += (win->default_font_len_y*(CS_MAX_DISPLAY_CHANS-i-1));
			break;
		}
	}
	glDisable(GL_TEXTURE_2D);
	glColor3fv(gui_color);
	glBegin(GL_LINES);
		glVertex2i(0, y - chan_sel_sep_offset);
		glVertex2i(win->len_x, y - chan_sel_sep_offset);
	glEnd();
	glEnable(GL_TEXTURE_2D);

	line_height = get_line_height(UI_FONT, local_zoom);
	num_lines = reset_soft_breaks((unsigned char*)channel_help_str, strlen(channel_help_str),
		sizeof(channel_help_str), UI_FONT, local_zoom, win->len_x - chan_sel_border * 2,
		NULL, NULL);
	draw_string_zoomed(x, y+=chan_sel_border, (unsigned char*)channel_help_str,
		num_lines, local_zoom);
	win->len_y = win->default_font_len_y * CS_MAX_DISPLAY_CHANS
		+ chan_sel_border
		+ num_lines * line_height
		+ chan_sel_border - chan_sel_sep_offset;
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 0;
}

static int click_chan_sel_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int i = 0, y = my - chan_sel_border;
	node_t *step = queue_front_node(chan_name_queue);
	y /= win->default_font_len_y;
	i = vscrollbar_get_pos(chan_sel_win, chan_sel_scroll_id);
	if(i>0){
		y+=i;
	}

	if(flags&ELW_WHEEL_UP) {
		vscrollbar_scroll_up(chan_sel_win, chan_sel_scroll_id);
	} else if(flags&ELW_WHEEL_DOWN) {
		vscrollbar_scroll_down(chan_sel_win, chan_sel_scroll_id);
	} else {
		int width;
		for (i = 0; i < y ; ++i) {
			if(step->next == NULL) {
				return 0;
			}
			step = step->next;
		}
		width = get_string_width_zoom((const unsigned char*)((chan_name*)(step->data))->name,
			win->font_category, win->current_scale_small);
		if (mouse_x >= win->pos_x + chan_sel_border
			&& mouse_x <= win->pos_x + chan_sel_border + width)
		{
			char tmp[20];
			safe_snprintf(tmp, sizeof(tmp), "#jc %d", ((chan_name*)(step->data))->channel);
			send_input_text_line(tmp, strlen(tmp));
			do_click_sound();
		}
	}
	return 1;
}

static int ui_scale_chan_sel_handler(window_info *win)
{
	int len_x = (int)(0.5 + win->small_font_max_len_x * 20 + 2 * chan_sel_border + win->box_size);
	int len_y = (int)(0.5 + win->default_font_len_y * CS_MAX_DISPLAY_CHANS + chan_sel_border);

	resize_window(win->window_id, len_x, len_y);
	widget_resize(win->window_id, chan_sel_scroll_id, win->box_size, len_y - win->box_size - chan_sel_sep_offset);
	widget_move(win->window_id, chan_sel_scroll_id, len_x - win->box_size, win->box_size);

	return 1;
}

static int change_chan_sel_font_handler(window_info *win, font_cat cat)
{
	if (cat != UI_FONT)
		return 0;
	ui_scale_chan_sel_handler(win);
	return 1;
}

static int tab_special_click(widget_list *w, int mx, int my, Uint32 flags)
{
	int itab = 0;
	for (itab = 0; itab < tabs_in_use; itab++) {
		if (tabs[itab].button == w->id){
			switch(tabs[itab].channel) {
				case CHAT_HIST:
					toggle_window(game_root_win);
					toggle_window_MW(MW_CONSOLE);
					do_click_sound();
					break;
				case CHAT_LIST:
					if(chan_sel_win >= 0) {
						toggle_window(chan_sel_win);
					} else {
						chan_sel_win = create_window ("Channel Selection", tab_bar_win, 0, w->pos_x,w->pos_y+w->len_y+1, 100, 0, (ELW_USE_UISCALE|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW|ELW_ALPHA_BORDER|ELW_CLOSE_BOX));
						windows_list.window[chan_sel_win].back_color[3]= 0.25f;
						set_window_handler (chan_sel_win, ELW_HANDLER_DISPLAY, &display_chan_sel_handler);
						set_window_handler (chan_sel_win, ELW_HANDLER_CLICK, &click_chan_sel_handler);
						set_window_handler (chan_sel_win, ELW_HANDLER_UI_SCALE, &ui_scale_chan_sel_handler);
						set_window_handler(chan_sel_win, ELW_HANDLER_FONT_CHANGE, &change_chan_sel_font_handler);
						if(chan_name_queue->nodes >= CS_MAX_DISPLAY_CHANS && chan_sel_scroll_id == -1) {
							int len = chan_name_queue->nodes-CS_MAX_DISPLAY_CHANS;
							chan_sel_scroll_id = vscrollbar_add_extended (chan_sel_win, 0, NULL, 0, 0, 0, 0, 0, 1.0, 0, 1, len);
						}
						if (chan_sel_win >= 0 && chan_sel_win < windows_list.num_windows)
							ui_scale_chan_sel_handler(&windows_list.window[chan_sel_win]);
					}
					do_click_sound();
					break;
				default:
					return tab_bar_button_click(w, mx, my, flags);	//grumble. this shouldn't happen
			}
			return 1;
		}
	}
	return 0;
}


// Draw details on the channel buttons.

static int draw_tab_details (widget_list *W)
{
	int underline_sep = (int)(0.5 + W->size * tab_control_underline_sep);
	int half_len = (int)(0.5 + W->size * tab_control_half_len);
	int itab;

	glColor3fv(gui_color);

	glDisable(GL_TEXTURE_2D);

	/* check for an active "#jc" channel */
	for (itab = 0; itab < tabs_in_use; itab++)
		if ((tabs[itab].button == W->id) && (tabs[itab].channel == CHAT_CHANNEL1 + current_channel))
		{
			int plus_x = W->pos_x + (int)(0.5 + W->size * tab_control_border_sep) + half_len;
			int plus_y = W->pos_y + (int)(0.5 + W->size * tab_control_border_sep) + half_len;
			int i, color;
			/* draw the "+" for the active channel */
			for(i=0; i<MAX_CHANNEL_COLORS; i++)
			{
				if(channel_colors[i].nr == get_active_channel(tabs[itab].channel) && channel_colors[i].color > -1)
					break;
			}
			if(i < MAX_CHANNEL_COLORS)
			{
				color = channel_colors[i].color;
				glColor3ub(colors_list[color].r1, colors_list[color].g1, colors_list[color].b1);
			}
			glBegin(GL_QUADS);
				glVertex2i(plus_x - half_len, plus_y - 1);
				glVertex2i(plus_x + half_len, plus_y - 1);
				glVertex2i(plus_x + half_len, plus_y + 1);
				glVertex2i(plus_x - half_len, plus_y + 1);
				glVertex2i(plus_x - 1, plus_y - half_len);
				glVertex2i(plus_x + 1, plus_y - half_len);
				glVertex2i(plus_x + 1, plus_y + half_len);
				glVertex2i(plus_x - 1, plus_y + half_len);
			glEnd();
			glColor3fv(gui_color);
			/* draw a dotted underline if input would go to this channel */
			if ((input_text_line.len > 0) && (input_text_line.data[0] == '@') && !((input_text_line.len > 1) && (input_text_line.data[1] == '@')))
			{
				glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
				glEnable(GL_LINE_STIPPLE);
				glLineStipple(1, 0xCCCC);
				glLineWidth(3.0);
				glBegin(GL_LINES);
					glVertex2i(W->pos_x, W->pos_y + W->len_y + underline_sep);
					glVertex2i(W->pos_x + W->len_x, W->pos_y + W->len_y + underline_sep);
				glEnd();
				glPopAttrib();
			}
			break;
		}

	/* draw the closing x */
	draw_cross(W->pos_x + W->len_x - (half_len + (int)(0.5 + W->size * tab_control_border_sep)),
		W->pos_y + half_len + (int)(0.5 + W->size * tab_control_border_sep), half_len, 1);
	glEnable(GL_TEXTURE_2D);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;
}


static int add_tab_button (Uint8 channel)
{
	int itab;
	const char *label;
	chan_name *chan;

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
	chan = tab_label (channel);
	if(chan == NULL){
		return -1;
	}
	label = chan->name;
	tabs[tabs_in_use].description = chan->description;

	tabs[tabs_in_use].button = button_add_extended(tab_bar_win, cur_button_id++, NULL, tab_bar_width, 0, 0, tab_bar_height, BUTTON_SQUARE, tab_bar_zoom, label);
	if(channel == CHAT_HIST || channel == CHAT_LIST) {
		//a couple of special cases
		widget_set_OnClick (tab_bar_win, tabs[tabs_in_use].button, tab_special_click);
		widget_set_color (tab_bar_win, tabs[tabs_in_use].button, 0.5f, 0.75f, 1.0f);
	} else {
		//general case
		widget_set_OnClick (tab_bar_win, tabs[tabs_in_use].button, tab_bar_button_click);
	}
	widget_set_OnMouseover (tab_bar_win, tabs[tabs_in_use].button, chan_tab_mouseover_handler);
 	// Handlers for the 'x'
 	// Make sure it's a CHANNEL first
	if(tabs[tabs_in_use].channel == CHAT_CHANNEL1 || tabs[tabs_in_use].channel == CHAT_CHANNEL2 ||
		tabs[tabs_in_use].channel == CHAT_CHANNEL3)
 	{
 		widget_set_OnDraw (tab_bar_win, tabs[tabs_in_use].button, draw_tab_details);
 	}
	tab_bar_width += widget_get_width (tab_bar_win, tabs[tabs_in_use].button)+1;
	resize_window (tab_bar_win, tab_bar_width, tab_bar_height);

	tabs_in_use++;
	return tabs_in_use - 1;
}

static void remove_tab_button (Uint8 channel)
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

static void update_tab_bar (text_message * msg)
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
		lines_to_show += rewrap_message(msg, CHAT_FONT, 1.0,
			get_console_text_width(), NULL);
		if (lines_to_show >= max_chat_lines.value) lines_to_show = max_chat_lines.value;
		return;
	}

	if (tabs[current_tab].channel == CHAT_ALL) {
		lines_to_show += rewrap_message(msg, CHAT_FONT, 1.0,
			get_console_text_width(), NULL);
		if (lines_to_show >= max_chat_lines.value) lines_to_show = max_chat_lines.value;
	}

	for (itab = 2; itab < tabs_in_use; itab++)
	{
		if (tabs[itab].channel == channel)
		{
			if (current_tab != itab && !tabs[itab].highlighted && tabs[current_tab].channel != CHAT_ALL && !get_show_window_MW(MW_CONSOLE))
				widget_set_color (tab_bar_win, tabs[itab].button, 1.0f, 1.0f, 0.0f);
			if (current_tab == itab) {
				lines_to_show += rewrap_message(msg, CHAT_FONT, 1.0,
					get_console_text_width(), NULL);
				if (lines_to_show >= max_chat_lines.value) lines_to_show = max_chat_lines.value;
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

static int ui_scale_tab_bar_handler(window_info *win)
{
	size_t i;

	tab_bar_height = win->default_font_len_y;
	tab_bar_zoom = win->current_scale * 0.75;
	tab_bar_width = 0;

	for (i=0; i<tabs_in_use; i++)
	{
		button_resize(win->window_id, tabs[i].button, 0, tab_bar_height, tab_bar_zoom);
		widget_move(win->window_id, tabs[i].button, tab_bar_width, 0);
		tab_bar_width += 1 + widget_get_width(win->window_id, tabs[i].button);
	}
	resize_window(win->window_id, tab_bar_width, tab_bar_height);

	move_window(chan_sel_win, win->pos_id, win->pos_loc, win->pos_x, win->pos_y + win->len_y + 1);

	return 1;
}

static int change_tab_bar_font_handler(window_info *win, font_cat cat)
{
	if (cat != UI_FONT)
		return 0;
	ui_scale_tab_bar_handler(win);
	return 1;
}

static void create_tab_bar(void)
{
	int tab_bar_x = 10;
	int tab_bar_y = 3;

	tab_bar_win = create_window ("Tab bar", -1, 0, tab_bar_x, tab_bar_y, 100, 0, ELW_USE_UISCALE|ELW_USE_BACKGROUND|ELW_SHOW);
	set_window_handler(tab_bar_win, ELW_HANDLER_UI_SCALE, &ui_scale_tab_bar_handler );
	set_window_handler(tab_bar_win, ELW_HANDLER_FONT_CHANGE, &change_tab_bar_font_handler);
	if (tab_bar_win >= 0 && tab_bar_win < windows_list.num_windows)
		ui_scale_tab_bar_handler(&windows_list.window[tab_bar_win]);

	add_tab_button (CHAT_LIST);
	add_tab_button (CHAT_HIST);
	add_tab_button (CHAT_ALL);
	//add_tab_button (CHAT_NONE);
	current_tab = 2;
	widget_set_color (tab_bar_win, tabs[current_tab].button, 0.57f, 1.0f, 0.59f);
	current_filter = tabs[current_tab].channel;
}

void display_tab_bar(void)
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

static void change_to_current_tab(const char *input)
{
	Uint8 channel;
	int itab;
	int input_len = strlen(input);

	if(input[0] == '@' || input[0] == char_at_str[0])
	{
		channel = CHAT_CHANNEL1 + current_channel;
	}
	else if(!strncasecmp(input, "#gm ", 4) || (!strncasecmp(input, gm_cmd_str,strlen(gm_cmd_str)) && input_len > strlen(gm_cmd_str)+1 && input[strlen(gm_cmd_str)] == ' '))
	{
		channel = CHAT_GM;
	}
	else if(!strncasecmp(input, "#mod ", 5) || (!strncasecmp(input, mod_cmd_str, strlen(mod_cmd_str)) && input_len > strlen(mod_cmd_str)+1 && input[strlen(mod_cmd_str)] == ' '))
	{
		channel = CHAT_MOD;
	}
	else if(!strncasecmp(input, "#bc ", 4) || (!strncasecmp(input, bc_cmd_str, strlen(bc_cmd_str)) && input_len > strlen(bc_cmd_str)+1 && input[strlen(bc_cmd_str)] == ' '))
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
		if (itab >= 0)
			switch_to_tab (itab);
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
		for (ibc = 2; ibc < tabs_in_use; ibc++)
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
		for (iwc = 2; iwc < MAX_CHAT_TABS; iwc++)
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
		for (iwc = 2; iwc < MAX_CHAT_TABS; iwc++)
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
		for (ibc = 2; ibc < tabs_in_use; ibc++)
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

int command_jlc(char * text, int len)
{
	unsigned int num;
	char number[12];

	num = chan_int_from_name(text, NULL);
	if(num <= 0) {
		return 0;//Don't know this name
	}
	safe_snprintf(number, sizeof(number), " %d", num);
	if(strlen(number) <= strlen(text)) { //it is entirely possible that the number
		safe_strncpy(text, number, strlen(text));	//could be longer than the name, and hence we may
	}							//not have enough storage space to replace the name
	return 0; //note: this change could also put us over the 160-char limit if not checked
}

////////////////////////////////////////////////////////////////////////
//  channel color stuff

#define COLROWS 7
#define COLCOLS 4

static Uint32 channel_to_change = 0;
static int selected_color = -1;
static int channel_colors_set = 0;
static int channel_color_win = -1;
static int color_set_button_id = COLROWS * COLCOLS;
static int color_delete_button_id = COLROWS * COLCOLS + 1;
static int color_label_id = COLROWS * COLCOLS + 2;

static int display_channel_color_handler(window_info *win)
{
	char string[50] = {0};

	safe_snprintf(string, sizeof(string), "%s %i", channel_color_str, channel_to_change);
	label_set_text(channel_color_win, color_label_id, string);
	return 1;
}

static void set_button_highlight(widget_list *w, int set_on)
{
	if (w == NULL)
		return;
	if (set_on)
		widget_set_flags(w->window_id, w->id, BUTTON_ACTIVE);
	else
		widget_unset_flags(w->window_id, w->id, BUTTON_ACTIVE);
}

static int click_channel_color_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	if(w->id < COLROWS * COLCOLS)
	{
		set_button_highlight(widget_find(w->window_id, selected_color), 0);
		set_button_highlight(w, 1);
		selected_color = w->id;
		do_click_sound();
		return 1;
	}
	if(w->id == color_set_button_id)
	{
		if(channel_to_change > 0 && selected_color >= 0)
		{
			int i;
			for(i=0; i<MAX_CHANNEL_COLORS; i++)
			{
				if(channel_colors[i].nr == channel_to_change)
					break;
			}
			if(i<MAX_CHANNEL_COLORS)
			{
				channel_colors[i].color = selected_color;
				do_click_sound();
				hide_window(channel_color_win);
				channel_to_change = 0;
				selected_color = -1;
				channel_colors_set = 1;
				return 1;
			}
			for(i=0; i<MAX_CHANNEL_COLORS; i++)
			{
				if(channel_colors[i].nr == 0)
					break;
			}

			if(i<MAX_CHANNEL_COLORS)
			{
				channel_colors[i].nr = channel_to_change;
				channel_colors[i].color = selected_color;
				do_click_sound();
				hide_window(channel_color_win);
				channel_to_change = 0;
				selected_color = -1;
				channel_colors_set = 1;
				return 1;
			} else {
				LOG_TO_CONSOLE(c_red3, "You reached the maximum numbers of channel colors.");
				return 1;
			}
		}
		return 0;
	}
	if(w->id == color_delete_button_id)
	{
		int i;
		for(i=0; i<MAX_CHANNEL_COLORS; i++)
		{
			if(channel_colors[i].nr == channel_to_change)
				break;
		}
		if(i<MAX_CHANNEL_COLORS)
		{
			channel_colors[i].color = -1;
			channel_colors[i].nr = 0;
			channel_colors_set = 1;
		}
		do_click_sound();
		hide_window(channel_color_win);
		channel_to_change = 0;
		selected_color = -1;
		return 1;
	}
	return 0;
}

static int hide_channel_color_handler(window_info *win)
{
	set_button_highlight(widget_find(win->window_id, selected_color), 0);
	return 1;
}

static int ui_scale_color_handler(window_info *win)
{
	int row, col;
	int button_width = (int)(0.5 + 110 * win->current_scale);
	int button_height = (int)(0.5 + 30 * win->current_scale);
	int spacing = (int)(0.5 + win->current_scale * 10);
	float label_size = 0.9 * win->current_scale;
	int buttons_offset = 2 * spacing + label_size * win->default_font_len_y;
	int len_x = 0;
	int len_y = 0;

	widget_set_size(win->window_id, color_label_id, 0.9 * win->current_scale);
	widget_resize(win->window_id, color_label_id, widget_get_width(win->window_id, color_label_id) * win->current_scale, widget_get_height(win->window_id, color_label_id) * win->current_scale);
	widget_move(win->window_id, color_label_id, spacing, spacing);

	for (row = 0; row < COLROWS; row++)
		for (col = 0; col < COLCOLS; col++)
		{
			button_resize(win->window_id, row + COLROWS * col, button_width, button_height, win->current_scale);
			widget_move(win->window_id, row + COLROWS * col, spacing + col * (button_width + spacing), buttons_offset + row * (button_height + spacing));
		}

	button_resize(win->window_id, color_set_button_id, 0, 0, win->current_scale);
	button_resize(win->window_id, color_delete_button_id, 0, 0, win->current_scale);

	len_x = spacing * (COLCOLS + 1) + COLCOLS * button_width;
	len_y = buttons_offset + COLROWS * (button_height + spacing) + widget_get_height(win->window_id, color_set_button_id) + spacing;

	widget_move(win->window_id, color_set_button_id, (len_x/2 - widget_get_width(win->window_id, color_set_button_id)) / 2, len_y - spacing - widget_get_height(win->window_id, color_set_button_id));
	widget_move(win->window_id, color_delete_button_id, len_x/2 + (len_x/2 - widget_get_width(win->window_id, color_delete_button_id)) / 2, len_y - spacing - widget_get_height(win->window_id, color_delete_button_id));

	resize_window(win->window_id, len_x, len_y);
	if (win->pos_y - win->title_height < get_tab_bar_y() * 1.25)
		move_window(win->window_id, win->pos_id, win->pos_loc, win->pos_x, win->title_height + get_tab_bar_y() * 1.25);

	return 1;
}

static int display_channel_color_win(Uint32 channel_number)
{
	if(channel_number == 0){
		return -1;
	}

	channel_to_change = channel_number;

	if(channel_color_win < 0)
	{
		/* Create the window */
		channel_color_win = create_window(channel_color_title_str, -1, 0, 300, 0, 0, 0, ELW_USE_UISCALE|ELW_WIN_DEFAULT);
		set_window_handler(channel_color_win, ELW_HANDLER_DISPLAY, &display_channel_color_handler);
		set_window_handler(channel_color_win, ELW_HANDLER_HIDE, &hide_channel_color_handler);
		set_window_handler(channel_color_win, ELW_HANDLER_UI_SCALE, &ui_scale_color_handler);

		/* Add labels */
		color_label_id = label_add_extended(channel_color_win, color_label_id, NULL, 0, 0, 0, 0.9f, channel_color_str);

		/* Add color buttons */
		{
			int row, col;
			char *name[COLROWS][COLROWS] = {{"red1", "red2", "red3", "red4"},
							    {"orange1", "orange2", "orange3", "orange4" },
							    {"yellow1", "yellow2", "yellow3", "yellow4"},
							    {"green1", "green2", "green3", "green4"},
							    {"blue1", "blue2", "blue3", "blue4"},
							    {"purple1", "purple2", "purple3", "purple4"},
							    {"grey1", "grey2", "grey3", "grey4"}};
			for (row = 0; row < COLROWS; row++)
				for (col = 0; col < COLCOLS; col++)
				{
					size_t the_colour = row + COLROWS * col;
					button_add_extended(channel_color_win, the_colour, NULL, 0, 0, 0, 0, 0, 1.0,
						name[row][col]);
					widget_set_color(channel_color_win, the_colour,
						colors_list[the_colour].r1/256.0, colors_list[the_colour].g1/256.0,
						colors_list[the_colour].b1/256.0);
					widget_set_OnClick(channel_color_win, the_colour, click_channel_color_handler);
				}
		}

		/* Add set/delete buttons */
		color_set_button_id = button_add_extended(channel_color_win, color_set_button_id, NULL, 0, 0, 0, 0, 0, 1.0, channel_color_add_str);
		widget_set_OnClick(channel_color_win, color_set_button_id, click_channel_color_handler);
		color_delete_button_id = button_add_extended(channel_color_win, color_delete_button_id, NULL, 0, 0, 0, 0, 0, 1.0, channel_color_delete_str);
		widget_set_OnClick(channel_color_win, color_delete_button_id, click_channel_color_handler);

		/* scale the window */
		if (channel_color_win >= 0 && channel_color_win < windows_list.num_windows)
			ui_scale_color_handler(&windows_list.window[channel_color_win]);
	}
	else
	{
		toggle_window(channel_color_win);
	}

	/* highlight the button if there is an active colour */
	if (get_show_window(channel_color_win))
	{
		int i;
		for(i=0; i<MAX_CHANNEL_COLORS; i++)
		{
			if(channel_colors[i].nr == channel_to_change)
			{
				if (channel_colors[i].color >= 0)
				{
					selected_color = channel_colors[i].color;
					set_button_highlight(widget_find(channel_color_win, selected_color), 1);
				}
				break;
			}
		}
	}

	return channel_color_win;
}

void load_channel_colors ()
{
	char fname[128];
	FILE *fp;
	int i;
	off_t file_size;

	if (channel_colors_set) {
		/*
		 * save existing channel colors instead of loading them if we are already logged in
		 * this will take place when relogging after disconnection
		 */
		save_channel_colors();
		return;
	}

	for(i=0; i<MAX_CHANNEL_COLORS; i++)
	{
		channel_colors[i].nr = 0;
		channel_colors[i].color = -1;
	}

#ifdef JSON_FILES
	if (get_use_json_user_files())
	{
		USE_JSON_DEBUG("Loading json file");
		// try to load the json file
		safe_snprintf(fname, sizeof(fname), "%schannel_colors_%s.json", get_path_config(), get_lowercase_username());
		if (json_load_channel_colours(fname, channel_colors, MAX_CHANNEL_COLORS) >= 0)
		{
			channel_colors_set = 1;
			return;
		}
	}

	// if there is no json file, or json use disabled, try to load the old binary format
	USE_JSON_DEBUG("Loading binary file");
#endif

	/* silently ignore non existing file */
	safe_snprintf(fname, sizeof(fname), "channel_colors_%s.dat",get_lowercase_username());
	if (file_exists_config(fname)!=1)
		return;

	file_size = get_file_size_config(fname);

	/* if the file exists but is not a valid size, don't use it */
	if ((file_size == 0) || (file_size != sizeof(channel_colors)))
	{
		LOG_ERROR("%s: Invalid format (size mismatch) \"%s\"\n", reg_error_str, fname);
		return;
	}

	fp = open_file_config(fname,"rb");
	if(fp == NULL){
		LOG_ERROR("%s: %s \"%s\": %s\n", reg_error_str, cant_open_file, fname, strerror(errno));
		return;
	}

	if (fread (channel_colors,sizeof(channel_colors),1, fp) != 1)
	{
		LOG_ERROR("%s() fail during read of file [%s] : %s\n", __FUNCTION__, fname, strerror(errno));
		fclose (fp);
		return;
	}

	fclose (fp);
	channel_colors_set = 1;
}

void save_channel_colors()
{
	char fname[128];
	FILE *fp;

	if (!channel_colors_set)
		return;

#ifdef JSON_FILES
	if (get_use_json_user_files())
	{
		USE_JSON_DEBUG("Saving json file");
		// save the json file
		safe_snprintf(fname, sizeof(fname), "%schannel_colors_%s.json", get_path_config(), get_lowercase_username());
		if (json_save_channel_colours(fname, channel_colors, MAX_CHANNEL_COLORS) < 0)
			LOG_ERROR("%s: %s \"%s\"\n", reg_error_str, cant_open_file, fname);
		return;
	}
	USE_JSON_DEBUG("Saving binary file");
#endif

	safe_snprintf(fname, sizeof(fname), "channel_colors_%s.dat",get_lowercase_username());
	fp=open_file_config(fname,"wb");
	if(fp == NULL){
		LOG_ERROR("%s: %s \"%s\": %s\n", reg_error_str, cant_open_file, fname, strerror(errno));
		return;
	}

	if (fwrite (channel_colors,sizeof(channel_colors),1, fp) != 1)
	{
		LOG_ERROR("%s() fail during write of file [%s] : %s\n", __FUNCTION__, fname, strerror(errno));
	}

	fclose(fp);
}

int command_channel_colors(char * text, int len)
{
	int i;
	char string[20];

	LOG_TO_CONSOLE(c_grey1, "Your currently set channel colors:");
	for(i=0; i<MAX_CHANNEL_COLORS; i++)
	{
		if(channel_colors[i].nr >0 && channel_colors[i].color > -1)
		{
			safe_snprintf(string, sizeof(string), "Channel %u", channel_colors[i].nr);
			LOG_TO_CONSOLE(channel_colors[i].color, string);
		}
	}
	return 1;
}

int get_tab_bar_x(void)
{
	if (use_windowed_chat == 1 && tab_bar_win >= 0 && tab_bar_win < windows_list.num_windows)
		return windows_list.window[tab_bar_win].pos_x;
	return 10;
}

int get_tab_bar_y(void)
{
	if (use_windowed_chat == 1 && tab_bar_win >= 0 && tab_bar_win < windows_list.num_windows)
		return 4 + windows_list.window[tab_bar_win].pos_y + windows_list.window[tab_bar_win].len_y;
	return 20;
}

void next_channel_tab(void)
{
	int next_tab;
	widget_list *widget;
	tab_collection *collection;
	switch(use_windowed_chat)
	{
		case 1: //Tabs
			if(current_tab == tabs_in_use-1)
			{
				next_tab = 2;
			}
			else
			{
				next_tab = current_tab + 1;
			}
			switch_to_tab(next_tab);
		break;
		case 2: //Window
			widget = widget_find(get_id_MW(MW_CHAT), chat_tabcollection_id);
			collection = widget->widget_info;
			if(active_tab == collection->nr_tabs - 1)
			{
				next_tab = 2;
			}
			else
			{
				next_tab = active_tab + 1;
			}
			switch_to_chat_tab(channels[next_tab].tab_id, 0);
		break;
		default:
			break;
	}
}

void prev_channel_tab(void)
{
	int next_tab;
	widget_list *widget;
	tab_collection *collection;
	switch(use_windowed_chat)
	{
		case 1: //Tab
			if(current_tab == 2)
			{
				next_tab = tabs_in_use-1;
			}
			else
			{
				next_tab = current_tab-1;
			}
			switch_to_tab(next_tab);
			break;
		case 2: //Window
			widget = widget_find(get_id_MW(MW_CHAT), chat_tabcollection_id);
			collection = widget->widget_info;
			if(active_tab == 2)
			{
				next_tab = collection->nr_tabs - 1;
			}
			else
			{
				next_tab = active_tab - 1;
			}
			switch_to_chat_tab(channels[next_tab].tab_id, 0);
		break;
		default:
			break;
	}
}

void update_text_windows (text_message * pmsg)
{
	if (get_id_MW(MW_CONSOLE) >= 0) update_console_win (pmsg);
	switch (use_windowed_chat) {
		case 0:
			lines_to_show += pmsg->wrap_lines;
			if (lines_to_show > max_chat_lines.value) lines_to_show = max_chat_lines.value;
			break;
		case 1:
			update_tab_bar (pmsg);
			break;
		case 2:
			update_chat_window (pmsg, 1);
			break;
	}
}

void recolour_message(text_message *msg){
	if (msg->chan_idx >= CHAT_CHANNEL1 && msg->chan_idx <= CHAT_CHANNEL3 && msg->len > 0 && msg->data[0] && !msg->deleted)
	{
		int i;
		for(i=0; i< MAX_CHANNEL_COLORS; i++)
		{
			if(channel_colors[i].nr == msg->channel)
				break;
		}
		if(i< MAX_CHANNEL_COLORS && channel_colors[i].color != -1) {
			msg->data[0] = to_color_char (channel_colors[i].color);
		} else if (active_channels[current_channel] != msg->channel){
			msg->data[0] = to_color_char (c_grey2);
		} else {
			msg->data[0] = to_color_char (c_grey1);
		}
	}
}

void recolour_messages(text_message *msgs){
	int i;
	for(i=0;i<DISPLAY_TEXT_BUFFER_SIZE && msgs[i].data;++i){
		recolour_message(&msgs[i]);
	}
}

void reset_tab_channel_colours(void)
{
	int chat_win = get_id_MW(MW_CHAT);
	int i;
	for (i=0; i < MAX_CHAT_TABS; i++) {
		if (channels[i].open) {
			tab_set_label_color_by_id (chat_win, chat_tabcollection_id, channels[i].tab_id, -1.0f, -1.0f, -1.0f);
		}
	}
}


int skip_message (const text_message *msg, Uint8 filter)
{
	int skip = 0;
	int channel = msg->chan_idx;
	if (filter == FILTER_ALL) return 0;
	if (channel != filter)
	{
		switch (channel)
		{
			case CHAT_LOCAL:    skip = local_chat_separate;    break;
			case CHAT_PERSONAL: skip = personal_chat_separate; break;
			case CHAT_GM:       skip = guild_chat_separate;    break;
			case CHAT_SERVER:   skip = server_chat_separate;   break;
			case CHAT_MOD:      skip = mod_chat_separate;      break;
			case CHAT_MODPM:    skip = 0;                      break;
			default:            skip = 1;
		}
		return skip;
	}
	switch (channel) {
		case CHAT_CHANNEL1:
		case CHAT_CHANNEL2:
		case CHAT_CHANNEL3:
			skip = (msg->channel != active_channels[filter - CHAT_CHANNEL1]);
	}
	return skip;
}

static node_t *p_channel_queue = NULL;

void set_first_tab_channel(void)
{
	p_channel_queue = queue_front_node(chan_name_queue);
}

const char * get_tab_channel_name(void)
{
	if (p_channel_queue != NULL)
		return ((chan_name*)(p_channel_queue->data))->name;
	else
		return NULL;
}

void set_next_tab_channel(void)
{
	if (p_channel_queue != NULL)
		p_channel_queue = p_channel_queue->next;
}

void change_to_channel_tab(const char *line)
{
	switch(use_windowed_chat)
	{
		case 1:
			if(tabs[current_tab].channel != CHAT_ALL) {
				change_to_current_tab(line);
			}
		break;
		case 2:
			if(channels[active_tab].chan_nr != CHAT_ALL) {
				change_to_current_chat_tab(line);
			}
		break;
	}
}

int get_input_height()
{
	return get_line_height(CHAT_FONT, 1.0) + 2*INPUT_MARGIN;
}
