#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL_keyboard.h>

#include "asc.h"
#include "buddy.h"
#include "consolewin.h"
#include "cursors.h"
#include "elconfig.h"
#include "emotes.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud.h"
#include "hud_indicators.h"
#include "hud_misc_window.h"
#include "hud_quickbar_window.h"
#include "hud_quickspells_window.h"
#include "hud_statsbar_window.h"
#include "icon_window.h"
#include "interface.h"
#include "manufacture.h"
#include "mapwin.h"
#include "minimap.h"
#include "missiles.h"
#include "new_character.h"
#include "questlog.h"
#include "spells.h"
#include "storage.h"
#include "tabs.h"
#include "textures.h"
#include "trade.h"
#include "user_menus.h"
#include "url.h"


int hud_x= 64;
int hud_y= 48;
int hud_text;
int show_help_text=1;
int always_enlarge_text=1;
Uint32 exp_lev[MAX_EXP_LEVEL];
int logo_click_to_url = 1;
char LOGO_URL_LINK[128] = "http://www.eternal-lands.com";

static int mouse_over_logo = 0;
static hud_interface last_interface = HUD_INTERFACE_NEW_CHAR; //Current interface (game or new character)

/* called on client exit to free resources */
void cleanup_hud(void)
{
	destroy_hud_indicators();
	destroy_icon_window();
	destroy_window(misc_win);
	destroy_window(stats_bar_win);
	destroy_window(get_id_MW(MW_QUICKBAR));
	stats_bar_win = misc_win = -1;
	set_id_MW(MW_QUICKBAR, -1);
}


/* #exp console command, display current exp information */
int show_exp(char *text, int len)
{
	int thestat;
	char buf[256];
	for (thestat=0; thestat<NUM_WATCH_STAT-1; thestat++)
	{
		safe_snprintf(buf, sizeof(buf), "%s: level %u, %u/%u exp (%u to go)",
			statsinfo[thestat].skillnames->name, statsinfo[thestat].skillattr->base,
			*statsinfo[thestat].exp, *statsinfo[thestat].next_lev,
			*statsinfo[thestat].next_lev - *statsinfo[thestat].exp );
		LOG_TO_CONSOLE(c_green1, buf);
	}
	return 1;
}

// initialize anything related to the hud
void init_hud_interface (hud_interface type)
{
	if (type == HUD_INTERFACE_LAST)
		type = last_interface;

	if (type == HUD_INTERFACE_NEW_CHAR)
	{
		if (newchar_root_win >= 0 && newchar_root_win < windows_list.num_windows)
		{
			hud_x = (int)(0.5 + windows_list.window[newchar_root_win].current_scale * NEW_CHARACTER_BASE_HUD_X);
			resize_root_window();
			init_icon_window (NEW_CHARACTER_ICONS);
		}
	}
	else
	{
		if (hud_x>0)
			hud_x=HUD_MARGIN_X;
		resize_root_window();
		init_icon_window (MAIN_WINDOW_ICONS);
		init_stats_display ();
		init_misc_display ();
		init_quickbar ();
		init_quickspell ();
		init_hud_indicators ();
		ready_for_user_menus = 1;
		if (enable_user_menus)
			display_user_menus();
		if ((get_id_MW(MW_MINIMAP) < 0) && open_minimap_on_start)
		{
			static int first_time = 1;
			if (first_time)
				view_window(MW_MINIMAP);
			first_time = 0;
		}
	}

	last_interface = type;
}

void show_moveable_hud_windows(void)
{
	show_window_MW(MW_QUICKBAR);
	show_window_MW(MW_QUICKSPELLS);
	show_hud_indicators_window();
}

void show_hud_windows (void)
{
	if (icons_win >= 0) show_window (icons_win);
	if (stats_bar_win >= 0) show_window (stats_bar_win);
	if (misc_win >= 0) show_window (misc_win);
	show_moveable_hud_windows();
}

void hide_hud_windows (void)
{
	if (icons_win >= 0) hide_window (icons_win);
	if (stats_bar_win >= 0) hide_window (stats_bar_win);
	if (misc_win >= 0) hide_window (misc_win);
	hide_window_MW(MW_QUICKBAR);
	hide_window_MW(MW_QUICKSPELLS);
	hide_hud_indicators_window();
}

void hide_moved_hud_windows(void)
{
	size_t i;
	int list_of_windows[2] = { get_id_MW(MW_QUICKBAR), get_id_MW(MW_QUICKSPELLS) };
	for (i=0; i<2; i++)
	{
		if (get_show_window (list_of_windows[i])
				&& windows_list.window[list_of_windows[i]].cur_x < window_width - HUD_MARGIN_X
				&& window_height - windows_list.window[list_of_windows[i]].cur_y > HUD_MARGIN_Y)
			hide_window (list_of_windows[i]);
	}
}

