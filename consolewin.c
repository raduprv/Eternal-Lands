#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL.h>
#include "consolewin.h"
#include "asc.h"
#include "books.h"
#include "chat.h"
#include "console.h"
#include "cursors.h"
#include "draw_scene.h"
#include "elwindows.h"
#include "events.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud.h"
#include "init.h"
#include "interface.h"
#include "mapwin.h"
#include "missiles.h"
#include "new_character.h"
#if !defined OSX && !defined WINDOWS
#ifdef MIDDLE_MOUSE_PASTE
#include "paste.h"
#endif
#endif
#include "spells.h"
#include "text.h"
#include "special_effects.h"
#include "eye_candy_wrapper.h"

/* To Do
 * Existing bugs before I even started on the scroll bar:
 * - Font size of zero - number of lines go crazy
 * - Changing font size does not update total_nr_lines
 * - Resize of chat window going very wide after font size change
 * - Mouse paste does not resize input box
 * - Mouse paste can resize input box (have a patch)
 * Code Tidy:
 * 	- Sort out len_y of console output text field - sep/margin etc
 * 	- Intimate use of input_widget all over the place in different modules
 */

int console_scrollbar_enabled = 1;
int locked_to_console = 0;

static int console_out_id = 40;
static int console_in_id = 41;
static int console_scrollbar_id = 42;

static int CONSOLE_Y_OFFSET = 25;
static const int CONSOLE_TEXT_X_BORDER = 10;

static int nr_console_lines = 0;
static int total_nr_lines = 0;
static int scroll_up_lines = 0;
static int console_text_changed = 0;
static int console_text_width = -1;

static inline int get_console_sep_height(void)
{
	return get_line_height(CHAT_FONT, 1.0);
}

static void update_console_scrollbar(void)
{
	int barlen = 0, barpos = 0;
	if (!console_scrollbar_enabled)
		return;
	if (total_nr_lines - nr_console_lines > 0)
	{
		barlen = total_nr_lines - nr_console_lines;
		barpos = barlen - scroll_up_lines;
	}
	vscrollbar_set_bar_len(get_id_MW(MW_CONSOLE), console_scrollbar_id, barlen);
	vscrollbar_set_pos(get_id_MW(MW_CONSOLE), console_scrollbar_id, barpos);
	//printf("pos=%d len=%d scroll_up_lines=%d total_nr_lines=%d nr_console_lines=%d\n",
	//	barpos, barlen, scroll_up_lines, total_nr_lines, nr_console_lines );
}

static int display_console_handler (window_info *win)
{
	static int msg = 0, offset = 0;

	if (get_tab_bar_y() != CONSOLE_Y_OFFSET)
	{
		CONSOLE_Y_OFFSET = get_tab_bar_y();
		resize_window(win->window_id, win->len_x, win->len_y);
	}

	if (console_text_changed)
	{
		find_line_nr(total_nr_lines, total_nr_lines - nr_console_lines - scroll_up_lines,
			FILTER_ALL, &msg, &offset, CHAT_FONT, 1.0, console_text_width);
		text_field_set_buf_pos(win->window_id, console_out_id, msg, offset);
		update_console_scrollbar();
		console_text_changed = 0;
	}

	draw_console_pic (cons_text);
	if (scroll_up_lines != 0)
	{
		glColor3f (1.0, 1.0, 1.0);
		draw_console_separator(CONSOLE_TEXT_X_BORDER,
			win->len_y - HUD_MARGIN_Y - input_widget->len_y - get_console_sep_height(),
			console_text_width, 1.0);
	}
	//ttlanhil: disabled, until the scrolling in console is adusted to work with filtering properly
	//if the users prefer that console not be filtered, the following line can be removed.
	//if they want it filtered, then more work can be done until it works properly
	//((text_field*)((widget_find(win->window_id, console_out_id))->widget_info))->chan_nr = current_filter;

	draw_hud_interface (win);

	display_handling_common(win);

	return 1;
}

