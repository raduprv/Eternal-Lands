#ifndef __E3D_H__
#define __E3D_H__

#define max_obj_3d 15000
#define max_e3d_cache 1000
extern int highest_obj_3d;

//the new array structures
typedef struct
{
  float x;
  float y;
  float z;
}e3d_array_vertex;

typedef struct
{
  float nx;
  float ny;
  float nz;
}e3d_array_normal;

typedef struct
{
  float u;
  float v;
}e3d_array_uv_main;

typedef struct
{
  float u;
  float v;
}e3d_array_uv_detail;

typedef struct
{
  int texture_id;
  int count;
  int start;
}e3d_array_order;

//end of the new things

typedef struct
{
	int material_id;
	char material_name[40];
}e3d_material;

typedef struct
{
  float x;
  float y;
  float z;

  float nx;
  float ny;
  float nz;
}e3d_vertex;

typedef struct
{
  int a;
  int b;
  int c;
  float au;
  float av;
  float bu;
  float bv;
  float cu;
  float cv;
  int material;
}e3d_face;


typedef struct
{
	char magic[4];
	int face_no;
	int face_size;
	int face_offset;
	int vertex_no;
	int vertex_size;
	int vertex_offset;
	int material_no;
	int material_size;
	int material_offset;
	float min_x;
	float min_y;
	float min_z;
	float max_x;
	float max_y;
	float max_z;
	int crc;
	char is_ground;
	char char_reserved_1;
	char char_reserved_2;
	char char_reserved_3;
	int reserved_2;
	int reserved_3;
	int reserved_4;
	int reserved_5;
	int reserved_6;
	char is_transparent;

}e3d_header;

typedef struct
{
	e3d_array_vertex *array_vertex;
	e3d_array_normal *array_normal;
	e3d_array_uv_main *array_uv_main;
	e3d_array_order *array_order;
	int materials_no;
	int face_no;
	float min_x;
	float min_y;
	float min_z;
	float max_x;
	float max_y;
	float max_z;
	char is_transparent;
	char is_ground;

    cache_item_struct	*cache_ptr;
	char file_name[128];
}e3d_object;

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
   float r,g,b;
   e3d_array_uv_detail *clouds_uv;

   e3d_object *e3d_data;
   unsigned int last_acessed_time;
}object3d;


extern object3d *objects_list[max_obj_3d];


//proto
e3d_object * load_e3d(char *file_name);
e3d_object * load_e3d_detail(e3d_object *cur_object);
int add_e3d(char * file_name, float x_pos, float y_pos, float z_pos, float x_rot, float y_rot, float z_rot, char self_lit, char blended, float r, float g, float b);
Uint32 free_e3d_va(e3d_object *e3d_id);

#endif

