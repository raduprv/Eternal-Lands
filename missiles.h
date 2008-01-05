#ifdef MISSILES

#ifndef __MISSILES_H__
#define __MISSILES_H__

#include "actors.h"
#include "cal3d_wrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {MISSILE_ARROW} MissileType;

/*!
 * \brief Structure that handle flying missiles
 */
typedef struct
{
	MissileType type;   /*!< The type of the missile */
	float position[3];  /*!< The position of the missile */
	float direction[3]; /*!< The direction of the missile */
	float speed;        /*!< The speed of the missile */
	float covered_distance;   /*!< The distance covered by the missile */
	float remaining_distance; /*!< The remaining distance to cover */
} Missile;

void missiles_log_message(const char *format, ...);

/*!
 * \brief Removes all the missiles
 */
void clear_missiles();

/*!
 * \brief Adds a new missile
 * \param type the type of the missile
 * \param origin the origin of the missile
 * \param target the target of the missile
 * \param speed the speed of the missile
 * \param shift allows to tune if the missile should stop before or after the target
 */
unsigned int add_missile(MissileType type,
						 float origin[3],
						 float target[3],
						 float speed,
						 float shift);

/*!
 * \brief Computes the next position for all missiles
 */
void update_missiles();

/*!
 * \brief Draws all the missiles
 */
void draw_missiles();

/*!
 * \brief Adds a new arrow (calls add_missile)
 * \param a the actor throwing the arrow
 * \param target the target
 */
unsigned int fire_arrow(actor *a, float target[3]);

/*!
 * \brief Computes the rotation matrix of an actor
 * \param in_act the actor
 * \param out_rot the resulting matrix (3x3 matrix: 9 floats)
 */
void get_actor_rotation_matrix(actor *in_act, float *out_rot);

/*!
 * \brief Gets the local position of char bone
 * \param in_act the actor
 * \param in_bone_id the bone
 * \param in_shift a shift according to the orientation of the bone (can be NULL)
 * \param out_pos the resulting position
 */
void get_actor_bone_local_position(actor *in_act, int in_bone_id, float *in_shift, float *out_pos);

/*!
 * \brief Transforms a local position on a char to an absolute position
 * \param in_act the actor
 * \param in_local_pos the local position
 * \param in_act_rot the rotation matrix of the actor (computed inside if NULL)
 * \param out_pos the resulting position
 */
void transform_actor_local_position_to_absolute(actor *in_act, float *in_local_pos, float *in_act_rot, float *out_pos);

/*!
 * \brief Gets the absolute position of char bone
 * \param in_act the actor
 * \param in_bone_id the bone
 * \param in_shift a shift according to the orientation of the bone (can be NULL)
 * \param out_pos the resulting position
 */
void get_actor_bone_absolute_position(actor *in_act, int in_bone_id, float *in_shift, float *out_pos);

/*!
 * \brief Computes the rotations to apply to a char when aiming something
 * \param out_q the quaternion corresponding to a rotation between -22.5 and +22.5 degrees.
 *              This quaternion has to be applied to bones of the char in order to have a fine orientation.
 * \param in_act the actor
 * \param in_target the target
 * \return the rotation to apply to the whole char to face the target
 */
float compute_actor_rotation(struct CalQuaternion *out_q, actor *in_act, float *in_target);

/*!
 * \brief Rotates bones of an actor according to what is defined in its structure
 * \param a the actor
 */
void rotate_actor_bones(actor *a);

/*!
 * \brief Tells an actor to aim at another actor
 * \param actor1_id the ID of the current actor
 * \param actor2_id the ID of the other actor
 */
void actor_aim_at_b(int actor1_id, int actor2_id);

/*!
 * \brief Tells an actor to aim at a target
 * \param actor_id the ID of the actor
 * \param target the target
 */
void actor_aim_at_xyz(int actor_id, float *target);

/*!
 * \brief Tells an actor to fire an arrow on another actor
 * \param actor1_id the ID of the current actor
 * \param actor2_id the ID of the other actor
 */
void missile_fire_a_to_b(int actor1_id, int actor2_id);

/*!
 * \brief Tells an actor to fire an arrow on a target
 * \param actor_id the ID of the actor
 * \param target the target
 */
void missile_fire_a_to_xyz(int actor_id, float *target);

/*!
 * \brief Fires a missile from a position to an actor
 * \param origin the origin
 * \param actor_id the actor
 */
void missile_fire_xyz_to_b(float *origin, int actor_id);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __MISSILES_H__

#endif // MISSILES
