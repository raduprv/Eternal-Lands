#ifndef __EDIT_WINDOW_H__
#define __EDIT_WINDOW_H__

#include "2d_objects.h"
#include "e3d.h"

extern int edit_window_x;
extern int edit_window_y;
extern int edit_window_x_len;
extern int edit_window_y_len;
extern int view_edit_window;
extern int edit_window_win;
extern int ew_selected_object;
extern int ew_object_type;
extern obj_2d o2t;
extern object3d o3t;

void init_edit_window();
void display_edit_window();
int display_edit_window_handler();
int check_edit_window_interface();

#endif

