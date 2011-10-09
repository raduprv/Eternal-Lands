/*!
 * \file
 * \ingroup load_3d
 * \brief the E3D i/o data format and supporting functions.
 */
#ifndef	_E3D_IO_H_
#define _E3D_IO_H_

#ifdef	MAP_EDITOR
 #include "../map_editor/gl_init.h"
#else
 #include "../gl_init.h"
#endif
#include "elc_io.h"
#include "../e3d_object.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * the magic number for an e3d file.
 */
static const magic_number EL3D_FILE_MAGIC_NUMBER = {'e', '3', 'd', 'x'};

/*!
 * the current version number for an e3d file.
 */
static const version_number EL3D_FILE_VERSION_NUMBER_1_0 = {1, 0, 0, 0};
static const version_number EL3D_FILE_VERSION_NUMBER_1_1 = {1, 1, 0, 0};

/*!
 * the header structure for an e3d file.
 */
typedef struct
{
	int vertex_no;		/*!< the number of vertices in the file */
	int vertex_size;	/*!< the size of each vertex in the file */
	int vertex_offset;	/*!< the offset of the vertices in the file */
	int index_no;		/*!< the number of indicies in the file */
	int index_size;		/*!< the size of each index in the file */
	int index_offset;	/*!< the offset of the indicies in the file */
	int material_no;	/*!< the number of materials in the file */
 	int material_size;	/*!< the size of this material in the file */
	int material_offset;	/*!< the offset of the materials in the file */
	
	char vertex_options;	/*!< flag determining whether this is a ground object, has tangents or extra uv's */
	char vertex_format;	/*!< flag determining whether haf floats are used for position, uv and/or extra uv's and if normals and tangents are compressed */
	char reserved_2;
	char reserved_3;

} e3d_header;

/*!
 * defines the material used in e3d
 */
typedef struct
{
	int options;		/*!< flag determining whether this object is transparent or not */
	char material_name[128];	/*!< name of the material */

    /*!
     * \name min/max values of x,y,z
     */
    /*! @{ */
	float min_x;
	float min_y;
	float min_z;
	float max_x;
	float max_y;
	float max_z;
    /*! @} */
	
	int triangles_min_index;
	int triangles_max_index;
	int index;		/*!< index of the index list */
	int count;		/*!< number of indicies */
} e3d_material;

/*!
 * defines the extra texture used in e3d
 */
typedef struct
{
	char material_name[128];	/*!< name of the material */
} e3d_extra_texture;

e3d_object* load_e3d_detail(e3d_object* cur_object);

static __inline void load_e3d_detail_if_needed(e3d_object* e3d_data)
{
	if (use_vertex_buffers)
	{
		if (e3d_data->vertex_vbo == 0)
		{
			load_e3d_detail(e3d_data);
		}
	}
	else
	{
		if (e3d_data->vertex_data == NULL)
		{
			load_e3d_detail(e3d_data);
		}
	}
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif	// _E3D_IO_H_
