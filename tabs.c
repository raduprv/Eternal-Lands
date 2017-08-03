#include <stdlib.h>
#include "tabs.h"
#include "elwindows.h"
#include "encyclopedia.h"
#include "gamewin.h"
#include "help.h"
#include "knowledge.h"
#include "rules.h"
#include "session.h"
#include "skills.h"
#include "translate.h"
#include "counters.h"
#include "url.h"
#include "notepad.h"

int tab_stats_win = -1;
int tab_stats_collection_id = 16;
int tab_stats_x = 150;
int tab_stats_y = 70;
unsigned tab_selected = 0;

int tab_help_win = -1;
int tab_help_collection_id = 17;
int tab_help_x = 150;
int tab_help_y = 70;

int tab_info_win = -1;
int tab_info_collection_id = 18;
int tab_info_x = 150;
int tab_info_y = 70;

static int ui_scale_stats_handler(window_info *win)
{
	int tab_tag_height = (int)(0.5 + win->current_scale * 25);
	int new_width = (int)(0.5 + win->current_scale * 580);
	int new_height = (int)(0.5 + win->current_scale * 350);
	widget_list *w = widget_find (win->window_id, tab_stats_collection_id);

	resize_window(win->window_id, new_width + 2*TAB_MARGIN, new_height + tab_tag_height + 2*TAB_MARGIN);
	widget_resize(win->window_id, tab_stats_collection_id, new_width, new_height + tab_tag_height);

	tab_collection_resize(w, new_width, new_height, tab_tag_height);
	tab_collection_move(w, win->pos_x + TAB_MARGIN, win->pos_y + tab_tag_height + TAB_MARGIN);

	return 1;
}

void display_tab_stats ()
{
	if (tab_stats_win < 0)
	{
		int our_root_win = -1;
		if (!windows_on_top) {
			our_root_win = game_root_win;
		}
		tab_stats_win = create_window (win_statistics, our_root_win, 0, tab_stats_x, tab_stats_y, 0, 0, ELW_USE_UISCALE|ELW_WIN_DEFAULT);
		set_window_handler(tab_stats_win, ELW_HANDLER_UI_SCALE, &ui_scale_stats_handler );
		tab_stats_collection_id = tab_collection_add_extended (tab_stats_win, tab_stats_collection_id, NULL, TAB_MARGIN, TAB_MARGIN, 0, 0, 0, 0.7, 0.77f, 0.57f, 0.39f, 3, TAB_TAG_HEIGHT);

		fill_session_win(tab_add(tab_stats_win, tab_stats_collection_id, tab_session, 0, 0, ELW_USE_UISCALE));
		fill_stats_win (tab_add (tab_stats_win, tab_stats_collection_id, tab_statistics, 0, 0, ELW_USE_UISCALE));
		fill_knowledge_win (tab_add (tab_stats_win, tab_stats_collection_id, tab_knowledge, 0, 0, ELW_USE_UISCALE));
		fill_counters_win(tab_add(tab_stats_win, tab_stats_collection_id, tab_counters, 0, 0, ELW_USE_UISCALE));

		if ((tab_stats_win > -1) && (tab_stats_win < windows_list.num_windows))
			ui_scale_stats_handler(&windows_list.window[tab_stats_win]);

		tab_collection_select_tab (tab_stats_win, tab_stats_collection_id, tab_selected & 0xf);
	}
	else
	{
		show_window (tab_stats_win);
		select_window (tab_stats_win);
	}
}

static int ui_scale_help_handler(window_info *win)
{
	int tab_tag_height = (int)(0.5 + win->current_scale * 25);
	int new_width = (int)(0.5 + win->current_scale * 500);
	int new_height = (int)(0.5 + win->current_scale * 350);
	widget_list *w = widget_find (win->window_id, tab_help_collection_id);

	resize_window(win->window_id, new_width + 2*TAB_MARGIN, new_height + tab_tag_height + 2*TAB_MARGIN);
	widget_resize(win->window_id, tab_help_collection_id, new_width, new_height + tab_tag_height);

	tab_collection_resize(w, new_width, new_height, tab_tag_height);
	tab_collection_move(w, win->pos_x + TAB_MARGIN, win->pos_y + tab_tag_height + TAB_MARGIN);

	return 1;
}


