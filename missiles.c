#ifdef MISSILES

#include "3d_objects.h"
#include "actor_scripts.h"
#include "asc.h"
#include "cal3d_wrapper.h"
#include "e3d.h"
#include "gl_init.h"
#include "init.h"
#include "missiles.h"
#include "tiles.h"
#include "vmath.h"

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#define MAX_MISSILES 1024
#define EPSILON 1E-4

#define MAT3_ROT_X(mat,angle) \
(mat[0]=1.0,mat[3]=0.0       ,mat[6]=0.0        ,\
 mat[1]=0.0,mat[4]=cos(angle),mat[7]=-sin(angle),\
 mat[2]=0.0,mat[5]=-mat[7]   ,mat[8]=mat[4]     )

#define MAT3_ROT_Y(mat,angle) \
(mat[0]=cos(angle),mat[3]=0.0,mat[6]=sin(angle),\
 mat[1]=0.0       ,mat[4]=1.0,mat[7]=0.0       ,\
 mat[2]=-mat[6]   ,mat[5]=0.0,mat[8]=mat[0]    )

#define MAT3_ROT_Z(mat,angle) \
(mat[0]=cos(angle),mat[3]=-sin(angle),mat[6]=0.0,\
 mat[1]=-mat[3]   ,mat[4]=mat[0]     ,mat[7]=0.0,\
 mat[2]=0.0       ,mat[5]=0.0        ,mat[8]=1.0)

#define MAT3_VECT3_MULT(res,mat,vect) \
((res)[0]=mat[0]*(vect)[0]+mat[3]*(vect)[1]+mat[6]*(vect)[2],\
 (res)[1]=mat[1]*(vect)[0]+mat[4]*(vect)[1]+mat[7]*(vect)[2],\
 (res)[2]=mat[2]*(vect)[0]+mat[5]*(vect)[1]+mat[8]*(vect)[2])

#define MAT3_MULT(res,mat1,mat2) \
(MAT3_VECT3_MULT(&res[0],mat1,&mat2[0]),\
 MAT3_VECT3_MULT(&res[3],mat1,&mat2[3]),\
 MAT3_VECT3_MULT(&res[6],mat1,&mat2[6]))

const float arrow_length = 0.75;
const float bolt_length = 0.4;

const float arrow_speed = 50.0;
const float arrow_trace_length = 7.0;
const float arrow_color[3] = {0.8, 0.8, 0.8};

Missile missiles_list[MAX_MISSILES];

unsigned int missiles_count = 0;

FILE *missiles_log = NULL;

void open_missiles_log()
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
		open_missiles_log();

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

void clear_missiles()
{
	missiles_count = 0;
}

float get_actor_scale(actor *a)
{
	float scale = a->scale;

#ifdef NEW_ACTOR_SCALE
	scale *= actors_defs[a->actor_type].actor_scale;
#endif

	return scale;
}

float get_actor_z(actor *a)
{
	return -2.2f + height_map[a->y_tile_pos*tile_map_size_x*6+a->x_tile_pos]*0.2f;
}

void get_actor_rotation_matrix(actor *in_act, float *out_rot)
{
	float tmp_rot1[9], tmp_rot2[9];

	MAT3_ROT_Z(out_rot, (180.0 - in_act->z_rot) * (M_PI / 180.0));
	MAT3_ROT_X(tmp_rot1, in_act->x_rot * (M_PI / 180.0));
	MAT3_MULT(tmp_rot2, out_rot, tmp_rot1);
	MAT3_ROT_Y(tmp_rot1, in_act->y_rot * (M_PI / 180.0));
	MAT3_MULT(out_rot, tmp_rot2, tmp_rot1);
}

