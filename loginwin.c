#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "loginwin.h"
#include "asc.h"
#include "books.h"
#include "draw_scene.h"
#include "elconfig.h"
#include "elwindows.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud.h"
#include "interface.h"
#include "multiplayer.h"
#include "new_character.h"
#include "password_manager.h"
#include "rules.h"
#include "sound.h"
#include "tabs.h"
#include "textures.h"
#include "translate.h"

int login_root_win = -1;
int login_text = -1;

static int username_field_id = 0;
static int password_field_id = 1;

static char username_box_selected = 1;
static char password_box_selected = 0;

static int game_buttons;
static int login_screen_menus;

static char log_in_error_str[520] = {0};

static int username_text_x;
static int username_text_y;
static int password_text_x;
static int password_text_y;

static int username_bar_x;
static int username_bar_y;
static int username_bar_x_len = 0;
static int username_bar_y_len = 0;

static int password_bar_x;
static int password_bar_y;
static int password_bar_x_len = 0;
static int password_bar_y_len = 0;

static int passmngr_button_mouse_over = 0;
static int passmngr_button_x = 0;
static int passmngr_button_y = 0;
static int passmngr_button_size = 0;

static int log_in_x;
static int log_in_y;
static int log_in_x_len = 0;
static int log_in_y_len = 0;

static int new_char_x;
static int new_char_y;
static int new_char_x_len = 0;
static int new_char_y_len = 0;

static int settings_x;
static int settings_y;
static int settings_x_len = 0;
static int settings_y_len = 0;

static int num_rules_lines = 0;

static char log_in_button_selected = 0;
static char new_char_button_selected = 0;
static char settings_button_selected = 0;

char active_username_str[MAX_USERNAME_LENGTH]={0};
char active_password_str[MAX_USERNAME_LENGTH]={0};

static char input_username_str[MAX_USERNAME_LENGTH]={0};
static char input_password_str[MAX_USERNAME_LENGTH]={0};
static char lower_username_str[MAX_USERNAME_LENGTH]={0};
static int username_initial = 1;
static int password_initial = 1;

#define SELBOX_X_LEN 174
#define SELBOX_Y_LEN 28
#define UNSELBOX_X_LEN 170
#define UNSELBOX_Y_LEN 23
#define LOGIN_BUTTON_X_LEN 87
#define SETTINGS_BUTTON_X_LEN 87
#define NEW_CHAR_BUTTON_X_LEN 138
#define BUTTON_Y_LEN 35

typedef enum
{
	// Keypress not valid for this input field
	KP_INVALID = 0,
	// Regular input character
	KP_INPUT,
	// Cursor control, i.e. arrow, home/end, etc.
	KP_CURSOR
} keypress_type;

static void toggle_selected_box(int which)
{
	Uint32 sel_flags = PWORD_FIELD_NO_BORDER|PWORD_FIELD_NO_KEYPRESS;
	Uint32 unsel_flags = PWORD_FIELD_NO_BORDER|PWORD_FIELD_NO_KEYPRESS|PWORD_FIELD_NO_CURSOR;
	if (which == username_field_id)
	{
		sel_flags |= username_initial ? PWORD_FIELD_NO_CURSOR : PWORD_FIELD_DRAW_CURSOR;
		widget_set_flags(login_root_win, username_field_id, sel_flags);
		widget_set_flags(login_root_win, password_field_id, unsel_flags);
		username_box_selected = 1;
		password_box_selected = 0;
	}
	else
	{
		sel_flags |= password_initial ? PWORD_FIELD_NO_CURSOR : PWORD_FIELD_DRAW_CURSOR;
		widget_set_flags(login_root_win, username_field_id, unsel_flags);
		widget_set_flags(login_root_win, password_field_id, sel_flags);
		username_box_selected = 0;
		password_box_selected = 1;
	}
}

static int select_username_box()
{
	toggle_selected_box(username_field_id);
	return 1;
}

static int select_password_box()
{
	toggle_selected_box(password_field_id);
	return 1;
}

void init_login_screen (void)
{
	CHECK_GL_ERRORS();
	game_buttons = load_texture_cached("textures/gamebuttons.dds", tt_image);
	login_screen_menus = load_texture_cached("textures/login_menu.dds", tt_image);
	login_text = load_texture_cached("textures/login_back.dds", tt_image);
	CHECK_GL_ERRORS();

	set_username(active_username_str);
	set_password(active_password_str);
	passmngr_init();
	passmngr_set_login();

	if (strlen(get_username()) && !strlen(get_password()))
		select_password_box();
}

