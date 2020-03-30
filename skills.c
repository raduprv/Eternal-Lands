#include <stdlib.h>
#include <string.h>
#include "skills.h"
#include "asc.h"
#include "elwindows.h"
#include "encyclopedia.h"

static int skills_menu_scroll_id = 0;
static size_t skillspage;

int display_skills_handler(window_info *win)
{
	return common_encyclopedia_display_handler(win, skillspage, skills_menu_scroll_id);
}

int click_skills_handler(window_info *win, int mx, int my, Uint32 flags)
{
	return common_encyclopedia_click_handler(win, mx, my, flags, &skillspage, skills_menu_scroll_id);
}

int resize_skills_handler(window_info *win, int new_width, int new_height)
{
	widget_resize(win->window_id, skills_menu_scroll_id, win->box_size, win->len_y);
	widget_move(win->window_id, skills_menu_scroll_id, win->len_x - win->box_size, 0);
	return 0;
}

void fill_skills_win (int window_id)
{
	window_info *win = &windows_list.window[window_id];
	size_t i;

	for(i=0;i<=numpage;i++)
	{
		if(my_strcompare(Page[i].Name,"newskills"))
			break;
	}
	skillspage = i;
	set_window_custom_scale(window_id, &custom_scale_factors.help);
	set_window_handler (window_id, ELW_HANDLER_DISPLAY, &display_skills_handler);
	set_window_handler (window_id, ELW_HANDLER_CLICK, &click_skills_handler);
	set_window_handler (window_id, ELW_HANDLER_RESIZE, &resize_skills_handler);

	skills_menu_scroll_id = vscrollbar_add_extended(window_id, skills_menu_scroll_id, NULL,
		win->len_x-win->box_size, 0, win->box_size, win->len_y, 0, 1.0, 0.77f, 0.57f, 0.39f, 0, 30, Page[skillspage].max_y);
}
