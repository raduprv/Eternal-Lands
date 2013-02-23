////////////////////////////////////////////////////////////////////////////////
// eye_candy_wrapper.h                                                        //
// Copyright (C) 2006 Karen Pease                                             //
// Based on the cal3d wrapper by Bruno 'Beosil' Heidelberger                  //
////////////////////////////////////////////////////////////////////////////////
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef EYE_CANDY_WRAPPER_H
#define EYE_CANDY_WRAPPER_H

#include "eye_candy_types.h"

#include "platform.h"
#include "actors.h"
#include "e3d.h"

#ifdef __cplusplus
#include "eye_candy/eye_candy.h"
#include "eye_candy/math_cache.h"
#include "eye_candy/effect_lamp.h"
#include "eye_candy/effect_candle.h"
#include "eye_candy/effect_campfire.h"
#include "eye_candy/effect_fountain.h"
#include "eye_candy/effect_teleporter.h"
#include "eye_candy/effect_firefly.h"
#include "eye_candy/effect_sword.h"
#include "eye_candy/effect_staff.h"
#include "eye_candy/effect_summon.h"
#include "eye_candy/effect_selfmagic.h"
#include "eye_candy/effect_targetmagic.h"
#include "eye_candy/effect_ongoing.h"
#include "eye_candy/effect_impact.h"
#include "eye_candy/effect_smoke.h"
#include "eye_candy/effect_bag.h"
#include "eye_candy/effect_glow.h"
#include "eye_candy/effect_cloud.h"
#include "eye_candy/effect_harvesting.h"
#include "eye_candy/effect_wind.h"
#include "eye_candy/effect_breath.h"
#include "eye_candy/effect_mines.h"
#include "eye_candy/effect_missile.h"
#endif

#ifdef __cplusplus
extern "C" int use_eye_candy;
extern "C" int use_harvesting_eye_candy;
#ifdef MAP_EDITOR
extern ec::SmoothPolygonBoundingRange initial_bounds;
#endif
#else
extern int use_eye_candy;
extern int use_harvesting_eye_candy;
extern int use_lamp_halo;
extern float min_ec_framerate;
extern float max_ec_framerate;
#ifndef	NEW_TEXTURES
extern int transparency_resolution_fix;
#endif	/* NEW_TEXTURES */
extern int light_columns_threshold;
extern int use_fancy_smoke;
extern int max_idle_cycles_per_second;
#endif

////////////////////////////////////////////////////////////////////////////////
// Defines for Win32 and MingW32                                              //
////////////////////////////////////////////////////////////////////////////////

#ifdef WINDOWS
#ifndef __MINGW32__
#pragma warning(disable : 4251)
#pragma warning(disable : 4786)
#pragma warning(disable : 4099)
#endif
#endif

