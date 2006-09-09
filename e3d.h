/*!
 * \file
 * \ingroup load_3d
 * \brief the E3D data format and supporting functions.
 */
#ifndef __E3D_H__
#define __E3D_H__

#ifdef	NEW_FRUSTUM
#ifdef	NEW_E3D_FORMAT
#include "md5.h"
#endif
#include "vmath.h"
#endif
/*!
 * \name    3D objects array sizes
 */
/*! @{ */
#define MAX_OBJ_3D 15000
#ifndef	NEW_FRUSTUM
#define MAX_NEAR_3D_OBJECTS 1000
#define MAX_NEAR_BLENDED_3D_OBJECTS 25
#endif

#define OBJ_3D_ENTRABLE 	1
#define OBJ_3D_HARVESTABLE 	1<<1
#define OBJ_3D_BAG 			1<<2

//#define MAX_E3D_CACHE 1000 /* unused */
/*! @} */

extern int highest_obj_3d;  /*!< pointer to the highes 3D object in map */
#ifndef	NEW_FRUSTUM
extern int no_near_3d_objects;
#endif

#ifdef	NEW_E3D_FORMAT
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
	int triangle_strips_no;
	void** triangle_strips_indicies_index;
	int* triangle_strips_indicies_count;
} e3d_draw_list;

/*!
 * structure of an el3d object.
 */
typedef struct
{
	void* vertex_data; /*!< an array of el3d vertex data */
	void* indicies; /*!< an array of el3d indicies */
	e3d_draw_list* materials; /*!< an array of triangle data for every material */
	int vertex_no; /*!< number of vertexes, normals and texture coordinates in this object */
	int index_no; /*!< number of all indicies */
	int material_no; /*!< number of materials in this object */
	int index_type; /*!< type of the indicies: GL_UNSIGNED_BYTE, GL_UNSIGNED_WORD or GL_UNSIGNED_INT */

   	GLuint vbo[2]; /*!< Vertex buffer objects */
    
	char is_ground; /*!< flag determining whether this is a ground object or not */

	cache_item_struct *cache_ptr; /*!< pointer to a cache item. If this is !=NULL, this points to a valid cached item of this object */
	MD5_DIGEST md5; /*!< the MD5 digest of the file */
	char file_name[128]; /*!< filename where this object is stored. */
} e3d_object;
#else

//the new array structures

/*!
 * a vertex for use in an e3d_array
 */
typedef struct
{
  float x;
  float y;
  float z;
}e3d_array_vertex;

/*!
 * a normal for use in an e3d_array
 */
typedef struct
{
  float nx;
  float ny;
  float nz;
}e3d_array_normal;

/*!
 * main texture coordinates for use in an e3d_array
 */
typedef struct
{
  float u;
  float v;
}e3d_array_uv_main;

/*!
 * texture coordinates for use with detailed display.
 */
typedef struct
{
  float u;
  float v;
}e3d_array_uv_detail;

/*!
 * order of an e3d_array
 */
typedef struct
{
  int texture_id; /*!< id of the texture used by the order */
  int count;
  int start;
}e3d_array_order;

//end of the new things

/*!
 * defines the material used in e3d
 */
typedef struct
{
	int material_id; /*!< unique id for this material */
	char material_name[40]; /*!< name of the material */
}e3d_material;

/*!
 * defines a vertex in e3d format, containing vertex coordinates as well as coordinates of the normal
 */
typedef struct
{
  /*!
   * \name vertex coordinates
   */
  /*! @{ */
  float x;
  float y;
  float z;
  /*! @} */

  /*!
   * \name normal coordinates
   */
  /*! @{ */
  float nx;
  float ny;
  float nz;
  /*! @} */
}e3d_vertex;

/*!
 * a (triangular) face in e3d format.
 */
typedef struct
{
  /*!
   * \name face vertices
   *    the ids of the vertices making up the face
   */
  /*! @{ */
  int a;
  int b;
  int c;
  /*! @} */
  /*!
   * \name face texture coordinates
   *    the uv coordinates of each of the vertices that make up the face
   */
  /*! @{ */
  float au;
  float av;
  float bu;
  float bv;
  float cu;
  float cv;
  /*! @} */
  int material; /*!< a pointer to the associated material */
}e3d_face;

/*!
 * the header structure for an e3d file.
 */
