#ifndef __MISSILES_H__
#define __MISSILES_H__

#include "actors.h"
#include "cal3d_wrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_MISSILES 1024
#define MAX_MISSILES_DEFS 16

typedef enum {
	MISSED_SHOT = 0,
	NORMAL_SHOT = 1,
	CRITICAL_SHOT = 2
} MissileShotType;

typedef enum {
	REGULAR_MISSILE = 0,
	MAGIC_MISSILE = 1,
	FIRE_MISSILE = 2,
	ICE_MISSILE = 3,
	EXPLOSIVE_MISSILE = 4
} MissileEffectType;

/*!
 * \brief Structure that handle flying missiles
 */
typedef struct
{
	int type;                  /*!< The type of the missile: corresponds to the quiver type */
	MissileShotType shot_type; /*!< Specifies the type of the shot (normal, missed...) */
	float position[3];         /*!< The position of the missile */
	float direction[3];        /*!< The direction of the missile */
	float speed;               /*!< The speed of the missile */
	float trace_length;        /*!< The length of the trace let by the missile */
	float covered_distance;    /*!< The distance covered by the missile */
	float remaining_distance;  /*!< The remaining distance to cover */
} missile;

typedef struct
{
	char lost_mesh[MAX_FILE_PATH]; /*!< The name of the mesh used for lost missiles */
	float length;       /*!< Length of the missile mesh: used to know the position from where to launch the missile */
	float trace_length; /*!< Length of the trace that the missile leaves behind it */
	float speed;        /*!< Speed of the missile */
	MissileEffectType effect; /*!< Special effect to use for the missile */
} missile_type;

extern int missiles_count;
extern missile missiles_list[MAX_MISSILES];
extern missile_type missiles_defs[MAX_MISSILES_DEFS];

#ifdef DEBUG
extern int enable_client_aiming;
#endif // DEBUG

#ifdef MISSILES_DEBUG
/*!
 * \brief Log a message in the file config_dir/missiles_log.txt
 * \param format the format of the message to log (same options than printf)
 * \param ... data corresponding to the format
 */
void missiles_log_message_func(const char *format, ...);

#define missiles_log_message(format, ...) (missiles_log_message_func("%s: %d: " format, __FUNCTION__, __LINE__, __VA_ARGS__))
#else // MISSILES_DEBUG
#define missiles_log_message(format, ...)
#endif // MISSILES_DEBUG

static __inline__ missile *get_missile_ptr_from_id(int id)
{
	return ((id >= 0 || id < missiles_count) ? &missiles_list[id] : NULL);
}

/*!
 * \brief Removes all the missiles
 */
void missiles_clear();

/*!
 * \brief Adds a new missile
 * \param type the type of the missile
 * \param origin the origin of the missile
 * \param shift allows to tune if the missile should stop before or after the target
 * \param shot_type tells if the shot is missed, normal or critical (will be drawn diferently)
 */
int missiles_add(int type,
				 float origin[3],
				 float target[3],
				 float shift,
				 MissileShotType shot_type);

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
 * \param shot_type the type of the shot (normal, missed, critical)
 */
int missiles_fire_arrow(actor *a, float target[3], MissileShotType shot_type);

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
 * \brief Cleans the range action queue of an actor from the finished actions
 * \param act the actor
 */
void missiles_clean_range_actions_queue(actor *act);

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

/*!
 * \brief Initializes the missiles definitions
 */
void missiles_init_defs();

/*!
 * Display ranging win
 */ 
void display_range_win();
/*!
 * Ranging win ID
 */
extern int range_win;

extern int range_total_shots;
extern int range_success_hits;
extern int range_critical_hits;

extern int ranging_win_x;
extern int ranging_win_y;
 
#ifdef __cplusplus
} // extern "C"
#endif

#endif // __MISSILES_H__
