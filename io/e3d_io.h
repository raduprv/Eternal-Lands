/*!
 * \file
 * \ingroup load_3d
 * \brief the E3D i/o data format and supporting functions.
 */
#ifndef	__E3D_IO_H__
#define __E3D_IO_H__

#ifndef	E3D_CONVERTER
#include "../global.h"
#include "../e3d.h"
#endif
#include "elc_io.h"

/*!
 * the magic number for an e3d file.
 */
const static MAGIC_NUMBER EL3D_FILE_MAGIC_NUMBER = {'e', '3', 'd', 'x'};

/*!
 * the current version number for an e3d file.
 */
const static VERSION_NUMBER EL3D_FILE_VERSION_NUMBER = {1, 0, 0, 0};

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
	int material_offset;	/*!< the offset of the materials in the file */
	
	char is_ground;		/*!< flag determining whether this is a ground object or not */
	char reserved_1;
	char reserved_2;
	char reserved_3;

} e3d_header;

/*!
 * defines a vertex in e3d format, containing vertex coordinates as well as coordinates of the normal and texture.
 */
typedef struct
{
  /*!
   * \name texture coordinates
   */
  /*! @{ */
	float u;
	float v;
  /*! @} */

  /*!
   * \name normal coordinates
   */
  /*! @{ */
	float nx;
	float ny;
	float nz;
  /*! @} */

  /*!
   * \name vertex coordinates
   */
  /*! @{ */
	float x;
	float y;
	float z;
  /*! @} */
} e3d_T2F_N3F_V3F_vertex;

/*!
 * defines a vertex in e3d format, containing vertex coordinates as well as texture coordinates.
 */
typedef struct
{
  /*!
   * \name texture coordinates
   */
  /*! @{ */
	float u;
	float v;
  /*! @} */

  /*!
   * \name vertex coordinates
   */
  /*! @{ */
	float x;
	float y;
	float z;
  /*! @} */
} e3d_T2F_V3F_vertex;

/*!
 * defines the triangle list used in el3d
 */
typedef struct
{
	int index;		/*!< index of the index list */
	int count;		/*!< number of indicies */
} e3d_triangle_list;

/*!
 * defines the material used in e3d
 */
typedef struct
{
 	int material_size;	/*!< the size of this material in the file */
	int options;		/*!< flag determining whether this object is transparent or not */
	char material_name[64];	/*!< name of the material */

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
	e3d_triangle_list triangles;
	int triangle_strips_no;	/*!< number of triangle strip lists */
	/* el3d_triangle_list strip_lists[strip_list_no]; */
} e3d_material;

#ifndef	E3D_CONVERTER
e3d_object* load_e3d_detail(e3d_object* cur_object);

static __inline void load_e3d_detail_if_needed(e3d_object* e3d_data)
{
	if ((e3d_data->vertex_data == NULL)|| (e3d_data->materials == NULL) || (e3d_data->indicies == NULL))
	{
		load_e3d_detail(e3d_data);
	}
}
#endif
#endif
