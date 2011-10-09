#ifndef __obj_3d_H__
#define __obj_3d_H__

#include "e3d.h"

void draw_3d_object(object3d * object_id);
e3d_object * load_e3d_cache(char * file_name);
int add_e3d (char * file_name, float x_pos, float y_pos, float z_pos, float x_rot, float y_rot, float z_rot, char self_lit, char blended, float r, float g, float b);
int add_e3d_keep_deleted (char * file_name, float x_pos, float y_pos, float z_pos, float x_rot, float y_rot, float z_rot, char self_lit, char blended, float r, float g, float b);
void display_objects();
e3d_object * load_e3d(char *file_name);
void compute_clouds_map(object3d * object_id);
void clear_clouds_cache();
void destroy_3d_object(int i);
void flag_for_destruction();
void destroy_the_flagged();
void clear_e3d_heightmap (int K);
void add_e3d_heightmap(int K, int D);
#endif
