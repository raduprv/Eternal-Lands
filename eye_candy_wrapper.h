#ifdef SFX

//****************************************************************************//
// eye_candy_wrapper.h                                                            //
// Copyright (C) 2006 Karen Pease
// Based on the cal3d wrapper by Bruno 'Beosil' Heidelberger                             //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#ifndef CAL_EYE_CANDY_WRAPPER_H
#define CAL_EYE_CANDY_WRAPPER_H

#include <SDL/SDL.h>

#ifdef __cplusplus
#include "eye_candy/eye_candy.h"
#include "eye_candy/effect_lamp.h"
#include "eye_candy/effect_campfire.h"
#include "eye_candy/effect_fountain.h"
#include "eye_candy/effect_teleporter.h"
#include "eye_candy/effect_firefly.h"
#include "eye_candy/effect_sword.h"
#include "eye_candy/effect_summon.h"
#include "eye_candy/effect_selfmagic.h"
#include "eye_candy/effect_targetmagic.h"
#include "eye_candy/effect_ongoing.h"
#include "eye_candy/effect_impact.h"
#include "eye_candy/effect_smoke.h"
#include "eye_candy/effect_bag.h"
#include "eye_candy/effect_cloud.h"
#include "eye_candy/effect_harvesting.h"
#include "eye_candy/effect_wind.h"
#include "eye_candy/effect_breath.h"
#endif

#include "actors.h"

//****************************************************************************//
// Defines for Win32 and MingW32                                              //
//****************************************************************************//

#ifdef WINDOWS

#ifdef __MINGW32__

#define EYE_CANDY_WRAPPER_API

#else

#pragma warning(disable : 4251)
#pragma warning(disable : 4786)
#pragma warning(disable : 4099)

#endif

#else

//****************************************************************************//
// Defines for Linux, Cygwin, FreeBSD Sun and Mips...                         //
//****************************************************************************//

#define EYE_CANDY_WRAPPER_API

#endif 

//****************************************************************************//
// "C" wrapper functions declaration                                          //
//****************************************************************************//

typedef void* ec_reference;
typedef void* ec_obstructions;
typedef void* ec_bounds;
typedef void* ec_effects;

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
  };
  ~ec_internal_reference() {};

  ec::Effect* effect;
  ec::Vec3 position;
  ec::Vec3 position2;
  actor* caster;
  actor* target;
  std::vector<actor*> target_actors;
  std::vector<ec::Vec3> targets;
  bool dead;
} ec_internal_reference;

typedef struct ec_internal_obstructions
{
  std::vector<ec::Vec3> positions;
  std::vector<ec::Obstruction*> obstructions;
} ec_internal_obstructions;

typedef std::vector<ec::PolarCoordElement> ec_internal_bounds;

typedef std::vector<ec::Effect*> ec_internal_effects;

#endif

typedef enum ec_EffectEnum	//Keep in sync with eye_candy/eye_candy.h!
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
  EC_BREATH = 16
} ec_EffectEnum;

