#include <assert.h>
#include "asc.h"
#include "cal3d_wrapper.h"
#include "errors.h"
#include "skeletons.h"

skeleton_types skeletons_defs[MAX_SKELETONS];
int skeletons_count = 0;

int find_core_bone_id(struct CalCoreSkeleton *skel, const char *name)
{
	int i = CalCoreSkeleton_GetCoreBonesNumber(skel);
	while (i--) {
		struct CalCoreBone *bone = CalCoreSkeleton_GetCoreBone(skel, i);
		if (!strcmp(CalCoreBone_GetName(bone), name))
			return i;
	}
	LOG_ERROR("no bone with name '%s' found in skeleton!\n", name);
	return -1;
}

int get_skeleton(struct CalCoreModel *cal_model, const char *skeleton_name)
{
	int i;
	int length = strlen(skeleton_name);
	skeleton_types *skel;
	struct CalCoreSkeleton *cal_skel = CalCoreModel_GetCoreSkeleton(cal_model);

	// looking for an existing skeleton
	for (i = 0; i < skeletons_count; ++i) {
		if (!strcmp(skeletons_defs[i].name, skeleton_name)) {
			return i;
		}
	}

	// if no skeleton found, we add a new one
	assert(skeletons_count < MAX_SKELETONS);
	skel = &skeletons_defs[skeletons_count++];
	safe_strncpy(skel->name, skeleton_name, MAX_FILE_PATH);

	for (i = MAX_MAIN_CAL_BONES; i--;)
		skel->cal_bones_id[i] = -1;

	if (!strcmp(&skeleton_name[length-8], "bear.csf")) {
		skel->cal_bones_id[head_bone] = find_core_bone_id(cal_skel, "head");
		skel->cal_bones_id[mouth_bone] = find_core_bone_id(cal_skel, "mouth");
		skel->cal_bones_id[body_top_bone] = find_core_bone_id(cal_skel, "neck1");
		skel->cal_bones_id[body_bottom_bone] = find_core_bone_id(cal_skel, "bodyF");
		skel->cal_bones_id[highest_bone] = skel->cal_bones_id[head_bone];
	}
	else if (!strcmp(&skeleton_name[length-8], "bird.csf")) {
		skel->cal_bones_id[head_bone] = find_core_bone_id(cal_skel, "head");
		skel->cal_bones_id[mouth_bone] = find_core_bone_id(cal_skel, "mouth");
		skel->cal_bones_id[body_top_bone] = find_core_bone_id(cal_skel, "body2");
		skel->cal_bones_id[body_bottom_bone] = find_core_bone_id(cal_skel, "body1");
		skel->cal_bones_id[highest_bone] = skel->cal_bones_id[body_top_bone];
	}
	else if (!strcmp(&skeleton_name[length-9], "bird2.csf")) {
		skel->cal_bones_id[head_bone] = find_core_bone_id(cal_skel, "head");
		skel->cal_bones_id[mouth_bone] = find_core_bone_id(cal_skel, "mouth");
		skel->cal_bones_id[body_top_bone] = find_core_bone_id(cal_skel, "body2");
		skel->cal_bones_id[body_bottom_bone] = find_core_bone_id(cal_skel, "body1");
		skel->cal_bones_id[highest_bone] = skel->cal_bones_id[head_bone];
	}
	else if (!strcmp(&skeleton_name[length-8], "boar.csf")) {
		skel->cal_bones_id[head_bone] = find_core_bone_id(cal_skel, "head");
		skel->cal_bones_id[mouth_bone] = find_core_bone_id(cal_skel, "mouth");
		skel->cal_bones_id[body_top_bone] = find_core_bone_id(cal_skel, "neck1");
		skel->cal_bones_id[body_bottom_bone] = find_core_bone_id(cal_skel, "bodyF");
		skel->cal_bones_id[highest_bone] = skel->cal_bones_id[body_top_bone];
	}
	else if (!strcmp(&skeleton_name[length-10], "canine.csf")) {
		skel->cal_bones_id[head_bone] = find_core_bone_id(cal_skel, "head");
		skel->cal_bones_id[mouth_bone] = find_core_bone_id(cal_skel, "mouth");
		skel->cal_bones_id[body_top_bone] = find_core_bone_id(cal_skel, "neck");
		skel->cal_bones_id[body_bottom_bone] = find_core_bone_id(cal_skel, "bodyF");
		skel->cal_bones_id[highest_bone] = skel->cal_bones_id[head_bone];
	}
	else if (!strcmp(&skeleton_name[length-13], "castellan.csf")) {
		skel->cal_bones_id[head_bone] = find_core_bone_id(cal_skel, "head");
		skel->cal_bones_id[mouth_bone] = skel->cal_bones_id[head_bone];
		skel->cal_bones_id[body_top_bone] = find_core_bone_id(cal_skel, "spine3");
		skel->cal_bones_id[body_bottom_bone] = find_core_bone_id(cal_skel, "spine1");
		skel->cal_bones_id[weapon_left_bone] = find_core_bone_id(cal_skel, "weaponL");
		skel->cal_bones_id[hand_left_bone] = find_core_bone_id(cal_skel, "handL");
		skel->cal_bones_id[hand_right_bone] = find_core_bone_id(cal_skel, "handR");
		skel->cal_bones_id[highest_bone] = skel->cal_bones_id[head_bone];
	}
	else if (!strcmp(&skeleton_name[length-8], "deer.csf")) {
		skel->cal_bones_id[head_bone] = find_core_bone_id(cal_skel, "head");
		skel->cal_bones_id[mouth_bone] = find_core_bone_id(cal_skel, "mouth");
		skel->cal_bones_id[body_top_bone] = find_core_bone_id(cal_skel, "neck1");
		skel->cal_bones_id[body_bottom_bone] = find_core_bone_id(cal_skel, "bodyF");
		skel->cal_bones_id[highest_bone] = skel->cal_bones_id[head_bone];
	}
	else if (!strcmp(&skeleton_name[length-10], "dragon.csf")) {
		skel->cal_bones_id[head_bone] = find_core_bone_id(cal_skel, "head");
		skel->cal_bones_id[mouth_bone] = find_core_bone_id(cal_skel, "jaw");
		skel->cal_bones_id[body_top_bone] = find_core_bone_id(cal_skel, "body3");
		skel->cal_bones_id[body_bottom_bone] = find_core_bone_id(cal_skel, "body1");
		skel->cal_bones_id[highest_bone] = skel->cal_bones_id[head_bone];
	}
	else if (!strcmp(&skeleton_name[length-10], "feline.csf")) {
		skel->cal_bones_id[head_bone] = find_core_bone_id(cal_skel, "head");
		skel->cal_bones_id[mouth_bone] = find_core_bone_id(cal_skel, "mouth");
		skel->cal_bones_id[body_top_bone] = find_core_bone_id(cal_skel, "neck");
		skel->cal_bones_id[body_bottom_bone] = find_core_bone_id(cal_skel, "bodyF");
		skel->cal_bones_id[highest_bone] = skel->cal_bones_id[head_bone];
	}
	else if (!strcmp(&skeleton_name[length-9], "horse.csf")) {
		skel->cal_bones_id[head_bone] = find_core_bone_id(cal_skel, "head");
		skel->cal_bones_id[mouth_bone] = find_core_bone_id(cal_skel, "jaw");
		skel->cal_bones_id[body_top_bone] = find_core_bone_id(cal_skel, "neck1");
		skel->cal_bones_id[body_bottom_bone] = find_core_bone_id(cal_skel, "front_body");
		skel->cal_bones_id[highest_bone] = skel->cal_bones_id[head_bone];
	}
	else if (!strcmp(&skeleton_name[length-10], "medium.csf")) {
		skel->cal_bones_id[head_bone] = find_core_bone_id(cal_skel, "head");
		skel->cal_bones_id[mouth_bone] = skel->cal_bones_id[head_bone];
		skel->cal_bones_id[body_top_bone] = find_core_bone_id(cal_skel, "back3");
		skel->cal_bones_id[body_bottom_bone] = find_core_bone_id(cal_skel, "back1");
		skel->cal_bones_id[cape_top_bone] = find_core_bone_id(cal_skel, "cape1");
		skel->cal_bones_id[cape_middle_bone] = find_core_bone_id(cal_skel, "cape2");
		skel->cal_bones_id[cape_bottom_bone] = find_core_bone_id(cal_skel, "cape3");
		skel->cal_bones_id[weapon_left_bone] = find_core_bone_id(cal_skel, "weaponL");
		skel->cal_bones_id[weapon_right_bone] = find_core_bone_id(cal_skel, "weaponR");
		skel->cal_bones_id[staff_right_bone] = find_core_bone_id(cal_skel, "staffR");
		skel->cal_bones_id[arrow_bone] = find_core_bone_id(cal_skel, "arrow");
		skel->cal_bones_id[hand_left_bone] = find_core_bone_id(cal_skel, "handL");
		skel->cal_bones_id[hand_right_bone] = find_core_bone_id(cal_skel, "handR");
		skel->cal_bones_id[highest_bone] = skel->cal_bones_id[head_bone];
	}
	else if (!strcmp(&skeleton_name[length-12], "monster1.csf")) {
		skel->cal_bones_id[head_bone] = find_core_bone_id(cal_skel, "head");
		skel->cal_bones_id[mouth_bone] = skel->cal_bones_id[head_bone];
		skel->cal_bones_id[body_top_bone] = find_core_bone_id(cal_skel, "spine3");
		skel->cal_bones_id[body_bottom_bone] = find_core_bone_id(cal_skel, "spine1");
		skel->cal_bones_id[cape_top_bone] = find_core_bone_id(cal_skel, "cape1");
		skel->cal_bones_id[cape_middle_bone] = find_core_bone_id(cal_skel, "cape2");
		skel->cal_bones_id[cape_bottom_bone] = find_core_bone_id(cal_skel, "cape3");
		skel->cal_bones_id[weapon_left_bone] = find_core_bone_id(cal_skel, "weaponL");
		skel->cal_bones_id[weapon_right_bone] = find_core_bone_id(cal_skel, "weaponR");
		skel->cal_bones_id[hand_left_bone] = find_core_bone_id(cal_skel, "handL");
		skel->cal_bones_id[hand_right_bone] = find_core_bone_id(cal_skel, "handR");
		skel->cal_bones_id[highest_bone] = skel->cal_bones_id[head_bone];
	}
	else if (!strcmp(&skeleton_name[length-13], "nenorocit.csf")) {
		skel->cal_bones_id[head_bone] = find_core_bone_id(cal_skel, "head");
		skel->cal_bones_id[mouth_bone] = find_core_bone_id(cal_skel, "mouth");
		skel->cal_bones_id[body_top_bone] = find_core_bone_id(cal_skel, "neck1");
		skel->cal_bones_id[body_bottom_bone] = find_core_bone_id(cal_skel, "bodyF");
		skel->cal_bones_id[highest_bone] = skel->cal_bones_id[head_bone];
	}
	else if (!strcmp(&skeleton_name[length-11], "penguin.csf")) {
		skel->cal_bones_id[head_bone] = find_core_bone_id(cal_skel, "head");
		skel->cal_bones_id[mouth_bone] = find_core_bone_id(cal_skel, "mouth");
		skel->cal_bones_id[body_top_bone] = find_core_bone_id(cal_skel, "body2");
		skel->cal_bones_id[body_bottom_bone] = find_core_bone_id(cal_skel, "body1");
		skel->cal_bones_id[highest_bone] = skel->cal_bones_id[head_bone];
	}
	else if (!strcmp(&skeleton_name[length-10], "rabbit.csf")) {
		skel->cal_bones_id[head_bone] = find_core_bone_id(cal_skel, "head");
		skel->cal_bones_id[mouth_bone] = skel->cal_bones_id[head_bone];
		skel->cal_bones_id[body_top_bone] = find_core_bone_id(cal_skel, "root");
		skel->cal_bones_id[body_bottom_bone] = skel->cal_bones_id[body_top_bone];
		skel->cal_bones_id[highest_bone] = skel->cal_bones_id[head_bone];
	}
	else if (!strcmp(&skeleton_name[length-7], "rat.csf")) {
		skel->cal_bones_id[head_bone] = find_core_bone_id(cal_skel, "head");
		skel->cal_bones_id[mouth_bone] = find_core_bone_id(cal_skel, "mouth");
		skel->cal_bones_id[body_top_bone] = find_core_bone_id(cal_skel, "neck");
		skel->cal_bones_id[body_bottom_bone] = find_core_bone_id(cal_skel, "root");
		skel->cal_bones_id[highest_bone] = skel->cal_bones_id[head_bone];
	}
	else if (!strcmp(&skeleton_name[length-9], "snake.csf")) {
		skel->cal_bones_id[head_bone] = find_core_bone_id(cal_skel, "head");
		skel->cal_bones_id[mouth_bone] = find_core_bone_id(cal_skel, "jaw");
		skel->cal_bones_id[body_top_bone] = find_core_bone_id(cal_skel, "body4");
		skel->cal_bones_id[body_bottom_bone] = find_core_bone_id(cal_skel, "body1");
		skel->cal_bones_id[highest_bone] = skel->cal_bones_id[head_bone];
	}
	else if (!strcmp(&skeleton_name[length-10], "spider.csf")) {
		skel->cal_bones_id[head_bone] = find_core_bone_id(cal_skel, "jaw");
		skel->cal_bones_id[mouth_bone] = skel->cal_bones_id[head_bone];
		skel->cal_bones_id[body_top_bone] = find_core_bone_id(cal_skel, "bodyF");
		skel->cal_bones_id[body_bottom_bone] = find_core_bone_id(cal_skel, "root");
		skel->cal_bones_id[highest_bone] = skel->cal_bones_id[body_top_bone];
	}
	else if (!strcmp(&skeleton_name[length-10], "wraith.csf")) {
		skel->cal_bones_id[head_bone] = find_core_bone_id(cal_skel, "head");
		skel->cal_bones_id[mouth_bone] = skel->cal_bones_id[head_bone];
		skel->cal_bones_id[body_top_bone] = find_core_bone_id(cal_skel, "spine2");
		skel->cal_bones_id[body_bottom_bone] = find_core_bone_id(cal_skel, "spine1");
		skel->cal_bones_id[hand_left_bone] = find_core_bone_id(cal_skel, "handL");
		skel->cal_bones_id[hand_right_bone] = find_core_bone_id(cal_skel, "handR");
		skel->cal_bones_id[highest_bone] = skel->cal_bones_id[head_bone];
	}
	else if (!strcmp(&skeleton_name[length-10], "target.csf")) {
		skel->cal_bones_id[head_bone] = find_core_bone_id(cal_skel, "head");
		skel->cal_bones_id[mouth_bone] = skel->cal_bones_id[head_bone];
		skel->cal_bones_id[body_top_bone] = skel->cal_bones_id[head_bone];
		skel->cal_bones_id[body_bottom_bone] = skel->cal_bones_id[head_bone];
		skel->cal_bones_id[highest_bone] = skel->cal_bones_id[head_bone];
	}
	else {
		LOG_ERROR("The skeleton '%s' is unknown, trying to guess main parts\n",
				  skeleton_name);
		skel->cal_bones_id[head_bone] = find_core_bone_id(cal_skel, "head");
		skel->cal_bones_id[mouth_bone] = find_core_bone_id(cal_skel, "mouth");
		if (skel->cal_bones_id[mouth_bone] < 0)
			skel->cal_bones_id[mouth_bone] = find_core_bone_id(cal_skel, "jaw");
		if (skel->cal_bones_id[head_bone] < 0)
			skel->cal_bones_id[head_bone] = skel->cal_bones_id[mouth_bone];
		if (skel->cal_bones_id[body_top_bone] < 0)
			skel->cal_bones_id[body_top_bone] = skel->cal_bones_id[mouth_bone];
		skel->cal_bones_id[highest_bone] = skel->cal_bones_id[head_bone];
	}

	return skeletons_count-1;
}

int get_actor_bone_id(actor *act, cal_bone_name name)
{
	int skel_type;
	if (!act || act->actor_type < 0) return -1;
	skel_type = actors_defs[act->actor_type].skeleton_type;
	if (skel_type < 0) return -1;
	return skeletons_defs[skel_type].cal_bones_id[name];
}
