#ifndef __EDIT_WINDOW_H__
#define __EDIT_WINDOW_H__

extern int edit_window_x;
extern int edit_window_y;
extern int edit_window_x_len;
extern int edit_window_y_len;
extern int view_edit_window;
int ew_selected_object;

void init_edit_window();
void display_edit_window();
int display_edit_window_handler();
int check_edit_window_interface();

#endif

