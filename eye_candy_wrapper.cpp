#ifdef SFX

#include "eye_candy_wrapper.h"
#include "cal3d_wrapper.h"

ec::EyeCandy eye_candy;
Uint64 ec_cur_time;
std::vector<ec_internal_reference*> references;

extern int use_point_particles; /*!< specifies if we use point particles or not */
extern int enable_blood; /*!< specifies whether or not to use the blood special effect in combat */
extern int poor_man; /*<! specifies whether we need to limit our resources */
extern int window_width;
extern int window_height;
extern float camera_x;
extern float camera_y;
extern float camera_z;

const float MAX_EFFECT_DISTANCE = 16.0;
const float WALK_RATE = 1.0;

ec_internal_obstructions null_obstructions;

extern "C" EYE_CANDY_WRAPPER_API void ec_init()
{
  eye_candy.load_textures("./textures/eye_candy/");
  ec_cur_time = ec::get_time();
  ec_set_draw_method();
}

extern "C" EYE_CANDY_WRAPPER_API void ec_add_light(GLenum light_id)
{
  eye_candy.add_light(light_id);
}

extern "C" EYE_CANDY_WRAPPER_API void ec_set_draw_method()
{
  if (use_point_particles)
    eye_candy.draw_method = ec::EyeCandy::POINT_SPRITES;
  else
    eye_candy.draw_method = ec::EyeCandy::FAST_BILLBOARDS;
}

extern "C" EYE_CANDY_WRAPPER_API void ec_set_draw_detail()
{
  if (poor_man)
    eye_candy.set_thresholds(3500, 20);	//Max particles, min framerate.
  else
    eye_candy.set_thresholds(15000, 20);
}

void ec_set_vec3_actor_bone(ec::Vec3& position, actor* _actor, int bone)
{
  float points[1024][3];

  CalSkeleton_GetBonePoints(CalModel_GetSkeleton(_actor->calmodel), &points[0][0]);

  position.x = points[bone][0];
  position.y = points[bone][1];
  position.z = points[bone][2];
}

extern "C" EYE_CANDY_WRAPPER_API void ec_idle()
{
//  GLfloat rot_matrix[16];
//  glGetFloatv(GL_MODELVIEW_MATRIX, rot_matrix);
//  const float x = rot_matrix[12];
//  const float y = rot_matrix[13];
//  const float z = rot_matrix[14];
  eye_candy.set_camera(ec::Vec3(-camera_x, -camera_z, camera_y));
  eye_candy.set_dimensions(window_width, window_height);
  Uint64 new_time = ec::get_time();
  for (int i = 0; i < (int)references.size(); )
  {
    std::vector<ec_internal_reference*>::iterator iter = references.begin() + i;
    if ((*iter)->dead)
    {
      delete *iter;
      references.erase(iter);
      continue;
    }
    
    if ((*iter)->caster)
      ec_set_vec3_actor_bone((*iter)->position, (*iter)->caster, 25);
    if ((*iter)->target)
      ec_set_vec3_actor_bone((*iter)->position2, (*iter)->target, 25);
    for (int j = 0; j < (int)(*iter)->target_actors.size(); j++)
      ec_set_vec3_actor_bone((*iter)->targets[j], (*iter)->target_actors[j], 25);
    i++;
  }
  eye_candy.time_diff = new_time - ec_cur_time;
  eye_candy.idle();
  ec_cur_time = new_time;
}

extern "C" EYE_CANDY_WRAPPER_API void ec_draw()
{
  glPushMatrix();
  glRotatef(90, 1.0, 0.0, 0.0);
  eye_candy.draw();
  glPopMatrix();
}

extern "C" EYE_CANDY_WRAPPER_API void ec_actor_delete(actor* _actor)
{
  for (int i = 0; i < (int)references.size(); )
  {
    std::vector<ec_internal_reference*>::iterator iter = references.begin() + i;
    if (((*iter)->caster == _actor) || ((*iter)->target == _actor))
    {
      (*iter)->effect->recall = true;
      delete *iter;
      references.erase(iter);
      continue;
    }
    for (int j = 0; j < (int)(*iter)->target_actors.size(); )
    {
      std::vector<actor*>::iterator iter2 = (*iter)->target_actors.begin() + j;
      if (*iter2 == _actor)
      {
        (*iter)->target_actors.erase(iter2);
        continue;
      }
      j++;
    }
    i++;
  }
}

extern "C" EYE_CANDY_WRAPPER_API void ec_recall_effect(const ec_reference ref)
{
  ec_internal_reference* cast_reference = (ec_internal_reference*)ref;
  cast_reference->effect->recall = true;
}

