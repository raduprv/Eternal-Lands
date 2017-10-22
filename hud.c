#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "hud.h"
#include "asc.h"
#include "buddy.h"
#include "consolewin.h"
#include "context_menu.h"
#include "elconfig.h"
#include "elwindows.h"
#include "events.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud_indicators.h"
#include "hud_misc_window.h"
#include "hud_quickbar_window.h"
#include "icon_window.h"
#include "interface.h"
#include "manufacture.h"
#include "mapwin.h"
#include "minimap.h"
#include "missiles.h"
#include "questlog.h"
#include "sound.h"
#include "spells.h"
#include "storage.h"
#include "tabs.h"
#include "textures.h"
#include "trade.h"
#ifdef ECDEBUGWIN
#include "eye_candy_debugwin.h"
#endif
#include "user_menus.h"
#include "emotes.h"

#define UI_SCALED_VALUE(BASE) ((int)(0.5 + ((BASE) * get_global_scale())))

Uint32 exp_lev[200];
hud_interface last_interface = HUD_INTERFACE_NEW_CHAR; //Current interface (game or new character)

static int display_stats_bar_handler(window_info *win);
static int mouseover_stats_bar_handler(window_info *win, int mx, int my);
static void init_hud_frame();
static void draw_exp_display(window_info *win);
static size_t cm_id = CM_INIT_VALUE;

int hud_x= 64;
int hud_y= 48;
int hud_text;
int copy_next_LOCATE_ME = 0;
int	stats_bar_win= -1;
int	quickspell_win= -1;
int show_help_text=1;
int always_enlarge_text=1;


/* called on client exit to free resources */
void cleanup_hud(void)
{
	destroy_hud_indicators();
	destroy_window(misc_win);
	destroy_window(stats_bar_win);
	destroy_window(quickbar_win);
	stats_bar_win = quickbar_win = misc_win = -1;
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

	init_hud_frame ();
	if(type == HUD_INTERFACE_GAME)
		init_misc_display (type);

	if (type == HUD_INTERFACE_NEW_CHAR)
	{
		hud_x = UI_SCALED_VALUE(270);
		resize_root_window();
		init_icon_window (NEW_CHARACTER_ICONS);
	}
	else
	{
		if (hud_x>0)
			hud_x=HUD_MARGIN_X;
		resize_root_window();
		init_icon_window (MAIN_WINDOW_ICONS);
		init_stats_display ();
		init_quickbar ();
		init_quickspell ();
		init_hud_indicators (); 
		ready_for_user_menus = 1;
		if (enable_user_menus)
			display_user_menus();
		if ((minimap_win < 0) && open_minimap_on_start)
			view_window (&minimap_win, 0);
	}

	last_interface = type;
}

void show_hud_windows ()
{
	if (icons_win >= 0) show_window (icons_win);
	if (stats_bar_win >= 0) show_window (stats_bar_win);
	if (misc_win >= 0) show_window (misc_win);
	if (quickbar_win >= 0) show_window (quickbar_win);
	if (quickspell_win >= 0) show_window (quickspell_win);
	show_hud_indicators_window();
}

void hide_hud_windows ()
{
	if (icons_win >= 0) hide_window (icons_win);
	if (stats_bar_win >= 0) hide_window (stats_bar_win);
	if (misc_win >= 0) hide_window (misc_win);
	if (quickbar_win >= 0) hide_window (quickbar_win);
	if (quickspell_win >= 0) hide_window (quickspell_win);
	hide_hud_indicators_window();
}

// draw everything related to the hud
void draw_hud_interface()
{
	glColor3f(1.0f, 1.0f, 1.0f);
	draw_hud_frame();
}

// hud frame section
static float vertical_bar_u_start = (float)192/256;
static float vertical_bar_u_end = 1.0f;
static float vertical_bar_v_end = 0.0f;
static float vertical_bar_v_start = 0.0f;

static float horizontal_bar_u_start = (float)144/256;
static float horizontal_bar_u_end = (float)191/256;
static float horizontal_bar_v_start = 0.0f;
static float horizontal_bar_v_end = 0.0f;

static void init_hud_frame()
{
	vertical_bar_v_end = (float)window_height/256;
	horizontal_bar_v_start = (float)(window_width-hud_x)/256;
}

static float logo_u_start = (float)64/256;
static float logo_v_start = (float)128/256;
static float logo_u_end = (float)127/256;
static float logo_v_end = (float)191/256;

