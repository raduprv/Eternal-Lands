#ifndef	__HUD_H
#define	__HUD_H

extern int view_clock;
extern int view_compass;

// the main hud handling
void init_hud_interface();
void draw_hud_interface();
int check_hud_interface();

// icons subsection
void init_peace_icons();
void draw_peace_icons();
int check_peace_icons();

//stats/health section
void init_stats_display();
void draw_stats_display();
int check_stats_display();

//misc section (compass, clock, ?)
void init_misc_display();
void draw_misc_display();
int check_misc_display();

#endif	//__HUD_H