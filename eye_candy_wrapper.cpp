#ifdef EYE_CANDY

#include "eye_candy_wrapper.h"
#include "cal3d_wrapper.h"
#include "draw_scene.h"
#include "gl_init.h"
#include "particles.h"
#include "init.h"
#include "gamewin.h"

ec::EyeCandy eye_candy;
Uint64 ec_cur_time, ec_last_time;
std::vector<ec_internal_reference*> references;

const float MAX_EFFECT_DISTANCE = 20.0;
const float MAX_OBSTRUCT_DISTANCE_SQUARED = 90.0;
const float OBSTRUCTION_FORCE = 2.0;
const float WALK_RATE = 1.0;
const float SWORD_LENGTH = 0.4;

ec_object_obstructions object_obstructions;
ec_actor_obstructions actor_obstructions;
ec_actor_obstruction self_actor;
std::vector<ec::Obstruction*> general_obstructions_list;
std::vector<ec::Obstruction*> fire_obstructions_list;

extern "C" void ec_init()
{
  eye_candy.load_textures("./textures/eye_candy/");
  ec_last_time = 0;
  ec_cur_time = 0;
  ec_set_draw_method();
  //TODO: Free this when the program quits.  Not a big deal if it doesn't
  //happen, but it'd be proper to do so.
  self_actor.obstruction = new ec::SimpleCylinderObstruction(&(self_actor.center), 0.6, 3.0);
}

extern "C" void ec_add_light(GLenum light_id)
{
  eye_candy.add_light(light_id);
}

extern "C" void ec_set_draw_method()
{
  if (use_point_particles)
    eye_candy.draw_method = ec::EyeCandy::POINT_SPRITES;
  else
    eye_candy.draw_method = ec::EyeCandy::FAST_BILLBOARDS;
}

extern "C" void ec_set_draw_detail()
{
  if (poor_man)
    eye_candy.set_thresholds(3500, 10);	//Max particles, min framerate.
  else
    eye_candy.set_thresholds(15000, 10);
}

void set_vec3_actor_bone(ec::Vec3& position, actor* _actor, int bone)
{
  float points[1024][3];

  CalSkeleton_GetBonePoints(CalModel_GetSkeleton(_actor->calmodel), &points[0][0]);

  ec::Vec3 unrotated_position;
  unrotated_position.x = points[bone][0];
  unrotated_position.y = points[bone][1];
  unrotated_position.z = points[bone][2];
  
  const float s_rx = sin(_actor->x_rot * (ec::PI / 180));
  const float c_rx = cos(_actor->x_rot * (ec::PI / 180));
  const float s_ry = sin(_actor->y_rot * (ec::PI / 180));
  const float c_ry = cos(_actor->y_rot * (ec::PI / 180));
  const float s_rz = sin((180 -_actor->z_rot) * (ec::PI / 180));
  const float c_rz = cos((180 -_actor->z_rot) * (ec::PI / 180));
  
  ec::Vec3 rotz_position;
  rotz_position.x = unrotated_position.x * c_rz - unrotated_position.y * s_rz;
  rotz_position.y = unrotated_position.x * s_rz + unrotated_position.y * c_rz;
  rotz_position.z = unrotated_position.z;
  
  ec::Vec3 rotx_position;
  rotx_position.x = rotz_position.x;
  rotx_position.y = rotz_position.y * c_rx - rotz_position.z * s_rx;
  rotx_position.z = rotz_position.y * s_rx + rotz_position.z * c_rx;

  ec::Vec3 roty_position;
  roty_position.x = rotx_position.z * s_ry + rotx_position.x * c_ry;
  roty_position.y = rotx_position.y;
  roty_position.z = rotx_position.z * c_ry - rotx_position.x * s_ry;

  position.x = roty_position.x + _actor->x_pos + 0.25;
  position.y = roty_position.z + _actor->z_pos;
  position.z = -(roty_position.y + _actor->y_pos + 0.25);
}

extern "C" void get_sword_positions(actor* _actor, ec::Vec3& base, ec::Vec3& tip)
{
  ec::Vec3 arm_pos;
  
  set_vec3_actor_bone(arm_pos, _actor, 24);
  set_vec3_actor_bone(base, _actor, 25);
  
  ec::Vec3 sword_angle = (base - arm_pos).normalize();
  tip = base + sword_angle * SWORD_LENGTH;
}

extern "C" void ec_idle()
{
//  GLfloat rot_matrix[16];
//  glGetFloatv(GL_MODELVIEW_MATRIX, rot_matrix);
//  const float x = rot_matrix[12];
//  const float y = rot_matrix[13];
//  const float z = rot_matrix[14];
  
  const float s_rx = sin(rx * ec::PI / 180);
  const float c_rx = cos(rx * ec::PI / 180);
  const float s_rz = sin(rz * ec::PI / 180);
  const float c_rz = cos(rz * ec::PI / 180);
  float new_camera_x = zoom_level*camera_distance * s_rx * s_rz + camera_x;
  float new_camera_y = -zoom_level*camera_distance * s_rx * c_rz + camera_y;
  float new_camera_z = zoom_level*camera_distance * c_rx + camera_z;
  eye_candy.set_camera(ec::Vec3(-new_camera_x, -new_camera_z, new_camera_y));
  
  eye_candy.set_dimensions(window_width, window_height, zoom_level);
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
    {
      if ((*iter)->effect->get_type() == ec::EC_SWORD)
        get_sword_positions((*iter)->caster, (*iter)->position, (*iter)->position2);
      else
        set_vec3_actor_bone((*iter)->position, (*iter)->caster, 25);
    }
    if ((*iter)->target)
      set_vec3_actor_bone((*iter)->position2, (*iter)->target, 25);
    for (int j = 0; j < (int)(*iter)->target_actors.size(); j++)
    {
      if ((*iter)->target_actors[j])
        set_vec3_actor_bone((*iter)->targets[j], (*iter)->target_actors[j], 25);
    }
    i++;
  }
  
  if (ec_cur_time == 0)
    eye_candy.time_diff = 100000;
  else
    eye_candy.time_diff = new_time - ec_cur_time;
  eye_candy.framerate = 1000000.0 / (eye_candy.time_diff + 1);