void draw_hud_frame()
{
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	bind_texture(hud_text);
	glBegin(GL_QUADS);
	draw_2d_thing_r(horizontal_bar_u_start, horizontal_bar_v_start, horizontal_bar_u_end, horizontal_bar_v_end,0,window_height,window_width, window_height-hud_y);
	if(last_interface == HUD_INTERFACE_GAME)
	{
		draw_2d_thing(vertical_bar_u_start, vertical_bar_v_start, vertical_bar_u_end, vertical_bar_v_end,window_width-hud_x, 0, window_width, window_height);
		//draw the logo
		draw_2d_thing(logo_u_start, logo_v_start, logo_u_end, logo_v_end,window_width-hud_x, 0, window_width, UI_SCALED_VALUE(64));
	}
	glEnd();
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

static void view_console_win (int *win, int id)
{
	if ( get_show_window (console_root_win) && !locked_to_console )
		return_to_gamewin_common();
	else
	{
		if ( get_show_window (game_root_win) )
			hide_window (game_root_win);
		if ( get_show_window (map_root_win) )
			hide_window (map_root_win);
		show_window (console_root_win);
	}
}

static void view_map_win (int * win, int id)
{
	if ( get_show_window (map_root_win) && !locked_to_console )
		return_to_gamewin_common();
	else if ( switch_to_game_map () && !locked_to_console )
	{
		if ( get_show_window (game_root_win) )
			hide_window (game_root_win);
		if ( get_show_window (console_root_win) )
			hide_window (console_root_win);
		show_window (map_root_win);
	}
}

typedef struct
{
	char name[20];
	int *id;
} windowid_by_name;

int* get_winid(const char *name)
{
	static windowid_by_name win_ids[] = {
		{ "invent", &items_win },
		{ "spell", &sigil_win },
		{ "manu", &manufacture_win },
		{ "emotewin", &emotes_win },
		{ "quest", &questlog_win },
		{ "map", &map_root_win },
		{ "info", &tab_info_win },
		{ "buddy", &buddy_win },
		{ "stats", &tab_stats_win },
		{ "console", &console_root_win },
		{ "help", &tab_help_win },
		{ "opts", &elconfig_win },
		{ "range", &range_win },
		{ "minimap", &minimap_win } };
	size_t i;
	if (name == NULL)
		return NULL;
	for (i=0; i<sizeof(win_ids)/sizeof(windowid_by_name); i++)
		if (strcmp(win_ids[i].name, name) == 0)
			return win_ids[i].id;
	return NULL;
}

void view_window(int * window, int id)
{
	if (window == NULL)
		return;

	if (window == &map_root_win)
	{
		view_map_win(window, id);
		return;
	}

	if (window == &console_root_win)
	{
		view_console_win(window, id);
		return;
	}
	
	if(window==&sigil_win||window==&manufacture_win)
		{
			if(get_show_window(trade_win))
				{
					LOG_TO_CONSOLE(c_red2,no_open_on_trade);
					return;
				}
		}

	if(*window < 0)
		{
			//OK, the window has not been created yet - use the standard functions
			if(window==&items_win)display_items_menu();
			else if(window==&sigil_win) display_sigils_menu();
			else if(window==&manufacture_win) display_manufacture_menu();
			else if(window==&emotes_win) display_emotes_menu();
			else if(window==&elconfig_win) display_elconfig_win();
			else if(window==&buddy_win) display_buddy();
			else if(window==&trade_win) display_trade_menu();
			else if(window==&tab_info_win) display_tab_info();
			else if(window==&minimap_win) display_minimap();
#ifdef ECDEBUGWIN
			else if(window==&ecdebug_win) display_ecdebugwin();
#endif
			else if(window==&storage_win) display_storage_menu();
			else if(window==&tab_stats_win) display_tab_stats();
			else if(window==&tab_help_win) display_tab_help();
			else if(window==&questlog_win) display_questlog();
			else if(window==&range_win) display_range_win();
		}
	else toggle_window(*window);
}

void view_tab (int *window, int *col_id, int tab)
{
	if (get_show_window (*window))
	{
		if (tab_collection_get_tab (*window, *col_id) == tab)
		{
			hide_window (*window);
		}
		else
		{
			tab_collection_select_tab (*window, *col_id, tab);
		}
	}
	else
	{
		view_window (window, 0);
		tab_collection_select_tab (*window, *col_id, tab);
	}
}

int enlarge_text(void)
{
	if (always_enlarge_text)
		return 1;
	return ((SDL_GetModState() & (KMOD_CTRL|KMOD_ALT)));
}

static void show_help_coloured_scaled(const char *help_message, int x, int y, float r, float g, float b, int use_big_font, float size)
{
	int y_font_len = 0;
	int len = 0;
	int width=window_width-80;

	if (use_big_font)
	{
		y_font_len = (int)(0.5 + DEFAULT_FONT_Y_LEN * size);
		len = strlen(help_message) * (int)(0.5 + DEFAULT_FONT_X_LEN * size) + 1;
	}
	else
	{
		y_font_len = (int)(0.5 + SMALL_FONT_Y_LEN * size);
		len = strlen(help_message) * (int)(0.5 + SMALL_FONT_X_LEN * size) + 1;
	}

	if(x+len>width) x-=(x+len)-width;

	glColor4f(0.0f,0.0f,0.0f,0.5f);
	glDisable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glBegin(GL_QUADS);
	glVertex3i(x-1,y+y_font_len,0);
	glVertex3i(x-1,y,0);
	glVertex3i(x+len,y,0);
	glVertex3i(x+len,y+y_font_len,0);
	glEnd();

	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);

	glColor3f(r,g,b);
	if (use_big_font)
		draw_string_zoomed(x, y, (unsigned char*)help_message, 1, size);
	else
		draw_string_small_zoomed(x, y, (unsigned char*)help_message, 1, size);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void show_help(const char *help_message, int x, int y, float scale)
{
	show_help_coloured_scaled(help_message, x, y, 1.0f, 1.0f, 1.0f, 0, scale);
}

void show_help_big(const char *help_message, int x, int y, float scale)
{
	show_help_coloured_scaled(help_message, x, y, 1.0f, 1.0f, 1.0f, 1, scale);
}


// Stats bars in the bottom HUD .....

int show_action_bar = 0;
int watch_this_stats[MAX_WATCH_STATS]={NUM_WATCH_STAT -1, 0, 0, 0, 0};  // default to only watching overall
int max_food_level=45;
static int exp_bar_text_len = 0;
static int stats_bar_text_len = 0;

// return the number of watched stat bars, the number displayed in the botton HUD
static int get_num_statsbar_exp(void)
{
	size_t i;
	int num_skills_bar = 0;
	for (i=0; i<MAX_WATCH_STATS; i++)
		if ((watch_this_stats[i] > 0) && statsinfo[watch_this_stats[i]-1].is_selected)
			num_skills_bar++;
	return num_skills_bar;
}


// calculate the number of digits for the specified Uint32
static Uint32 Uint32_digits(Uint32 number)
{
	Uint32 digits = 1;
	long step = 10;
	while ((step <= number) && (digits<11))
	{
		digits++;
		step *= 10;
	}
	return digits;
}


// check if we need to adjust exp_bar_text_len due to an exp change
static int recalc_exp_bar_text_len(void)
{
	static int init_flag = 1;
	static Uint32 last_exp[NUM_WATCH_STAT-1];
	static Uint32 last_to_go_len[NUM_WATCH_STAT-1];
	static Uint32 last_selected[NUM_WATCH_STAT-1];
	int recalc = 1;
	int i;

	if (init_flag)
	{
		for (i=0; i<NUM_WATCH_STAT-1; i++)
		{
			last_exp[i] = *statsinfo[i].exp;
			last_to_go_len[i] = Uint32_digits(*statsinfo[i].next_lev - *statsinfo[i].exp);
			last_selected[i] = 0;
		}
		init_flag =  0;
	}

	for (i=0; i<NUM_WATCH_STAT-1; i++)
	{
		/* if any exp changes, recalculate the number of digits for next level value */
		if (last_exp[i] != *statsinfo[i].exp)
		{
			unsigned int curr = Uint32_digits(*statsinfo[i].next_lev - *statsinfo[i].exp);
			/* if the number of digit changes, we need to recalulate exp_bar_text_len */
			if (last_to_go_len[i] != curr)
			{
				last_to_go_len[i] = curr;
				recalc = 1;
			}
			last_exp[i] = *statsinfo[i].exp;
		}
		/* if the selected status of any skill changed, we need to recalulate exp_bar_text_len */
		if (last_selected[i] != statsinfo[i].is_selected)
		{
			last_selected[i] = statsinfo[i].is_selected;
			recalc = 1;
		}
	}

	/* recalc based only the skills being watched */
	if (recalc)
	{
		int max_len = 0;
		for (i=0; i<MAX_WATCH_STATS; i++)
			if ((watch_this_stats[i] > 0) && statsinfo[watch_this_stats[i]-1].is_selected &&
					(last_to_go_len[watch_this_stats[i]-1] > max_len))
				max_len = last_to_go_len[watch_this_stats[i]-1];
		return UI_SCALED_VALUE(SMALL_FONT_X_LEN)*(max_len+1.5);
	}
	else
		return exp_bar_text_len;
}


// return the optimal length for stat bars for the botton HUD
static int calc_stats_bar_len(int num_exp)
{
	// calculate the maximum length for stats bars given the current number of both bar types
	int num_stat = (show_action_bar) ?5: 4;
	int prosed_len = (window_width-HUD_MARGIN_X-1) - (num_stat * stats_bar_text_len) - (num_exp * exp_bar_text_len);
	prosed_len /= num_stat + num_exp;

	// constrain the maximum and minimum length of the skills bars to reasonable size
	if (prosed_len < 50)
		prosed_len = 50;
	else if (prosed_len > UI_SCALED_VALUE(100))
		prosed_len = UI_SCALED_VALUE(100);

	return prosed_len;
}

// calculate the maximum number of exp bars
static int calc_max_disp_stats(int suggested_stats_bar_len)
{
	int exp_offset = ((show_action_bar)?5:4) * (suggested_stats_bar_len + stats_bar_text_len);
	int preposed_max_disp_stats = (window_width - HUD_MARGIN_X - exp_offset) / (suggested_stats_bar_len + exp_bar_text_len);
	if (preposed_max_disp_stats > MAX_WATCH_STATS)
		preposed_max_disp_stats = MAX_WATCH_STATS;
	return preposed_max_disp_stats;
}

static int	statbar_cursor_x;
static int stats_bar_len;
static int max_disp_stats=1;
static int health_bar_start_x;
static int health_bar_start_y;
static int mana_bar_start_x;
static int mana_bar_start_y;
static int food_bar_start_x;
static int food_bar_start_y;
static int load_bar_start_x;
static int load_bar_start_y;
static int action_bar_start_x;
static int action_bar_start_y;
static int exp_bar_start_x;
static int exp_bar_start_y;

static int player_statsbar_y_offset = 4;
static int player_statsbar_bar_height = 8;

static int get_player_statsbar_active_height(void)
{
	return player_statsbar_y_offset + UI_SCALED_VALUE(player_statsbar_bar_height);
}

// clear the context menu regions for all stats bars and set up again
static void reset_statsbar_exp_cm_regions(void)
{
	size_t i;
	cm_remove_regions(stats_bar_win);
	for (i=0; i<max_disp_stats; i++)
		if (watch_this_stats[i] > 0)
			cm_add_region(cm_id, stats_bar_win, exp_bar_start_x+i*(stats_bar_len+exp_bar_text_len), exp_bar_start_y, stats_bar_len, get_player_statsbar_active_height());
}

static int cm_statsbar_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	int i;
	int add_bar = 0;

	// selecting the same stat more than once, removing the last bar
	// or adding too many is not possible as options are greyed out.
	
	for (i=0; i<max_disp_stats;i++)
	{
		if ((mx >= exp_bar_start_x+i*(stats_bar_len+exp_bar_text_len)) && (mx <= exp_bar_start_x+i*(stats_bar_len+exp_bar_text_len)+stats_bar_len))
		{
			// if deleting the bar, close any gap
			if (option == NUM_WATCH_STAT+1)
			{
				statsinfo[watch_this_stats[i]-1].is_selected=0;
				if (i<max_disp_stats-1)
					memmove(&(watch_this_stats[i]), &(watch_this_stats[i+1]), (max_disp_stats-i-1) * sizeof(int));
				watch_this_stats[max_disp_stats-1] = 0;
				init_stats_display();
			}
			else if (option == NUM_WATCH_STAT)
				add_bar = 1;
			else
			{
				statsinfo[watch_this_stats[i]-1].is_selected=0;
				watch_this_stats[i] = option+1;
				statsinfo[option].is_selected=1;
			}
			break;
		}
	}

	// if we want another bar, assigning it to the first unwatched stat
	if (add_bar)
	{
		int proposed_max_disp_stats = calc_max_disp_stats(calc_stats_bar_len(get_num_statsbar_exp()+1));
		int next_free = -1;
		for (i=0; i<NUM_WATCH_STAT-1; i++)
			if (statsinfo[i].is_selected == 0)
			{
				next_free = i;
				break;
			}
		for (i=0; next_free>=0 && i<proposed_max_disp_stats; i++)
			if (watch_this_stats[i] == 0)
			{
				watch_this_stats[i] = next_free + 1;
				statsinfo[next_free].is_selected =1 ;
				break;
			}
		init_stats_display();
	}

	reset_statsbar_exp_cm_regions();

	return 1;
}