int hud_mouse_over(window_info *win, int mx, int my)
{
	int dead_space = (int)(0.5 + win->current_scale * 10);
	int hud_logo_size = get_hud_logo_size();
	mouse_over_logo = 0;
	// exclude some 	dead space to try to prevent accidental misclicks
	if (logo_click_to_url && hud_x && (mx > win->len_x - (hud_logo_size - dead_space)) && (my < hud_logo_size - dead_space))
	{
		elwin_mouse = CURSOR_USE;
		mouse_over_logo = 1;
		return 1;
	}
	if (hud_x && hud_y && ((mx > win->len_x - hud_x) || (my > win->len_y - hud_y)))
	{
		elwin_mouse = CURSOR_ARROW;
		return 1;
	}
	return 0;
}

int hud_click(window_info *win, int mx, int my, Uint32 flags)
{
	if (mouse_over_logo)
	{
		if (logo_click_to_url)
			open_web_link(LOGO_URL_LINK);
		return 1;
	}
	return 0;
}

int get_hud_logo_size(void)
{
	return HUD_MARGIN_X;
}

void draw_hud_interface(window_info *win)
{
	const float vertical_bar_u_start = (float)192/256;
	const float vertical_bar_u_end = 1.0f;
	const float vertical_bar_v_start = 0.0f;
	const float horizontal_bar_u_start = (float)144/256;
	const float horizontal_bar_u_end = (float)191/256;
	const float horizontal_bar_v_end = 0.0f;
	const float logo_u_start = (float)64/256;
	const float logo_v_start = (float)128/256;
	const float logo_u_end = (float)127/256;
	const float logo_v_end = (float)191/256;
	float vertical_bar_v_end = (float)window_height/256;
	float horizontal_bar_v_start = (float)(window_width-hud_x)/256;
	int hud_logo_size = get_hud_logo_size();

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	glColor3f(1.0f, 1.0f, 1.0f);
	bind_texture(hud_text);
	glBegin(GL_QUADS);
	draw_2d_thing_r(horizontal_bar_u_start, horizontal_bar_v_start, horizontal_bar_u_end, horizontal_bar_v_end,0,window_height,window_width, window_height-hud_y);
	if(last_interface == HUD_INTERFACE_GAME)
	{
		draw_2d_thing(vertical_bar_u_start, vertical_bar_v_start, vertical_bar_u_end, vertical_bar_v_end,window_width-hud_x, 0, window_width, window_height);
		//draw the logo
		if (hud_x)
			draw_2d_thing(logo_u_start, logo_v_start, logo_u_end, logo_v_end, window_width - hud_logo_size, 0, window_width, hud_logo_size);
	}
	glEnd();
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

static void view_console_win(void)
{
	if ( get_show_window_MW(MW_CONSOLE) && !locked_to_console )
		return_to_gamewin_common();
	else
	{
		if ( get_show_window (game_root_win) )
			hide_window (game_root_win);
		if ( get_show_window_MW(MW_TABMAP) )
			hide_window_MW(MW_TABMAP);
		show_window_MW(MW_CONSOLE);
	}
}

static void view_map_win(void)
{
	if ( get_show_window_MW(MW_TABMAP) && !locked_to_console )
		return_to_gamewin_common();
	else if ( switch_to_game_map () && !locked_to_console )
	{
		if ( get_show_window (game_root_win) )
			hide_window (game_root_win);
		if ( get_show_window_MW(MW_CONSOLE) )
			hide_window_MW(MW_CONSOLE);
		show_window_MW(MW_TABMAP);
	}
}

void view_window(enum managed_window_enum managed_win)
{
	if (managed_win >= MW_MAX)
		return;

	if (managed_win == MW_TABMAP)
	{
		view_map_win();
		return;
	}

	if (managed_win == MW_CONSOLE)
	{
		view_console_win();
		return;
	}

	if(managed_win == MW_SPELLS || managed_win == MW_MANU)
	{
		if(get_show_window_MW(MW_TRADE))
		{
			LOG_TO_CONSOLE(c_red2,no_open_on_trade);
			return;
		}
	}

	// If not created, call the display function, otherwise toggle the window.
	if(get_id_MW(managed_win) < 0)
		call_display_MW(managed_win);
	else
		toggle_window_MW(managed_win);
}

void view_tab (enum managed_window_enum managed_win, int col_id, int tab)
{
	if (get_show_window_MW(managed_win))
	{
		if (tab_collection_get_tab(get_id_MW(managed_win), col_id) == tab)
		{
			hide_window_MW(managed_win);
		}
		else
		{
			tab_collection_select_tab(get_id_MW(managed_win), col_id, tab);
		}
	}
	else
	{
		view_window(managed_win);
		tab_collection_select_tab(get_id_MW(managed_win), col_id, tab);
	}
}

int enlarge_text(void)
{
	if (always_enlarge_text)
		return 1;
	return ((SDL_GetModState() & (KMOD_CTRL|KMOD_ALT)));
}

void build_levels_table()
{
	int i;
	Uint64 exp=100;

	exp_lev[0]=0;
	for(i=1;i<MAX_EXP_LEVEL;i++)
	{
		if(i<=10)exp+=exp*40/100;
		else
		if(i<=20)exp+=exp*30/100;
		else
		if(i<=30)exp+=exp*20/100;
		else
		if(i<=40)exp+=exp*14/100;
		else
		if(i<=90)exp+=exp*7/100;
		else exp+=exp*5/100;
		exp_lev[i]=(Uint32)exp;
	}
}

