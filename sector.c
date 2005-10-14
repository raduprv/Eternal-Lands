#include <stdlib.h>
#include <string.h>
#ifdef MAP_EDITOR2
#include "../map_editor2/global.h"
#else
#include "global.h"
#endif
#ifdef	NEW_FRUSTUM
#include <math.h>
#include "bbox_tree.h"
#endif

/* NOTE: This file contains implementations of the following, currently unused, and commented functions:
 *          Look at the end of the file.
 *
 * int sector_add_light(int);
 */

map_sector sectors[256*256];
int num_sectors=256*256;
Uint16 active_sector;
int current_sector;
#ifdef	NEW_FRUSTUM
BBOX_TREE* bbox_tree = NULL;

#define INVALID -1
#define GROUND 0
#define PLANT 1
#define FENCE 2
#endif


void get_supersector(int sector, int *sx, int *sy, int *ex, int *ey)
{
	int tile_map_size_y_4=tile_map_size_y>>2;
	int tile_map_size_x_4=tile_map_size_x>>2;
	int fy=sector/(tile_map_size_y_4);
	int fx=sector%(tile_map_size_x_4);

	*sx=fx-2;
	if(*sx<0)*sx=0;
	*sy=fy-2;
	if(*sy<0)*sy=0;
	*ex=fx+2;
	if(*ex>=tile_map_size_x_4)*ex=tile_map_size_x_4-1;
	*ey=fy+2;
	if(*ey>=tile_map_size_y_4)*ey=tile_map_size_y_4-1;
}


int sector_add_3do(int objectid)
{
	int i;
	int sector_no=SECTOR_GET(objects_list[objectid]->x_pos, objects_list[objectid]->y_pos);

	if(sector_no>=num_sectors) return -1;

	for(i=0;i<MAX_3D_OBJECTS;i++){
		if(sectors[sector_no].e3d_local[i]==-1){
			sectors[sector_no].e3d_local[i]=objectid;
//			add_change();
			return i;
		}
	}
	return -1;
}


#ifndef	NEW_FRUSTUM
int sector_add_2do(int objectid)
{
	int i;
	int sector_no=SECTOR_GET(obj_2d_list[objectid]->x_pos, obj_2d_list[objectid]->y_pos);

	if(sector_no>=num_sectors) return -1;

	for(i=0;i<100;i++){
		if(sectors[sector_no].e2d_local[i]==-1){
			sectors[sector_no].e2d_local[i]=objectid;
	//		add_change();
			return i;
		}
	}
	return -1;
}
#endif

int sector_add_particle(int objectid)
{
	int i;
	int sector_no=SECTOR_GET(particles_list[objectid]->x_pos, particles_list[objectid]->y_pos);

	if(sector_no>=num_sectors) return -1;

	for(i=0;i<8;i++){
		if(sectors[sector_no].particles_local[i]==-1){
			sectors[sector_no].particles_local[i]=objectid;
	//		add_change();
			return i;
		}
	}
	return -1;
}