static void cm_statsbar_pre_show_handler(window_info *win, int widget_id, int mx, int my, window_info *cm_win)
{
	size_t i;
	int proposed_max_disp_stats = calc_max_disp_stats(calc_stats_bar_len(get_num_statsbar_exp()+1));
	for (i=0; i<NUM_WATCH_STAT-1; i++)
		cm_grey_line(cm_id, i, 0);
	for (i=0; i<max_disp_stats; i++)
		if (watch_this_stats[i] > 0)
			cm_grey_line(cm_id, watch_this_stats[i]-1, 1);
	cm_grey_line(cm_id, NUM_WATCH_STAT, ((get_num_statsbar_exp() < proposed_max_disp_stats) ?0 :1));
	cm_grey_line(cm_id, NUM_WATCH_STAT+1, ((watch_this_stats[1]==0)?1:0));
}

// the stats display
void init_stats_display()
{
	int num_exp = get_num_statsbar_exp();
	int actual_num_exp = 0;
	int stats_height = get_player_statsbar_active_height();
	int stats_width = window_width - HUD_MARGIN_X;
	int stats_y_pos = window_height - (HUD_MARGIN_Y - player_statsbar_y_offset);

	//create the stats bar window
	if(stats_bar_win < 0)
	{
		static size_t cm_id_ap = CM_INIT_VALUE;
		stats_bar_win= create_window("Stats Bar", -1, 0, 0, stats_y_pos, stats_width, stats_height, ELW_USE_UISCALE|ELW_TITLE_NONE|ELW_SHOW_LAST);
		set_window_handler(stats_bar_win, ELW_HANDLER_DISPLAY, &display_stats_bar_handler);
		set_window_handler(stats_bar_win, ELW_HANDLER_MOUSEOVER, &mouseover_stats_bar_handler);

		// context menu to enable/disable the action points bar
		cm_id_ap = cm_create(cm_action_points_str, NULL);
		cm_add_window(cm_id_ap, stats_bar_win);
		cm_bool_line(cm_id_ap, 0, &show_action_bar, "show_action_bar");
	}
	else
		init_window(stats_bar_win, -1, 0, 0, stats_y_pos, stats_width, stats_height);

	/* use a fixed width for user attrib stat bar text */
	stats_bar_text_len = 4.5 * UI_SCALED_VALUE(SMALL_FONT_X_LEN);

	// calculate the statsbar len given curent config
	stats_bar_len = calc_stats_bar_len(num_exp);

	// calculate the maximum number of exp bars we can have
	max_disp_stats = calc_max_disp_stats(stats_bar_len);

	// if we need to reduce the number of bars, recalculate the optimum stats bar len
	if (num_exp > max_disp_stats)
		stats_bar_len = calc_stats_bar_len(max_disp_stats);

	// all the bars are at the top of the window
	mana_bar_start_y = food_bar_start_y = health_bar_start_y = load_bar_start_y = action_bar_start_y = exp_bar_start_y = 0;

	// calculate the stats bar x position
	mana_bar_start_x = stats_bar_text_len;
	food_bar_start_x = stats_bar_len + 2 * stats_bar_text_len;
	health_bar_start_x = 2 * stats_bar_len + 3 * stats_bar_text_len;
	load_bar_start_x = 3 * stats_bar_len + 4 * stats_bar_text_len;
	if (show_action_bar)
		action_bar_start_x = 4 * stats_bar_len + 5 * stats_bar_text_len;

	// clear any unused slots in the watch list and recalc how many are being displayed
	if (max_disp_stats < MAX_WATCH_STATS)
	{
		size_t i;
		for (i=max_disp_stats;i<MAX_WATCH_STATS;i++)
			if (watch_this_stats[i] > 0)
			{
				statsinfo[watch_this_stats[i]-1].is_selected=0;
				watch_this_stats[i]=0;
			}
	}
	actual_num_exp = get_num_statsbar_exp();

	// the x position of the first exp bar, keep right aligned
	exp_bar_start_x = window_width + exp_bar_text_len - HUD_MARGIN_X - 2
		- actual_num_exp * (exp_bar_text_len + stats_bar_len);

	// apologise if we had to reduce the number of exp bars
	if (num_exp > actual_num_exp)
		LOG_TO_CONSOLE(c_red2, remove_bar_message_str);

	// create the exp bars context menu, used by all ative exp bars
	if (!cm_valid(cm_id))
	{
		int thestat;
		cm_id = cm_create(NULL, cm_statsbar_handler);
		for (thestat=0; thestat<NUM_WATCH_STAT-1; thestat++)
			cm_add(cm_id, (char *)statsinfo[thestat].skillnames->name, NULL);
		cm_add(cm_id, cm_stats_bar_base_str, NULL);
		cm_set_pre_show_handler(cm_id,cm_statsbar_pre_show_handler);
	}
	reset_statsbar_exp_cm_regions();
}