//  eye_candy.framerate = fps_average;
  ec_last_time = ec_cur_time;
  ec_cur_time = new_time;

  if (ec_last_time % 1000000 >= ec_cur_time % 1000000)
    ec_heartbeat();

  eye_candy.idle();
}

extern "C" void ec_heartbeat()
{
//  std::cout << "Actor: <" << camera_x << ", " << camera_z << ", " << -camera_y << ">" << std::endl;
//  if (!((int)(ec_cur_time / 1000000.0) % 9))
//    ec_create_breath_fire(44.75, 38.0, 1.0, 44.75, 43.0, 0.6, 2, 1.5);
  general_obstructions_list.clear();
  fire_obstructions_list.clear();
  for (ec_object_obstructions::iterator iter = object_obstructions.begin(); iter != object_obstructions.end(); iter++)
  {
    (*iter)->center.x = (*iter)->obj3d->x_pos;
    (*iter)->center.y = (*iter)->obj3d->z_pos;
    (*iter)->center.z = -(*iter)->obj3d->y_pos;
    const float dist_squared = ((*iter)->center - ec::Vec3(-camera_x, -camera_z, camera_y)).magnitude_squared();
    if (dist_squared > MAX_OBSTRUCT_DISTANCE_SQUARED)
      continue;
    (*iter)->center.x += 0.25;
    (*iter)->center.y += 0.25;
    (*iter)->center.z -= 0.25;
    (*iter)->sin_rot_x = sin((*iter)->obj3d->x_rot * (ec::PI / 180));
    (*iter)->cos_rot_x = cos((*iter)->obj3d->x_rot * (ec::PI / 180));
    (*iter)->sin_rot_y = sin((*iter)->obj3d->z_rot * (ec::PI / 180));
    (*iter)->cos_rot_y = cos((*iter)->obj3d->z_rot * (ec::PI / 180));
    (*iter)->sin_rot_z = sin(-((*iter)->obj3d->y_rot * (ec::PI / 180)));
    (*iter)->cos_rot_z = cos(-((*iter)->obj3d->y_rot * (ec::PI / 180)));
    (*iter)->sin_rot_x2 = sin(-(*iter)->obj3d->x_rot * (ec::PI / 180));
    (*iter)->cos_rot_x2 = cos(-(*iter)->obj3d->x_rot * (ec::PI / 180));
    (*iter)->sin_rot_y2 = sin(-(*iter)->obj3d->z_rot * (ec::PI / 180));
    (*iter)->cos_rot_y2 = cos(-(*iter)->obj3d->z_rot * (ec::PI / 180));
    (*iter)->sin_rot_z2 = sin(((*iter)->obj3d->y_rot * (ec::PI / 180)));
    (*iter)->cos_rot_z2 = cos(((*iter)->obj3d->y_rot * (ec::PI / 180)));
    general_obstructions_list.push_back((*iter)->obstruction);
    if ((*iter)->fire_related)
      fire_obstructions_list.push_back((*iter)->obstruction);
  }
  for (ec_actor_obstructions::iterator iter = actor_obstructions.begin(); iter != actor_obstructions.end(); iter++)
  {
    (*iter)->center.x = (*iter)->obstructing_actor->x_pos;
    (*iter)->center.y = (*iter)->obstructing_actor->z_pos;
    (*iter)->center.z = -(*iter)->obstructing_actor->y_pos;
    const float dist_squared = ((*iter)->center - ec::Vec3(-camera_x, -camera_z, camera_y)).magnitude_squared();
    if (dist_squared > MAX_OBSTRUCT_DISTANCE_SQUARED)
      continue;
    (*iter)->center.x += 0.25;
    (*iter)->center.y += 0.25;
    (*iter)->center.z -= 0.25;
    general_obstructions_list.push_back((*iter)->obstruction);
  }
  // Last but not least... the actor.
  self_actor.center.x = -camera_x;
  self_actor.center.y = -camera_z;
  self_actor.center.z = camera_y;
  general_obstructions_list.push_back(self_actor.obstruction);
}

extern "C" void ec_draw()
{
  glPushMatrix();
  glRotatef(90, 1.0, 0.0, 0.0);
  eye_candy.draw();
  glPopMatrix();
}