#ifdef	NEW_FRUSTUM
static __inline__ void rotate_aabb(AABBOX* bbox, const float r_x, const float r_y, const float r_z)
{
	float matrix_1[16], matrix_2[16];
	matrix_1[0] = bbox->bbmax[X];
	matrix_1[1] = bbox->bbmax[Y];
	matrix_1[2] = bbox->bbmax[Z];
	matrix_1[3] = 1.0f;
	matrix_1[4] = bbox->bbmax[X];
	matrix_1[5] = bbox->bbmax[Y];
	matrix_1[6] = bbox->bbmin[Z];
	matrix_1[7] = 1.0f;
	matrix_1[8] = bbox->bbmax[X];
	matrix_1[9] = bbox->bbmin[Y];
	matrix_1[10] = bbox->bbmax[Z];
	matrix_1[11] = 1.0f;
	matrix_1[12] = bbox->bbmax[X];
	matrix_1[13] = bbox->bbmin[Y];
	matrix_1[14] = bbox->bbmin[Z];
	matrix_1[15] = 1.0f;
	matrix_2[0] = bbox->bbmin[X];
	matrix_2[1] = bbox->bbmax[Y];
	matrix_2[2] = bbox->bbmax[Z];
	matrix_2[3] = 1.0f;
	matrix_2[4] = bbox->bbmin[X];
	matrix_2[5] = bbox->bbmax[Y];
	matrix_2[6] = bbox->bbmin[Z];
	matrix_2[7] = 1.0f;
	matrix_2[8] = bbox->bbmin[X];
	matrix_2[9] = bbox->bbmin[Y];
	matrix_2[10] = bbox->bbmax[Z];
	matrix_2[11] = 1.0f;
	matrix_2[12] = bbox->bbmin[X];
	matrix_2[13] = bbox->bbmin[Y];
	matrix_2[14] = bbox->bbmin[Z];
	matrix_2[15] = 1.0f;
	glPushMatrix();
	glLoadIdentity();
	glRotatef(r_z, 0.0f, 0.0f, 1.0f);
	glRotatef(r_x, 1.0f, 0.0f, 0.0f);
	glRotatef(r_y, 0.0f, 1.0f, 0.0f);
	glPushMatrix();
	glMultMatrixf(matrix_1);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix_1);
	glPopMatrix();
	glMultMatrixf(matrix_2);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix_2);
	glPopMatrix();
	bbox->bbmin[X] = min(min(matrix_1[0], matrix_1[4]), min(matrix_1[8], matrix_1[12]));
	bbox->bbmin[X] = min(bbox->bbmin[X], min(min(matrix_2[0], matrix_2[4]), min(matrix_2[8], matrix_2[12])));
	bbox->bbmin[Y] = min(min(matrix_1[1], matrix_1[5]), min(matrix_1[9], matrix_1[13]));
	bbox->bbmin[Y] = min(bbox->bbmin[Y], min(min(matrix_2[1], matrix_2[5]), min(matrix_2[9], matrix_2[13])));
	bbox->bbmin[Z] = min(min(matrix_1[2], matrix_1[6]), min(matrix_1[10], matrix_1[14]));
	bbox->bbmin[Z] = min(bbox->bbmin[Z], min(min(matrix_2[2], matrix_2[6]), min(matrix_2[10], matrix_2[14])));

	bbox->bbmax[X] = max(max(matrix_1[0], matrix_1[4]), max(matrix_1[8], matrix_1[12]));
	bbox->bbmax[X] = max(bbox->bbmax[X], max(max(matrix_2[0], matrix_2[4]), max(matrix_2[8], matrix_2[12])));
	bbox->bbmax[Y] = max(max(matrix_1[1], matrix_1[5]), max(matrix_1[9], matrix_1[13]));
	bbox->bbmax[Y] = max(bbox->bbmax[Y], max(max(matrix_2[1], matrix_2[5]), max(matrix_2[9], matrix_2[13])));
	bbox->bbmax[Z] = max(max(matrix_1[2], matrix_1[6]), max(matrix_1[10], matrix_1[14]));
	bbox->bbmax[Z] = max(bbox->bbmax[Z], max(max(matrix_2[2], matrix_2[6]), max(matrix_2[10], matrix_2[14])));
}
#endif

