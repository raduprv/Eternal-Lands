#ifdef MISSILES

#include "3d_objects.h"
#include "actor_scripts.h"
#include "asc.h"
#include "cal.h"
#include "cal3d_wrapper.h"
#include "e3d.h"
#include "gl_init.h"
#include "init.h"
#include "missiles.h"
#include "skeletons.h"
#include "tiles.h"
#include "vmath.h"

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#define MAX_MISSILES 1024
#define MAX_LOST_MISSILES 512
#define LOST_MISSILE_MAX_LIFE 120000
#define EPSILON 1E-4

typedef struct {
	int obj_3d_id;
	Uint32 end_time;
} LostMissile;

float arrow_length = 0.75;
float bolt_length = 0.4;
float arrow_trace_length = 7.0;
float arrow_speed = 50.0;

const float arrow_color[4] = {0.8, 0.8, 0.8, 1.0};
const float arrow_border_color[4] = {0.8, 0.8, 0.8, 0.5};
const float miss_color[4] = {0.9, 0.6, 0.6, 1.0};
const float critical_color[4] = {0.6, 0.9, 1.0, 1.0};
const float critical_border1_color[4] = {0.3, 0.7, 1.0, 0.6};
const float critical_border2_color[4] = {0.0, 0.5, 1.0, 0.4};

Missile missiles_list[MAX_MISSILES];
LostMissile lost_missiles_list[MAX_LOST_MISSILES];

unsigned int missiles_count = 0;
int begin_lost_missiles = -1;
int end_lost_missiles = -1;

FILE *missiles_log = NULL;

void missiles_open_log()
{
	char log_name[1024];
	char starttime[200], sttime[200];
	struct tm *l_time; time_t c_time;

	safe_snprintf(log_name, 1024, "%smissiles_log.txt", configdir);

	missiles_log = fopen(log_name, "a");

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

void missiles_log_message(const char *format, ...)
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
	fprintf(missiles_log, logmsg);
  	fflush (missiles_log);
}

void missiles_clear()
{
	missiles_count = 0;
	begin_lost_missiles = end_lost_missiles = -1;
}