void get_actor_bone_local_position(actor *in_act, int in_bone_id, float *in_shift, float *out_pos)
{
	struct CalSkeleton *skel;
	struct CalBone *bone;
	struct CalVector *point;

	skel = CalModel_GetSkeleton(in_act->calmodel);
	bone = CalSkeleton_GetBone(skel, in_bone_id);
	point = CalBone_GetTranslationAbsolute(bone);

	memcpy(out_pos, CalVector_Get(point), 3*sizeof(float));

	if (in_shift) {
		struct CalQuaternion *rot;
		struct CalVector *vect;
		float *tmp;
		rot = CalBone_GetRotationAbsolute(bone);
		vect = CalVector_New();
		CalVector_Set(vect, in_shift[0], in_shift[1], in_shift[2]);
		CalVector_Transform(vect, rot);
		tmp = CalVector_Get(vect);
		out_pos[0] += tmp[0];
		out_pos[1] += tmp[1];
		out_pos[2] += tmp[2];
		CalVector_Delete(vect);
	}
}

void transform_actor_local_position_to_absolute(actor *in_act, float *in_local_pos, float *in_act_rot, float *out_pos)
{
	float scale = get_actor_scale(in_act);

	if (in_act_rot) {
		MAT3_VECT3_MULT(out_pos, in_act_rot, in_local_pos);
	}
	else {
		float rot[9];
		get_actor_rotation_matrix(in_act, rot);
		MAT3_VECT3_MULT(out_pos, rot, in_local_pos);
	}

	out_pos[0] = out_pos[0] * scale + in_act->x_pos + 0.25;
	out_pos[1] = out_pos[1] * scale + in_act->y_pos + 0.25;
	out_pos[2] = out_pos[2] * scale + get_actor_z(in_act);
}

void get_actor_bone_absolute_position(actor *in_act, int in_bone_id, float *in_shift, float *out_pos)
{
	float act_rot[9];
	float pos[3];
	get_actor_rotation_matrix(in_act, act_rot);
	get_actor_bone_local_position(in_act, in_bone_id, in_shift, pos);
	transform_actor_local_position_to_absolute(in_act, pos, act_rot, out_pos);
}

unsigned int add_missile(MissileType type,
						 float origin[3],
						 float target[3],
						 float speed,
						 float shift)
{
	Missile *mis;
	
	assert(missiles_count < MAX_MISSILES);

#ifdef DEBUG
	printf("add_missile: origin=(%0.2f,%0.2f,%0.2f), target=(%0.2f,%0.2f,%0.2f)\n",
		   origin[0], origin[1], origin[2], target[0], target[1], target[2]);
#endif // DEBUG
	
	mis = &missiles_list[missiles_count++];
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
	mis->covered_distance = 0;
	mis->remaining_distance += shift;
	
	return missiles_count;
}

void remove_missile(unsigned int missile_id)
{
	assert(missile_id < missiles_count);

	--missiles_count;
	if (missile_id < missiles_count) {
		memcpy(&missiles_list[missile_id],
			   &missiles_list[missiles_count],
			   sizeof(Missile));
	}
}

void draw_current_actor_nodes()
{
	actor *a = get_actor_ptr_from_id(yourself);
	float act_rot[9];
	float pos1[3], pos2[3], shift[3], tmp[3];
	int i;
	
	if (!a) return;
	
	get_actor_rotation_matrix(a, act_rot);

	glColor3f(1.0, 1.0, 1.0);
	glLineWidth(5.0);
	glBegin(GL_LINES);

 	for (i = 37; i <= 37; ++i)
	{
/* 		switch(i) { */
/* 		case 0: glColor3f(1.0, 0.0, 0.0); break; */
/* 		case 1: glColor3f(0.0, 1.0, 0.0); break; */
/* 		case 2: glColor3f(0.0, 0.0, 1.0); break; */
/* 		} */
/* 		if (i == 13) */
/* 			glColor3f(1.0, 0.0, 0.0); */
/* 		else */
/* 			glColor3f(1.0, 1.0, 1.0); */

		get_actor_bone_local_position(a, i, NULL, tmp);
		transform_actor_local_position_to_absolute(a, tmp, act_rot, pos1);

		shift[0] = 0.3; shift[1] = 0.0; shift[2] = 0.0;
		get_actor_bone_local_position(a, i, shift, tmp);
		transform_actor_local_position_to_absolute(a, tmp, act_rot, pos2);
		glColor3f(1.0, 0.0, 0.0);
		glVertex3fv(pos1);
		glVertex3fv(pos2);

		shift[0] = 0.0; shift[1] = 0.3; shift[2] = 0.0;
		get_actor_bone_local_position(a, i, shift, tmp);
		transform_actor_local_position_to_absolute(a, tmp, act_rot, pos2);
		glColor3f(0.0, 1.0, 0.0);
		glVertex3fv(pos1);
		glVertex3fv(pos2);

		shift[0] = 0.0; shift[1] = 0.0; shift[2] = 0.3;
		get_actor_bone_local_position(a, i, shift, tmp);
		transform_actor_local_position_to_absolute(a, tmp, act_rot, pos2);
		glColor3f(0.0, 0.0, 1.0);
		glVertex3fv(pos1);
		glVertex3fv(pos2);
	}
	
	glEnd();
}

