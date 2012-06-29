#include <stdlib.h>
#include <SDL.h>
#include "openingwin.h"
#include "books.h"
#include "consolewin.h"
#include "draw_scene.h"
#include "elconfig.h"
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

void opening_win_update_zoom () {
	nr_opening_lines = opening_win_text_height / (18 * chat_zoom);
	widget_set_size(opening_root_win, opening_out_id, chat_zoom);
}

int display_opening_handler ()
{
	if (SDL_GetAppState () & SDL_APPACTIVE)
	{
		int msg, offset, iline;
		
		iline = get_total_nr_lines() - nr_opening_lines;
		if (iline < 0) iline = 0;
		
		find_line_nr (get_total_nr_lines(), iline, FILTER_ALL, &msg, &offset, chat_zoom, opening_win_text_width);
		text_field_set_buf_pos (opening_root_win, opening_out_id, msg, offset);
		draw_console_pic (cons_text);
		CHECK_GL_ERRORS();
	}

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

int keypress_opening_handler (window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
#ifndef MAP_EDITOR2
	int alt_on = key & ELW_ALT;
	int ctrl_on = key & ELW_CTRL;
#endif

	if(check_quit_or_fullscreen(key))
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
	
	return 1;
}

int show_opening_handler (window_info *win) {
#ifndef MAP_EDITOR2
	hide_window(book_win);
	hide_window(paper_win);
	hide_window(color_race_win);
#endif
	hide_window(elconfig_win);
	hide_window(tab_help_win);
	return 1;
}

void create_opening_root_window (int width, int height)
{
	if (opening_root_win < 0)
	{
		opening_root_win = create_window ("Opening", -1, -1, 0, 0, width, height, ELW_TITLE_NONE|ELW_SHOW_LAST);

		set_window_handler (opening_root_win, ELW_HANDLER_DISPLAY, &display_opening_handler);
		set_window_handler (opening_root_win, ELW_HANDLER_KEYPRESS, &keypress_opening_handler);
		set_window_handler (opening_root_win, ELW_HANDLER_CLICK, &click_opening_handler);
		set_window_handler (opening_root_win, ELW_HANDLER_SHOW, &show_opening_handler);
		
		opening_out_id = text_field_add_extended (opening_root_win, opening_out_id, NULL, 0, 0, width, height, 0, chat_zoom, -1.0f, -1.0f, -1.0f, display_text_buffer, DISPLAY_TEXT_BUFFER_SIZE, FILTER_ALL, 0, 0);
		
		nr_opening_lines = height / (18 * chat_zoom);
		opening_win_text_width = width;
		opening_win_text_height = height;
	}
}