void set_login_error (const char *msg, int len, int print_err)
{
#ifdef NEW_SOUND
	int snd;
#endif // NEW_SOUND
	if (len <= 0)
	{
		// server didn't send a message, use the default
		safe_snprintf (log_in_error_str, sizeof(log_in_error_str), "%s: %s", reg_error_str, invalid_pass);
	}
	else if (print_err)
	{
		safe_snprintf (log_in_error_str, sizeof (log_in_error_str), "%s: %.*s", reg_error_str, len, msg);
	}
	else
	{
		safe_strncpy2 (log_in_error_str, msg, sizeof (log_in_error_str), len);
	}

#ifdef NEW_SOUND
	if ((snd = get_index_for_sound_type_name("Login Error")) > -1)
		add_sound_object(snd, 0, 0, 1);
#endif // NEW_SOUND
}

static int resize_login_handler (window_info *win, Uint32 w, Uint32 h)
{
	int button_y_len = (int)(0.5 + win->current_scale * BUTTON_Y_LEN);
	int half_screen_x = w / 2;
	int half_screen_y = h / 2;
	int username_str_len_x = get_string_width_zoom((const unsigned char*)login_username_str,
		win->font_category, win->current_scale);
	int password_str_len_x = get_string_width_zoom((const unsigned char*)login_username_str,
		win->font_category, win->current_scale);
	int max_login_str = max2i(username_str_len_x, password_str_len_x);
	int height = 0, max_width = 0, login_sep_x = 0, button_sep_x = 0;
	int normal_char_width = (int)(0.5 + win->current_scale * DEFAULT_FIXED_FONT_WIDTH);

	username_bar_x_len = password_bar_x_len = 10 * (MAX_USERNAME_LENGTH-1) * normal_char_width / 9;
	username_bar_y_len = password_bar_y_len = 1.5 * win->default_font_len_y;
	log_in_y_len = new_char_y_len = settings_y_len = button_y_len;
	passmngr_button_size = (int)(0.5 + win->current_scale * 32);

	log_in_x_len = (int)(0.5 + win->current_scale * LOGIN_BUTTON_X_LEN);
	new_char_x_len = (int)(0.5 + win->current_scale * NEW_CHAR_BUTTON_X_LEN);
	settings_x_len = (int)(0.5 + win->current_scale * SETTINGS_BUTTON_X_LEN);

	max_width = max2i(max_login_str + username_bar_x_len + passmngr_button_size, log_in_x_len + new_char_x_len + settings_x_len) + 3 * win->default_font_max_len_x;
	login_sep_x = (max_width - max_login_str - username_bar_x_len - passmngr_button_size) / 2;
	button_sep_x = (max_width - log_in_x_len - new_char_x_len - settings_x_len) / 2;

	username_text_x = password_text_x = half_screen_x - max_width / 2;
	username_bar_x = password_bar_x = username_text_x + max_login_str + login_sep_x;
	passmngr_button_x = username_bar_x + username_bar_x_len + login_sep_x;
	log_in_x = username_text_x;
	new_char_x = log_in_x + log_in_x_len + button_sep_x;
	settings_x = new_char_x + new_char_x_len + button_sep_x;

	num_rules_lines = reset_soft_breaks((unsigned char*)login_rules_str,
		strlen(login_rules_str), sizeof(login_rules_str), UI_FONT, win->current_scale,
		max_width, NULL, NULL);

	height = username_bar_y_len + password_bar_y_len + button_y_len + (3 + num_rules_lines) * win->default_font_len_y;
	username_bar_y = passmngr_button_y = half_screen_y - height / 2;
	username_text_y = username_bar_y + username_bar_y_len/4;
	password_bar_y = username_bar_y + username_bar_y_len + win->default_font_len_y;
	password_text_y = password_bar_y + password_bar_y_len/4;
	log_in_y = settings_y = new_char_y = password_bar_y + username_bar_y_len + win->default_font_len_y;

	widget_move(win->window_id, username_field_id,
		username_bar_x + 0.025 * username_bar_x_len, username_text_y);
	widget_resize(win->window_id, username_field_id, (1.0 - 2*0.025) * username_bar_x_len,
		username_bar_y + username_bar_y_len - username_text_y);
	widget_set_size(win->window_id, username_field_id, win->current_scale);

	widget_move(win->window_id, password_field_id,
		password_bar_x + 0.025 * password_bar_x_len, password_text_y);
	widget_resize(win->window_id, password_field_id, (1.0 - 2*0.025) * username_bar_x_len,
		password_bar_y + password_bar_y_len - password_text_y);
	widget_set_size(win->window_id, password_field_id, win->current_scale);

	passmngr_resize();

	return 1;
}

