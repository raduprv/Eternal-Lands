#include "global.h"

#ifdef WINDOW_CHAT

int opening_win = -1;

int display_opening_handler ()
{
	draw_console_pic (cons_text);
	display_console_text ();
	check_gl_errors();

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

	interface_mode = interface_log_in;
}

int click_opening_handler ()
{
	if (!disconnected) switch_to_login ();
	return 1;
}

int keypress_opening_handler (window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
	Uint16 keysym = key & 0xffff;
	int alt_on = key & ELW_ALT;
	int ctrl_on = key & ELW_CTRL;

	// first, try to see if we pressed Alt+x, to quit.
	if ( (keysym == SDLK_x && alt_on) || (keysym == SDLK_q && ctrl_on && !alt_on) )
	{
		exit_now = 1;
	}
	else if (!disconnected)
	{
		switch_to_login ();
	}
	else if (keysym == SDLK_RETURN && alt_on)
	{
		toggle_full_screen ();
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

#endif // WINDOW_CHAT
