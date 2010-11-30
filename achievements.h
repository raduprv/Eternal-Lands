#ifndef __SHOW_ACHIEVEMENTS_H__
#define __SHOW_ACHIEVEMENTS_H__

#ifdef ACHIEVEMENTS

#include "actors.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ACHIEVEMENTS 1024

void requested_achievements_for_player(const char *name, int len);
void here_is_achievements_data(Uint32 *data, size_t word_count);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ACHIEVEMENTS

#endif // __SHOW_ACHIEVEMENTS_H__
