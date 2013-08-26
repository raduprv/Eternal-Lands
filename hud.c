#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "hud.h"
#include "asc.h"
#include "buddy.h"
#include "consolewin.h"
#include "context_menu.h"
#include "cursors.h"
#include "draw_scene.h"
#include "elconfig.h"
#include "elwindows.h"
#include "events.h"
#include "gamewin.h"
#include "gl_init.h"
#include "global.h"
#include "icon_window.h"
#include "init.h"
#include "interface.h"
#include "items.h"
#include "item_info.h"
#include "keys.h" //Avoid problems with SHIFT, ALT, CTRL
#include "knowledge.h"
#include "manufacture.h"
#include "mapwin.h"
#include "minimap.h"
#include "missiles.h"
#include "multiplayer.h"
#include "new_character.h"
#include "platform.h"
#include "questlog.h"
#include "sound.h"
#include "spells.h"
#include "stats.h"
#include "storage.h"
#include "tabs.h"
#include "textures.h"
#include "trade.h"
#include "translate.h"
#ifdef ECDEBUGWIN
#include "eye_candy_debugwin.h"
#endif
#include "user_menus.h"
#include "url.h"
#include "emotes.h"

#define WALK 0
#define SIT 1
#define LOOK 2
#define TRADE 3
#define ATTACK 4
#define USE 5

Uint32 exp_lev[200];
#ifdef NEW_NEW_CHAR_WINDOW
hud_interface last_interface = HUD_INTERFACE_NEW_CHAR; //Current interface (game or new character)
#endif

int	display_stats_bar_handler(window_info *win);
int	display_misc_handler(window_info *win);
int	click_misc_handler(window_info *win, int mx, int my, Uint32 flags);
int	mouseover_misc_handler(window_info *win, int mx, int my);
int	display_quickbar_handler(window_info *win);
int	click_quickbar_handler(window_info *win, int mx, int my, Uint32 flags);
int	mouseover_quickbar_handler(window_info *win, int mx, int my);
int	mouseover_stats_bar_handler(window_info *win, int mx, int my);
void init_hud_frame();
void init_stats_display();
void draw_exp_display();
void draw_stats();
void init_misc_display(hud_interface type);
void init_quickbar();
void toggle_quickbar_draggable();
void flip_quickbar();
void reset_quickbar();
void change_flags(int win_id, Uint32 flags);
Uint32 get_flags(int win_id);
int get_quickbar_y_base();

static int context_hud_handler(window_info *win, int widget_id, int mx, int my, int option);
static size_t cm_hud_id = CM_INIT_VALUE;
static size_t cm_quickbar_id = CM_INIT_VALUE;
static size_t cm_id = CM_INIT_VALUE;
static int cm_quickbar_enabled = 0;
static int cm_sound_enabled = 0;
static int cm_music_enabled = 0;
static int cm_minimap_shown = 0;
static int cm_rangstats_shown = 0;
enum {	CMH_STATS=0, CMH_STATBARS, CMH_KNOWBAR, CMH_TIMER, CMH_DIGCLOCK, CMH_ANACLOCK,
		CMH_SECONDS, CMH_FPS, CMH_QUICKBM, CMH_SEP1, CMH_MINIMAP, CMH_RANGSTATS,
		CMH_SEP2, CMH_SOUND, CMH_MUSIC, CMH_SEP3, CMH_LOCATION };
enum {	CMQB_RELOC=0, CMQB_DRAG, CMQB_RESET, CMQB_FLIP, CMQB_ENABLE };

int hud_x= 64;
int hud_y= 48;
int hud_text;
int view_analog_clock= 1;
int view_digital_clock= 0;
int view_knowledge_bar = 1;
int view_hud_timer = 1;
int copy_next_LOCATE_ME = 0;
int	stats_bar_win= -1;
int	misc_win= -1;
int	quickbar_win= -1;
int	quickspell_win= -1;
int show_help_text=1;

int qb_action_mode=ACTION_USE;

int show_stats_in_hud=0;
int show_statbars_in_hud=0;

static int first_disp_stat = 0;					/* first skill that will be display */
static int num_disp_stat = NUM_WATCH_STAT-1;		/* number of skills to be displayed */
static int statbar_start_y = 0;					/* y coord in window of top if stats bar */
static int stat_mouse_is_over = -1;				/* set to stat of the is mouse over that bar */
static int mouse_over_clock = 0;					/* 1 if mouse is over digital or analogue clock */
static int mouse_over_compass = 0;					/* 1 if mouse is over the compass */
static int mouse_over_knowledge_bar = 0;			/* 1 if mouse is over the knowledge bar */

static const int knowledge_bar_height = SMALL_FONT_Y_LEN + 6;
static const int stats_bar_height = SMALL_FONT_Y_LEN;


/*
 *  The countdown / stopwatch timer code
 *
 * 	TODO
 * 		add #command start/stop/reset/mode/set
 * 		add context menu including dynamic list of previous start values
 */

static struct
{
	int running;
	int current_value;
	int start_value;
	int mode_coundown;
	int mouse_over;
	const int max_value;
	const int height;
} hud_timer = {0, 90, 90, 1, 0, 9*60+59, DEFAULT_FONT_Y_LEN };

static const int get_height_of_timer(void)
{
	return hud_timer.height;
}

static void set_mouse_over_timer(void)
{
	hud_timer.mouse_over = 1;
}


/* Called from the main thread 500 ms timer */
void update_hud_timer(void)
{
	static int use_it = 0;
	if (hud_timer.running && use_it)
	{
		if (hud_timer.mode_coundown)
		{
			if ((--hud_timer.current_value) == 0)
				do_alert1_sound();
		}
		else
			hud_timer.current_value++;
		if (hud_timer.current_value > hud_timer.max_value)
			hud_timer.current_value = 0;
		else if (hud_timer.current_value < 0)
			hud_timer.current_value = 0;
	}
	use_it = !use_it;
}


/* display the current time for the hud timer, coloured by stopped or running */
static int display_timer(window_info *win, int base_y_start)
{
	char str[10];
	int x;
	base_y_start -= hud_timer.height;
	safe_snprintf(str, sizeof(str), "%c%1d:%02d", ((hud_timer.mode_coundown) ?countdown_str[0] :stopwatch_str[0]), hud_timer.current_value/60, hud_timer.current_value%60);
	x= 3+(win->len_x - (get_string_width((unsigned char*)str)*11)/12)/2;
	if (hud_timer.running)
		draw_string_shadowed(x, 2 + base_y_start, (unsigned char*)str, 1,0.5f, 1.0f, 0.5f,0.0f,0.0f,0.0f);
	else
		draw_string_shadowed(x, 2 + base_y_start, (unsigned char*)str, 1,1.0f, 0.5f, 0.5f,0.0f,0.0f,0.0f);
	if (hud_timer.mouse_over)
	{
		char *use_str = ((hud_timer.mode_coundown) ?countdown_str:stopwatch_str);
		draw_string_small_shadowed(-(int)(SMALL_FONT_X_LEN*(strlen(use_str)+0.5)), base_y_start, (unsigned char*)use_str, 1,1.0f,1.0f,1.0f,0.0f,0.0f,0.0f);
		hud_timer.mouse_over = 0;
	}
	return base_y_start;
}


/* return true if the coords are over the hud timer */
static int mouse_is_over_timer(window_info *win, int mx, int my)
{
	if (view_hud_timer)
	{
		int bar_y_pos = win->len_y - 64;
		if (view_analog_clock) bar_y_pos -= 64;
		if (view_digital_clock) bar_y_pos -= DEFAULT_FONT_Y_LEN;
		if (view_knowledge_bar) bar_y_pos -= knowledge_bar_height;
		if ((my > (bar_y_pos - hud_timer.height)) && (my < bar_y_pos))
			return 1;
	}
	return 0;
}


/*
 * Control the hud timer by variosu mouse clicks:
 * Shift+click changed mode Countdown / Stopwatch
 * For Countdown, mouse wheel up/down decrease/increase start time.
 * 		Defaul step 5, +ctrl 1, +alt 30
 * Left-click start/stop timer
 * Mouse wheel click - reset timer.
 */