extern "C" void ec_actor_delete(actor* _actor)
{
  for (int i = 0; i < (int)references.size(); )
  {
    std::vector<ec_internal_reference*>::iterator iter = references.begin() + i;
    if ((*iter)->dead)
    {
      delete *iter;
      references.erase(iter);
      continue;
    }

    i++;
    if (((*iter)->caster == _actor) || ((*iter)->target == _actor))
    {
      (*iter)->effect->recall = true;
      (*iter)->caster = NULL;
      (*iter)->target = NULL;
      continue;
    }
    for (int j = 0; j < (int)(*iter)->target_actors.size(); j++)
    {
      std::vector<actor*>::iterator iter2 = (*iter)->target_actors.begin() + j;
      if (*iter2 == _actor)
        (*iter2) = NULL;
    }
  }
  for (ec_actor_obstructions::iterator iter = actor_obstructions.begin(); iter != actor_obstructions.end(); iter++)
  {
    if ((*iter)->obstructing_actor == _actor)
    {
      for (std::vector<ec::Obstruction*>::iterator iter2 = general_obstructions_list.begin(); iter2 != general_obstructions_list.end(); iter2++)
      {
        if (*iter2 == (*iter)->obstruction)
        {
          general_obstructions_list.erase(iter2);
          break;
        }
      }
      delete (*iter)->obstruction;
      delete *iter;
      actor_obstructions.erase(iter);
      break;
    }
  }
}

extern "C" void ec_recall_effect(const ec_reference ref)
{
  ec_internal_reference* cast_reference = (ec_internal_reference*)ref;
  cast_reference->effect->recall = true;
}

extern "C" void ec_delete_all_effects()
{
  for (int i = 0; i < (int)references.size(); )
  {
    std::vector<ec_internal_reference*>::iterator iter = references.begin() + i;
    if ((*iter)->dead)
    {
      delete *iter;
      references.erase(iter);
      continue;
    }

    (*iter)->effect->recall = true;
    i++;
  }
}

extern "C" void ec_delete_effect_loc(float x, float y)
{
  for (int i = 0; i < (int)references.size(); )
  {
    std::vector<ec_internal_reference*>::iterator iter = references.begin() + i;
    if ((*iter)->dead)
    {
      delete *iter;
      references.erase(iter);
      continue;
    }

    i++;
    if (((*iter)->position.x == x) && ((*iter)->position.z == -y))
    {
      (*iter)->effect->recall = true;
      continue;
    }
  }
}

extern "C" void ec_delete_effect_loc_type(float x, float y, ec_EffectEnum type)
{
  for (int i = 0; i < (int)references.size(); )
  {
    std::vector<ec_internal_reference*>::iterator iter = references.begin() + i;
    if ((*iter)->dead)
    {
      delete *iter;
      references.erase(iter);
      continue;
    }

    i++;
    if (((*iter)->position.x == x) && ((*iter)->position.z == -y) && (type == (ec_EffectEnum)(*iter)->effect->get_type()))
    {
      (*iter)->effect->recall = true;
      continue;
    }
  }
}

extern "C" void ec_delete_effect_type(ec_EffectEnum type)
{
  for (int i = 0; i < (int)references.size(); )
  {
    std::vector<ec_internal_reference*>::iterator iter = references.begin() + i;
    if ((*iter)->dead)
    {
      delete *iter;
      references.erase(iter);
      continue;
    }

    i++;
    if (type == (ec_EffectEnum)(*iter)->effect->get_type())
    {
      (*iter)->effect->recall = true;
      continue;
    }
  }
}

extern "C" void ec_delete_reference(ec_reference ref)
{
  ec_internal_reference* cast_reference = (ec_internal_reference*)ref;
  delete cast_reference;
}

extern "C" void ec_set_position(ec_reference ref, float x, float y, float z)
{
  ((ec_internal_reference*)ref)->position = ec::Vec3(x, 0, -y);
}

extern "C" void ec_set_position2(ec_reference ref, float x, float y, float z)
{
  ((ec_internal_reference*)ref)->position2 = ec::Vec3(x, 0, -y);
}

extern "C" void ec_clear_obstruction_list()
{
  for (ec_object_obstructions::iterator iter = object_obstructions.begin(); iter != object_obstructions.end(); iter++)
  {
    delete (*iter)->obstruction;
    delete *iter;
  }
  object_obstructions.clear();
  for (ec_actor_obstructions::iterator iter = actor_obstructions.begin(); iter != actor_obstructions.end(); iter++)
  {
    delete (*iter)->obstruction;
    delete *iter;
  }
  actor_obstructions.clear();
  general_obstructions_list.clear();
  fire_obstructions_list.clear();
}

