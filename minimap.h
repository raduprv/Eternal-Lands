#ifndef _MINIMAP_H_
#define _MINIMAP_H_

#ifdef MINIMAP

#ifdef __cplusplus
extern "C" {
#endif

extern int minimap_win;
extern int minimap_win_x;
extern int minimap_win_y;
extern int minimap_win_pin;
extern int minimap_win_above;
extern int minimap_zoom;

void display_minimap();
int display_minimap_handler(window_info *win);

//called when map changes
void change_minimap();

//called when player moves
void update_exploration_map();

void save_exploration_map();

#ifdef __cplusplus
} // extern "C"
#endif

#endif //MINIMAP

#endif // _MINIMAP_H_

