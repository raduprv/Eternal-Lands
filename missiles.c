#include "3d_objects.h"
#include "actor_scripts.h"
#include "asc.h"
#include "cal.h"
#include "cal3d_wrapper.h"
#include "context_menu.h"
#include "e3d.h"
#include "elwindows.h"
#include "errors.h"
#include "eye_candy_wrapper.h"
#include "gamewin.h"
#include "gl_init.h"
#include "global.h"
#include "init.h"
#include "missiles.h"
#include "skeletons.h"
#include "stats.h"
#include "session.h"
#include "tiles.h"
#include "translate.h"
#include "vmath.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#define MAX_LOST_MISSILES 512
#define LOST_MISSILE_MAX_LIFE 120000
#define EPSILON 1E-4

typedef struct {
	int obj_3d_id;
	Uint32 end_time;
} lost_missile;

const float arrow_color[4] = {0.8, 0.8, 0.8, 1.0};
const float arrow_border_color[4] = {0.8, 0.8, 0.8, 0.5};
const float miss_color[4] = {0.9, 0.6, 0.6, 1.0};
const float critical_color[4] = {0.6, 0.9, 1.0, 1.0};
const float critical_border1_color[4] = {0.3, 0.7, 1.0, 0.6};
const float critical_border2_color[4] = {0.0, 0.5, 1.0, 0.4};

missile missiles_list[MAX_MISSILES];
missile_type missiles_defs[MAX_MISSILES_DEFS];
lost_missile lost_missiles_list[MAX_LOST_MISSILES];

int missiles_count = 0;
int begin_lost_missiles = -1;
int end_lost_missiles = -1;

int range_total_shots = 0;
int range_success_hits = 0;
int range_critical_hits = 0;

#ifdef MISSILES_DEBUG
FILE *missiles_log = NULL;

void missiles_open_log()
{
	char log_name[1024];
	char starttime[200], sttime[200];
	struct tm *l_time; time_t c_time;

	safe_snprintf(log_name, 1024, "%smissiles_log.txt", configdir);

	missiles_log = fopen(log_name, "w");

	if (missiles_log == NULL)
	{
		fprintf (stderr, "Unable to open log file \"%s\"\n", log_name);
		exit (1);
	}

	time (&c_time);
	l_time = localtime (&c_time);
	strftime(sttime, sizeof(sttime), "\n\nLog started at %Y-%m-%d %H:%M:%S localtime", l_time);
	safe_snprintf(starttime, sizeof(starttime), "%s (%s)\n\n", sttime, tzname[l_time->tm_isdst>0]);
	fwrite (starttime, strlen(starttime), 1, missiles_log);
}

void missiles_log_message_func(const char *format, ...)
{
	va_list ap;
	struct tm *l_time; time_t c_time;
	char logmsg[512];
	char errmsg[512];

	va_start(ap, format);
	vsnprintf(errmsg, 512, format, ap);
	va_end(ap);

	if (missiles_log == NULL)
		missiles_open_log();

	time(&c_time);
	l_time = localtime(&c_time);
	strftime(logmsg, sizeof(logmsg), "[%H:%M:%S] ", l_time);
	strcat(logmsg, errmsg);

	if(format[strlen(format)-1] != '\n') {
		strcat(logmsg, "\n");
	}
	fprintf(missiles_log, "%s", logmsg);
	fflush (missiles_log);
}
#endif // MISSILES_DEBUG

void missiles_clear()
{
	missiles_count = 0;
	begin_lost_missiles = end_lost_missiles = -1;
}

int missiles_add(int type,
				 float origin[3],
				 float target[3],
				 float shift,
				 MissileShotType shot_type)
{
	missile *mis;
	missile_type *mis_type = &missiles_defs[type];
	float direction[3];
	float dist;
        
	if (missiles_count >= MAX_MISSILES) {
		LOG_ERROR("too many missiles, can't add the last one!");
		return MAX_MISSILES;
	}

	missiles_log_message("missiles_add: origin=(%.2f,%.2f,%.2f), target=(%.2f,%.2f,%.2f) type %u",
						 origin[0], origin[1], origin[2], target[0], target[1], target[2], shot_type);
        
	direction[0] = target[0] - origin[0];
	direction[1] = target[1] - origin[1];
	direction[2] = target[2] - origin[2];
	dist = sqrtf(direction[0]*direction[0] +
				 direction[1]*direction[1] +
				 direction[2]*direction[2]);

	if (fabs(dist) < EPSILON) {
		LOG_ERROR("null length shot detected between (%f,%f,%f) and (%f,%f,%f), not adding the missile!",
				  origin[0], origin[1], origin[2],
				  target[0], target[1], target[2]);
		return MAX_MISSILES;
	}
    else {
        missiles_log_message("missiles_add: distance of the shot: %f", dist);
    }
        
	mis = &missiles_list[missiles_count++];

	mis->type = type;
	mis->shot_type = shot_type;
	memcpy(mis->position, origin, sizeof(float)*3);
	memcpy(mis->direction, direction, sizeof(float)*3);
	mis->remaining_distance = dist;
	mis->direction[0] /= mis->remaining_distance;
	mis->direction[1] /= mis->remaining_distance;
	mis->direction[2] /= mis->remaining_distance;
	mis->speed = mis_type->speed;
	mis->trace_length = mis_type->trace_length;
	mis->covered_distance = 0;
	mis->remaining_distance += shift;
        
	if (use_eye_candy == 1)
	{
		ec_create_missile_effect(missiles_count-1, (poor_man ? 6 : 10), shot_type);
	}

	return missiles_count-1;
}