static int keypress_console_handler (window_info *win, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	// first try the keypress handler for all root windows
	if ( keypress_root_common (key_code, key_unicode, key_mod) )
	{
		return 1;
	}
	else if(key_code == SDLK_UP)
 	{
		if (total_nr_lines > nr_console_lines + scroll_up_lines)
		{
			scroll_up_lines++;
			console_text_changed = 1;
		}
 	}
	else if (key_code == SDLK_DOWN)
 	{
		if(scroll_up_lines > 0)
		{
			scroll_up_lines--;
			console_text_changed = 1;
		}
	}
	else if (key_mod & KMOD_ALT && key_code == SDLK_PAGEUP && total_nr_lines > nr_console_lines + scroll_up_lines)
	{
		scroll_up_lines = total_nr_lines - nr_console_lines;
		console_text_changed = 1;
	}
	else if (key_mod & KMOD_ALT && key_code == SDLK_PAGEDOWN && scroll_up_lines > 0)
	{
		scroll_up_lines = 0;
		console_text_changed = 1;
	}
	else if (key_code == SDLK_PAGEUP && total_nr_lines > nr_console_lines + scroll_up_lines)
	{
		scroll_up_lines += nr_console_lines - 1;
		if (nr_console_lines + scroll_up_lines > total_nr_lines)
			scroll_up_lines = total_nr_lines - nr_console_lines;
		console_text_changed = 1;
	}
	else if (key_code == SDLK_PAGEDOWN && scroll_up_lines > 0)
	{
		scroll_up_lines -= nr_console_lines - 1;
		if (scroll_up_lines < 0)
			scroll_up_lines = 0;
		console_text_changed = 1;
	}
	else if (KEY_DEF_CMP(K_MAP, key_code, key_mod) || KEY_DEF_CMP(K_MARKFILTER, key_code, key_mod))
	{
		if (!locked_to_console && switch_to_game_map())
		{
			// if K_MARKFILTER pressed, open the map window with the filter active
			if (KEY_DEF_CMP(K_MARKFILTER, key_code, key_mod))
				mark_filter_active = 1;
			hide_window (win->window_id);
			show_window_MW(MW_TABMAP);
		}
	}
	else
	{
		Uint8 ch = key_to_char (key_unicode);

		if ((ch == '`' || KEY_DEF_CMP(K_CONSOLE, key_code, key_mod)) && !locked_to_console)
		{
			return_to_gamewin_common();
		}
		else if ( !text_input_handler (key_code, key_unicode, key_mod) )
		{
			// nothing we can handle
			return 0;
		}
	}

	// we handled it, return 1 to let the window manager know
	return 1;
}

static void recalc_message_lines(void)
{
	size_t i;
	total_nr_lines = 0;
	for (i = 0; i < DISPLAY_TEXT_BUFFER_SIZE; i++)
	{
		if (display_text_buffer[i].len && !display_text_buffer[i].deleted)
		{
			total_nr_lines += rewrap_message(&display_text_buffer[i], CHAT_FONT,
				1.0, console_text_width, NULL);
		}
	}
}

static int resize_console_handler (window_info *win, int width, int height)
{
	int scrollbar_x_adjust = (console_scrollbar_enabled) ?win->box_size :0;
	int console_active_width = width - HUD_MARGIN_X;
	int console_active_height = height - HUD_MARGIN_Y;
	int text_display_height = console_active_height - input_widget->len_y - get_console_sep_height() - CONSOLE_Y_OFFSET;

	console_text_width = (int) (console_active_width - 2*CONSOLE_TEXT_X_BORDER - scrollbar_x_adjust);

	widget_resize (win->window_id, console_out_id, console_text_width, text_display_height);
	widget_move (win->window_id, console_out_id, CONSOLE_TEXT_X_BORDER, CONSOLE_Y_OFFSET);
	widget_resize (win->window_id, input_widget->id, console_active_width, input_widget->len_y);
	widget_move (win->window_id, input_widget->id, 0, console_active_height - input_widget->len_y);

	nr_console_lines = get_max_nr_lines(text_display_height, CHAT_FONT, 1.0);
	recalc_message_lines();

	if (console_scrollbar_enabled)
	{
		widget_resize(win->window_id, console_scrollbar_id, win->box_size, text_display_height);
		widget_move(win->window_id, console_scrollbar_id, console_active_width - win->box_size, CONSOLE_Y_OFFSET);
		update_console_scrollbar();
	}

	/* making the font smaller can leave the scroll position invalid */
	if (scroll_up_lines && (total_nr_lines <= nr_console_lines))
		scroll_up_lines = 0;

	if (get_show_window(win->window_id))
		init_hud_interface (HUD_INTERFACE_GAME);

	return 1;
}

static int console_scroll_click(widget_list *widget, int mx, int my, Uint32 flags)
{
	scroll_up_lines = (total_nr_lines - nr_console_lines) - vscrollbar_get_pos(widget->window_id, console_scrollbar_id);
	console_text_changed = 1;
	return 1;
}

static int console_scroll_drag(widget_list *widget, int mx, int my, Uint32 flags, int dx, int dy)
{
	return console_scroll_click(widget, mx, my, flags);
}

