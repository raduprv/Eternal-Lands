#ifndef __CAL_H__
#define __CAL_H__

#include "actors.h"
#include "cal3d_wrapper.h"
#include "cal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

enum CalBoolean CalMixer_ExecuteAction_Stop(struct CalMixer *self, int id, float delayIn, float delayOut);
struct CalMesh *CalModel_GetAttachedMesh(struct CalModel *self,int i);
void CalCoreSkeleton_Scale(struct CalCoreSkeleton *self,float factor);
void CalMixer_RemoveAction(struct CalMixer *self,int id);
void CalCoreAnimation_Scale(struct CalCoreAnimation *self, float factor);
void CalCoreMesh_Scale(struct CalCoreMesh *self,float factor);
#ifdef NEW_SOUND
void cal_set_anim_sound(struct cal_anim *my_cal_anim, const char *sound, const char *sound_scale);
#endif // NEW_SOUND

void cal_render_actor(actor *act);
#ifdef	NEW_ACTOR_ANIMATION
	#ifdef NEW_SOUND
struct cal_anim cal_load_anim(actor_types *act, const char *str, const char *sound, const char *sound_scale, int duration);
	#else
struct cal_anim cal_load_anim(actor_types *act, const char *str, int duration);
	#endif	//NEW_SOUND
#else
	#ifdef NEW_SOUND
struct cal_anim cal_load_anim(actor_types *act, const char *str, const char *sound, const char *sound_scale);
	#else
struct cal_anim cal_load_anim(actor_types *act, const char *str);
	#endif	//NEW_SOUND
#endif

#define cal_cycle_blending_delay  0.1f	/*!< time in seconds for blending from cycle to action or cycle. */
#define cal_action_blending_delay 0.6f	/*!< time in seconds for blending from action to action or cycle. */

void cal_actor_set_anim_delay(int id, struct cal_anim anim, float delay);
void cal_actor_set_anim(int id, struct cal_anim anim);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
