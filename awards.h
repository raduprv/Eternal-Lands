#ifndef __SHOW_AWARDS_H__
#define __SHOW_AWARDS_H__

#ifdef AWARDS

#include "actors.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AWARD_32BIT_WORDS 5

void requested_awards_for_player(actor *player);
void here_is_awards_data(Uint32 *data);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // AWARDS

#endif // __SHOW_AWARDS_H__