static int mouse_click_timer(Uint32 flags)
{
	/* change countdown start */
	if (flags & (ELW_WHEEL_DOWN|ELW_WHEEL_UP))
	{
		int step = 5;
		if (!hud_timer.mode_coundown)
			return 1;
		if (flags & ELW_CTRL)
			step = 1;
		else if (flags & ELW_ALT)
			step = 30;
		hud_timer.running = 0;
		hud_timer.start_value += step * ((flags & ELW_WHEEL_DOWN)!=0) - step * ((flags & ELW_WHEEL_UP)!=0);
		if (hud_timer.start_value < 0)
			hud_timer.start_value = 0;
		else if (hud_timer.start_value > hud_timer.max_value)
			hud_timer.start_value = hud_timer.max_value;
		hud_timer.current_value = hud_timer.start_value;
	}
	/* control mode */
	else if (flags & ELW_SHIFT)
	{
		if (hud_timer.mode_coundown)
			hud_timer.mode_coundown = hud_timer.current_value = 0;
		else
		{
			hud_timer.mode_coundown = 1;
			hud_timer.current_value = hud_timer.start_value;
		}
		hud_timer.running = 0;
		do_window_close_sound();
	}
	else
	{
		/* reset */
		if (flags & ELW_MID_MOUSE)
			hud_timer.current_value = (hud_timer.mode_coundown) ?hud_timer.start_value : 0;
		/* start / stop */
		else if (flags & ELW_LEFT_MOUSE)
			hud_timer.running = !hud_timer.running;
		do_click_sound();
	}
	return 1;
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
#ifndef NEW_NEW_CHAR_WINDOW
	static hud_interface last_interface = HUD_INTERFACE_NEW_CHAR;
#endif

	if (type == HUD_INTERFACE_LAST)
		type = last_interface;

	init_hud_frame ();
#ifndef NEW_NEW_CHAR_WINDOW
	init_misc_display (type);
#else
	if(type == HUD_INTERFACE_GAME)
		init_misc_display (type);
#endif

	if (type == HUD_INTERFACE_NEW_CHAR)
	{
#ifdef NEW_NEW_CHAR_WINDOW
		hud_x=270;
		resize_root_window();
#endif
		init_icon_window (NEW_CHARACTER_ICONS);
	}
	else
	{
#ifdef NEW_NEW_CHAR_WINDOW
		if (hud_x>0)
			hud_x=HUD_MARGIN_X;
		resize_root_window();
#endif
		init_icon_window (MAIN_WINDOW_ICONS);
		init_stats_display ();
		init_quickbar ();
		init_quickspell ();
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
}

void hide_hud_windows ()
{
	if (icons_win >= 0) hide_window (icons_win);
	if (stats_bar_win >= 0) hide_window (stats_bar_win);
	if (misc_win >= 0) hide_window (misc_win);
	if (quickbar_win >= 0) hide_window (quickbar_win);
	if (quickspell_win >= 0) hide_window (quickspell_win);
}

// draw everything related to the hud
void draw_hud_interface()
{
	glColor3f(1.0f, 1.0f, 1.0f);
	draw_hud_frame();
}

// check to see if a mouse click was on the hud
// used in non-standard modes
int check_hud_interface()
{
	return click_in_windows(mouse_x, mouse_y, 0);	// temporarily here for testing
}

// hud frame section
float vertical_bar_u_start = (float)192/256;
float vertical_bar_u_end = 1.0f;
float vertical_bar_v_end = 0.0f;
float vertical_bar_v_start = 0.0f;

float horizontal_bar_u_start = (float)144/256;
float horizontal_bar_u_end = (float)191/256;
float horizontal_bar_v_start = 0.0f;
float horizontal_bar_v_end = 0.0f;

void init_hud_frame()
{
#ifdef	NEW_TEXTURES
	vertical_bar_v_end = (float)window_height/256;
	horizontal_bar_v_start = (float)(window_width-hud_x)/256;
#else	/* NEW_TEXTURES */
	vertical_bar_v_start = (float)window_height/256;
	horizontal_bar_v_end = (float)(window_width-hud_x)/256;
#endif	/* NEW_TEXTURES */
}

#ifdef	NEW_TEXTURES
float logo_u_start = (float)64/256;
float logo_v_start = (float)128/256;
float logo_u_end = (float)127/256;
float logo_v_end = (float)191/256;
#else	/* NEW_TEXTURES */
float logo_u_start=(float)64/256;
float logo_v_start=1.0f-(float)128/256;

float logo_u_end=(float)127/256;
float logo_v_end=1.0f-(float)191/256;
#endif	/* NEW_TEXTURES */

void draw_hud_frame()
{
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
#ifdef	NEW_TEXTURES
	bind_texture(hud_text);
#else	/* NEW_TEXTURES */
	get_and_set_texture_id(hud_text);
#endif	/* NEW_TEXTURES */
	glBegin(GL_QUADS);
#ifndef NEW_NEW_CHAR_WINDOW
	draw_2d_thing(vertical_bar_u_start, vertical_bar_v_start, vertical_bar_u_end, vertical_bar_v_end,window_width-hud_x, 0, window_width, window_height);
	draw_2d_thing_r(horizontal_bar_u_start, horizontal_bar_v_start, horizontal_bar_u_end, horizontal_bar_v_end,0,window_height,window_width-hud_x , window_height-hud_y);
#else
	draw_2d_thing_r(horizontal_bar_u_start, horizontal_bar_v_start, horizontal_bar_u_end, horizontal_bar_v_end,0,window_height,window_width, window_height-hud_y);
#endif
#ifdef NEW_NEW_CHAR_WINDOW
	if(last_interface == HUD_INTERFACE_GAME)
	{
		draw_2d_thing(vertical_bar_u_start, vertical_bar_v_start, vertical_bar_u_end, vertical_bar_v_end,window_width-hud_x, 0, window_width, window_height);
#endif
	//draw the logo
	draw_2d_thing(logo_u_start, logo_v_start, logo_u_end, logo_v_end,window_width-hud_x, 0, window_width, 64);
#ifdef NEW_NEW_CHAR_WINDOW
	}
#endif
	glEnd();
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void switch_action_mode(int * mode, int id)
{
	item_action_mode=qb_action_mode=action_mode=*mode;
}

void view_console_win (int *win, int id)
{
	if ( get_show_window (console_root_win) && !locked_to_console )
	{
		hide_window (console_root_win);
		show_window (game_root_win);
		// Undo stupid quickbar hack
		if ( !get_show_window (quickbar_win) )
			show_window (quickbar_win);
		if ( !get_show_window (quickspell_win) )
			show_window (quickspell_win);
	}
	else
	{
		if ( get_show_window (game_root_win) )
			hide_window (game_root_win);
		if ( get_show_window (map_root_win) )
		{
			switch_from_game_map ();
			hide_window (map_root_win);
		}
		show_window (console_root_win);
	}
}

void view_map_win (int * win, int id)
{
	if ( get_show_window (map_root_win) && !locked_to_console )
	{
		switch_from_game_map ();
		hide_window (map_root_win);
		show_window (game_root_win);
		// Undo stupid quickbar hack
		if ( !get_show_window (quickbar_win) )
			show_window (quickbar_win);

	}
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
		{ "minimap", &minimap_win },
		{ "name_pass", &namepass_win },
		{ "customize", &color_race_win } };
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
#ifndef NEW_NEW_CHAR_WINDOW
			else if(window==&namepass_win) show_account_win();
			else if(window==&color_race_win) show_color_race_win();
#endif
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


void show_help(const char *help_message, int x, int y)
{
	show_help_coloured(help_message, x, y, 1.0f, 1.0f, 1.0f);
}

void show_help_coloured(const char *help_message, int x, int y, float r, float g, float b)
{
	int len=strlen(help_message)*8+1;
	int width=window_width-80;

	if(x+len>width) x-=(x+len)-width;

	glColor4f(0.0f,0.0f,0.0f,0.5f);
	glDisable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glBegin(GL_QUADS);
	glVertex3i(x-1,y+SMALL_FONT_Y_LEN,0);
	glVertex3i(x-1,y,0);
	glVertex3i(x+len,y,0);
	glVertex3i(x+len,y+SMALL_FONT_Y_LEN,0);
	glEnd();

	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);

	glColor3f(r,g,b);
	draw_string_small(x, y, (unsigned char*)help_message, 1);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

// Stats bars in the bottom HUD .....

int show_action_bar = 0;
int watch_this_stats[MAX_WATCH_STATS]={NUM_WATCH_STAT -1, 0, 0, 0, 0};  // default to only watching overall
int max_food_level=45;
static int exp_bar_text_len = 9.5*SMALL_FONT_X_LEN;
static const int stats_bar_text_len = 4.5 * SMALL_FONT_X_LEN;

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
		return SMALL_FONT_X_LEN*(max_len+1.5);
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
	else if (prosed_len > 100)
		prosed_len = 100;

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
static int free_statbar_space;

// clear the context menu regions for all stats bars and set up again
static void reset_statsbar_exp_cm_regions(void)
{
	size_t i;
	cm_remove_regions(stats_bar_win);
	for (i=0; i<max_disp_stats; i++)
		if (watch_this_stats[i] > 0)
			cm_add_region(cm_id, stats_bar_win, exp_bar_start_x+i*(stats_bar_len+exp_bar_text_len), exp_bar_start_y, stats_bar_len, 12);
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

	//create the stats bar window
	if(stats_bar_win < 0)
	{
		static size_t cm_id_ap = CM_INIT_VALUE;
		stats_bar_win= create_window("Stats Bar", -1, 0, 0, window_height-44, window_width-HUD_MARGIN_X, 12, ELW_TITLE_NONE|ELW_SHOW_LAST);
		set_window_handler(stats_bar_win, ELW_HANDLER_DISPLAY, &display_stats_bar_handler);
		set_window_handler(stats_bar_win, ELW_HANDLER_MOUSEOVER, &mouseover_stats_bar_handler);

		// context menu to enable/disable the action points bar
		cm_id_ap = cm_create(cm_action_points_str, NULL);
		cm_add_window(cm_id_ap, stats_bar_win);
		cm_bool_line(cm_id_ap, 0, &show_action_bar, "show_action_bar");
	}
	else
		init_window(stats_bar_win, -1, 0, 0, window_height-44, window_width-HUD_MARGIN_X, 12);

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

	// the x position of the first exp bar
	exp_bar_start_x = ((show_action_bar)?5:4) * (stats_bar_len + stats_bar_text_len) + exp_bar_text_len;

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

	free_statbar_space = (window_width-HUD_MARGIN_X-1) -
		(exp_bar_start_x - exp_bar_text_len + actual_num_exp * (exp_bar_text_len + stats_bar_len));

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

void draw_stats_bar(int x, int y, int val, int len, float r, float g, float b, float r2, float g2, float b2)
{
	char buf[32];
	int i; // i deals with massive bars by trimming at 110%
	
	if(len>stats_bar_len*1.1)
		i=stats_bar_len*1.1;
	else
		i=len;
	glDisable(GL_TEXTURE_2D);
	
	if(i >= 0){
		glBegin(GL_QUADS);
		//draw the colored section
 		glColor3f(r2, g2, b2);
		glVertex3i(x, y+8, 0);
		glColor3f(r, g, b);
		glVertex3i(x, y, 0);
		glColor3f(r, g, b);
		glVertex3i(x+i, y, 0);
		glColor3f(r2, g2, b2);
		glVertex3i(x+i, y+8, 0);
		glEnd();
	}
	// draw the bar frame
	glColor3f(0.77f, 0.57f, 0.39f);
	glBegin(GL_LINE_LOOP);
	glVertex3i(x, y, 0);
	glVertex3i(x+stats_bar_len, y, 0);
	glVertex3i(x+stats_bar_len, y+8, 0);
	glVertex3i(x, y+8, 0);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	
	// handle the text
	safe_snprintf(buf, sizeof(buf), "%d", val);
	//glColor3f(0.8f, 0.8f, 0.8f); moved to next line
	draw_string_small_shadowed(x-(1+8*strlen(buf))-1, y-2, (unsigned char*)buf, 1,0.8f, 0.8f, 0.8f,0.0f,0.0f,0.0f);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

static void draw_side_stats_bar(const int x, const int y, const int baselev, const int cur_exp, const int nl_exp, size_t colour)
{
	int len = 58-58.0f/(float)((float)(nl_exp-exp_lev[baselev])/(float)(nl_exp-cur_exp));

	GLfloat colours[2][2][3] = { { {0.11f, 0.11f, 0.11f}, {0.3f, 0.5f, 0.2f} },
								 { {0.10f,0.10f,0.80f}, {0.40f,0.40f,1.00f} } };

	if (colour > 1)
		colour = 0;

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	glDisable(GL_TEXTURE_2D);
	if(len >= 0){
		glBegin(GL_QUADS);
		//draw the colored section
		glColor3fv(colours[colour][0]);
		glVertex3i(x, y+13, 0);
		glColor3fv(colours[colour][1]);
		glVertex3i(x, y, 0);
		glColor3fv(colours[colour][1]);
		glVertex3i(x+len, y, 0);
		glColor3fv(colours[colour][0]);
		glVertex3i(x+len, y+13, 0);
		glEnd();
	}
	
	// draw the bar frame
	glColor3f(0.77f, 0.57f, 0.39f);
	glBegin(GL_LINE_LOOP);
	glVertex3i(x, y, 0);
	glVertex3i(x+58, y, 0);
	glVertex3i(x+58, y+13, 0);
	glVertex3i(x, y+13, 0);
	glEnd();
	glEnable(GL_TEXTURE_2D);
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
static void draw_last_health_change(void)
{
	char str[20];
	static const Uint32 timeoutms = 2*60*1000;
	static const int yoff = (int)(-(SMALL_FONT_Y_LEN+5));
	/* damage in red */
	if (my_last_health.d != 0)
	{
		if (abs(SDL_GetTicks() - my_last_health.dt) > timeoutms)
			my_last_health.d = 0;
		else
		{
			safe_snprintf(str, sizeof(str), " %d ", my_last_health.d);
			show_help_coloured(str, health_bar_start_x+stats_bar_len/2-strlen(str)*SMALL_FONT_X_LEN, yoff, 1.0f, 0.0f, 0.0f);
		}
	}
	/* heal in green */
	if (my_last_health.h != 0)
	{
		if (abs(SDL_GetTicks() - my_last_health.ht) > timeoutms)
			my_last_health.h = 0;
		else
		{
			safe_snprintf(str, sizeof(str), " %d ", my_last_health.h);
			show_help_coloured(str, health_bar_start_x+stats_bar_len/2, yoff, 0.0f, 1.0f, 0.0f);
		}
	}
}

int	display_stats_bar_handler(window_info *win)
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
	if (abs(SDL_GetTicks()-last_time) > 250)
	{
		int proposed_len = recalc_exp_bar_text_len();
		if (proposed_len != exp_bar_text_len) // it will very rarely change
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

	draw_stats_bar(health_bar_start_x, health_bar_start_y, your_info.material_points.cur, health_adjusted_x_len, 1.0f, 0.2f, 0.2f, 0.5f, 0.2f, 0.2f);

	if (your_info.food_level<=max_food_level) //yellow
		draw_stats_bar(food_bar_start_x, food_bar_start_y, your_info.food_level, food_adjusted_x_len, 1.0f, 1.0f, 0.2f, 0.5f, 0.5f, 0.2f);
	else draw_stats_bar(food_bar_start_x, food_bar_start_y, your_info.food_level, food_adjusted_x_len, 1.0f, 0.5f, 0.0f, 0.7f, 0.3f, 0.0f); //orange

	draw_stats_bar(mana_bar_start_x, mana_bar_start_y, your_info.ethereal_points.cur, mana_adjusted_x_len, 0.2f, 0.2f, 1.0f, 0.2f, 0.2f, 0.5f);
	draw_stats_bar(load_bar_start_x, load_bar_start_y, your_info.carry_capacity.base-your_info.carry_capacity.cur, load_adjusted_x_len, 0.6f, 0.4f, 0.4f, 0.4f, 0.2f, 0.2f);
	if (show_action_bar)
		draw_stats_bar(action_bar_start_x, action_bar_start_y, your_info.action_points.cur, action_adjusted_x_len, 0.8f, 0.3f, 0.8f, 0.5f, 0.1f, 0.5f);

	draw_exp_display();

	if(show_help_text && statbar_cursor_x>=0)
	{
		if(over_health_bar) show_help((char*)attributes.material_points.name,health_bar_start_x+stats_bar_len+10,-3);
		else if(statbar_cursor_x>food_bar_start_x && statbar_cursor_x < food_bar_start_x+stats_bar_len) show_help((char*)attributes.food.name,food_bar_start_x+stats_bar_len+10,-3);
		else if(statbar_cursor_x>mana_bar_start_x && statbar_cursor_x < mana_bar_start_x+stats_bar_len) show_help((char*)attributes.ethereal_points.name,mana_bar_start_x+stats_bar_len+10,-3);
		else if(statbar_cursor_x>load_bar_start_x && statbar_cursor_x < load_bar_start_x+stats_bar_len) show_help((char*)attributes.carry_capacity.name,load_bar_start_x+stats_bar_len+10,-3);
		else if(show_action_bar && statbar_cursor_x>action_bar_start_x && statbar_cursor_x < action_bar_start_x+stats_bar_len) show_help((char*)attributes.action_points.name,action_bar_start_x+stats_bar_len+10,-3);
	}

	if (over_health_bar)
		draw_last_health_change();

	statbar_cursor_x = -1;

	return 1;
}

int mouseover_stats_bar_handler(window_info *win, int mx, int my)
{
	statbar_cursor_x=mx;
	return 0;
}

// the misc section (compass, clock, ?)
#ifdef	NEW_TEXTURES
float compass_u_start = (float)32/256;
float compass_v_start = (float)192/256;

float compass_u_end = (float)95/256;
float compass_v_end = 1.0f;

float clock_u_start = 0.0f;
float clock_v_start = (float)128/256;

float clock_u_end = (float)63/256;
float clock_v_end = (float)191/256;

float needle_u_start = (float)4/256;
float needle_v_start = (float)200/256;

float needle_u_end = (float)14/256;
float needle_v_end = (float)246/256;

float clock_needle_u_start = (float)21/256;
float clock_needle_v_start = (float)192/256;

float clock_needle_u_end = (float)31/256;
float clock_needle_v_end = (float)223/256;
#else	/* NEW_TEXTURES */
float compass_u_start=(float)32/256;
float compass_v_start=1.0f-(float)192/256;

float compass_u_end=(float)95/256;
float compass_v_end=0;

float clock_u_start=0;
float clock_v_start=1.0f-(float)128/256;

float clock_u_end=(float)63/256;
float clock_v_end=1.0f-(float)191/256;

float needle_u_start=(float)4/256;
float needle_v_start=1.0f-(float)200/256;

float needle_u_end=(float)14/256;
float needle_v_end=1.0f-(float)246/256;

float clock_needle_u_start=(float)21/256;
float clock_needle_v_start=1.0f-(float)192/256;

float clock_needle_u_end=(float)31/256;
float clock_needle_v_end=1.0f-(float)223/256;
#endif	/* NEW_TEXTURES */

static int context_hud_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	unsigned char protocol_name;
	switch (option)
	{
		case CMH_MINIMAP: view_window(&minimap_win, 0); break;
		case CMH_RANGSTATS: view_window(&range_win, 0); break;
#ifdef NEW_SOUND
		case CMH_SOUND: toggle_sounds(&sound_on); set_var_unsaved("enable_sound", INI_FILE_VAR); break;
		case CMH_MUSIC: toggle_music(&music_on); set_var_unsaved("enable_music", INI_FILE_VAR); break;
#endif // NEW_SOUND
		case CMH_LOCATION:
			copy_next_LOCATE_ME = 1;
			protocol_name= LOCATE_ME;
			my_tcp_send(my_socket,&protocol_name,1);
			break;
	}
	return 1;
}

static int context_quickbar_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	switch (option)
	{
		case CMQB_DRAG: quickbar_draggable ^= 1; toggle_quickbar_draggable(); break;
		case CMQB_RESET: reset_quickbar(); break;
		case CMQB_FLIP: flip_quickbar(); break;
	}
	return 1;
}

static void context_hud_pre_show_handler(window_info *win, int widget_id, int mx, int my, window_info *cm_win)
{
#ifdef NEW_SOUND
	cm_sound_enabled = sound_on;
	cm_music_enabled = music_on;
#endif // NEW_SOUND
	cm_minimap_shown = get_show_window(minimap_win);
	cm_rangstats_shown = get_show_window(range_win);
}

void init_misc_display(hud_interface type)
{
	int y_len = 128 + DEFAULT_FONT_Y_LEN + knowledge_bar_height + get_height_of_timer() + (NUM_WATCH_STAT-1) * stats_bar_height;
	int i;
	//create the misc window
	if(misc_win < 0)
		{
			misc_win= create_window("Misc", -1, 0, window_width-HUD_MARGIN_X, window_height-y_len, HUD_MARGIN_X, y_len, ELW_TITLE_NONE|ELW_SHOW_LAST);
			set_window_handler(misc_win, ELW_HANDLER_DISPLAY, &display_misc_handler);
			set_window_handler(misc_win, ELW_HANDLER_CLICK, &click_misc_handler);
			set_window_handler(misc_win, ELW_HANDLER_MOUSEOVER, &mouseover_misc_handler );
			cm_hud_id = cm_create(cm_hud_menu_str, context_hud_handler);
			cm_bool_line(cm_hud_id, CMH_STATS, &show_stats_in_hud, "show_stats_in_hud");
			cm_bool_line(cm_hud_id, CMH_STATBARS, &show_statbars_in_hud, "show_statbars_in_hud");
			cm_bool_line(cm_hud_id, CMH_KNOWBAR, &view_knowledge_bar, "view_knowledge_bar");
			cm_bool_line(cm_hud_id, CMH_TIMER, &view_hud_timer, "view_hud_timer");
			cm_bool_line(cm_hud_id, CMH_DIGCLOCK, &view_digital_clock, "view_digital_clock");
			cm_bool_line(cm_hud_id, CMH_ANACLOCK, &view_analog_clock, "view_analog_clock");
			cm_bool_line(cm_hud_id, CMH_SECONDS, &show_game_seconds, "show_game_seconds");
			cm_bool_line(cm_hud_id, CMH_FPS, &show_fps, "show_fps");
			cm_bool_line(cm_hud_id, CMH_MINIMAP, &cm_minimap_shown, NULL);
			cm_bool_line(cm_hud_id, CMH_RANGSTATS, &cm_rangstats_shown, NULL);
			cm_bool_line(cm_hud_id, CMH_QUICKBM, &cm_quickbar_enabled, NULL);
			cm_bool_line(cm_hud_id, CMH_SOUND, &cm_sound_enabled, NULL);
			cm_bool_line(cm_hud_id, CMH_MUSIC, &cm_music_enabled, NULL);
			cm_add_window(cm_hud_id, misc_win);
			cm_set_pre_show_handler(cm_hud_id, context_hud_pre_show_handler);
		}
	else
		{
			move_window(misc_win, -1, 0, window_width-HUD_MARGIN_X, window_height-y_len);
		}
	
	cm_grey_line(cm_hud_id, CMH_STATS, (type == HUD_INTERFACE_NEW_CHAR));
	cm_grey_line(cm_hud_id, CMH_STATBARS, (type == HUD_INTERFACE_NEW_CHAR));
	cm_grey_line(cm_hud_id, CMH_FPS, (type == HUD_INTERFACE_NEW_CHAR));
	cm_grey_line(cm_hud_id, CMH_MINIMAP, (type == HUD_INTERFACE_NEW_CHAR));
	cm_grey_line(cm_hud_id, CMH_RANGSTATS, (type == HUD_INTERFACE_NEW_CHAR));
	cm_grey_line(cm_hud_id, CMH_QUICKBM, (type == HUD_INTERFACE_NEW_CHAR));
	cm_grey_line(cm_hud_id, CMH_LOCATION, (type == HUD_INTERFACE_NEW_CHAR));

	for (i=0; i<MAX_WATCH_STATS; i++)
	{
		if (watch_this_stats[i] > 0)
			statsinfo[watch_this_stats[i]-1].is_selected = 1;
	}
}


/*	Get the longest of the active quickspells and the
	quickbar (if its in default place)
*/
static int get_max_quick_y(void)
{
	int quickspell_base = get_quickspell_y_base();
	int quickbar_base = get_quickbar_y_base();
	int max_quick_y = window_height;
	
	if (quickspell_base > quickbar_base)
		max_quick_y -= quickspell_base;
	else
		max_quick_y -= quickbar_base;

	return max_quick_y;
}


/*	Calculate the start y coord for the statsbar.
	Also calculates statbar_start_y, num_disp_stat and first_disp_stat
*/
static int calc_statbar_start_y(int base_y_start, int win_y_len)
{
	int winoverlap = 0;

	statbar_start_y = base_y_start - stats_bar_height*(NUM_WATCH_STAT-1);
	
	/* calculate the overlap between the longest of the quickspell/bar and the statsbar */
	winoverlap = (win_y_len - statbar_start_y) - get_max_quick_y();

	/* if they overlap, only display some skills and allow to scroll */
	if (winoverlap > 0)
	{
		num_disp_stat = (NUM_WATCH_STAT-1) - (winoverlap + stats_bar_height-1)/stats_bar_height;
		statbar_start_y = base_y_start - stats_bar_height*num_disp_stat;
		if ((first_disp_stat + num_disp_stat) > (NUM_WATCH_STAT-1))
			first_disp_stat = (NUM_WATCH_STAT-1) - num_disp_stat;
	}
	/* else display them all */
	else
	{
		first_disp_stat = 0;
		num_disp_stat = (NUM_WATCH_STAT-1);
	}
	
	/* return start y position in window */
	return statbar_start_y;
}



int display_misc_handler(window_info *win)
{
	int base_y_start = win->len_y - (view_analog_clock?128:64) - (view_digital_clock?DEFAULT_FONT_Y_LEN:0);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
#ifdef	NEW_TEXTURES
	bind_texture(hud_text);
#else	/* NEW_TEXTURES */
	get_and_set_texture_id(hud_text);
#endif	/* NEW_TEXTURES */

	// allow for transparency
	glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
	glAlphaFunc(GL_GREATER, 0.09f);
	
	glBegin(GL_QUADS);

	//draw the compass
	draw_2d_thing(compass_u_start, compass_v_start, compass_u_end, compass_v_end, 0,win->len_y-64,64,win->len_y);
	if(view_analog_clock > 0){
		//draw the clock
		draw_2d_thing(clock_u_start, clock_v_start, clock_u_end, clock_v_end,
				  0, win->len_y-128, 64, win->len_y-64);
	}
	glEnd();

	//draw the compass needle
	glPushMatrix();
	glTranslatef(32, win->len_y-32, 0);
	glRotatef(rz, 0.0f, 0.0f, 1.0f);

	glBegin(GL_QUADS);
	draw_2d_thing(needle_u_start, needle_v_start, needle_u_end, needle_v_end,-5, -28, 5, 28);
	glEnd();
	glPopMatrix();

	if(view_analog_clock > 0){
		//draw the clock needle
		glAlphaFunc(GL_GREATER, 0.05f);
		glPushMatrix();
		glTranslatef(32, win->len_y-96, 0);
		glRotatef(real_game_minute, 0.0f, 0.0f, 1.0f);
		glBegin(GL_QUADS);
		draw_2d_thing(clock_needle_u_start, clock_needle_v_start, clock_needle_u_end, clock_needle_v_end, -5, -24,5, 6);
		glEnd();
		glPopMatrix();
		glDisable(GL_ALPHA_TEST);
	}

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	//Digital Clock
	if(view_digital_clock > 0){
		int x;
		char str[10];

		//glColor3f(0.77f, 0.57f, 0.39f); // useless
		if (show_game_seconds)
		{
			safe_snprintf(str, sizeof(str), "%1d:%02d:%02d", real_game_minute/60, real_game_minute%60, real_game_second);
			draw_string_shadowed_width(5, 4 + base_y_start, (unsigned char*)str, win->len_x-5, 1, 0.77f, 0.57f, 0.39f, 0.0f, 0.0f, 0.0f);
		}
		else
		{
			safe_snprintf(str, sizeof(str), "%1d:%02d", real_game_minute/60, real_game_minute%60);
			x= 3+(win->len_x - (get_string_width((unsigned char*)str)*11)/12)/2;
			draw_string_shadowed(x, 2 + base_y_start, (unsigned char*)str, 1,0.77f, 0.57f, 0.39f,0.0f,0.0f,0.0f);
		}
	}

	/* if mouse over the either of the clocks - display the time & date */
	if (mouse_over_clock)
	{
		char str[20];
		const char *the_date = get_date(NULL);
		int centre_y =  (view_analog_clock) ?win->len_y-96 : base_y_start + DEFAULT_FONT_Y_LEN/2;

		safe_snprintf(str, sizeof(str), "%1d:%02d:%02d", real_game_minute/60, real_game_minute%60, real_game_second);
		draw_string_small_shadowed(-(int)(SMALL_FONT_X_LEN*(strlen(str)+0.5)), centre_y-SMALL_FONT_Y_LEN, (unsigned char*)str, 1, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f);
		if (the_date != NULL)
		{
			safe_snprintf(str, sizeof(str), "%s", the_date);
			draw_string_small_shadowed(-(int)(SMALL_FONT_X_LEN*(strlen(str)+0.5)), centre_y, (unsigned char*)str, 1, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f);
		}
		mouse_over_clock = 0;
	}

	/* if mouse over the compass - display the coords */
	if (mouse_over_compass)
	{
		char str[12];
		actor *me = get_our_actor ();
		if (me != NULL)
		{
			safe_snprintf(str, sizeof(str), "%d,%d", me->x_tile_pos, me->y_tile_pos);
			draw_string_small_shadowed(-(int)(SMALL_FONT_X_LEN*(strlen(str)+0.5)), win->len_y-64, (unsigned char*)str, 1, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f);
		}
		mouse_over_compass = 0;
	}

	/* if the knowledge bar is enabled, show progress in bar and ETA as hover over */
	if (view_knowledge_bar)
	{
		char str[20];
		char *use_str = idle_str;
		int percentage_done = 0;
		int x = 4;
		int y = base_y_start - stats_bar_height - ((knowledge_bar_height - stats_bar_height) / 2);
		int off = 0;
		
		if (is_researching())
		{
			percentage_done = (int)(100 * get_research_fraction());
			safe_snprintf(str, sizeof(str), "%d%%", percentage_done);
			use_str = str;
		}
		off = (58 - SMALL_FONT_X_LEN * strlen(use_str)) / 2;
		draw_side_stats_bar( x, y, 0, percentage_done, 100, 1);
		draw_string_small_shadowed(x+off+gx_adjust, y+gy_adjust, (unsigned char *)use_str, 1,1.0f,1.0f,1.0f,0.0f,0.0f,0.0f);

		if (mouse_over_knowledge_bar)
		{
			use_str = (is_researching()) ?get_research_eta_str(str, sizeof(str)) : not_researching_str;
			draw_string_small_shadowed(-(int)(SMALL_FONT_X_LEN*(strlen(use_str)+0.5)), y+gy_adjust, (unsigned char*)use_str, 1,1.0f,1.0f,1.0f,0.0f,0.0f,0.0f);
			mouse_over_knowledge_bar = 0;
		}

		base_y_start -= knowledge_bar_height;
	}

	/* if the timer is visable, draw it */
	if (view_hud_timer)
		base_y_start = display_timer(win, base_y_start);

	// Trade the number of quickbar slots if too much is displayed (not considering stats yet)
	while (((win->len_y - base_y_start) - get_max_quick_y()) > 0)
	{
		num_quickbar_slots--;
		set_var_OPT_INT("num_quickbar_slots", num_quickbar_slots);
	}

	/*	Optionally display the stats bar.  If the current window size does not
		provide enough room, display only some skills and allow scrolling to view
		the rest */
	if(show_stats_in_hud && have_stats)
	{
		char str[20];
		int x = 6;
		int thestat;
		int y = calc_statbar_start_y(base_y_start, win->len_y);
		int skill_modifier;

		// trade the number of quickbar slots if there is not enough space
		while ((num_disp_stat < 5) && (num_quickbar_slots > 1))
		{
			num_quickbar_slots--;
			set_var_OPT_INT("num_quickbar_slots", num_quickbar_slots);
			y = calc_statbar_start_y(base_y_start, win->len_y);
		}

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
		
		for (thestat=0; thestat<NUM_WATCH_STAT-1; thestat++)
		{
			int hover_offset = 0;
			
			/* skill skills until we have the skill displayed first */
			if (thestat < first_disp_stat)
				continue;
				
			/* end now if we have display all we can */
			if (thestat > (first_disp_stat+num_disp_stat-1))
				break;
		
			if (show_statbars_in_hud)
				draw_side_stats_bar( x-2, y+1, statsinfo[thestat].skillattr->base,
					*statsinfo[thestat].exp, *statsinfo[thestat].next_lev, 0);
		
			safe_snprintf(str,sizeof(str),"%-3s %3i",
				statsinfo[thestat].skillnames->shortname,
				statsinfo[thestat].skillattr->base );
			if (statsinfo[thestat].is_selected == 1)
				draw_string_small_shadowed(x+gx_adjust, y+gy_adjust, (unsigned char*)str, 1,0.77f, 0.57f, 0.39f,0.0f,0.0f,0.0f);
			else
				draw_string_small_shadowed(x+gx_adjust, y+gy_adjust, (unsigned char*)str, 1,1.0f,1.0f,1.0f,0.0f,0.0f,0.0f);
			
			if((thestat!=NUM_WATCH_STAT-2) && floatingmessages_enabled &&
				(skill_modifier = statsinfo[thestat].skillattr->cur -
				 	statsinfo[thestat].skillattr->base) != 0){
				safe_snprintf(str,sizeof(str),"%+i",skill_modifier);
				hover_offset = strlen(str)+1;
				if(skill_modifier > 0){
					draw_string_small_shadowed(-(int)(SMALL_FONT_X_LEN*(strlen(str)+0.5)), y+gy_adjust, (unsigned char*)str, 1,0.3f, 1.0f, 0.3f,0.0f,0.0f,0.0f);
				} else {
					draw_string_small_shadowed(-(int)(SMALL_FONT_X_LEN*(strlen(str)+0.5)), y+gy_adjust, (unsigned char*)str, 1,1.0f, 0.1f, 0.2f,0.0f,0.0f,0.0f);
				}
			}

			/* if the mouse is over the stat bar, draw the XP remaining */
			if (stat_mouse_is_over == thestat)
			{
				safe_snprintf(str,sizeof(str),"%li",(*statsinfo[thestat].next_lev - *statsinfo[thestat].exp));
				draw_string_small_shadowed(-(int)(SMALL_FONT_X_LEN*(strlen(str)+0.5+hover_offset)), y+gy_adjust, (unsigned char*)str, 1,1.0f,1.0f,1.0f,0.0f,0.0f,0.0f);
				stat_mouse_is_over = -1;
			}

			y+=stats_bar_height;
		}
	}

	{
		static int last_window_width = -1;
		static int last_window_height = -1;

		if (window_width != last_window_width || window_height != last_window_height)
		{
			cm_remove_regions(game_root_win);
			cm_remove_regions(map_root_win);
			cm_remove_regions(console_root_win);
			cm_remove_regions(newchar_root_win);
			cm_add_region(cm_hud_id, game_root_win, window_width-hud_x, 0, hud_x, window_height);
			cm_add_region(cm_hud_id, map_root_win, window_width-hud_x, 0, hud_x, window_height);
			cm_add_region(cm_hud_id, console_root_win, window_width-hud_x, 0, hud_x, window_height);
			cm_add_region(cm_hud_id, newchar_root_win, window_width-hud_x, 0, hud_x, window_height);
			last_window_width = window_width;
			last_window_height = window_height;
		}
	}
	
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return	1;
}

static int mouse_is_over_knowedge_bar(window_info *win, int mx, int my)
{
	if (view_knowledge_bar)
	{
		int bar_y_pos = win->len_y - 64;
		if (view_analog_clock) bar_y_pos -= 64;
		if (view_digital_clock) bar_y_pos -= DEFAULT_FONT_Y_LEN;
		if (my>bar_y_pos-knowledge_bar_height && my<bar_y_pos)
			return 1;
	}
	return 0;
}


int	click_misc_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int clockheight = 0;
	int in_stats_bar = 0;

	// handle scrolling the stats bars if not all displayed
	if (show_stats_in_hud && (my - statbar_start_y >= 0) && (my - statbar_start_y < num_disp_stat*stats_bar_height))
	{
		in_stats_bar = 1;

		if ((first_disp_stat > 0) && ((flags & ELW_WHEEL_UP) ||
			((flags & ELW_LEFT_MOUSE) && (flags & ELW_CTRL))))
		{
			first_disp_stat--;
			return 1;
		}
		else if ((first_disp_stat + num_disp_stat < NUM_WATCH_STAT-1) &&
				 ((flags & ELW_WHEEL_DOWN) || ((flags & ELW_RIGHT_MOUSE) && (flags & ELW_CTRL))))
		{
			first_disp_stat++;
			return 1;
		}
	}

	if (mouse_is_over_timer(win, mx, my))
		return mouse_click_timer(flags);

	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	// reserve CTRL clicks for scrolling
	if (flags & ELW_CTRL) return 0;

	//check to see if we clicked on the clock
	if(view_digital_clock>0){
		clockheight+=DEFAULT_FONT_Y_LEN;
	}
	if(view_analog_clock>0){
		clockheight+=64;
	}
	if(my>win->len_y-(64+clockheight) && my<win->len_y-64)
	{
		unsigned char protocol_name;
		do_click_sound();
		protocol_name= GET_TIME;
		my_tcp_send(my_socket,&protocol_name,1);
		return 1;
	}

	/* show research if click on the knowledge bar */
	if (mouse_is_over_knowedge_bar(win, mx, my))
	{
		do_click_sound();
		send_input_text_line("#research", 9);
		return 1;
	}

	//check to see if we clicked on the compass
	if(my>win->len_y-64 && my<win->len_y)
	{
		unsigned char protocol_name;
		do_click_sound();
		protocol_name= LOCATE_ME;
		if (flags & ELW_SHIFT)
		{
			copy_next_LOCATE_ME = 2;
		}
		my_tcp_send(my_socket,&protocol_name,1);
		return 1;
	}
	//check to see if we clicked on the stats
	if (in_stats_bar)
	{
		handle_stats_selection(first_disp_stat + ((my - statbar_start_y ) / stats_bar_height) + 1, flags);
		return 1;
	}

	return 0;
}

int mouseover_misc_handler(window_info *win, int mx, int my)
{
	/* Optionally display scrolling help if statsbar is active and restricted in size */
	if (show_help_text && show_stats_in_hud && (num_disp_stat < NUM_WATCH_STAT-1) &&
		(my - statbar_start_y >= 0) && (my - statbar_start_y < num_disp_stat*stats_bar_height))
		show_help(stats_scroll_help_str, -10-strlen(stats_scroll_help_str)*SMALL_FONT_X_LEN, win->len_y-70);

	/* stat hover experience left */
	if (show_stats_in_hud && have_stats && (my - statbar_start_y >= 0) && (my - statbar_start_y < num_disp_stat*stats_bar_height))
		stat_mouse_is_over = first_disp_stat + ((my - statbar_start_y ) / stats_bar_height);

	/* if the mouse is over either clock - display the date and time */
	if (view_analog_clock)
	{
		if (my>win->len_y-128 && my<win->len_y-64)
			mouse_over_clock = 1;
	}
	if (view_digital_clock && !mouse_over_clock)
	{
		int digital_clock_y_pos = win->len_y-64;
		if (view_analog_clock) digital_clock_y_pos-=64;
		if (my>digital_clock_y_pos-DEFAULT_FONT_Y_LEN && my<digital_clock_y_pos)
			mouse_over_clock = 1;
	}

	/* check if over the knowledge bar */
	if (mouse_is_over_knowedge_bar(win, mx, my))
		mouse_over_knowledge_bar = 1;

	/* check if over the timer */
	if (mouse_is_over_timer(win, mx, my))
		set_mouse_over_timer();

	/* if mouse over the compass - display the coords */
	if(my>win->len_y-64 && my<win->len_y)
		mouse_over_compass = 1;

	return 0;
}

/* #define as these numbers are used many times */
#define DEF_QUICKBAR_X_LEN 30
#define DEF_QUICKBAR_Y_LEN 30
#define DEF_QUICKBAR_X (DEF_QUICKBAR_X_LEN+4)
#define DEF_QUICKBAR_Y 64

int quickbar_x_len = DEF_QUICKBAR_X_LEN;
int quickbar_x = DEF_QUICKBAR_X;
int quickbar_y = DEF_QUICKBAR_Y;
int quickbar_draggable=0;
int quickbar_dir=VERTICAL;
int quickbar_relocatable=0;
int num_quickbar_slots = 6;
static int last_num_quickbar_slots = 6;

// return the window y len based on the number of slots
int get_quickbar_y_len(void)
{
	return num_quickbar_slots * DEF_QUICKBAR_Y_LEN + 1;
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


/* get the base y coord of the quick bar if its in 
   it's default place, otherwise return where the top would be */
int get_quickbar_y_base()
{
	if ((quickbar_draggable) || (quickbar_dir!=VERTICAL) ||
		(quickbar_x_len != DEF_QUICKBAR_X_LEN) || 
		(quickbar_x != DEF_QUICKBAR_X) || (quickbar_y != DEF_QUICKBAR_Y))
		return DEF_QUICKBAR_Y;
	else
		return DEF_QUICKBAR_Y + get_quickbar_y_len();
}

//quickbar section
void init_quickbar ()
{
	Uint32 flags = ELW_USE_BACKGROUND | ELW_USE_BORDER;
	
	quickbar_x_len = DEF_QUICKBAR_X_LEN;
	
	if (!quickbar_relocatable)
	{
		flags |= ELW_SHOW_LAST;
		quickbar_draggable = 0;
	}
	if (quickbar_draggable) flags |= ELW_TITLE_BAR | ELW_DRAGGABLE;	
	
	if (quickbar_win < 0)
	{
		if (quickbar_dir == VERTICAL)
			quickbar_win = create_window ("Quickbar", -1, 0, window_width - quickbar_x, quickbar_y, quickbar_x_len, get_quickbar_y_len(), flags);
		else
			quickbar_win = create_window ("Quickbar", -1, 0, window_width - quickbar_x, quickbar_y, get_quickbar_y_len(), quickbar_x_len, flags);
		last_num_quickbar_slots = num_quickbar_slots;

		set_window_handler(quickbar_win, ELW_HANDLER_DISPLAY, &display_quickbar_handler);
		set_window_handler(quickbar_win, ELW_HANDLER_CLICK, &click_quickbar_handler);
		set_window_handler(quickbar_win, ELW_HANDLER_MOUSEOVER, &mouseover_quickbar_handler );

		cm_quickbar_id = cm_create(cm_quickbar_menu_str, context_quickbar_handler);
		cm_bool_line(cm_quickbar_id, CMQB_RELOC, &quickbar_relocatable, "relocate_quickbar");
		cm_bool_line(cm_quickbar_id, CMQB_DRAG, &quickbar_draggable, NULL);
		cm_bool_line(cm_quickbar_id, CMQB_ENABLE, &cm_quickbar_enabled, NULL);
	}
	else
	{
		change_flags (quickbar_win, flags);
		if (quickbar_draggable) 
		{
			show_window (quickbar_win);
		}
		else if (quickbar_y > window_height || quickbar_x > window_width) 
		{
			move_window (quickbar_win, -1, 0, 200, 64); // The player has done something stupid... let him/her correct it
		}
		else
		{
			move_window (quickbar_win, -1, 0, window_width - quickbar_x, quickbar_y);
		}
	}
}

int	display_quickbar_handler(window_info *win)
{
	char str[80];
	int y, i;
	Uint32 _cur_time = SDL_GetTicks(); /* grab a snapshot of current time */

	// check if the number of slots has changes and adjust if needed
	if (last_num_quickbar_slots == -1)
		last_num_quickbar_slots = num_quickbar_slots;
	else if (last_num_quickbar_slots != num_quickbar_slots)
	{
		last_num_quickbar_slots = num_quickbar_slots;
		if (quickbar_dir==VERTICAL) 
			init_window(win->window_id, -1, 0, win->cur_x, win->cur_y, quickbar_x_len, get_quickbar_y_len());
		else
			init_window(win->window_id, -1, 0, win->cur_x, win->cur_y, get_quickbar_y_len(), quickbar_x_len);
	}

	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f,1.0f,1.0f);
	//ok, now let's draw the objects...
	for(i=0;i<num_quickbar_slots;i++)
	{
		if(item_list[i].quantity > 0)
		{
			float u_start,v_start,u_end,v_end;
			int this_texture,cur_item,cur_pos;
			int x_start,x_end,y_start,y_end, itmp;

			//get the UV coordinates.
			cur_item=item_list[i].image_id%25;
#ifdef	NEW_TEXTURES
			get_item_uv(cur_item, &u_start, &v_start, &u_end,
				&v_end);
#else	/* NEW_TEXTURES */
			u_start=0.2f*(cur_item%5);
			u_end=u_start+(float)50/256;
			v_start=(1.0f+((float)50/256)/256.0f)-((float)50/256*(cur_item/5));
			v_end=v_start-(float)50/256;
#endif	/* NEW_TEXTURES */

			//get the x and y
			cur_pos=item_list[i].pos;
					
			x_start= 2;
			x_end= x_start+27;
			y_start= DEF_QUICKBAR_Y_LEN*(cur_pos%num_quickbar_slots)+2;
			y_end= y_start+27;

			if(quickbar_dir != VERTICAL)
			{
				itmp = x_start; x_start = y_start; y_start = itmp;
				itmp = x_end; x_end = y_end; y_end = itmp;
			}

			//get the texture this item belongs to
			this_texture=get_items_texture(item_list[i].image_id/25);

#ifdef	NEW_TEXTURES
			bind_texture(this_texture);
#else	/* NEW_TEXTURES */
			get_and_set_texture_id(this_texture);
#endif	/* NEW_TEXTURES */
			glBegin(GL_QUADS);
				draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
			glEnd();
			
			if (item_list[i].cooldown_time > _cur_time)
			{
				float cooldown = ((float)(item_list[i].cooldown_time - _cur_time)) / ((float)item_list[i].cooldown_rate);
				float x_center = (x_start + x_end)*0.5f;
				float y_center = (y_start + y_end)*0.5f;

				if (cooldown < 0.0f)
					cooldown = 0.0f;
				else if (cooldown > 1.0f)
					cooldown = 1.0f;
				
				glDisable(GL_TEXTURE_2D);
				glEnable(GL_BLEND);
				
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glBegin(GL_TRIANGLE_FAN);
					glColor4f(0.14f, 0.35f, 0.82f, 0.50f); 

					glVertex2f(x_center, y_center);

					if (cooldown >= 0.875f) {
						float t = tan(2.0f*M_PI*(1.0f - cooldown));
						glVertex2f(t*x_end + (1.0f - t)*x_center, y_start);
						glVertex2f(x_end, y_start);
						glVertex2f(x_end, y_end);
						glVertex2f(x_start, y_end);
						glVertex2f(x_start, y_start);
					} else if (cooldown >= 0.625f) {
						float t = 0.5f + 0.5f*tan(2.0f*M_PI*(0.75f - cooldown));
						glVertex2f(x_end, t*y_end + (1.0f - t)*y_start);
						glVertex2f(x_end, y_end);
						glVertex2f(x_start, y_end);
						glVertex2f(x_start, y_start);
					} else if (cooldown >= 0.375f) {
						float t = 0.5f + 0.5f*tan(2.0f*M_PI*(0.5f - cooldown));
						glVertex2f(t*x_start + (1.0f - t)*x_end, y_end);
						glVertex2f(x_start, y_end);
						glVertex2f(x_start, y_start);
					} else if (cooldown >= 0.125f) {
						float t = 0.5f + 0.5f*tan(2.0f*M_PI*(0.25f - cooldown));
						glVertex2f(x_start, t*y_start + (1.0f - t)*y_end);
						glVertex2f(x_start, y_start);
					} else {
						float t = tan(2.0f*M_PI*(cooldown));
						glVertex2f(t*x_start + (1.0f - t)*x_center, y_start);
					}

					glVertex2f(x_center, y_start);
				glEnd();

				glDisable(GL_BLEND);
				glEnable(GL_TEXTURE_2D);
			}
			
			safe_snprintf(str,sizeof(str),"%i",item_list[i].quantity);
			draw_string_small_shadowed(x_start,y_end-SMALL_FONT_Y_LEN,(unsigned char*)str,1,1.0f,1.0f,1.0f,0.0f,0.0f,0.0f);
		}
	}
	
	// Render the grid *after* the images. It seems impossible to code
	// it such that images are rendered exactly within the boxes on all 
	// cards
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINES);
	use_window_color(quickbar_win, ELW_COLOR_LINE);
	//draw the grid
	if(quickbar_dir==VERTICAL)
		{
			for(y=1;y<num_quickbar_slots;y++)
				{
					glVertex3i(0, y*DEF_QUICKBAR_Y_LEN+1, 0);
					glVertex3i(quickbar_x_len, y*DEF_QUICKBAR_Y_LEN+1, 0);
				}
		}
	else
		{
			for(y=1;y<num_quickbar_slots;y++)
				{
					glVertex3i(y*DEF_QUICKBAR_Y_LEN+1, 0, 0);
					glVertex3i(y*DEF_QUICKBAR_Y_LEN+1, quickbar_x_len, 0);
				}
		}
	glEnd();
	glEnable(GL_TEXTURE_2D);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 1;
}

int last_type=0;

static void quickbar_item_description_help(window_info *win, int pos, int slot)
{
	Uint16 item_id = item_list[pos].id;
	int image_id = item_list[pos].image_id;
	if (show_item_desc_text && item_info_available() && (get_item_count(item_id, image_id) == 1))
	{
		const char *str = get_item_description(item_id, image_id);
		if (str != NULL)
		{
			int xpos = 0, ypos = 0;
			int len_str = (strlen(str) + 1) * SMALL_FONT_X_LEN;
			/* vertical place right (or left) and aligned with slot */
			if (quickbar_dir==VERTICAL)
			{
				xpos = win->len_x + 5;
				if ((xpos + len_str + win->cur_x) > window_width)
					xpos = -len_str;
				ypos = slot * DEF_QUICKBAR_Y_LEN + (DEF_QUICKBAR_Y_LEN - SMALL_FONT_Y_LEN) / 2;
			}
			/* horizontal place right at bottom (or top) of window */
			else
			{
				xpos = 0;
				ypos = win->len_y + 5;
				if ((xpos + len_str + win->cur_x) > window_width)
					xpos = window_width - win->cur_x - len_str;
				if ((xpos + win->cur_x) < 0)
					xpos = -win->cur_x + 5;
				if ((ypos + SMALL_FONT_Y_LEN + win->cur_y) > window_height)
					ypos = -(5 + SMALL_FONT_Y_LEN + (quickbar_draggable * ELW_TITLE_HEIGHT));
			}
			show_help(str, xpos, ypos);
		}
	}
}

int mouseover_quickbar_handler(window_info *win, int mx, int my) {
	int y,i=0;
	int x_screen,y_screen;
	for(y=0;y<num_quickbar_slots;y++)
		{
			if(quickbar_dir==VERTICAL)
				{
					x_screen=0;
					y_screen=y*DEF_QUICKBAR_Y_LEN;
				}
			else
				{
					x_screen=y*DEF_QUICKBAR_Y_LEN;
					y_screen=0;
				}
			if(mx>x_screen && mx<x_screen+DEF_QUICKBAR_Y_LEN && my>y_screen && my<y_screen+DEF_QUICKBAR_Y_LEN)
				{
					for(i=0;i<ITEM_NUM_ITEMS;i++){
						if(item_list[i].quantity && item_list[i].pos==y)
							{
								if(qb_action_mode==ACTION_LOOK) {
									elwin_mouse=CURSOR_EYE;
								} else if(qb_action_mode==ACTION_USE) {
									elwin_mouse=CURSOR_USE;
								} else if(qb_action_mode==ACTION_USE_WITEM) {
									elwin_mouse=CURSOR_USE_WITEM;
								} else {
									elwin_mouse=CURSOR_PICK;
								}
								quickbar_item_description_help(win, i, y);
								return 1;
							}
					}
				}
			}
	return 0;
}

int	click_quickbar_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int i,y;
	int x_screen,y_screen;
	Uint8 str[100];
	int trigger=ELW_LEFT_MOUSE|ELW_CTRL|ELW_SHIFT;//flags we'll use for the quickbar relocation handling
	int right_click = flags & ELW_RIGHT_MOUSE;
	int ctrl_on = flags & ELW_CTRL;
	int shift_on = flags & ELW_SHIFT;

	// only handle mouse button clicks, not scroll wheels moves or clicks
	if (( (flags & ELW_MOUSE_BUTTON) == 0) || ( (flags & ELW_MID_MOUSE) != 0)) return 0;

	if(right_click) {
		switch(qb_action_mode) {
		case ACTION_WALK:
			qb_action_mode=ACTION_LOOK;
			break;
		case ACTION_LOOK:
			qb_action_mode=ACTION_USE;
			break;
		case ACTION_USE:
			qb_action_mode=ACTION_USE_WITEM;
			break;
		case ACTION_USE_WITEM:
			if(use_item!=-1)
				use_item=-1;
			else
				qb_action_mode=ACTION_WALK;
			break;
		default:
			use_item=-1;
			qb_action_mode=ACTION_WALK;
		}
		if (cm_quickbar_enabled)
			cm_show_direct(cm_quickbar_id, quickbar_win, -1);
		return 1;
	}
	
	if(qb_action_mode==ACTION_USE_WITEM)	action_mode=ACTION_USE_WITEM;

	// no in window check needed, already done
	//see if we clicked on any item in the main category
	for(y=0;y<num_quickbar_slots;y++)
		{
			if(quickbar_dir==VERTICAL)
				{
					x_screen=0;
					y_screen=y*DEF_QUICKBAR_Y_LEN;
				}
			else
				{
					x_screen=y*DEF_QUICKBAR_Y_LEN;
					y_screen=0;
				}
			if(mx>x_screen && mx<x_screen+DEF_QUICKBAR_Y_LEN && my>y_screen && my<y_screen+DEF_QUICKBAR_Y_LEN)
				{
					//see if there is an empty space to drop this item over.
					if(item_dragged!=-1)//we have to drop this item
						{
							int any_item=0;
						        if(item_dragged == y) 
						        {		        
							
								 //let's try auto equip
								 int i;
								 for(i = ITEM_WEAR_START; i<ITEM_WEAR_START+8;i++)
								 {
								       if(item_list[i].quantity<1)
								       {
								              move_item(y,i);
								              break;
								       }								     
								  }								
							     
						                  item_dragged = -1;
						                  return 1;
						        }
						   	for(i=0;i<num_quickbar_slots;i++)
								{
									if(item_list[i].quantity && item_list[i].pos==y)
										{
											any_item=1;
											if(item_dragged==i)//drop the item only over itself
												item_dragged=-1;
											do_drop_item_sound();
											return 1;
										}
								}
							if(!any_item)
								{
									//send the drop info to the server
									str[0]=MOVE_INVENTORY_ITEM;
									str[1]=item_list[item_dragged].pos;
									str[2]=y;
									my_tcp_send(my_socket,str,3);
									item_dragged=-1;
									do_drag_item_sound();
									return 1;
								}
						}
					if(quickbar_relocatable>0)
						{
							if((flags&trigger)==(ELW_LEFT_MOUSE|ELW_CTRL))
							{
								//toggle draggable
								toggle_quickbar_draggable();
							}
							else if ( (flags & trigger)== (ELW_LEFT_MOUSE | ELW_SHIFT) && (get_flags (quickbar_win) & (ELW_TITLE_BAR | ELW_DRAGGABLE)) == (ELW_TITLE_BAR | ELW_DRAGGABLE) )
							{
								//toggle vertical/horisontal
								flip_quickbar();
							}
							else if (((flags&trigger)==trigger))
								{
									//reset
									reset_quickbar();
								}
						}
					//see if there is any item there
					for(i=0;i<num_quickbar_slots;i++)
						{
							//should we get the info for it?
							if(item_list[i].quantity && item_list[i].pos==y)
								{

									if(ctrl_on){
										str[0]=DROP_ITEM;
										str[1]=item_list[i].pos;
										*((Uint32 *)(str+2))=item_list[i].quantity;
										my_tcp_send(my_socket, str, 4);
										do_drop_item_sound();
										return 1;
									} else if(qb_action_mode==ACTION_LOOK)
										{
											click_time=cur_time;
											str[0]=LOOK_AT_INVENTORY_ITEM;
											str[1]=item_list[i].pos;
											my_tcp_send(my_socket,str,2);
										}
									else if(qb_action_mode==ACTION_USE)
										{
											if(item_list[i].use_with_inventory)
												{
													str[0]=USE_INVENTORY_ITEM;
													str[1]=item_list[i].pos;
													my_tcp_send(my_socket,str,2);
#ifdef NEW_SOUND
													item_list[i].action = USE_INVENTORY_ITEM;
#endif // NEW_SOUND
													return 1;
												}
											return 1;
										}
									else if(qb_action_mode==ACTION_USE_WITEM) {
										if(use_item!=-1) {
											str[0]=ITEM_ON_ITEM;
											str[1]=item_list[use_item].pos;
											str[2]=item_list[i].pos;
											my_tcp_send(my_socket,str,3);
#ifdef NEW_SOUND
											item_list[use_item].action = ITEM_ON_ITEM;
											item_list[i].action = ITEM_ON_ITEM;
#endif // NEW_SOUND
											if (!shift_on)
												use_item=-1;
										}
										else
											use_item=i;
										return 1;
									}
									else//we might test for other things first, like use or drop
										{
											if(item_dragged==-1)//we have to drag this item
												{
													item_dragged=i;
													do_drag_item_sound();
												}
										}

									return 1;
								}
						}
				}
		}
	return 1;
}

