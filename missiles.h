#ifdef MISSILES

#ifndef __MISSILES_H__
#define __MISSILES_H__

#include "actors.h"
#include "cal3d_wrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {MISSILE_ARROW} MissileType;
typedef enum {MISSED_HIT, NORMAL_HIT, CRITICAL_HIT} MissileHitType;
typedef enum {RANGE_WEAPON_BOW, RANGE_WEAPON_CROSSBOW} RangeWeaponType;

/*!
 * \brief Structure that handle flying missiles
 */
typedef struct
{
	MissileType type;         /*!< The type of the missile */
	MissileHitType hit_type;  /*!< Specifies the type of the hit (normal, missed...) */
	float position[3];        /*!< The position of the missile */
	float direction[3];       /*!< The direction of the missile */
	float speed;              /*!< The speed of the missile */
	float trace_length;       /*!< The length of the trace let by the missile */
	float covered_distance;   /*!< The distance covered by the missile */
	float remaining_distance; /*!< The remaining distance to cover */
} Missile;

#ifdef DEBUG
/*!
 * \brief Log a message in the file config_dir/missiles_log.txt
 * \param format the format of the message to log (same options than printf)
 * \param ... data corresponding to the format
 */
void missiles_log_message(const char *format, ...);
#else // DEBUG
#define missiles_log_message(format, ...)
#endif // DEBUG

/*!
 * \brief Removes all the missiles
 */
void missiles_clear();

/*!
 * \brief Adds a new missile
 * \param type the type of the missile
 * \param origin the origin of the missile
 * \param target the target of the missile
 * \param speed the speed of the missile
 * \param shift allows to tune if the missile should stop before or after the target
 * \param miss_target tells if the missile will miss its target (will be drawn diferently)
 */
unsigned int missiles_add(MissileType type,
						  float origin[3],
						  float target[3],
						  float speed,
						  float trace_length,
						  float shift,
						  MissileHitType hit_type);

/*!
 * \brief Computes the next position for all missiles
 */
void missiles_update();

/*!
 * \brief Draws all the missiles
 */
void missiles_draw();

/*!
 * \brief Adds a new arrow (calls add_missile)
 * \param a the actor throwing the arrow
 * \param target the target
 * \param miss_target tells if the target is missed
 */
unsigned int missiles_fire_arrow(actor *a, float target[3], MissileHitType hit_type);

/*!
 * \brief Computes the rotations to apply to a char when aiming something
 * \param out_h_rot the returned horizontal rotation
 * \param out_v_rot the returned vertical rotation
 * \param in_act the actor
 * \param in_target the target
 * \return the rotation to apply to the whole char to face the target
 *
 * The rotations stored in out_h_rot and out_v_rot have to be applied to bones of the
 * char in order to have a fine orientation. The horizontal orientation should stay
 * between -22.5 and +22.5 degrees.
 */
float missiles_compute_actor_rotation(float *out_h_rot, float *out_v_rot,
									  actor *in_act, float *in_target);

/*!
 * \brief Rotates bones of an actor according to what is defined in its structure
 * \param a the actor
 */
void missiles_rotate_actor_bones(actor *a);

/*!
 * \brief Tells an actor to aim at another actor
 * \param actor1_id the ID of the current actor
 * \param actor2_id the ID of the other actor
 */
void missiles_aim_at_b(int actor1_id, int actor2_id);

/*!
 * \brief Tells an actor to aim at a target
 * \param actor_id the ID of the actor
 * \param target the target
 */
void missiles_aim_at_xyz(int actor_id, float *target);

/*!
 * \brief Tells an actor to fire an arrow on another actor
 * \param actor1_id the ID of the current actor
 * \param actor2_id the ID of the other actor
 */
void missiles_fire_a_to_b(int actor1_id, int actor2_id);

/*!
 * \brief Tells an actor to fire an arrow on a target
 * \param actor_id the ID of the actor
 * \param target the target
 */
void missiles_fire_a_to_xyz(int actor_id, float *target);

/*!
 * \brief Fires a missile from a position to an actor
 * \param origin the origin
 * \param actor_id the actor
 */
void missiles_fire_xyz_to_b(float *origin, int actor_id);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __MISSILES_H__

#endif // MISSILES
