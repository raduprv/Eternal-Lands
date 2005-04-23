#ifndef __STORAGE_H__
#define __STORAGE_H__

extern int storage_win;
extern int storage_item_dragged;
extern ground_item storage_items[200];

void get_storage_categories(char * in_data, int len);
void get_storage_items(Uint8 * in_data, int len);
void get_storage_text(Uint8 * in_data, int len);
void rendergrid(int columns, int rows, int left, int top, int width, int height);
int get_mouse_pos_in_grid(int mx, int my, int columns, int rows, int left, int top, int width, int height);
void display_storage_menu();

#endif
