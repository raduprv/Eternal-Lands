#ifndef __obj_2d_H__
#define __obj_2d_H__

#include "../bbox_tree.h"

#define MAX_OBJ_2D 15000
#define MAX_OBJ_2D_DEF 1000

typedef struct
{
	float u_start;
	float u_end;
	float v_start;
	float v_end;
	float x_size;
	float y_size;
	float alpha_test;
	int object_type;
	int texture_id;
}obj_2d_def;

typedef struct
{
  char file_name[80];
  float x_pos;
  float y_pos;
  float z_pos;
  float x_rot;
  float y_rot;
  float z_rot;
  short sector;
  obj_2d_def *obj_pointer;
}obj_2d;

typedef struct
{
	char file_name[128];
	obj_2d_def *obj_2d_def_id;
}obj_2d_cache_struct;

extern obj_2d_cache_struct obj_2d_def_cache[MAX_OBJ_2D_DEF];

extern obj_2d *obj_2d_list[MAX_OBJ_2D];

void draw_2d_object(obj_2d * object_id);
obj_2d_def * load_obj_2d_def(char *file_name);
obj_2d_def * load_obj_2d_def_cache(char * file_name);
#ifdef CLUSTER_INSIDES
int get_2d_bbox (int id, AABBOX* box);
#endif
int add_2d_obj(char * file_name, float x_pos, float y_pos, float z_pos, float x_rot, float y_rot, float z_rot);
void display_2d_objects();

#endif
