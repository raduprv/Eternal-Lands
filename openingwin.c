#include "global.h"

#ifndef OLD_EVENT_HANDLER

int opening_win = -1;

int display_opening_handler ()
{
	draw_console_pic (cons_text);
	display_console_text ();
	CHECK_GL_ERRORS();

	draw_delay = 20;
	return 1;
}

void switch_to_login ()
{
	// bring up the login screen
	show_window (login_win);

	// destroy ourselves, we're no longer needed
	destroy_window (opening_win);
	opening_win = -1;

	interface_mode = INTERFACE_LOG_IN;
}

int click_opening_handler ()
{
	if (!disconnected) switch_to_login ();
	return 1;
}

int keypress_opening_handler (window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
	int alt_on = key & ELW_ALT;
	int ctrl_on = key & ELW_CTRL;

	if ( check_quit_or_fullscreen (key) )
	{
		return 1;
	}
	else if (!disconnected)
	{
		switch_to_login ();
	}
	else if (!alt_on && !ctrl_on)
	{
		connect_to_server ();
	}
	
	return 1;
}

void create_opening_window ()
{
	if (opening_win < 0)
	{
		opening_win = create_window ("Opening", -1, -1, 0, 0, window_width, window_height, ELW_TITLE_NONE|ELW_SHOW_LAST);

                set_window_handler (opening_win, ELW_HANDLER_DISPLAY, &display_opening_handler);
                set_window_handler (opening_win, ELW_HANDLER_KEYPRESS, &keypress_opening_handler);
                set_window_handler (opening_win, ELW_HANDLER_CLICK, &click_opening_handler);
	}
}

#endif // not def OLD_EVENT_HANDLER