void missiles_add_lost(int obj_id)
{
	if (begin_lost_missiles < 0) {
		end_lost_missiles = begin_lost_missiles = 0;
	}
	else {
		end_lost_missiles = (end_lost_missiles + 1) % MAX_LOST_MISSILES;
		if (end_lost_missiles == begin_lost_missiles) {
			destroy_3d_object(lost_missiles_list[begin_lost_missiles].obj_3d_id);
			begin_lost_missiles = (begin_lost_missiles + 1) % MAX_LOST_MISSILES;
		}
	}
	lost_missiles_list[end_lost_missiles].obj_3d_id = obj_id;
	lost_missiles_list[end_lost_missiles].end_time = cur_time + LOST_MISSILE_MAX_LIFE;
}

void missiles_remove(int missile_id)
{
	missile *mis = get_missile_ptr_from_id(missile_id);

	if (!mis) {
		LOG_ERROR("missile id %i is out of range!", missile_id);
		return;
	}

    /* if the shot is missed and if it has travel a distance which is under
     * the distance used on server side (20.0), we display a stuck arrow
     * where the shot has ended */
	if (mis->shot_type == MISSED_SHOT &&
		mis->covered_distance < 19.0) {
        float x_rot = 0.0;
		float y_rot = -asinf(mis->direction[2])*180.0/M_PI;
		float z_rot = atan2f(mis->direction[1], mis->direction[0])*180.0/M_PI;
		float dist = -mis->remaining_distance;
		int obj_3d_id = -1;
		// don't change the missile position after it arrived at the target position
		//mis->position[0] -= mis->direction[0] * dist;
		//mis->position[1] -= mis->direction[1] * dist;
		//mis->position[2] -= mis->direction[2] * dist;
		missiles_log_message("adding a lost missile at (%f,%f,%f) with rotation (%f,%f,%f)",
							 mis->position[0] - mis->direction[0] * dist, 
							 mis->position[1] - mis->direction[1] * dist, 
							 mis->position[2] - mis->direction[2] * dist,
                             x_rot, y_rot, z_rot);
		obj_3d_id = add_e3d(missiles_defs[mis->type].lost_mesh,
							mis->position[0] - mis->direction[0] * dist, 
							mis->position[1] - mis->direction[1] * dist, 
							mis->position[2] - mis->direction[2] * dist,
							x_rot, y_rot, z_rot, 0, 0, 1.0, 1.0, 1.0, 1);
		if (obj_3d_id >= 0)
			missiles_add_lost(obj_3d_id);
	}
        
	ec_remove_missile(missile_id);

	--missiles_count;
	if (missile_id < missiles_count) {
		memcpy(&missiles_list[missile_id],
			   &missiles_list[missiles_count],
			   sizeof(missile));
		ec_rename_missile(missiles_count, missile_id);
	}
}

void missiles_update()
{
	int i;
	static int last_update = 0;
	float time_diff = (cur_time - last_update) / 1000.0;
        
	for (i = 0; i < missiles_count; ) {
		missile *mis = &missiles_list[i];
		float dist = mis->speed * time_diff;
		mis->position[0] += mis->direction[0] * dist;
		mis->position[1] += mis->direction[1] * dist;
		mis->position[2] += mis->direction[2] * dist;
		mis->covered_distance += dist;
		mis->remaining_distance -= dist;
		if (mis->remaining_distance < -mis->trace_length)
			missiles_remove(i);
		else
			++i;
	}

	while (begin_lost_missiles >= 0 &&
		   cur_time > lost_missiles_list[begin_lost_missiles].end_time) {
		destroy_3d_object(lost_missiles_list[begin_lost_missiles].obj_3d_id);
		if (begin_lost_missiles == end_lost_missiles)
			begin_lost_missiles = end_lost_missiles = -1;
		else
			begin_lost_missiles = (begin_lost_missiles + 1) % MAX_LOST_MISSILES;
	}

	last_update = cur_time;
}

void missiles_draw_single(missile *mis, const float color[4])
{
	float z_shift = 0.0;

/* 	if (mis->shot_type == MISSED_SHOT) */
/* 		z_shift = cosf(mis->covered_distance*M_PI/2.0)/10.0; */

    if (mis->covered_distance < mis->trace_length) {
        glColor4f(color[0], color[1], color[2],
                  color[3] * (mis->trace_length - mis->covered_distance) / mis->trace_length);
        glVertex3f(mis->position[0] - mis->covered_distance * mis->direction[0],
                   mis->position[1] - mis->covered_distance * mis->direction[1],
                   mis->position[2] - mis->covered_distance * mis->direction[2]);
    }
    else {
        glColor4f(color[0], color[1], color[2], 0.0);
        glVertex3f(mis->position[0] - mis->trace_length * mis->direction[0],
                   mis->position[1] - mis->trace_length * mis->direction[1],
                   mis->position[2] - mis->trace_length * mis->direction[2]);
    }
    if (mis->remaining_distance < 0.0) {
        glColor4f(color[0], color[1], color[2],
                  color[3] * (mis->trace_length + mis->remaining_distance) / mis->trace_length);
        glVertex3f(mis->position[0] + mis->remaining_distance * mis->direction[0],
                   mis->position[1] + mis->remaining_distance * mis->direction[1],
                   mis->position[2] + mis->remaining_distance * mis->direction[2] + z_shift);
    }
    else {
        glColor4f(color[0], color[1], color[2], color[3]);
        glVertex3f(mis->position[0], mis->position[1], mis->position[2] + z_shift);
    }
}