// the code was removed from draw_login_screen () in interface.c since I don't
// want to introduce new global variables, but the mouseover and click handlers
// need to know the positions of the buttons and input fields. The other option
// was to pass (a struct of) 24 integers to draw_login_screen, which seemed a
// bit excessive.
static int display_login_handler (window_info *win)
{
	float selected_bar_u_start = (float)0/256;
	float selected_bar_v_start = (float)0/256;

	float selected_bar_u_end = (float)SELBOX_X_LEN/256;
	float selected_bar_v_end = (float)SELBOX_Y_LEN/256;

	float unselected_bar_u_start = (float)0/256;
	float unselected_bar_v_start = (float)40/256;

	float unselected_bar_u_end = (float)UNSELBOX_X_LEN/256;
	float unselected_bar_v_end = (float)(40+UNSELBOX_Y_LEN)/256;
	/////////////////////////
	float log_in_unselected_start_u = (float)0/256;
	float log_in_unselected_start_v = (float)80/256;

	float log_in_unselected_end_u = (float)LOGIN_BUTTON_X_LEN/256;
	float log_in_unselected_end_v = (float)(80+BUTTON_Y_LEN)/256;

	float log_in_selected_start_u = (float)0/256;
	float log_in_selected_start_v = (float)120/256;

	float log_in_selected_end_u = (float)LOGIN_BUTTON_X_LEN/256;
	float log_in_selected_end_v = (float)(120+BUTTON_Y_LEN)/256;
	/////////////////////////
	float new_char_unselected_start_u = (float)100/256;
	float new_char_unselected_start_v = (float)80/256;

	float new_char_unselected_end_u = (float)(100+NEW_CHAR_BUTTON_X_LEN)/256;
	float new_char_unselected_end_v = (float)(80+BUTTON_Y_LEN)/256;

	float new_char_selected_start_u = (float)100/256;
	float new_char_selected_start_v = (float)120/256;

	float new_char_selected_end_u = (float)(100+NEW_CHAR_BUTTON_X_LEN)/256;
	float new_char_selected_end_v = (float)(120+BUTTON_Y_LEN)/256;
	/////////////////////////
	float settings_unselected_start_u = (float)0/256;
	float settings_unselected_start_v = (float)160/256;

	float settings_unselected_end_u = (float)SETTINGS_BUTTON_X_LEN/256;
	float settings_unselected_end_v = (float)(160+BUTTON_Y_LEN)/256;

	float settings_selected_start_u = (float)0/256;
	float settings_selected_start_v = (float)200/256;

	float settings_selected_end_u = (float)SETTINGS_BUTTON_X_LEN/256;
	float settings_selected_end_v = (float)(200+BUTTON_Y_LEN)/256;

	float select_uoffset = 31.0/256.0, select_voffset = 31.0/256.0;
	float select_u[2] = {32.0 * (float)(10 % 8)/256.0, 32.0 * (float)(24 % 8)/256.0 };
	float select_v[2] = {32.0 * (float)(10 >> 3)/256.0, select_v[1] = 32.0 * (float)(24 >> 3)/256.0 };

	draw_console_pic(login_text);

	// ok, start drawing the interface...
	draw_text(username_text_x, username_text_y, (const unsigned char*)login_username_str,
		strlen(login_username_str), win->font_category, TDO_ZOOM, win->current_scale, TDO_END);
	draw_text(username_text_x, password_text_y, (const unsigned char*)login_password_str,
		strlen(login_password_str), win->font_category, TDO_ZOOM, win->current_scale, TDO_END);

	draw_string_zoomed(username_text_x, log_in_y + log_in_y_len + win->default_font_len_y, (unsigned char*)login_rules_str, num_rules_lines, win->current_scale);

	bind_texture(game_buttons);
	glColor3f (1.0f,1.0f,1.0f);
	glBegin (GL_QUADS);
	if (passmngr_button_mouse_over)
		draw_2d_thing( select_u[1], select_v[1], select_u[1]+select_uoffset, select_v[1]+select_voffset, passmngr_button_x, passmngr_button_y, passmngr_button_x + passmngr_button_size, passmngr_button_y + passmngr_button_size);
	else
		draw_2d_thing( select_u[0], select_v[0], select_u[0]+select_uoffset, select_v[0]+select_voffset, passmngr_button_x, passmngr_button_y, passmngr_button_x + passmngr_button_size, passmngr_button_y + passmngr_button_size);
	glEnd();
	if (passmngr_button_mouse_over)
	{
		const unsigned char* msg = (const unsigned char*)(passmngr_enabled ? passmngr_enabled_str : passmngr_disabled_str);
		draw_string_zoomed_centered(win->len_x/2, passmngr_button_y - 1.25 * win->default_font_len_y, msg, 1, win->current_scale);
	}

	// start drawing the actual interface pieces
	bind_texture(login_screen_menus);
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

	// settings button
	if (settings_button_selected)
		draw_2d_thing (settings_selected_start_u, settings_selected_start_v, settings_selected_end_u, settings_selected_end_v, settings_x, settings_y, settings_x + settings_x_len, settings_y + settings_y_len);
	else
		draw_2d_thing (settings_unselected_start_u, settings_unselected_start_v, settings_unselected_end_u, settings_unselected_end_v, settings_x, settings_y, settings_x + settings_x_len, settings_y + settings_y_len);

	glEnd();

	// print the current error, if any
	if (strlen (log_in_error_str))
	{
		int max_win_width = window_width - 2 * win->default_font_max_len_x;
		float max_line_width = 0;
		int num_lines = reset_soft_breaks((unsigned char*)log_in_error_str,
			strlen(log_in_error_str), sizeof (log_in_error_str), UI_FONT,
			win->current_scale, max_win_width, NULL, &max_line_width);
		glColor3f (1.0f, 0.0f, 0.0f);
		draw_string_zoomed_centered(window_width/2, username_bar_y - (num_lines + 2) * win->default_font_len_y, (const unsigned char*)log_in_error_str, num_lines, win->current_scale);
	}

	CHECK_GL_ERRORS ();
	draw_delay = 20;
	return 1;
}

