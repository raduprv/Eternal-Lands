#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include "session.h"
#include "actors.h"
#include "asc.h"
#include "elwindows.h"
#include "init.h"
#include "hud.h"
#include "missiles.h"
#include "multiplayer.h"
#include "named_colours.h"
#include "platform.h"
#include "sound.h"
#include "stats.h"
#include "tabs.h"
#include "translate.h"
#include "counters.h"
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif
#include "widgets.h"

static const unsigned char* skill_label = (const unsigned char*)"Skill";
static const unsigned char* total_exp_label = (const unsigned char*)"Total Exp";
static const unsigned char* max_exp_label = (const unsigned char*)"Max Exp";
static const unsigned char* last_exp_label = (const unsigned char*)"Last Exp";
static const unsigned char* session_time_label = (const unsigned char*)"Session Time";
static const unsigned char* exp_per_min_label = (const unsigned char*)"Exp/Min";
static const unsigned char* distance_label = (const unsigned char*)"Distance";

int exp_log_threshold = 5000;
static int reconnecting = 0;
static int last_port = -1;
static unsigned char last_server_address[60];
static int reset_button_id = -1;
static int show_reset_help = 0;
static int last_mouse_click_skill = -1;
static int last_mouse_over_skill = -1;
static int start_skills_y = -1;
static int distance_moved = -1;

static int x_border = 0;
static int y_offset = 0;
static int y_step = 0;
static int tot_exp_left = 0;
static int tot_exp_right = 0;
static int max_exp_right = 0;
static int last_exp_right = 0;


static Uint32 session_exp[NUM_SKILLS];
static Uint32 max_exp[NUM_SKILLS];
Uint32 last_exp[NUM_SKILLS];

static Uint32 session_start_time;

int display_session_handler(window_info *win);

int get_session_exp_ranging(void)
{
	return *(statsinfo[SI_RAN].exp) - session_exp[SI_RAN];
}

static int mouseover_session_reset_handler(void)
{
	if (!disable_double_click && show_help_text)
		show_reset_help = 1;
	return 0;
}

static int click_session_handler(window_info *win, int mx, int my, Uint32 flags)
{
	if (flags & (ELW_WHEEL_UP|ELW_WHEEL_DOWN))
		return 0;
	if ((my > start_skills_y) && (my < (start_skills_y + NUM_SKILLS * y_step)))
	{
		last_mouse_click_skill = (my - start_skills_y) / y_step;
		do_click_sound();
	}
	return 1;
}

static int mouseover_session_handler(window_info *win, int mx, int my)
{
	if ((my > start_skills_y) && (my < (start_skills_y + NUM_SKILLS * y_step)))
		last_mouse_over_skill = (my - start_skills_y) / y_step;;
	return 1;
}

static void set_content_widths(window_info *win)
{
	float zoom = win->current_scale_small;
	int sep_width = 2 * win->small_font_max_len_x;
	int max_val_width = get_string_width_zoom((const unsigned char*)"88888888",
		win->font_category, zoom);
	int tot_exp_width = get_string_width_zoom(total_exp_label, win->font_category, zoom);
	int max_exp_width = get_string_width_zoom(max_exp_label, win->font_category, zoom);
	int last_exp_width = get_string_width_zoom(last_exp_label, win->font_category, zoom);
	int max_name_width = get_string_width_zoom(skill_label, win->font_category, zoom);

	for (int i =0; i < NUM_SKILLS; ++i)
	{
		int width = get_string_width_zoom(statsinfo[i].skillnames->name, win->font_category, zoom);
		if (width > max_name_width)
			max_name_width = width;
	}
	max_name_width = max2i(max_name_width,
		get_string_width_zoom(session_time_label, win->font_category, zoom));
	max_name_width = max2i(max_name_width,
		get_string_width_zoom(exp_per_min_label, win->font_category, zoom));
	max_name_width = max2i(max_name_width,
		get_string_width_zoom(distance_label, win->font_category, zoom));

	x_border = (int)(win->current_scale * 10);
	y_offset = (int)(0.5 + win->default_font_len_y * 21.0 / DEFAULT_FIXED_FONT_HEIGHT);
	y_step = (int)(0.5 + win->default_font_len_y * 16.0 / DEFAULT_FIXED_FONT_HEIGHT);
	start_skills_y = y_offset + 2 * y_step;
	tot_exp_left = x_border + max_name_width + sep_width;
	tot_exp_right = tot_exp_left + max2i(max_val_width, tot_exp_width);
	max_exp_right = tot_exp_right + sep_width + max2i(max_val_width, max_exp_width);
	last_exp_right = max_exp_right + sep_width + max2i(max_val_width, last_exp_width);

	win->min_len_x = last_exp_right + x_border;
	win->min_len_y = y_offset + (NUM_SKILLS + 6) * y_step;
}

