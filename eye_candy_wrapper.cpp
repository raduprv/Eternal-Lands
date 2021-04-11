// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy_wrapper.h"
#ifndef MAP_EDITOR
#include "cal.h"
#endif
#include "cal3d_wrapper.h"
#include "client_serv.h" // For mine_type defines
#ifndef MAP_EDITOR
#include "counters.h"
#endif
#include "draw_scene.h"
#include "elconfig.h"
#include "errors.h"
#if !defined(MAP_EDITOR)
#include "gamewin.h"
#endif
#include "gl_init.h"
#include "map.h"
#ifndef MAP_EDITOR
#include "missiles.h"
#endif
#include "particles.h"
#include "shadows.h"
#ifndef MAP_EDITOR
#include "skeletons.h"
#endif
#include "tiles.h"
#include "weather.h"

// G L O B A L S //////////////////////////////////////////////////////////////

#ifdef MAP_EDITOR
extern int day_shadows_on;
ec::SmoothPolygonBoundingRange initial_bounds;
#endif

extern "C"
{
	int use_eye_candy = 1;
	int use_harvesting_eye_candy = 0;
	int use_lamp_halo = 0;
	float min_ec_framerate = 13.0;
	float max_ec_framerate = 37.0;
	int light_columns_threshold = 5;
	int use_fancy_smoke = 1;
	int max_idle_cycles_per_second = 40;
	ec_reference harvesting_effect_reference = NULL;
}


namespace
{

struct ec_internal_reference
{
	ec_internal_reference(): effect(nullptr), position(), position2(),
#ifndef MAP_EDITOR
	caster(nullptr), target(nullptr), target_actors(),
#endif
	targets(), bounds(), dead(false), casterbone(), targetbone(), missile_id() {}

	ec::Effect* effect;
	ec::Vec3 position;
	ec::Vec3 position2;
#ifndef MAP_EDITOR
	actor* caster;
	actor* target;
	std::vector<actor*> target_actors;
#endif
	std::vector<ec::Vec3> targets;
	ec::SmoothPolygonBoundingRange bounds;
	bool dead;
	int casterbone;
	int targetbone;
	int missile_id;
};

struct ec_object_obstruction
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
};
typedef std::vector<ec_object_obstruction*> ec_object_obstructions;

#ifndef MAP_EDITOR
struct ec_actor_obstruction
{
	actor* obstructing_actor;
	ec::Vec3 center;
	ec::Obstruction* obstruction;
};
typedef std::vector<ec_actor_obstruction*> ec_actor_obstructions;
#endif

typedef std::vector<ec::Effect*> ec_internal_effects;


ec::EyeCandy eye_candy;
Uint64 ec_cur_time, ec_last_time;
std::vector<ec_internal_reference*> references;
int idle_cycles_this_second = 0;

const float MAX_EFFECT_DISTANCE = 16.0;
const float MAX_OBSTRUCT_DISTANCE_SQUARED = MAX_EFFECT_DISTANCE * MAX_EFFECT_DISTANCE;
const float WALK_RATE = 1.0;
const float SWORD_HILT_LENGTH = 0.1;
const float SWORD_BLADE_LENGTH = 0.5;
const float X_OFFSET = 0.25;
const float Y_OFFSET = 0.25;

ec_object_obstructions object_obstructions;
#ifndef MAP_EDITOR
ec_actor_obstructions actor_obstructions;
ec_actor_obstruction self_actor;
#endif
std::vector<ec::Obstruction*> general_obstructions_list;
std::vector<ec::Obstruction*> fire_obstructions_list;
bool force_idle = false;
volatile bool idle_semaphore = false;

float average_framerate = 20000.0; // Windows has such horrible timer resolution, I have to average these out.  Anyways, it doesn't hurt to do this in Linux, either.


void ec_heartbeat()
{
	//  std::cout << "Actor: <" << camera_x << ", " << camera_z << ", " << -camera_y << ">" << std::endl;
	//  if (!((int)(ec_cur_time / 1000000.0) % 9))
	//    ec_create_breath_fire(44.75, 38.0, 1.0, 44.75, 43.0, 0.6, 2, 1.5);
	idle_cycles_this_second = 0;
	general_obstructions_list.clear();
	fire_obstructions_list.clear();
	for (auto obstruction: object_obstructions)
	{
		obstruction->center.x = obstruction->obj3d->x_pos;
		obstruction->center.y = obstruction->obj3d->z_pos;
		obstruction->center.z = -obstruction->obj3d->y_pos;
		const float dist_squared = (obstruction->center - ec::Vec3(-camera_x, -camera_z, camera_y)).magnitude_squared();
		if (dist_squared> MAX_OBSTRUCT_DISTANCE_SQUARED)
			continue;
		/*
		 obstruction->center.x += X_OFFSET;
		 obstruction->center.y += 0;
		 obstruction->center.z -= Y_OFFSET;
		 */
		obstruction->sin_rot_x = sin(obstruction->obj3d->x_rot * (ec::PI / 180));
		obstruction->cos_rot_x = cos(obstruction->obj3d->x_rot * (ec::PI / 180));
		obstruction->sin_rot_y = sin(obstruction->obj3d->z_rot * (ec::PI / 180));
		obstruction->cos_rot_y = cos(obstruction->obj3d->z_rot * (ec::PI / 180));
		obstruction->sin_rot_z = sin(-(obstruction->obj3d->y_rot * (ec::PI / 180)));
		obstruction->cos_rot_z = cos(-(obstruction->obj3d->y_rot * (ec::PI / 180)));
		obstruction->sin_rot_x2 = sin(-obstruction->obj3d->x_rot * (ec::PI / 180));
		obstruction->cos_rot_x2 = cos(-obstruction->obj3d->x_rot * (ec::PI / 180));
		obstruction->sin_rot_y2 = sin(-obstruction->obj3d->z_rot * (ec::PI / 180));
		obstruction->cos_rot_y2 = cos(-obstruction->obj3d->z_rot * (ec::PI / 180));
		obstruction->sin_rot_z2 = sin((obstruction->obj3d->y_rot * (ec::PI / 180)));
		obstruction->cos_rot_z2 = cos((obstruction->obj3d->y_rot * (ec::PI / 180)));
		general_obstructions_list.push_back(obstruction->obstruction);
		if (obstruction->fire_related)
			fire_obstructions_list.push_back(obstruction->obstruction);
	}
#ifndef MAP_EDITOR
	for (auto obstruction: actor_obstructions)
	{
		obstruction->center.x = obstruction->obstructing_actor->x_pos;
		obstruction->center.y = ec_get_z(obstruction->obstructing_actor);
		obstruction->center.z = -obstruction->obstructing_actor->y_pos;
		const float dist_squared = (obstruction->center - ec::Vec3(-camera_x, -camera_z, camera_y)).magnitude_squared();
		if (dist_squared> MAX_OBSTRUCT_DISTANCE_SQUARED)
			continue;
		obstruction->center.x += X_OFFSET;
		obstruction->center.y += Y_OFFSET;
		obstruction->center.z -= 0.25;
		general_obstructions_list.push_back(obstruction->obstruction);
	}
	// Last but not least... the actor.
	self_actor.center.x = -camera_x;
	self_actor.center.y = -camera_z;
	self_actor.center.z = camera_y;
	general_obstructions_list.push_back(self_actor.obstruction);
#endif
}

int ec_in_range(float x, float y, float z, Uint64 effect_max_time)
{
	float dist_squared = (ec::Vec3(x, z, -y) - ec::Vec3(-camera_x, -camera_z, camera_y)).magnitude_squared();
	if (dist_squared < ec::square(MAX_EFFECT_DISTANCE + (effect_max_time * WALK_RATE) / 1000000.0))
		return 1;
	else
		return 0;
}

#ifndef MAP_EDITOR
void set_vec3_actor_bone(ec::Vec3& position, actor* _actor, int bone,
	const ec::Vec3 shift)
{
	float points[1024][3];

	const int num_bones = CalSkeleton_GetBonePoints(
		CalModel_GetSkeleton(_actor->calmodel), &points[0][0]);
	if (num_bones <= bone)
		return;

	ec::Vec3 unrotated_position;
	unrotated_position.x = points[bone][0] + shift.x;
	unrotated_position.y = points[bone][1] + shift.z;
	unrotated_position.z = points[bone][2] - shift.y;

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

	position.x = roty_position.x + _actor->x_pos + X_OFFSET;
	position.y = roty_position.z + ec_get_z(_actor);
	position.z = -(roty_position.y + _actor->y_pos + Y_OFFSET);
}

void set_vec3_actor_bone2(ec::Vec3& position, actor* _actor, int bone,
	const ec::Vec3 shift)
{
	float act_rot[9];
	float tmp_pos[3], pos[3];
	float _shift[3] = { 0.0, 0.0, 0.0 };

	get_actor_rotation_matrix(_actor, act_rot);

	cal_get_actor_bone_local_position(_actor, bone, _shift, tmp_pos);
	transform_actor_local_position_to_absolute(_actor, tmp_pos, act_rot, pos);
	position.x = pos[0] + shift.x;
	position.y = pos[2] + shift.y;
	position.z = -pos[1] + shift.z;
}

void set_vec3_actor_bone2(ec::Vec3& position, actor* _actor, int bone)
{
	set_vec3_actor_bone2(position, _actor, bone, ec::Vec3(0.0, 0.0, 0.0));
}

