#include <string.h>
#include <ctype.h>
#include "global.h"

#ifdef WINDOW_CHAT

int login_win = -1;

int username_text_x;
int username_text_y;

int password_text_x;
int password_text_y;

int username_bar_x;
int username_bar_y;
int username_bar_x_len = 174;
int username_bar_y_len = 28;

int password_bar_x;
int password_bar_y;
int password_bar_x_len = 174;
int password_bar_y_len = 28;

int log_in_x;
int log_in_y;
int log_in_x_len = 87;
int log_in_y_len = 35;

int new_char_x;
int new_char_y;
int new_char_x_len = 138;
int new_char_y_len = 35;

char log_in_button_selected = 0;
char new_char_button_selected = 0;

int resize_login_handler (window_info *win, Uint32 w, Uint32 h)
{
	int half_screen_x = w / 2;
	int half_screen_y = h / 2;
	int len1 = strlen (login_username_str);
	int len2 = strlen (login_password_str);
	int offset = 20 + (len1 > len2 ? (len1+1) * 16 : (len2+1) * 16);
	
	username_text_x = half_screen_x - offset;
	username_text_y = half_screen_y - 130;

	password_text_x = half_screen_x - offset;
	password_text_y = half_screen_y - 100;

	username_bar_x = half_screen_x - 50;
	username_bar_y = username_text_y - 7;

	password_bar_x = half_screen_x - 50;
	password_bar_y = password_text_y - 7;

	log_in_x = half_screen_x - 125;
	log_in_y = half_screen_y - 50;

	new_char_x = half_screen_x + 50;
	new_char_y = half_screen_y - 50;
	
	return 1;
}

// the code was removed from draw_login_screen () in interface.c since I don't
// want to introduce new global variables, but the mouseover and click handlers
// need to know the positions of the buttons and input fields. The other option
// was to pass (a struct of) 24 integers to draw_login_screen, which seemed a 
// bit excessive.
int display_login_handler (window_info *win)
{
	char str[20];
	float selected_bar_u_start = (float)0/256;
	float selected_bar_v_start = 1.0f - (float)0/256;

	float selected_bar_u_end = (float)174/256;
	float selected_bar_v_end = 1.0f - (float)28/256;

	float unselected_bar_u_start = (float)0/256;
	float unselected_bar_v_start = 1.0f - (float)40/256;

	float unselected_bar_u_end = (float)170/256;
	float unselected_bar_v_end = 1.0f - (float)63/256;
	/////////////////////////
	float log_in_unselected_start_u = (float)0/256;
	float log_in_unselected_start_v = 1.0f - (float)80/256;

	float log_in_unselected_end_u = (float)87/256;
	float log_in_unselected_end_v = 1.0f - (float)115/256;

	float log_in_selected_start_u = (float)0/256;
	float log_in_selected_start_v = 1.0f - (float)120/256;

	float log_in_selected_end_u = (float)87/256;
	float log_in_selected_end_v = 1.0f-(float)155/256;
	/////////////////////////
	float new_char_unselected_start_u = (float)100/256;
	float new_char_unselected_start_v = 1.0f-(float)80/256;

	float new_char_unselected_end_u = (float)238/256;
	float new_char_unselected_end_v = 1.0f-(float)115/256;

	float new_char_selected_start_u = (float)100/256;
	float new_char_selected_start_v = 1.0f-(float)120/256;

	float new_char_selected_end_u = (float)238/256;
	float new_char_selected_end_v = 1.0f-(float)155/256;

	draw_console_pic(login_text);

	// ok, start drawing the interface...
	sprintf (str, "%s: ", login_username_str);
	draw_string (username_text_x, username_text_y, str, 1);
	sprintf (str, "%s: ", login_password_str);
	draw_string (password_text_x, password_text_y, str, 1);

	// start drawing the actual interface pieces
	get_and_set_texture_id (login_screen_menus);
	glColor3f (1.0f,1.0f,1.0f);
	glBegin (GL_QUADS);

	// username box
	if (username_box_selected)
		draw_2d_thing (selected_bar_u_start, selected_bar_v_start, selected_bar_u_end, selected_bar_v_end, username_bar_x, username_bar_y, username_bar_x + username_bar_x_len, username_bar_y + username_bar_y_len);
	else
		draw_2d_thing (unselected_bar_u_start, unselected_bar_v_start, unselected_bar_u_end, unselected_bar_v_end, username_bar_x, username_bar_y, username_bar_x + username_bar_x_len, username_bar_y + username_bar_y_len);

	// password box
	if (password_box_selected)
		draw_2d_thing (selected_bar_u_start, selected_bar_v_start, selected_bar_u_end, selected_bar_v_end, password_bar_x, password_bar_y, password_bar_x + password_bar_x_len, password_bar_y + password_bar_y_len);
	else
		draw_2d_thing (unselected_bar_u_start, unselected_bar_v_start, unselected_bar_u_end, unselected_bar_v_end, password_bar_x, password_bar_y, password_bar_x + password_bar_x_len, password_bar_y + password_bar_y_len);

	// log in button
	if (log_in_button_selected)
		draw_2d_thing (log_in_selected_start_u, log_in_selected_start_v, log_in_selected_end_u, log_in_selected_end_v, log_in_x, log_in_y, log_in_x + log_in_x_len, log_in_y + log_in_y_len);
	else
		draw_2d_thing (log_in_unselected_start_u, log_in_unselected_start_v, log_in_unselected_end_u, log_in_unselected_end_v, log_in_x, log_in_y, log_in_x + log_in_x_len, log_in_y + log_in_y_len);

	// new char button
	if (new_char_button_selected)
		draw_2d_thing (new_char_selected_start_u, new_char_selected_start_v, new_char_selected_end_u, new_char_selected_end_v, new_char_x, new_char_y, new_char_x + new_char_x_len, new_char_y + new_char_y_len);
	else
		draw_2d_thing (new_char_unselected_start_u, new_char_unselected_start_v, new_char_unselected_end_u, new_char_unselected_end_v, new_char_x, new_char_y, new_char_x + new_char_x_len, new_char_y + new_char_y_len);
		
	glEnd();

	glColor3f (0.0f, 0.9f, 1.0f);
	draw_string (username_bar_x + 4, username_text_y, username_str, 1);
	draw_string (password_bar_x + 4, password_text_y, display_password_str, 1);
	glColor3f (1.0f, 0.0f, 0.0f);
	// print the current error, if any
	draw_string (0, log_in_y + 40, log_in_error_str, 5);
	
	check_gl_errors ();
	draw_delay = 20;
	return 1;
}