void update_missiles(Uint32 time_diff)
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
		if (mis->remaining_distance < -arrow_trace_length)
			remove_missile(i);
		else
			++i;
	}
}

void draw_missiles()
{
	unsigned int i;

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_BLEND);
	glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glLineWidth(2.0);
	glBegin(GL_LINES);
	for (i = 0; i < missiles_count; ++i) {
		Missile *mis = &missiles_list[i];
		switch (mis->type) {
		case MISSILE_ARROW:
			if (mis->covered_distance < arrow_trace_length) {
				glColor4f(arrow_color[0], arrow_color[1], arrow_color[2],
						  (arrow_trace_length - mis->covered_distance) / arrow_trace_length);
				glVertex3f(mis->position[0] - mis->covered_distance * mis->direction[0],
						   mis->position[1] - mis->covered_distance * mis->direction[1],
						   mis->position[2] - mis->covered_distance * mis->direction[2]);
			}
			else {
				glColor4f(arrow_color[0], arrow_color[1], arrow_color[2], 0.0);
				glVertex3f(mis->position[0] - arrow_trace_length * mis->direction[0],
						   mis->position[1] - arrow_trace_length * mis->direction[1],
						   mis->position[2] - arrow_trace_length * mis->direction[2]);
			}
			if (mis->remaining_distance < 0.0) {
				glColor4f(arrow_color[0], arrow_color[1], arrow_color[2],
						  (arrow_trace_length + mis->remaining_distance) / arrow_trace_length);
				glVertex3f(mis->position[0] + mis->remaining_distance * mis->direction[0],
						   mis->position[1] + mis->remaining_distance * mis->direction[1],
						   mis->position[2] + mis->remaining_distance * mis->direction[2]);
			}
			else {
				glColor4f(arrow_color[0], arrow_color[1], arrow_color[2], 1.0);
				glVertex3fv(mis->position);
			}
			break;
		}
	}
	glEnd();

#ifdef DEBUG
	draw_current_actor_nodes();
#endif // DEBUG

	glPopAttrib();
}

void shortest_arc(struct CalQuaternion *out_q, float *in_from, float *in_to)
{
	float cross[3];
	float dot;

	VCross(cross, in_from, in_to);
	dot = VDot(in_from, in_to);

	dot = (float)sqrt(2*(dot+1));
	
	VScale(cross, cross, 1/dot);
	
	CalQuaternion_Set(out_q, cross[0], cross[1], cross[2], -dot/2); 
}