/*Enable/disable quickbar title bar and dragability*/
void toggle_quickbar_draggable()
{
	Uint32 flags = get_flags(quickbar_win);
	if (!quickbar_draggable)
	{
		flags &= ~ELW_SHOW_LAST;
		flags |= ELW_DRAGGABLE | ELW_TITLE_BAR;
		change_flags (quickbar_win, flags);
		quickbar_draggable = 1;
	}
	else 
	{
		flags |= ELW_SHOW_LAST;
		flags &= ~(ELW_DRAGGABLE | ELW_TITLE_BAR);
		change_flags (quickbar_win, flags);
		quickbar_draggable = 0;
		quickbar_x = window_width - windows_list.window[quickbar_win].cur_x;
		quickbar_y = windows_list.window[quickbar_win].cur_y;
	}
}

/*Change the quickbar from vertical to horizontal, or vice versa*/
void flip_quickbar() 
{
	if (quickbar_dir==VERTICAL) 
		{
			quickbar_dir=HORIZONTAL;
			init_window(quickbar_win, -1, 0, windows_list.window[quickbar_win].cur_x, windows_list.window[quickbar_win].cur_y, get_quickbar_y_len(), quickbar_x_len);
		}      
	else if (quickbar_dir==HORIZONTAL) 
		{
			quickbar_dir=VERTICAL;
			init_window(quickbar_win, -1, 0, windows_list.window[quickbar_win].cur_x, windows_list.window[quickbar_win].cur_y, quickbar_x_len, get_quickbar_y_len());
		}
}


