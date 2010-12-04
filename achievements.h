#ifndef __SHOW_ACHIEVEMENTS_H__
#define __SHOW_ACHIEVEMENTS_H__

#ifdef ACHIEVEMENTS

#include "actors.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ACHIEVEMENTS 1024

void achievements_player_name(const char *name, int len);
void achievements_data(Uint32 *data, size_t word_count);
void achievements_mouse_pos(int mouse_pos_x, int mouse_pos_y);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ACHIEVEMENTS

#endif // __SHOW_ACHIEVEMENTS_H__
