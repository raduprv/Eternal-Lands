/*!
 * \file
 * \ingroup load_3d
 * \brief the E3D data format and supporting functions.
 */
#ifndef __E3D_H__
#define __E3D_H__

#include "e3d_object.h"
#include "vmath.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \name    3D objects array sizes
 */
/*! @{ */
#define MAX_OBJ_3D 15000

#define OBJ_3D_ENTRABLE 	1
#define OBJ_3D_HARVESTABLE 	1<<1
#define OBJ_3D_BAG 			1<<2
 #define OBJ_3D_MINE			1<<3

//#define MAX_E3D_CACHE 1000 /* unused */
/*! @} */

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
	MATRIX4x4 matrix; /*!< translation and rotaion matrix */
   
	char self_lit; /*!< flag determining whether this object is self lit or not. Self lit objects contains their own lighs. */
	char blended; /*!< flag determining whether the object is blended with some other objects. */
	char display; /*!< flag determining whether the object is to be shown on screen. */
	char state; /*!< state flag for future expansion & data alignment. */
   
	float color[4]; /*!< color values (red, green, blue) for this object */

	e3d_object *e3d_data; /*!< e3d model data */
	unsigned int last_acessed_time; /*!< timestamp when this object was last accessed. */
	unsigned int flags; /*!< Flag determining whether this object is entrable/harvestable/bag */
	VECTOR4 clouds_planes[2]; /**< Clouds s and t planes. */

#ifdef CLUSTER_INSIDES
	short cluster;
#endif

} object3d;


extern object3d *objects_list[MAX_OBJ_3D]; /*!< global variable containing up to \see max_obj_3d \see object3d objects. */

//proto

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

#ifdef __cplusplus
} // extern "C"
#endif

#endif