static int resize_session_handler(window_info *win, int new_width, int new_height)
{
	int width;
	set_content_widths(win);
	button_resize(win->window_id, reset_button_id, 0, 0, win->current_scale);
	width = widget_get_width(win->window_id, reset_button_id);
	widget_move(win->window_id, reset_button_id, last_exp_right - width, y_offset + 16*y_step);
	return 0;
}

static int ui_scale_session_handler(window_info *win)
{
	set_content_widths(win);
	return 1;
}

static int change_session_font_handler(window_info *win, font_cat cat)
{
	if (cat != UI_FONT)
		return 0;
	set_content_widths(win);
	return 1;
}

void fill_session_win(int window_id)
{
	set_window_custom_scale(window_id, MW_STATS);
	set_window_handler(window_id, ELW_HANDLER_DISPLAY, &display_session_handler);
	set_window_handler(window_id, ELW_HANDLER_CLICK, &click_session_handler );
	set_window_handler(window_id, ELW_HANDLER_MOUSEOVER, &mouseover_session_handler );
	set_window_handler(window_id, ELW_HANDLER_RESIZE, &resize_session_handler );
	set_window_handler(window_id, ELW_HANDLER_UI_SCALE, &ui_scale_session_handler);
	set_window_handler(window_id, ELW_HANDLER_FONT_CHANGE, &change_session_font_handler );

	reset_button_id=button_add_extended(window_id, reset_button_id, NULL, 0, 0, 0, 0, 0, 1.0f, reset_str);
	widget_set_OnClick(window_id, reset_button_id, session_reset_handler);
	widget_set_OnMouseover(window_id, reset_button_id, mouseover_session_reset_handler);

	if (window_id >= 0 && window_id < windows_list.num_windows)
		set_content_widths(&windows_list.window[window_id]);
}

void set_last_skill_exp(size_t skill, int exp)
{
	if (skill < NUM_SKILLS)
	{
		last_exp[skill] = exp;
		if (exp > max_exp[skill])
			max_exp[skill] = exp;
		if ((skill != SI_ALL) && (exp >= exp_log_threshold) && (exp_log_threshold > 0))
		{
			char str[80];
			safe_snprintf(str, sizeof(str), "You gained %d exp for %s.", exp, statsinfo[skill].skillnames->name);
			LOG_TO_CONSOLE(c_green2,str);
		}
	}
}

void set_session_exp_to_current(void)
{
	int i;
	for (i=0; i<NUM_SKILLS; i++)
	{
		max_exp[i] = last_exp[i] = 0;
		session_exp[i] = *(statsinfo[i].exp);
	}
}

void update_session_distance(void)
{
	static int last_x = -1, last_y = -1;
	actor *me = get_our_actor ();
	if (me == NULL)
		return;
	if ((me->x_tile_pos != last_x) || (me->y_tile_pos != last_y))
	{
		last_x = me->x_tile_pos;
		last_y = me->y_tile_pos;
		distance_moved++;
	}
}

static void draw_session_line(window_info *win, int x, int y, const unsigned char* skill,
	const unsigned char* tot_exp, const unsigned char* max_exp, const unsigned char* last_exp)
{
	float scale = win->current_scale;

	draw_string_small_zoomed(x, y, skill, 1, scale);
	draw_string_small_zoomed_right(tot_exp_right, y, tot_exp, 1, scale);
	draw_string_small_zoomed_right(max_exp_right, y, max_exp, 1, scale);
	draw_string_small_zoomed_right(last_exp_right, y, last_exp, 1, scale);
}

