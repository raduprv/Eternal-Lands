#ifndef	__HUD_TIMER_H
#define	__HUD_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

void update_hud_timer(void);
int get_height_of_timer(void);
void set_mouse_over_timer(void);
int display_timer(window_info *win, int base_y_start);
int mouse_is_over_timer(window_info *win, int mx, int my);
int mouse_click_timer(Uint32 flags);
void destroy_timer(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif	//__HUD_TIMER_H