void missiles_draw()
{
	int i;

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_BLEND);
	glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	glLineWidth(7.0);
	glBegin(GL_LINES);
	for (i = missiles_count; i--;) {
		if (missiles_list[i].shot_type == CRITICAL_SHOT)
			missiles_draw_single(&missiles_list[i], critical_border2_color);
	}
	glEnd();

	glLineWidth(3.0);
	glBegin(GL_LINES);
	for (i = missiles_count; i--;) {
		if (missiles_list[i].shot_type == NORMAL_SHOT)
			missiles_draw_single(&missiles_list[i], arrow_border_color);
		else if (missiles_list[i].shot_type == CRITICAL_SHOT)
			missiles_draw_single(&missiles_list[i], critical_border1_color);
	}
	glEnd();

	glLineWidth(1.0);
	glBegin(GL_LINES);
	for (i = missiles_count; i--;) {
		if (missiles_list[i].shot_type == NORMAL_SHOT)
			missiles_draw_single(&missiles_list[i], arrow_color);
		else if (missiles_list[i].shot_type == CRITICAL_SHOT)
			missiles_draw_single(&missiles_list[i], critical_color);
	}
	glEnd();

	glLineWidth(2.0);
	glLineStipple(1, 0x003F);
	glEnable(GL_LINE_STIPPLE);
	glBegin(GL_LINES);
	for (i = missiles_count; i--;)
		if (missiles_list[i].shot_type == MISSED_SHOT)
			missiles_draw_single(&missiles_list[i], miss_color);
	glEnd();
	glDisable(GL_LINE_STIPPLE);

	glPopAttrib();
}

float missiles_compute_actor_rotation(float *out_h_rot, float *out_v_rot,
									  actor *in_act, float *in_target)
{
	float cz, sz;
	float from[3], to[3], tmp[3], origin[3];
	float actor_rotation = 0;
	float act_z_rot = in_act->z_rot;

	if (in_act->rotating) {
        missiles_log_message("%s (%d): already rotating so we get the final position first",
                             in_act->actor_name, in_act->actor_id);
		act_z_rot += in_act->rotate_z_speed * in_act->rotate_time_left;
	}

	// we first compute the global rotation
	cz = cosf((act_z_rot) * M_PI/180.0);
	sz = sinf((act_z_rot) * M_PI/180.0);
	tmp[0] = in_target[0] - in_act->x_pos - 0.25;
	tmp[1] = in_target[1] - in_act->y_pos - 0.25;
	tmp[2] = 0.0;
	Normalize(tmp, tmp);

	actor_rotation = asinf(tmp[0] * cz - tmp[1] * sz) * 180.0/M_PI;
	if (tmp[0] * sz + tmp[1] * cz < 0.0) {
		if (actor_rotation < 0.0)
			actor_rotation = ((int)(-180-actor_rotation-22.5) / 45) * 45.0;
		else
			actor_rotation = ((int)( 180-actor_rotation+22.5) / 45) * 45.0;
	}
	else {
		if (actor_rotation < 0.0)
			actor_rotation = ((int)(actor_rotation-22.5) / 45) * 45.0;
		else
			actor_rotation = ((int)(actor_rotation+22.5) / 45) * 45.0;
	}

	missiles_log_message("%s (%d): cos = %f ; sin = %f", in_act->actor_name, in_act->actor_id, cz, sz);
	missiles_log_message("%s (%d): direction = %f %f %f", in_act->actor_name, in_act->actor_id, tmp[0], tmp[1], tmp[2]);
	missiles_log_message("%s (%d): actor rotation = %f", in_act->actor_name, in_act->actor_id, actor_rotation);

	// we then compute the fine rotation
	cz = cosf((act_z_rot + actor_rotation) * M_PI/180.0);
	sz = sinf((act_z_rot + actor_rotation) * M_PI/180.0);

	origin[0] = in_act->x_pos + 0.25;
	origin[1] = in_act->y_pos + 0.25;
	origin[2] = get_actor_z(in_act) + 1.4 * get_actor_scale(in_act);

	missiles_log_message("%s (%d): compute_actor_rotation: origin=(%.2f,%.2f,%.2f), target=(%.2f,%.2f,%.2f)",
						 in_act->actor_name, in_act->actor_id,
                         origin[0], origin[1], origin[2], in_target[0], in_target[1], in_target[2]);

	tmp[0] = in_target[1] - origin[1];
	tmp[1] = in_target[2] - origin[2];
	tmp[2] = in_target[0] - origin[0];
	from[0] = 0.0;
	from[1] = 0.0;
	from[2] = 1.0;

	to[0] = tmp[0] * sz - tmp[2] * cz;
	to[1] = 0.0;
	to[2] = tmp[0] * cz + tmp[2] * sz;
	Normalize(tmp, to);
	*out_h_rot = asinf(-tmp[0]);

	missiles_log_message("%s (%d): horizontal rotation: from=(%.2f,%.2f,%.2f), to=(%.2f,%.2f,%.2f), h_rot=%f",
						 in_act->actor_name, in_act->actor_id,
						 from[0], from[1], from[2], tmp[0], tmp[1], tmp[2], *out_h_rot);

	from[0] = tmp[0];
	from[1] = tmp[1];
	from[2] = tmp[2];
	to[1] = in_target[2] - origin[2];
	Normalize(to, to);
	VCross(tmp, from, to);
	*out_v_rot = asinf(sqrtf(tmp[0]*tmp[0] + tmp[1]*tmp[1] + tmp[2]*tmp[2]));
	if (to[1] < from[1]) *out_v_rot = -*out_v_rot;

	missiles_log_message("%s (%d): vertical rotation: from=(%.2f,%.2f,%.2f), to=(%.2f,%.2f,%.2f), v_rot=%f",
						 in_act->actor_name, in_act->actor_id,
						 from[0], from[1], from[2], to[0], to[1], to[2], *out_v_rot);
/*#ifdef MORE_ATTACHED_ACTORS
	//no horses can be processed here
	if(in_act->attached_actor>=0) actor_rotation-=HORSE_FIGHT_ROTATION;
#endif*/
	return actor_rotation;

}