int display_session_handler(window_info *win)
{
	int i;
	Uint32 timediff;
	unsigned char tot_buf[32];
	unsigned char max_buf[32];
	unsigned char last_buf[32];
	unsigned char buffer[32];
	float oa_exp;
	int x = x_border;
	int y = y_offset;

	timediff = 0;
	oa_exp = 0.0f;

	glColor3f(1.0f, 1.0f, 1.0f);
	draw_session_line(win, x, y, skill_label, total_exp_label, max_exp_label, last_exp_label);

	y += y_step;

	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f, 0.57f, 0.39f);
	glBegin(GL_LINES);
	glVertex3i(0, y, 0);
	glVertex3i(win->len_x, y, 0);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);

	y += y_step;

	for (i=0; i<NUM_SKILLS; i++)
	{
		if (last_mouse_click_skill == i)
			elglColourN("global.mouseselected");
		else if (last_mouse_over_skill == i)
			elglColourN("global.mousehighlight");
		else
			glColor3f(1.0f, 1.0f, 1.0f);
		safe_snprintf((char*)tot_buf, sizeof(tot_buf), "%u", *(statsinfo[i].exp) - session_exp[i]);
		safe_snprintf((char*)max_buf, sizeof(tot_buf), "%u", max_exp[i]);
		safe_snprintf((char*)last_buf, sizeof(tot_buf), "%u", last_exp[i]);
		draw_session_line(win, x, y, statsinfo[i].skillnames->name, tot_buf, max_buf, last_buf);
		y += y_step;
		if(i < NUM_SKILLS-1)
			oa_exp += *(statsinfo[i].exp) - session_exp[i];
	}

	y += y_step;

	glColor3f(1.0f, 1.0f, 1.0f);
	draw_string_small_zoomed(x, y, session_time_label, 1, win->current_scale);
	timediff = cur_time - session_start_time;
	safe_snprintf((char*)buffer, sizeof(buffer), "%02d:%02d:%02d",
		timediff/3600000, (timediff/60000)%60, (timediff/1000)%60);
	draw_string_small_zoomed(tot_exp_left, y, buffer, 1, win->current_scale);

	y += y_step;

	draw_string_small_zoomed(x, y, exp_per_min_label, 1, win->current_scale);

	if(timediff<=0){
		timediff=1;
	}
	safe_snprintf((char*)buffer, sizeof(buffer), "%2.2f", oa_exp/((float)timediff/60000.0f));
	draw_string_small_zoomed(tot_exp_left, y, buffer, 1, win->current_scale);

	y += y_step;
	draw_string_small_zoomed(x, y, distance_label, 1, win->current_scale);
	safe_snprintf((char*)buffer, sizeof(buffer), "%d", (distance_moved<0) ?0: distance_moved);
	draw_string_small_zoomed(tot_exp_left, y, buffer, 1, win->current_scale);

	if (show_reset_help)
	{
		show_help(session_reset_help, -TAB_MARGIN, win->len_y+10+TAB_MARGIN, win->current_scale);
		show_reset_help = 0;
	}

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 1;
}

void init_session(void)
{
	int save_server = 1;

	/* if we have server info saved, compare with current */
	if (last_port > 0)
	{
		/* if changed, we need to reset the session stats */
		if ((last_port != port) || (strcmp((char *)last_server_address, (char *)server_address)))
		{
			LOG_TO_CONSOLE(c_red2,"Server changed so resetting session stats");
			reconnecting = 0;
		}
		/* else if the same, no need */
		else
			save_server = 0;
	}

	/* save the server info if first time or changed */
	if (save_server)
	{
		last_port = port;
		safe_strncpy((char *)last_server_address, (char *)server_address, sizeof(last_server_address));
	}

	if (!reconnecting){
		set_session_exp_to_current();
		session_start_time = cur_time;
		reconnecting = 1;
	}
	else if ( disconnect_time != 0 ) {
		session_start_time += (cur_time-disconnect_time);
		disconnect_time = 0;
	}
}

int session_reset_handler(void)
{
	static Uint32 last_click = 0;
	/* provide some protection for inadvertent pressing (double click that can be disabled) */
	if (safe_button_click(&last_click))
	{
		init_session();
		set_session_exp_to_current();
		session_start_time = cur_time;
		reset_session_counters();
		range_critical_hits = 0;
		range_success_hits = 0;
		range_total_shots = 0;
		distance_moved = 0;
	}
	return 0;
}