extern "C" EYE_CANDY_WRAPPER_API void ec_delete_all_effects()
{
  while (references.size())
  {
    std::vector<ec_internal_reference*>::iterator iter = references.begin();
    (*iter)->effect->recall = true;
    delete *iter;
    references.erase(iter);
  }
}

extern "C" EYE_CANDY_WRAPPER_API void ec_delete_effect_loc(float x, float y)
{
  for (int i = 0; i < (int)references.size(); )
  {
    std::vector<ec_internal_reference*>::iterator iter = references.begin() + i;
    if (((*iter)->position.x == x) && ((*iter)->position.z == -y))
    {
      (*iter)->effect->recall = true;
      delete *iter;
      references.erase(iter);
      continue;
    }
    i++;
  }
}

extern "C" EYE_CANDY_WRAPPER_API void ec_delete_effect_loc_type(float x, float y, ec_EffectEnum type)
{
  for (int i = 0; i < (int)references.size(); )
  {
    std::vector<ec_internal_reference*>::iterator iter = references.begin() + i;
    if (((*iter)->position.x == x) && ((*iter)->position.z == -y) && (type == (ec_EffectEnum)(*iter)->effect->get_type()))
    {
      (*iter)->effect->recall = true;
      delete *iter;
      references.erase(iter);
      continue;
    }
    i++;
  }
}

extern "C" EYE_CANDY_WRAPPER_API void ec_delete_effect_type(ec_EffectEnum type)
{
  for (int i = 0; i < (int)references.size(); )
  {
    std::vector<ec_internal_reference*>::iterator iter = references.begin() + i;
    if (type == (ec_EffectEnum)(*iter)->effect->get_type())
    {
      (*iter)->effect->recall = true;
      delete *iter;
      references.erase(iter);
      continue;
    }
    i++;
  }
}

extern "C" EYE_CANDY_WRAPPER_API void ec_delete_reference(ec_reference ref)
{
  ec_internal_reference* cast_reference = (ec_internal_reference*)ref;
  cast_reference->effect->recall = true;
  delete cast_reference;
}

extern "C" EYE_CANDY_WRAPPER_API void ec_set_position(ec_reference ref, float x, float y, float z)
{
  ((ec_internal_reference*)ref)->position = ec::Vec3(x, 0, -y);
}

extern "C" EYE_CANDY_WRAPPER_API void ec_set_position2(ec_reference ref, float x, float y, float z)
{
  ((ec_internal_reference*)ref)->position2 = ec::Vec3(x, 0, -y);
}

extern "C" EYE_CANDY_WRAPPER_API ec_obstructions ec_create_obstruction_list()
{
  return (ec_obstructions)(new ec_internal_obstructions);
}

extern "C" EYE_CANDY_WRAPPER_API void ec_free_obstruction_list(ec_obstructions obstructions)
{
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  for (std::vector<ec::Obstruction*>::iterator iter = cast_obstructions->obstructions.begin(); iter != cast_obstructions->obstructions.end(); iter++)
    delete *iter;
  delete cast_obstructions;
}

extern "C" EYE_CANDY_WRAPPER_API int ec_delete_obstruction(ec_obstructions obstructions, int index)
{
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  if (index >= (int)cast_obstructions->obstructions.size())
    return false;
  cast_obstructions->obstructions.erase(cast_obstructions->obstructions.begin() + index);
  cast_obstructions->positions.erase(cast_obstructions->positions.begin() + index);
  return true;
}

extern "C" EYE_CANDY_WRAPPER_API void ec_add_spherical_obstruction(ec_obstructions obstructions, float x, float y, float z, float max_distance, float force)
{
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  cast_obstructions->positions.push_back(ec::Vec3(x, z, -y));
  cast_obstructions->obstructions.push_back(new ec::SphereObstruction(&(*(cast_obstructions->positions.rbegin())), max_distance, force));
}

extern "C" EYE_CANDY_WRAPPER_API void ec_add_simple_cylindrical_obstruction(ec_obstructions obstructions, float x, float y, float max_distance, float force)
{
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  cast_obstructions->positions.push_back(ec::Vec3(x, 0, -y));
  cast_obstructions->obstructions.push_back(new ec::SimpleCylinderObstruction(&(*(cast_obstructions->positions.rbegin())), max_distance, force));
}

extern "C" EYE_CANDY_WRAPPER_API void ec_add_cylindrical_obstruction(ec_obstructions obstructions, float x1, float y1, float z1, float x2, float y2, float z2, float max_distance, float force)
{
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  cast_obstructions->positions.push_back(ec::Vec3(x1 + 0.25, z1, -(y1 + 0.25)));
  ec::Vec3* start = &(*(cast_obstructions->positions.rbegin()));
  cast_obstructions->positions.push_back(ec::Vec3(x2 + 0.25, z2, -(y2 + 0.25)));
  ec::Vec3* end = &(*(cast_obstructions->positions.rbegin()));
  cast_obstructions->obstructions.push_back(new ec::CylinderObstruction(start, end, max_distance, force));
}

