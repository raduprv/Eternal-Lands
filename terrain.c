#ifdef	TERRAIN
/*!
 * \file
 * \ingroup 	display_utils
 * \brief 	Terrain calculation.
 */
#include <zlib.h>
#ifdef MAP_EDITOR2
#include "../map_editor2/global.h"
#else
#include "global.h"
#endif
#include "simd.h"

/*!
 * The hf-map is a float array of map_tile_size_x*map_tile_size_y size 
 * storing the height for the vertexes of the terrain.
 */
float *hf_map;
#ifdef MAP_EDITOR2
/*!
 * The h-map is the full terrain height map, an array of unsigned shorts, 
 * storing the height for normal mapping of the terrain.
 */
unsigned short *h_map;
#endif
/*!
 * The extra_texture_coordinates an array with extra texture coordinates for
 * multitexturing terrain.
 */
TEXTCOORD2 *extra_texture_coordinates;

/*! 
 * \ingroup 	display_utils
 * \brief 	Calculates the current height map for the vertexes of the terrain.
 *
 * Calculates the current height map for the vertexes of the terrain.
 * \param 	map_data The terrain height map.
 * \param 	size_x The size of the terrain height map in x direction. Must be a multiple
 * 		of four plus one.
 * \param 	size_y The size of the terrain height map in y direction. Must be a multiple
 * 		of four plus one.
 * \param 	h_scale The scale of the terrain height (z direction).
 * \param 	h_map The height map used for the vertexes of the terrain.
 *  
 * \callgraph
 */
static __inline__ void calc_current_terrain(unsigned short* map_data, const unsigned int size_x,
		const unsigned int size_y, const float h_scale, float* h_map)
{
	unsigned int i, j, tmp, index;
 	
	index = 0;
	for (i = 0; i < size_y; i += 4)
	{
		for (j = 0; j < size_x; j += 4)
		{
			tmp = map_data[i*size_x+j];
			h_map[index] = tmp*h_scale;
			index++;
		}
	}
}

/*!
 * \ingroup 	display_utils
 * \brief 	Loads the terrain height map.
 *
 * The function reads the compressed terrain height map from file and
 * uncompressions the data.
 * \param 	file The file descriptor for the terrain height map data.
 * \param 	size_x The size of the terrain height map in x direction.
 * \param 	size_y The size of the terrain height map in y direction.
 * \retval	unsigned_short_array The uncompressed terrain.
 *  
 * \callgraph
 */
static __inline__ unsigned short *load_terrain_height_map(FILE *file, unsigned int size_x, unsigned int size_y)
{
	unsigned int size;
	unsigned long file_size, buffer_size;
	unsigned char *buffer;
	unsigned short *h_map;

	fread(&size, 1, sizeof(unsigned int), file);
	buffer_size = size_x*size_y*sizeof(unsigned short);
	size = SDL_SwapLE32(size);
	file_size = size;
	
	buffer = (unsigned char*)SIMD_MALLOC(size);
	h_map = (unsigned short*)SIMD_MALLOC(buffer_size);
	fread(buffer, 1, size, file);
	uncompress ((unsigned char*)h_map, &buffer_size, buffer, file_size);
	SIMD_FREE(buffer);
	
#ifdef	OSX
//	SwapLE16_mem(h_map, buffer_size/2);
#endif
	return h_map;
}

/*!
 * \ingroup 	display_utils
 * \brief 	Loads the extra texture coordinates.
 *
 * The function reads the compressed extra texture coordinates from file and
 * uncompress the data.
 * \param 	file The file descriptor for the  extra texture coordinates.
 * \param 	size_x The size of the extra texture coordinates in x direction.
 * \param 	size_y The size of the extra texture coordinates in y direction.
 *  
 * \callgraph
 */
static __inline__ void load_extra_texture_coordinates(FILE *file, unsigned int size_x, unsigned int size_y)
{
	unsigned int size;
	unsigned long file_size, buffer_size;
	unsigned char *buffer;

	fread(&size, 1, sizeof(unsigned int), file);
	buffer_size = (size_x*size_y*sizeof(TEXTCOORD2))/(NORMALS_PER_VERTEX_X*NORMALS_PER_VERTEX_Y);
	size = SDL_SwapLE32(size);
	file_size = size;
	
	buffer = (unsigned char*)SIMD_MALLOC(size);
	extra_texture_coordinates = (TEXTCOORD2*)SIMD_MALLOC(buffer_size);
	fread(buffer, 1, size, file);
	uncompress ((unsigned char*)extra_texture_coordinates, &buffer_size, buffer, file_size);
	SIMD_FREE(buffer);
	
#ifdef	OSX
//	SwapLE32_mem(extra_texture_coordinates, buffer_size/4);
#endif
}

