#ifdef	TERRAIN
/*!
 * \file
 * \ingroup 	display_utils
 * \brief 	Terrain calculation.
 */
#ifndef	TERRAIN_H
#define	TERRAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * The hf-map is a float array of map_tile_size_x*map_tile_size_y size 
 * storing the height for the vertexes of the terrain.
 */
extern float *hf_map;
#ifdef MAP_EDITOR2
/*!
 * The h-map is the full terrain height map, an array of unsigned shorts, 
 * storing the height for normal mapping of the terrain.
 */
extern unsigned short *h_map;
#endif
/*!
 * The extra_texture_coordinates an array with extra texture coordinates for
 * multitexturing terrain.
 */
extern TEXTCOORD2 *extra_texture_coordinates;

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

#ifdef __cplusplus
} // extern "C"
#endif

#endif
#endif