extern "C" EYE_CANDY_WRAPPER_API ec_bounds ec_create_bounds_list()
{
  return (ec_bounds)(new ec_internal_bounds);
}

extern "C" EYE_CANDY_WRAPPER_API void ec_free_bounds_list(ec_bounds bounds)
{
  ec_internal_bounds* cast_bounds = (ec_internal_bounds*)bounds;
  delete cast_bounds;
}

extern "C" EYE_CANDY_WRAPPER_API void ec_add_polar_coords_bound(ec_bounds bounds, float frequency, float offset, float scalar, float power)
{
  ec_internal_bounds* cast_bounds = (ec_internal_bounds*)bounds;
  ec::PolarCoordElement e(frequency, offset, scalar, power);
  cast_bounds->push_back(e);
}

extern "C" EYE_CANDY_WRAPPER_API ec_effects ec_create_effects_list()
{
  return (ec_effects)(new ec_internal_effects);
}

extern "C" EYE_CANDY_WRAPPER_API void ec_free_effects_list(ec_effects effects)
{
  ec_internal_effects* cast_effects = (ec_internal_effects*)effects;
  delete cast_effects;
}

extern "C" EYE_CANDY_WRAPPER_API void ec_add_effect(ec_effects effects, ec_reference ref)
{
  ec_internal_effects* cast_effects = (ec_internal_effects*)effects;
  ec_internal_reference* cast_reference = (ec_internal_reference*)ref;
  cast_effects->push_back(cast_reference->effect);
}

