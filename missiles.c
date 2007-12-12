#ifdef MISSILES

#include "3d_objects.h"
#include "cal3d_wrapper.h"
#include "e3d.h"
#include "gl_init.h"
#include "missiles.h"
#include "vmath.h"
#include "tiles.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

#define MAX_MISSILES 1024
#define EPSILON 1E-4

const float arrow_length = 3.0;
const float arrow_color[3] = {0.5, 0.5, 0.5};

Missile missiles_list[MAX_MISSILES];

unsigned int missiles_count = 0;

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

float get_actor_height(actor *a)
{
	return -2.2f+height_map[a->y_tile_pos*tile_map_size_x*6+a->x_tile_pos]*0.2f;
}

void get_actor_bone_position(actor *a, int bone, float pos[3])
{
	float points[1024][3];
	float rotx_pos[3], roty_pos[3], rotz_pos[3];
	float s_rx, c_rx, s_ry, c_ry, s_rz, c_rz;
	float scale = get_actor_scale(a);
	
	int num_bones = CalSkeleton_GetBonePoints(CalModel_GetSkeleton(a->calmodel),
											  &points[0][0]);
	assert(bone < num_bones);
	
	pos[0] = points[bone][0];
	pos[1] = points[bone][1];
	pos[2] = points[bone][2];
	
	s_rx = sin(a->x_rot * (M_PI / 180));
	c_rx = cos(a->x_rot * (M_PI / 180));
	s_ry = sin(a->y_rot * (M_PI / 180));
	c_ry = cos(a->y_rot * (M_PI / 180));
	s_rz = sin((180 -a->z_rot) * (M_PI / 180));
	c_rz = cos((180 -a->z_rot) * (M_PI / 180));
	
	rotz_pos[0] = pos[0] * c_rz - pos[1] * s_rz;
	rotz_pos[1] = pos[0] * s_rz + pos[1] * c_rz;
	rotz_pos[2] = pos[2];
	
	rotx_pos[0] = rotz_pos[0];
	rotx_pos[1] = rotz_pos[1] * c_rx - rotz_pos[2] * s_rx;
	rotx_pos[2] = rotz_pos[1] * s_rx + rotz_pos[2] * c_rx;
	
	roty_pos[0] = rotx_pos[2] * s_ry + rotx_pos[0] * c_ry;
	roty_pos[1] = rotx_pos[1];
	roty_pos[2] = rotx_pos[2] * c_ry - rotx_pos[0] * s_ry;
	
	pos[0] = roty_pos[0] * scale + a->x_pos + 0.25;
	pos[1] = roty_pos[1] * scale + a->y_pos + 0.25;
	pos[2] = roty_pos[2] * scale + get_actor_height(a);
}

unsigned int add_missile(MissileType type,
						 float origin[3],
						 float target[3],
						 float speed,
						 float delta)
{
	Missile *mis;
	
	assert(missiles_count < MAX_MISSILES);

	//printf("add_missile: origin=(%0.2f,%0.2f,%0.2f), target=(%0.2f,%0.2f,%0.2f)\n",
	//	   origin[0], origin[1], origin[2], target[0], target[1], target[2]);
	
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
	mis->remaining_distance += delta;
	
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
	float points[1024][3];
	float pos[3];
	int i, num_bones;
	
	if (!a) return;
	
	num_bones = CalSkeleton_GetBonePoints(CalModel_GetSkeleton(a->calmodel), &points[0][0]);
	
/* 	glPushAttrib(GL_ALL_ATTRIB_BITS); */
/* 	glEnable(GL_COLOR_MATERIAL); */
/* 	glDisable(GL_LIGHTING); */
/* 	glDisable(GL_TEXTURE_2D); */
/* 	glPointSize(10.0); */
	glColor3f(1.0, 1.0, 1.0);
	glLineWidth(1.0);
	glBegin(GL_LINES);

	for (i = 0; i < num_bones; ++i)
/*  	for (i = 0; i < 3; ++i) */
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
		get_actor_bone_position(a, i, pos);
		glVertex3f(pos[0]-0.2, pos[1], pos[2]);
		glVertex3f(pos[0]+0.2, pos[1], pos[2]);
		glVertex3f(pos[0], pos[1]-0.2, pos[2]);
		glVertex3f(pos[0], pos[1]+0.2, pos[2]);
		glVertex3f(pos[0], pos[1], pos[2]-0.2);
		glVertex3f(pos[0], pos[1], pos[2]+0.2);
	}
	
	glEnd();
/* 	glPopAttrib(); */
}