int missiles_fire_arrow(actor *a, float target[3], MissileShotType shot_type)
{
	int mis_id;
	float origin[3];
	float shift[3] = {0.0, get_actor_scale(a), 0.0};
	missile_type *mis_type;
	int mis_type_id;
        
	mis_type_id = actors_defs[a->actor_type].shield[a->cur_shield].missile_type;

	if (mis_type_id < 0 || mis_type_id >= MAX_MISSILES_DEFS) {
		LOG_ERROR("%d is not a valid missile type for shield %d of actor type %d\n", mis_type_id, a->cur_shield, a->actor_type);
		mis_type_id = 0;
	}

	mis_type = &missiles_defs[mis_type_id];

	shift[1] *= mis_type->length;

	cal_get_actor_bone_absolute_position(a, get_actor_bone_id(a, arrow_bone), shift, origin);
        
/* 	if (shot_type != MISSED_SHOT) */
	mis_id = missiles_add(mis_type_id, origin, target, 0.0, shot_type);
	
	if(a->actor_id == yourself)
	{
		range_total_shots++;
		if (shot_type == MISSED_SHOT)
		{
			add_floating_message(a->actor_id, "miss", FLOATINGMESSAGE_NORTH, 1.0, 0.55, 0.0, 1250);
		}
		else if (shot_type == NORMAL_SHOT)
		{
			range_success_hits++;
		}
		else if(shot_type == CRITICAL_SHOT)
		{
			range_critical_hits++;
			range_success_hits++;
		}
	}
	/* 	else */
/* 		mis_id = missiles_add(a->cur_shield, origin, target, arrow_speed*2.0/3.0, arrow_trace_length*2.0/3.0, 0.0, shot_type); */
        
	return mis_id;
}