static void create_console_scrollbar(window_info *win)
{
	int console_active_width = window_width - HUD_MARGIN_X;
	int console_active_height = window_height - HUD_MARGIN_Y;
	if (input_widget == NULL)
		return;
	console_scrollbar_id = vscrollbar_add_extended(win->window_id, console_scrollbar_id, NULL,
		console_active_width - win->box_size, CONSOLE_Y_OFFSET,
		win->box_size, console_active_height - get_console_sep_height() - CONSOLE_Y_OFFSET - input_widget->len_y,
		0, 1.0, 0, 1, total_nr_lines-nr_console_lines);
	widget_set_OnDrag(win->window_id, console_scrollbar_id, console_scroll_drag);
	widget_set_OnClick(win->window_id, console_scrollbar_id, console_scroll_click);
}

static int mouseover_console_handler(window_info *win, int mx, int my)
{
	if (hud_mouse_over(win, mx, my))
		return 1;
	elwin_mouse = CURSOR_ARROW;
	return 1;
}

static int click_console_handler(window_info *win, int mx, int my, Uint32 flags)
{
	if (hud_click(win, mx, my, flags))
		return 1;

#if !defined OSX && !defined WINDOWS
#ifdef MIDDLE_MOUSE_PASTE
	if ( (flags & ELW_MID_MOUSE) )
	{
		start_paste_from_primary(NULL);
	}
	else
#endif
#endif
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

static int show_console_handler (window_info *win)
{
	reset_tab_channel_colours();
	close_book_window();
	if (use_windowed_chat == 1) {
		display_tab_bar ();
	}
	return 1;
}

int get_console_text_width(void)
{
	return console_text_width;
}

int get_total_nr_lines(void)
{
	return total_nr_lines;
}

void clear_console(){
	console_text_changed = 1;
	clear_lines_to_show();
	scroll_up_lines = 0;
	total_nr_lines = 0;
}

void update_console_win (text_message * msg)
{
	if (msg->deleted) {
		if (scroll_up_lines > msg->wrap_lines) {
			scroll_up_lines -= msg->wrap_lines;
		} else {
			scroll_up_lines = 0;
			console_text_changed = 1;
		}
		total_nr_lines -= msg->wrap_lines;
	} else {
		int nlines = rewrap_message(msg, CHAT_FONT, 1.0, console_text_width, NULL);
		if (scroll_up_lines == 0) {
			console_text_changed = 1;
		} else {
			scroll_up_lines += nlines;
			if(scroll_up_lines > DISPLAY_TEXT_BUFFER_SIZE){
				scroll_up_lines = DISPLAY_TEXT_BUFFER_SIZE;
			}
		}
		total_nr_lines += nlines;
	}
}

void toggle_console_scrollbar(int *enable)
{
	int console_root_win = get_id_MW(MW_CONSOLE);
	*enable = !*enable;
	if ((console_root_win >= 0) && (console_root_win < windows_list.num_windows))
	{
		window_info *win = &windows_list.window[console_root_win];
		if (!*enable)
			widget_destroy(console_root_win, console_scrollbar_id);
		else
			create_console_scrollbar(win);
		resize_console_handler (win, window_width, window_height);
	}
}

static int ui_scale_console_handler(window_info *win)
{
	resize_window(win->window_id, win->len_x, win->len_y);
	return 1;
}

static int change_console_font_handler(window_info *win, font_cat cat)
{
	if (cat != CHAT_FONT)
		return 0;

	nr_console_lines = get_max_nr_lines(window_height - input_widget->len_y - get_console_sep_height() - hud_y - CONSOLE_Y_OFFSET,
		CHAT_FONT, 1.0);
	resize_console_handler(win, window_width, window_height);
	return 1;
}

void create_console_root_window (int width, int height)
{
	int console_root_win = get_id_MW(MW_CONSOLE);
	if (console_root_win < 0)
	{
		window_info *win = NULL;
		int scrollbar_x_adjust = 0;
		int console_active_width = width - HUD_MARGIN_X;
		int console_active_height = height - HUD_MARGIN_Y;
		int input_height = get_input_height();

		console_root_win = create_window ("Console", -1, -1, 0, 0, width, height, ELW_USE_UISCALE|ELW_TITLE_NONE|ELW_SHOW_LAST);
		set_id_MW(MW_CONSOLE, console_root_win);
		if (console_root_win < 0 || console_root_win >= windows_list.num_windows)
			return;

		win = &windows_list.window[console_root_win];
		scrollbar_x_adjust = (console_scrollbar_enabled) ?win->box_size :0;

		console_text_width = (int) (console_active_width - 2*CONSOLE_TEXT_X_BORDER - scrollbar_x_adjust);

		set_window_handler (console_root_win, ELW_HANDLER_DISPLAY, &display_console_handler);
		set_window_handler (console_root_win, ELW_HANDLER_KEYPRESS, (int (*)())&keypress_console_handler);
		set_window_handler (console_root_win, ELW_HANDLER_RESIZE, &resize_console_handler);
		set_window_handler (console_root_win, ELW_HANDLER_CLICK, &click_console_handler);
		set_window_handler (console_root_win, ELW_HANDLER_MOUSEOVER, &mouseover_console_handler);
		set_window_handler (console_root_win, ELW_HANDLER_SHOW, &show_console_handler);
		set_window_handler (console_root_win, ELW_HANDLER_UI_SCALE, &ui_scale_console_handler);
		set_window_handler(console_root_win, ELW_HANDLER_FONT_CHANGE, &change_console_font_handler);

		console_out_id = text_field_add_extended(console_root_win, console_out_id, NULL,
			CONSOLE_TEXT_X_BORDER, CONSOLE_Y_OFFSET, console_text_width,
			console_active_height - input_height - get_console_sep_height() - CONSOLE_Y_OFFSET,
			0, CHAT_FONT, 1.0, display_text_buffer, DISPLAY_TEXT_BUFFER_SIZE, CHAT_ALL, 0, 0);
		widget_unset_color(console_root_win, console_out_id);

		recalc_message_lines();

		if (input_widget == NULL)
		{
			Uint32 id;
			id = text_field_add_extended(console_root_win, console_in_id, NULL,
				0, console_active_height - input_height, console_active_width, input_height,
				(INPUT_DEFAULT_FLAGS|TEXT_FIELD_BORDER)^WIDGET_CLICK_TRANSPARENT,
				CHAT_FONT, 1.0, &input_text_line, 1, FILTER_ALL, INPUT_MARGIN, INPUT_MARGIN);
			input_widget = widget_find(console_root_win, id);
			input_widget->OnResize = input_field_resize;
		}
		widget_set_OnKey(input_widget->window_id, input_widget->id, (int (*)())chat_input_key);

		nr_console_lines = get_max_nr_lines(console_active_height - input_widget->len_y - get_console_sep_height() - CONSOLE_Y_OFFSET,
			CHAT_FONT, 1.0);

		if (console_scrollbar_enabled && (console_root_win >= 0) && (console_root_win < windows_list.num_windows))
		{
			create_console_scrollbar(&windows_list.window[console_root_win]);
			update_console_scrollbar();
		}
	}
}

int input_field_resize(widget_list *w, Uint32 x, Uint32 y)
{
	int console_root_win = get_id_MW(MW_CONSOLE);
	window_info *console_win = &windows_list.window[console_root_win];
	widget_list *console_out_w = widget_find(console_root_win, console_out_id);
	text_field *tf = w->widget_info;
	text_message *msg = &(tf->buffer[tf->msg]);
	int console_active_height;

	// set invalid width to force rewrap
	msg->wrap_width = 0;
	tf->nr_lines = rewrap_message(msg, w->fcat, w->size,
		w->len_x - 2 * tf->x_space, &tf->cursor);
	if(use_windowed_chat != 2 || !get_show_window(get_id_MW(MW_CHAT))) {
		window_info *win = &windows_list.window[w->window_id];
		widget_move(input_widget->window_id, input_widget->id, 0, win->len_y - input_widget->len_y - HUD_MARGIN_Y);
	}

	console_active_height = console_win->len_y - HUD_MARGIN_Y - input_widget->len_y - get_console_sep_height() - CONSOLE_Y_OFFSET;
	widget_resize(console_root_win, console_out_id, console_out_w->len_x, console_active_height);
	if (console_scrollbar_enabled)
		widget_resize(console_root_win, console_scrollbar_id, console_win->box_size, console_active_height);
	nr_console_lines = get_max_nr_lines(console_out_w->len_y, CHAT_FONT, 1.0);
	console_text_changed = 1;
	return 1;
}

int history_grep (const char* text, int len)
{
	unsigned int i = 0, wraps = 1;
	int idx = last_message;
	int skip;

	for (skip = 0; skip < len; skip++)
		if (text[skip] != ' ') break;
	if (skip >= len) return 1;

	text += skip;
	len -= skip;

	for (i = 0; i <= total_nr_lines; ++i)
	{
		if (++wraps >= display_text_buffer[idx].wrap_lines)
		{
			wraps = 1;
			if (--idx < 0)
				break;
		}

		if (i <= scroll_up_lines || display_text_buffer[idx].len < len)
			// line is already visible, or the message is too
			// short to contain the search term
			continue;

		if (safe_strcasestr (display_text_buffer[idx].data, display_text_buffer[idx].len, text, len))
		{
			if(i > total_nr_lines - nr_console_lines)
				scroll_up_lines = total_nr_lines - nr_console_lines;
			else
				scroll_up_lines = i+1;
			console_text_changed = 1;
			break;
		}
	}

	return 1;
}
