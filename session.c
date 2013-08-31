#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include "session.h"
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
#include "translate.h"
#include "counters.h"
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif
#include "widgets.h"

int session_win = -1;
int exp_log_threshold = 5000;
static int reconnecting = 0;
static int last_port = -1;
static unsigned char last_server_address[60];
static int show_reset_help = 0;
static int last_mouse_click_y = -1;
static int last_mouse_over_y = -1;

static Uint32 session_exp[NUM_SKILLS];
static Uint32 max_exp[NUM_SKILLS];
static Uint32 last_exp[NUM_SKILLS];

Uint32 session_start_time;

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

void fill_session_win(void)
{
	int reset_button_id = -1;
	set_window_handler(session_win, ELW_HANDLER_DISPLAY, &display_session_handler);
	set_window_handler(session_win, ELW_HANDLER_CLICK, &click_session_handler );
	set_window_handler(session_win, ELW_HANDLER_MOUSEOVER, &mouseover_session_handler );

	reset_button_id=button_add_extended(session_win, reset_button_id, NULL, 450, 280, 0, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, reset_str);
	widget_set_OnClick(session_win, reset_button_id, session_reset_handler);
	widget_set_OnMouseover(session_win, reset_button_id, mouseover_session_reset_handler);
	
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

int display_session_handler(window_info *win)
{
	int i, x, y, timediff;
	char buffer[128];

	x = 10;
	y = 21;
	timediff = 0;

	glColor3f(1.0f, 1.0f, 1.0f);
	safe_snprintf(buffer, sizeof(buffer), "%-20s%-17s%-17s%-17s",
		"Skill", "Total Exp", "Max Exp", "Last Exp" );
	draw_string_small(x, y, (unsigned char*)buffer, 1);

	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f, 0.57f, 0.39f);
	glBegin(GL_LINES);
	glVertex3i(0, 37, 0);
	glVertex3i(win->len_x, 37, 0);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);

	y = 55;

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
		draw_string_small(x, y, (unsigned char*)buffer, 1);
		y += 16;
	}

	y += 16;

	glColor3f(1.0f, 1.0f, 1.0f);
	draw_string_small(x, y, (unsigned char*)"Session Time", 1);
	timediff = cur_time - session_start_time;
	safe_snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", timediff/3600000, (timediff/60000)%60, (timediff/1000)%60);
	draw_string_small(x + 200, y, (unsigned char*)buffer, 1);

	y += 16;

	draw_string_small(x, y, (unsigned char*)"Exp/Min", 1);
	
	if(timediff<=0){
		timediff=1;
	}
	safe_snprintf(buffer, sizeof(buffer), "%2.2f", (float)(*(statsinfo[SI_ALL].exp) - session_exp[SI_ALL])/((float)timediff/60000.0f));
	draw_string_small(x + 200, y, (unsigned char*)buffer, 1);

	if (show_reset_help)
	{
		show_help(session_reset_help, 0, win->len_y+10);
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
	}
	return 0;
}
