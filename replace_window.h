#ifndef __REPLACE_WINDOW_H__
#define __REPLACE_WINDOW_H__

extern int replace_window_x;
extern int replace_window_y;
extern int replace_window_x_len;
extern int replace_window_y_len;
extern int view_replace_window;
extern int replace_window_win;

void init_replace_window();
void display_replace_window();
int display_replace_window_handler();
int check_replace_window_interface();

#endif

