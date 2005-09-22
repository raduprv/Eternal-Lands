#ifdef	TERRAIN
/*!
 * \file
 * \ingroup 	display_utils
 * \brief 	Terrain calculation.
 */
#ifndef	TERRAIN_H
#define	TERRAIN_H

/*!
 * The hf-map is a float array of map_tile_size_x*map_tile_size_y size 
 * storing the height for the vertexes of the terrain.
 */
extern float *hf_map;

/*!
 * \ingroup 	display_utils
 * \brief 	Initialises the terrain data.
 *
 * The function reads the compressed terrain height map from file. After 
 * uncompression the functions for creating normal maps are called.
 * \param 	file The file descriptor for the terrain height map data.
 * \param 	size_x The size of the terrain height map in x direction.
 * \param 	size_y The size of the terrain height map in y direction.
 *  
 * \callgraph
 */
extern void init_terrain(FILE *file, const unsigned int size_x, const unsigned int size_y);

/*!
 * \ingroup 	display_utils
 * \brief 	Frees the terrain data.
 *
 * Frees the normal maps and the terrain data.
 *  
 * \callgraph
 */
extern void free_terrain();

#endif
#endif
