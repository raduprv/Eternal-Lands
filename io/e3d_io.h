/*!
 * \file
 * \ingroup load_3d
 * \brief the E3D i/o data format and supporting functions.
 */
#ifndef	__E3D_IO_H__
#define __E3D_IO_H__

#ifdef	MAP_EDITOR
#include "../../map_editor/global.h"
#include "../../map_editor/e3d.h"
#else
#include "../global.h"
#include "../e3d.h"
#endif
#include "elc_io.h"

__inline__ static int get_vertex_size(int vo)
{
	int size;

	size = 5;
	if (!is_ground(vo))
	{
		size += 3;
	}
	if (!has_tangen(vo))
	{
		size += 3;
	}
	if (!has_extra_uv(vo))
	{
		size += 2;
	}
	return size * sizeof(float);
}

/*!
 * the magic number for an e3d file.
 */
static const MAGIC_NUMBER EL3D_FILE_MAGIC_NUMBER = {'e', '3', 'd', 'x'};

/*!
 * the current version number for an e3d file.
 */
static const VERSION_NUMBER EL3D_FILE_VERSION_NUMBER = {1, 0, 0, 0};

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
	char reserved_1;
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

e3d_object* load_e3d_detail(e3d_object* cur_object);

static __inline void load_e3d_detail_if_needed(e3d_object* e3d_data)
{
	if ((e3d_data->vertex_data == NULL) && (e3d_data->vertex_vbo == 0))
	{
		load_e3d_detail(e3d_data);
	}
}
#endif
