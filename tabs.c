#include <stdlib.h>
#include "tabs.h"
#include "elwindows.h"
#include "encyclopedia.h"
#include "font.h"
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

unsigned tab_selected = 0;
int tab_stats_collection_id = 16;
int tab_help_collection_id = 17;
int tab_info_collection_id = 18;
static int tab_stat_scale_changed = 0;
static int tab_help_scale_changed = 0;
static int tab_info_scale_changed = 0;

static int do_scale_stats_handler(window_info *win)
{
	int tab_tag_height = 0;
	int new_width = 0;
	int new_height = 0;
	widget_list *w = widget_find (win->window_id, tab_stats_collection_id);
	const tab_collection *col = (const tab_collection*)w->widget_info;

	for (int i = 0; i < col->nr_tabs; ++i)
	{
		int id = col->tabs[i].content_id;
		if (id >= 0 && id < windows_list.num_windows)
		{
			const window_info *win = &windows_list.window[id];
			new_width = max2i(new_width, win->min_len_x);
			new_height = max2i(new_height, win->min_len_y);
		}
	}

	widget_set_size(win->window_id, tab_stats_collection_id, win->current_scale_small);
	tab_tag_height = tab_collection_calc_tab_height(win->font_category, win->current_scale_small);
	resize_window(win->window_id, new_width + 2*TAB_MARGIN, new_height + tab_tag_height + 2*TAB_MARGIN);
	widget_resize(win->window_id, tab_stats_collection_id, new_width, new_height + tab_tag_height);

	tab_collection_resize(w, new_width, new_height);
	tab_collection_move(w, win->pos_x + TAB_MARGIN, win->pos_y + tab_tag_height + TAB_MARGIN);

	return 1;
}

/*!
 * This display handler is only used to react to font changes *after* the
 * changes in the content windows have been handled. We cannot handle this in
 * the font change handler itself, as it is called before the font change
 * handlers of the child windows.
 */
static int display_stats_handler(window_info *win)
{
	if (!tab_stat_scale_changed)
		return 0;
	do_scale_stats_handler(win);
	tab_stat_scale_changed = 0;
	return 1;
}

static int ui_scale_stats_handler(window_info *win)
{
	tab_stat_scale_changed = 1;
	return 1;
}

static int change_stats_font_handler(window_info* win, font_cat cat)
{
	if (cat != UI_FONT)
		return 0;
	tab_stat_scale_changed = 1;
	return 1;
}

void display_tab_stats ()
{
	int tab_stats_win = get_id_MW(MW_STATS);

	if (tab_stats_win < 0)
	{
		tab_stats_win = create_window (win_statistics, (not_on_top_now(MW_STATS) ?game_root_win : -1), 0, get_pos_x_MW(MW_STATS), get_pos_y_MW(MW_STATS), 0, 0, ELW_USE_UISCALE|ELW_WIN_DEFAULT);
		set_id_MW(MW_STATS, tab_stats_win);
		set_window_custom_scale(tab_stats_win, MW_STATS);
		set_window_handler(tab_stats_win, ELW_HANDLER_UI_SCALE, &ui_scale_stats_handler );
		set_window_handler(tab_stats_win, ELW_HANDLER_DISPLAY, &display_stats_handler);
		set_window_handler(tab_stats_win, ELW_HANDLER_FONT_CHANGE, &change_stats_font_handler);
		tab_stats_collection_id = tab_collection_add_extended (tab_stats_win, tab_stats_collection_id, NULL, TAB_MARGIN, TAB_MARGIN, 0, 0, 0, DEFAULT_SMALL_RATIO, 3);

		fill_stats_win (tab_add (tab_stats_win, tab_stats_collection_id, tab_statistics, 0, 0, ELW_USE_UISCALE));
		fill_knowledge_win (tab_add (tab_stats_win, tab_stats_collection_id, tab_knowledge, 0, 0, ELW_USE_UISCALE));
		fill_counters_win(tab_add(tab_stats_win, tab_stats_collection_id, tab_counters, 0, 0, ELW_USE_UISCALE));
		fill_session_win(tab_add(tab_stats_win, tab_stats_collection_id, tab_session, 0, 0, ELW_USE_UISCALE));

		if ((tab_stats_win > -1) && (tab_stats_win < windows_list.num_windows))
			do_scale_stats_handler(&windows_list.window[tab_stats_win]);
		check_proportional_move(MW_STATS);

		tab_collection_select_tab (tab_stats_win, tab_stats_collection_id, tab_selected & 0xf);
	}
	else
	{
		show_window (tab_stats_win);
		select_window (tab_stats_win);
	}
}

