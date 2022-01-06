#ifndef __SHOW_ACHIEVEMENTS_H__
#define __SHOW_ACHIEVEMENTS_H__


#include "actors.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ACHIEVEMENTS 1024

extern int achievements_ctrl_click;

void achievements_player_name(const char *name, int len);
void achievements_data(Uint32 *data, size_t word_count);
void achievements_requested(int mouse_pos_x, int mouse_pos_y, int control_used);

#ifdef __cplusplus
} // extern "C"
#endif


#endif // __SHOW_ACHIEVEMENTS_H__
