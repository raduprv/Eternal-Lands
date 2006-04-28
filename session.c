#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include "global.h"

int session_win = -1;

player_attribs session_stats;
Uint32 session_start_time;

int display_session_handler(window_info *win);

void fill_session_win()
{
	int reset_button_id = -1;
	set_window_handler(session_win, ELW_HANDLER_DISPLAY, &display_session_handler);

	reset_button_id=button_add_extended(session_win, reset_button_id, NULL, 450, 3, 0, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, reset_str);
	widget_set_OnClick(session_win, reset_button_id, session_reset_handler);
	
}

int display_session_handler(window_info *win)
{
	int i, n, x, y, timediff;
	char buffer[80];
	player_attribs cur_stats = your_info;

	x = 10;
	y = 21;
	timediff = 0;
	
	glColor3f(1.0f, 1.0f, 1.0f);
	draw_string_small(x, y, "Skill", 1);

	glColor3f(1.0f, 1.0f, 1.0f);
	draw_string_small(x + 200, y, "Experience Gain", 1);

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
	sprintf(buffer, "%d", cur_stats.attack_exp - session_stats.attack_exp);
	draw_string_small(x + 200, y, buffer, 1);
	y += 16;
	
	draw_string_small(x, y, attributes.defense_skill.name , 1);
	sprintf(buffer, "%d", cur_stats.defense_exp - session_stats.defense_exp);
	draw_string_small(x + 200, y, buffer, 1);
	y += 16;
	
	draw_string_small(x, y, attributes.harvesting_skill.name , 1);
	sprintf(buffer, "%d", cur_stats.harvesting_exp - session_stats.harvesting_exp);
	draw_string_small(x + 200, y, buffer, 1);
	y += 16;
	
	draw_string_small(x, y, attributes.alchemy_skill.name , 1);
	sprintf(buffer, "%d", cur_stats.alchemy_exp - session_stats.alchemy_exp);
	draw_string_small(x + 200, y, buffer, 1);
	y += 16;
	
	draw_string_small(x, y, attributes.magic_skill.name , 1);
	sprintf(buffer, "%d", cur_stats.magic_exp - session_stats.magic_exp);
	draw_string_small(x + 200, y, buffer, 1);
	y += 16;
	
	draw_string_small(x, y, attributes.potion_skill.name , 1);
	sprintf(buffer, "%d", cur_stats.potion_exp - session_stats.potion_exp);
	draw_string_small(x + 200, y, buffer, 1);
	y += 16;
	
	draw_string_small(x, y, attributes.summoning_skill.name , 1);
	sprintf(buffer, "%d", cur_stats.summoning_exp - session_stats.summoning_exp);
	draw_string_small(x + 200, y, buffer, 1);
	y += 16;
	
	draw_string_small(x, y, attributes.manufacturing_skill.name , 1);
	sprintf(buffer, "%d", cur_stats.manufacturing_exp - session_stats.manufacturing_exp);
	draw_string_small(x + 200, y, buffer, 1);
	y += 16;
	
	draw_string_small(x, y, attributes.crafting_skill.name , 1);
	sprintf(buffer, "%d", cur_stats.crafting_exp - session_stats.crafting_exp);
	draw_string_small(x + 200, y, buffer, 1);
	y += 16;
	
	draw_string_small(x, y, attributes.overall_skill.name , 1);
	sprintf(buffer, "%d", cur_stats.overall_exp - session_stats.overall_exp);
	draw_string_small(x + 200, y, buffer, 1);
	
	y += 32;
	
	draw_string_small(x, y,"Session Time", 1);
	timediff = cur_time - session_start_time;
	sprintf(buffer, "%02d:%02d:%02d", timediff/3600000, (timediff/60000)%60, (timediff/1000)%60);
	draw_string_small(x + 200, y, buffer, 1);
	 	
	
	return 1;
}

void init_session()
{
	session_stats = your_info;
	session_start_time = cur_time;
}
int session_reset_handler()
{
	init_session();
	return 0;
}