static int do_scale_help_handler(window_info *win)
{
	int tab_tag_height = 0;
	int new_width = 0;
	int new_height = 0;
	widget_list *w = widget_find (win->window_id, tab_help_collection_id);
	const tab_collection *col = (const tab_collection*)w->widget_info;

	for (int i = 0; i < col->nr_tabs; ++i)
	{
		int id = col->tabs[i].content_id;
		if (id >= 0 && id < windows_list.num_windows)
		{
			const window_info *win = &windows_list.window[id];
			new_width = max2i(new_width, win->min_len_x);
			new_height = max2i(new_height, win->min_len_y);
		}
	}

	widget_set_size(win->window_id, tab_help_collection_id, win->current_scale_small);
	tab_tag_height = tab_collection_calc_tab_height(win->font_category, win->current_scale_small);
	resize_window(win->window_id, new_width + 2*TAB_MARGIN, new_height + tab_tag_height + 2*TAB_MARGIN);
	widget_resize(win->window_id, tab_help_collection_id, new_width, new_height + tab_tag_height);

	tab_collection_resize(w, new_width, new_height);
	tab_collection_move(w, win->pos_x + TAB_MARGIN, win->pos_y + tab_tag_height + TAB_MARGIN);

	return 1;
}

/*!
 * This display handler is only used to react to font changes *after* the
 * changes in the content windows have been handled. We cannot handle this in
 * the font change handler itself, as it is called before the font change
 * handlers of the child windows.
 */
static int display_help_handler(window_info *win)
{
	if (!tab_help_scale_changed)
		return 0;
	do_scale_help_handler(win);
	tab_help_scale_changed = 0;
	return 1;
}

static int ui_scale_help_handler(window_info* win, font_cat cat)
{
	tab_help_scale_changed = 1;
	return 1;
}

static int change_help_font_handler(window_info* win, font_cat cat)
{
	if (cat != ENCYCLOPEDIA_FONT && cat != RULES_FONT && cat != UI_FONT)
		return 0;
	tab_help_scale_changed = 1;
	return 1;
}

void display_tab_help ()
{
	int tab_help_win = get_id_MW(MW_HELP);

	if (tab_help_win < 0)
	{
		tab_help_win = create_window (win_help, -1, 0, get_pos_x_MW(MW_HELP), get_pos_y_MW(MW_HELP), 0, 0, ELW_USE_UISCALE|ELW_WIN_DEFAULT);
		set_id_MW(MW_HELP, tab_help_win);
		set_window_custom_scale(tab_help_win, MW_HELP);
		set_window_handler(tab_help_win, ELW_HANDLER_DISPLAY, &display_help_handler);
		set_window_handler(tab_help_win, ELW_HANDLER_UI_SCALE, &ui_scale_help_handler );
		set_window_handler(tab_help_win, ELW_HANDLER_FONT_CHANGE, &change_help_font_handler);
		tab_help_collection_id = tab_collection_add_extended (tab_help_win, tab_help_collection_id, NULL, TAB_MARGIN, TAB_MARGIN, 0, 0, 0, DEFAULT_SMALL_RATIO, 3);

		fill_help_win (tab_add (tab_help_win, tab_help_collection_id, tab_help, 0, 0, ELW_USE_UISCALE));
		fill_skills_win (tab_add (tab_help_win, tab_help_collection_id, tab_skills, 0, 0, ELW_USE_UISCALE));
		fill_encyclopedia_win (tab_add (tab_help_win, tab_help_collection_id, tab_encyclopedia, 0, 0, ELW_USE_UISCALE));
		fill_rules_window(tab_add(tab_help_win, tab_help_collection_id, tab_rules, 0, 0, ELW_USE_UISCALE));

		if ((tab_help_win > -1) && (tab_help_win < windows_list.num_windows))
			do_scale_help_handler(&windows_list.window[tab_help_win]);
		check_proportional_move(MW_HELP);

		tab_collection_select_tab (tab_help_win, tab_help_collection_id, (tab_selected >> 4) & 0xf);
	}
	else
	{
		show_window (tab_help_win);
		select_window (tab_help_win);
	}
}

