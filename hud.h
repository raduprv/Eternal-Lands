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

//stats/health section
void init_stats_display();
void draw_stats_display();
int check_stats_display();

void draw_load_exp_display();

//misc section (compass, clock, ?)
void init_misc_display();
void draw_misc_display();
int check_misc_display();

//quickbar section
void init_quickbar();
void draw_quickbar();
int check_quickbar();

extern int hud_x;
extern int hud_y;

#endif	//__HUD_H
