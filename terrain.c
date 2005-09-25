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

/*!
 * The hf-map is a float array of map_tile_size_x*map_tile_size_y size 
 * storing the height for the vertexes of the terrain.
 */
float *hf_map;

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
static inline void calc_current_terrain(unsigned short* map_data, const unsigned int size_x,
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

void init_terrain(FILE *file, const unsigned int size_x, const unsigned int size_y)
{
	unsigned int size;
	unsigned short *h_map;
	float h_scale;
#ifdef	NEW_MAP_FORMAT
	unsigned long file_size, buffer_size;
	unsigned char *buffer;
	
	h_scale = 0.25f;

	fread(&size, 1, sizeof(unsigned int), file);
	buffer_size = size_x*size_y*sizeof(unsigned short);
	file_size = SDL_SwapLE32(size);

	buffer = (unsigned char*)SIMD_MALLOC(size);
	h_map = (unsigned short*)SIMD_MALLOC(buffer_size);
	fread(buffer, 1, size, file);
	uncompress ((unsigned char*)h_map, &buffer_size, buffer, file_size);
	SIMD_FREE(buffer);

#ifdef	OSX
//	swap_16_mem(h_map, buffer_size/2);
#endif
#else
	int i;
	
	h_scale = -0.001f;
	size = size_x*size_y*sizeof(unsigned short);
	h_map = (unsigned short*)malloc(size);
	for (i = 0; i < size/2; i++)
		h_map[i] = 1;
#endif
	init_normal_mapping(h_map, size_x, size_y, h_scale);
	hf_map = (float*)malloc((size_x/NORMALS_PER_VERTEX_X+1)*(size_y/NORMALS_PER_VERTEX_Y+1)*sizeof(float));

	calc_current_terrain(h_map, size_x, size_y, h_scale, hf_map);

	free(h_map);
}

void free_terrain()
{
	free_normal_mapping();
	free(hf_map);
}
#endif