extern "C" EYE_CANDY_WRAPPER_API int ec_in_range(float x, float y, float z, Uint64 effect_max_time)
{
  float dist_squared = (ec::Vec3(x, z, -y) - ec::Vec3(-camera_x, -camera_z, camera_y)).magnitude_squared();
  if (dist_squared < ec::square(MAX_EFFECT_DISTANCE + (effect_max_time * WALK_RATE) / 1000000.0))
  {
//    std::cout << "In range: " << dist_squared << std::endl;
    return 1;
  }
  else
  {
//    std::cout << "Not in range: " << dist_squared << std::endl;
    return 0;
  }
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_generic()
{
  references.push_back(new ec_internal_reference);
  return (ec_reference)(references[references.size() - 1]);
}

extern "C" EYE_CANDY_WRAPPER_API void ec_add_target(ec_reference reference, float x, float y, float z)
{
  ec_internal_reference* cast_reference = (ec_internal_reference*)reference;
  ec::Vec3 target(x, z, -y);
  cast_reference->targets.push_back(target);
}

extern "C" EYE_CANDY_WRAPPER_API int ec_change_target(ec_reference reference, int index, float x, float y, float z)
{
  ec_internal_reference* cast_reference = (ec_internal_reference*)reference;
  if (index >= (int)cast_reference->targets.size())
    return false;
  cast_reference->targets[index] = ec::Vec3(x, z, -y);
  return true;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_bag_pickup(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::BagEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::BagEffect(&eye_candy, &ret->dead, &ret->position, true, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_bag_drop(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::BagEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::BagEffect(&eye_candy, &ret->dead, &ret->position, false, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_breath_fire(float sx, float sy, float sz, float tx, float ty, float tz, ec_obstructions obstructions, int LOD, float scale)
{
  if (!ec_in_range(sx, sy, sz, ec::BreathEffect::get_max_end_time()))
    return NULL;
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  if (cast_obstructions == NULL)
    cast_obstructions = &null_obstructions;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(sx, sz, -(sy + 0.25));
  ret->position2 = ec::Vec3(tx, tz, -(ty + 0.25));
  ret->effect = new ec::BreathEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, cast_obstructions->obstructions, ec::BreathEffect::FIRE, LOD, scale);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_breath_ice(float sx, float sy, float sz, float tx, float ty, float tz, ec_obstructions obstructions, int LOD, float scale)
{
  if (!ec_in_range(sx, sy, sz, ec::BreathEffect::get_max_end_time()))
    return NULL;
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  if (cast_obstructions == NULL)
    cast_obstructions = &null_obstructions;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(sx, sz, -(sy + 0.25));
  ret->position2 = ec::Vec3(tx, tz, -(ty + 0.25));
  ret->effect = new ec::BreathEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, cast_obstructions->obstructions, ec::BreathEffect::ICE, LOD, scale);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_breath_poison(float sx, float sy, float sz, float tx, float ty, float tz, ec_obstructions obstructions, int LOD, float scale)
{
  if (!ec_in_range(sx, sy, sz, ec::BreathEffect::get_max_end_time()))
    return NULL;
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  if (cast_obstructions == NULL)
    cast_obstructions = &null_obstructions;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(sx, sz, -(sy + 0.25));
  ret->position2 = ec::Vec3(tx, tz, -(ty + 0.25));
  ret->effect = new ec::BreathEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, cast_obstructions->obstructions, ec::BreathEffect::POISON, LOD, scale);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_breath_magic(float sx, float sy, float sz, float tx, float ty, float tz, ec_obstructions obstructions, int LOD, float scale)
{
  if (!ec_in_range(sx, sy, sz, ec::BreathEffect::get_max_end_time()))
    return NULL;
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  if (cast_obstructions == NULL)
    cast_obstructions = &null_obstructions;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(sx, sz, -(sy + 0.25));
  ret->position2 = ec::Vec3(tx, tz, -(ty + 0.25));
  ret->effect = new ec::BreathEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, cast_obstructions->obstructions, ec::BreathEffect::MAGIC, LOD, scale);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_breath_lightning(float sx, float sy, float sz, float tx, float ty, float tz, ec_obstructions obstructions, int LOD, float scale)
{
  if (!ec_in_range(sx, sy, sz, ec::BreathEffect::get_max_end_time()))
    return NULL;
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  if (cast_obstructions == NULL)
    cast_obstructions = &null_obstructions;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(sx, sz, -(sy + 0.25));
  ret->position2 = ec::Vec3(tx, tz, -(ty + 0.25));
  ret->effect = new ec::BreathEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, cast_obstructions->obstructions, ec::BreathEffect::LIGHTNING, LOD, scale);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_breath_wind(float sx, float sy, float sz, float tx, float ty, float tz, ec_obstructions obstructions, int LOD, float scale)
{
  if (!ec_in_range(sx, sy, sz, ec::BreathEffect::get_max_end_time()))
    return NULL;
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  if (cast_obstructions == NULL)
    cast_obstructions = &null_obstructions;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(sx, sz, -(sy + 0.25));
  ret->position2 = ec::Vec3(tx, tz, -(ty + 0.25));
  ret->effect = new ec::BreathEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, cast_obstructions->obstructions, ec::BreathEffect::WIND, LOD, scale);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_campfire(float x, float y, float z, ec_obstructions obstructions, int LOD, float scale)
{
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  if (cast_obstructions == NULL)
    cast_obstructions = &null_obstructions;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::CampfireEffect(&eye_candy, &ret->dead, &ret->position, cast_obstructions->obstructions, scale, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_cloud(float x, float y, float z, float density, ec_bounds bounds, int LOD)
{
  ec_internal_bounds* cast_bounds = (ec_internal_bounds*)bounds;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::CloudEffect(&eye_candy, &ret->dead, &ret->position, density, *cast_bounds, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_fireflies(float x, float y, float z, ec_obstructions obstructions, float density, ec_bounds bounds)
{
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  if (cast_obstructions == NULL)
    cast_obstructions = &null_obstructions;
  ec_internal_bounds* cast_bounds = (ec_internal_bounds*)bounds;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::FireflyEffect(&eye_candy, &ret->dead, &ret->position, cast_obstructions->obstructions, density, *cast_bounds);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_fountain(float x, float y, float z, float base_height, int backlit, float scale, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::FountainEffect(&eye_candy, &ret->dead, &ret->position, base_height, backlit, scale, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_harvesting_radon_pouch(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::HarvestingEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::HarvestingEffect(&eye_candy, &ret->dead, &ret->position, ec::HarvestingEffect::RADON_POUCH, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_harvesting_cavern_wall(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::HarvestingEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::HarvestingEffect(&eye_candy, &ret->dead, &ret->position, ec::HarvestingEffect::CAVERN_WALL, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_harvesting_mother_nature(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::HarvestingEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::HarvestingEffect(&eye_candy, &ret->dead, &ret->position, ec::HarvestingEffect::MOTHER_NATURE, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_harvesting_queen_of_nature(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::HarvestingEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::HarvestingEffect(&eye_candy, &ret->dead, &ret->position, ec::HarvestingEffect::QUEEN_OF_NATURE, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_harvesting_bees(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::HarvestingEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::HarvestingEffect(&eye_candy, &ret->dead, &ret->position, ec::HarvestingEffect::BEES, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_harvesting_bag_of_gold(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::HarvestingEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::HarvestingEffect(&eye_candy, &ret->dead, &ret->position, ec::HarvestingEffect::BAG_OF_GOLD, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_harvesting_rare_stone(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::HarvestingEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::HarvestingEffect(&eye_candy, &ret->dead, &ret->position, ec::HarvestingEffect::RARE_STONE, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_impact_magic_protection(float x, float y, float z, float angle_x, float angle_y, float angle_z, int LOD, float strength)
{
  if (!ec_in_range(x, y, z, ec::ImpactEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ec::Vec3 angle_vec(angle_x, angle_z, -angle_y);
  ret->position -= angle_vec;
  angle_vec.normalize(0.4);
  ret->effect = new ec::ImpactEffect(&eye_candy, &ret->dead, &ret->position, angle_vec, ec::ImpactEffect::MAGIC_PROTECTION, LOD, strength);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_impact_shield(float x, float y, float z, float angle_x, float angle_y, float angle_z, int LOD, float strength)
{
  if (!ec_in_range(x, y, z, ec::ImpactEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ec::Vec3 angle_vec(angle_x, angle_z, -angle_y);
  ret->position -= angle_vec;
  angle_vec.normalize(0.4);
  ret->effect = new ec::ImpactEffect(&eye_candy, &ret->dead, &ret->position, angle_vec, ec::ImpactEffect::SHIELD, LOD, strength);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_impact_magic_immunity(float x, float y, float z, float angle_x, float angle_y, float angle_z, int LOD, float strength)
{
  if (!ec_in_range(x, y, z, ec::ImpactEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ec::Vec3 angle_vec(angle_x, angle_z, -angle_y);
  ret->position -= angle_vec;
  angle_vec.normalize(0.5);
  ret->effect = new ec::ImpactEffect(&eye_candy, &ret->dead, &ret->position, angle_vec, ec::ImpactEffect::MAGIC_IMMUNITY, LOD, strength);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_impact_poison(float x, float y, float z, float angle_x, float angle_y, float angle_z, int LOD, float strength)
{
  if (!ec_in_range(x, y, z, ec::ImpactEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ec::Vec3 angle_vec(angle_x, angle_z, -angle_y);
  ret->position -= angle_vec;
  angle_vec.normalize(0.1);
  ret->effect = new ec::ImpactEffect(&eye_candy, &ret->dead, &ret->position, angle_vec, ec::ImpactEffect::POISON, LOD, strength);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_impact_blood(float x, float y, float z, float angle_x, float angle_y, float angle_z, int LOD, float strength)
{
  if (!ec_in_range(x, y, z, ec::ImpactEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ec::Vec3 angle_vec(angle_x, angle_z, -angle_y);
  ret->position -= angle_vec;
  angle_vec.normalize(0.05);
  ret->effect = new ec::ImpactEffect(&eye_candy, &ret->dead, &ret->position, angle_vec, ec::ImpactEffect::BLOOD, LOD, strength);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_lamp(float x, float y, float z, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::LampEffect(&eye_candy, &ret->dead, &ret->position, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_ongoing_magic_protection(float x, float y, float z, int LOD, float scale)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::OngoingEffect(&eye_candy, &ret->dead, &ret->position, ec::OngoingEffect::MAGIC_PROTECTION, LOD, scale);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_ongoing_shield(float x, float y, float z, int LOD, float scale)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::OngoingEffect(&eye_candy, &ret->dead, &ret->position, ec::OngoingEffect::SHIELD, LOD, scale);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_ongoing_magic_immunity(float x, float y, float z, int LOD, float scale)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::OngoingEffect(&eye_candy, &ret->dead, &ret->position, ec::OngoingEffect::MAGIC_IMMUNITY, LOD, scale);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_ongoing_poison(float x, float y, float z, int LOD, float scale)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::OngoingEffect(&eye_candy, &ret->dead, &ret->position, ec::OngoingEffect::POISON, LOD, scale);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_heal(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::HEAL, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_heal2(actor* caster, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = caster;
  ret->position = ec::Vec3(caster->x_pos, caster->z_pos, -caster->y_pos);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::HEAL, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_magic_protection(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::MAGIC_PROTECTION, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_magic_protection2(actor* caster, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = caster;
  ret->position = ec::Vec3(caster->x_pos, caster->z_pos, -caster->y_pos);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::MAGIC_PROTECTION, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_shield(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::SHIELD, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_shield2(actor* caster, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = caster;
  ret->position = ec::Vec3(caster->x_pos, caster->z_pos, -caster->y_pos);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::SHIELD, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_restoration(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::RESTORATION, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_restoration2(actor* caster, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = caster;
  ret->position = ec::Vec3(caster->x_pos, caster->z_pos, -caster->y_pos);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::RESTORATION, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_bones_to_gold(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::BONES_TO_GOLD, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_bones_to_gold2(actor* caster, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = caster;
  ret->position = ec::Vec3(caster->x_pos, caster->z_pos, -caster->y_pos);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::BONES_TO_GOLD, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_teleport_to_the_portals_room(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::TELEPORT_TO_THE_PORTALS_ROOM, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_teleport_to_the_portals_room2(actor* caster, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = caster;
  ret->position = ec::Vec3(caster->x_pos, caster->z_pos, -caster->y_pos);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::TELEPORT_TO_THE_PORTALS_ROOM, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_magic_immunity(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::MAGIC_IMMUNITY, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_selfmagic_magic_immunity2(actor* caster, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = caster;
  ret->position = ec::Vec3(caster->x_pos, caster->z_pos, -caster->y_pos);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::MAGIC_IMMUNITY, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_alert(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::ALERT, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_alert2(actor* caster, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = caster;
  ret->position = ec::Vec3(caster->x_pos, caster->z_pos, -caster->y_pos);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::ALERT, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_smoke(float x, float y, float z, float scale, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SmokeEffect(&eye_candy, &ret->dead, &ret->position, scale, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_rabbit(float x, float y, float z, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::RABBIT, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_rat(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::RAT, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_beaver(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::BEAVER, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_deer(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::DEER, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_green_snake(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::GREEN_SNAKE, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_red_snake(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::RED_SNAKE, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_brown_snake(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::BROWN_SNAKE, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_fox(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::FOX, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_boar(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::BOAR, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_wolf(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::WOLF, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_puma(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::PUMA, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_bear(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::BEAR, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_skeleton(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::SKELETON, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_small_gargoyle(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::SMALL_GARGOYLE, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_medium_gargoyle(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::MEDIUM_GARGOYLE, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_large_gargoyle(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::LARGE_GARGOYLE, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_fluffy(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::FLUFFY, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_chimeran_wolf(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::CHIMERAN_WOLF, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_yeti(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::YETI, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_arctic_chimeran(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::ARCTIC_CHIMERAN, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_summon_giant(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::GIANT, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_sword_serpent(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(start_x, start_z, -start_y);
  ret->position2 = ec::Vec3(end_x, end_z, -end_y);
  ret->effect = new ec::SwordEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::SwordEffect::SERPENT, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_sword_cutlass(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(start_x, start_z, -start_y);
  ret->position2 = ec::Vec3(end_x, end_z, -end_y);
  ret->effect = new ec::SwordEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::SwordEffect::CUTLASS, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_sword_emerald_claymore(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(start_x, start_z, -start_y);
  ret->position2 = ec::Vec3(end_x, end_z, -end_y);
  ret->effect = new ec::SwordEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::SwordEffect::EMERALD_CLAYMORE, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_sword_sunbreaker(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(start_x, start_z, -start_y);
  ret->position2 = ec::Vec3(end_x, end_z, -end_y);
  ret->effect = new ec::SwordEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::SwordEffect::SUNBREAKER, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_sword_orc_slayer(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(start_x, start_z, -start_y);
  ret->position2 = ec::Vec3(end_x, end_z, -end_y);
  ret->effect = new ec::SwordEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::SwordEffect::ORC_SLAYER, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_sword_eagle_wing(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(start_x, start_z, -start_y);
  ret->position2 = ec::Vec3(end_x, end_z, -end_y);
  ret->effect = new ec::SwordEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::SwordEffect::EAGLE_WING, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_sword_jagged_saber(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(start_x, start_z, -start_y);
  ret->position2 = ec::Vec3(end_x, end_z, -end_y);
  ret->effect = new ec::SwordEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::SwordEffect::JAGGED_SABER, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_sword_of_fire(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(start_x, start_z, -start_y);
  ret->position2 = ec::Vec3(end_x, end_z, -end_y);
  ret->effect = new ec::SwordEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::SwordEffect::SWORD_OF_FIRE, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_sword_of_ice(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(start_x, start_z, -start_y);
  ret->position2 = ec::Vec3(end_x, end_z, -end_y);
  ret->effect = new ec::SwordEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::SwordEffect::SWORD_OF_ICE, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_sword_of_magic(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(start_x, start_z, -start_y);
  ret->position2 = ec::Vec3(end_x, end_z, -end_y);
  ret->effect = new ec::SwordEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::SwordEffect::SWORD_OF_MAGIC, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_targetmagic_remote_heal(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, ec_obstructions obstructions, int LOD)
{
  if (!ec_in_range(start_x, start_y, start_z, ec::TargetMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  if (cast_obstructions == NULL)
    cast_obstructions = &null_obstructions;
  ret->position = ec::Vec3(start_x, start_z, -start_y);
  ret->position2 = ec::Vec3(end_x, end_z, -end_y);
  ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::REMOTE_HEAL, cast_obstructions->obstructions, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_targetmagic_remote_heal2(actor* caster, actor* target, ec_obstructions obstructions, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::TargetMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  if (cast_obstructions == NULL)
    cast_obstructions = &null_obstructions;
  ret->caster = caster;
  ret->target = target;
  ec_set_vec3_actor_bone(ret->position, caster, 25);
  ret->position2 = ec::Vec3(target->x_pos, target->z_pos, -target->y_pos);
  ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::REMOTE_HEAL, cast_obstructions->obstructions, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_targetmagic_poison(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, ec_obstructions obstructions, int LOD)
{
  if (!ec_in_range(start_x, start_y, start_z, ec::TargetMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  if (cast_obstructions == NULL)
    cast_obstructions = &null_obstructions;
  ret->position = ec::Vec3(start_x, start_z, -start_y);
  ret->position2 = ec::Vec3(end_x, end_z, -end_y);
  ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::POISON, cast_obstructions->obstructions, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_targetmagic_poison2(actor* caster, actor* target, ec_obstructions obstructions, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::TargetMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  if (cast_obstructions == NULL)
    cast_obstructions = &null_obstructions;
  ret->caster = caster;
  ret->target = target;
  ec_set_vec3_actor_bone(ret->position, caster, 25);
  ret->position2 = ec::Vec3(target->x_pos, target->z_pos, -target->y_pos);
  ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::POISON, cast_obstructions->obstructions, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_targetmagic_teleport_to_range(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, ec_obstructions obstructions, int LOD)
{
  if (!ec_in_range(start_x, start_y, start_z, ec::TargetMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  if (cast_obstructions == NULL)
    cast_obstructions = &null_obstructions;
  ret->position = ec::Vec3(start_x, start_z, -start_y);
  ret->position2 = ec::Vec3(end_x, end_z, -end_y);
  ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::TELEPORT_TO_RANGE, cast_obstructions->obstructions, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_targetmagic_harm(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, ec_obstructions obstructions, int LOD)
{
  if (!ec_in_range(start_x, start_y, start_z, ec::TargetMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  if (cast_obstructions == NULL)
    cast_obstructions = &null_obstructions;
  ret->position = ec::Vec3(start_x, start_z, -start_y);
  ret->position2 = ec::Vec3(end_x, end_z, -end_y);
  ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::HARM, cast_obstructions->obstructions, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_targetmagic_harm2(actor* caster, actor* target, ec_obstructions obstructions, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::TargetMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  if (cast_obstructions == NULL)
    cast_obstructions = &null_obstructions;
  ret->caster = caster;
  ret->target = target;
  ec_set_vec3_actor_bone(ret->position, caster, 25);
  ret->position2 = ec::Vec3(target->x_pos, target->z_pos, -target->y_pos);
  ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::HARM, cast_obstructions->obstructions, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_targetmagic_life_drain(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, ec_obstructions obstructions, int LOD)
{
  if (!ec_in_range(start_x, start_y, start_z, ec::TargetMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  if (cast_obstructions == NULL)
    cast_obstructions = &null_obstructions;
  ret->position = ec::Vec3(start_x, start_z, -start_y);
  ret->position2 = ec::Vec3(end_x, end_z, -end_y);
  ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::LIFE_DRAIN, cast_obstructions->obstructions, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_targetmagic_life_drain2(actor* caster, actor* target, ec_obstructions obstructions, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::TargetMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  if (cast_obstructions == NULL)
    cast_obstructions = &null_obstructions;
  ret->caster = caster;
  ret->target = target;
  ec_set_vec3_actor_bone(ret->position, caster, 25);
  ret->position2 = ec::Vec3(target->x_pos, target->z_pos, -target->y_pos);
  ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::LIFE_DRAIN, cast_obstructions->obstructions, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API void ec_launch_targetmagic_heal_summoned(ec_reference reference, float start_x, float start_y, float start_z, ec_obstructions obstructions, int LOD)
{
  ec_internal_reference* cast_reference = (ec_internal_reference*)reference;
  if (!ec_in_range(start_x, start_y, start_z, ec::TargetMagicEffect::get_max_end_time()))
  {
    delete cast_reference;
    return;
  }
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  if (cast_obstructions == NULL)
    cast_obstructions = &null_obstructions;
  cast_reference->position = ec::Vec3(start_x, start_z, -start_y);
  std::vector<ec::Vec3*> target_ptrs;
  for (std::vector<ec::Vec3>::iterator iter = cast_reference->targets.begin(); iter != cast_reference->targets.end(); iter++)
    target_ptrs.push_back(&(*iter));
  cast_reference->effect = new ec::TargetMagicEffect(&eye_candy, &cast_reference->dead, &cast_reference->position, target_ptrs, ec::TargetMagicEffect::HEAL_SUMMONED, cast_obstructions->obstructions, LOD);
  eye_candy.push_back_effect(cast_reference->effect);
}

extern "C" EYE_CANDY_WRAPPER_API void ec_launch_targetmagic_smite_summoned(ec_reference reference, float start_x, float start_y, float start_z, ec_obstructions obstructions, int LOD)
{
  ec_internal_reference* cast_reference = (ec_internal_reference*)reference;
  if (!ec_in_range(start_x, start_y, start_z, ec::TargetMagicEffect::get_max_end_time()))
  {
    delete cast_reference;
    return;
  }
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  if (cast_obstructions == NULL)
    cast_obstructions = &null_obstructions;
  cast_reference->position = ec::Vec3(start_x, start_z, -start_y);
  std::vector<ec::Vec3*> target_ptrs;
  for (std::vector<ec::Vec3>::iterator iter = cast_reference->targets.begin(); iter != cast_reference->targets.end(); iter++)
    target_ptrs.push_back(&(*iter));
  cast_reference->effect = new ec::TargetMagicEffect(&eye_candy, &cast_reference->dead, &cast_reference->position, target_ptrs, ec::TargetMagicEffect::SMITE_SUMMONED, cast_obstructions->obstructions, LOD);
  eye_candy.push_back_effect(cast_reference->effect);
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_targetmagic_drain_mana(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, ec_obstructions obstructions, int LOD)
{
  if (!ec_in_range(start_x, start_y, start_z, ec::TargetMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  if (cast_obstructions == NULL)
    cast_obstructions = &null_obstructions;
  ret->position = ec::Vec3(start_x, start_z, -start_y);
  ret->position2 = ec::Vec3(end_x, end_z, -end_y);
  ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::DRAIN_MANA, cast_obstructions->obstructions, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_targetmagic_drain_mana2(actor* caster, actor* target, ec_obstructions obstructions, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::TargetMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  if (cast_obstructions == NULL)
    cast_obstructions = &null_obstructions;
  ret->caster = caster;
  ret->target = target;
  ec_set_vec3_actor_bone(ret->position, caster, 25);
  ret->position2 = ec::Vec3(target->x_pos, target->z_pos, -target->y_pos);
  ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::DRAIN_MANA, cast_obstructions->obstructions, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_teleporter(float x, float y, float z, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::TeleporterEffect(&eye_candy, &ret->dead, &ret->position, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_wind_leaves(float x, float y, float z, ec_obstructions obstructions, float density, ec_bounds bounds, float prevailing_wind_x, float prevailing_wind_y, float prevailing_wind_z)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  if (cast_obstructions == NULL)
    cast_obstructions = &null_obstructions;
  ec_internal_bounds* cast_bounds = (ec_internal_bounds*)bounds;
  ret->position = ec::Vec3(x, z, -y);
  ret->position2 = ec::Vec3(prevailing_wind_x, prevailing_wind_z, -(prevailing_wind_y + 0.25));
  ret->effect = new ec::WindEffect(&eye_candy, &ret->dead, &ret->position, cast_obstructions->obstructions, density, *cast_bounds, ec::WindEffect::LEAVES, ret->position2);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API ec_reference ec_create_wind_petals(float x, float y, float z, ec_obstructions obstructions, float density, ec_bounds bounds, float prevailing_wind_x, float prevailing_wind_y, float prevailing_wind_z)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ec_internal_obstructions* cast_obstructions = (ec_internal_obstructions*)obstructions;
  if (cast_obstructions == NULL)
    cast_obstructions = &null_obstructions;
  ec_internal_bounds* cast_bounds = (ec_internal_bounds*)bounds;
  ret->position = ec::Vec3(x, z, -y);
  ret->position2 = ec::Vec3(prevailing_wind_x, prevailing_wind_z, -(prevailing_wind_y + 0.25));
  ret->effect = new ec::WindEffect(&eye_candy, &ret->dead, &ret->position, cast_obstructions->obstructions, density, *cast_bounds, ec::WindEffect::FLOWER_PETALS, ret->position2);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" EYE_CANDY_WRAPPER_API void ec_add_wind_effect_list(ec_reference reference, ec_effects effects)
{
  ec_internal_reference* cast_reference = (ec_internal_reference*)reference;
  ec_internal_effects* cast_effects = (ec_internal_effects*)effects;
  ((ec::WindEffect*)(cast_reference->effect))->set_pass_off(*cast_effects);
}

#endif	// #ifdef SFX
