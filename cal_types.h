#ifndef __CAL_TYPES_H__
#define __CAL_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
	cycle=0,
 	action=1
}  cal_animation_type;

struct cal_anim
{
	int anim_index;
	cal_animation_type kind;
	float duration;
	float duration_scale;
#ifdef NEW_SOUND
	int sound;
	float sound_scale;
#endif	//NEW_SOUND
};

#ifdef __cplusplus
} // extern "C"
#endif

#endif
