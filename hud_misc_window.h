#ifndef	__HUD_MISC_WINDOW_H
#define	__HUD_MISC_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

extern int misc_win;
extern int view_analog_clock;
extern int view_digital_clock;
extern int view_knowledge_bar;
extern int view_hud_timer;
extern int show_stats_in_hud;
extern int show_statbars_in_hud;
extern int copy_next_LOCATE_ME;

void init_misc_display(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif	//__HUD_MISC_WINDOW_H