void missiles_rotate_actor_bones(actor *a)
{
	struct CalSkeleton *skel;
	struct CalBone *bone;
	struct CalQuaternion *bone_rot, *bone_rot_abs, *hrot_quat, *vrot_quat;
	struct CalVector *vect;
	skeleton_types *skt = &skeletons_defs[actors_defs[a->actor_type].skeleton_type];
	float *tmp_vect;
	float hrot, vrot, tmp;

	if (a->cal_rotation_blend < 0.0)
		return;

	skel = CalModel_GetSkeleton(a->calmodel);
        
	if (a->cal_rotation_blend < 1.0) {
		a->cal_rotation_blend += a->cal_rotation_speed*(cur_time-a->cal_last_rotation_time);

		hrot = (a->cal_h_rot_start * (1.0 - a->cal_rotation_blend) +
				a->cal_h_rot_end * a->cal_rotation_blend);
		vrot = (a->cal_v_rot_start * (1.0 - a->cal_rotation_blend) +
				a->cal_v_rot_end * a->cal_rotation_blend);
	}
	else {
		if (fabs(a->cal_h_rot_end) < EPSILON && fabs(a->cal_v_rot_end) < EPSILON) {
			a->cal_rotation_blend = -1.0; // stop rotating bones every frames
			a->cal_h_rot_start = 0.0;
			a->cal_v_rot_start = 0.0;
			missiles_log_message("%s (%d): stopping bones rotation",
                                 a->actor_name, a->actor_id);
		}
		else
			a->cal_rotation_blend = 1.0;

		hrot = a->cal_h_rot_end;
		vrot = a->cal_v_rot_end;

		if (a->are_bones_rotating) {
			a->are_bones_rotating = 0;
			if (a->cur_anim.anim_index >= 0 &&
				a->anim_time >= a->cur_anim.duration)
				a->busy = 0;
		}
	}

	vect = CalVector_New();

	hrot_quat = CalQuaternion_New();
	vrot_quat = CalQuaternion_New();

	// get the rotation of the parent bone
	bone = CalSkeleton_GetBone(skel, 0);
	bone_rot_abs = CalBone_GetRotationAbsolute(bone);

	// getting the chest bone to rotate
	bone = CalSkeleton_GetBone(skel, skt->cal_bones_id[body_bottom_bone]);
	bone_rot = CalBone_GetRotation(bone);

	// rotating the bone horizontally
	CalVector_Set(vect, 0.0, 0.0, 1.0);
	CalQuaternion_Invert(bone_rot_abs);
	CalVector_Transform(vect, bone_rot_abs);
	CalQuaternion_Invert(bone_rot_abs);
	tmp_vect = CalVector_Get(vect);
	tmp = sinf(hrot/2.0);
	CalQuaternion_Set(hrot_quat, tmp_vect[0]*tmp, tmp_vect[1]*tmp, tmp_vect[2]*tmp, cosf(hrot/2.0));
	CalQuaternion_Multiply(bone_rot, hrot_quat);

	// rotating the bone vertically
	CalVector_Set(vect, cosf(hrot), -sinf(hrot), 0.0);
	CalQuaternion_Invert(bone_rot_abs);
	CalVector_Transform(vect, bone_rot_abs);
	CalQuaternion_Invert(bone_rot_abs);
	tmp_vect = CalVector_Get(vect);
	tmp = sinf(vrot/2.0);
	CalQuaternion_Set(vrot_quat, tmp_vect[0]*tmp, tmp_vect[1]*tmp, tmp_vect[2]*tmp, cosf(vrot/2.0));
	CalQuaternion_Multiply(bone_rot, vrot_quat);

	// updating the bone state
	CalBone_CalculateState(bone);

	// rotating the cape bones
	hrot = -hrot;
	vrot = -vrot;

	// rotating the bone horizontally
	if (hrot > 0.0) {
		bone = CalSkeleton_GetBone(skel, skt->cal_bones_id[cape_top_bone]);
		CalQuaternion_Set(hrot_quat, 0.0, sinf(hrot/2.0), 0.0, cosf(hrot/2.0));
		bone_rot = CalBone_GetRotation(bone);
		CalQuaternion_Multiply(bone_rot, hrot_quat);
		CalBone_CalculateState(bone);
	}

	// rotating the bone vertically
	if (vrot < 0.0) {
		bone = CalSkeleton_GetBone(skel, skt->cal_bones_id[body_top_bone]);
		bone_rot_abs = CalBone_GetRotationAbsolute(bone);
		bone = CalSkeleton_GetBone(skel, skt->cal_bones_id[cape_top_bone]);
		bone_rot = CalBone_GetRotation(bone);
	}
	else {
		bone = CalSkeleton_GetBone(skel, skt->cal_bones_id[cape_top_bone]);
		bone_rot_abs = CalBone_GetRotationAbsolute(bone);
		bone = CalSkeleton_GetBone(skel, skt->cal_bones_id[cape_middle_bone]);
		bone_rot = CalBone_GetRotation(bone);
	}

	CalVector_Set(vect, cosf(hrot), sinf(hrot), 0.0);
	CalQuaternion_Invert(bone_rot_abs);
	CalVector_Transform(vect, bone_rot_abs);
	CalQuaternion_Invert(bone_rot_abs);
	tmp_vect = CalVector_Get(vect);
	tmp = sinf(vrot/2.0);
	CalQuaternion_Set(vrot_quat, tmp_vect[0]*tmp, tmp_vect[1]*tmp, tmp_vect[2]*tmp, cosf(vrot/2.0));
	CalQuaternion_Multiply(bone_rot, vrot_quat);
	CalBone_CalculateState(bone);

	CalVector_Delete(vect);
	CalQuaternion_Delete(hrot_quat);
	CalQuaternion_Delete(vrot_quat);

    a->cal_last_rotation_time = cur_time;
}

void missiles_test_target_validity(float target[3], char *msg)
{
	if (target[0] < 0.0 || target[0] > tile_map_size_x*3.0 ||
		target[1] < 0.0 || target[1] > tile_map_size_y*3.0)
		LOG_ERROR("%s: target (%f,%f,%f) is out of the map!",
				  msg, target[0], target[1], target[2]);
}

void missiles_clean_range_actions_queue(actor *act)
{
	int j = 0;

	// we search the first action that is not finished
	while (j < act->range_actions_count &&
		   (act->range_actions[j].state == 1 ||
			act->range_actions[j].state == 3)) {
		++j;
	}

	// we then remove all the finished actions
	if (j > 0) {
        int i;
		for (i = j; i < act->range_actions_count; ++i) {
			memcpy(&act->range_actions[i-j], &act->range_actions[i], sizeof(range_action));
		}
		act->range_actions_count -= j;
		missiles_log_message("%s (%d): removing %d actions from the queue",
							 act->actor_name, act->actor_id, j);
	}
}