static int do_scale_info_handler(window_info *win)
{
	int tab_tag_height = 0;
	int new_width = (int)(0.5 + win->current_scale * 500);
	int new_height = (int)(0.5 + win->current_scale * 350);
	widget_list *w = widget_find (win->window_id, tab_info_collection_id);
	const tab_collection *col = (const tab_collection*)w->widget_info;

	for (int i = 0; i < col->nr_tabs; ++i)
	{
		int id = col->tabs[i].content_id;
		if (id >= 0 && id < windows_list.num_windows)
		{
			const window_info *win = &windows_list.window[id];
			new_width = max2i(new_width, win->min_len_x);
			new_height = max2i(new_height, win->min_len_y);
		}
	}

	widget_set_size(win->window_id, tab_info_collection_id, win->current_scale_small);
	tab_tag_height = tab_collection_calc_tab_height(win->font_category, win->current_scale_small);
	resize_window(win->window_id, new_width + 2*TAB_MARGIN, new_height + tab_tag_height + 2*TAB_MARGIN);
	widget_resize(win->window_id, tab_info_collection_id, new_width, new_height + tab_tag_height);

	tab_collection_resize(w, new_width, new_height);
	tab_collection_move(w, win->pos_x + TAB_MARGIN, win->pos_y + tab_tag_height + TAB_MARGIN);

	return 1;
}

/*!
 * This display handler is only used to react to font changes *after* the
 * changes in the content windows have been handled. We cannot handle this in
 * the font change handler itself, as it is called before the font change
 * handlers of the child windows.
 */
static int display_info_handler(window_info *win)
{
	if (!tab_info_scale_changed)
		return 0;
	do_scale_info_handler(win);
	tab_info_scale_changed = 0;
	return 1;
}

static int ui_scale_info_handler(window_info *win)
{
	tab_info_scale_changed = 1;
	return 1;
}

static int change_info_font_handler(window_info* win, font_cat cat)
{
	if (cat != win->font_category)
		return 0;
	tab_info_scale_changed = 1;
	return 1;
}

void display_tab_info()
{
	int tab_info_win = get_id_MW(MW_INFO);

	if (tab_info_win < 0)
	{
		tab_info_win = create_window (tt_info, (not_on_top_now(MW_INFO) ?game_root_win : -1), 0,
			get_pos_x_MW(MW_INFO), get_pos_y_MW(MW_INFO), 0, 0, ELW_USE_UISCALE|ELW_WIN_DEFAULT);
		set_id_MW(MW_INFO, tab_info_win);
		set_window_custom_scale(tab_info_win, MW_INFO);
		set_window_handler(tab_info_win, ELW_HANDLER_DISPLAY, &display_info_handler);
		set_window_handler(tab_info_win, ELW_HANDLER_UI_SCALE, &ui_scale_info_handler);
		set_window_handler(tab_info_win, ELW_HANDLER_FONT_CHANGE, &change_info_font_handler);
		tab_info_collection_id = tab_collection_add_extended (tab_info_win, tab_info_collection_id, NULL, TAB_MARGIN, TAB_MARGIN, 0, 0, 0, DEFAULT_SMALL_RATIO, 3);

		fill_notepad_window(tab_add(tab_info_win, tab_info_collection_id, win_notepad, 0, 0, ELW_USE_UISCALE));
		fill_url_window(tab_add(tab_info_win, tab_info_collection_id, win_url_str, 0, 0, ELW_USE_UISCALE));

		if ((tab_info_win > -1) && (tab_info_win < windows_list.num_windows))
			do_scale_info_handler(&windows_list.window[tab_info_win]);
		check_proportional_move(MW_INFO);

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

	tabnr = tab_collection_get_tab(get_id_MW(MW_STATS), tab_stats_collection_id);
	tab_selected |= ((tabnr < 0) ?old_tab_selected :tabnr) & 0xf;

	tabnr = tab_collection_get_tab(get_id_MW(MW_STATS), tab_help_collection_id);
	tab_selected |= ((tabnr < 0) ?old_tab_selected :(tabnr<<4)) & 0xf0;

	tabnr = tab_collection_get_tab(get_id_MW(MW_INFO), tab_info_collection_id);
	tab_selected |= ((tabnr < 0) ?old_tab_selected :(tabnr<<8)) & 0xf00;

	return tab_selected;
}