extern "C" void ec_add_object_obstruction(object3d* obj3d, e3d_object *e3dobj, float force)
{
  // First, verify that this object isn't "flat"
  if ((e3dobj->min_x == e3dobj->max_x) || (e3dobj->min_y == e3dobj->max_y) || (e3dobj->min_z == e3dobj->max_z))
    return;

  ec_object_obstruction* obstruction = new ec_object_obstruction;
  obstruction->obj3d = obj3d;
  obstruction->e3dobj = e3dobj;
  obstruction->center = ec::Vec3(obj3d->x_pos + 0.25, obj3d->z_pos, -(obj3d->y_pos + 0.25));
  obstruction->sin_rot_x = sin(obj3d->x_rot * (ec::PI / 180));
  obstruction->cos_rot_x = cos(obj3d->x_rot * (ec::PI / 180));
  obstruction->sin_rot_y = sin(obj3d->z_rot * (ec::PI / 180));
  obstruction->cos_rot_y = cos(obj3d->z_rot * (ec::PI / 180));
  obstruction->sin_rot_z = sin(-(obj3d->y_rot * (ec::PI / 180)));
  obstruction->cos_rot_z = cos(-(obj3d->y_rot * (ec::PI / 180)));
  obstruction->sin_rot_x2 = sin(-obj3d->x_rot * (ec::PI / 180));
  obstruction->cos_rot_x2 = cos(-obj3d->x_rot * (ec::PI / 180));
  obstruction->sin_rot_y2 = sin(-obj3d->z_rot * (ec::PI / 180));
  obstruction->cos_rot_y2 = cos(-obj3d->z_rot * (ec::PI / 180));
  obstruction->sin_rot_z2 = sin((obj3d->y_rot * (ec::PI / 180)));
  obstruction->cos_rot_z2 = cos((obj3d->y_rot * (ec::PI / 180)));
  obstruction->fire_related = false;
  if (!strncmp(obj3d->file_name + 2, "misc_ob", 7))
  {
    if (!strncmp(obj3d->file_name + 15, "cook", 4) ||
        !strncmp(obj3d->file_name + 15, "pot_", 4) ||
        !strncmp(obj3d->file_name + 15, "cauld", 5))
      obstruction->fire_related = true;
  }
  else if (!strncmp(obj3d->file_name + 2, "struc", 5))
  {
    if (!strncmp(obj3d->file_name + 13, "chimn", 5) ||
        !strncmp(obj3d->file_name + 13, "forge", 5))
      obstruction->fire_related = true;
  }
  else if (!strncmp(obj3d->file_name + 2, "trees/fire", 10))
    obstruction->fire_related = true;
  obstruction->obstruction = new ec::BoxObstruction(ec::Vec3(e3dobj->min_x, e3dobj->min_z, -e3dobj->max_y), ec::Vec3(e3dobj->max_x, e3dobj->max_z, -e3dobj->min_y), &(obstruction->center), &(obstruction->sin_rot_x), &(obstruction->cos_rot_x), &(obstruction->sin_rot_y), &(obstruction->cos_rot_y), &(obstruction->sin_rot_z), &(obstruction->cos_rot_z), &(obstruction->sin_rot_x2), &(obstruction->cos_rot_x2), &(obstruction->sin_rot_y2), &(obstruction->cos_rot_y2), &(obstruction->sin_rot_z2), &(obstruction->cos_rot_z2), force);
  object_obstructions.push_back(obstruction);
}

extern "C" void ec_add_actor_obstruction(actor* _actor, float force)
{
  ec_actor_obstruction* obstruction = new ec_actor_obstruction;
  obstruction->obstructing_actor = _actor;
  obstruction->center = ec::Vec3(_actor->x_pos + 0.25, _actor->z_pos, -(_actor->y_pos + 0.25));
  obstruction->obstruction = new ec::SimpleCylinderObstruction(&(obstruction->center), 0.85, force);
  actor_obstructions.push_back(obstruction);
}

extern "C" void ec_remove_obstruction_by_object3d(object3d* obj3d)
{
  for (ec_object_obstructions::iterator iter = object_obstructions.begin(); iter != object_obstructions.end(); iter++)
  {
    if ((*iter)->obj3d == obj3d)
    {
      for (std::vector<ec::Obstruction*>::iterator iter2 = general_obstructions_list.begin(); iter2 != general_obstructions_list.end(); iter2++)
      {
        if (*iter2 == (*iter)->obstruction)
        {
          general_obstructions_list.erase(iter2);
          break;
        }
      }
      for (std::vector<ec::Obstruction*>::iterator iter2 = fire_obstructions_list.begin(); iter2 != fire_obstructions_list.end(); iter2++)
      {
        if (*iter2 == (*iter)->obstruction)
        {
          fire_obstructions_list.erase(iter2);
          break;
        }
      }
      delete (*iter)->obstruction;
      delete *iter;
      object_obstructions.erase(iter);
      return;
    }
  }
}

extern "C" void ec_remove_obstruction_by_e3d_object(e3d_object* e3dobj)
{
  for (ec_object_obstructions::iterator iter = object_obstructions.begin(); iter != object_obstructions.end(); iter++)
  {
    if ((*iter)->e3dobj == e3dobj)
    {
      for (std::vector<ec::Obstruction*>::iterator iter2 = general_obstructions_list.begin(); iter2 != general_obstructions_list.end(); iter2++)
      {
        if (*iter2 == (*iter)->obstruction)
        {
          general_obstructions_list.erase(iter2);
          break;
        }
      }
      for (std::vector<ec::Obstruction*>::iterator iter2 = fire_obstructions_list.begin(); iter2 != fire_obstructions_list.end(); iter2++)
      {
        if (*iter2 == (*iter)->obstruction)
        {
          fire_obstructions_list.erase(iter2);
          break;
        }
      }
      delete (*iter)->obstruction;
      delete *iter;
      object_obstructions.erase(iter);
      return;
    }
  }
}

extern "C" ec_bounds ec_create_bounds_list()
{
  return (ec_bounds)(new ec_internal_bounds);
}

extern "C" void ec_free_bounds_list(ec_bounds bounds)
{
  ec_internal_bounds* cast_bounds = (ec_internal_bounds*)bounds;
  delete cast_bounds;
}

extern "C" void ec_add_polar_coords_bound(ec_bounds bounds, float frequency, float offset, float scalar, float power)
{
  ec_internal_bounds* cast_bounds = (ec_internal_bounds*)bounds;
  ec::PolarCoordElement e(frequency, offset, scalar, power);
  cast_bounds->push_back(e);
}

extern "C" ec_effects ec_create_effects_list()
{
  return (ec_effects)(new ec_internal_effects);
}

extern "C" void ec_free_effects_list(ec_effects effects)
{
  ec_internal_effects* cast_effects = (ec_internal_effects*)effects;
  delete cast_effects;
}

