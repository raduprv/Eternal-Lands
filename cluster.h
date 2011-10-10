#ifdef CLUSTER_INSIDES

#ifndef CLUSTER_H
#define CLUSTER_H

#include "tiles.h"
#include "bbox_tree.h"
#include "e3d.h"
#include "lights.h"
#include "particles.h"
#ifdef MAP_EDITOR
#include "map_editor/2d_objects.h"
#include "map_editor/3d_objects.h"
#else
#include "2d_objects.h"
#include "3d_objects.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \ingroup maps
 * \brief Update the occupation array with the height map
 *
 *	Mark the fields in the occupation array that are walkable.
 *
 * \param occupied   The occupation array
 * \param height_map The height map 
 */
static __inline__ void update_occupied_with_height_map (char* occupied, const unsigned char* height_map)
{
	int i;

	for (i = 0; i < tile_map_size_x*tile_map_size_y*6*6; i++)
		if (height_map[i]) occupied[i] = 1;
}

/*!
 * \ingroup maps
 * \brief Update the occupation array with the tile map
 *
 *	Mark the fields in the occupation array that are occupied by 
 *	ground tiles
 *
 * \param occupied The occupation array
 * \param tile_map The tile map 
 */
static __inline__ void update_occupied_with_tile_map (char* occupied, const unsigned char* tile_map)
{
	int nx = tile_map_size_x * 6;
	int ny = tile_map_size_y * 6;
	int x, y, idx;

	idx = 0;
	for (y = 0; y < ny; y += 6)
	{
		for (x = 0; x < nx; x += 6, idx++)
		{
			if (tile_map[idx] != 255)
			{
				int offset = y*nx + x;
				int i, j;

				for (j = 0; j < 6; j++)
					for (i = 0; i < 6; i++)
						occupied[offset+j*nx+i] = 1;
			}
		}
	}
}

/*!
 * \ingroup maps
 * \brief Update the occupation array with a bounding box
 *
 *	Mark the fields in the occupation array that are included in
 *	the bounding box.
 *
 * \param occupied The occupation array
 * \param box      The axis aligned bounding box
 */
static __inline__ void update_occupied_with_bbox (char* occupied, const AABBOX* box)
{
	int xs = (int) (box->bbmin[X] / 0.5f);
	int ys = (int) (box->bbmin[Y] / 0.5f);
	int xe = (int) (box->bbmax[X] / 0.5f) + 1;
	int ye = (int) (box->bbmax[Y] / 0.5f) + 1;
	int x, y;

	if (xs < 0) xs = 0;
	if (ys < 0) ys = 0;
	if (xe > tile_map_size_x*6) xe = tile_map_size_x*6;
	if (ye > tile_map_size_y*6) ye = tile_map_size_y*6;

	for (y = ys; y < ye; y++)
	{
		for (x = xs; x < xe; x++)
			occupied[y*tile_map_size_x*6+x] = 1;
	}
}

/*!
 * \ingroup maps
 * \brief Update the occupation array with a 3D objec t
 *
 *      Mark the fields in the occupation array that are included in
 *      the bounding box of the 3D object.
 *
 * \param occupied The occupation array
 * \param id       The position in objects_list of the 3D object
 */

static __inline__ void update_occupied_with_3d (char* occupied, int id)
{
	const e3d_object* obj;
	int i;
	AABBOX box;
#ifdef MAP_EDITOR
	MATRIX4x4 matrix;
#endif

	if (id < 0 || id >= MAX_OBJ_3D || !objects_list[id])
		return;

	obj = objects_list[id]->e3d_data;
	if (!obj)
		return;

	for (i = 0; i < obj->material_no; i++)
	{
		box.bbmin[X] = obj->materials[i].min_x;
		box.bbmin[Y] = obj->materials[i].min_y;
		box.bbmin[Z] = obj->materials[i].min_z;
		box.bbmax[X] = obj->materials[i].max_x;
		box.bbmax[Y] = obj->materials[i].max_y;
		box.bbmax[Z] = obj->materials[i].max_z;
#ifdef MAP_EDITOR
		// the map editor doesn't store the object transformation matrices
		calc_rotation_and_translation_matrix (matrix, 
		                                      objects_list[id]->x_pos, objects_list[id]->y_pos, objects_list[id]->z_pos, 
		                                      objects_list[id]->x_rot, objects_list[id]->y_rot, objects_list[id]->z_rot);
		matrix_mul_aabb (&box, matrix); 
#else
		matrix_mul_aabb (&box, objects_list[id]->matrix);
#endif

		update_occupied_with_bbox (occupied, &box);
	}
}

