#include "global.h"
#include "elwindows.h"

int use_tabbed_windows = 1;

int tab_stats_win = 0;
int tab_collection_id = 0;

Uint16 tab_stats_x = 150;
Uint16 tab_stats_y = 70;
Uint16 tab_stats_len_x = TAB_WIDTH + 2*TAB_MARGIN;
Uint16 tab_stats_len_y = TAB_HEIGHT + TAB_TAG_HEIGHT + 2*TAB_MARGIN;

int display_tab_stats_handler () 
{
	return 1;
}

void display_tab_stats ()
{
	if (tab_stats_win <= 0)
	{
		tab_stats_win = create_window ("statistics", 0, 0, tab_stats_x, tab_stats_y, tab_stats_len_x, tab_stats_len_y, ELW_WIN_DEFAULT);

		set_window_handler (tab_stats_win, ELW_HANDLER_DISPLAY, &display_tab_stats_handler);
		
		tab_collection_id = tab_collection_add_extended (tab_stats_win, 120, NULL, 5, 5, tab_stats_len_x-10, tab_stats_len_y-10, 0, 0.7, 0.77f, 0.57f, 0.39f, 3, 20, 3);

		stats_win = tab_add (tab_stats_win, tab_collection_id, "Statistics", 90);
		fill_stats_win ();
		
		knowledge_win = tab_add (tab_stats_win, tab_collection_id, "Knowledge", 80);
		fill_knowledge_win ();

		questlog_win = tab_add (tab_stats_win, tab_collection_id, "Quest log", 80);
		fill_questlog_win ();
		
		tab_collection_select_tab (tab_stats_win, tab_collection_id, 0);
	}
	else
	{
		show_window (tab_stats_win);
		select_window (tab_stats_win);
	}
}