void update_missiles(Uint32 time_diff)
{
	unsigned int i;
	float delta_t = time_diff / 1000.0;
	
	for (i = 0; i < missiles_count; ) {
		Missile *mis = &missiles_list[i];
		float dist = mis->speed * delta_t;
		mis->position[0] += mis->direction[0] * dist;
		mis->position[1] += mis->direction[1] * dist;
		mis->position[2] += mis->direction[2] * dist;
		mis->covered_distance += dist;
		mis->remaining_distance -= dist;
		if (mis->remaining_distance < -arrow_length)
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
			if (mis->covered_distance < arrow_length) {
				glColor4f(arrow_color[0], arrow_color[1], arrow_color[2],
						  (arrow_length - mis->covered_distance) / arrow_length);
				glVertex3f(mis->position[0] - mis->covered_distance * mis->direction[0],
						   mis->position[1] - mis->covered_distance * mis->direction[1],
						   mis->position[2] - mis->covered_distance * mis->direction[2]);
			}
			else {
				glColor4f(arrow_color[0], arrow_color[1], arrow_color[2], 0.0);
				glVertex3f(mis->position[0] - arrow_length * mis->direction[0],
						   mis->position[1] - arrow_length * mis->direction[1],
						   mis->position[2] - arrow_length * mis->direction[2]);
			}
			if (mis->remaining_distance < 0.0) {
				glColor4f(arrow_color[0], arrow_color[1], arrow_color[2],
						  (arrow_length + mis->remaining_distance) / arrow_length);
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

/* 	draw_current_actor_nodes(); */

	glPopAttrib();
}

void shortest_arc(struct CalQuaternion *q, float from[3], float to[3])
{
	float cross[3];
	float dot;

	VCross(cross, from, to);
	dot = VDot(from, to);

	dot = (float)sqrt(2*(dot+1));
	
	VScale(cross, cross, 1/dot);
	
	CalQuaternion_Set(q, cross[0], cross[1], cross[2], -dot/2); 
}

void compute_actor_rotation(struct CalQuaternion *q, actor *a, float target[3])
{
	float cz = cos((a->z_rot) * M_PI/180.0);
	float sz = sin((a->z_rot) * M_PI/180.0);
	float from[3] = {0.0, 0.0, 1.0};
	float tmp[3] = {target[1] - a->y_pos - 0.25,
					target[2] - get_actor_height(a) - 1.2f,
					target[0] - a->x_pos - 0.25};
	float to[3] = {tmp[0] * sz - tmp[2] * cz,
				   tmp[1],
				   tmp[0] * cz + tmp[2] * sz};

	Normalize(to, to);

	shortest_arc(q, from, to);
}

unsigned int fire_arrow(actor *a, float target[3])
{
	unsigned int mis_id;
	float origin[3];

	get_actor_bone_position(a, 28, origin);
	
	mis_id = add_missile(MISSILE_ARROW, origin, target, 30.0, 0.0);
	
	return mis_id;
}

void rotate_actor_bones(actor *a)
{
	struct CalSkeleton *skel;
	struct CalBone *bone[2];
	struct CalQuaternion *rot[2];
	struct CalQuaternion *tmp;

	if (!a->rotating_bones)
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

		a->cal_rotation_blend += a->cal_rotation_blend_speed;

		tmp2 = CalQuaternion_New();

		CalQuaternion_Set(tmp2, 0.0, 0.0, 0.0, 1.0);

		CalQuaternion_Blend(tmp, (1.0 - a->cal_rotation_blend)/2.0, a->cal_starting_rotation);
		CalQuaternion_Blend(tmp2, a->cal_rotation_blend/2.0, a->cal_ending_rotation);

		CalQuaternion_Multiply(tmp, tmp2);

		CalQuaternion_Delete(tmp2);
	}
	else {
		float *quat;

		a->cal_rotation_blend = 1.0;

		quat = CalQuaternion_Get(a->cal_ending_rotation);
		if (fabs(quat[0]) < EPSILON &&
			fabs(quat[1]) < EPSILON &&
			fabs(quat[2]) < EPSILON &&
			fabs(1.0 - quat[3]) < EPSILON) {
			a->rotating_bones = 0;
			printf("stopping bones rotation\n");
		}

		CalQuaternion_Blend(tmp, 0.5, a->cal_ending_rotation);
	}

	CalQuaternion_Multiply(rot[0], tmp);
	CalQuaternion_Multiply(rot[1], tmp);

	CalBone_SetRotation(bone[0], rot[0]);
	CalBone_SetRotation(bone[1], rot[1]);
	CalBone_CalculateState(bone[0]);
	CalBone_CalculateState(bone[1]);

	CalQuaternion_Delete(tmp);
}

#endif // MISSILES