static void draw_stats_bar(window_info *win, int x, int y, int val, int len, float r, float g, float b, float r2, float g2, float b2)
{
	char buf[32];
	int i; // i deals with massive bars by trimming at 110%
	int bar_height = (int)(0.5 + win->current_scale * player_statsbar_bar_height);
	
	if(len>stats_bar_len*1.1)
		i=stats_bar_len*1.1;
	else
		i=len;
	glDisable(GL_TEXTURE_2D);
	
	if(i >= 0){
		glBegin(GL_QUADS);
		//draw the colored section
 		glColor3f(r2, g2, b2);
		glVertex3i(x, y+bar_height, 0);
		glColor3f(r, g, b);
		glVertex3i(x, y, 0);
		glColor3f(r, g, b);
		glVertex3i(x+i, y, 0);
		glColor3f(r2, g2, b2);
		glVertex3i(x+i, y+bar_height, 0);
		glEnd();
	}
	// draw the bar frame
	glColor3f(0.77f, 0.57f, 0.39f);
	glBegin(GL_LINE_LOOP);
	glVertex3i(x, y, 0);
	glVertex3i(x+stats_bar_len, y, 0);
	glVertex3i(x+stats_bar_len, y+bar_height, 0);
	glVertex3i(x, y+bar_height, 0);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	
	// handle the text
	safe_snprintf(buf, sizeof(buf), "%d", val);
	//glColor3f(0.8f, 0.8f, 0.8f); moved to next line
	draw_string_small_shadowed_zoomed(x-(1+win->small_font_len_x*strlen(buf))-1, y-2, (unsigned char*)buf, 1,0.8f, 0.8f, 0.8f,0.0f,0.0f,0.0f, win->current_scale);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}