////////////////////////////////////////////////////////////////////////////////
// "C" wrapper functions declaration                                          //
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C"
{
	typedef class ec_internal_reference
	{
		public:
			ec_internal_reference()
			{
				effect = NULL;
				caster = NULL;
				target = NULL;
				dead = false;
			}
			;
			~ec_internal_reference()
			{
			}
			;

			ec::Effect* effect;
			ec::Vec3 position;
			ec::Vec3 position2;
			actor* caster;
			actor* target;
			std::vector<actor*> target_actors;
			std::vector<ec::Vec3> targets;
			ec::SmoothPolygonBoundingRange bounds;
			bool dead;
			int casterbone;
			int targetbone;
			int missile_id;
	} ec_internal_reference;

	typedef struct ec_object_obstruction
	{
			object3d* obj3d;
			e3d_object* e3dobj;
			ec::Vec3 center;
			float sin_rot_x;
			float cos_rot_x;
			float sin_rot_y;
			float cos_rot_y;
			float sin_rot_z;
			float cos_rot_z;
			float sin_rot_x2;
			float cos_rot_x2;
			float sin_rot_y2;
			float cos_rot_y2;
			float sin_rot_z2;
			float cos_rot_z2;
			bool fire_related;
			ec::Obstruction* obstruction;
	} ec_object_obstruction;

	typedef struct ec_actor_obstruction
	{
			actor* obstructing_actor;
			ec::Vec3 center;
			ec::Obstruction* obstruction;
	} ec_actor_obstruction;

	typedef std::vector<ec_object_obstruction*> ec_object_obstructions;
	typedef std::vector<ec_actor_obstruction*> ec_actor_obstructions;
	typedef std::vector<ec::Effect*> ec_internal_effects;

#endif

	typedef enum ec_EffectEnum // Keep in sync with eye_candy/eye_candy.h!

	{
		EC_LAMP = 0,
		EC_CAMPFIRE = 1,
		EC_FOUNTAIN = 2,
		EC_TELEPORTER = 3,
		EC_FIREFLY = 4,
		EC_SWORD = 5,
		EC_SUMMON = 6,
		EC_SELFMAGIC = 7,
		EC_TARGETMAGIC = 8,
		EC_ONGOING = 9,
		EC_IMPACT = 10,
		EC_SMOKE = 11,
		EC_BAG = 12,
		EC_CLOUD = 13,
		EC_HARVESTING = 14,
		EC_WIND = 15,
		EC_BREATH = 16,
		EC_CANDLE = 17,
		EC_MINES = 18,
		EC_GLOW = 19,
		EC_MISSILE = 20,
		EC_STAFF = 21
	} ec_EffectEnum;

	////////////////////////////////////////////////////////////////////////////////
	// EyeCandy wrapper functions declaration                                     //
	////////////////////////////////////////////////////////////////////////////////

#ifndef	NEW_TEXTURES
	void ec_clear_textures();
#endif	/* NEW_TEXTURES */
	void ec_load_textures();
	void ec_init();
	void ec_add_light(GLenum light_id);
#ifndef	NEW_TEXTURES
	void ec_set_draw_method();
#endif	/* NEW_TEXTURES */
	float ec_get_z(actor* _actor);
	float ec_get_z2(int x, int y);
	void ec_idle(); //!< \callergraph
	void ec_heartbeat(); // Once per second.
	void ec_draw(); //!< \callergraph
	void ec_actor_delete(actor* _actor);
	void ec_recall_effect(ec_reference ref);
	void ec_destroy_all_effects();
	void ec_delete_all_effects();
	void ec_delete_effect_loc(float x, float y);
	void ec_delete_effect_loc_type(float x, float y, ec_EffectEnum type);
	void ec_delete_effect_type(ec_EffectEnum type);
	void ec_delete_reference(ec_reference ref);
	void ec_set_position(ec_reference ref, float x, float y, float z);
	void ec_set_position2(ec_reference ref, float x, float y, float z);
	void ec_clear_obstruction_list();
	void ec_free_obstruction_list();
	void ec_add_object_obstruction(object3d* obj3d, e3d_object *e3dobj,
		float force);
	void ec_add_actor_obstruction(actor* actor, float force);
	ec_bounds ec_create_bounds_list();
	void ec_remove_obstruction_by_object3d(object3d* obj3d);
	void ec_remove_obstruction_by_e3d_object(e3d_object* obj3d);
	void ec_free_bounds_list(ec_bounds bounds);
	//  void ec_add_polar_coords_bound(ec_bounds bounds, float frequency, float offset, float scalar, float power);
	void
		ec_add_smooth_polygon_bound(ec_bounds bounds, float angle, float radius);
	ec_effects ec_create_effects_list();
	void ec_free_effects_list(ec_effects effects);
	void ec_remove_weapon(actor* _actor);
	void ec_remove_missile(int missile_id);
	void ec_rename_missile(int old_id, int new_id);
	void ec_add_effect(ec_effects effects, ec_reference ref);
	int ec_in_range(float x, float y, float z, Uint64 effect_max_length);
	ec_reference ec_create_generic();
	void ec_add_target(ec_reference reference, float x, float y, float z);
	int ec_change_target(ec_reference reference, int index, float x, float y,
		float z);
	ec_reference ec_create_effect_from_map_code(char* code, float x, float y,
		float z, int LOD);
	ec_reference ec_create_bag_pickup(float x, float y, float z, int LOD);
	ec_reference ec_create_bag_drop(float x, float y, float z, int LOD);
	ec_reference ec_create_breath_fire(float sx, float sy, float sz, float tx,
		float ty, float tz, int LOD, float scale);
	ec_reference ec_create_breath_fire2(actor* caster, actor* target, int LOD,
		float scale);
	ec_reference ec_create_breath_ice(float sx, float sy, float sz, float tx,
		float ty, float tz, int LOD, float scale);
	ec_reference ec_create_breath_ice2(actor* caster, actor* target, int LOD,
		float scale);
	ec_reference ec_create_breath_poison(float sx, float sy, float sz,
		float tx, float ty, float tz, int LOD, float scale);
	ec_reference ec_create_breath_poison2(actor* caster, actor* target,
		int LOD, float scale);
	ec_reference ec_create_breath_magic(float sx, float sy, float sz, float tx,
		float ty, float tz, int LOD, float scale);
	ec_reference ec_create_breath_magic2(actor* caster, actor* target, int LOD,
		float scale);
	ec_reference ec_create_breath_lightning(float sx, float sy, float sz,
		float tx, float ty, float tz, int LOD, float scale);
	ec_reference ec_create_breath_lightning2(actor* caster, actor* target,
		int LOD, float scale);
	ec_reference ec_create_breath_wind(float sx, float sy, float sz, float tx,
		float ty, float tz, int LOD, float scale);
	ec_reference ec_create_breath_wind2(actor* caster, actor* target, int LOD,
		float scale);
	ec_reference ec_create_campfire(float x, float y, float z,
		float hue_adjust, float saturation_adjust, int LOD, float scale);
	ec_reference ec_create_cloud(float x, float y, float z, float hue_adjust,
		float saturation_adjust, float density, ec_bounds bounds, int LOD);
	ec_reference ec_create_fireflies(float x, float y, float z,
		float hue_adjust, float saturation_adjust, float density, float scale,
		ec_bounds bounds);
	ec_reference ec_create_fountain(float x, float y, float z,
		float hue_adjust, float saturation_adjust, float base_height,
		int backlit, float scale, int LOD);
	ec_reference ec_create_glow_harm(actor* caster, int LOD);
	ec_reference ec_create_glow_level_up_att(actor* caster, int LOD);
	ec_reference ec_create_glow_level_up_def(actor* caster, int LOD);
	ec_reference ec_create_glow_level_up_har(actor* caster, int LOD);
	ec_reference ec_create_glow_level_up_alc_left(actor* caster, int LOD);
	ec_reference ec_create_glow_level_up_alc_right(actor* caster, int LOD);
	ec_reference ec_create_glow_level_up_mag(actor* caster, int LOD);
	ec_reference ec_create_glow_level_up_pot_left(actor* caster, int LOD);
	ec_reference ec_create_glow_level_up_pot_right(actor* caster, int LOD);
	ec_reference ec_create_glow_level_up_sum(actor* caster, int LOD);
	ec_reference ec_create_glow_level_up_man_left(actor* caster, int LOD);
	ec_reference ec_create_glow_level_up_man_right(actor* caster, int LOD);
	ec_reference ec_create_glow_level_up_cra_left(actor* caster, int LOD);
	ec_reference ec_create_glow_level_up_cra_right(actor* caster, int LOD);
	ec_reference ec_create_glow_level_up_eng_left(actor* caster, int LOD);
	ec_reference ec_create_glow_level_up_eng_right(actor* caster, int LOD);
	ec_reference ec_create_glow_level_up_tai_left(actor* caster, int LOD);
	ec_reference ec_create_glow_level_up_tai_right(actor* caster, int LOD);
	ec_reference ec_create_glow_level_up_ran(actor* caster, int LOD);
	ec_reference ec_create_glow_level_up_default(actor* caster, int LOD);
	ec_reference ec_create_glow_level_up_oa(actor* caster, int LOD);
	ec_reference ec_create_glow_poison(actor* caster, int LOD);
	ec_reference ec_create_glow_remote_heal(actor* caster, int LOD);
	ec_reference ec_create_harvesting_radon_pouch(float x, float y, float z,
		int LOD);
	ec_reference ec_create_harvesting_radon_pouch2(actor* caster, int LOD);
	ec_reference ec_create_harvesting_cavern_wall(float x, float y, float z,
		int LOD);
	ec_reference ec_create_harvesting_cavern_wall2(actor* caster, int LOD);
	ec_reference ec_create_harvesting_mother_nature(float x, float y, float z,
		int LOD);
	ec_reference ec_create_harvesting_mother_nature2(actor* caster, int LOD);
	ec_reference ec_create_harvesting_queen_of_nature(float x, float y,
		float z, int LOD);
	ec_reference ec_create_harvesting_queen_of_nature2(actor* caster, int LOD);
	ec_reference ec_create_harvesting_bees(float x, float y, float z, int LOD);
	ec_reference ec_create_harvesting_bees2(actor* caster, int LOD);
	ec_reference ec_create_harvesting_bag_of_gold(float x, float y, float z,
		int LOD);
	ec_reference ec_create_harvesting_bag_of_gold2(actor* caster, int LOD);
	ec_reference ec_create_harvesting_tool_break(actor* caster, int LOD);
	ec_reference ec_create_harvesting_rare_stone(float x, float y, float z,
		int LOD);
	ec_reference ec_create_harvesting_rare_stone2(actor* caster, int LOD);
	ec_reference ec_create_impact_magic_protection(float x, float y, float z,
		float angle_x, float angle_y, float angle_z, int LOD, float strength);
	ec_reference ec_create_impact_shield(float x, float y, float z,
		float angle_x, float angle_y, float angle_z, int LOD, float strength);
	ec_reference ec_create_impact_magic_immunity(float x, float y, float z,
		float angle_x, float angle_y, float angle_z, int LOD, float strength);
	ec_reference ec_create_impact_poison(float x, float y, float z,
		float angle_x, float angle_y, float angle_z, int LOD, float strength);
	ec_reference ec_create_impact_blood(float x, float y, float z,
		float angle_x, float angle_y, float angle_z, int LOD, float strength);
	ec_reference ec_create_lamp(float x, float y, float z, float hue_adjust,
		float saturation_adjust, float scale, int LOD);
	ec_reference ec_create_candle(float x, float y, float z, float hue_adjust,
		float saturation_adjust, float scale, int LOD);
	ec_reference ec_create_ongoing_magic_protection(float x, float y, float z,
		float hue_adjust, float saturation_adjust, int LOD, float scale);
	ec_reference ec_create_ongoing_shield(float x, float y, float z,
		float hue_adjust, float saturation_adjust, int LOD, float scale);
	ec_reference ec_create_ongoing_magic_immunity(float x, float y, float z,
		float hue_adjust, float saturation_adjust, int LOD, float scale);
	ec_reference ec_create_ongoing_poison(float x, float y, float z,
		float hue_adjust, float saturation_adjust, int LOD, float scale);
	ec_reference ec_create_ongoing_magic_protection2(actor* caster,
		float hue_adjust, float saturation_adjust, int LOD, float scale);
	ec_reference ec_create_ongoing_shield2(actor* caster, float hue_adjust,
		float saturation_adjust, int LOD, float scale);
	ec_reference ec_create_ongoing_magic_immunity2(actor* caster,
		float hue_adjust, float saturation_adjust, int LOD, float scale);
	ec_reference ec_create_ongoing_poison2(actor* caster, float hue_adjust,
		float saturation_adjust, int LOD, float scale);
	ec_reference ec_create_ongoing_harvesting2(actor* caster, float hue_adjust,
		float saturation_adjust, int LOD, float scale);
	ec_reference ec_create_selfmagic_heal(float x, float y, float z, int LOD);
	ec_reference ec_create_selfmagic_heal2(actor* caster, int LOD);
	ec_reference ec_create_selfmagic_magic_protection(float x, float y,
		float z, int LOD);
	ec_reference ec_create_selfmagic_magic_protection2(actor* caster, int LOD);
	ec_reference ec_create_selfmagic_shield(float x, float y, float z, int LOD);
	ec_reference ec_create_selfmagic_shield_generic(actor* caster, int LOD,
		special_effect_enum type);
	ec_reference ec_create_selfmagic_shield2(actor* caster, int LOD);
	ec_reference ec_create_selfmagic_restoration(float x, float y, float z,
		int LOD);
	ec_reference ec_create_selfmagic_restoration2(actor* caster, int LOD);
	ec_reference ec_create_selfmagic_bones_to_gold(float x, float y, float z,
		int LOD);
	ec_reference ec_create_selfmagic_bones_to_gold2(actor* caster, int LOD);
	ec_reference ec_create_selfmagic_teleport_to_the_portals_room(float x,
		float y, float z, int LOD);
	ec_reference ec_create_selfmagic_teleport_to_the_portals_room2(
		actor* caster, int LOD);
	ec_reference ec_create_selfmagic_magic_immunity(float x, float y, float z,
		int LOD);
	ec_reference ec_create_selfmagic_magic_immunity2(actor* caster, int LOD);
	ec_reference ec_create_alert(float x, float y, float z, int LOD);
	ec_reference ec_create_alert2(actor* caster, int LOD);
	ec_reference ec_create_smoke(float x, float y, float z, float hue_adjust,
		float saturation_adjust, float scale, int LOD);
	ec_reference ec_create_summon_rabbit(float x, float y, float z, int LOD);
	ec_reference ec_create_summon_rabbit2(actor* caster, int LOD);
	ec_reference ec_create_summon_rat(float x, float y, float z, int LOD);
	ec_reference ec_create_summon_rat2(actor* caster, int LOD);
	ec_reference ec_create_summon_beaver(float x, float y, float z, int LOD);
	ec_reference ec_create_summon_beaver2(actor* caster, int LOD);
	ec_reference ec_create_summon_skunk(float x, float y, float z, int LOD);
	ec_reference ec_create_summon_skunk2(actor* caster, int LOD);
	ec_reference ec_create_summon_racoon(float x, float y, float z, int LOD);
	ec_reference ec_create_summon_racoon2(actor* caster, int LOD);
	ec_reference ec_create_summon_deer(float x, float y, float z, int LOD);
	ec_reference ec_create_summon_deer2(actor* caster, int LOD);
	ec_reference
		ec_create_summon_green_snake(float x, float y, float z, int LOD);
	ec_reference ec_create_summon_green_snake2(actor* caster, int LOD);
	ec_reference ec_create_summon_red_snake(float x, float y, float z, int LOD);
	ec_reference ec_create_summon_red_snake2(actor* caster, int LOD);
	ec_reference
		ec_create_summon_brown_snake(float x, float y, float z, int LOD);
	ec_reference ec_create_summon_brown_snake2(actor* caster, int LOD);
	ec_reference ec_create_summon_fox(float x, float y, float z, int LOD);
	ec_reference ec_create_summon_fox2(actor* caster, int LOD);
	ec_reference ec_create_summon_boar(float x, float y, float z, int LOD);
	ec_reference ec_create_summon_boar2(actor* caster, int LOD);
	ec_reference ec_create_summon_wolf(float x, float y, float z, int LOD);
	ec_reference ec_create_summon_wolf2(actor* caster, int LOD);
	ec_reference ec_create_summon_skeleton(float x, float y, float z, int LOD);
	ec_reference ec_create_summon_skeleton2(actor* caster, int LOD);
	ec_reference ec_create_summon_small_gargoyle(float x, float y, float z,
		int LOD);
	ec_reference ec_create_summon_small_gargoyle2(actor* caster, int LOD);
	ec_reference ec_create_summon_medium_gargoyle(float x, float y, float z,
		int LOD);
	ec_reference ec_create_summon_medium_gargoyle2(actor* caster, int LOD);
	ec_reference ec_create_summon_large_gargoyle(float x, float y, float z,
		int LOD);
	ec_reference ec_create_summon_large_gargoyle2(actor* caster, int LOD);
	ec_reference ec_create_summon_puma(float x, float y, float z, int LOD);
	ec_reference ec_create_summon_puma2(actor* caster, int LOD);
	ec_reference ec_create_summon_female_goblin(float x, float y, float z,
		int LOD);
	ec_reference ec_create_summon_female_goblin2(actor* caster, int LOD);
	ec_reference
		ec_create_summon_polar_bear(float x, float y, float z, int LOD);
	ec_reference ec_create_summon_polar_bear2(actor* caster, int LOD);
	ec_reference ec_create_summon_bear(float x, float y, float z, int LOD);
	ec_reference ec_create_summon_bear2(actor* caster, int LOD);
	ec_reference ec_create_summon_armed_male_goblin(float x, float y, float z,
		int LOD);
	ec_reference ec_create_summon_armed_male_goblin2(actor* caster, int LOD);
	ec_reference ec_create_summon_armed_skeleton(float x, float y, float z,
		int LOD);
	ec_reference ec_create_summon_armed_skeleton2(actor* caster, int LOD);
	ec_reference
		ec_create_summon_female_orc(float x, float y, float z, int LOD);
	ec_reference ec_create_summon_female_orc2(actor* caster, int LOD);
	ec_reference ec_create_summon_male_orc(float x, float y, float z, int LOD);
	ec_reference ec_create_summon_male_orc2(actor* caster, int LOD);
	ec_reference ec_create_summon_armed_female_orc(float x, float y, float z,
		int LOD);
	ec_reference ec_create_summon_armed_female_orc2(actor* caster, int LOD);
	ec_reference ec_create_summon_armed_male_orc(float x, float y, float z,
		int LOD);
	ec_reference ec_create_summon_armed_male_orc2(actor* caster, int LOD);
	ec_reference ec_create_summon_cyclops(float x, float y, float z, int LOD);
	ec_reference ec_create_summon_cyclops2(actor* caster, int LOD);
	ec_reference ec_create_summon_fluffy(float x, float y, float z, int LOD);
	ec_reference ec_create_summon_fluffy2(actor* caster, int LOD);
	ec_reference ec_create_summon_phantom_warrior(float x, float y, float z,
		int LOD);
	ec_reference ec_create_summon_phantom_warrior2(actor* caster, int LOD);
	ec_reference ec_create_summon_mountain_chimeran(float x, float y, float z,
		int LOD);
	ec_reference ec_create_summon_mountain_chimeran2(actor* caster, int LOD);
	ec_reference ec_create_summon_yeti(float x, float y, float z, int LOD);
	ec_reference ec_create_summon_yeti2(actor* caster, int LOD);
	ec_reference ec_create_summon_arctic_chimeran(float x, float y, float z,
		int LOD);
	ec_reference ec_create_summon_arctic_chimeran2(actor* caster, int LOD);
	ec_reference ec_create_summon_giant(float x, float y, float z, int LOD);
	ec_reference ec_create_summon_giant2(actor* caster, int LOD);
	ec_reference
		ec_create_summon_giant_snake(float x, float y, float z, int LOD);
	ec_reference ec_create_summon_giant_snake2(actor* caster, int LOD);
	ec_reference ec_create_summon_spider(float x, float y, float z, int LOD);
	ec_reference ec_create_summon_spider2(actor* caster, int LOD);
	ec_reference ec_create_summon_tiger(float x, float y, float z, int LOD);
	ec_reference ec_create_summon_tiger2(actor* caster, int LOD);
	ec_reference ec_create_sword_serpent(actor* _actor, int LOD);
	ec_reference ec_create_sword_cutlass(actor* _actor, int LOD);
	ec_reference ec_create_sword_emerald_claymore(actor* _actor, int LOD);
	ec_reference ec_create_sword_sunbreaker(actor* _actor, int LOD);
	ec_reference ec_create_sword_orc_slayer(actor* _actor, int LOD);
	ec_reference ec_create_sword_eagle_wing(actor* _actor, int LOD);
	ec_reference ec_create_sword_jagged_saber(actor* _actor, int LOD);
	ec_reference ec_create_sword_of_fire(actor* _actor, int LOD);
	ec_reference ec_create_sword_of_ice(actor* _actor, int LOD);
	ec_reference ec_create_sword_of_magic(actor* _actor, int LOD);
	ec_reference ec_create_staff_of_protection(actor* _actor, int LOD);
	ec_reference ec_create_staff_of_the_mage(actor* _actor, int LOD);
	ec_reference ec_create_targetmagic_remote_heal(float start_x,
		float start_y, float start_z, float end_x, float end_y, float end_z,
		int LOD);
	ec_reference ec_create_targetmagic_remote_heal2(actor* caster,
		actor* target, int LOD);
	ec_reference ec_create_targetmagic_poison(float start_x, float start_y,
		float start_z, float end_x, float end_y, float end_z, int LOD);
	ec_reference ec_create_targetmagic_poison2(actor* caster, actor* target,
		int LOD);
	ec_reference ec_create_targetmagic_teleport_to_range(float start_x,
		float start_y, float start_z, float end_x, float end_y, float end_z,
		int LOD);
	ec_reference ec_create_targetmagic_teleport_to_range2(actor* caster,
		actor* target, int LOD);
	ec_reference ec_create_targetmagic_harm(float start_x, float start_y,
		float start_z, float end_x, float end_y, float end_z, int LOD);
	ec_reference ec_create_targetmagic_harm2(actor* caster, actor* target,
		int LOD);
	ec_reference ec_create_targetmagic_life_drain(float start_x, float start_y,
		float start_z, float end_x, float end_y, float end_z, int LOD);
	ec_reference ec_create_targetmagic_life_drain2(actor* caster,
		actor* target, int LOD);
	void ec_launch_targetmagic_heal_summoned(ec_reference reference,
		float start_x, float start_y, float start_z, int LOD);
	void ec_launch_targetmagic_smite_summoned(ec_reference reference,
		float start_x, float start_y, float start_z, int LOD);
	ec_reference ec_create_targetmagic_drain_mana(float start_x, float start_y,
		float start_z, float end_x, float end_y, float end_z, int LOD);
	ec_reference ec_create_targetmagic_drain_mana2(actor* caster,
		actor* target, int LOD);
	ec_reference ec_create_teleporter(float x, float y, float z,
		float hue_adjust, float saturation_adjust, float scale, int LOD);
	ec_reference ec_create_wind_leaves(float x, float y, float z,
		float hue_adjust, float saturation_adjust, float scale, float density,
		ec_bounds bounds, float prevailing_wind_x, float prevailing_wind_y,
		float prevailing_wind_z);
	ec_reference ec_create_wind_petals(float x, float y, float z,
		float hue_adjust, float saturation_adjust, float scale, float density,
		ec_bounds bounds, float prevailing_wind_x, float prevailing_wind_y,
		float prevailing_wind_z);
	void ec_add_wind_effect_list(ec_reference reference, ec_effects effects);
	ec_reference ec_create_mine_drop(float x, float y, float z, int mine_type,
		int LOD);
	ec_reference ec_create_mine_prime(float x, float y, float z, int mine_type,
		int LOD);
	ec_reference ec_create_mine_remove(float x, float y, float z,
		int mine_type, int LOD);
	ec_reference ec_create_mine_detonate(float x, float y, float z,
		int mine_type, int LOD);
	ec_reference
		ec_create_mine_detonate2(actor* caster, int mine_type, int LOD);
	ec_reference
		ec_create_missile_effect(int missile_id, int LOD, int hitOrMiss);

#ifdef __cplusplus
}
#endif

#endif /* EYE_CANDY_WRAPPER_H */

////////////////////////////////////////////////////////////////////////////////