extern "C" void ec_remove_weapon(actor* _actor)
{
  for (int i = 0; i < (int)references.size(); )
  {
    std::vector<ec_internal_reference*>::iterator iter = references.begin() + i;
    if ((*iter)->dead)
    {
      delete *iter;
      references.erase(iter);
      continue;
    }

    i++;
    if (((*iter)->caster == _actor) && ((*iter)->effect->get_type() == ec::EC_SWORD))
    {
      (*iter)->effect->recall = true;
      (*iter)->caster = NULL;
      (*iter)->target = NULL;
      continue;
    }
  }
}

extern "C" void ec_add_effect(ec_effects effects, ec_reference ref)
{
  ec_internal_effects* cast_effects = (ec_internal_effects*)effects;
  ec_internal_reference* cast_reference = (ec_internal_reference*)ref;
  cast_effects->push_back(cast_reference->effect);
}

extern "C" int ec_in_range(float x, float y, float z, Uint64 effect_max_time)
{
  float dist_squared = (ec::Vec3(x, z, -y) - ec::Vec3(-camera_x, -camera_z, camera_y)).magnitude_squared();
  if (dist_squared < ec::square(MAX_EFFECT_DISTANCE + (effect_max_time * WALK_RATE) / 1000000.0))
    return 1;
  else
    return 0;
}

extern "C" ec_reference ec_create_generic()
{
  references.push_back(new ec_internal_reference);
  return (ec_reference)(references[references.size() - 1]);
}

extern "C" void ec_add_target(ec_reference reference, float x, float y, float z)
{
  ec_internal_reference* cast_reference = (ec_internal_reference*)reference;
  ec::Vec3 target(x, z, -y);
  cast_reference->targets.push_back(target);
}

extern "C" int ec_change_target(ec_reference reference, int index, float x, float y, float z)
{
  ec_internal_reference* cast_reference = (ec_internal_reference*)reference;
  if (index >= (int)cast_reference->targets.size())
    return false;
  cast_reference->targets[index] = ec::Vec3(x, z, -y);
  return true;
}

