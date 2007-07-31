#ifndef _SPECIAL_EFFECTS_H_
#define _SPECIAL_EFFECTS_H_

#include <SDL_types.h>
#include "client_serv.h"

#ifdef __cplusplus
extern "C" {
#endif

void display_special_effects(int do_render);
void parse_special_effect(special_effect_enum sfx, const Uint16 *data);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _SPECIAL_EFFECTS_H_