static int mouseover_login_handler (window_info *win, int mx, int my)
{
	// check to see if the log in button is active, or not
	if (mx >= log_in_x && mx <= log_in_x + log_in_x_len && my >= log_in_y && my <= log_in_y + log_in_y_len && input_username_str[0] && input_password_str[0])
		log_in_button_selected = 1;
	else
		log_in_button_selected = 0;

	// check to see if the new char button is active, or not
	if (mx >= new_char_x && mx <= new_char_x + new_char_x_len && my >= new_char_y && my <= new_char_y + new_char_y_len)
		new_char_button_selected = 1;
	else
		new_char_button_selected = 0;

	// check to see if the settings button is active, or not
	if (mx >= settings_x && mx <= settings_x + settings_x_len && my >= settings_y && my <= settings_y + settings_y_len)
		settings_button_selected = 1;
	else
		settings_button_selected = 0;

	if (mx >= passmngr_button_x && mx <= passmngr_button_x + passmngr_button_size && my >= passmngr_button_y && my <= passmngr_button_y + passmngr_button_size)
		passmngr_button_mouse_over = 1;
	else
		passmngr_button_mouse_over = 0;

	return 1;
}

static int click_login_handler (window_info *win, int mx, int my, Uint32 flags)
{
	int left_click = flags & ELW_LEFT_MOUSE;
	extern int force_elconfig_win_ontop;
	force_elconfig_win_ontop = 0;

	if (left_click == 0) return 0;

	// check to see if we clicked on the username box
	if (mx >= username_bar_x && mx <= username_bar_x + username_bar_x_len && my >= username_bar_y && my <= username_bar_y + username_bar_y_len)
	{
		select_username_box();
	}
	// check to see if we clicked on the password box
	else if (mx >= password_bar_x && mx <= password_bar_x + password_bar_x_len && my >= password_bar_y && my <= password_bar_y + password_bar_y_len)
	{
		select_password_box();
	}
	// check to see if we clicked login select button
	else if (mx >= passmngr_button_x && mx <= passmngr_button_x + passmngr_button_size && my >= passmngr_button_y && my <= passmngr_button_y + passmngr_button_size)
	{
		log_in_error_str[0] = '\0';
		if (passmngr_enabled)
		{
			do_click_sound();
			passmngr_open_window();
		}
		else
			do_alert1_sound();
	}
	// check to see if we clicked on the ACTIVE Log In button
	if (log_in_button_selected)
	{
		log_in_error_str[0] = '\0';
		set_username(input_username_str);
		set_password(input_password_str);
		passmngr_destroy_window();
		send_login_info ();
	}
	//check to see if we clicked on the ACTIVE New Char button
	else if (new_char_button_selected)
	{
		// don't destroy the login window just yet, the user might
		// click the back button
		hide_window (login_root_win);
		create_newchar_root_window ();
		passmngr_destroy_window();
		if (last_display == -1)
		{
			create_rules_root_window (win->len_x, win->len_y, newchar_root_win, 15);
			show_window (rules_root_win);
		}
		else
		{
			show_window (newchar_root_win);
		}
	}
	// to see if we clicked on the ACTIVE settings button
	else if (settings_button_selected)
	{
		force_elconfig_win_ontop = 1;
		view_window(MW_CONFIG);
	}
	return 1;
}

