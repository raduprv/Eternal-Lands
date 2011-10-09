#ifndef __O3DOW_H__
#define __O3DOW_H__

extern int o3dow_x;
extern int o3dow_y;
extern int o3dow_x_len;
extern int o3dow_y_len;
extern int o3dow_dragged;
extern int view_o3dow;
extern int o3dow_win;
extern int c1,c2,c3,c4;
extern int minay,minaz,maxay,maxaz,minax,maxax,minh,maxh,randomheight, randomanglex, randomangley, randomanglez;

void init_o3dow();
void display_o3dow();
int display_o3dow_handler();
int check_o3dow_interface();

#endif

