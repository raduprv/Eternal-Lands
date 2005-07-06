#ifndef __CAL_H__
#define __CAL_H__
#include "cal3d_wrapper.h"
#include "cal_types.h"
void cal_render_actor(actor *act);
struct cal_anim cal_load_anim(actor_types *act, char *str);
void cal_actor_set_anim(int id,struct cal_anim anim);
#endif
