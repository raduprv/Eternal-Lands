#ifndef	__HUD_MISC_WINDOW_H
#define	__HUD_MISC_WINDOW_H

#include "hud.h"

extern int misc_win;
extern int view_analog_clock;
extern int view_digital_clock;
extern int view_knowledge_bar;
extern int view_hud_timer;
extern int show_stats_in_hud;
extern int show_statbars_in_hud;

void init_misc_display(hud_interface type);

#endif	//__HUD_MISC_WINDOW_H
