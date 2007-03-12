/*!
 * \file
 * \ingroup maps
 * \brief handles sectoring and partitioning of maps
 */
#ifndef __SECTOR_H__
#define __SECTOR_H__

#include "misc.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef	NEW_FRUSTUM
/*!
 * returns the sector associated with the coordinates x and y
 */
#define SECTOR_GET(x,y) (clampi((int)(y)/12, 0, (tile_map_size_y>>2) - 1)*(tile_map_size_x>>2) + clampi((int)(x)/12, 0, (tile_map_size_x>>2) - 1))

#define MAX_3D_OBJECTS 400 /*!< maximum number of 3d objects in a sector */

/*!
 * map_sector handles the data of one sector
 */
typedef struct{
	Uint32 objects_checksum; /*!< a MD5 checksum to check the objects in this sector */
	Uint32 tiles_checksum; /*!< a MD5 checksum for the tiles in this sector */
	short e3d_local[MAX_3D_OBJECTS]; /*!< array of local \see e3d objects in this sector */
	short e2d_local[100]; /*!< array of local \see e2d objects in this sector */
	short lights_local[4]; /*!< up to 4 lights are possible in one sector */
	short particles_local[8]; /*!< up to 8 particles are possible in one sector */
}map_sector;

extern map_sector sectors[256*256]; /*!< the global variable sectors stores all the currently loaded \see map_sector. */

/*!
 * \ingroup maps
 * \brief Returns the <em>parent</em> sector of the given sector and stores the start and end coordinates in the given parameters.
 *
 *      Returns the supersector of the given sector and stores the start coordinates in sx and sy and the end coordinates in ex and ey.
 *
 * \param sector    handle for the sector for which we search the supersector
 * \param sx        variable to hold the x coordinate of the start position
 * \param sy        variable to hold the y coordinate of the start position
 * \param ex        variable to hold the x coordinate of the end position
 * \param ey        variable to hold the y coordinate of the end position
 */
void get_supersector(int sector, int *sx, int *sy, int *ex, int *ey);

/*!
 * \ingroup maps
 * \brief Adds an 3d object, specified by objectid to the current sector.
 *
 *      Adds an 3d object, specified by objectid to the current sector.
 *
 * \param objectid  points to the 3d object to add.
 * \retval int
 */
int sector_add_3do(int objectid);
#endif

/*!
 * \ingroup maps
 * \brief Adds the given particle to the current sector
 *
 *      Adds the particle given by objectid to the current sector
 *
 * \param objectid  points to the particle to add
 * \retval int
 */
int sector_add_particle(int objectid);

/*!
 * \ingroup maps
 * \brief Adds the given 2d object to the current sector
 *
 *      Adds the 2d object given by objectid to the current sector
 *
 * \param objectid  points to the 2d object to add
 * \retval int
 */
int sector_add_2do(int objectid);

/*!
 * \ingroup maps
 * \brief Adds a map to the sector
 *
 *      Adds a previously loaded map to the current sector
 *
 * \callgraph
 */
void sector_add_map();

#ifdef __cplusplus
} // extern "C"
#endif

#endif
