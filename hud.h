#ifndef	__HUD_H
#define	__HUD_H

#include "elwindows.h"

#define WALK 0
#define SIT 1
#define LOOK 2
#define TRADE 3
#define ATTACK 4
#define USE 5

#define DATA_NONE -1
#define DATA_WINDOW 0
#define DATA_ACTIONMODE 1

#define HORIZONTAL 2
#define VERTICAL 1

typedef struct
{
	int state;
	//Image position
	float u[2];
	float v[2];
	//Help message
	char * help_message;
	//Function pointer and data
	int (*func)(void*, int);
	void * data;
	char data_type;
	char free_data;
} icon_struct;

struct icons_struct
{
	int no;
	int y;
	int x;
	icon_struct *icon[25];
};

extern struct icons_struct icons;

//These aren't handled by the windowmanager - yet?
extern int map_win;
extern int console_win;

// the main hud handling
void init_hud_interface();
void draw_hud_interface();
int check_hud_interface();
void init_hud_frame();
void draw_hud_frame();

// icons subsection
void init_peace_icons();
void draw_peace_icons();
int check_peace_icons();
void add_icon(float u_start, float v_start, float colored_u_start, float colored_v_start, char * help_message, void * func, void * data, char data_type);
void reset_states(int id, int state);
int translate_win_id(int * win_id);
void free_icons();
extern int	icons_win;

//Functions for the function pointers
void switch_action_mode(int * mode, int id);
void sit_button_pressed(void *unused, int id);
void view_window(int * win, int id);
void view_console_win(int * win, int id);//This is not handled by the window manager, so we have to call this function
void view_map_win(int *win, int id);
void show_help(char *message, int x, int y);

//stats/health section
void init_stats_display();
void draw_stats_display();
int check_stats_display();
void draw_exp_display();
void build_levels_table();
extern int	stats_bar_win;
void draw_stats();

//misc section (compass, clock, ?)
void init_misc_display();
void draw_misc_display();
int check_misc_display();
extern int	misc_win;

//quickbar section
void init_quickbar();
void draw_quickbar();
int check_quickbar();
extern int	quickbar_win;
extern int 	quickbar_relocatable;
void flip_quickbar();
void reset_quickbar();
void change_flags(int win_id, Uint32 flags);
Uint32 get_flags(int win_id);

extern int hud_x;
extern int hud_y;
extern int hud_y;
extern int map_icon_x_start;
extern int map_icon_y_start;
extern int map_icon_x_end;
extern int map_icon_y_end;
extern int view_digital_clock;
void build_levels_table();

extern int quickbar_x;
extern int quickbar_y;
extern int quickbar_dir;
extern int quickbar_draggable;

#endif	//__HUD_H