extern "C" ec_reference ec_create_bag_pickup(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::BagEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::BagEffect(&eye_candy, &ret->dead, &ret->position, true, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_bag_drop(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::BagEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::BagEffect(&eye_candy, &ret->dead, &ret->position, false, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_breath_fire(float sx, float sy, float sz, float tx, float ty, float tz, int LOD, float scale)
{
  if (!ec_in_range(sx, sy, sz, ec::BreathEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(sx, sz, -(sy + 0.25));
  ret->position2 = ec::Vec3(tx, tz, -(ty + 0.25));
  ret->effect = new ec::BreathEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, &general_obstructions_list, ec::BreathEffect::FIRE, LOD, scale);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_breath_ice(float sx, float sy, float sz, float tx, float ty, float tz, int LOD, float scale)
{
  if (!ec_in_range(sx, sy, sz, ec::BreathEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(sx, sz, -(sy + 0.25));
  ret->position2 = ec::Vec3(tx, tz, -(ty + 0.25));
  ret->effect = new ec::BreathEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, &general_obstructions_list, ec::BreathEffect::ICE, LOD, scale);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_breath_poison(float sx, float sy, float sz, float tx, float ty, float tz, int LOD, float scale)
{
  if (!ec_in_range(sx, sy, sz, ec::BreathEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(sx, sz, -(sy + 0.25));
  ret->position2 = ec::Vec3(tx, tz, -(ty + 0.25));
  ret->effect = new ec::BreathEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, &general_obstructions_list, ec::BreathEffect::POISON, LOD, scale);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_breath_magic(float sx, float sy, float sz, float tx, float ty, float tz, int LOD, float scale)
{
  if (!ec_in_range(sx, sy, sz, ec::BreathEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(sx, sz, -(sy + 0.25));
  ret->position2 = ec::Vec3(tx, tz, -(ty + 0.25));
  ret->effect = new ec::BreathEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, &general_obstructions_list, ec::BreathEffect::MAGIC, LOD, scale);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_breath_lightning(float sx, float sy, float sz, float tx, float ty, float tz, int LOD, float scale)
{
  if (!ec_in_range(sx, sy, sz, ec::BreathEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(sx, sz, -(sy + 0.25));
  ret->position2 = ec::Vec3(tx, tz, -(ty + 0.25));
  ret->effect = new ec::BreathEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, &general_obstructions_list, ec::BreathEffect::LIGHTNING, LOD, scale);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_breath_wind(float sx, float sy, float sz, float tx, float ty, float tz, int LOD, float scale)
{
  if (!ec_in_range(sx, sy, sz, ec::BreathEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(sx, sz, -(sy + 0.25));
  ret->position2 = ec::Vec3(tx, tz, -(ty + 0.25));
  ret->effect = new ec::BreathEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, &general_obstructions_list, ec::BreathEffect::WIND, LOD, scale);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_campfire(float x, float y, float z, int LOD, float scale)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::CampfireEffect(&eye_candy, &ret->dead, &ret->position, &fire_obstructions_list, scale, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_cloud(float x, float y, float z, float density, ec_bounds bounds, int LOD)
{
  ec_internal_bounds* cast_bounds = (ec_internal_bounds*)bounds;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::CloudEffect(&eye_candy, &ret->dead, &ret->position, density, *cast_bounds, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_fireflies(float x, float y, float z, float density, ec_bounds bounds)
{
  ec_internal_bounds* cast_bounds = (ec_internal_bounds*)bounds;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::FireflyEffect(&eye_candy, &ret->dead, &ret->position, &general_obstructions_list, density, *cast_bounds);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_fountain(float x, float y, float z, float base_height, int backlit, float scale, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::FountainEffect(&eye_candy, &ret->dead, &ret->position, backlit, base_height, scale, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_harvesting_radon_pouch(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::HarvestingEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::HarvestingEffect(&eye_candy, &ret->dead, &ret->position, ec::HarvestingEffect::RADON_POUCH, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_harvesting_cavern_wall(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::HarvestingEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::HarvestingEffect(&eye_candy, &ret->dead, &ret->position, ec::HarvestingEffect::CAVERN_WALL, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_harvesting_mother_nature(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::HarvestingEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::HarvestingEffect(&eye_candy, &ret->dead, &ret->position, ec::HarvestingEffect::MOTHER_NATURE, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_harvesting_queen_of_nature(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::HarvestingEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::HarvestingEffect(&eye_candy, &ret->dead, &ret->position, ec::HarvestingEffect::QUEEN_OF_NATURE, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_harvesting_bees(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::HarvestingEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::HarvestingEffect(&eye_candy, &ret->dead, &ret->position, ec::HarvestingEffect::BEES, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_harvesting_bag_of_gold(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::HarvestingEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::HarvestingEffect(&eye_candy, &ret->dead, &ret->position, ec::HarvestingEffect::BAG_OF_GOLD, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_harvesting_rare_stone(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::HarvestingEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::HarvestingEffect(&eye_candy, &ret->dead, &ret->position, ec::HarvestingEffect::RARE_STONE, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_impact_magic_protection(float x, float y, float z, float angle_x, float angle_y, float angle_z, int LOD, float strength)
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

extern "C" ec_reference ec_create_impact_shield(float x, float y, float z, float angle_x, float angle_y, float angle_z, int LOD, float strength)
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

extern "C" ec_reference ec_create_impact_magic_immunity(float x, float y, float z, float angle_x, float angle_y, float angle_z, int LOD, float strength)
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

extern "C" ec_reference ec_create_impact_poison(float x, float y, float z, float angle_x, float angle_y, float angle_z, int LOD, float strength)
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

extern "C" ec_reference ec_create_impact_blood(float x, float y, float z, float angle_x, float angle_y, float angle_z, int LOD, float strength)
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

extern "C" ec_reference ec_create_lamp(float x, float y, float z, float scale, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::LampEffect(&eye_candy, &ret->dead, &ret->position, scale, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_ongoing_magic_protection(float x, float y, float z, int LOD, float scale)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::OngoingEffect(&eye_candy, &ret->dead, &ret->position, ec::OngoingEffect::MAGIC_PROTECTION, LOD, scale);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_ongoing_shield(float x, float y, float z, int LOD, float scale)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::OngoingEffect(&eye_candy, &ret->dead, &ret->position, ec::OngoingEffect::SHIELD, LOD, scale);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_ongoing_magic_immunity(float x, float y, float z, int LOD, float scale)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::OngoingEffect(&eye_candy, &ret->dead, &ret->position, ec::OngoingEffect::MAGIC_IMMUNITY, LOD, scale);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_ongoing_poison(float x, float y, float z, int LOD, float scale)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::OngoingEffect(&eye_candy, &ret->dead, &ret->position, ec::OngoingEffect::POISON, LOD, scale);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_selfmagic_heal(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::HEAL, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_selfmagic_heal2(actor* caster, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = caster;
  set_vec3_actor_bone(ret->position, ret->caster, 25);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::HEAL, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_selfmagic_magic_protection(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::MAGIC_PROTECTION, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_selfmagic_magic_protection2(actor* caster, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = caster;
  set_vec3_actor_bone(ret->position, ret->caster, 25);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::MAGIC_PROTECTION, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_selfmagic_shield(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::SHIELD, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_selfmagic_shield2(actor* caster, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = caster;
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::SHIELD, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_selfmagic_restoration(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::RESTORATION, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_selfmagic_restoration2(actor* caster, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = caster;
  set_vec3_actor_bone(ret->position, ret->caster, 25);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::RESTORATION, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_selfmagic_bones_to_gold(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::BONES_TO_GOLD, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_selfmagic_bones_to_gold2(actor* caster, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = caster;
  set_vec3_actor_bone(ret->position, ret->caster, 25);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::BONES_TO_GOLD, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_selfmagic_teleport_to_the_portals_room(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::TELEPORT_TO_THE_PORTALS_ROOM, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_selfmagic_teleport_to_the_portals_room2(actor* caster, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = caster;
  set_vec3_actor_bone(ret->position, ret->caster, 25);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::TELEPORT_TO_THE_PORTALS_ROOM, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_selfmagic_magic_immunity(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::MAGIC_IMMUNITY, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_selfmagic_magic_immunity2(actor* caster, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = caster;
  set_vec3_actor_bone(ret->position, ret->caster, 25);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::MAGIC_IMMUNITY, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_alert(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::ALERT, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_alert2(actor* caster, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::SelfMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = caster;
  set_vec3_actor_bone(ret->position, ret->caster, 25);
  ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::ALERT, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_smoke(float x, float y, float z, float scale, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SmokeEffect(&eye_candy, &ret->dead, &ret->position, scale, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_rabbit(float x, float y, float z, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::RABBIT, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_rat(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::RAT, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_beaver(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::BEAVER, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_skunk(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::SKUNK, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_racoon(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::RACOON, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_deer(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::DEER, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_green_snake(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::GREEN_SNAKE, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_red_snake(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::RED_SNAKE, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_brown_snake(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::BROWN_SNAKE, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_fox(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::FOX, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_boar(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::BOAR, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_wolf(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::WOLF, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_skeleton(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::SKELETON, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_small_gargoyle(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::SMALL_GARGOYLE, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_medium_gargoyle(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::MEDIUM_GARGOYLE, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_large_gargoyle(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::LARGE_GARGOYLE, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_puma(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::PUMA, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_female_goblin(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::FEMALE_GOBLIN, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_polar_bear(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::POLAR_BEAR, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_bear(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::BEAR, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_armed_male_goblin(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::ARMED_MALE_GOBLIN, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_armed_skeleton(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::ARMED_SKELETON, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_female_orc(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::FEMALE_ORC, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_male_orc(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::MALE_ORC, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_armed_female_orc(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::ARMED_FEMALE_ORC, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_armed_male_orc(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::ARMED_MALE_ORC, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_cyclops(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::CYCLOPS, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_fluffy(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::FLUFFY, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_phantom_warrior(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::PHANTOM_WARRIOR, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_mountain_chimeran(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::MOUNTAIN_CHIMERAN, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_yeti(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::YETI, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_arctic_chimeran(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::ARCTIC_CHIMERAN, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_giant(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::GIANT, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_giant_snake(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::GIANT_SNAKE, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_spider(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::SPIDER, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_tiger(float x, float y, float z, int LOD)
{
  if (!ec_in_range(x, y, z, ec::SummonEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::TIGER, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_sword_serpent(actor* _actor, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = _actor;
  get_sword_positions(_actor, ret->position, ret->position2);
  ret->effect = new ec::SwordEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::SwordEffect::SERPENT, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_sword_cutlass(actor* _actor, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = _actor;
  get_sword_positions(_actor, ret->position, ret->position2);
  ret->effect = new ec::SwordEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::SwordEffect::CUTLASS, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_sword_emerald_claymore(actor* _actor, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = _actor;
  get_sword_positions(_actor, ret->position, ret->position2);
  ret->effect = new ec::SwordEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::SwordEffect::EMERALD_CLAYMORE, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_sword_sunbreaker(actor* _actor, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = _actor;
  get_sword_positions(_actor, ret->position, ret->position2);
  ret->effect = new ec::SwordEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::SwordEffect::SUNBREAKER, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_sword_orc_slayer(actor* _actor, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = _actor;
  get_sword_positions(_actor, ret->position, ret->position2);
  ret->effect = new ec::SwordEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::SwordEffect::ORC_SLAYER, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_sword_eagle_wing(actor* _actor, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = _actor;
  get_sword_positions(_actor, ret->position, ret->position2);
  ret->effect = new ec::SwordEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::SwordEffect::EAGLE_WING, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_sword_jagged_saber(actor* _actor, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = _actor;
  get_sword_positions(_actor, ret->position, ret->position2);
  ret->effect = new ec::SwordEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::SwordEffect::JAGGED_SABER, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_sword_of_fire(actor* _actor, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = _actor;
  get_sword_positions(_actor, ret->position, ret->position2);
  ret->effect = new ec::SwordEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::SwordEffect::SWORD_OF_FIRE, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_sword_of_ice(actor* _actor, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = _actor;
  get_sword_positions(_actor, ret->position, ret->position2);
  ret->effect = new ec::SwordEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::SwordEffect::SWORD_OF_ICE, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_sword_of_magic(actor* _actor, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = _actor;
  get_sword_positions(_actor, ret->position, ret->position2);
  ret->effect = new ec::SwordEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::SwordEffect::SWORD_OF_MAGIC, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_targetmagic_remote_heal(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD)
{
  if (!ec_in_range(start_x, start_y, start_z, ec::TargetMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(start_x, start_z, -start_y);
  ret->position2 = ec::Vec3(end_x, end_z, -end_y);
  ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::REMOTE_HEAL, &general_obstructions_list, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_targetmagic_remote_heal2(actor* caster, actor* target, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::TargetMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = caster;
  ret->target = target;
  set_vec3_actor_bone(ret->position, caster, 25);
  ret->position2 = ec::Vec3(target->x_pos, target->z_pos, -target->y_pos);
  ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::REMOTE_HEAL, &general_obstructions_list, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_targetmagic_poison(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD)
{
  if (!ec_in_range(start_x, start_y, start_z, ec::TargetMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(start_x, start_z, -start_y);
  ret->position2 = ec::Vec3(end_x, end_z, -end_y);
  ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::POISON, &general_obstructions_list, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_targetmagic_poison2(actor* caster, actor* target, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::TargetMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = caster;
  ret->target = target;
  set_vec3_actor_bone(ret->position, caster, 25);
  ret->position2 = ec::Vec3(target->x_pos, target->z_pos, -target->y_pos);
  ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::POISON, &general_obstructions_list, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_targetmagic_teleport_to_range(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD)
{
  if (!ec_in_range(start_x, start_y, start_z, ec::TargetMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(start_x, start_z, -start_y);
  ret->position2 = ec::Vec3(end_x, end_z, -end_y);
  ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::TELEPORT_TO_RANGE, &general_obstructions_list, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_targetmagic_harm(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD)
{
  if (!ec_in_range(start_x, start_y, start_z, ec::TargetMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(start_x, start_z, -start_y);
  ret->position2 = ec::Vec3(end_x, end_z, -end_y);
  ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::HARM, &general_obstructions_list, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_targetmagic_harm2(actor* caster, actor* target, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::TargetMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = caster;
  ret->target = target;
  set_vec3_actor_bone(ret->position, caster, 25);
  ret->position2 = ec::Vec3(target->x_pos, target->z_pos, -target->y_pos);
  ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::HARM, &general_obstructions_list, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_targetmagic_life_drain(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD)
{
  if (!ec_in_range(start_x, start_y, start_z, ec::TargetMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(start_x, start_z, -start_y);
  ret->position2 = ec::Vec3(end_x, end_z, -end_y);
  ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::LIFE_DRAIN, &general_obstructions_list, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_targetmagic_life_drain2(actor* caster, actor* target, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::TargetMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = caster;
  ret->target = target;
  set_vec3_actor_bone(ret->position, caster, 25);
  ret->position2 = ec::Vec3(target->x_pos, target->z_pos, -target->y_pos);
  ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::LIFE_DRAIN, &general_obstructions_list, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" void ec_launch_targetmagic_heal_summoned(ec_reference reference, float start_x, float start_y, float start_z, int LOD)
{
  ec_internal_reference* cast_reference = (ec_internal_reference*)reference;
  if (!ec_in_range(start_x, start_y, start_z, ec::TargetMagicEffect::get_max_end_time()))
  {
    delete cast_reference;
    return;
  }
  cast_reference->position = ec::Vec3(start_x, start_z, -start_y);
  std::vector<ec::Vec3*> target_ptrs;
  for (std::vector<ec::Vec3>::iterator iter = cast_reference->targets.begin(); iter != cast_reference->targets.end(); iter++)
    target_ptrs.push_back(&(*iter));
  cast_reference->effect = new ec::TargetMagicEffect(&eye_candy, &cast_reference->dead, &cast_reference->position, target_ptrs, ec::TargetMagicEffect::HEAL_SUMMONED, &general_obstructions_list, LOD);
  eye_candy.push_back_effect(cast_reference->effect);
}

extern "C" void ec_launch_targetmagic_smite_summoned(ec_reference reference, float start_x, float start_y, float start_z, int LOD)
{
  ec_internal_reference* cast_reference = (ec_internal_reference*)reference;
  if (!ec_in_range(start_x, start_y, start_z, ec::TargetMagicEffect::get_max_end_time()))
  {
    delete cast_reference;
    return;
  }
  cast_reference->position = ec::Vec3(start_x, start_z, -start_y);
  std::vector<ec::Vec3*> target_ptrs;
  for (std::vector<ec::Vec3>::iterator iter = cast_reference->targets.begin(); iter != cast_reference->targets.end(); iter++)
    target_ptrs.push_back(&(*iter));
  cast_reference->effect = new ec::TargetMagicEffect(&eye_candy, &cast_reference->dead, &cast_reference->position, target_ptrs, ec::TargetMagicEffect::SMITE_SUMMONED, &general_obstructions_list, LOD);
  eye_candy.push_back_effect(cast_reference->effect);
}

extern "C" ec_reference ec_create_targetmagic_drain_mana(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, int LOD)
{
  if (!ec_in_range(start_x, start_y, start_z, ec::TargetMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(start_x, start_z, -start_y);
  ret->position2 = ec::Vec3(end_x, end_z, -end_y);
  ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::DRAIN_MANA, &general_obstructions_list, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_targetmagic_drain_mana2(actor* caster, actor* target, int LOD)
{
  if (!ec_in_range(caster->x_pos, caster->y_pos, caster->z_pos, ec::TargetMagicEffect::get_max_end_time()))
    return NULL;
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->caster = caster;
  ret->target = target;
  set_vec3_actor_bone(ret->position, caster, 25);
  ret->position2 = ec::Vec3(target->x_pos, target->z_pos, -target->y_pos);
  ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::DRAIN_MANA, &general_obstructions_list, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_teleporter(float x, float y, float z, int LOD)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ret->position = ec::Vec3(x, z, -y);
  ret->effect = new ec::TeleporterEffect(&eye_candy, &ret->dead, &ret->position, LOD);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_wind_leaves(float x, float y, float z, float density, ec_bounds bounds, float prevailing_wind_x, float prevailing_wind_y, float prevailing_wind_z)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ec_internal_bounds* cast_bounds = (ec_internal_bounds*)bounds;
  ret->position = ec::Vec3(x, z, -y);
  ret->position2 = ec::Vec3(prevailing_wind_x, prevailing_wind_z, -(prevailing_wind_y + 0.25));
  ret->effect = new ec::WindEffect(&eye_candy, &ret->dead, &ret->position, &general_obstructions_list, density, *cast_bounds, ec::WindEffect::LEAVES, ret->position2);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" ec_reference ec_create_wind_petals(float x, float y, float z, float density, ec_bounds bounds, float prevailing_wind_x, float prevailing_wind_y, float prevailing_wind_z)
{
  ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
  ec_internal_bounds* cast_bounds = (ec_internal_bounds*)bounds;
  ret->position = ec::Vec3(x, z, -y);
  ret->position2 = ec::Vec3(prevailing_wind_x, prevailing_wind_z, -(prevailing_wind_y + 0.25));
  ret->effect = new ec::WindEffect(&eye_candy, &ret->dead, &ret->position, &general_obstructions_list, density, *cast_bounds, ec::WindEffect::FLOWER_PETALS, ret->position2);
  eye_candy.push_back_effect(ret->effect);
  return (ec_reference)ret;
}

extern "C" void ec_add_wind_effect_list(ec_reference reference, ec_effects effects)
{
  ec_internal_reference* cast_reference = (ec_internal_reference*)reference;
  ec_internal_effects* cast_effects = (ec_internal_effects*)effects;
  ((ec::WindEffect*)(cast_reference->effect))->set_pass_off(*cast_effects);
}

#endif	// #ifdef EYE_CANDY