float compute_actor_rotation(struct CalQuaternion *out_q, actor *in_act, float *in_target)
{
	float cz, sz;
	float from[3], to[3], tmp[3];
	float actor_rotation = 0;
	float act_z_rot = in_act->z_rot;

	if (in_act->rotating) {
		// the actor is already rotating so we get the final position first
		act_z_rot += in_act->rotate_z_speed * in_act->rotate_frames_left;
	}

	cz = cosf((act_z_rot) * M_PI/180.0);
	sz = sinf((act_z_rot) * M_PI/180.0);
	tmp[0] = in_target[0] - in_act->x_pos;
	tmp[1] = in_target[1] - in_act->y_pos;
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
	printf("cos = %f ; sin = %f\n", cz, sz);
	printf("direction = %f %f %f\n", tmp[0], tmp[1], tmp[2]);
	printf("actor rotation = %f\n", actor_rotation);
#endif // DEBUG

	cz = cosf((act_z_rot + actor_rotation) * M_PI/180.0);
	sz = sinf((act_z_rot + actor_rotation) * M_PI/180.0);
	from[0] = 0.0;
	from[1] = 0.0;
	from[2] = 1.0;
	tmp[0] = in_target[1] - in_act->y_pos - 0.25;
	tmp[1] = in_target[2] - get_actor_z(in_act) - 1.2 * get_actor_scale(in_act);
	tmp[2] = in_target[0] - in_act->x_pos - 0.25;
	to[0] = tmp[0] * sz - tmp[2] * cz;
	to[1] = tmp[1];
	to[2] = tmp[0] * cz + tmp[2] * sz;

	/* adjusting the aiming direction according to the position of
	 * the range weapon */
	switch(in_act->cur_weapon)
	{
	case 64: // long bow
	case 65: // short bow
		to[0] -= 0.2 * get_actor_scale(in_act);
		break;

	case 68: // crossbow
		break;

	default:
		missiles_log_message("compute_actor_rotation: weapon %d is an unknown range weapon, unable to estimate the position of the weapon", in_act->cur_weapon);
		break;
	}

	Normalize(to, to);

	shortest_arc(out_q, from, to);

	return actor_rotation;
}

unsigned int fire_arrow(actor *a, float target[3])
{
	unsigned int mis_id;
	float origin[3];
	float shift[3] = {0.0, get_actor_scale(a), 0.0};

	switch(a->cur_weapon)
	{
	case 64: // long bow
	case 65: // short bow
		shift[1] *= arrow_length;
		break;

	case 68: // crossbow
		shift[1] *= bolt_length;
		break;

	default:
		shift[1] = 0.0;
		missiles_log_message("fire_arrow: weapon %d is an unknown range weapon, unable to determine precisely the position of the arrow", a->cur_weapon);
		break;
	}

	get_actor_bone_absolute_position(a, 37, shift, origin);
	
	mis_id = add_missile(MISSILE_ARROW, origin, target, arrow_speed, 0.0);
	
	return mis_id;
}

void rotate_actor_bones(actor *a)
{
	struct CalSkeleton *skel;
	struct CalBone *bone[2];
	struct CalQuaternion *rot[2];
	struct CalQuaternion *tmp;

	if (a->cal_rotation_blend < 0.0)
		return;

	skel = CalModel_GetSkeleton(a->calmodel);
	
	bone[0] = CalSkeleton_GetBone(skel, 11);
	bone[1] = CalSkeleton_GetBone(skel, 12);

	rot[0] = CalBone_GetRotation(bone[0]);
	rot[1] = CalBone_GetRotation(bone[1]);

	tmp = CalQuaternion_New();

	CalQuaternion_Set(tmp, 0.0, 0.0, 0.0, 1.0);

	if (a->cal_rotation_blend < 1.0) {
		struct CalQuaternion *tmp2;

		a->cal_rotation_blend += a->cal_rotation_speed;

		tmp2 = CalQuaternion_New();

		CalQuaternion_Set(tmp2, 0.0, 0.0, 0.0, 1.0);

		CalQuaternion_Blend(tmp, (1.0 - a->cal_rotation_blend)/2.0, a->cal_starting_rotation);
		CalQuaternion_Blend(tmp2, a->cal_rotation_blend/2.0, a->cal_ending_rotation);

		CalQuaternion_Multiply(tmp, tmp2);

		CalQuaternion_Delete(tmp2);
	}
	else {
		float *quat;

		quat = CalQuaternion_Get(a->cal_ending_rotation);
		if (fabs(quat[0]) < EPSILON &&
			fabs(quat[1]) < EPSILON &&
			fabs(quat[2]) < EPSILON &&
			fabs(1.0 - quat[3]) < EPSILON) {
			a->cal_rotation_blend = -1.0; // stop rotating bones every frames
			CalQuaternion_Set(a->cal_starting_rotation, 0.0, 0.0, 0.0, 1.0);
#ifdef DEBUG
			printf("stopping bones rotation\n");
#endif // DEBUG
		}
		else
			a->cal_rotation_blend = 1.0;

		CalQuaternion_Blend(tmp, 0.5, a->cal_ending_rotation);

		if (a->are_bones_rotating) {
			a->are_bones_rotating = 0;
			if (a->cur_anim.anim_index >= 0 &&
				a->anim_time >= a->cur_anim.duration)
				a->busy = 0;
		}
	}

	CalQuaternion_Multiply(rot[0], tmp);
	CalQuaternion_Multiply(rot[1], tmp);

	CalBone_SetRotation(bone[0], rot[0]);
	CalBone_SetRotation(bone[1], rot[1]);
	CalBone_CalculateState(bone[0]);
	CalBone_CalculateState(bone[1]);

	CalQuaternion_Delete(tmp);
}

