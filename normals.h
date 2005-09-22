#ifdef	TERRAIN
/*!
 * \file
 * \ingroup 	display_utils
 * \brief 	Normal map calculation.
 */
#ifndef NORMAL_H
#define NORMAL_H

#include <stdlib.h>
#include <stdio.h>
#include <GL/gl.h>
#include "terrain.h"

/*!
 * The number of normals per vertex in x direction.
 */
#define NORMALS_PER_VERTEX_X		4
/*!
 * The number of normals per vertex in y direction.
 */
#define NORMALS_PER_VERTEX_Y		4
/*!
 * The number of vertexes per tile in x direction.
 */
#define VERTEXES_PER_TILE_X		6
/*!
 * The number of vertexes per tile in y direction.
 */
#define VERTEXES_PER_TILE_Y		6
/*!
 * The number of normals per tile in x direction.
 */
#define NORMALS_PER_TILE_X		(VERTEXES_PER_TILE_X*NORMALS_PER_VERTEX_X)
/*!
 * The number of normals per tile in y direction.
 */
#define NORMALS_PER_TILE_Y		(VERTEXES_PER_TILE_Y*NORMALS_PER_VERTEX_Y)

/*!
 * The normal texture size in x direction.
 */
#define	NORMAL_TEXTURE_MAX_X		128
/*!
 * The normal texture size in x direction.
 */
#define	NORMAL_TEXTURE_MAX_Y		128
/*!
 * The number of tiles per normal texture in x direction.
 */
#define TILES_PER_NORMAL_TEXTURE_X	(NORMAL_TEXTURE_MAX_X/NORMALS_PER_TILE_X)
/*!
 * The number of tiles per normal texture in y direction.
 */
#define TILES_PER_NORMAL_TEXTURE_Y	(NORMAL_TEXTURE_MAX_Y/NORMALS_PER_TILE_Y)
/*!
 * The normal texture size in x direction, rounded to full tiles.
 */
#define	NORMAL_MAP_MAX_X		(TILES_PER_NORMAL_TEXTURE_X*NORMALS_PER_TILE_X)
/*!
 * The normal texture size in y direction, rounded to full tiles.
 */
#define	NORMAL_MAP_MAX_Y		(TILES_PER_NORMAL_TEXTURE_Y*NORMALS_PER_TILE_Y)
/*!
 * The pad betwen normal texture and normal map x direction.
 */
#define NORMAL_MAP_PAD_X		(NORMAL_TEXTURE_MAX_X-NORMAL_MAP_MAX_X)
/*!
 * The pad betwen normal texture and normal map y direction.
 */
#define NORMAL_MAP_PAD_Y		(NORMAL_TEXTURE_MAX_Y-NORMAL_MAP_MAX_Y)

/*!
 * The units per tile in x direction.
 */
#define UNITS_PER_TILE_X		3
/*!
 * The units per tile in y direction.
 */
#define UNITS_PER_TILE_Y		3
/*!
 * The units per vertex in x direction.
 */
#define UNITS_PER_VERTEX_X		(((float)UNITS_PER_TILE_X)/VERTEXES_PER_TILE_X)
/*!
 * The units per vertex in y direction.
 */
#define UNITS_PER_VERTEX_Y		(((float)UNITS_PER_TILE_Y)/VERTEXES_PER_TILE_Y)

/*!
 * \ingroup 	display_utils
 * \brief 	Inits the normal mapping.
 * 
 * Inits the normal mapping by calling create_normal_texture_IDs to get
 * the normal texture ids and then calls calc_normal_map.
 * \param 	h_map The terrain height map.
 * \param 	size_x The size of the terrain height map in x direction.
 * \param 	size_y The size of the terrain height map in y direction.
 * \param	h_scale The scale of the terrain height (z direction).
 *  
 * \callgraph
 */
extern void init_normal_mapping(unsigned short *h_map, const unsigned int size_x, 
		const unsigned int size_y, const float h_scale);
/*!
 * \ingroup 	display_utils
 * \brief 	Frees the texture ids for the normap texture maps.
 * 
 * Frees the texture ids for the normap texture maps.
 *  
 * \callgraph
 */