void missiles_aim_at_b(int actor1_id, int actor2_id)
{
	actor *act1, *act2;

	LOCK_ACTORS_LISTS();
	act1 = get_actor_ptr_from_id(actor1_id);
	act2 = get_actor_ptr_from_id(actor2_id);

	if (!act1) {
		LOG_ERROR("the actor %d does not exists!", actor1_id);
		UNLOCK_ACTORS_LISTS();
		return;
	}
	if (!act2) {
		LOG_ERROR("the actor %d does not exists!", actor2_id);
		UNLOCK_ACTORS_LISTS();
		return;
	}
	
	missiles_log_message("%s (%d): cleaning the queue from missiles_aim_at_b",
						 act1->actor_name, actor1_id);
	missiles_clean_range_actions_queue(act1);

	if (act1->range_actions_count < MAX_RANGE_ACTION_QUEUE)	{
		range_action *action = &act1->range_actions[act1->range_actions_count];
	
		missiles_log_message("%s (%d): will aim at actor %d (time=%d)", act1->actor_name, actor1_id, actor2_id, cur_time);

		cal_get_actor_bone_absolute_position(act2, get_actor_bone_id(act2, body_top_bone), NULL, action->aim_position);
		missiles_test_target_validity(action->aim_position, "missiles_aim_at_b");

		action->aim_actor = actor2_id;
		action->fire_actor = -1;
		action->shot_type = NORMAL_SHOT;
		action->reload = 0;
		action->state = 0;
		++act1->range_actions_count;
		UNLOCK_ACTORS_LISTS();

		add_command_to_actor(actor1_id, enter_aim_mode);
	}
	else	{
		LOG_ERROR("%s (%d): unable to add a new aim action, the queue is full!",
				  act1->actor_name, actor1_id);
		UNLOCK_ACTORS_LISTS();
	}
}

void missiles_aim_at_xyz(int actor_id, float *target)
{
	actor *act;

	LOCK_ACTORS_LISTS();
	act = get_actor_ptr_from_id(actor_id);

	if (!act) {
		LOG_ERROR("the actor %d does not exists!", actor_id);
		UNLOCK_ACTORS_LISTS();
		return;
	}

	missiles_log_message("%s (%d): cleaning the queue from missiles_aim_at_xyz",
						 act->actor_name, actor_id);
	missiles_clean_range_actions_queue(act);

	if (act->range_actions_count < MAX_RANGE_ACTION_QUEUE) {
		range_action *action = &act->range_actions[act->range_actions_count];

		missiles_log_message("%s (%d): will aim at target %f,%f,%f (time=%d)", act->actor_name, actor_id, target[0], target[1], target[2], cur_time);

		memcpy(action->aim_position, target, sizeof(float) * 3);
		missiles_test_target_validity(action->aim_position, "missiles_aim_at_xyz");

		action->aim_actor = -1;
		action->fire_actor = -1;
		action->shot_type = NORMAL_SHOT;
		action->reload = 0;
		action->state = 0;
		++act->range_actions_count;
		UNLOCK_ACTORS_LISTS();

		add_command_to_actor(actor_id, enter_aim_mode);
	}
	else {
		LOG_ERROR("%s (%d): unable to add a new aim action, the queue is full!",
				  act->actor_name, actor_id);
		UNLOCK_ACTORS_LISTS();
	}
}

void missiles_fire_a_to_b(int actor1_id, int actor2_id)
{
	actor *act1, *act2;

	LOCK_ACTORS_LISTS();
	act1 = get_actor_ptr_from_id(actor1_id);
	act2 = get_actor_ptr_from_id(actor2_id);
        
	if (!act1) {
		LOG_ERROR("missiles_fire_a_to_b: the actor %d does not exists!", actor1_id);
		UNLOCK_ACTORS_LISTS();
		return;
	}
	if (!act2) {
		LOG_ERROR("missiles_fire_a_to_b: the actor %d does not exists!", actor2_id);
		UNLOCK_ACTORS_LISTS();
		return;
	}

	if (act1->range_actions_count <= MAX_RANGE_ACTION_QUEUE &&
		act1->range_actions_count > 0) {
		range_action *action = &act1->range_actions[act1->range_actions_count-1];

		missiles_log_message("%s (%d): will fire to actor %d", act1->actor_name, actor1_id, actor2_id);

		cal_get_actor_bone_absolute_position(act2, get_actor_bone_id(act2, body_top_bone), NULL, action->fire_position);
		missiles_test_target_validity(action->fire_position, "missiles_fire_a_to_b");

		action->fire_actor = actor2_id;
		action->state = 2;

		act2->last_range_attacker_id = actor1_id;

		UNLOCK_ACTORS_LISTS();

		add_command_to_actor(actor1_id, aim_mode_fire);
		if (act2->actor_type == 97)
		{
			add_command_to_actor(actor2_id, pain1);
		}
	}
	else {
		LOG_ERROR("%s (%d): unable to add a fire action, the queue is empty!",
				  act1->actor_name, actor1_id);
		UNLOCK_ACTORS_LISTS();
	}
}

