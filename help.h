#ifndef __HELP_H__
#define __HELP_H__
extern int help_win;

/*int help_menu_x=150;
int help_menu_y=70;
int help_menu_x_len=150;
int help_menu_y_len=200;
*/
int display_help_handler(window_info *win);
int click_help_handler(window_info *win, int mx, int my, Uint32 flags);
void display_help();

#endif
