#ifndef __CAL_TYPES_H__
#define __CAL_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NEW_SOUND
#include "sound.h"
#endif	//NEW_SOUND

typedef enum{
	cycle=0,
 	action=1
}  cal_animation_type;

struct cal_anim
{
	int anim_index;
	cal_animation_type kind;
	float duration;
#ifdef	NEW_ACTOR_ANIMATION
	float duration_scale;
#endif
#ifdef NEW_SOUND
	char sound[MAX_SOUND_NAME_LENGTH];
#endif	//NEW_SOUND
};

#ifdef __cplusplus
} // extern "C"
#endif

#endif