void missiles_fire_a_to_xyz(int actor_id, float *target)
{
	actor *act;

	LOCK_ACTORS_LISTS();
	act = get_actor_ptr_from_id(actor_id);

	if (!act) {
		LOG_ERROR("missiles_fire_a_to_xyz: the actor %d does not exists!", actor_id);
		UNLOCK_ACTORS_LISTS();
		return;
	}

	if (act->range_actions_count <= MAX_RANGE_ACTION_QUEUE &&
		act->range_actions_count > 0) {
		range_action *action = &act->range_actions[act->range_actions_count-1];

		missiles_log_message("%s (%d): will fire to target %f,%f,%f", act->actor_name, actor_id, target[0], target[1], target[2]);
		
		memcpy(action->fire_position, target, sizeof(float) * 3);
		missiles_test_target_validity(action->fire_position, "missiles_fire_a_to_xyz");
		action->fire_actor = -1;
		action->state = 2;

		UNLOCK_ACTORS_LISTS();

		add_command_to_actor(actor_id, aim_mode_fire);
	}
	else {
		LOG_ERROR("%s (%d): unable to add a fire action, the queue is empty!",
				  act->actor_name, actor_id);
		UNLOCK_ACTORS_LISTS();
	}
}

void missiles_fire_xyz_to_b(float *origin, int actor_id)
{
	actor * act;
	float target[3];

	missiles_log_message("missile was fired from %f,%f,%f to actor %d", origin[0], origin[1], origin[2], actor_id);

	LOCK_ACTORS_LISTS();
	act = get_actor_ptr_from_id(actor_id);

	if (!act) {
		LOG_ERROR("the actor %d does not exists!", actor_id);
		UNLOCK_ACTORS_LISTS();
		return;
	}

	cal_get_actor_bone_absolute_position(act, get_actor_bone_id(act, body_top_bone), NULL, target);
	missiles_test_target_validity(target, "missiles_fire_xyz_to_b");
	act->last_range_attacker_id = -1;
	UNLOCK_ACTORS_LISTS();

	// here, there's no way to know if the target is missed or not as we don't know the actor who fired!
	missiles_add(0, origin, target, 0.0, 0);
}

int missiles_parse_nodes(xmlNode *node)
{
	int mis_idx;
	missile_type *mis;
	xmlNode *item;
	int     ok = 1;

	if(node == NULL || node->children == NULL) return 0;

	mis_idx = get_int_property(node, "id");

	if (mis_idx < 0 || mis_idx >= MAX_MISSILES_DEFS) {
		LOG_ERROR("missiles_parse_node: no ID found for node %s or ID out of range: id=%d\n", get_string_property(node, "type"), mis_idx);
		return 0;
	}

	mis = &missiles_defs[mis_idx];

	for(item=node->children; item; item=item->next) {
		if(item->type == XML_ELEMENT_NODE) {
			if(xmlStrcasecmp(item->name, (xmlChar*)"mesh") == 0) {
				get_string_value(mis->lost_mesh, sizeof(mis->lost_mesh), item);
			}
			else if(xmlStrcasecmp(item->name, (xmlChar*)"mesh_length") == 0) {
				mis->length = get_float_value(item);
			}
			else if(xmlStrcasecmp(item->name, (xmlChar*)"trace_length") == 0) {
				mis->trace_length = get_float_value(item);
			}
			else if(xmlStrcasecmp(item->name, (xmlChar*)"speed") == 0) {
				mis->speed = get_float_value(item);
			}
			else if(xmlStrcasecmp(item->name, (xmlChar*)"effect") == 0) {
				char effect_name[64];
				get_string_value(effect_name, sizeof(effect_name), item);
				if (!strcasecmp(effect_name, "none")) {
					mis->effect = REGULAR_MISSILE;
				}
				else if (!strcasecmp(effect_name, "magic")) {
					mis->effect = MAGIC_MISSILE;
				}
				else if (!strcasecmp(effect_name, "fire")) {
					mis->effect = FIRE_MISSILE;
				}
				else if (!strcasecmp(effect_name, "ice")) {
					mis->effect = ICE_MISSILE;
				}
				else if (!strcasecmp(effect_name, "explosive")) {
					mis->effect = EXPLOSIVE_MISSILE;
				}
				else {
					mis->effect = REGULAR_MISSILE;
					LOG_ERROR("missiles_parse_node: \"%s\" is an unknown effect", effect_name);
				}
			}
			else {
				LOG_ERROR("missiles_parse_node: unknown attribute \"%s\"", item->name);
				ok = 0;
			}
		}
		else if (item->type == XML_ENTITY_REF_NODE) {
			ok &= missiles_parse_nodes(item->children);
		}
	}

	return ok;
}

int missiles_parse_defs(xmlNode *node)
{
	xmlNode *def;
	int ok = 1;

	for (def = node->children; def; def = def->next) {
		if (def->type == XML_ELEMENT_NODE)
			if (xmlStrcasecmp(def->name, (xmlChar*)"missile") == 0) {
				ok &= missiles_parse_nodes(def);
			} else {
				LOG_ERROR("parse error: missile or include expected");
				ok = 0;
			}
		else if (def->type == XML_ENTITY_REF_NODE) {
			ok &= missiles_parse_defs(def->children);
		}
	}

	return ok;
}