/*Return the quickbar to it's Built-in position*/
void reset_quickbar() 
{
	//Necessary Variables
	quickbar_x_len= DEF_QUICKBAR_X_LEN;
	quickbar_x= DEF_QUICKBAR_X;
	quickbar_y= DEF_QUICKBAR_Y;
	//Re-set to default orientation
	quickbar_dir=VERTICAL;
	quickbar_draggable=0;
	init_window(quickbar_win, -1, 0, quickbar_x, quickbar_y, quickbar_x_len, get_quickbar_y_len());
	//Re-set  Flags
	change_flags(quickbar_win, ELW_TITLE_NONE|ELW_SHOW|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW_LAST);   
	//NEED x_offset
	move_window(quickbar_win, -1, 0, window_width-quickbar_x, 64);
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

void draw_exp_display()
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
			int name_y = exp_bar_start_y+10;
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

			name_x = my_exp_bar_start_x + stats_bar_len - strlen((char *)name) * SMALL_FONT_X_LEN;
			// the the name would overlap with the icons...
			if (name_x < icon_x)
			{
				// if there is space just use it
				int need = icon_x - name_x;
				if (need < free_statbar_space)
				{
					name_x += need;
					my_exp_bar_start_x += need;
				}
				// otherwise move the name onto the bar and use short form
				else
				{
					name = statsinfo[watch_this_stats[i]-1].skillnames->shortname;
					name_x = my_exp_bar_start_x + stats_bar_len - strlen((char *)name) * SMALL_FONT_X_LEN - 3;
					name_y = -3;
				}
			}

			draw_stats_bar(my_exp_bar_start_x, exp_bar_start_y, nl_exp - cur_exp, exp_adjusted_x_len, 0.1f, 0.8f, 0.1f, 0.1f, 0.4f, 0.1f);
			draw_string_small_shadowed(name_x, name_y, name, 1,1.0f,1.0f,1.0f,0.0f,0.0f,0.0f);

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

/*Change flags*/
void change_flags(int win_id, Uint32 flags)
{
	int order = windows_list.window[win_id].order;
	
	windows_list.window[win_id].flags = flags;
	if ( (order > 0 && (flags & ELW_SHOW_LAST)) || (order < 0 && !(flags & ELW_SHOW_LAST)) )
		windows_list.window[win_id].order = -order;
}

/*Return flags*/
Uint32 get_flags(int win_id) {
	return windows_list.window[win_id].flags;
}

