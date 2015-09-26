#ifndef	__HUD_INDICATORS_H
#define	__HUD_INDICATORS_H

#ifdef __cplusplus
extern "C" {
#endif

extern int show_hud_indicators;
void init_hud_indicators(void);
void destroy_hud_indicators(void);
void show_hud_indicators_window(void);
void hide_hud_indicators_window(void);
void toggle_hud_indicators_window(int *show);

#ifdef __cplusplus
} // extern "C"
#endif

#endif	//__HUD_INDICATORS_H
