#ifndef _MINIMAP_H_
#define _MINIMAP_H_


#include "elwindows.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int minimap_win;
extern int minimap_win_x;
extern int minimap_win_y;
extern float minimap_tiles_distance;
extern int rotate_minimap;
extern int pin_minimap;
extern int open_minimap_on_start;
extern float minimap_size_coefficient;

void display_minimap();
int display_minimap_handler(window_info *win);

//called when map changes or window size changes (we have to reload textures)
void change_minimap();


void save_exploration_map();

#ifdef __cplusplus
} // extern "C"
#endif


#endif // _MINIMAP_H_