/*!
 * \ingroup maps
 * \brief Update the occupation array with a 2D object
 *
 *      Mark the fields in the occupation array that are included in
 *      the bounding box of the 2D object.
 *
 * \param occupied The occupation array
 * \param id       The index in obj_2d_list of the 2D object
 */
static __inline__ void update_occupied_with_2d (char* occupied, int id)
{
	AABBOX box;

	if (get_2d_bbox (id, &box))
		update_occupied_with_bbox (occupied, &box);
}

/*!
 * \ingroup maps
 * \brief Update the occupation array with a light
 *
 *      Mark the position of the light as occupied
 *
 * \param occupied The occupation array
 * \param id       The index in lights_list of the light
 */
static __inline__  void update_occupied_with_light (char* occupied, int id)
{
	int x, y;

	if (id < 0 || id >= MAX_LIGHTS)
		return;

	x = (int) (lights_list[id]->pos_x / 0.5f);
	y = (int) (lights_list[id]->pos_y / 0.5f);

	if (x >= 0 && x < tile_map_size_x*6 && y >= 0 && y < tile_map_size_y*6)
		occupied[y*tile_map_size_x*6+x] = 1;
}

/*!
 * \ingroup maps
 * \brief Update the occupation array with a particle system
 *
 *      Mark the fields in the occupation array that are included in
 *      the bounding box of the particle system.
 *
 * \param occupied The occupation array
 * \param id       The index in particles_list of the particle system
 */
static __inline__ void update_occupied_with_particle_system (char* occupied, int id)
{
	AABBOX box;

	if (id < 0 || id >= MAX_PARTICLE_SYSTEMS || !particles_list[id])
		return;

	calc_bounding_box_for_particle_sys (&box, particles_list[id]);
	update_occupied_with_bbox (occupied, &box);
}

/*!
 * \ingroup maps
 * \brief Set the cluster map from file data
 *
 *	Allocate and read the cluster map from file data.
 *
 * \param data The cluster data from the map file
 * \note Either this function should be used, or when no cluster data is 
 *       present in the file, compute_clusters() should be used. Doing 
 *       both will lead to memory leaks and unnecesary CPU usage.
 */
void set_clusters (const char* data);

#ifdef MAP_EDITOR
/*!
 * \ingroup maps
 * \brief Get file data for the cluster map
 *
 *	Serialize the cluster map data, and return it through 
 *	character array \a data of length \a len.
 *
 * \param data Address where to store the pointer to the data
 * \param len  The length of the data in bytes
 * \note The array in \a *data will be dynamically allocated, and should 
 *       be \c free'd by the caller.
 */
void get_clusters (char** data, int *len);
#endif

/*!
 * \ingroup maps
 * \brief Group occupied areas into clusters
 *
 *	Detect and number contiguous occupied areas in the occupation array.
 *
 * \param occupied The occupation array
 * \note When reading maps, this function should only be used when the 
 *       cluster map is not present in the file, otherwise 
 *       set_clusters() should be used. Doing both will lead to memory 
 *       leaks and unnecesary CPU usage.
 */
void compute_clusters (const char* occupied);

/*!
 * \ingroup maps
 * \brief Get the cluster number of a position
 *
 *      Get the number of the visibility cluster to which point
 *      (\a x, \a y) belongs.
 *
 * \param x the x coordinate of the point to check
 * \param y the y coordinate of the point to check
 * \retval short The number of the visibility cluster, or 0 if the point
 *               is not inside any cluster.
 */
short get_cluster (int x, int y);

/*!
 * \ingroup maps
 * \brief Destroy the clusters array
 *
 *	Free the memory associated with the clusters array, and clear 
 *	the pointer associated with it.
 */
void destroy_clusters_array ();

#ifndef MAP_EDITOR
/*!
 * \ingroup maps
 * \brief Get the cluster where the actor is currently on
 *
 *	Check the cluster map at the actor's current position, and 
 *	return the number of the cluster that he's currently in.
 *
 * \retval short The number of the actor's current visibility cluster
 */
short get_actor_cluster ();
#endif

extern short current_cluster;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CLUSTER_H

#endif // CLUSTER_INSIDES