#ifdef MAP_EDITOR2
/*!
 * \ingroup 	display_utils
 * \brief 	Saves the terrain height map.
 *
 * The function compress the terrain height map and saves it to a file.
 * \param 	file The file descriptor for the terrain height map data.
 *  
 * \callgraph
 */
static __inline__ void save_terrain_height_map(FILE *file)
{
	unsigned long buffer_size;
	unsigned char *buffer;
	unsigned int size;
	
	size = normal_map_size_x*normal_map_size_y*sizeof(unsigned short);
	buffer_size = (size*3)/4;
	buffer = (unsigned char*)SIMD_MALLOC(buffer_size);
	
#ifdef	OSX
//	SwapLE16_mem(h_map, buffer_size/2);
#endif
	
	compress2(buffer, &buffer_size, (unsigned char*)h_map, size, 9);
	size = buffer_size;
	size = SDL_SwapLE32(size);
	fwrite(&size, 1, sizeof(unsigned int), file);
	fwrite(buffer, 1, size, file);
	SIMD_FREE(buffer);
}

/*!
 * \ingroup 	display_utils
 * \brief 	Saves the extra texture coordinates.
 *
 * The function compress the extra texture coordinates and save them to a file.
 * \param 	file The file descriptor for the  extra texture coordinates.
 *
 * \callgraph
 */
static __inline__ void save_extra_texture_coordinates(FILE *file)
{
	unsigned long buffer_size;
	unsigned char *buffer;
	unsigned int size;
	
	size = (normal_map_size_x*normal_map_size_y*sizeof(TEXTCOORD2))/(NORMALS_PER_VERTEX_X*NORMALS_PER_VERTEX_Y);
	buffer_size = (size*3)/4;
	buffer = (unsigned char*)SIMD_MALLOC(buffer_size);
	
#ifdef	OSX
//	SwapLE32_mem(h_map, buffer_size/4);
#endif
	
	compress2(buffer, &buffer_size, (unsigned char*)extra_texture_coordinates, size, 9);
	size = buffer_size;
	size = SDL_SwapLE32(size);
	fwrite(&size, 1, sizeof(unsigned int), file);
	fwrite(buffer, 1, size, file);
	SIMD_FREE(buffer);
}

int save_terrain(FILE *file)
{
	int pos;

	pos = ftell(file);

	save_terrain_height_map(file);
	save_extra_texture_coordinates(file);
	
	pos = ftell(file) - pos;

	return pos;
}
#endif

void init_terrain(FILE *file, const unsigned int size_x, const unsigned int size_y)
{
	unsigned int size;
	float h_scale;
#ifndef MAP_EDITOR2
	unsigned short *h_map;
#endif
#ifdef	NEW_MAP_FORMAT
	
	h_scale = 0.25f;
	h_map = load_terrain_height_map(file, size_x, size_y);
	load_extra_texture_coordinates(file, size_x, size_y);
#else
	int i;
	
	h_scale = -0.001f;
	size = size_x*size_y*sizeof(unsigned short);
	h_map = (unsigned short*)SIMD_MALLOC(size);
	for (i = 0; i < size/2; i++)
		h_map[i] = 1;
	
	size = (size_x*size_y*sizeof(TEXTCOORD2))/(NORMALS_PER_VERTEX_X*NORMALS_PER_VERTEX_Y);
	extra_texture_coordinates = (TEXTCOORD2*)SIMD_MALLOC(size);
#endif
	init_normal_mapping(h_map, size_x, size_y, h_scale);
	hf_map = (float*)malloc((size_x/NORMALS_PER_VERTEX_X+1)*(size_y/NORMALS_PER_VERTEX_Y+1)*sizeof(float));

	calc_current_terrain(h_map, size_x, size_y, h_scale, hf_map);

#ifndef MAP_EDITOR2
	SIMD_FREE(h_map);
#endif
}

void free_terrain()
{
	free_normal_mapping();
	SIMD_FREE(hf_map);
#ifdef MAP_EDITOR2
	SIMD_FREE(h_map);
#endif
	SIMD_FREE(extra_texture_coordinates);
}
#endif
