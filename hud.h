#ifndef	__HUD_H
#define	__HUD_H

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
extern int	icons_win;

//stats/health section
void init_stats_display();
void draw_stats_display();
int check_stats_display();
void draw_exp_display();
void build_levels_table();
extern int	stats_bar_win;

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

extern int hud_x;
extern int hud_y;
extern int hud_y;
extern int map_icon_x_start;
extern int map_icon_y_start;
extern int map_icon_x_end;
extern int map_icon_y_end;
extern int view_digital_clock;
void build_levels_table();

#endif	//__HUD_H
