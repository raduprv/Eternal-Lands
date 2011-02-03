#ifndef ITEM_LIST_H
#define ITEM_LIST_H

#include "context_menu.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern int show_item_list_menu;
extern int disable_item_list_preview;
extern size_t cm_item_list_but;
extern size_t cm_item_list_options_but;
extern Uint32 il_pickup_fail_time;

int cm_item_list_handler(window_info *win, int widget_id, int mx, int my, int option);
void cm_item_list_pre_show_handler(window_info *win, int widget_id, int mx, int my, window_info *cm_win);
int cm_item_list_options_handler(window_info *win, int widget_id, int mx, int my, int option);
void show_items_list_window(int is_delete);
void setup_item_list_menus(void);
void update_category_maps(int image_id, Uint16 item_id, int cat_id);
void save_category_maps(void);

#ifdef __cplusplus
}
#endif


#endif