static keypress_type special_keypress_ok(SDL_Keycode key_code)
{
	switch (key_code)
	{
		case SDLK_DELETE:
		case SDLK_BACKSPACE:
			return KP_INPUT;
		case SDLK_LEFT:
		case SDLK_RIGHT:
		case SDLK_HOME:
		case SDLK_END:
			return KP_CURSOR;
	}
	return KP_INVALID;
}

static keypress_type username_keypress_ok(SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	Uint8 ch = key_to_char(key_unicode);
	if ((ch >= '0' && ch <= '9') || (ch>='A' && ch<='Z') || (ch>='a' && ch<='z') || ch=='_')
		return KP_INPUT;
	return special_keypress_ok(key_code);
}

static keypress_type password_keypress_ok(SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	return VALID_PASSWORD_CHAR(key_unicode) ? KP_INPUT : special_keypress_ok(key_code);
}

static int keypress_login_handler (window_info *win, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	// First check key presses common to all root windows. Many of these
	// don't make sense at this point, but it should be harmless.
	if ( keypress_root_common (key_code, key_unicode, key_mod) )
		return 1;
	else if ((key_code == SDLK_RETURN || key_code == SDLK_KP_ENTER) && input_username_str[0] && input_password_str[0])
	{
		log_in_error_str[0] = '\0';
		set_username(input_username_str);
		set_password(input_password_str);
		passmngr_destroy_window();
		send_login_info();
		return 1;
	}
	else if (key_code == SDLK_TAB)
	{
		toggle_selected_box(username_box_selected ? password_field_id : username_field_id);
		return 1;
	}
	else if (username_box_selected)
	{
		keypress_type kp_type = username_keypress_ok(key_code, key_unicode, key_mod);
		log_in_error_str[0] = '\0';
		if (kp_type)
		{
			widget_list *field = widget_find(win->window_id, username_field_id);
			if (username_initial && kp_type == KP_INPUT)
				pword_clear(login_root_win, username_field_id);
			widget_unset_flags(win->window_id, username_field_id, PWORD_FIELD_NO_KEYPRESS);
			widget_handle_keypress(field, 0, 0, key_code, key_unicode, key_mod);
			widget_set_flags(login_root_win, username_field_id, PWORD_FIELD_NO_BORDER|PWORD_FIELD_NO_KEYPRESS|PWORD_FIELD_DRAW_CURSOR);
			username_initial = 0;
		}
	}
	else
	{
		keypress_type kp_type = password_keypress_ok(key_code, key_unicode, key_mod);
		log_in_error_str[0] = '\0';
		if (kp_type)
		{
			widget_list *field = widget_find(win->window_id, password_field_id);
			if (password_initial && kp_type == KP_INPUT)
				pword_clear(login_root_win, password_field_id);
			widget_unset_flags(win->window_id, password_field_id, PWORD_FIELD_NO_KEYPRESS);
			widget_handle_keypress(field, 0, 0, key_code, key_unicode, key_mod);
			widget_set_flags(login_root_win, password_field_id, PWORD_FIELD_NO_BORDER|PWORD_FIELD_NO_KEYPRESS|PWORD_FIELD_DRAW_CURSOR);
			password_initial = 0;
		}
	}
	return 0;
}

static int show_login_handler(window_info * win)
{
	close_book_window();
	hide_window_MW(MW_CONFIG);
	hide_window_MW(MW_HELP);
	return 1;
}

static int ui_scale_login_handler(window_info *win)
{
	resize_window(win->window_id, win->len_x, win->len_y);
	return 1;
}

static int change_login_font_handler(window_info *win, font_cat cat)
{
	if (cat != win->font_category)
		return 0;
	resize_window(win->window_id, win->len_x, win->len_y);
	return 1;
}