typedef struct
{
	char magic[4];/*!< file magic number */
	int face_no; /*!< number of faces in the file */
	int face_size; /*!< size (in bytes) of all faces in the file */
	int face_offset; /*!< offset address where, after the header the faces start */
	int vertex_no; /*!< number of vertices in the file */
	int vertex_size; /*!< size (in bytes) of all vertices in the file */
	int vertex_offset; /*!< offset address where, after the header, the vertices start */
	int material_no; /*!< number of materials in the file */
	int material_size; /*!< size (in bytes) of all materials in the file */
	int material_offset; /*!< offset addres where, after the header, the materials start */
    
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
	int crc; /*!< checksum of the file */
	char is_ground; /*!< flag determining whether this is a ground object or not */
    
    /*!
     * \name reserved chars
     */
    /*! @{ */
	char char_reserved_1;
	char char_reserved_2;
	char char_reserved_3;
    /*! @} */
    
    /*!
     * \name reserved ints
     */
    /*! @{ */
	int reserved_2;
	int reserved_3;
	int reserved_4;
	int reserved_5;
	int reserved_6;
    /*! @} */
	char is_transparent; /*!< flag determining whether this object is transparent or not. */

}e3d_header;

/*!
 * structure of an e3d object.
 */
typedef struct
{
	e3d_array_vertex *array_vertex; /*!< an array of e3d vertices */
	e3d_array_normal *array_normal; /*!< an array of e3d normals */
	e3d_array_uv_main *array_uv_main; /*!< an array of e3d main texture coordinates */
	e3d_array_order *array_order; /*!< an order array */
	int materials_no; /*!< number of materials in this object */
	int face_no; /*!< number of faces for this object */

   	GLuint vbo[3];/*!< Vertex buffer objects*/

#ifndef	NEW_FRUSTUM
	float radius;
#endif
    
    /*!
     * \name min/max of x,y,z as well as the max size/dimension
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
    
	char is_transparent; /*!< flag determining whether this object is transparent or not */
	char is_ground; /*!< flag determining whether this is a ground object or not */

	cache_item_struct *cache_ptr; /*!< pointer to a cache item. If this is !=NULL, this points to a valid cached item of this object */
	char file_name[128]; /*!< filename where this object is stored. */
}e3d_object;
#endif

/*!
 * object3d structure
 */
typedef struct
{
   char file_name[80]; /*!< filename where the 3d object is stored in */
   
   int id; /*!< The object ID */
   /*!
    * \name position vector
    */
   /*! @{ */
   float x_pos; /*!< x coordinate of the position */
   float y_pos; /*!< y coordinate of the position */
   float z_pos; /*!< z coordinate of the position */
   /*! @} */

    /*!
     * \name rotation vector
     */
   /*! @{ */
   float x_rot; /*!< x coordinate of the rotation */
   float y_rot; /*!< y coordinate of the rotation */
   float z_rot; /*!< z coordinate of the rotation */
#ifdef	NEW_FRUSTUM
   MATRIX4x4 matrix; /*!< translation and rotaion matrix */
#endif
   
   char self_lit; /*!< flag determining whether this object is self lit or not. Self lit objects contains their own lighs. */
   char blended; /*!< flag determining whether the object is blended with some other objects. */
   char display; /*!< flag determining whether the object is to be shown on screen. */
   char state; /*!< state flag for future expansion & data alignment. */
   
   float r,g,b; /*!< color values (red, green, blue) for this object */
#ifndef	NEW_E3D_FORMAT
   e3d_array_uv_detail *clouds_uv; /*!< detailed texture coordinates used by clouds. */
   GLuint cloud_vbo; /*! the vertex buffer object for the clouds uv*/
#endif

   e3d_object *e3d_data; /*!< e3d model data */
   unsigned int last_acessed_time; /*!< timestamp when this object was last accessed. */
   unsigned int flags; /*!< Flag determining whether this object is entrable/harvestable/bag */
}object3d;


extern object3d *objects_list[MAX_OBJ_3D]; /*!< global variable containing up to \see max_obj_3d \see object3d objects. */

//proto

#ifndef	NEW_E3D_FORMAT
/*!
 * \ingroup load_3d
 * \brief   loads detail, attachs it to the \a cur_object and returns it.
 *
 *      Loads detail, attachs it to the \a cur_object and returns it.
 *
 * \param cur_object    the object for which detail is loaded and attached.
 * \retval e3d_object*  a pointer to the modified \a cur_object.
 * \callgraph
 */
e3d_object * load_e3d_detail(e3d_object *cur_object);
#endif

/*!
 * \ingroup load_3d
 * \brief   free_e3d_va
 *
 *      free_e3d_va(e3d_object*) to be documented.
 *
 * \param e3d_id    id of the \see e3d_object
 * \retval Uint32
 * \sa destroy_e3d
 * \sa init_e3d_cache
 */
Uint32 free_e3d_va(e3d_object *e3d_id);

#endif
