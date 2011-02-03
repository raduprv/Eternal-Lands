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
enum CalBoolean CalMixer_ExecuteActionExt(struct CalMixer *self, int id, float delayIn, float delayOut, float weight, int autoLock);
 

void cal_actor_set_emote_anim(actor *pActor, emote_frame *anims);
void handle_cur_emote(actor *pActor);
void cal_reset_emote_anims(actor *pActor, int cycles_too);
/*!
 * \brief	Draw a specific actor
 * 
 * 		Draw a specific actor, with optinal enabled lightning and textures.
 *
 * \param	act The actor to draw
 * \param	use_lightning Should lightning be used (normals, colors and material properties)
 * \param	use_textures Should default texturing be used
 * \param	use_glow Should glow be used
 *
 * \callgraph
 */
void cal_render_actor(actor *act, Uint32 use_lightning, Uint32 use_textures, Uint32 use_glow);
	#ifdef NEW_SOUND
struct cal_anim cal_load_anim(actor_types *act, const char *str, const char *sound, const char *sound_scale, int duration);
	#else
struct cal_anim cal_load_anim(actor_types *act, const char *str, int duration);
	#endif	//NEW_SOUND

#define cal_cycle_blending_delay  0.1f	/*!< time in seconds for blending from cycle to action or cycle. */
#define cal_action_blending_delay 0.6f	/*!< time in seconds for blending from action to action or cycle. */

void cal_actor_set_anim_delay(int id, struct cal_anim anim, float delay);
void cal_actor_set_anim(int id, struct cal_anim anim);

/*!
 * \brief Gets the local position of char bone
 * \param in_act the actor
 * \param in_bone_id the bone
 * \param in_shift a shift according to the orientation of the bone (can be NULL)
 * \param out_pos the resulting position
 */
void cal_get_actor_bone_local_position(actor *in_act, int in_bone_id, float *in_shift, float *out_pos);

/*!
 * \brief Gets the absolute position of char bone
 * \param in_act the actor
 * \param in_bone_id the bone
 * \param in_shift a shift according to the orientation of the bone (can be NULL)
 * \param out_pos the resulting position
 */
void cal_get_actor_bone_absolute_position(actor *in_act, int in_bone_id, float *in_shift, float *out_pos);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
