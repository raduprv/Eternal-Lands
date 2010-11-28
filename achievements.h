#ifndef __SHOW_ACHIEVEMENTS_H__
#define __SHOW_ACHIEVEMENTS_H__

#ifdef ACHIEVEMENTS

#include "actors.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ACHIEVEMENT_32BIT_WORDS 5
#define NUM_ACHIEVEMENTS 160

void requested_achievements_for_player(actor *player);
void here_is_achievements_data(Uint32 *data);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ACHIEVEMENTS

#endif // __SHOW_ACHIEVEMENTS_H__