int missiles_read_defs(const char *file_name)
{
	xmlNode *root;
	xmlDoc *doc;
	int ok = 1;

	doc = xmlReadFile(file_name, NULL, 0);
	if (doc == NULL) {
		LOG_ERROR("Unable to read missiles definition file %s", file_name);
		return 0;
	}

	root = xmlDocGetRootElement(doc);
	if (root == NULL) {
		LOG_ERROR("Unable to parse missiles definition file %s", file_name);
		ok = 0;
	} else if (xmlStrcasecmp(root->name, (xmlChar*)"missiles") != 0) {
		LOG_ERROR("Unknown key \"%s\" (\"missiles\" expected).", root->name);
		ok = 0;
	} else {
		ok = missiles_parse_defs(root);
	}

	xmlFreeDoc(doc);
	return ok;
}

void missiles_init_defs()
{
	// initialize the whole thing to zero
	memset(missiles_defs, 0, sizeof(missiles_defs));

	missiles_read_defs("actor_defs/missile_defs.xml");
}

/**********************************************/
/*! Ranging win */
/**********************************************/

int range_win = -1;
int range_win_x_len = 10;
int range_win_y_len = 20;
int ranging_win_x = 10;
int ranging_win_y = 20;
static int print_to_console = 0;

int display_range_handler(window_info *win)
{
	char str[50];
	const int margin = 5;
	const int step_y = (int)(SMALL_FONT_Y_LEN+0.5);
	const int pos_x = margin;
	int pos_y = margin;
	int max_width = 0;
	int len_x = 0;
	int len_y = 0;
	int resize = 0;

	if (print_to_console) LOG_TO_CONSOLE(c_green2, ranging_win_title_str);

	safe_snprintf(str, sizeof(str), ranging_total_shots_str, range_total_shots);
	if (print_to_console) LOG_TO_CONSOLE(c_green1, str);
	if (max_width < strlen(str)) max_width = strlen(str);
	draw_string_small(pos_x, pos_y, (unsigned char*)str,2);
	pos_y += step_y;

	safe_snprintf(str, sizeof(str), ranging_sucessful_shots_str, range_success_hits);
	if (print_to_console) LOG_TO_CONSOLE(c_green1, str);
	if (max_width < strlen(str)) max_width = strlen(str);
	draw_string_small(pos_x, pos_y, (unsigned char*)str,2);
	pos_y += step_y;

	safe_snprintf(str, sizeof(str), ranging_missed_shots_str, range_total_shots - range_success_hits);
	if (print_to_console) LOG_TO_CONSOLE(c_green1, str);
	if (max_width < strlen(str)) max_width = strlen(str);
	draw_string_small(pos_x, pos_y, (unsigned char*)str,2);
	pos_y += 2*step_y;

	if(range_success_hits > 0)
		safe_snprintf(str, sizeof(str), ranging_success_rate_str, ((double)range_success_hits/(double)range_total_shots)*100.0);
	else
		safe_snprintf(str, sizeof(str), ranging_success_rate_str, 0.0);
	if (print_to_console) LOG_TO_CONSOLE(c_green1, str);
	if (max_width < strlen(str)) max_width = strlen(str);
	draw_string_small(pos_x, pos_y, (unsigned char*)str,2);
	pos_y += step_y;

	if(range_critical_hits > 0)
		safe_snprintf(str, sizeof(str), ranging_critical_rate_str, ((double)range_critical_hits/(double)range_success_hits)*100.0);
	else
		safe_snprintf(str, sizeof(str), ranging_critical_rate_str, 0.0);
	if (print_to_console) LOG_TO_CONSOLE(c_green1, str);
	if (max_width < strlen(str)) max_width = strlen(str);
	draw_string_small(pos_x, pos_y, (unsigned char*)str,2);
	pos_y += 2*step_y;

	if(range_total_shots > 0)
		safe_snprintf(str, sizeof(str), ranging_exp_per_arrow_str, get_session_exp_ranging()/(double)range_total_shots);
	else
		safe_snprintf(str, sizeof(str), ranging_exp_per_arrow_str, 0.0);
	if (print_to_console) LOG_TO_CONSOLE(c_green1, str);
	if (max_width < strlen(str)) max_width = strlen(str);
	draw_string_small(pos_x, pos_y, (unsigned char*)str,2);
	pos_y += step_y;

	len_x = (max_width * SMALL_FONT_X_LEN) + 2*margin + ELW_BOX_SIZE;
	if (len_x != win->len_x)
		resize = 1;

	len_y = pos_y + margin;
	if (len_y != win->len_y)
		resize = 1;

	if (resize)
		resize_window(win->window_id, len_x, len_y);

	print_to_console = 0;

	return 1;
}

static int cm_ranging_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	if (option<ELW_CM_MENU_LEN)
		return cm_title_handler(win, widget_id, mx, my, option);
	if (option == ELW_CM_MENU_LEN+1)
		print_to_console = 1;
	return 1;
}

void display_range_win(void)
{
	if(range_win < 0){
		int our_root_win = -1;
			
		if (!windows_on_top) {
			our_root_win = game_root_win;
		}
		range_win = create_window(ranging_win_title_str, our_root_win, 0, ranging_win_x, ranging_win_y, range_win_x_len, range_win_y_len, ELW_WIN_DEFAULT);
		set_window_handler(range_win, ELW_HANDLER_DISPLAY, &display_range_handler );
		cm_add(windows_list.window[range_win].cm_id, cm_ranging_menu_str, cm_ranging_handler);
	} else {
		show_window(range_win);
		select_window(range_win);
	}
}
