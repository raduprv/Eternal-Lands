#ifndef __CAL_H__
#define __CAL_H__
#include "cal3d_wrapper.h"
#include "cal_types.h"

enum Boolean CalMixer_ExecuteAction_Stop(struct CalMixer *self, int id, float delayIn, float delayOut);
struct CalMesh *CalModel_GetAttachedMesh(struct CalModel *self,int i);
void CalCoreSkeleton_Scale(struct CalCoreSkeleton *self,float factor);
void CalMixer_RemoveAction(struct CalMixer *self,int id);
void CalCoreAnimation_Scale(struct CalCoreAnimation *self, float factor);
void CalCoreMesh_Scale(struct CalCoreMesh *self,float factor);

void cal_render_actor(actor *act);
#ifdef	NEW_ACTOR_ANIMATION
	#ifdef NEW_SOUND
struct cal_anim cal_load_anim(actor_types *act, char *str, char *sound, int duration);
	#else
struct cal_anim cal_load_anim(actor_types *act, char *str, int duration);
	#endif	//NEW_SOUND
#else
	#ifdef NEW_SOUND
struct cal_anim cal_load_anim(actor_types *act, char *str, char *sound);
	#else
struct cal_anim cal_load_anim(actor_types *act, char *str);
	#endif	//MEW_SOUND
#endif

void cal_actor_set_anim(int id,struct cal_anim anim);
#endif
