#ifndef __E3D_H__
#define __E3D_H__

#define max_obj_3d 15000
#define max_e3d_cache 1000

#ifdef	NEW_E3D_FORMAT
#include "../elc/md5.h"

__inline__ static int is_ground(int vo)
{
	return (vo & 1) != 0;
}

__inline__ static int has_tangen(int vo)
{
	return (vo & 2) != 0;
}

__inline__ static int has_extra_uv(int vo)
{
	return (vo & 4) != 0;
}

typedef struct
{
	int texture_id;
	int options;	/*!< flag determining whether this object is transparent or not */

    /*!
     * \name min/max values of x,y,z as well as the max size/dimension of the material
     */
    /*! @{ */
	float min_x;
	float min_y;
	float min_z;
	float max_x;
	float max_y;
	float max_z;
	float max_size;
    /*! @} */

	void* triangles_indicies_index;
	int triangles_indicies_count;
	int triangles_indicies_min;
	int triangles_indicies_max;
} e3d_draw_list;

/*!
 * structure of an el3d object.
 */
typedef struct
{
	void* vertex_data; /*!< an array of e3d vertex data */
	void* normal_data; /*!< an array of e3d normal data */
	void* texture_data; /*!< an array of e3d texture data */
	void* tangent_data; /*!< an array of e3d tangent data */
	void* extra_uv_data; /*!< an array of e3d extra texture data */
	void* indicies; /*!< an array of el3d indicies */
	e3d_draw_list* materials; /*!< an array of triangle data for every material */
	int vertex_no; /*!< number of vertexe, normal, tangent, texture and extra texture coordinates in this object */
	int index_no; /*!< number of all indicies */
	int material_no; /*!< number of materials in this object */
	int index_type; /*!< type of the indicies: GL_UNSIGNED_BYTE, GL_UNSIGNED_WORD or GL_UNSIGNED_INT */

	GLuint vertex_vbo; /*!< an array of e3d vertex data */
	GLuint normal_vbo; /*!< an array of e3d normal data */
	GLuint texture_vbo; /*!< an array of e3d texture data */
	GLuint tangent_vbo; /*!< an array of e3d tangent data */
	GLuint extra_uv_vbo; /*!< an array of e3d extra texture data */
	GLuint indicies_vbo; /*!< an array of el3d indicies */
    
	char vertex_options;	/*!< flag determining whether this is a ground object, has tangents or extra uv's */

	MD5_DIGEST md5; /*!< the MD5 digest of the file */
	char file_name[128]; /*!< filename where this object is stored. */
} e3d_object;
#else	// NEW_E3D_FORMAT

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

}e3d_object;
#endif	// NEW_E3D_FORMAT

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
#ifndef	NEW_E3D_FORMAT
   e3d_array_uv_detail *clouds_uv;
#endif	// NEW_E3D_FORMAT

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


extern object3d *objects_list[max_obj_3d];


//proto
e3d_object * load_e3d(char *file_name);

#endif

