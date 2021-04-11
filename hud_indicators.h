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
void set_settings_hud_indicators(unsigned int opts, unsigned int pos);
void get_settings_hud_indicators(unsigned int *opts, unsigned int *pos);
#ifdef JSON_FILES
void write_settings_hud_indicators(const char *dict_name);
void read_settings_hud_indicators(const char *dict_name);
#endif
int get_hud_indicators_default_width(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif	//__HUD_INDICATORS_H
