#ifndef ITEM_LIST_H
#define ITEM_LIST_H

#include "context_menu.h"

#ifdef __cplusplus
extern "C"
{
#endif

void toggle_items_list_window(window_info *win);
void update_category_maps(int image_id, Uint16 item_id, int cat_id);
void save_item_lists(void);
unsigned int item_lists_get_active(void);
void item_lists_set_active(unsigned int active_list);
void item_lists_reset_pickup_fail_time(void);

extern int items_list_disable_find_list;

#ifdef __cplusplus
}
#endif


#endif