extern void free_normal_mapping();

/*!
 * The array of texture ids for the normap texture maps.
 */
extern GLuint* normal_map_IDs;
/*!
 * The size of the normap map in x direction.
 */
extern unsigned int normal_map_size_x;
/*!
 * The size of the normap map in y direction.
 */
extern unsigned int normal_map_size_y;

/*!
 * \ingroup 	display_utils
 * \brief 	Returns the ID normal texture ID of the tile for normal mapping.
 *
 * Returns the normap texture ID of tile (tile_x, tile_y) for normal mapping.
 * \param 	tile_x The tile number in x direction.
 * \param 	tile_y The tile number in y direction.
 * \retval 	GLuint The ID of the normal texture for this tile.
 *  
 * \callgraph
 */
static inline GLuint get_normal_texture_ID(const unsigned int tile_x, const unsigned int tile_y)
{
	unsigned int x, y;
	unsigned int normal_texture_count_x;

	x = tile_x/TILES_PER_NORMAL_TEXTURE_X;
	y = tile_y/TILES_PER_NORMAL_TEXTURE_Y;
	normal_texture_count_x = (normal_map_size_x+NORMAL_MAP_MAX_X-1)/NORMAL_MAP_MAX_X;
	return normal_map_IDs[x+y*normal_texture_count_x];
}

/*!
 * \ingroup 	display_utils
 * \brief 	Returns the u texture coordinate of the vertex for normal mapping.
 *
 * Returns the u texture coordinate of the vertex (x, *) in tile (tile_x, *) for normal mapping.
 * \param	tile_x The tile number in x direction.
 * \param	x The vertex number in x direction for this tile.
 * \retval	float The normal texture u texture coordinate for this vertex.
 *  
 * \callgraph
 */
static inline float get_texture_coord_u(const unsigned int tile_x, const unsigned int x)
{
	float coord;
	float pad;
	pad = (NORMAL_TEXTURE_MAX_X-NORMAL_MAP_MAX_X)/2.0f;
	coord = (x+(tile_x%TILES_PER_NORMAL_TEXTURE_X)*VERTEXES_PER_TILE_X)*NORMALS_PER_VERTEX_X+pad;
	coord /= NORMAL_TEXTURE_MAX_X;
	return coord;
}

/*!
 * \ingroup 	display_utils
 * \brief 	Returns the v texture coordinate of the vertex for normal mapping.
 *
 * Returns the v texture coordinate of the vertex (*, y) in tile (*, tile_y) for normal mapping.
 * \param 	tile_y The tile number in y direction.
 * \param 	y The vertex number in y direction for this tile.
 * \retval 	float The normal map v texture coordinate for this vertex.
 *  
 * \callgraph
 */
static inline float get_texture_coord_v(const unsigned int tile_y, const unsigned int y)
{
	float coord;
	float pad;
	pad = (NORMAL_TEXTURE_MAX_Y-NORMAL_MAP_MAX_Y)/2.0f;
	coord = (y+(tile_y%TILES_PER_NORMAL_TEXTURE_Y)*VERTEXES_PER_TILE_Y)*NORMALS_PER_VERTEX_Y+pad;
	coord /= NORMAL_TEXTURE_MAX_Y;
	return coord;
}

/*!
 * \ingroup 	display_utils
 * \brief 	Returns the height of the vertex.
 *
 * Returns the height of the vertex (x, y) in tile (tile_x, tile_y).
 * \param	tile_x The tile number in x direction.
 * \param	tile_y The tile number in y direction.
 * \param	x The vertex number in x direction for this tile.
 * \param	y The vertex number in y direction for this tile.
 * \retval	float The height (z coordinate) for this vertex.
 *  
 * \callgraph
 */
static inline float get_vertex_height(const unsigned int tile_x, const unsigned int tile_y, const unsigned int x, const unsigned int y)
{
	return hf_map[(y+tile_y*VERTEXES_PER_TILE_Y)*(normal_map_size_x/NORMALS_PER_VERTEX_X)+(x+VERTEXES_PER_TILE_X*tile_x)];
}

#endif
#endif
