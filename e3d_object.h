/*!
 * \file
 * \ingroup load_3d
 * \brief the E3D data format and supporting functions.
 */
#ifndef	__E3D_OBJECT_H__
#define	__E3D_OBJECT_H__

#include "cache.h"

#define	VERTEX_FLOAT_COUNT		3
#define	NORMAL_FLOAT_COUNT		3
#define	TEXTURE_FLOAT_COUNT		2
#define	TANGENT_FLOAT_COUNT		3
#define	EXTRA_TEXTURE_FLOAT_COUNT	2

#define	OPTION_HAS_NORMAL		0x01
#define	OPTION_HAS_TANGENT		0x02
#define	OPTION_HAS_EXTRA_TEXTURE	0x04

#define OPTION_MATERIAL_TRANSPARENT	0x00000001

__inline__ static int is_ground(int vertex_options)
{
	return (vertex_options & OPTION_HAS_NORMAL) != 0;
}

__inline__ static int has_normal(int vertex_options)
{
	return (vertex_options & OPTION_HAS_NORMAL) == 0;
}

__inline__ static int has_tangent(int vertex_options)
{
	return (vertex_options & OPTION_HAS_TANGENT) != 0;
}

__inline__ static int has_extra_texture(int vertex_options)
{
	return (vertex_options & OPTION_HAS_EXTRA_TEXTURE) != 0;
}

__inline__ static int material_is_transparent(int options)
{
	return (options & OPTION_MATERIAL_TRANSPARENT) != 0;
}

__inline__ static int get_vertex_float_count(int vertex_options)
{
	int count;

	count = VERTEX_FLOAT_COUNT + TEXTURE_FLOAT_COUNT;
	if (has_normal(vertex_options))
	{
		count += NORMAL_FLOAT_COUNT;
	}
	if (has_tangent(vertex_options))
	{
		count += TANGENT_FLOAT_COUNT;
	}
	if (has_extra_texture(vertex_options))
	{
		count += EXTRA_TEXTURE_FLOAT_COUNT;
	}
	return count;
}

__inline__ static int get_vertex_size(int vertex_options)
{
	return get_vertex_float_count(vertex_options) * sizeof(float);
}

__inline__ static int get_texture_offset(int vertex_options)
{
	return 0;
}

__inline__ static int get_extra_texture_offset(int vertex_options)
{
	int size;

	size = get_texture_offset(vertex_options) + 2 * sizeof(float);

	return size;
}

__inline__ static int get_normal_offset(int vertex_options)
{
	int size;

	size = get_extra_texture_offset(vertex_options);
	if (has_extra_texture(vertex_options))
	{
		size += 2 * sizeof(float);
	}
	

	return size;
}

__inline__ static int get_tangent_offset(int vertex_options)
{
	int size;

	size = get_normal_offset(vertex_options);

	if (has_normal(vertex_options))
	{
		size += 3 * sizeof(float);
	}

	return size;
}

__inline__ static int get_vertex_offset(int vertex_options)
{
	int size;

	size = get_tangent_offset(vertex_options);

	if (has_tangent(vertex_options))
	{
		size += 3 * sizeof(float);
	}
	
	return size;
}

typedef struct
{
	GLuint texture_id;
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
	void* indicies; /*!< an array of el3d indicies */
	e3d_draw_list* materials; /*!< an array of triangle data for every material */
	int vertex_no; /*!< number of vertexe, normal, tangent, texture and extra texture coordinates in this object */
	int index_no; /*!< number of all indicies */
	int material_no; /*!< number of materials in this object */
	int index_type; /*!< type of the indicies: GL_UNSIGNED_BYTE, GL_UNSIGNED_WORD or GL_UNSIGNED_INT */

	GLuint vertex_vbo; /*!< an array of e3d vertex data */
	GLuint indicies_vbo; /*!< an array of el3d indicies */

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

	int vertex_options;	/*!< flags determining whether this is a ground object, has tangents or extra uv's */

	cache_item_struct *cache_ptr; /*!< pointer to a cache item. If this is !=NULL, this points to a valid cached item of this object */
	MD5_DIGEST md5; /*!< the MD5 digest of the file */
	char file_name[128]; /*!< filename where this object is stored. */
} e3d_object;

#endif	//__E3D_OBJECT_H__