void actor_aim_at_b(int actor1_id, int actor2_id)
{
	actor *act1, *act2;

	missiles_log_message("actor %d is aiming at actor %d", actor1_id, actor2_id);

	act1 = get_actor_ptr_from_id(actor1_id);
	act2 = get_actor_ptr_from_id(actor2_id);
	
	LOCK_ACTORS_LISTS();
	act1->range_target[0] = (float)act2->x_pos + 0.25;
	act1->range_target[1] = (float)act2->y_pos + 0.25;
	act1->range_target[2] = get_actor_z(act2) + 1.2 * get_actor_scale(act2);
	UNLOCK_ACTORS_LISTS();

	add_command_to_actor(actor1_id, enter_aim_mode);
}

void actor_aim_at_xyz(int actor_id, float *target)
{
	actor *act;

	missiles_log_message("actor %d is aiming at target %f,%f,%f", actor_id, target[0], target[1], target[2]);

	act = get_actor_ptr_from_id(actor_id);

	LOCK_ACTORS_LISTS();
	memcpy(act->range_target, target, sizeof(float) * 3);
	UNLOCK_ACTORS_LISTS();

	add_command_to_actor(actor_id, enter_aim_mode);
}

void missile_fire_a_to_b(int actor1_id, int actor2_id)
{
	actor *act1, *act2;

	missiles_log_message("actor %d is firing to actor %d", actor1_id, actor2_id);

	act1 = get_actor_ptr_from_id(actor1_id);
	act2 = get_actor_ptr_from_id(actor2_id);
	
	LOCK_ACTORS_LISTS();
	act1->range_target[0] = (float)act2->x_pos + 0.25;
	act1->range_target[1] = (float)act2->y_pos + 0.25;
	act1->range_target[2] = get_actor_z(act2) + 1.2 * get_actor_scale(act2);
	UNLOCK_ACTORS_LISTS();

	add_command_to_actor(actor1_id, aim_mode_fire);
}

void missile_fire_a_to_xyz(int actor_id, float *target)
{
	actor *act;

	missiles_log_message("actor %d is firing to target %f,%f,%f", actor_id, target[0], target[1], target[2]);

	act = get_actor_ptr_from_id(actor_id);

	LOCK_ACTORS_LISTS();
	memcpy(act->range_target, target, sizeof(float) * 3);
	UNLOCK_ACTORS_LISTS();

	add_command_to_actor(actor_id, aim_mode_fire);
}

void missile_fire_xyz_to_b(float *origin, int actor_id)
{
	actor * act;
	unsigned int mis_id;
	float target[3];

	missiles_log_message("missile was fired from %f,%f,%f to actor %d", origin[0], origin[1], origin[2], actor_id);

	act = get_actor_ptr_from_id(actor_id);

	LOCK_ACTORS_LISTS();
	target[0] = (float)act->x_pos + 0.25;
	target[1] = (float)act->y_pos + 0.25;
	target[2] = get_actor_z(act) + 1.2 * get_actor_scale(act);
	UNLOCK_ACTORS_LISTS();

	mis_id = add_missile(MISSILE_ARROW, origin, target, arrow_speed, 0.0);
}

#endif // MISSILES
