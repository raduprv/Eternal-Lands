#include <stdlib.h>
#include <SDL.h>
#include "openingwin.h"
#include "books.h"
#include "consolewin.h"
#include "draw_scene.h"
#include "elconfig.h"
#include "events.h"
#include "font.h"
#include "gamewin.h"
#include "gl_init.h"
#include "init.h"
#include "interface.h"
#include "loginwin.h"
#include "multiplayer.h"
#include "new_character.h"
#include "tabs.h"
#include "widgets.h"

int opening_root_win = -1;

int opening_out_id = 40;

int nr_opening_lines;

int opening_win_text_width = -1;
int opening_win_text_height = -1;

int display_opening_handler ()
{
	int msg, offset, iline;

	iline = get_total_nr_lines() - nr_opening_lines;
	if (iline < 0) iline = 0;

	find_line_nr(get_total_nr_lines(), iline, FILTER_ALL, &msg, &offset,
		CHAT_FONT, 1.0, opening_win_text_width);
	text_field_set_buf_pos (opening_root_win, opening_out_id, msg, offset);
	draw_console_pic (cons_text);
	CHECK_GL_ERRORS();

	draw_delay = 20;
	return 1;
}

void switch_to_login ()
{
#ifdef MAP_EDITOR2
	show_window (game_root_win);
#else
	// bring up the login screen
	show_window (login_root_win);
#endif

	// destroy ourselves, we're no longer needed
	destroy_window (opening_root_win);
	opening_root_win = -1;
}

int click_opening_handler ()
{
	if (!disconnected) switch_to_login ();
	return 1;
}

int keypress_opening_handler (window_info *win, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
#ifndef MAP_EDITOR2
	int alt_on = key_mod & KMOD_ALT;
	int ctrl_on = key_mod & KMOD_CTRL;
#endif

	if(check_quit_or_fullscreen(key_code, key_mod))
	{
		return 1;
	}
	else if(!disconnected)
	{
		switch_to_login();
	}
#ifndef MAP_EDITOR2
	else if (!alt_on && !ctrl_on)
	{
		connect_to_server();
	}
#endif
	else
		return 0;
	return 1;
}

int show_opening_handler (window_info *win) {
#ifndef MAP_EDITOR2
	close_book_window();
#endif
	hide_window_MW(MW_CONFIG);
	hide_window_MW(MW_HELP);
	return 1;
}

static int change_opening_font_handler(window_info *win, font_cat cat)
{
	if (cat != CHAT_FONT)
		return 0;
	nr_opening_lines = get_max_nr_lines(opening_win_text_height, CHAT_FONT, 1.0);
	return 1;
}

void create_opening_root_window (int width, int height)
{
	if (opening_root_win < 0)
	{
		opening_root_win = create_window ("Opening", -1, -1, 0, 0, width, height, ELW_TITLE_NONE|ELW_SHOW_LAST);

		set_window_handler (opening_root_win, ELW_HANDLER_DISPLAY, &display_opening_handler);
		set_window_handler (opening_root_win, ELW_HANDLER_KEYPRESS, (int (*)())&keypress_opening_handler);
		set_window_handler (opening_root_win, ELW_HANDLER_CLICK, &click_opening_handler);
		set_window_handler (opening_root_win, ELW_HANDLER_SHOW, &show_opening_handler);
		set_window_handler(opening_root_win, ELW_HANDLER_FONT_CHANGE, &change_opening_font_handler);

		opening_out_id = text_field_add_extended (opening_root_win, opening_out_id,
			NULL, 0, 0, width, height, 0, CHAT_FONT, 1.0,
			display_text_buffer, DISPLAY_TEXT_BUFFER_SIZE, FILTER_ALL, 0, 0);
		widget_unset_color(opening_root_win, opening_out_id);

		nr_opening_lines = get_max_nr_lines(height, CHAT_FONT, 1.0);
		opening_win_text_width = width;
		opening_win_text_height = height;
	}
}
