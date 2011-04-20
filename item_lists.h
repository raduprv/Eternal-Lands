#ifndef ITEM_LIST_H
#define ITEM_LIST_H

#include "context_menu.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern Uint32 il_pickup_fail_time;

void toggle_items_list_window(window_info *win);
void update_category_maps(int image_id, Uint16 item_id, int cat_id);
void save_item_lists(void);

#ifdef __cplusplus
}
#endif


#endif
