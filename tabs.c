#include <stdlib.h>
#include "global.h"
#include "elwindows.h"

int tab_stats_win = -1;
int tab_stats_collection_id = 16;
Uint16 tab_stats_x = 150;
Uint16 tab_stats_y = 70;
Uint16 tab_stats_len_x = STATS_TAB_WIDTH + 2*TAB_MARGIN;
Uint16 tab_stats_len_y = STATS_TAB_HEIGHT + TAB_TAG_HEIGHT + 2*TAB_MARGIN;

int tab_help_win = -1;
int tab_help_collection_id = 17;
Uint16 tab_help_x = 150;
Uint16 tab_help_y = 70;
Uint16 tab_help_len_x = HELP_TAB_WIDTH + 2*TAB_MARGIN;
Uint16 tab_help_len_y = HELP_TAB_HEIGHT + TAB_TAG_HEIGHT + 2*TAB_MARGIN;

int HELP_TAB_HELP = 0, HELP_TAB_RULES = 3, HELP_TAB_ENCYCLOPEDIA = 2, HELP_TAB_SKILLS = 1;
int STATS_TAB_STATS = 0, STATS_TAB_KNOWLEDGE = 1, STATS_TAB_QUESTLOG = 2, STATS_TAB_KILLS = 3;

int display_tab_stats_handler () 
{
	return 1;
}

void display_tab_stats ()
{
	if (tab_stats_win < 0)
	{
		tab_stats_win = create_window (win_statistics, game_root_win, 0, tab_stats_x, tab_stats_y, tab_stats_len_x, tab_stats_len_y, ELW_WIN_DEFAULT);

		set_window_handler (tab_stats_win, ELW_HANDLER_DISPLAY, &display_tab_stats_handler);
		
		tab_stats_collection_id = tab_collection_add_extended (tab_stats_win, tab_stats_collection_id, NULL, TAB_MARGIN, TAB_MARGIN, STATS_TAB_WIDTH, STATS_TAB_HEIGHT+TAB_TAG_HEIGHT, 0, 0.7, 0.77f, 0.57f, 0.39f, 3, TAB_TAG_HEIGHT);

		stats_win = tab_add (tab_stats_win, tab_stats_collection_id, tab_statistics, 0, 0);
		fill_stats_win ();
		STATS_TAB_STATS = 0;
		
		knowledge_win = tab_add (tab_stats_win, tab_stats_collection_id, tab_knowledge, 0, 0);
		fill_knowledge_win ();
		STATS_TAB_KNOWLEDGE = 1;

		questlog_win = tab_add (tab_stats_win, tab_stats_collection_id, tab_questlog, 0, 0);
		fill_questlog_win ();
		STATS_TAB_QUESTLOG = 2;

		kills_win = tab_add(tab_stats_win, tab_stats_collection_id, tab_kills, 0, 0);
		fill_kills_win();
		STATS_TAB_KILLS = 3;
		
		tab_collection_select_tab (tab_stats_win, tab_stats_collection_id, 0);
	}
	else
	{
		show_window (tab_stats_win);
		select_window (tab_stats_win);
	}
}

int display_tab_help_handler () 
{
	return 1;
}

void display_tab_help ()
{
	if (tab_help_win < 0)
	{
		tab_help_win = create_window (win_help, -1, 0, tab_help_x, tab_help_y, tab_help_len_x, tab_help_len_y, ELW_WIN_DEFAULT);

		set_window_handler (tab_help_win, ELW_HANDLER_DISPLAY, &display_tab_help_handler);
		
		tab_help_collection_id = tab_collection_add_extended (tab_help_win, tab_help_collection_id, NULL, TAB_MARGIN, TAB_MARGIN, HELP_TAB_WIDTH, HELP_TAB_HEIGHT+TAB_TAG_HEIGHT, 0, 0.7, 0.77f, 0.57f, 0.39f, 3, TAB_TAG_HEIGHT);

		help_win = tab_add (tab_help_win, tab_help_collection_id, tab_help, 0, 0);
		fill_help_win ();
		HELP_TAB_HELP = 0;

		skills_win = tab_add (tab_help_win, tab_help_collection_id, tab_skills, 0, 0);
		fill_skills_win ();
		HELP_TAB_SKILLS = 1;
		
		encyclopedia_win = tab_add (tab_help_win, tab_help_collection_id, tab_encyclopedia, 0, 0);
		fill_encyclopedia_win ();
		HELP_TAB_ENCYCLOPEDIA = 2;

		rules_win = tab_add(tab_help_win, tab_help_collection_id, tab_rules, 0, 0);
		fill_rules_window();
		HELP_TAB_RULES = 3;

		tab_collection_select_tab (tab_help_win, tab_help_collection_id, 0);
	}
	else
	{
		show_window (tab_help_win);
		select_window (tab_help_win);
	}
}