//****************************************************************************//
// EyeCandy wrapper functions declaration                                     //
//****************************************************************************//

  EYE_CANDY_WRAPPER_API void ec_init();
  EYE_CANDY_WRAPPER_API void ec_add_light(GLenum light_id);
  EYE_CANDY_WRAPPER_API void ec_set_draw_method();
  EYE_CANDY_WRAPPER_API void ec_idle();
  EYE_CANDY_WRAPPER_API void ec_draw();
  EYE_CANDY_WRAPPER_API void ec_actor_delete(actor* _actor);
  EYE_CANDY_WRAPPER_API void ec_recall_effect(ec_reference ref);
  EYE_CANDY_WRAPPER_API void ec_delete_all_effects();
  EYE_CANDY_WRAPPER_API void ec_delete_effect_loc(float x, float y);
  EYE_CANDY_WRAPPER_API void ec_delete_effect_loc_type(float x, float y, ec_EffectEnum type);
  EYE_CANDY_WRAPPER_API void ec_delete_effect_type(ec_EffectEnum type);
  EYE_CANDY_WRAPPER_API void ec_delete_reference(ec_reference ref);
  EYE_CANDY_WRAPPER_API void ec_set_position(ec_reference ref, float x, float y, float z);
  EYE_CANDY_WRAPPER_API void ec_set_position2(ec_reference ref, float x, float y, float z);
  EYE_CANDY_WRAPPER_API ec_obstructions ec_create_obstruction_list();
  EYE_CANDY_WRAPPER_API void ec_free_obstruction_list(ec_obstructions obstructions);
  EYE_CANDY_WRAPPER_API int ec_delete_obstruction(ec_obstructions obstructions, int index);
  EYE_CANDY_WRAPPER_API void ec_add_spherical_obstruction(ec_obstructions obstructions, float x, float y, float z, float max_distance, float force);
  EYE_CANDY_WRAPPER_API void ec_add_simple_cylindrical_obstruction(ec_obstructions obstructions, float x, float y, float max_distance, float force);
  EYE_CANDY_WRAPPER_API void ec_add_cylindrical_obstruction(ec_obstructions obstructions, float x1, float y1, float z1, float x2, float y2, float z2, float max_distance, float force);
  EYE_CANDY_WRAPPER_API ec_bounds ec_create_bounds_list();
  EYE_CANDY_WRAPPER_API void ec_free_bounds_list(ec_bounds bounds);
  EYE_CANDY_WRAPPER_API void ec_add_polar_coords_bound(ec_bounds bounds, float frequency, float offset, float scalar, float power);
  EYE_CANDY_WRAPPER_API ec_effects ec_create_effects_list();
  EYE_CANDY_WRAPPER_API void ec_free_effects_list(ec_effects effects);
  EYE_CANDY_WRAPPER_API void ec_add_effect(ec_effects effects, ec_reference ref);
  EYE_CANDY_WRAPPER_API int ec_check_distance(float x, float y, float z, Uint64 effect_max_length);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_generic();
  EYE_CANDY_WRAPPER_API void ec_add_target(ec_reference reference, float x, float y, float z);
  EYE_CANDY_WRAPPER_API int ec_change_target(ec_reference reference, int index, float x, float y, float z);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_bag_pickup(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_bag_drop(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_breath_fire(float sx, float sy, float sz, float tx, float ty, float tz, ec_obstructions obstructions, int LOD, float scale);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_breath_ice(float sx, float sy, float sz, float tx, float ty, float tz, ec_obstructions obstructions, int LOD, float scale);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_breath_poison(float sx, float sy, float sz, float tx, float ty, float tz, ec_obstructions obstructions, int LOD, float scale);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_breath_magic(float sx, float sy, float sz, float tx, float ty, float tz, ec_obstructions obstructions, int LOD, float scale);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_breath_lightning(float sx, float sy, float sz, float tx, float ty, float tz, ec_obstructions obstructions, int LOD, float scale);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_breath_wind(float sx, float sy, float sz, float tx, float ty, float tz, ec_obstructions obstructions, int LOD, float scale);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_campfire(float x, float y, float z, ec_obstructions obstructions, int LOD, float scale);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_cloud(float x, float y, float z, float density, ec_bounds bounds, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_fireflies(float x, float y, float z, ec_obstructions obstructions, float density, ec_bounds bounds);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_fountain(float x, float y, float z, float base_height, int backlit, float scale, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_harvesting_radon_pouch(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_harvesting_cavern_wall(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_harvesting_mother_nature(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_harvesting_queen_of_nature(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_harvesting_bees(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_harvesting_bag_of_gold(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_harvesting_rare_stone(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_impact_magic_protection(float x, float y, float z, float angle_x, float angle_y, float angle_z, int LOD, float strength);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_impact_shield(float x, float y, float z, float angle_x, float angle_y, float angle_z, int LOD, float strength);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_impact_magic_immunity(float x, float y, float z, float angle_x, float angle_y, float angle_z, int LOD, float strength);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_impact_poison(float x, float y, float z, float angle_x, float angle_y, float angle_z, int LOD, float strength);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_impact_blood(float x, float y, float z, float angle_x, float angle_y, float angle_z, int LOD, float strength);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_lamp(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_ongoing_magic_protection(float x, float y, float z, int LOD, float scale);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_ongoing_shield(float x, float y, float z, int LOD, float scale);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_ongoing_magic_immunity(float x, float y, float z, int LOD, float scale);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_ongoing_poison(float x, float y, float z, int LOD, float scale);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_heal(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_heal2(actor* caster, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_magic_protection(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_magic_protection2(actor* caster, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_shield(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_shield2(actor* caster, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_restoration(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_restoration2(actor* caster, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_bones_to_gold(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_bones_to_gold2(actor* caster, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_teleport_to_the_portals_room(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_teleport_to_the_portals_room2(actor* caster, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_magic_immunity(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_magic_immunity2(actor* caster, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_alert(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_alert2(actor* caster, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_smoke(float x, float y, float z, float scale, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_rabbit(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_rat(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_beaver(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_deer(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_green_snake(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_red_snake(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_brown_snake(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_fox(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_boar(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_wolf(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_puma(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_bear(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_skeleton(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_small_gargoyle(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_medium_gargoyle(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_large_gargoyle(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_fluffy(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_chimeran_wolf(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_yeti(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_arctic_chimeran(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_giant(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_sword_serpent(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_sword_cutlass(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_sword_emerald_claymore(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_sword_sunbreaker(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_sword_orc_slayer(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_sword_eagle_wing(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_sword_jagged_saber(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_sword_of_fire(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_sword_of_ice(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_sword_of_magic(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_targetmagic_remote_heal(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, ec_obstructions obstructions, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_targetmagic_remote_heal2(actor* caster, actor* target, ec_obstructions obstructions, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_targetmagic_poison(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, ec_obstructions obstructions, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_targetmagic_poison2(actor* caster, actor* target, ec_obstructions obstructions, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_targetmagic_teleport_to_range(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, ec_obstructions obstructions, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_targetmagic_teleport_to_range2(actor* caster, actor* target, ec_obstructions obstructions, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_targetmagic_harm(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, ec_obstructions obstructions, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_targetmagic_harm2(actor* caster, actor* target, ec_obstructions obstructions, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_targetmagic_life_drain(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, ec_obstructions obstructions, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_targetmagic_life_drain2(actor* caster, actor* target, ec_obstructions obstructions, int LOD);
  EYE_CANDY_WRAPPER_API void ec_launch_targetmagic_heal_summoned(ec_reference reference, float start_x, float start_y, float start_z, ec_obstructions obstructions, int LOD);
  EYE_CANDY_WRAPPER_API void ec_launch_targetmagic_smite_summoned(ec_reference reference, float start_x, float start_y, float start_z, ec_obstructions obstructions, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_targetmagic_drain_mana(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, ec_obstructions obstructions, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_targetmagic_drain_mana2(actor* caster, actor* target, ec_obstructions obstructions, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_teleporter(float x, float y, float z, int LOD);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_wind_leaves(float x, float y, float z, ec_obstructions obstructions, float density, ec_bounds bounds, float prevailing_wind_x, float prevailing_wind_y, float prevailing_wind_z);
  EYE_CANDY_WRAPPER_API ec_reference ec_create_wind_petals(float x, float y, float z, ec_obstructions obstructions, float density, ec_bounds bounds, float prevailing_wind_x, float prevailing_wind_y, float prevailing_wind_z);
  EYE_CANDY_WRAPPER_API void ec_add_wind_effect_list(ec_reference reference, ec_effects effects);

#ifdef __cplusplus
}
#endif

#endif

//****************************************************************************//

#endif	// #ifdef SFX
