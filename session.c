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
#include "platform.h"
#include "stats.h"
#include "translate.h"
#include "counters.h"
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif
#include "widgets.h"

int session_win = -1;
static int reconnecting = 0;
static int last_port = -1;
static unsigned char last_server_address[60];
static int show_reset_help = 0;

player_attribs session_stats;
Uint32 session_start_time;

int display_session_handler(window_info *win);

int get_session_exp_ranging(void)
{
	return your_info.ranging_exp - session_stats.ranging_exp;
}

static int mouseover_session_reset_handler(void)
{
	if (!disable_double_click && show_help_text)
		show_reset_help = 1;
	return 0;
}

void fill_session_win(void)
{
	int reset_button_id = -1;
	set_window_handler(session_win, ELW_HANDLER_DISPLAY, &display_session_handler);

	reset_button_id=button_add_extended(session_win, reset_button_id, NULL, 450, 3, 0, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, reset_str);
	widget_set_OnClick(session_win, reset_button_id, session_reset_handler);
	widget_set_OnMouseover(session_win, reset_button_id, mouseover_session_reset_handler);
	
}

int display_session_handler(window_info *win)
{
	int x, y, timediff;
	char buffer[80];
	player_attribs cur_stats = your_info;

	x = 10;
	y = 21;
	timediff = 0;

	glColor3f(1.0f, 1.0f, 1.0f);
	draw_string_small(x, y, (unsigned char*)"Skill", 1);

	glColor3f(1.0f, 1.0f, 1.0f);
	draw_string_small(x + 200, y, (unsigned char*)"Experience Gain", 1);

	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f, 0.57f, 0.39f);
	glBegin(GL_LINES);
	glVertex3i(0, 37, 0);
	glVertex3i(win->len_x, 37, 0);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);

	y = 55;

	draw_string_small(x, y, attributes.attack_skill.name , 1);
	safe_snprintf(buffer, sizeof(buffer), "%d", cur_stats.attack_exp - session_stats.attack_exp);
	draw_string_small(x + 200, y, (unsigned char*)buffer, 1);
	y += 16;

	draw_string_small(x, y, attributes.defense_skill.name , 1);
	safe_snprintf(buffer, sizeof(buffer), "%d", cur_stats.defense_exp - session_stats.defense_exp);
	draw_string_small(x + 200, y, (unsigned char*)buffer, 1);
	y += 16;

	draw_string_small(x, y, attributes.harvesting_skill.name , 1);
	safe_snprintf(buffer, sizeof(buffer), "%d", cur_stats.harvesting_exp - session_stats.harvesting_exp);
	draw_string_small(x + 200, y, (unsigned char*)buffer, 1);
	y += 16;

	draw_string_small(x, y, attributes.alchemy_skill.name , 1);
	safe_snprintf(buffer, sizeof(buffer), "%d", cur_stats.alchemy_exp - session_stats.alchemy_exp);
	draw_string_small(x + 200, y, (unsigned char*)buffer, 1);
	y += 16;

	draw_string_small(x, y, attributes.magic_skill.name , 1);
	safe_snprintf(buffer, sizeof(buffer), "%d", cur_stats.magic_exp - session_stats.magic_exp);
	draw_string_small(x + 200, y, (unsigned char*)buffer, 1);
	y += 16;

	draw_string_small(x, y, attributes.potion_skill.name , 1);
	safe_snprintf(buffer, sizeof(buffer), "%d", cur_stats.potion_exp - session_stats.potion_exp);
	draw_string_small(x + 200, y, (unsigned char*)buffer, 1);
	y += 16;

	draw_string_small(x, y, attributes.summoning_skill.name , 1);
	safe_snprintf(buffer, sizeof(buffer), "%d", cur_stats.summoning_exp - session_stats.summoning_exp);
	draw_string_small(x + 200, y, (unsigned char*)buffer, 1);
	y += 16;

	draw_string_small(x, y, attributes.manufacturing_skill.name , 1);
	safe_snprintf(buffer, sizeof(buffer), "%d", cur_stats.manufacturing_exp - session_stats.manufacturing_exp);
	draw_string_small(x + 200, y, (unsigned char*)buffer, 1);
	y += 16;

	draw_string_small(x, y, attributes.crafting_skill.name , 1);
	safe_snprintf(buffer, sizeof(buffer), "%d", cur_stats.crafting_exp - session_stats.crafting_exp);
	draw_string_small(x + 200, y, (unsigned char*)buffer, 1);
	y += 16;

	draw_string_small(x, y, attributes.engineering_skill.name , 1);
	safe_snprintf(buffer, sizeof(buffer), "%d", cur_stats.engineering_exp - session_stats.engineering_exp);
	draw_string_small(x + 200, y, (unsigned char*)buffer, 1);
	y += 16;

	draw_string_small(x, y, attributes.tailoring_skill.name , 1);
	safe_snprintf(buffer, sizeof(buffer), "%d", cur_stats.tailoring_exp - session_stats.tailoring_exp);
	draw_string_small(x + 200, y, (unsigned char*)buffer, 1);
	y += 16;

	draw_string_small(x, y, attributes.ranging_skill.name , 1);
	safe_snprintf(buffer, sizeof(buffer), "%d", get_session_exp_ranging());
	draw_string_small(x + 200, y, (unsigned char*)buffer, 1);
	y += 16;

	draw_string_small(x, y, attributes.overall_skill.name , 1);
	safe_snprintf(buffer, sizeof(buffer), "%d", cur_stats.overall_exp - session_stats.overall_exp);
	draw_string_small(x + 200, y, (unsigned char*)buffer, 1);

	y += 32;

	draw_string_small(x, y, (unsigned char*)"Session Time", 1);
	timediff = cur_time - session_start_time;
	safe_snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", timediff/3600000, (timediff/60000)%60, (timediff/1000)%60);
	draw_string_small(x + 200, y, (unsigned char*)buffer, 1);

	y += 16;

	draw_string_small(x, y, (unsigned char*)"Exp/Min", 1);
	
	if(timediff<=0){
		timediff=1;
	}
	safe_snprintf(buffer, sizeof(buffer), "%2.2f", (float)(cur_stats.overall_exp - session_stats.overall_exp)/((float)timediff/60000.0f));
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
		session_stats = your_info;
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
		session_stats = your_info;
		session_start_time = cur_time;
		reset_session_counters();
		range_critical_hits = 0;
		range_success_hits = 0;
		range_total_shots = 0;
	}
	return 0;
}
