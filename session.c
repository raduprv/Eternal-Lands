#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include "session.h"
#include "actors.h"
#include "asc.h"
#include "elwindows.h"
#include "init.h"
#include "global.h"
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

int exp_log_threshold = 5000;
static int reconnecting = 0;
static int last_port = -1;
static unsigned char last_server_address[60];
static int reset_button_id = -1;
static int show_reset_help = 0;
static int last_mouse_click_y = -1;
static int last_mouse_over_y = -1;
static int distance_moved = -1;

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
	last_mouse_click_y = my;
	do_click_sound();
	return 1;
}

static int mouseover_session_handler(window_info *win, int mx, int my)
{
	last_mouse_over_y = my;
	return 1;
}

static int resize_session_handler(window_info *win, int new_width, int new_height)
{
	button_resize(win->window_id, reset_button_id, 0, 0, win->current_scale);
	widget_move(win->window_id, reset_button_id, (int)(win->current_scale * 450), (int)(win->current_scale * 280));
	return 0;
}

void fill_session_win(int window_id)
{
	set_window_handler(window_id, ELW_HANDLER_DISPLAY, &display_session_handler);
	set_window_handler(window_id, ELW_HANDLER_CLICK, &click_session_handler );
	set_window_handler(window_id, ELW_HANDLER_MOUSEOVER, &mouseover_session_handler );
	set_window_handler(window_id, ELW_HANDLER_RESIZE, &resize_session_handler );

	reset_button_id=button_add_extended(window_id, reset_button_id, NULL, 0, 0, 0, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, reset_str);
	widget_set_OnClick(window_id, reset_button_id, session_reset_handler);
	widget_set_OnMouseover(window_id, reset_button_id, mouseover_session_reset_handler);
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

int display_session_handler(window_info *win)
{
	int i, timediff;
	char buffer[128];
	float oa_exp;
	int y_step = (int)(0.5 + win->current_scale * 16);
	int extra_x_offset = (int)(0.5 + win->current_scale * 200);
	int start_y_offset = (int)(0.5 + win->current_scale * 55);
	int start_line_y_offset = (int)(0.5 + win->current_scale * 37);
	int x = (int)(0.5 + win->current_scale * 10);
	int y = (int)(0.5 + win->current_scale * 21);

	timediff = 0;
	oa_exp = 0.0f;

	glColor3f(1.0f, 1.0f, 1.0f);
	safe_snprintf(buffer, sizeof(buffer), "%-20s%-17s%-17s%-17s",
		"Skill", "Total Exp", "Max Exp", "Last Exp" );
	scaled_draw_string_small(x, y, (unsigned char*)buffer, 1);

	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f, 0.57f, 0.39f);
	glBegin(GL_LINES);
	glVertex3i(0, start_line_y_offset, 0);
	glVertex3i(win->len_x, start_line_y_offset, 0);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);

	y = start_y_offset;

	for (i=0; i<NUM_SKILLS; i++)
	{
		if ((last_mouse_click_y >= y) && (last_mouse_click_y < y+16))
			elglColourN("global.mouseselected");
		else if ((last_mouse_over_y >= y) && (last_mouse_over_y < y+16))
			elglColourN("global.mousehighlight");
		else
			glColor3f(1.0f, 1.0f, 1.0f);
		safe_snprintf(buffer, sizeof(buffer), "%-20s%-17u%-17u%-17u",
			statsinfo[i].skillnames->name, *(statsinfo[i].exp) - session_exp[i], max_exp[i], last_exp[i]);
		scaled_draw_string_small(x, y, (unsigned char*)buffer, 1);
		y += y_step;
		if(i < NUM_SKILLS-1)
			oa_exp += *(statsinfo[i].exp) - session_exp[i];
	}

	y += y_step;

	glColor3f(1.0f, 1.0f, 1.0f);
	scaled_draw_string_small(x, y, (unsigned char*)"Session Time", 1);
	timediff = cur_time - session_start_time;
	safe_snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", timediff/3600000, (timediff/60000)%60, (timediff/1000)%60);
	scaled_draw_string_small(x + extra_x_offset, y, (unsigned char*)buffer, 1);

	y += y_step;

	scaled_draw_string_small(x, y, (unsigned char*)"Exp/Min", 1);
	
	if(timediff<=0){
		timediff=1;
	}
	safe_snprintf(buffer, sizeof(buffer), "%2.2f", oa_exp/((float)timediff/60000.0f));
	scaled_draw_string_small(x + extra_x_offset, y, (unsigned char*)buffer, 1);

	y += y_step;
	scaled_draw_string_small(x, y, (unsigned char*)"Distance", 1);
	safe_snprintf(buffer, sizeof(buffer), "%d", (distance_moved<0) ?0: distance_moved);
	scaled_draw_string_small(x + extra_x_offset, y, (unsigned char*)buffer, 1);

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
