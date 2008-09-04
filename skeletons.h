#ifndef __SKELETON_H__
#define __SKELETON_H__

#include "actors.h"
#include "cal3d_wrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief The main used bones
 */
typedef enum {
	head_bone = 0,
	mouth_bone = 1,
	body_top_bone = 2,
	body_bottom_bone = 3,
	cape_top_bone = 4,
	cape_middle_bone = 5,
	cape_bottom_bone = 6,
	weapon_left_bone = 7,
	weapon_right_bone = 8,
	staff_right_bone = 9,
	arrow_bone = 10,
	hand_left_bone = 11,
	hand_right_bone = 12,
	highest_bone = 13
} cal_bone_name;

#define MAX_MAIN_CAL_BONES 14

/*!
 * \brief Structure that holds the IDs of the main bones for a skeleton
 */
typedef struct {
	char name[MAX_FILE_PATH]; /*!< The name of the skeleton */
	int cal_bones_id[MAX_MAIN_CAL_BONES]; /*!< The main bones IDs by their names */
} skeleton_types;

#define MAX_SKELETONS 20

/*!
 * \brief Array that contains all the main bones definitions for each existing skeleton
 */
extern skeleton_types skeletons_defs[MAX_SKELETONS];
extern int skeletons_count;

/*!
 * \brief Get the ID of an actor bone by its name in a core skeleton
 * \param skel the core skeleton
 * \param name the name of the bone
 * \return the ID of the bone if it exists, else -1
 */
int find_core_bone_id(struct CalCoreSkeleton *skel, const char *name);

/*!
 * \brief Get the ID of a skeleton by its name
 * \param cal_model the cal model that contains the cal skeleton
 * \param skeleton_name the name of the skeleton
 *
 * This function checks in the skeletons_defs array if the skeleton already
 * exists and returns the corresponding ID if true.
 * Otherwise, it adds a new skeleton to the array and setup the IDs for the
 * different bones names according to the name of the skeleton.
 */
int get_skeleton(struct CalCoreModel *cal_model, const char *skeleton_name);

/*!
 * \brief Get the ID of an actor bone by its name
 * \param act the actor
 * \param name the predefined name of the bone
 * \return the ID of the bone if it exists, else -1
 *
 * When accessing to several bones IDs in the same function for the
 * same actor, prefer using directly the data structure to get a pointer
 * on the corresponding skeleton_types structure.
 */
int get_actor_bone_id(actor *act, cal_bone_name name);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __SKELETON_H__