static struct { int d; int h; Uint32 dt; Uint32 ht; } my_last_health = { 0, 0, 0, 0 };

/* called when our actor receives damage, displayed as hover over health bar */
void set_last_damage(int quantity)
{
	my_last_health.d = quantity;
	my_last_health.dt = SDL_GetTicks();
}

/* called when our actor heals, displayed as hover over health bar */
void set_last_heal(int quantity)
{
	my_last_health.h = quantity;
	my_last_health.ht = SDL_GetTicks();
}

/* draws damage and heal above the health bar */
static void draw_last_health_change(window_info *win)
{
	char str[20];
	static const Uint32 timeoutms = 2*60*1000;
	const int yoff = (int)(-UI_SCALED_VALUE(SMALL_FONT_Y_LEN+5));
	/* damage in red */
	if (my_last_health.d != 0)
	{
		if ((SDL_GetTicks() - my_last_health.dt) > timeoutms)
			my_last_health.d = 0;
		else
		{
			safe_snprintf(str, sizeof(str), " %d ", my_last_health.d);
			show_help_coloured_scaled(str, health_bar_start_x+stats_bar_len/2-strlen(str)*UI_SCALED_VALUE(SMALL_FONT_X_LEN)-2, yoff, 1.0f, 0.0f, 0.0f, 0, win->current_scale);
		}
	}
	/* heal in green */
	if (my_last_health.h != 0)
	{
		if ((SDL_GetTicks() - my_last_health.ht) > timeoutms)
			my_last_health.h = 0;
		else
		{
			safe_snprintf(str, sizeof(str), " %d ", my_last_health.h);
			show_help_coloured_scaled(str, health_bar_start_x+stats_bar_len/2+2, yoff, 0.0f, 1.0f, 0.0f, 0, win->current_scale);
		}
	}
}