void set_vec3_target_bone2(ec::Vec3& position, actor* _actor, int bone,
	const ec::Vec3 shift)
{
	set_vec3_actor_bone2(position, _actor, bone, ec::Vec3(0.0, 0.0, 0.0));
}

void set_vec3_target_bone2(ec::Vec3& position, actor* _actor, int bone)
{
	set_vec3_target_bone2(position, _actor, bone, ec::Vec3(0.0, 0.0, 0.0));
}

void get_sword_positions(actor* _actor, ec::Vec3& base, ec::Vec3& tip)
{
	float act_rot[9];
	float tmp_pos[3], pos[3];
	float shift[3] =
	{	0.0, SWORD_HILT_LENGTH, 0.0};
	int weapon_bone_id = get_actor_bone_id(_actor, weapon_right_bone);

	get_actor_rotation_matrix(_actor, act_rot);

	cal_get_actor_bone_local_position(_actor, weapon_bone_id, shift, tmp_pos);
	transform_actor_local_position_to_absolute(_actor, tmp_pos, act_rot, pos);
	base.x = pos[0]; base.y = pos[2]; base.z = -pos[1];

	shift[1] += SWORD_BLADE_LENGTH;
	cal_get_actor_bone_local_position(_actor, weapon_bone_id, shift, tmp_pos);
	transform_actor_local_position_to_absolute(_actor, tmp_pos, act_rot, pos);
	tip.x = pos[0]; tip.y = pos[2]; tip.z = -pos[1];
}

void get_staff_position(actor* _actor, ec::Vec3& tip)
{
	float act_rot[9];
	float tmp_pos[3], pos[3];
	float shift[3] = { 0.0, 0.45, 0.0 };
	int weapon_bone_id = get_actor_bone_id(_actor, staff_right_bone);

	get_actor_rotation_matrix(_actor, act_rot);

	cal_get_actor_bone_local_position(_actor, weapon_bone_id, shift, tmp_pos);
	transform_actor_local_position_to_absolute(_actor, tmp_pos, act_rot, pos);
	tip.x = pos[0]; tip.y = pos[2]; tip.z = -pos[1];
}

#endif // !MAP_EDITOR

} // namespace

extern "C" void ec_init()
{
	eye_candy.load_textures();
	ec_last_time = 0;
	ec_cur_time = 0;
#ifdef MAP_EDITOR
	ec::SmoothPolygonElement e(0.0, 25.0);
	initial_bounds.elements.push_back(e);
#else
	self_actor.obstruction = new ec::CappedSimpleCylinderObstruction(&(self_actor.center), 0.45, 3.0, self_actor.center.y, self_actor.center.y + 0.9);
#endif

}

extern "C" void ec_add_light(GLenum light_id)
{
	eye_candy.add_light(light_id);
}

extern "C" float ec_get_z2(int x, int y)
{
	return get_tile_height(x, y);
}

extern "C" void ec_idle()
{
	if (idle_semaphore)
		return;

	if ((!use_eye_candy) && (!force_idle))
		return;

	idle_semaphore = true;

	force_idle = false;

	const std::vector<std::string> ec_errors = ec::logger.fetch();
	for (const auto& error: ec_errors)
		LOG_ERROR(error.c_str());

	if (ec::get_error_status())
	{
		idle_semaphore = false;
		return;
	}

	//  GLfloat rot_matrix[16];
	//  glGetFloatv(GL_MODELVIEW_MATRIX, rot_matrix);
	//  const float x = rot_matrix[12];
	//  const float y = rot_matrix[13];
	//  const float z = rot_matrix[14];

	if (poor_man)
		eye_candy.set_thresholds(3500, min_ec_framerate, max_ec_framerate); //Max particles, min framerate, max framerate

	else
		eye_candy.set_thresholds(15000, min_ec_framerate, max_ec_framerate);

	const float s_rx = sin(rx * ec::PI / 180);
	const float c_rx = cos(rx * ec::PI / 180);
	const float s_rz = sin(rz * ec::PI / 180);
	const float c_rz = cos(rz * ec::PI / 180);
	float new_camera_x = -zoom_level*camera_distance * s_rx * s_rz + camera_x;
	float new_camera_y = -zoom_level*camera_distance * s_rx * c_rz + camera_y;
	float new_camera_z = -zoom_level*camera_distance * c_rx + camera_z;
	eye_candy.set_camera(ec::Vec3(-new_camera_x, -new_camera_z, new_camera_y));
	eye_candy.set_center(ec::Vec3(-camera_x, -camera_z, camera_y));

	if ((average_framerate >= light_columns_threshold * 1.15 + 2.5) && (!eye_candy.draw_shapes))
		eye_candy.draw_shapes = true;
	else if ((average_framerate < light_columns_threshold / 1.15 - 2.5) && (eye_candy.draw_shapes))
		eye_candy.draw_shapes = false;

	eye_candy.set_dimensions(window_width, window_height, powf(zoom_level, 0.1));

	Uint64 new_time = ec::get_time();
#if defined CLUSTER_INSIDES && !defined MAP_EDITOR
	short cluster = get_actor_cluster ();
#endif
	for (int i = 0; i < (int)references.size(); )
	{
		std::vector<ec_internal_reference*>::iterator iter = references.begin() + i;
		if ((*iter)->dead)
		{
			delete *iter;
			references.erase(iter);
			continue;
		}

		if (use_eye_candy)
		{
#if defined CLUSTER_INSIDES && !defined MAP_EDITOR
			if ((*iter)->effect && !(*iter)->effect->belongsToCluster (cluster))
			{
				(*iter)->effect->active = false;
				i++;
				continue;
			}
#endif
#ifndef MAP_EDITOR
			if ((*iter)->caster)
			{
				if ((*iter)->effect->get_type() == ec::EC_SWORD)
					get_sword_positions((*iter)->caster, (*iter)->position, (*iter)->position2);
				else if ((*iter)->effect->get_type() == ec::EC_STAFF)
					get_staff_position((*iter)->caster, (*iter)->position);
				else if ((*iter)->effect->get_type() == ec::EC_TARGETMAGIC)
				{
					if ((*iter)->casterbone> -1)
					{
						set_vec3_actor_bone2((*iter)->position, (*iter)->caster, (*iter)->casterbone);
					}
					else
					{ // use default bone
						set_vec3_actor_bone2((*iter)->position, (*iter)->caster, get_actor_bone_id((*iter)->caster, head_bone));
					}
				}
				else if ((*iter)->effect->get_type() == ec::EC_GLOW)
				{
					if ((*iter)->casterbone> -1)
					{
						set_vec3_actor_bone2((*iter)->position, (*iter)->caster, (*iter)->casterbone);
					}
					else
					{ // use default bone
						set_vec3_actor_bone2((*iter)->position, (*iter)->caster, get_actor_bone_id((*iter)->caster, hand_right_bone));
					}
				}
				else
				{
					//          (*iter)->position = ec::Vec3((*iter)->caster->x_pos + X_OFFSET, ec_get_z((*iter)->caster) - 0.25, -((*iter)->caster->y_pos + Y_OFFSET));
					//                std::cout << "ec_idle: old position: " << (*iter)->position << std::endl;
					if ((*iter)->casterbone> -1)
					{
						set_vec3_actor_bone2((*iter)->position, (*iter)->caster, (*iter)->casterbone);
					}
					else
					{ // use default bone
						set_vec3_actor_bone2((*iter)->position, (*iter)->caster, get_actor_bone_id((*iter)->caster, body_bottom_bone));
					}
					//                std::cout << "ec_idle: new position: " << (*iter)->position << std::endl;
				}
			}
			if ((*iter)->target)
			{
				//          (*iter)->position2 = ec::Vec3((*iter)->target->x_pos, ec_get_z((*iter)->target) + 0.4, -(*iter)->target->y_pos);
				if ((*iter)->targetbone> -1)
				{
					set_vec3_target_bone2((*iter)->position2, (*iter)->target, (*iter)->targetbone);
				}
				else
				{ // use default bone
					set_vec3_actor_bone2((*iter)->position2, (*iter)->target, get_actor_bone_id((*iter)->target, body_bottom_bone));
				}
			}
			for (int j = 0; j < (int)(*iter)->target_actors.size(); j++)
			{
				if ((*iter)->target_actors[j])
				{
					//set_vec3_target_bone2((*iter)->targets[j], (*iter)->target_actors[j], 25);
					// we do not store target bones for multiple targets, so use default target bone
					set_vec3_target_bone2((*iter)->targets[j], (*iter)->target_actors[j], get_actor_bone_id((*iter)->target_actors[j], body_bottom_bone));
				}
			}
#endif //!MAP_EDITOR
			if ((*iter)->effect && ((*iter)->effect->get_type() == ec::EC_LAMP) && (!(*iter)->effect->recall))
			{
				ec::LampEffect* eff = (ec::LampEffect*)((*iter)->effect);
				if (eff->halo != use_lamp_halo)
				{
					ec_create_lamp((*iter)->position.x, -(*iter)->position.z, (*iter)->position.y, 0.0, 1.0, eff->scale, eff->LOD);
					eff->recall = true;
					force_idle = true;
				}
			}
			if ((*iter)->effect && (*iter)->effect->get_type() == ec::EC_FOUNTAIN)
			{
				ec::FountainEffect* eff = (ec::FountainEffect*)((*iter)->effect);
				eff->LOD = (poor_man ? 6 : 10);
			}
			if ((*iter)->effect && (*iter)->effect->get_type() == ec::EC_SMOKE)
			{
				ec::SmokeEffect* eff = (ec::SmokeEffect*)((*iter)->effect);
				eff->LOD = (poor_man ? 6 : 10);
			}

			if ((*iter)->effect && (((*iter)->effect->get_type() == ec::EC_CLOUD)
				|| ((*iter)->effect->get_type() == ec::EC_FIREFLY)
				|| ((*iter)->effect->get_type() == ec::EC_WIND)))
			{
#ifndef MAP_EDITOR
				// doesn't work, moves effects to -2.2 under the ground
				// (*iter)->position.y = ec_get_z2(-(int)camera_x, -(int)camera_y); // Keep the effect level with the ground.
				// std::cout << (-(int)camera_x) << ", " << (-(int)camera_y) << ": " << (*iter)->position.y << std::endl;
#else //MAP_EDITOR
				(*iter)->position.y = 0.0;
#endif //!MAP_EDITOR
			}
#ifndef MAP_EDITOR
			if ((*iter)->effect && (*iter)->effect->get_type() == ec::EC_MISSILE)
			{
				missile *mis = get_missile_ptr_from_id((*iter)->missile_id);
				if (mis &&
					mis->remaining_distance >= 0.0 &&
					(*iter)->missile_id != -1)
				{
					(*iter)->position.x = mis->position[0];
					(*iter)->position.y = mis->position[2];
					(*iter)->position.z = -mis->position[1];
				}
			}
#endif //!MAP_EDITOR
		}
		i++;
	}
	if (is_day)
		eye_candy.use_lights = false;
	else
		eye_candy.use_lights = true;

	if (ec_cur_time == 0)
		eye_candy.time_diff = 100000;
	else
		eye_candy.time_diff = new_time - ec_cur_time;
	if (eye_candy.time_diff> 400000) // Don't want it to jump if it's been very long between frames.
		eye_candy.time_diff = 400000;
// 	average_framerate = average_framerate * 0.7 + 1000000.0 / eye_candy.time_diff * 0.3;
#ifndef MAP_EDITOR
	average_framerate = fps_average;
#endif //!MAP_EDITOR
	eye_candy.framerate = average_framerate;
	ec_last_time = ec_cur_time;
	ec_cur_time = new_time;

	eye_candy.max_fps = (max_fps ? max_fps : 255);

	if (use_eye_candy && ec_last_time % 1000000 >= ec_cur_time % 1000000)
		ec_heartbeat();

#if 0
	// Put debugging effects here.
	if (ec_last_time % 100000 >= ec_cur_time % 100000)
	{
		float test_x = 31.0 + ec::randfloat(6.0);
		float test_y = 36.0 + ec::randfloat(6.0);
		if (rand() & 1)
			ec_create_bag_pickup(test_x, test_y, 0.0, 10);
		else
			ec_create_bag_drop(test_x, test_y, 0.0, 10);
	}
#endif

	if ((unsigned int)(ec::get_time() % 1000000) >= (unsigned int)(1000000 * idle_cycles_this_second / max_idle_cycles_per_second))
	{
		eye_candy.idle();
		idle_cycles_this_second++;
	}

	idle_semaphore = false;
}