int mouseover_login_handler (window_info *win, int mx, int my)
{
	// check to see if the log in button is active, or not
	if (mx >= log_in_x && mx <= log_in_x + log_in_x_len && my >= log_in_y && my <= log_in_y + log_in_y_len && username_str[0] && password_str[0])
		log_in_button_selected = 1;
	else
		log_in_button_selected = 0;
	
	// check to see if the new char button is active, or not
	if (mx >= new_char_x && mx <= new_char_x + new_char_x_len && my >= new_char_y && my <= new_char_y + new_char_y_len)
		new_char_button_selected = 1;
	else
		new_char_button_selected = 0;

	return 1;
}

int click_login_handler (window_info *win, int mx, int my, Uint32 flags)
{
	int left_click = flags & ELW_LEFT_MOUSE;

	// check to see if we clicked on the username box
	if (mx >= username_bar_x && mx <= username_bar_x + username_bar_x_len && my >= username_bar_y && my <= username_bar_y + username_bar_y_len && left_click)
	{
		username_box_selected = 1;
		password_box_selected = 0;
	}
	// check to see if we clicked on the password box
	else if (mx >= password_bar_x && mx <= password_bar_x + password_bar_x_len && my >= password_bar_y && my <= password_bar_y + password_bar_y_len && left_click)
	{
		username_box_selected = 0;
		password_box_selected = 1;
	}
	// check to see if we clicked on the ACTIVE Log In button
	if (log_in_button_selected && left_click)
	{
		send_login_info ();
		// XXX FIXME (Grum): figure out how to do this cleanly
		//left_click=2;//don't relogin 100 times like a moron
	}
	//check to see if we clicked on the ACTIVE New Char button
	else if (new_char_button_selected && left_click)
	{
		// don't destroy the login window just yet, the user might 
		// click the back button
		hide_window (login_win);
		create_newchar_window ();
		if (last_display == -1)
		{
			create_rules_root_window (newchar_win, 15);
			show_window (rules_root_win);
			interface_mode = interface_rules;
		}
		else 
		{
			show_window (newchar_win);
			interface_mode = interface_new_char;
		}
		// XXX FIXME (Grum): figure out how to do this cleanly
		//left_click=2;
	}
	return 1;
}

int keypress_login_handler (window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
	Uint8 ch = key_to_char (unikey);
	
	// First check key presses common to all root windows. Many of these
	// don't make sense at this point, but it should be harmless.
	if ( keypress_root_common (key, unikey) )
	{
		return 1;
	}	
	else if (ch == SDLK_RETURN && username_str[0] && password_str[0])
	{
		send_login_info();
	}
	else if (ch == SDLK_TAB)
	{
		username_box_selected = !username_box_selected;
		password_box_selected = !password_box_selected;
	}
	else if (username_box_selected)
	{
		add_char_to_username (ch);
	} 
	else
	{
		add_char_to_password (ch);
	}
	
	return 1;
}

void create_login_window ()
{
	if (login_win < 0)
	{
		login_win = create_window ("Login", -1, -1, 0, 0, window_width, window_height, ELW_TITLE_NONE|ELW_SHOW_LAST);

                set_window_handler (login_win, ELW_HANDLER_DISPLAY, &display_login_handler);		
                set_window_handler (login_win, ELW_HANDLER_MOUSEOVER, &mouseover_login_handler);		
                set_window_handler (login_win, ELW_HANDLER_CLICK, &click_login_handler);		
                set_window_handler (login_win, ELW_HANDLER_KEYPRESS, &keypress_login_handler);
                set_window_handler (login_win, ELW_HANDLER_RESIZE, &resize_login_handler);
		
		resize_window (login_win, window_width, window_height);	
	}
}

#endif // def WINDOW_CHAT