static int	display_stats_bar_handler(window_info *win)
{
	static Uint32 last_time = 0;
	float health_adjusted_x_len;
	float food_adjusted_x_len;
	float mana_adjusted_x_len;
	float load_adjusted_x_len;
	float action_adjusted_x_len;
	int over_health_bar;

	// the space taken up by the exp bar text is minimised, but may change
	// don't have to check often but this is an easy place to do it and its quick anyway
	if ((SDL_GetTicks()-last_time) > 250)
	{
		int proposed_len = 0;
		stats_bar_text_len = 4.5 * UI_SCALED_VALUE(SMALL_FONT_X_LEN);
		if ((proposed_len = recalc_exp_bar_text_len()) != exp_bar_text_len) // it will very rarely change
		{
			exp_bar_text_len = proposed_len;
			init_stats_display();
		}
		last_time = SDL_GetTicks();
	}

	over_health_bar = statbar_cursor_x>health_bar_start_x && statbar_cursor_x < health_bar_start_x+stats_bar_len;

	//get the adjusted length

	if(!your_info.material_points.cur || !your_info.material_points.base)
		health_adjusted_x_len=0;//we don't want a div by 0
	else
		health_adjusted_x_len=stats_bar_len/((float)your_info.material_points.base/(float)your_info.material_points.cur);

	if(your_info.food_level<=0)
		food_adjusted_x_len=0;//we don't want a div by 0
	else
		food_adjusted_x_len=stats_bar_len/((float)max_food_level/(float)your_info.food_level);
	if(food_adjusted_x_len>stats_bar_len) food_adjusted_x_len=stats_bar_len;

	if(!your_info.ethereal_points.cur || !your_info.ethereal_points.base)
		mana_adjusted_x_len=0;//we don't want a div by 0
	else
		mana_adjusted_x_len=stats_bar_len/((float)your_info.ethereal_points.base/(float)your_info.ethereal_points.cur);

	if(!your_info.carry_capacity.cur || !your_info.carry_capacity.base)
		load_adjusted_x_len=0;//we don't want a div by 0
	else
		load_adjusted_x_len=stats_bar_len/((float)your_info.carry_capacity.base/(float)your_info.carry_capacity.cur);

	if(!your_info.action_points.cur || !your_info.action_points.base)
		action_adjusted_x_len=0;//we don't want a div by 0
	else
		action_adjusted_x_len=stats_bar_len/((float)your_info.action_points.base/(float)your_info.action_points.cur);

	draw_stats_bar(win, health_bar_start_x, health_bar_start_y, your_info.material_points.cur, health_adjusted_x_len, 1.0f, 0.2f, 0.2f, 0.5f, 0.2f, 0.2f);

	if (your_info.food_level<=max_food_level) //yellow
		draw_stats_bar(win, food_bar_start_x, food_bar_start_y, your_info.food_level, food_adjusted_x_len, 1.0f, 1.0f, 0.2f, 0.5f, 0.5f, 0.2f);
	else draw_stats_bar(win, food_bar_start_x, food_bar_start_y, your_info.food_level, food_adjusted_x_len, 1.0f, 0.5f, 0.0f, 0.7f, 0.3f, 0.0f); //orange

	draw_stats_bar(win, mana_bar_start_x, mana_bar_start_y, your_info.ethereal_points.cur, mana_adjusted_x_len, 0.2f, 0.2f, 1.0f, 0.2f, 0.2f, 0.5f);
	draw_stats_bar(win, load_bar_start_x, load_bar_start_y, your_info.carry_capacity.base-your_info.carry_capacity.cur, load_adjusted_x_len, 0.6f, 0.4f, 0.4f, 0.4f, 0.2f, 0.2f);
	if (show_action_bar)
		draw_stats_bar(win, action_bar_start_x, action_bar_start_y, your_info.action_points.cur, action_adjusted_x_len, 0.8f, 0.3f, 0.8f, 0.5f, 0.1f, 0.5f);

	draw_exp_display(win);

	if(show_help_text && statbar_cursor_x>=0)
	{
		if(over_health_bar) show_help((char*)attributes.material_points.name,health_bar_start_x+stats_bar_len+10,-3, win->current_scale);
		else if(statbar_cursor_x>food_bar_start_x && statbar_cursor_x < food_bar_start_x+stats_bar_len) show_help((char*)attributes.food.name,food_bar_start_x+stats_bar_len+10,-3, win->current_scale);
		else if(statbar_cursor_x>mana_bar_start_x && statbar_cursor_x < mana_bar_start_x+stats_bar_len) show_help((char*)attributes.ethereal_points.name,mana_bar_start_x+stats_bar_len+10,-3, win->current_scale);
		else if(statbar_cursor_x>load_bar_start_x && statbar_cursor_x < load_bar_start_x+stats_bar_len) show_help((char*)attributes.carry_capacity.name,load_bar_start_x+stats_bar_len+10,-3, win->current_scale);
		else if(show_action_bar && statbar_cursor_x>action_bar_start_x && statbar_cursor_x < action_bar_start_x+stats_bar_len) show_help((char*)attributes.action_points.name,action_bar_start_x+stats_bar_len+10,-3, win->current_scale);
	}

	if (over_health_bar)
		draw_last_health_change(win);

	statbar_cursor_x = -1;

	return 1;
}

