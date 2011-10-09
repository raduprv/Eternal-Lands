#ifndef __E3D_H__
#define __E3D_H__

#define MAX_OBJ_3D 15000
#define MAX_E3D_CACHE 1000


#include "../md5.h"
#include "../e3d_object.h"


typedef struct
{
   char file_name[80];
   float x_pos;
   float y_pos;
   float z_pos;

   float x_rot;
   float y_rot;
   float z_rot;

   char self_lit;
   char blended;
   float color[4];

   e3d_object *e3d_data;
   unsigned int last_acessed_time;
}object3d;

typedef struct
{
	char file_name[128];
	e3d_object * e3d_id;
	int flag_for_destruction;
}e3d_cache_struct;

extern e3d_cache_struct e3d_cache[1000];


extern object3d *objects_list[MAX_OBJ_3D];


//proto
e3d_object * load_e3d(char *file_name);

void e3d_enable_vertex_arrays(e3d_object *e3d_data, Uint32 use_lightning, Uint32 use_textures);
void e3d_disable_vertex_arrays();
void set_emission(object3d * object_id);

#endif

