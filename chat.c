#include "global.h"

#ifndef OLD_EVENT_HANDLER

int use_windowed_chat = 0;

int chat_win = -1;
int chat_scroll_id = 15;
int chat_win_x = 0; // upper left corner by default
int chat_win_y = 0;
int chat_win_len_x = CHAT_WIN_TEXT_WIDTH + CHAT_WIN_SCROLL_WIDTH + 4;
int chat_win_len_y = CHAT_WIN_TEXT_HEIGHT + 4;
int chat_win_text_width = CHAT_WIN_TEXT_WIDTH;
int chat_win_text_height = CHAT_WIN_TEXT_HEIGHT;

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
		line_start = find_line_nr (line);
	}
	draw_string_zoomed_clipped (2, 2, &display_text_buffer[line_start], chat_win_text_width, chat_win_text_height, chat_zoom);
	text_changed = 0;

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
	chat_win_text_width = width - 4 - CHAT_WIN_SCROLL_WIDTH;
	chat_win_text_height = height - 4;
	widget_move (chat_win, chat_scroll_id, width-CHAT_WIN_SCROLL_WIDTH, 0);
	widget_resize (chat_win, chat_scroll_id, CHAT_WIN_SCROLL_WIDTH, win->len_y - ELW_BOX_SIZE);
	nr_displayed_lines = (int) (height / (18.0 * chat_zoom));
	update_chat_scrollbar ();
	return 0;
}

void display_chat ()
{
	if (chat_win < 0)
	{
		int scroll_len;
		
		nr_displayed_lines = (int) (CHAT_WIN_TEXT_HEIGHT / (18.0 * chat_zoom));
		scroll_len = nr_text_buffer_lines >= nr_displayed_lines ? nr_text_buffer_lines-nr_displayed_lines : 0;
		
		chat_win = create_window ("chat", game_win, 0, chat_win_x, chat_win_y, chat_win_len_x, chat_win_len_y, (ELW_WIN_DEFAULT|ELW_RESIZEABLE) & ~ELW_CLOSE_BOX);
		
		set_window_handler(chat_win, ELW_HANDLER_DISPLAY, &display_chat_handler);
		set_window_handler(chat_win, ELW_HANDLER_DRAG, &drag_chat_handler);
		set_window_handler(chat_win, ELW_HANDLER_CLICK, &click_chat_handler);
		set_window_handler(chat_win, ELW_HANDLER_RESIZE, &resize_chat_handler);

		chat_scroll_id = vscrollbar_add_extended (chat_win, chat_scroll_id, NULL, chat_win_len_x-CHAT_WIN_SCROLL_WIDTH, 0, CHAT_WIN_SCROLL_WIDTH, chat_win_len_y-ELW_BOX_SIZE, 0, 1.0, 0.77f, 0.57f, 0.39f, 0, 1, scroll_len);
	}
	else
	{
		show_window (chat_win);
		select_window (chat_win);
	}
}

#endif // not def OLD_EVENT_HANDLER