extern "C" void ec_draw()
{
	if (ec::get_error_status())
		return;

	if (use_eye_candy)
	{
#if defined CLUSTER_INSIDES && !defined MAP_EDITOR
		short cluster = get_actor_cluster ();
#endif

		// Update firefly activity.
		for (int i = 0; i < (int)references.size(); )
		{
			std::vector<ec_internal_reference*>::iterator iter = references.begin() + i;
			if ((*iter)->dead)
			{
				delete *iter;
				references.erase(iter);
				continue;
			}

#ifdef MAP_EDITOR
			//      if ((*iter)->effect->get_type() == ec::EC_FIREFLY)
			//        (*iter)->effect->active = (!day_shadows_on);
#else
			if ((*iter)->effect && (*iter)->effect->get_type() == ec::EC_FIREFLY
#ifdef CLUSTER_INSIDES
				&& (*iter)->effect->belongsToCluster (cluster)
#endif
			)
			{
				(*iter)->effect->active = (!(is_day || dungeon
					|| (weather_get_density() > 0.01f)
				));
			}
#endif
			i++;
		}

		glPushMatrix();
		glRotatef(90, 1.0, 0.0, 0.0);
		eye_candy.draw();
		glPopMatrix();
	}
}

extern "C" void ec_recall_effect(const ec_reference ref)
{
	force_idle = true;
	if (ref != NULL)
	{
		ec_internal_reference* cast_reference = (ec_internal_reference*)ref;
		if (cast_reference != NULL
			&& cast_reference->effect != NULL)
		{
			cast_reference->effect->recall = true;
		}
	}
}

extern "C" void ec_destroy_all_effects()
{
	// loop marking all active effects as done, cleaning up the dead till we're done
	for (int i=0; !references.empty() && i<50; ++i)
	{
		ec_delete_all_effects();
		ec_idle();
	}
	if (!references.empty()) // unlikely to happen but just so we don't get stick on exit.
		LOG_ERROR("%s: failed to clear up. references.size()=%lu", __PRETTY_FUNCTION__, references.size());
#ifndef MAP_EDITOR
	delete self_actor.obstruction;
#endif
}


extern "C" void ec_delete_all_effects()
{
	force_idle = true;
	for (int i = 0; i < (int)references.size(); )
	{
		std::vector<ec_internal_reference*>::iterator iter = references.begin() + i;
		if ((*iter)->dead)
		{
			delete *iter;
			references.erase(iter);
			continue;
		}

		if ((*iter)->effect)
			(*iter)->effect->recall = true;
		i++;
	}
}

