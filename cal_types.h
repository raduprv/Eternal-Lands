#ifndef __CAL_TYPES_H__
#define __CAL_TYPES_H__

struct cal_anim
{
int anim_index;
int kind;
float duration;
#ifdef	NEW_ACTOR_ANIMATION
float duration_scale;
#endif
};



#endif
