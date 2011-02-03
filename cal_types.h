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

enum {
	cal_actor_walk_frame = 0,
	cal_actor_run_frame = 1,
	cal_actor_die1_frame = 2,
	cal_actor_die2_frame = 3,
	cal_actor_pain1_frame = 4,
	cal_actor_pain2_frame = 5,
	cal_actor_pick_frame = 6,
	cal_actor_drop_frame = 7,
	cal_actor_idle1_frame = 8,
	cal_actor_idle2_frame = 9,
	cal_actor_idle_sit_frame = 10,
	cal_actor_harvest_frame = 11,
	cal_actor_attack_cast_frame = 12,
	cal_actor_attack_ranged_frame = 13,
	cal_actor_sit_down_frame = 14,
	cal_actor_stand_up_frame = 15,
	cal_actor_in_combat_frame = 16,
	cal_actor_out_combat_frame = 17,
	cal_actor_combat_idle_frame = 18,
	cal_actor_attack_up_1_frame = 19,
	cal_actor_attack_up_2_frame = 20,
	cal_actor_attack_up_3_frame = 21,
	cal_actor_attack_up_4_frame = 22,
	cal_actor_attack_up_5_frame = 23,
	cal_actor_attack_up_6_frame = 24,
	cal_actor_attack_up_7_frame = 25,
	cal_actor_attack_up_8_frame = 26,
	cal_actor_attack_up_9_frame = 27,
	cal_actor_attack_up_10_frame = 28,
	cal_actor_attack_down_1_frame = 29,
	cal_actor_attack_down_2_frame = 30,
	cal_actor_attack_down_3_frame = 31,
	cal_actor_attack_down_4_frame = 32,
	cal_actor_attack_down_5_frame = 33,
	cal_actor_attack_down_6_frame = 34,
	cal_actor_attack_down_7_frame = 35,
	cal_actor_attack_down_8_frame = 36,
	cal_actor_attack_down_9_frame = 37,
	cal_actor_attack_down_10_frame = 38,
	cal_actor_in_combat_held_frame = 39,
	cal_actor_out_combat_held_frame = 40,
	cal_actor_combat_idle_held_frame = 41,
	cal_actor_attack_up_1_held_frame = 42,
	cal_actor_attack_up_2_held_frame = 43,
	cal_actor_attack_up_3_held_frame = 44,
	cal_actor_attack_up_4_held_frame = 45,
	cal_actor_attack_up_5_held_frame = 46,
	cal_actor_attack_up_6_held_frame = 47,
	cal_actor_attack_up_7_held_frame = 48,
	cal_actor_attack_up_8_held_frame = 49,
	cal_actor_attack_up_9_held_frame = 50,
	cal_actor_attack_up_10_held_frame = 51,
	cal_actor_attack_down_1_held_frame = 52,
	cal_actor_attack_down_2_held_frame = 53,
	cal_actor_attack_down_3_held_frame = 54,
	cal_actor_attack_down_4_held_frame = 55,
	cal_actor_attack_down_5_held_frame = 56,
	cal_actor_attack_down_6_held_frame = 57,
	cal_actor_attack_down_7_held_frame = 58,
	cal_actor_attack_down_8_held_frame = 59,
	cal_actor_attack_down_9_held_frame = 60,
	cal_actor_attack_down_10_held_frame = 61,
	cal_actor_turn_left_frame=62,
	cal_actor_turn_right_frame=63,
	cal_actor_in_combat_held_unarmed_frame = 64,
	cal_actor_out_combat_held_unarmed_frame = 65,
	cal_actor_combat_idle_held_unarmed_frame = 66,
		NUM_ACTOR_FRAMES = 67
	
};

enum {
	cal_weapon_attack_up_1_frame = 0,
	cal_weapon_attack_up_2_frame = 1,
	cal_weapon_attack_up_3_frame = 2,
	cal_weapon_attack_up_4_frame = 3,
	cal_weapon_attack_up_5_frame = 4,
	cal_weapon_attack_up_6_frame = 5,
	cal_weapon_attack_up_7_frame = 6,
	cal_weapon_attack_up_8_frame = 7,
	cal_weapon_attack_up_9_frame = 8,
	cal_weapon_attack_up_10_frame = 9,
	cal_weapon_attack_down_1_frame = 10,
	cal_weapon_attack_down_2_frame = 11,
	cal_weapon_attack_down_3_frame = 12,
	cal_weapon_attack_down_4_frame = 13,
	cal_weapon_attack_down_5_frame = 14,
	cal_weapon_attack_down_6_frame = 15,
	cal_weapon_attack_down_7_frame = 16,
	cal_weapon_attack_down_8_frame = 17,
	cal_weapon_attack_down_9_frame = 18,
	cal_weapon_attack_down_10_frame = 19,
	cal_weapon_range_in_frame = 20,
	cal_weapon_range_out_frame = 21,
	cal_weapon_range_idle_frame = 22,
	cal_weapon_range_fire_frame = 23,
	cal_weapon_range_fire_out_frame = 24,
	//frames for held actors
	cal_weapon_range_in_held_frame = 25,
	cal_weapon_range_out_held_frame = 26,
	cal_weapon_range_idle_held_frame = 27,
	cal_weapon_range_fire_held_frame = 28,
	cal_weapon_range_fire_out_held_frame = 29,
	cal_weapon_attack_up_1_held_frame = 30,
	cal_weapon_attack_up_2_held_frame = 31,
	cal_weapon_attack_up_3_held_frame = 32,
	cal_weapon_attack_up_4_held_frame = 33,
	cal_weapon_attack_up_5_held_frame = 34,
	cal_weapon_attack_up_6_held_frame = 35,
	cal_weapon_attack_up_7_held_frame = 36,
	cal_weapon_attack_up_8_held_frame = 37,
	cal_weapon_attack_up_9_held_frame = 38,
	cal_weapon_attack_up_10_held_frame = 39,
	cal_weapon_attack_down_1_held_frame = 40,
	cal_weapon_attack_down_2_held_frame = 41,
	cal_weapon_attack_down_3_held_frame = 42,
	cal_weapon_attack_down_4_held_frame = 43,
	cal_weapon_attack_down_5_held_frame = 44,
	cal_weapon_attack_down_6_held_frame = 45,
	cal_weapon_attack_down_7_held_frame = 46,
	cal_weapon_attack_down_8_held_frame = 47,
	cal_weapon_attack_down_9_held_frame = 48,
	cal_weapon_attack_down_10_held_frame = 49,
	NUM_WEAPON_FRAMES = 50
};

#define EMOTES_FRAMES 100


enum {
	cal_attached_walk_frame = 0, /*!< walk animation to use for the held actor */
	cal_attached_run_frame = 1, /*!< run animation to use for the held actor */
	cal_attached_idle_frame = 2, /*!< idle animation to use for the held actor */
	cal_attached_pain_frame = 3, /*!< pain animation to use for the held actor */
	cal_attached_pain_armed_frame = 4,
	NUM_ATTACHED_ACTOR_FRAMES = 5
};

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __CAL_TYPES_H__
