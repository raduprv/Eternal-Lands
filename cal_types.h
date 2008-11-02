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
#ifndef EMOTES
	NUM_ACTOR_FRAMES = 39
#else // EMOTES
	cal_actor_emote_wave_frame = 39,
	cal_actor_emote_nod_head_frame = 40,
	cal_actor_emote_shake_head_frame = 41,
	cal_actor_emote_clap_hands_frame = 42,
	cal_actor_emote_shrug_frame = 43,
	cal_actor_emote_scratch_head_frame = 44,
	cal_actor_emote_jump_frame = 45,
	cal_actor_emote_stretch_frame = 46,
	cal_actor_emote_bow_frame = 47,
	NUM_ACTOR_FRAMES = 48
#endif // EMOTES
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
	NUM_WEAPON_FRAMES = 25
};

#ifdef ATTACHED_ACTORS
enum {
	cal_attached_walk_frame = 0, /*!< walk animation to use for the held actor */
	cal_attached_idle_frame = 1, /*!< idle animation to use for the held actor */
	cal_attached_pain_frame = 2, /*!< pain animation to use for the held actor */
	NUM_ATTACHED_ACTOR_FRAMES = 3
};
#endif // ATTACHED_ACTORS

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __CAL_TYPES_H__
