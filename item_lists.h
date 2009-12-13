#ifndef ITEM_LIST_H
#define ITEM_LIST_H

#if defined(CONTEXT_MENUS) && defined(ITEM_LISTS)
#include "context_menu.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern int show_item_list_menu;
extern int disable_item_list_preview;
extern size_t cm_item_list_but;
extern size_t cm_item_list_options_but;

int cm_item_list_handler(window_info *win, int widget_id, int mx, int my, int option);
void cm_item_list_pre_show_handler(window_info *win, int widget_id, int mx, int my, window_info *cm_win);
int cm_item_list_options_handler(window_info *win, int widget_id, int mx, int my, int option);
void show_items_list_window(int is_delete);
void setup_item_list_menus(void);

#endif // ITEM_LISTS

#ifdef __cplusplus
}
#endif

#endif