extern "C" void ec_delete_effect_loc_type(float x, float y, ec_EffectEnum type)
{
	force_idle = true;
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
	force_idle = true;
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

extern "C" void ec_set_position(ec_reference ref, float x, float y, float z)
{
	((ec_internal_reference*)ref)->position = ec::Vec3(x, z, -y);
}

extern "C" void ec_add_object_obstruction(object3d* obj3d, e3d_object *e3dobj, float force)
{
	// First, verify that this object isn't "flat"
	if ((e3dobj->min_x == e3dobj->max_x) || (e3dobj->min_y == e3dobj->max_y) || (e3dobj->min_z == e3dobj->max_z))
		return;

	ec_object_obstruction* obstruction = new ec_object_obstruction;
	obstruction->obj3d = obj3d;
	obstruction->e3dobj = e3dobj;
	obstruction->center = ec::Vec3(obj3d->x_pos, obj3d->z_pos - 0.5, -(obj3d->y_pos));
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

extern "C" void ec_remove_obstruction_by_object3d(object3d* obj3d)
{
	for (ec_object_obstructions::iterator iter = object_obstructions.begin(); iter != object_obstructions.end(); ++iter)
	{
		if ((*iter)->obj3d == obj3d)
		{
			for (std::vector<ec::Obstruction*>::iterator iter2 = general_obstructions_list.begin(); iter2 != general_obstructions_list.end(); ++iter2)
			{
				if (*iter2 == (*iter)->obstruction)
				{
					general_obstructions_list.erase(iter2);
					break;
				}
			}
			for (std::vector<ec::Obstruction*>::iterator iter2 = fire_obstructions_list.begin(); iter2 != fire_obstructions_list.end(); ++iter2)
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
	for (ec_object_obstructions::iterator iter = object_obstructions.begin(); iter != object_obstructions.end(); ++iter)
	{
		if ((*iter)->e3dobj == e3dobj)
		{
			for (std::vector<ec::Obstruction*>::iterator iter2 = general_obstructions_list.begin(); iter2 != general_obstructions_list.end(); ++iter2)
			{
				if (*iter2 == (*iter)->obstruction)
				{
					general_obstructions_list.erase(iter2);
					break;
				}
			}
			for (std::vector<ec::Obstruction*>::iterator iter2 = fire_obstructions_list.begin(); iter2 != fire_obstructions_list.end(); ++iter2)
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
	return (ec_bounds)(new ec::SmoothPolygonBoundingRange());
}

extern "C" void ec_free_bounds_list(ec_bounds bounds)
{
	ec::SmoothPolygonBoundingRange* cast_bounds = (ec::SmoothPolygonBoundingRange*)bounds;
	delete cast_bounds;
}

extern "C" void ec_add_smooth_polygon_bound(ec_bounds bounds, float angle, float radius)
{
	ec::SmoothPolygonBoundingRange* cast_bounds = (ec::SmoothPolygonBoundingRange*)bounds;
	ec::SmoothPolygonElement e(angle, radius);
	cast_bounds->elements.push_back(e);
}

extern "C" ec_reference ec_create_generic()
{
	references.push_back(new ec_internal_reference);
	((ec_internal_reference*)(ec_reference)(references[references.size() - 1]))->casterbone = -1;
	((ec_internal_reference*)(ec_reference)(references[references.size() - 1]))->targetbone = -1;
	return (ec_reference)(references[references.size() - 1]);
}

extern "C" void ec_add_target(ec_reference reference, float x, float y, float z)
{
	ec_internal_reference* cast_reference = (ec_internal_reference*)reference;
	ec::Vec3 target(x, z, -y);
	cast_reference->targets.push_back(target);
}

extern "C" ec_reference ec_create_effect_from_map_code(char* code, float x, float y, float z, int LOD)
{
	unsigned char raw_code[54];
	unsigned char const * const code2 = reinterpret_cast<unsigned char const *>(code);
	int i = 0;

	while (i < 18)
	{
		raw_code[i * 3] = ((code2[i * 4 + 0] - ' ') >> 0) | ((code2[i * 4 + 1] - ' ') << 6);
		raw_code[i * 3 + 1] = ((code2[i * 4 + 1] - ' ') >> 2) | ((code2[i * 4 + 2] - ' ') << 4);
		raw_code[i * 3 + 2] = ((code2[i * 4 + 2] - ' ') >> 4) | ((code2[i * 4 + 3] - ' ') << 2);
		i++;
	}

	int bounds_count = raw_code[1];
	if (bounds_count> 19)
		bounds_count = 19;
	ec_bounds bounds = ec_create_bounds_list();
	for (i = 0; i < bounds_count; i++)
		ec_add_smooth_polygon_bound(bounds, raw_code[i * 2 + 2] * (2 * ec::PI) / 256.0f, raw_code[i * 2 + 3]);
	ec_reference ref = NULL;

	switch (raw_code[0])
	{
		case 0x00: // Campfire

		{
			const float hue = raw_code[41] / 256.0;
			const float saturation = raw_code[42] / 16.0;
			const float scale = raw_code[43] + raw_code[44] / 256.0;
			ref = ec_create_campfire(x, y, z, hue, saturation, LOD, scale);
			break;
		}
		case 0x01: // Cloud

		{
			const float hue = raw_code[41] / 256.0;
			const float saturation = raw_code[42] / 16.0;
			const float density = raw_code[43] + raw_code[44] / 256.0;
			ref = ec_create_cloud(x, y, z, hue, saturation, density, bounds, LOD);
			break;
		}
		case 0x02: // Fireflies

		{
			const float hue = raw_code[41] / 256.0;
			const float saturation = raw_code[42] / 16.0;
			const float density = raw_code[43] + raw_code[44] / 256.0;
			const float scale = raw_code[45] + raw_code[46] / 256.0;
			ref = ec_create_fireflies(x, y, z, hue, saturation, density, scale, bounds);
			break;
		}
		case 0x03: // Fountain

		{
			const float hue = raw_code[41] / 256.0;
			const float saturation = raw_code[42] / 16.0;
			const float scale = raw_code[43] + raw_code[44] / 256.0;
			const float base_height = raw_code[45] * 8.0 + raw_code[46] / 32.0;
			const int backlit = raw_code[47];
			ref = ec_create_fountain(x, y, z, hue, saturation, base_height, backlit, scale, LOD);
			break;
		}
		case 0x04: // Lamp

		{
			const float hue = raw_code[41] / 256.0;
			const float saturation = raw_code[42] / 16.0;
			const float scale = raw_code[43] + raw_code[44] / 256.0;
			ref = ec_create_lamp(x, y, z, hue, saturation, scale, LOD);
			break;
		}
		case 0x05: // Magic protection

		{
			const float hue = raw_code[41] / 256.0;
			const float saturation = raw_code[42] / 16.0;
			const float scale = raw_code[43] + raw_code[44] / 256.0;
			ref = ec_create_ongoing_magic_protection(x, y, z, hue, saturation, LOD, scale);
			break;
		}
		case 0x06: // Shield

		{
			const float hue = raw_code[41] / 256.0;
			const float saturation = raw_code[42] / 16.0;
			const float scale = raw_code[43] + raw_code[44] / 256.0;
			ref = ec_create_ongoing_shield(x, y, z, hue, saturation, LOD, scale);
			break;
		}
		case 0x07: // Magic immunity

		{
			const float hue = raw_code[41] / 256.0;
			const float saturation = raw_code[42] / 16.0;
			const float scale = raw_code[43] + raw_code[44] / 256.0;
			ref = ec_create_ongoing_magic_immunity(x, y, z, hue, saturation, LOD, scale);
			break;
		}
		case 0x08: // Poison

		{
			const float hue = raw_code[41] / 256.0;
			const float saturation = raw_code[42] / 16.0;
			const float scale = raw_code[43] + raw_code[44] / 256.0;
			ref = ec_create_ongoing_poison(x, y, z, hue, saturation, LOD, scale);
			break;
		}
		case 0x09: // Smoke

		{
			const float hue = raw_code[41] / 256.0;
			const float saturation = raw_code[42] / 16.0;
			const float density = raw_code[43] + raw_code[44] / 256.0;
			ref = ec_create_smoke(x, y, z, hue, saturation, density, LOD);
			break;
		}
		case 0x0A: // Teleporter

		{
			const float hue = raw_code[41] / 256.0;
			const float saturation = raw_code[42] / 16.0;
			const float scale = raw_code[43] + raw_code[44] / 256.0;
			ref = ec_create_teleporter(x, y, z, hue, saturation, scale, LOD);
			break;
		}
		case 0x0B: // Leaves

		{
			const float hue = raw_code[41] / 256.0;
			const float saturation = raw_code[42] / 16.0;
			const float density = raw_code[43] + raw_code[44] / 256.0;
			const float scale = raw_code[45] + raw_code[46] / 256.0;
			ref = ec_create_wind_leaves(x, y, z, hue, saturation, scale, density, bounds, 1.0, 0.0, 0.0);
			break;
		}
		case 0x0C: // Petals

		{
			const float hue = raw_code[41] / 256.0;
			const float saturation = raw_code[42] / 16.0;
			const float density = raw_code[43] + raw_code[44] / 256.0;
			const float scale = raw_code[45] + raw_code[46] / 256.0;
			ref = ec_create_wind_petals(x, y, z, hue, saturation, scale, density, bounds, 1.0, 0.0, 0.0);
			break;
		}
		case 0x0D: // Waterfall

		{
			//      const float hue = raw_code[41] / 256.0;
			//      const float saturation = raw_code[42] / 16.0;
			//      const float density = raw_code[43] + raw_code[44] / 256.0;
			//      const float base_height = raw_code[45] * 8.0 + raw_code[46] / 32.0;
			//      const float angle = raw_code[47] * ec::PI / 128.0;
			// Effect does not yet exist.
			break;
		}
		case 0x0E: // Bees

		{
			//      const float hue = raw_code[41] / 256.0;
			//      const float saturation = raw_code[42] / 16.0;
			//      const float density = raw_code[43] + raw_code[44] / 256.0;
			//      const float scale = raw_code[45] + raw_code[46] / 256.0;
			// Effect does not yet exist.
			break;
		}
		case 0x0F: // Portal

		{
			//      const float hue = raw_code[41] / 256.0;
			//      const float saturation = raw_code[42] / 16.0;
			//      const float scale = raw_code[43] + raw_code[44] / 256.0;
			//      const float angle = raw_code[45] * ec::PI / 128.0;
			// Effect does not yet exist.
			break;
		}
		case 0x10: // Candle

		{
			const float hue = raw_code[41] / 256.0;
			const float saturation = raw_code[42] / 16.0;
			const float scale = raw_code[43] + raw_code[44] / 256.0;
			ref = ec_create_candle(x, y, z, hue, saturation, scale, LOD);
			break;
		}
	}
	ec_free_bounds_list(bounds);
	return ref;
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
	ret->position = ec::Vec3(sx, sz, -(sy + X_OFFSET));
	ret->position2 = ec::Vec3(tx, tz, -(ty + Y_OFFSET));
	ret->effect = new ec::BreathEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, &general_obstructions_list, ec::BreathEffect::FIRE, LOD, scale);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_breath_ice(float sx, float sy, float sz, float tx, float ty, float tz, int LOD, float scale)
{
	if (!ec_in_range(sx, sy, sz, ec::BreathEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(sx, sz, -(sy + X_OFFSET));
	ret->position2 = ec::Vec3(tx, tz, -(ty + Y_OFFSET));
	ret->effect = new ec::BreathEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, &general_obstructions_list, ec::BreathEffect::ICE, LOD, scale);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_breath_poison(float sx, float sy, float sz, float tx, float ty, float tz, int LOD, float scale)
{
	if (!ec_in_range(sx, sy, sz, ec::BreathEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(sx, sz, -(sy + X_OFFSET));
	ret->position2 = ec::Vec3(tx, tz, -(ty + Y_OFFSET));
	ret->effect = new ec::BreathEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, &general_obstructions_list, ec::BreathEffect::POISON, LOD, scale);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_breath_magic(float sx, float sy, float sz, float tx, float ty, float tz, int LOD, float scale)
{
	if (!ec_in_range(sx, sy, sz, ec::BreathEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(sx, sz, -(sy + X_OFFSET));
	ret->position2 = ec::Vec3(tx, tz, -(ty + Y_OFFSET));
	ret->effect = new ec::BreathEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, &general_obstructions_list, ec::BreathEffect::MAGIC, LOD, scale);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_breath_lightning(float sx, float sy, float sz, float tx, float ty, float tz, int LOD, float scale)
{
	if (!ec_in_range(sx, sy, sz, ec::BreathEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(sx, sz, -(sy + X_OFFSET));
	ret->position2 = ec::Vec3(tx, tz, -(ty + Y_OFFSET));
	ret->effect = new ec::BreathEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, &general_obstructions_list, ec::BreathEffect::LIGHTNING, LOD, scale);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_breath_wind(float sx, float sy, float sz, float tx, float ty, float tz, int LOD, float scale)
{
	if (!ec_in_range(sx, sy, sz, ec::BreathEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(sx, sz, -(sy + X_OFFSET));
	ret->position2 = ec::Vec3(tx, tz, -(ty + Y_OFFSET));
	ret->effect = new ec::BreathEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, &general_obstructions_list, ec::BreathEffect::WIND, LOD, scale);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_campfire(float x, float y, float z, float hue_adjust, float saturation_adjust, int LOD, float scale)
{
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(x, z, -y);
	ret->effect = new ec::CampfireEffect(&eye_candy, &ret->dead, &ret->position, &fire_obstructions_list, hue_adjust, saturation_adjust, scale, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_cloud(float x, float y, float z, float hue_adjust, float saturation_adjust, float density, ec_bounds bounds, int LOD)
{
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->bounds = *(ec::SmoothPolygonBoundingRange*)bounds;
	ret->position = ec::Vec3(x, z, -y);
	ret->effect = new ec::CloudEffect(&eye_candy, &ret->dead, &ret->position, hue_adjust, saturation_adjust, density, &ret->bounds, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_fireflies(float x, float y, float z, float hue_adjust, float saturation_adjust, float density, float scale, ec_bounds bounds)
{
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->bounds = *(ec::SmoothPolygonBoundingRange*)bounds;
	ret->position = ec::Vec3(x, z, -y);
	ret->effect = new ec::FireflyEffect(&eye_candy, &ret->dead, &ret->position, &general_obstructions_list, hue_adjust, saturation_adjust, density, scale, &ret->bounds);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_fountain(float x, float y, float z, float hue_adjust, float saturation_adjust, float base_height, int backlit, float scale, int LOD)
{
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(x, z, -y);
	ret->effect = new ec::FountainEffect(&eye_candy, &ret->dead, &ret->position, hue_adjust, saturation_adjust, (bool)backlit, base_height, scale, LOD);
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

extern "C" ec_reference ec_create_lamp(float x, float y, float z, float hue_adjust, float saturation_adjust, float scale, int LOD)
{
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(x, z, -y);
	ret->effect = new ec::LampEffect(&eye_candy, &ret->dead, &ret->position, hue_adjust, saturation_adjust, scale, use_lamp_halo, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_candle(float x, float y, float z, float hue_adjust, float saturation_adjust, float scale, int LOD)
{
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(x, z, -y);
	ret->effect = new ec::CandleEffect(&eye_candy, &ret->dead, &ret->position, hue_adjust, saturation_adjust, scale, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_ongoing_magic_protection(float x, float y, float z, float hue_adjust, float saturation_adjust, int LOD, float scale)
{
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(x, z, -y);
	ret->effect = new ec::OngoingEffect(&eye_candy, &ret->dead, &ret->position, hue_adjust, saturation_adjust, ec::OngoingEffect::OG_MAGIC_PROTECTION, LOD, scale, BUFF_MAGIC_PROTECTION);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_ongoing_shield(float x, float y, float z, float hue_adjust, float saturation_adjust, int LOD, float scale)
{
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(x, z, -y);
	ret->effect = new ec::OngoingEffect(&eye_candy, &ret->dead, &ret->position, hue_adjust, saturation_adjust, ec::OngoingEffect::OG_SHIELD, LOD, scale, BUFF_SHIELD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_ongoing_magic_immunity(float x, float y, float z, float hue_adjust, float saturation_adjust, int LOD, float scale)
{
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(x, z, -y);
	ret->effect = new ec::OngoingEffect(&eye_candy, &ret->dead, &ret->position, hue_adjust, saturation_adjust, ec::OngoingEffect::OG_MAGIC_IMMUNITY, LOD, scale, BUFF_MAGIC_IMMUNITY);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_ongoing_poison(float x, float y, float z, float hue_adjust, float saturation_adjust, int LOD, float scale)
{
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(x, z, -y);
	ret->effect = new ec::OngoingEffect(&eye_candy, &ret->dead, &ret->position, hue_adjust, saturation_adjust, ec::OngoingEffect::OG_POISON, LOD, scale, 0); // BUFF_POISON isn't defined yet!
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

extern "C" ec_reference ec_create_selfmagic_magic_immunity(float x, float y, float z, int LOD)
{
	if (!ec_in_range(x, y, z, ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(x + X_OFFSET, z, -(y + Y_OFFSET));
	ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::MAGIC_IMMUNITY, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_smoke(float x, float y, float z, float hue_adjust, float saturation_adjust, float scale, int LOD)
{
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(x, z, -y);
	ret->effect = new ec::SmokeEffect(&eye_candy, &ret->dead, &ret->position, hue_adjust, saturation_adjust, scale, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_rabbit(float x, float y, float z, int LOD)
{
	if (!ec_in_range(x, y, z, ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
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
	for (auto& target: cast_reference->targets)
		target_ptrs.push_back(&target);
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
	for (auto& target: cast_reference->targets)
		target_ptrs.push_back(&target);
	cast_reference->effect = new ec::TargetMagicEffect(&eye_candy, &cast_reference->dead, &cast_reference->position, target_ptrs, ec::TargetMagicEffect::SMITE_SUMMONED, &general_obstructions_list, LOD);
	eye_candy.push_back_effect(cast_reference->effect);
}

extern "C" ec_reference ec_create_teleporter(float x, float y, float z, float hue_adjust, float saturation_adjust, float scale, int LOD)
{
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(x, z, -y);
	ret->effect = new ec::TeleporterEffect(&eye_candy, &ret->dead, &ret->position, hue_adjust, saturation_adjust, scale, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_wind_leaves(float x, float y, float z, float hue_adjust, float saturation_adjust, float scale, float density, ec_bounds bounds, float prevailing_wind_x, float prevailing_wind_y, float prevailing_wind_z)
{
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->bounds = *(ec::SmoothPolygonBoundingRange*)bounds;
	ret->position = ec::Vec3(x, z, -y);
	ret->position2 = ec::Vec3(prevailing_wind_x, prevailing_wind_z, -(prevailing_wind_y + 0.25));
	ret->effect = new ec::WindEffect(&eye_candy, &ret->dead, &ret->position, &general_obstructions_list, hue_adjust, saturation_adjust, scale, density, &ret->bounds, ec::WindEffect::LEAVES, ret->position2);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_wind_petals(float x, float y, float z, float hue_adjust, float saturation_adjust, float scale, float density, ec_bounds bounds, float prevailing_wind_x, float prevailing_wind_y, float prevailing_wind_z)
{
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->bounds = *(ec::SmoothPolygonBoundingRange*)bounds;
	ret->position = ec::Vec3(x, z, -y);
	ret->position2 = ec::Vec3(prevailing_wind_x, prevailing_wind_z, -(prevailing_wind_y + 0.25));
	ret->effect = new ec::WindEffect(&eye_candy, &ret->dead, &ret->position, &general_obstructions_list, hue_adjust, saturation_adjust, scale, density, &ret->bounds, ec::WindEffect::FLOWER_PETALS, ret->position2);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

#ifndef MAP_EDITOR
float ec_get_z(actor* _actor)
{
	if (_actor != NULL)
	{
		return get_tile_height(_actor->x_tile_pos, _actor->y_tile_pos);
	}
	else
	{
		return 0.0f;
	}
}

extern "C" void ec_actor_delete(actor* _actor)
{
	force_idle = true;
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
	for (ec_actor_obstructions::iterator iter = actor_obstructions.begin(); iter != actor_obstructions.end(); ++iter)
	{
		if ((*iter)->obstructing_actor == _actor)
		{
			for (std::vector<ec::Obstruction*>::iterator iter2 = general_obstructions_list.begin(); iter2 != general_obstructions_list.end(); ++iter2)
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

extern "C" void ec_add_actor_obstruction(actor* _actor, float force)
{
	ec_actor_obstruction* obstruction = new ec_actor_obstruction;
	obstruction->obstructing_actor = _actor;
	obstruction->center = ec::Vec3(_actor->x_pos + X_OFFSET, ec_get_z(_actor), -(_actor->y_pos + Y_OFFSET));
	obstruction->obstruction = new ec::CappedSimpleCylinderObstruction(&(obstruction->center), 0.55, force, obstruction->center.y, obstruction->center.y + 0.9);
	actor_obstructions.push_back(obstruction);
}

extern "C" void ec_remove_weapon(actor* _actor)
{
	force_idle = true;
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
		if (((*iter)->caster == _actor) &&
			(((*iter)->effect->get_type() == ec::EC_SWORD)
			|| (*iter)->effect->get_type() == ec::EC_STAFF))
		{
			(*iter)->effect->recall = true;
			(*iter)->caster = NULL;
			(*iter)->target = NULL;
			continue;
		}
	}
}

extern "C" void ec_remove_missile(int missile_id)
{
	force_idle = true;
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
		if ((*iter)->effect->get_type() == ec::EC_MISSILE &&
			(*iter)->missile_id == missile_id)
		{
			// don't recall the effect, let the particles glow
			//(*iter)->effect->recall = true;
			// update position one last time
			missile *mis = get_missile_ptr_from_id(missile_id);
			if (mis)
			{
				(*iter)->position.x = mis->position[0] + mis->direction[0] * mis->remaining_distance;
				(*iter)->position.y = mis->position[2] + mis->direction[2] * mis->remaining_distance;
				(*iter)->position.z = -(mis->position[1] + mis->direction[1] * mis->remaining_distance);
			}
			// set missile_id to -1 so the position of the effect is not updated anymore
			(*iter)->missile_id = -1;
			continue;
		}
	}
}

void ec_rename_missile(int old_id, int new_id)
{
	std::vector<ec_internal_reference*>::iterator it;
	for (it = references.begin(); it != references.end(); ++it)
	{
		if ((*it)->effect->get_type() == ec::EC_MISSILE && (*it)->missile_id == old_id)
		{
			(*it)->missile_id = new_id;
		}
	}
}

extern "C" ec_reference ec_create_breath_fire2(actor* caster, actor* target, int LOD, float scale)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::BreathEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->target = target;
	ret->casterbone = get_actor_bone_id(caster, head_bone);
	ret->targetbone = get_actor_bone_id(target, body_bottom_bone);
	set_vec3_actor_bone2(ret->position, caster, ret->casterbone);
	set_vec3_target_bone2(ret->position2, target, ret->targetbone);
	ret->effect = new ec::BreathEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, &general_obstructions_list, ec::BreathEffect::FIRE, LOD, scale);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_breath_ice2(actor* caster, actor* target, int LOD, float scale)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::BreathEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->target = target;
	ret->casterbone = get_actor_bone_id(caster, head_bone);
	ret->targetbone = get_actor_bone_id(target, body_bottom_bone);
	set_vec3_actor_bone2(ret->position, caster, ret->casterbone);
	set_vec3_target_bone2(ret->position2, target, ret->targetbone);
	ret->effect = new ec::BreathEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, &general_obstructions_list, ec::BreathEffect::ICE, LOD, scale);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_breath_poison2(actor* caster, actor* target, int LOD, float scale)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::BreathEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->target = target;
	ret->casterbone = get_actor_bone_id(caster, head_bone);
	ret->targetbone = get_actor_bone_id(target, body_bottom_bone);
	set_vec3_actor_bone2(ret->position, caster, ret->casterbone);
	set_vec3_target_bone2(ret->position2, target, ret->targetbone);
	ret->effect = new ec::BreathEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, &general_obstructions_list, ec::BreathEffect::POISON, LOD, scale);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_breath_magic2(actor* caster, actor* target, int LOD, float scale)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::BreathEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->target = target;
	ret->casterbone = get_actor_bone_id(caster, head_bone);
	ret->targetbone = get_actor_bone_id(target, body_bottom_bone);
	set_vec3_actor_bone2(ret->position, caster, ret->casterbone);
	set_vec3_target_bone2(ret->position2, target, ret->targetbone);
	ret->effect = new ec::BreathEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, &general_obstructions_list, ec::BreathEffect::MAGIC, LOD, scale);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_breath_lightning2(actor* caster, actor* target, int LOD, float scale)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::BreathEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->target = target;
	ret->casterbone = get_actor_bone_id(caster, hand_right_bone);
	ret->targetbone = get_actor_bone_id(target, head_bone);
	set_vec3_actor_bone2(ret->position, caster, ret->casterbone);
	set_vec3_target_bone2(ret->position2, target, ret->targetbone);
	ret->effect = new ec::BreathEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, &general_obstructions_list, ec::BreathEffect::LIGHTNING, LOD, scale);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_breath_wind2(actor* caster, actor* target, int LOD, float scale)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::BreathEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->target = target;
	ret->casterbone = get_actor_bone_id(caster, head_bone);
	ret->targetbone = get_actor_bone_id(target, body_bottom_bone);
	set_vec3_actor_bone2(ret->position, caster, ret->casterbone);
	set_vec3_target_bone2(ret->position2, target, ret->targetbone);
	ret->effect = new ec::BreathEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, &general_obstructions_list, ec::BreathEffect::WIND, LOD, scale);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_harvesting_radon_pouch2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::MineEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::HarvestingEffect(&eye_candy, &ret->dead, &ret->position, ec::HarvestingEffect::RADON_POUCH, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_harvesting_cavern_wall2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::MineEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::HarvestingEffect(&eye_candy, &ret->dead, &ret->position, ec::HarvestingEffect::CAVERN_WALL, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_harvesting_mother_nature2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::MineEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::HarvestingEffect(&eye_candy, &ret->dead, &ret->position, ec::HarvestingEffect::MOTHER_NATURE, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_harvesting_queen_of_nature2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::MineEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::HarvestingEffect(&eye_candy, &ret->dead, &ret->position, ec::HarvestingEffect::QUEEN_OF_NATURE, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_harvesting_bees2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::HarvestingEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, head_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
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

extern "C" ec_reference ec_create_harvesting_bag_of_gold2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::HarvestingEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, hand_right_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
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

extern "C" ec_reference ec_create_harvesting_rare_stone2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::HarvestingEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, hand_right_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::HarvestingEffect(&eye_candy, &ret->dead, &ret->position, ec::HarvestingEffect::RARE_STONE, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_harvesting_tool_break(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::HarvestingEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	set_vec3_actor_bone2(ret->position, ret->caster, get_actor_bone_id(caster, hand_right_bone));
	set_vec3_actor_bone2(ret->position2, ret->caster, get_actor_bone_id(caster, hand_left_bone));
	ret->effect = new ec::HarvestingEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::HarvestingEffect::TOOL_BREAK, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_glow_remote_heal(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::GlowEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, hand_right_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::GlowEffect(&eye_candy, &ret->dead, &ret->position, ec::GlowEffect::REMOTE_HEAL_GLOW, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_glow_harm(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::GlowEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, hand_right_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::GlowEffect(&eye_candy, &ret->dead, &ret->position, ec::GlowEffect::HARM_GLOW, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_glow_poison(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::GlowEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, hand_right_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::GlowEffect(&eye_candy, &ret->dead, &ret->position, ec::GlowEffect::POISON_GLOW, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_glow_level_up_oa(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::GlowEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, body_bottom_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::GlowEffect(&eye_candy, &ret->dead, &ret->position, ec::GlowEffect::LEVEL_UP_OA_GLOW, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_glow_level_up_att(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::GlowEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, body_bottom_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::GlowEffect(&eye_candy, &ret->dead, &ret->position, ec::GlowEffect::LEVEL_UP_ATT_GLOW, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_glow_level_up_def(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::GlowEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, body_bottom_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::GlowEffect(&eye_candy, &ret->dead, &ret->position, ec::GlowEffect::LEVEL_UP_DEF_GLOW, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_glow_level_up_default(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::GlowEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, body_bottom_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::GlowEffect(&eye_candy, &ret->dead, &ret->position, ec::GlowEffect::LEVEL_UP_DEFAULT_GLOW, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_glow_level_up_har(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::GlowEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, body_bottom_bone);
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::GlowEffect(&eye_candy, &ret->dead, &ret->position, ec::GlowEffect::LEVEL_UP_HAR_GLOW, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_glow_level_up_alc_left(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::GlowEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, hand_left_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::GlowEffect(&eye_candy, &ret->dead, &ret->position, ec::GlowEffect::LEVEL_UP_ALC_GLOW_L, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_glow_level_up_alc_right(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::GlowEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, hand_right_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::GlowEffect(&eye_candy, &ret->dead, &ret->position, ec::GlowEffect::LEVEL_UP_ALC_GLOW_R, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_glow_level_up_mag(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::GlowEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, body_bottom_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::GlowEffect(&eye_candy, &ret->dead, &ret->position, ec::GlowEffect::LEVEL_UP_MAG_GLOW, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_glow_level_up_pot_left(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::GlowEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, hand_left_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::GlowEffect(&eye_candy, &ret->dead, &ret->position, ec::GlowEffect::LEVEL_UP_POT_GLOW_L, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_glow_level_up_pot_right(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::GlowEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, hand_right_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::GlowEffect(&eye_candy, &ret->dead, &ret->position, ec::GlowEffect::LEVEL_UP_POT_GLOW_R, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_glow_level_up_sum(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::GlowEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, head_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::GlowEffect(&eye_candy, &ret->dead, &ret->position, ec::GlowEffect::LEVEL_UP_SUM_GLOW, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_glow_level_up_man_left(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::GlowEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, hand_left_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::GlowEffect(&eye_candy, &ret->dead, &ret->position, ec::GlowEffect::LEVEL_UP_MAN_GLOW_L, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_glow_level_up_man_right(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::GlowEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, hand_right_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::GlowEffect(&eye_candy, &ret->dead, &ret->position, ec::GlowEffect::LEVEL_UP_MAN_GLOW_R, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_glow_level_up_cra_left(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::GlowEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, hand_left_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::GlowEffect(&eye_candy, &ret->dead, &ret->position, ec::GlowEffect::LEVEL_UP_CRA_GLOW_L, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_glow_level_up_cra_right(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::GlowEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, hand_right_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::GlowEffect(&eye_candy, &ret->dead, &ret->position, ec::GlowEffect::LEVEL_UP_CRA_GLOW_R, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_glow_level_up_eng_left(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::GlowEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, hand_left_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::GlowEffect(&eye_candy, &ret->dead, &ret->position, ec::GlowEffect::LEVEL_UP_ENG_GLOW_L, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_glow_level_up_eng_right(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::GlowEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, hand_right_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::GlowEffect(&eye_candy, &ret->dead, &ret->position, ec::GlowEffect::LEVEL_UP_ENG_GLOW_R, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_glow_level_up_tai_left(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::GlowEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, hand_left_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::GlowEffect(&eye_candy, &ret->dead, &ret->position, ec::GlowEffect::LEVEL_UP_TAI_GLOW_L, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_glow_level_up_tai_right(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::GlowEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, hand_right_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::GlowEffect(&eye_candy, &ret->dead, &ret->position, ec::GlowEffect::LEVEL_UP_TAI_GLOW_R, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_glow_level_up_ran(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::GlowEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, head_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::GlowEffect(&eye_candy, &ret->dead, &ret->position, ec::GlowEffect::LEVEL_UP_RAN_GLOW, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_ongoing_magic_protection2(actor *caster, float hue_adjust, float saturation_adjust, int LOD, float scale)
{
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, head_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::OngoingEffect(&eye_candy, &ret->dead, &ret->position, hue_adjust, saturation_adjust, ec::OngoingEffect::OG_MAGIC_PROTECTION, LOD, scale, BUFF_MAGIC_PROTECTION);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_ongoing_shield2(actor *caster, float hue_adjust, float saturation_adjust, int LOD, float scale)
{
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, head_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::OngoingEffect(&eye_candy, &ret->dead, &ret->position, hue_adjust, saturation_adjust, ec::OngoingEffect::OG_SHIELD, LOD, scale, BUFF_SHIELD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_ongoing_magic_immunity2(actor *caster, float hue_adjust, float saturation_adjust, int LOD, float scale)
{
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, head_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::OngoingEffect(&eye_candy, &ret->dead, &ret->position, hue_adjust, saturation_adjust, ec::OngoingEffect::OG_MAGIC_IMMUNITY, LOD, scale, BUFF_MAGIC_IMMUNITY);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_ongoing_poison2(actor *caster, float hue_adjust, float saturation_adjust, int LOD, float scale)
{
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, body_bottom_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::OngoingEffect(&eye_candy, &ret->dead, &ret->position, hue_adjust, saturation_adjust, ec::OngoingEffect::OG_POISON, LOD, scale, 0); // BUFF_POISON isn't defined yet!
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_ongoing_harvesting2(actor *caster, float hue_adjust, float saturation_adjust, int LOD, float scale)
{
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, hand_right_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::OngoingEffect(&eye_candy, &ret->dead, &ret->position, hue_adjust, saturation_adjust, ec::OngoingEffect::OG_HARVEST, LOD, scale, 0); // BUFF_HARVEST isn't defined yet!
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_selfmagic_heal2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, body_top_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::HEAL, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_selfmagic_magic_protection(float x, float y, float z, int LOD)
{
	if (!ec_in_range(x, y, z, ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(x + X_OFFSET, z, -(y + Y_OFFSET));
	ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::MAGIC_PROTECTION, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_selfmagic_magic_protection2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, body_top_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::MAGIC_PROTECTION, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_selfmagic_shield(float x, float y, float z, int LOD)
{
	if (!ec_in_range(x, y, z, ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(x + X_OFFSET, z, -(y + Y_OFFSET));
	ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::SHIELD, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_selfmagic_shield_generic(actor* caster, int LOD, special_effect_enum type)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, body_bottom_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	switch (type)
	{
		case SPECIAL_EFFECT_SHIELD:
			ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::SHIELD, LOD);
			break;
		case SPECIAL_EFFECT_HEATSHIELD:
			ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::HEATSHIELD, LOD);
			break;
		case SPECIAL_EFFECT_COLDSHIELD:
			ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::COLDSHIELD, LOD);
			break;
		case SPECIAL_EFFECT_RADIATIONSHIELD:
			ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::RADIATIONSHIELD, LOD);
			break;
		default:
			break;
	}
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_selfmagic_restoration2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, body_bottom_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::RESTORATION, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_selfmagic_bones_to_gold(float x, float y, float z, int LOD)
{
	if (!ec_in_range(x, y, z, ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(x + X_OFFSET, z, -(y + Y_OFFSET));
	ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::BONES_TO_GOLD, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_selfmagic_bones_to_gold2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, body_bottom_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::BONES_TO_GOLD, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_selfmagic_teleport_to_the_portals_room2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::TELEPORT_TO_THE_PORTALS_ROOM, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_selfmagic_magic_immunity2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, body_top_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::MAGIC_IMMUNITY, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_alert2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SelfMagicEffect(&eye_candy, &ret->dead, &ret->position, ec::SelfMagicEffect::ALERT, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_rabbit2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::RABBIT, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_rat2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::RAT, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_beaver2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::BEAVER, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_skunk2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::SKUNK, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_racoon2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::RACOON, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_deer2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::DEER, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_green_snake2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::GREEN_SNAKE, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_red_snake2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::RED_SNAKE, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_brown_snake2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::BROWN_SNAKE, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_fox2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::FOX, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_boar2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::BOAR, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_wolf2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::WOLF, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_skeleton2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::SKELETON, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_small_gargoyle2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::SMALL_GARGOYLE, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_medium_gargoyle2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::MEDIUM_GARGOYLE, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_large_gargoyle2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::LARGE_GARGOYLE, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_puma2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::PUMA, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_female_goblin2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::FEMALE_GOBLIN, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_polar_bear2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::POLAR_BEAR, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_bear2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::BEAR, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_armed_male_goblin2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::ARMED_MALE_GOBLIN, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_armed_skeleton2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::ARMED_SKELETON, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_female_orc2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::FEMALE_ORC, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_male_orc2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::MALE_ORC, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_armed_female_orc2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::ARMED_FEMALE_ORC, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_armed_male_orc2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::ARMED_MALE_ORC, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_cyclops2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::CYCLOPS, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_fluffy2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::FLUFFY, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_phantom_warrior2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::PHANTOM_WARRIOR, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_mountain_chimeran2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::MOUNTAIN_CHIMERAN, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_yeti2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::YETI, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_arctic_chimeran2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::ARCTIC_CHIMERAN, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_giant2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::GIANT, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_giant_snake2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::GIANT_SNAKE, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_spider2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::SPIDER, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_summon_tiger2(actor* caster, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::SelfMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->effect = new ec::SummonEffect(&eye_candy, &ret->dead, &ret->position, ec::SummonEffect::TIGER, LOD);
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

extern "C" ec_reference ec_create_staff_of_the_mage(actor* _actor, int LOD)
{
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = _actor;
	get_staff_position(_actor, ret->position);
	ret->effect = new ec::StaffEffect(&eye_candy, &ret->dead, &ret->position, ec::StaffEffect::STAFF_OF_THE_MAGE, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_staff_of_protection(actor* _actor, int LOD)
{
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = _actor;
	get_staff_position(_actor, ret->position);
	ret->effect = new ec::StaffEffect(&eye_candy, &ret->dead, &ret->position, ec::StaffEffect::STAFF_OF_PROTECTION, LOD);
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

extern "C" ec_reference ec_create_targetmagic_remote_heal2(actor* caster, actor* target, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::TargetMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->target = target;
	ret->casterbone = get_actor_bone_id(caster, hand_right_bone);
	ret->targetbone = get_actor_bone_id(target, body_bottom_bone);
	set_vec3_actor_bone2(ret->position, caster, ret->casterbone);
	set_vec3_target_bone2(ret->position2, target, ret->targetbone);
	ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::REMOTE_HEAL, &general_obstructions_list, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_targetmagic_poison2(actor* caster, actor* target, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::TargetMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, hand_right_bone);;
	ret->target = target;
	set_vec3_actor_bone(ret->position, caster, ret->casterbone, ec::Vec3(0.0, 0.0, 0.0));
	ret->position2 = ec::Vec3(target->x_pos, ec_get_z(target), -target->y_pos);
	ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::POISON, &general_obstructions_list, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_targetmagic_teleport_to_range2(actor* caster, actor* target, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::TargetMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->target = target;
	ret->position = ec::Vec3(caster->x_pos + X_OFFSET, ec_get_z(caster), -(caster->y_pos + Y_OFFSET));
	ret->position2 = ec::Vec3(target->x_pos + X_OFFSET, ec_get_z(target), -(target->y_pos + Y_OFFSET));
	ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::TELEPORT_TO_RANGE, &general_obstructions_list, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_targetmagic_harm2(actor* caster, actor* target, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::TargetMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, hand_right_bone);
	ret->target = target;
	set_vec3_actor_bone(ret->position, caster, ret->casterbone, ec::Vec3(0.0, 0.0, 0.0));
	ret->position2 = ec::Vec3(target->x_pos, ec_get_z(target), -target->y_pos);
	ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::HARM, &general_obstructions_list, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_targetmagic_life_drain2(actor* caster, actor* target, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::TargetMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, head_bone); // spell target
	ret->target = target;
	ret->targetbone = get_actor_bone_id(target, head_bone); // spell source
	set_vec3_actor_bone2(ret->position, caster, ret->casterbone);
	set_vec3_target_bone2(ret->position2, target, ret->targetbone);
	ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::LIFE_DRAIN, &general_obstructions_list, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_targetmagic_drain_mana2(actor* caster, actor* target, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::TargetMagicEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, head_bone); // spell target
	ret->target = target;
	ret->targetbone = get_actor_bone_id(target, head_bone); // spell source
	set_vec3_actor_bone2(ret->position, caster, ret->casterbone);
	set_vec3_target_bone2(ret->position2, target, ret->targetbone);
	ret->effect = new ec::TargetMagicEffect(&eye_candy, &ret->dead, &ret->position, &ret->position2, ec::TargetMagicEffect::DRAIN_MANA, &general_obstructions_list, LOD);
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_mine_detonate(float x, float y, float z, int mine_type, int LOD)
{
	if (!ec_in_range(x, y, z, ec::MineEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->position = ec::Vec3(x, z, -y);
	if (mine_type == MINE_TYPE_SMALL_MINE)
	{
		ret->effect = new ec::MineEffect(&eye_candy, &ret->dead, &ret->position, ec::MineEffect::DETONATE_TYPE1_SMALL, LOD);
	}
	else if (mine_type == MINE_TYPE_MEDIUM_MINE)
	{
		ret->effect = new ec::MineEffect(&eye_candy, &ret->dead, &ret->position, ec::MineEffect::DETONATE_TYPE1_MEDIUM, LOD);
	}
	else if (mine_type == MINE_TYPE_HIGH_EXPLOSIVE_MINE)
	{
		ret->effect = new ec::MineEffect(&eye_candy, &ret->dead, &ret->position, ec::MineEffect::DETONATE_TYPE1_LARGE, LOD);
	}
	else if (mine_type == MINE_TYPE_TRAP)
	{
		ret->effect = new ec::MineEffect(&eye_candy, &ret->dead, &ret->position, ec::MineEffect::DETONATE_TRAP, LOD);
	}
	else if (mine_type == MINE_TYPE_CALTROP)
	{
		ret->effect = new ec::MineEffect(&eye_candy, &ret->dead, &ret->position, ec::MineEffect::DETONATE_CALTROP, LOD);
	}
	else if (mine_type == MINE_TYPE_POISONED_CALTROP)
	{
		ret->effect = new ec::MineEffect(&eye_candy, &ret->dead, &ret->position, ec::MineEffect::DETONATE_CALTROP_POISON, LOD);
	}
	else if (mine_type == MINE_TYPE_MANA_DRAINER)
	{
		ret->effect = new ec::MineEffect(&eye_candy, &ret->dead, &ret->position, ec::MineEffect::DETONATE_MANA_DRAINER, LOD);
	}
	else if (mine_type == MINE_TYPE_MANA_BURNER)
	{
		ret->effect = new ec::MineEffect(&eye_candy, &ret->dead, &ret->position, ec::MineEffect::DETONATE_MANA_BURNER, LOD);
	}
	else if (mine_type == MINE_TYPE_UNINVIZIBILIZER)
	{
		ret->effect = new ec::MineEffect(&eye_candy, &ret->dead, &ret->position, ec::MineEffect::DETONATE_UNINVIZIBILIZER, LOD);
	}
	else if (mine_type == MINE_TYPE_MAGIC_IMMUNITY_REMOVAL)
	{
		ret->effect = new ec::MineEffect(&eye_candy, &ret->dead, &ret->position, ec::MineEffect::DETONATE_MAGIC_IMMUNITY_REMOVAL, LOD);
	}
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_mine_detonate2(actor* caster, int mine_type, int LOD)
{
	if (!ec_in_range(caster->x_pos, caster->y_pos, ec_get_z(caster), ec::MineEffect::get_max_end_time()))
		return NULL;
	ec_internal_reference* ret = (ec_internal_reference*)ec_create_generic();
	ret->caster = caster;
	ret->casterbone = get_actor_bone_id(caster, head_bone);
	set_vec3_actor_bone2(ret->position, ret->caster, ret->casterbone);
	ret->position = ec::Vec3(ret->position.x, ec_get_z(caster), ret->position.z);
	if (mine_type == MINE_TYPE_SMALL_MINE)
	{
		ret->effect = new ec::MineEffect(&eye_candy, &ret->dead, &ret->position, ec::MineEffect::DETONATE_TYPE1_SMALL, LOD);
	}
	else if (mine_type == MINE_TYPE_MEDIUM_MINE)
	{
		ret->effect = new ec::MineEffect(&eye_candy, &ret->dead, &ret->position, ec::MineEffect::DETONATE_TYPE1_MEDIUM, LOD);
	}
	else if (mine_type == MINE_TYPE_HIGH_EXPLOSIVE_MINE)
	{
		ret->effect = new ec::MineEffect(&eye_candy, &ret->dead, &ret->position, ec::MineEffect::DETONATE_TYPE1_LARGE, LOD);
	}
	else if (mine_type == MINE_TYPE_TRAP)
	{
		ret->effect = new ec::MineEffect(&eye_candy, &ret->dead, &ret->position, ec::MineEffect::DETONATE_TRAP, LOD);
	}
	else if (mine_type == MINE_TYPE_CALTROP)
	{
		ret->effect = new ec::MineEffect(&eye_candy, &ret->dead, &ret->position, ec::MineEffect::DETONATE_CALTROP, LOD);
	}
	else if (mine_type == MINE_TYPE_POISONED_CALTROP)
	{
		ret->effect = new ec::MineEffect(&eye_candy, &ret->dead, &ret->position, ec::MineEffect::DETONATE_CALTROP_POISON, LOD);
	}
	else if (mine_type == MINE_TYPE_MANA_DRAINER)
	{
		ret->effect = new ec::MineEffect(&eye_candy, &ret->dead, &ret->position, ec::MineEffect::DETONATE_MANA_DRAINER, LOD);
	}
	else if (mine_type == MINE_TYPE_MANA_BURNER)
	{
		ret->effect = new ec::MineEffect(&eye_candy, &ret->dead, &ret->position, ec::MineEffect::DETONATE_MANA_BURNER, LOD);
	}
	else if (mine_type == MINE_TYPE_UNINVIZIBILIZER)
	{
		ret->effect = new ec::MineEffect(&eye_candy, &ret->dead, &ret->position, ec::MineEffect::DETONATE_UNINVIZIBILIZER, LOD);
	}
	else if (mine_type == MINE_TYPE_MAGIC_IMMUNITY_REMOVAL)
	{
		ret->effect = new ec::MineEffect(&eye_candy, &ret->dead, &ret->position, ec::MineEffect::DETONATE_MAGIC_IMMUNITY_REMOVAL, LOD);
	}
	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

extern "C" ec_reference ec_create_missile_effect(int missile_id, int LOD, int hitOrMiss)
{
	missile *mis = get_missile_ptr_from_id(missile_id);
	ec_internal_reference* ret;

	if (mis == NULL ||
		missiles_defs[mis->type].effect < MAGIC_MISSILE ||
		missiles_defs[mis->type].effect> EXPLOSIVE_MISSILE
	)
		return NULL;

	ret = (ec_internal_reference*)ec_create_generic();
	ret->missile_id = missile_id;
	ret->position.x = mis->position[0];
	ret->position.y = mis->position[2];
	ret->position.z = -mis->position[1];

	switch(missiles_defs[mis->type].effect)
	{
		case MAGIC_MISSILE:
			ret->effect = new ec::MissileEffect(&eye_candy, &ret->dead, &ret->position, ec::MissileEffect::MAGIC, LOD, hitOrMiss);
			break;

		case FIRE_MISSILE:
			ret->effect = new ec::MissileEffect(&eye_candy, &ret->dead, &ret->position, ec::MissileEffect::FIRE, LOD, hitOrMiss);
			break;

		case ICE_MISSILE:
			ret->effect = new ec::MissileEffect(&eye_candy, &ret->dead, &ret->position, ec::MissileEffect::ICE, LOD, hitOrMiss);
			break;

		case EXPLOSIVE_MISSILE:
			ret->effect = new ec::MissileEffect(&eye_candy, &ret->dead, &ret->position, ec::MissileEffect::EXPLOSIVE, LOD, hitOrMiss);
			break;

		default:
			delete ret;
			return NULL;
	}

	eye_candy.push_back_effect(ret->effect);
	return (ec_reference)ret;
}

/* stop or restart the harvesting eye candy effect depending on the harvesting state */
extern "C" void check_harvesting_effect()
{
	/* if the harvesting effect is on but we're not harvesting, stop it */
	if ((!now_harvesting() || !use_harvesting_eye_candy) && (harvesting_effect_reference != NULL))
	{
		ec_recall_effect(harvesting_effect_reference);
		harvesting_effect_reference = NULL;
	}
	/* but if we are harvesting but there is no effect, start it if wanted */
	else if (now_harvesting() && use_eye_candy && use_harvesting_eye_candy && (harvesting_effect_reference == NULL))
	{
		actor *act;
		LOCK_ACTORS_LISTS();
		act = get_actor_ptr_from_id(yourself);
		if (act != NULL)
			harvesting_effect_reference = ec_create_ongoing_harvesting2(act, 1.0, 1.0, (poor_man ? 6 : 10), 1.0);
		UNLOCK_ACTORS_LISTS();
	}
}
#endif //!MAP_EDITOR
///////////////////////////////////////////////////////////////////////////////