// adds everything from the maps to the sectors
void sector_add_map()
{
	int i,j=0;
#ifndef	NEW_FRUSTUM
	int obj_3d_no=0;
	int obj_2d_no=0;
#endif
	//int lights_no=0;
	int particles_no=0;
	memset(sectors,-1,sizeof(map_sector)*256*256);

#ifndef	NEW_FRUSTUM
	for(i=0;i<MAX_OBJ_3D;i++)if(objects_list[i])obj_3d_no++;
	for(i=0;i<MAX_OBJ_2D;i++)if(obj_2d_list[i])obj_2d_no++;
#endif
	for(i=0;i<MAX_LIGHTS;i++){
		if(lights_list[i]){
			//lights_no++;
			num_lights= i;
		}
	}
	for(i=0;i<MAX_PARTICLE_SYSTEMS;i++){
		if(particles_list[i]){
			particles_no++;
		}
	}
#ifndef	NEW_FRUSTUM
	// 3d objects
	for(i=0;i<MAX_OBJ_3D;i++){
		if(j>obj_3d_no){
			break;
		}
		if(objects_list[i]){
			sector_add_3do(i);
			j++;
		}
	}

	//2d objects
	j=0;
	for(i=0;i<MAX_OBJ_2D;i++){
		if(j>obj_2d_no)
			break;
		if(obj_2d_list[i]){
			sector_add_2do(i);
			j++;
		}
	}
#endif
/*
	//lights
	j=0;
	for(i=0;i<MAX_LIGHTS;i++){
		if(j>lights_no)break;
		if(lights_list[i]){
			sector_add_light(i);
			j++;
		}
	}

	//particle systems
	j=0;
	for(i=0;i<MAX_PARTICLE_SYSTEMS;i++){
		if(j>particles_no)
			break;
		if(particles_list[i]){
			sector_add_particle(i);
			j++;
		}
	}
*/

#ifdef	NEW_FRUSTUM
	BBOX_ITEMS* items;
	float len_x, len_y, len_z;
	float r_x, r_y, r_z;
	AABBOX bbox;

	free_bbox_tree(bbox_tree);
	items = create_bbox_items(1024);

	for (i = 0; i < MAX_OBJ_3D; i++)
	{
		if (objects_list[i])
		{
			len_x = objects_list[i]->e3d_data->max_x - objects_list[i]->e3d_data->min_x;
			len_y = objects_list[i]->e3d_data->max_y - objects_list[i]->e3d_data->min_y;
			len_z = objects_list[i]->e3d_data->max_z - objects_list[i]->e3d_data->min_z;
			bbox.bbmin[X] = -len_x*0.5f;
			bbox.bbmax[X] = len_x*0.5f;
			bbox.bbmin[Y] = -len_y*0.5f;
			bbox.bbmax[Y] = len_y*0.5f;
			bbox.bbmin[Z] = -len_z*0.5f;
			bbox.bbmax[Z] = len_z*0.5f;
			r_x = objects_list[i]->x_rot;
			r_y = objects_list[i]->y_rot;
			r_z = objects_list[i]->z_rot;
			rotate_aabb(&bbox, r_x, r_y, r_z);
			bbox.bbmin[X] += objects_list[i]->x_pos;
			bbox.bbmin[Y] += objects_list[i]->y_pos;
			bbox.bbmin[Z] += objects_list[i]->z_pos;
			bbox.bbmax[X] += objects_list[i]->x_pos;
			bbox.bbmax[Y] += objects_list[i]->y_pos;
			bbox.bbmax[Z] += objects_list[i]->z_pos;
			add_3dobject_to_abt(items, i, &bbox);
		}
	}

	for (i = 0; i < MAX_OBJ_2D; i++)
	{
		if (obj_2d_list[i])
		{
			len_x = obj_2d_list[i]->obj_pointer->x_size;
			len_y = obj_2d_list[i]->obj_pointer->y_size;
			bbox.bbmin[X] = -len_x*0.5f;
			bbox.bbmax[X] = len_x*0.5f;
			if (obj_2d_list[i]->obj_pointer->object_type == GROUND)
			{
				bbox.bbmin[Y] = -len_y*0.5f;
				bbox.bbmax[Y] = len_y*0.5f;
			}
			else
			{
				bbox.bbmin[Y] = 0.0f;
				bbox.bbmax[Y] = len_y;
			}
			bbox.bbmin[Z] = 0.0f;
			bbox.bbmax[Z] = 0.0f;
			r_x = obj_2d_list[i]->x_rot;
			r_y = obj_2d_list[i]->y_rot;
			r_z = obj_2d_list[i]->z_rot;
			if (obj_2d_list[i]->obj_pointer->object_type == PLANT)
			{
				r_x += 90.0f;
				r_z = 0.0f;
				bbox.bbmin[X] *= sqrt(2);
				bbox.bbmax[X] *= sqrt(2);
				bbox.bbmin[Y] *= sqrt(2);
				bbox.bbmax[Y] *= sqrt(2);
			}
			if (obj_2d_list[i]->obj_pointer->object_type == FENCE) r_x += 90.0f;
			rotate_aabb(&bbox, r_x, r_y, r_z);
			bbox.bbmin[X] += obj_2d_list[i]->x_pos;
			bbox.bbmin[Y] += obj_2d_list[i]->y_pos;
			bbox.bbmin[Z] += obj_2d_list[i]->z_pos;
			bbox.bbmax[X] += obj_2d_list[i]->x_pos;
			bbox.bbmax[Y] += obj_2d_list[i]->y_pos;
			bbox.bbmax[Z] += obj_2d_list[i]->z_pos;
			add_2dobject_to_abt(items, i, &bbox);
		}
	}
	bbox_tree = build_bbox_tree(items);
	free_bbox_items(items);
#endif
}

/* currently UNUSED
int sector_add_light(int objectid)
{
	int i;
	int sector_no=SECTOR_GET(lights_list[objectid]->pos_x, lights_list[objectid]->pos_y);

	if(sector_no>=num_sectors) return -1;
	if(objectid >= num_lights)	num_lights= objectid+1;

	for(i=0;i<4;i++){
		if(sectors[sector_no].lights_local[i]==-1){
			sectors[sector_no].lights_local[i]=objectid;
	//		add_change();
			return i;
		}
	}
	return -1;
}
*/