unsigned int missiles_add(MissileType type,
						  float origin[3],
						  float target[3],
						  float speed,
						  float trace_length,
						  float shift,
						  MissileHitType hit_type)
{
	Missile *mis;
	
	assert(missiles_count < MAX_MISSILES);

#ifdef DEBUG
	missiles_log_message("add_missile: origin=(%.2f,%.2f,%.2f), target=(%.2f,%.2f,%.2f)",
						 origin[0], origin[1], origin[2], target[0], target[1], target[2]);
#endif // DEBUG
	
	mis = &missiles_list[missiles_count++];

	mis->type = type;
	mis->hit_type = hit_type;
	memcpy(mis->position, origin, sizeof(float)*3);
	mis->direction[0] = target[0] - origin[0];
	mis->direction[1] = target[1] - origin[1];
	mis->direction[2] = target[2] - origin[2];
	mis->remaining_distance = sqrt(mis->direction[0]*mis->direction[0] +
								   mis->direction[1]*mis->direction[1] +
								   mis->direction[2]*mis->direction[2]);
	assert(fabs(mis->remaining_distance) > EPSILON);
	mis->direction[0] /= mis->remaining_distance;
	mis->direction[1] /= mis->remaining_distance;
	mis->direction[2] /= mis->remaining_distance;
	mis->speed = speed;
	mis->trace_length = trace_length;
	mis->covered_distance = 0;
	mis->remaining_distance += shift;
	
	return missiles_count;
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

void missiles_remove(unsigned int missile_id)
{
	Missile *mis;
	assert(missile_id < missiles_count);

	mis = &missiles_list[missile_id];
	if (mis->hit_type == MISSED_HIT &&
		mis->covered_distance < 30.0) {
		float y_rot = -asinf(mis->direction[2]);
		float z_rot = atan2f(mis->direction[1], mis->direction[0]);
		float dist = -mis->remaining_distance;
		int obj_3d_id = -1;
		mis->position[0] -= mis->direction[0] * dist;
		mis->position[1] -= mis->direction[1] * dist;
		mis->position[2] -= mis->direction[2] * dist;
		missiles_log_message("adding a lost missile at (%f,%f,%f)",
							 mis->position[0], mis->position[1], mis->position[2]);
		switch(mis->type) {
		case MISSILE_ARROW:
			obj_3d_id = add_e3d("./3dobjects/misc_objects/arrow2.e3d",
								mis->position[0], mis->position[1], mis->position[2],
								0.0, y_rot*180.0/M_PI, z_rot*180.0/M_PI,
								0, 0, 1.0, 1.0, 1.0, 1);
			break;
		default:
			break;
		}
		if (obj_3d_id >= 0)
			missiles_add_lost(obj_3d_id);
	}
	
	--missiles_count;
	if (missile_id < missiles_count) {
		memcpy(&missiles_list[missile_id],
			   &missiles_list[missiles_count],
			   sizeof(Missile));
	}
}

/* void missiles_draw_current_actor_nodes() */
/* { */
/* 	actor *a = get_actor_ptr_from_id(yourself); */
/* 	float act_rot[9]; */
/* 	float pos1[3], pos2[3], shift[3], tmp[3]; */
/* 	int displayed_bones[] = {0, 11}; */
/* 	int i; */
	
/* 	if (!a) return; */
	
/* 	get_actor_rotation_matrix(a, act_rot); */

/* 	glColor3f(1.0, 1.0, 1.0); */
/* 	glLineWidth(5.0); */
/* 	glBegin(GL_LINES); */

/*  	for (i = 0; i <= 1; ++i) */
/* 	{ */
/* 		int bone_id = displayed_bones[i]; */

/* 		cal_get_actor_bone_local_position(a, bone_id, NULL, tmp); */
/* 		transform_actor_local_position_to_absolute(a, tmp, act_rot, pos1); */

/* 		shift[0] = 0.3; shift[1] = 0.0; shift[2] = 0.0; */
/* 		cal_get_actor_bone_local_position(a, bone_id, shift, tmp); */
/* 		transform_actor_local_position_to_absolute(a, tmp, act_rot, pos2); */
/* 		glColor3f(1.0, 0.0, 0.0); */
/* 		glVertex3fv(pos1); */
/* 		glVertex3fv(pos2); */

/* 		shift[0] = 0.0; shift[1] = 0.3; shift[2] = 0.0; */
/* 		cal_get_actor_bone_local_position(a, bone_id, shift, tmp); */
/* 		transform_actor_local_position_to_absolute(a, tmp, act_rot, pos2); */
/* 		glColor3f(0.0, 1.0, 0.0); */
/* 		glVertex3fv(pos1); */
/* 		glVertex3fv(pos2); */

/* 		shift[0] = 0.0; shift[1] = 0.0; shift[2] = 0.3; */
/* 		cal_get_actor_bone_local_position(a, bone_id, shift, tmp); */
/* 		transform_actor_local_position_to_absolute(a, tmp, act_rot, pos2); */
/* 		glColor3f(0.0, 0.0, 1.0); */
/* 		glVertex3fv(pos1); */
/* 		glVertex3fv(pos2); */
/* 	} */

/* 	glEnd(); */
/* } */

void missiles_update(Uint32 time_diff)
{
	unsigned int i;
	float shift_t = time_diff / 1000.0;
	
	for (i = 0; i < missiles_count; ) {
		Missile *mis = &missiles_list[i];
		float dist = mis->speed * shift_t;
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
}

void missiles_draw_single(Missile *mis, const float color[4])
{
	float z_shift = 0.0;

/* 	if (mis->hit_type == MISSED_HIT) */
/* 		z_shift = cosf(mis->covered_distance*M_PI/2.0)/10.0; */

	switch (mis->type) {
	case MISSILE_ARROW:
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
		break;
	}
}

void missiles_draw()
{
	unsigned int i;

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_BLEND);
	glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	glLineWidth(7.0);
	glBegin(GL_LINES);
	for (i = missiles_count; i--;) {
		if (missiles_list[i].hit_type == CRITICAL_HIT)
			missiles_draw_single(&missiles_list[i], critical_border2_color);
	}
	glEnd();

	glLineWidth(3.0);
	glBegin(GL_LINES);
	for (i = missiles_count; i--;) {
		if (missiles_list[i].hit_type == NORMAL_HIT)
			missiles_draw_single(&missiles_list[i], arrow_border_color);
		else if (missiles_list[i].hit_type == CRITICAL_HIT)
			missiles_draw_single(&missiles_list[i], critical_border1_color);
	}
	glEnd();

	glLineWidth(1.0);
	glBegin(GL_LINES);
	for (i = missiles_count; i--;) {
		if (missiles_list[i].hit_type == NORMAL_HIT)
			missiles_draw_single(&missiles_list[i], arrow_color);
		else if (missiles_list[i].hit_type == CRITICAL_HIT)
			missiles_draw_single(&missiles_list[i], critical_color);
	}
	glEnd();

	glLineWidth(2.0);
	glLineStipple(1, 0x003F);
	glEnable(GL_LINE_STIPPLE);
	glBegin(GL_LINES);
	for (i = missiles_count; i--;)
		if (missiles_list[i].hit_type == MISSED_HIT)
			missiles_draw_single(&missiles_list[i], miss_color);
	glEnd();
	glDisable(GL_LINE_STIPPLE);

/* #ifdef DEBUG */
/* 	missiles_draw_current_actor_nodes(); */
/* #endif // DEBUG */

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
		// the actor is already rotating so we get the final position first
		act_z_rot += in_act->rotate_z_speed * in_act->rotate_frames_left;
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

#ifdef DEBUG
	missiles_log_message("cos = %f ; sin = %f", cz, sz);
	missiles_log_message("direction = %f %f %f", tmp[0], tmp[1], tmp[2]);
	missiles_log_message("actor rotation = %f", actor_rotation);
#endif // DEBUG

	// we then compute the fine rotation
	cz = cosf((act_z_rot + actor_rotation) * M_PI/180.0);
	sz = sinf((act_z_rot + actor_rotation) * M_PI/180.0);

	origin[0] = in_act->x_pos + 0.25;
	origin[1] = in_act->y_pos + 0.25;
	origin[2] = get_actor_z(in_act) + 1.4 * get_actor_scale(in_act);

	missiles_log_message("compute_actor_rotation: origin=(%.2f,%.2f,%.2f), target=(%.2f,%.2f,%.2f)",
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

	missiles_log_message("horizontal rotation: from=(%.2f,%.2f,%.2f), to=(%.2f,%.2f,%.2f), h_rot=%f",
						 from[0], from[1], from[2], tmp[0], tmp[1], tmp[2], *out_h_rot);

	from[0] = tmp[0];
	from[1] = tmp[1];
	from[2] = tmp[2];
	to[1] = in_target[2] - origin[2];
	Normalize(to, to);
	VCross(tmp, from, to);
	*out_v_rot = asinf(sqrt(tmp[0]*tmp[0] + tmp[1]*tmp[1] + tmp[2]*tmp[2]));
	if (to[1] < from[1]) *out_v_rot = -*out_v_rot;

	missiles_log_message("vertical rotation: from=(%.2f,%.2f,%.2f), to=(%.2f,%.2f,%.2f), v_rot=%f",
						 from[0], from[1], from[2], to[0], to[1], to[2], *out_v_rot);

	return actor_rotation;
}

unsigned int missiles_fire_arrow(actor *a, float target[3], MissileHitType hit_type)
{
	unsigned int mis_id;
	float origin[3];
	float shift[3] = {0.0, get_actor_scale(a), 0.0};

	switch(a->range_weapon_type)
	{
	case RANGE_WEAPON_BOW:
		shift[1] *= arrow_length;
		break;

	case RANGE_WEAPON_CROSSBOW:
		shift[1] *= bolt_length;
		break;

	default:
		shift[1] = 0.0;
		missiles_log_message("fire_arrow: %d is an unknown range weapon type, unable to determine precisely the position of the arrow", a->range_weapon_type);
		break;
	}

	cal_get_actor_bone_absolute_position(a, get_actor_bone_id(a, arrow_bone), shift, origin);
	
/* 	if (hit_type != MISSED_HIT) */
		mis_id = missiles_add(MISSILE_ARROW, origin, target, arrow_speed, arrow_trace_length, 0.0, hit_type);
/* 	else */
/* 		mis_id = missiles_add(MISSILE_ARROW, origin, target, arrow_speed*2.0/3.0, arrow_trace_length*2.0/3.0, 0.0, hit_type); */
	
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
		a->cal_rotation_blend += a->cal_rotation_speed;

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
#ifdef DEBUG
			missiles_log_message("stopping bones rotation");
#endif // DEBUG
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
}

void missiles_aim_at_b(int actor1_id, int actor2_id)
{
	actor *act1, *act2;
	int bones_number;

	missiles_log_message("actor %d will aim at actor %d", actor1_id, actor2_id);

	act1 = get_actor_ptr_from_id(actor1_id);
	act2 = get_actor_ptr_from_id(actor2_id);

	if (!act1) {
		missiles_log_message("missiles_aim_at_b: the actor %d does not exists!", actor1_id);
		return;
	}
	if (!act2) {
		missiles_log_message("missiles_aim_at_b: the actor %d does not exists!", actor2_id);
		return;
	}

	bones_number = CalSkeleton_GetBonesNumber(CalModel_GetSkeleton(act2->calmodel));
	missiles_log_message("the target has %d bones", bones_number);

	LOCK_ACTORS_LISTS();
	cal_get_actor_bone_absolute_position(act2, get_actor_bone_id(act2, body_top_bone), NULL, act1->range_target);
	UNLOCK_ACTORS_LISTS();

	add_command_to_actor(actor1_id, enter_aim_mode);
}

void missiles_aim_at_xyz(int actor_id, float *target)
{
	actor *act;

	missiles_log_message("actor %d will aim at target %f,%f,%f", actor_id, target[0], target[1], target[2]);

	act = get_actor_ptr_from_id(actor_id);

	if (!act) {
		missiles_log_message("missiles_aim_at_xyz: the actor %d does not exists!", actor_id);
		return;
	}

	LOCK_ACTORS_LISTS();
	memcpy(act->range_target, target, sizeof(float) * 3);
	UNLOCK_ACTORS_LISTS();

	add_command_to_actor(actor_id, enter_aim_mode);
}

void missiles_fire_a_to_b(int actor1_id, int actor2_id)
{
	actor *act1, *act2;
	int bones_number;

	missiles_log_message("actor %d will fire to actor %d", actor1_id, actor2_id);

	act1 = get_actor_ptr_from_id(actor1_id);
	act2 = get_actor_ptr_from_id(actor2_id);
	
	if (!act1) {
		missiles_log_message("missiles_fire_a_to_b: the actor %d does not exists!", actor1_id);
		return;
	}
	if (!act2) {
		missiles_log_message("missiles_fire_a_to_b: the actor %d does not exists!", actor2_id);
		return;
	}

	bones_number = CalSkeleton_GetBonesNumber(CalModel_GetSkeleton(act2->calmodel));
	missiles_log_message("the target has %d bones", bones_number);

	LOCK_ACTORS_LISTS();
	cal_get_actor_bone_absolute_position(act2, get_actor_bone_id(act2, body_top_bone), NULL, act1->range_target);
#ifdef COUNTERS
	act2->last_range_attacker_id = actor1_id;
#endif // COUNTERS
	UNLOCK_ACTORS_LISTS();

	add_command_to_actor(actor1_id, aim_mode_fire);
}

void missiles_fire_a_to_xyz(int actor_id, float *target)
{
	actor *act;

	missiles_log_message("actor %d will fire to target %f,%f,%f", actor_id, target[0], target[1], target[2]);

	act = get_actor_ptr_from_id(actor_id);

	if (!act) {
		missiles_log_message("missiles_fire_a_to_xyz: the actor %d does not exists!", actor_id);
		return;
	}

	LOCK_ACTORS_LISTS();
	memcpy(act->range_target, target, sizeof(float) * 3);
	UNLOCK_ACTORS_LISTS();

	add_command_to_actor(actor_id, aim_mode_fire);
}

void missiles_fire_xyz_to_b(float *origin, int actor_id)
{
	actor * act;
	unsigned int mis_id;
	float target[3];

	missiles_log_message("missile was fired from %f,%f,%f to actor %d", origin[0], origin[1], origin[2], actor_id);

	act = get_actor_ptr_from_id(actor_id);

	if (!act) {
		missiles_log_message("missiles_fire_xyz_to_b: the actor %d does not exists!", actor_id);
		return;
	}

	LOCK_ACTORS_LISTS();
	missiles_log_message("the target has %d bones", CalSkeleton_GetBonesNumber(CalModel_GetSkeleton(act->calmodel)));
	cal_get_actor_bone_absolute_position(act, get_actor_bone_id(act, body_top_bone), NULL, target);
#ifdef COUNTERS
	act->last_range_attacker_id = -1;
#endif // COUNTERS
	UNLOCK_ACTORS_LISTS();

	// here, there's no way to know if the target is missed or not as we don't know the actor who fired!
	mis_id = missiles_add(MISSILE_ARROW, origin, target, arrow_speed, arrow_trace_length, 0.0, 0);
}

#endif // MISSILES