static int mouseover_stats_bar_handler(window_info *win, int mx, int my)
{
	statbar_cursor_x=mx;
	return 0;
}

// check if key is one of the item keys and use it if so.
int action_item_keys(Uint32 key)
{
	size_t i;
	Uint32 keys[] = {K_ITEM1, K_ITEM2, K_ITEM3, K_ITEM4, K_ITEM5, K_ITEM6,
					 K_ITEM7, K_ITEM8, K_ITEM9, K_ITEM10, K_ITEM11, K_ITEM12 };
	for (i=0; (i<sizeof(keys)/sizeof(Uint32)) & (i < num_quickbar_slots); i++)
		if(key == keys[i])
		{
			quick_use (i);
			return 1;
		}
	return 0;
}

void build_levels_table()
{
	int i;
	Uint64 exp=100;

	exp_lev[0]=0;
	for(i=1;i<180;i++)
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

static void draw_exp_display(window_info *win)
{
	size_t i;
	int my_exp_bar_start_x = exp_bar_start_x;

	// default to overall if no valid first skill is set
	if(watch_this_stats[0]<1 || watch_this_stats[0]>=NUM_WATCH_STAT)
	{
		watch_this_stats[0]=NUM_WATCH_STAT-1;
		statsinfo[watch_this_stats[0]-1].is_selected=1;
		reset_statsbar_exp_cm_regions();
	}

	for (i=0; i<max_disp_stats; i++)
	{
		if (watch_this_stats[i] > 0)
		{
			int name_x;
			int name_y = exp_bar_start_y + 2 + UI_SCALED_VALUE(player_statsbar_bar_height);
			int icon_x = get_icons_win_active_len();
			int cur_exp = *statsinfo[watch_this_stats[i]-1].exp;
			int nl_exp = *statsinfo[watch_this_stats[i]-1].next_lev;
			int baselev = statsinfo[watch_this_stats[i]-1].skillattr->base;
			unsigned char * name = statsinfo[watch_this_stats[i]-1].skillnames->name;
			int exp_adjusted_x_len;
			int delta_exp;
			float prev_exp;

			if(!baselev)
				prev_exp= 0;
			else
				prev_exp= exp_lev[baselev];

			delta_exp= nl_exp-prev_exp;

			if(!cur_exp || !nl_exp || delta_exp <=0)
				exp_adjusted_x_len= 0;
			else
				exp_adjusted_x_len= stats_bar_len-(float)stats_bar_len/(float)((float)delta_exp/(float)(nl_exp-cur_exp));

			name_x = my_exp_bar_start_x + stats_bar_len - strlen((char *)name) * UI_SCALED_VALUE(SMALL_FONT_X_LEN);
			// the the name would overlap with the icons...
			if (name_x < icon_x)
			{
				name = statsinfo[watch_this_stats[i]-1].skillnames->shortname;
				name_x = my_exp_bar_start_x + stats_bar_len - strlen((char *)name) * UI_SCALED_VALUE(SMALL_FONT_X_LEN) - 3;
				name_y = (int)(0.5 + (player_statsbar_y_offset + UI_SCALED_VALUE(player_statsbar_bar_height) - UI_SCALED_VALUE(SMALL_FONT_Y_LEN)) / 2) - 1;
			}

			draw_stats_bar(win, my_exp_bar_start_x, exp_bar_start_y, nl_exp - cur_exp, exp_adjusted_x_len, 0.1f, 0.8f, 0.1f, 0.1f, 0.4f, 0.1f);
			draw_string_small_shadowed_zoomed(name_x, name_y, name, 1,1.0f,1.0f,1.0f,0.0f,0.0f,0.0f, win->current_scale);

			my_exp_bar_start_x += stats_bar_len+exp_bar_text_len;
		}
		else
			break;
	}

}

void handle_stats_selection(int stat, Uint32 flags)
{
	int i;

	int proposed_max_disp_stats = calc_max_disp_stats(calc_stats_bar_len(get_num_statsbar_exp()+1));

	if (((flags & ELW_ALT) || (flags & ELW_SHIFT)) && (max_disp_stats > 1))
	{
		for (i=0;i<proposed_max_disp_stats;i++)
		{
			// if already selected, unselect and remove bar, closing any gap
			if (watch_this_stats[i]==stat)
			{
				statsinfo[stat-1].is_selected=0;
				if (i<proposed_max_disp_stats-1)
					memmove(&(watch_this_stats[i]), &(watch_this_stats[i+1]), (proposed_max_disp_stats-i-1) * sizeof(int));
				watch_this_stats[proposed_max_disp_stats-1] = 0;
				break;
			}
			// if the bar is not in use, set it to the new stat
			if (watch_this_stats[i]==0)
			{
				watch_this_stats[i]=stat;
				statsinfo[stat-1].is_selected=1;
				break;
			}
		}
	}
	else
	{
		// if not already selected, select the stat and replace the first bar
		if (statsinfo[stat-1].is_selected==0)
		{
			statsinfo[watch_this_stats[0]-1].is_selected=0;
			watch_this_stats[0] = stat;
			statsinfo[stat-1].is_selected=1;
		}
		// else unselect the stat and remove the bar, closing any gap
		else
		{
			statsinfo[stat-1].is_selected=0;
			for (i=0;i<max_disp_stats;i++)
			{
				if (watch_this_stats[i] == stat)
				{
					if (i<max_disp_stats-1)
						memmove(&(watch_this_stats[i]), &(watch_this_stats[i+1]), (max_disp_stats-i-1) * sizeof(int));
					watch_this_stats[max_disp_stats-1] = 0;
				}
			}
		}
	}

	// default to overall if no valid first skill is set
	if(watch_this_stats[0]<1 || watch_this_stats[0]>=NUM_WATCH_STAT)
	{
		watch_this_stats[0]=NUM_WATCH_STAT-1;
		statsinfo[watch_this_stats[0]-1].is_selected=1;
	}

	init_stats_display();
	do_click_sound();
}
