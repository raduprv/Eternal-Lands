/*!
 * \file
 * \ingroup load_3d
 * \brief the E3D data format and supporting functions.
 */
#ifndef	__E3D_OBJECT_H__
#define	__E3D_OBJECT_H__

#include "cache.h"
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	Uint32 position_offset;
	Uint32 texture_offset;
	Uint32 normal_offset;
	Uint32 color_offset;
	Uint32 size;
	Uint32 position_count;	
	Uint32 texture_count;	
	Uint32 normal_count;	
	Uint32 color_count;
	GLenum position_type;
	GLenum texture_type;
	GLenum normal_type;
	GLenum color_type;
} e3d_vertex_data;

typedef struct
{
	GLuint texture;			/**< texture. */
	Uint32 options;			/**< flag determining whether this object is transparent or not */

    /**
     * \name min/max values of x,y,z as well as the max size/dimension of the material
     */
    /** @{ */
	float min_x;
	float min_y;
	float min_z;
	float max_x;
	float max_y;
	float max_z;
	float max_size;
    /** @} */

	void* triangles_indices_index;
	Uint32 triangles_indices_count;
	Uint32 triangles_indices_min;
	Uint32 triangles_indices_max;
} e3d_draw_list;

/**
 * structure of an el3d object.
 */
typedef struct
{
	void* vertex_data;		/**< an array of e3d vertex data */
	void* indices;			/**< an array of el3d indices */
	e3d_draw_list* materials;	/**< an array of triangle data for every material */
	int vertex_no;			/**< number of vertexe, normal, tangent, texture and extra texture coordinates in this object */
	int index_no; 			/**< number of all indices */
	int material_no;		/**< number of materials in this object */
	int index_type;			/**< type of the indices: GL_UNSIGNED_BYTE, GL_UNSIGNED_WORD or GL_UNSIGNED_INT */

	GLuint vertex_vbo;		/**< an array of e3d vertex data */
	GLuint indices_vbo;		/**< an array of el3d indices */
	e3d_vertex_data* vertex_layout;	/**< Index of the vertex layout */

	/**
	 * \name min/max values of x,y,z as well as the max size/dimension of the material
	 */
	/** @{ */
	float min_x;
	float min_y;
	float min_z;
	float max_x;
	float max_y;
	float max_z;
	float max_size;
	/** @} */

	cache_item_struct *cache_ptr;	/**< pointer to a cache item. If this is !=NULL, this points to a valid cached item of this object */
	char file_name[128];		/**< filename where this object is stored. */
} e3d_object;

#ifdef __cplusplus
} // extern "C"
#endif

#endif	//__E3D_OBJECT_H__