void display_tab_help ()
{
	if (tab_help_win < 0)
	{
		tab_help_win = create_window (win_help, -1, 0, tab_help_x, tab_help_y, 0, 0, ELW_USE_UISCALE|ELW_WIN_DEFAULT);
		set_window_handler(tab_help_win, ELW_HANDLER_UI_SCALE, &ui_scale_help_handler );
		tab_help_collection_id = tab_collection_add_extended (tab_help_win, tab_help_collection_id, NULL, TAB_MARGIN, TAB_MARGIN, 0, 0, 0, 0.7, 0.77f, 0.57f, 0.39f, 3, TAB_TAG_HEIGHT);

		fill_help_win (tab_add (tab_help_win, tab_help_collection_id, tab_help, 0, 0, ELW_USE_UISCALE));
		fill_skills_win (tab_add (tab_help_win, tab_help_collection_id, tab_skills, 0, 0, ELW_USE_UISCALE));
		fill_encyclopedia_win (tab_add (tab_help_win, tab_help_collection_id, tab_encyclopedia, 0, 0, ELW_USE_UISCALE));
		fill_rules_window(tab_add(tab_help_win, tab_help_collection_id, tab_rules, 0, 0, ELW_USE_UISCALE));

		if ((tab_help_win > -1) && (tab_help_win < windows_list.num_windows))
			ui_scale_help_handler(&windows_list.window[tab_help_win]);

		tab_collection_select_tab (tab_help_win, tab_help_collection_id, (tab_selected >> 4) & 0xf);
	}
	else
	{
		show_window (tab_help_win);
		select_window (tab_help_win);
	}
}

static int ui_scale_info_handler(window_info *win)
{
	int tab_tag_height = (int)(0.5 + win->current_scale * 25);
	int new_width = (int)(0.5 + win->current_scale * 500);
	int new_height = (int)(0.5 + win->current_scale * 350);
	widget_list *w = widget_find (win->window_id, tab_info_collection_id);

	resize_window(win->window_id, new_width + 2*TAB_MARGIN, new_height + tab_tag_height + 2*TAB_MARGIN);
	widget_resize(win->window_id, tab_info_collection_id, new_width, new_height + tab_tag_height);

	tab_collection_resize(w, new_width, new_height, tab_tag_height);
	tab_collection_move(w, win->pos_x + TAB_MARGIN, win->pos_y + tab_tag_height + TAB_MARGIN);

	return 1;
}

void display_tab_info()
{
	if (tab_info_win < 0)
	{
		int our_root_win = -1;
		if (!windows_on_top)
			our_root_win = game_root_win;

		tab_info_win = create_window (tt_info, our_root_win, 0, tab_info_x, tab_info_y, 0, 0, ELW_USE_UISCALE|ELW_WIN_DEFAULT);
		set_window_handler(tab_info_win, ELW_HANDLER_UI_SCALE, &ui_scale_info_handler );
		tab_info_collection_id = tab_collection_add_extended (tab_info_win, tab_info_collection_id, NULL, TAB_MARGIN, TAB_MARGIN, 0, 0, 0, 0.7, 0.77f, 0.57f, 0.39f, 3, TAB_TAG_HEIGHT);

		fill_notepad_window(tab_add(tab_info_win, tab_info_collection_id, win_notepad, 0, 0, ELW_USE_UISCALE));
		fill_url_window(tab_add(tab_info_win, tab_info_collection_id, win_url_str, 0, 0, ELW_USE_UISCALE));

		if ((tab_info_win > -1) && (tab_info_win < windows_list.num_windows))
			ui_scale_info_handler(&windows_list.window[tab_info_win]);

		tab_collection_select_tab (tab_info_win, tab_info_collection_id, (tab_selected >> 8) & 0xf);
	}
	else
	{
		show_window (tab_info_win);
		select_window (tab_info_win);
	}
}

/* get selected tabs when saved in cfg file */
unsigned get_tab_selected(void)
{
	unsigned int old_tab_selected = tab_selected;
	int tabnr;
	tab_selected = 0;

	tabnr = tab_collection_get_tab(tab_stats_win, tab_stats_collection_id);
	tab_selected |= ((tabnr < 0) ?old_tab_selected :tabnr) & 0xf;

	tabnr = tab_collection_get_tab(tab_help_win, tab_help_collection_id);
	tab_selected |= ((tabnr < 0) ?old_tab_selected :(tabnr<<4)) & 0xf0;

	tabnr = tab_collection_get_tab(tab_info_win, tab_info_collection_id);
	tab_selected |= ((tabnr < 0) ?old_tab_selected :(tabnr<<8)) & 0xf00;

	return tab_selected;
}