void create_login_root_window (int width, int height)
{
	if (login_root_win < 0)
	{
		login_root_win = create_window ("Login", -1, -1, 0, 0, width, height, ELW_USE_UISCALE|ELW_TITLE_NONE|ELW_SHOW_LAST);

		set_window_handler (login_root_win, ELW_HANDLER_DISPLAY, &display_login_handler);
		set_window_handler (login_root_win, ELW_HANDLER_MOUSEOVER, &mouseover_login_handler);
		set_window_handler (login_root_win, ELW_HANDLER_CLICK, &click_login_handler);
		set_window_handler (login_root_win, ELW_HANDLER_KEYPRESS, (int (*)())&keypress_login_handler);
		set_window_handler (login_root_win, ELW_HANDLER_RESIZE, &resize_login_handler);
		set_window_handler (login_root_win, ELW_HANDLER_SHOW, &show_login_handler);
		set_window_handler (login_root_win, ELW_HANDLER_UI_SCALE, &ui_scale_login_handler);
		set_window_handler (login_root_win, ELW_HANDLER_FONT_CHANGE, &change_login_font_handler);

		username_field_id = pword_field_add_extended(login_root_win, username_field_id, NULL,
			0, 0, 0, 0, P_TEXT, 1.0, (unsigned char*)input_username_str, MAX_USERNAME_LENGTH);
		widget_set_color(login_root_win, username_field_id, 0.0f, 0.9f, 1.0f);
		widget_set_flags(login_root_win, username_field_id, PWORD_FIELD_NO_BORDER|PWORD_FIELD_NO_KEYPRESS|PWORD_FIELD_NO_CURSOR);
		widget_set_OnClick(login_root_win, username_field_id, select_username_box);
		password_field_id = pword_field_add_extended(login_root_win, password_field_id, NULL,
			0, 0, 0, 0, P_NORMAL, 1.0, (unsigned char*)input_password_str, MAX_USERNAME_LENGTH);
		widget_set_color(login_root_win, password_field_id, 0.0f, 0.9f, 1.0f);
		widget_set_flags(login_root_win, password_field_id, PWORD_FIELD_NO_BORDER|PWORD_FIELD_NO_KEYPRESS|PWORD_FIELD_NO_CURSOR);
		widget_set_OnClick(login_root_win, password_field_id, select_password_box);

		resize_window (login_root_win, width, height);
	}
}

const char * get_username(void)
{
	return active_username_str;
}

const char * get_lowercase_username(void)
{
	return lower_username_str;
}

const char * get_password(void)
{
	return active_password_str;
}

void set_username(const char * new_username)
{
	if (strcmp(active_username_str, new_username) != 0)
	{
		safe_strncpy(active_username_str, new_username, MAX_USERNAME_LENGTH);
		if (passmngr_enabled)
			set_var_unsaved("username", INI_FILE_VAR);
	}
	if (strcmp(input_username_str, new_username) != 0)
	{
		if (login_root_win >= 0)
			pword_field_set_content(login_root_win, username_field_id,
				(const unsigned char*)new_username, strlen(new_username));
		else
			safe_strncpy(input_username_str, new_username, MAX_USERNAME_LENGTH);
	}
	if (strcmp(lower_username_str, new_username) != 0)
	{
		safe_strncpy(lower_username_str, new_username, MAX_USERNAME_LENGTH);
		my_tolower(lower_username_str);
	}
	username_initial = 1;
}

void set_password(const char * new_password)
{
	if (strcmp(active_password_str, new_password) != 0)
		safe_strncpy(active_password_str, new_password, MAX_USERNAME_LENGTH);
	if (strcmp(input_password_str, new_password) != 0)
	{
		if (login_root_win >= 0)
			pword_field_set_content(login_root_win, password_field_id,
				(const unsigned char*)new_password, strlen(new_password));
		else
			safe_strncpy(input_password_str, new_password, MAX_USERNAME_LENGTH);
	}
	password_initial = 1;
}

int valid_username_pasword(void)
{
	int i, username_len, password_len;

	username_len = strlen(get_username());
	if (username_len < 3)
	{
		set_login_error (error_username_length, strlen (error_username_length), 1);
		return 0;
	}

	password_len = strlen(get_password());
	if (password_len < 4)
	{
		set_login_error (error_password_length, strlen (error_password_length), 1);
		return 0;
	}

	for (i=0; i<strlen(active_password_str); i++)
		if (!(VALID_PASSWORD_CHAR(active_password_str[i])))
		{
			set_login_error (error_bad_pass, strlen (error_bad_pass), 1);
			return 0;
		}

	return 1;
}
