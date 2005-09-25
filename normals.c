#ifdef	TERRAIN
/*!
 * \file
 * \ingroup 	display_utils
 * \brief 	Normal map calculation.
 */
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#ifdef MAP_EDITOR2
#include "../map_editor2/global.h"
#else
#include "global.h"
#endif
#include "vmath.h"
#include "shader.h"
#include "simd.h"

/*!
 * The array of texture ids for the normap texture maps.
 */
GLuint* normal_map_IDs;
/*!
 * The size of the normap map in x direction.
 */
unsigned int normal_map_size_x;
/*!
 * The size of the normap map in y direction.
 */
unsigned int normal_map_size_y;
/*!
 * Flag for normal mapping.
 */
unsigned int use_normal_mapping = 0;
/*!
 * Shader for normal mapping.
 */
GLhandleARB normal_mapping_shader;
/*!
 * Terrain vertex normals.
 */
VECTOR3* terrain_vertex_normals;

/*!
 * \ingroup 	display_utils
 * \brief	Calculates the normal textures of the normal map.
 * 
 * Calculates the normal textures of the normal map. 
 * \param 	normal_map The normal map.
 * \param 	size_x The size of the normal map in x direction.
 * \param 	size_y The size of the normal map in y direction.
 *  
 * \callgraph
 */
static inline void build_normal_texures(VECTOR4* normal_map, const unsigned int size_x, 
		const unsigned int size_y)
{
	unsigned int i, j, k, l, index, size, copy_size;
	unsigned int x_count, y_count;
	VECTOR4* normal_texture;
	
	normal_texture = (VECTOR4*)malloc(NORMAL_TEXTURE_MAX_X*NORMAL_TEXTURE_MAX_Y*sizeof(VECTOR4));

	x_count = (size_x+NORMAL_MAP_MAX_X-1)/NORMAL_MAP_MAX_X;
	y_count = (size_y+NORMAL_MAP_MAX_Y-1)/NORMAL_MAP_MAX_Y;
	
	for (i = 0; i < y_count; i++)
	{
		for (j = 0; j < x_count; j++)
		{		
			if ((i == 0) || (i == (y_count-1)) || (j == 0) || ((j == x_count-1)))
			{
				for (k = 0; k < NORMAL_TEXTURE_MAX_Y; k++)
				{
					for (l = 0; l < NORMAL_TEXTURE_MAX_X; l++)
					{
						index = min(max(j*NORMAL_MAP_MAX_X-NORMAL_MAP_PAD_X/2+l, 0), size_x-1);
						index += min(max(i*NORMAL_MAP_MAX_Y-NORMAL_MAP_PAD_Y/2+k, 0), size_y-1)*size_x;
						memcpy(normal_texture[k*NORMAL_TEXTURE_MAX_X+l], normal_map[index], 
								sizeof(VECTOR4));
					}
				}
			}
			else
			{
				index = j*NORMAL_MAP_MAX_X-NORMAL_MAP_PAD_X/2+(i*NORMAL_MAP_MAX_Y-NORMAL_MAP_PAD_Y/2)*size_x;
				for (k = 0; k < NORMAL_TEXTURE_MAX_Y; k++)
				{
					memcpy(normal_texture[k*NORMAL_TEXTURE_MAX_X], normal_map[index+k*size_x], 
							NORMAL_TEXTURE_MAX_X*sizeof(VECTOR4));
				}
			}

			index = i*x_count+j;
		
			glBindTexture(GL_TEXTURE_2D, normal_map_IDs[index]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB10, NORMAL_TEXTURE_MAX_X, NORMAL_TEXTURE_MAX_Y, 0, GL_RGBA,
				GL_FLOAT, normal_texture);			
		}
	}

	free(normal_texture);
}

/*!
 * \ingroup 	display_utils
 * \brief 	Returns array of texture ids for the normap texture maps.
 * 
 * Returns array of texture ids for the normap texture maps.
 * \param	count Number of texture IDs to create.
 * \retval	GLuint* The array of texture ids for the normap texture maps.
 *  
 * \callgraph
 */
static inline GLuint* create_normal_texture_IDs(const unsigned int count)
{
	GLuint* ret;
	ret = malloc(count*sizeof(GLuint));
	glGenTextures(count, ret);
	return ret;
}

#ifdef	USE_SSE
#include "normals_sse.h"
#endif

#define	USE_LOW_MEM
#ifdef	USE_LOW_MEM
#include "normals_low_mem.h"
#endif

/*!
 * \ingroup 	display_utils
 * \brief 	Calculates the current normal map of the terrain height map.
 *
 * Calculates the current normal map of the terrain height map.
 * \param 	h_map The terrain height map.
 * \param 	size_x The size of the terrain height map in x direction.
 * \param 	size_y The size of the terrain height map in y direction.
 * \param	h_scale The scale of the terrain height (z direction).
 *  
 * \callgraph
 */
static inline void calc_normal_map(unsigned short* h_map, unsigned int size_x, 
		unsigned int size_y, float h_scale)
{
	int i, j, n_index, n_row;
	float h0, h1, h2, h3;
	VECTOR3 v1, v2, v3, v4, v5, v6;
	VECTOR3* surface_normals;
	VECTOR4* normal_map;
	
	surface_normals = (VECTOR3*)malloc((size_x+1)*(size_y+1)*2*sizeof(VECTOR3));
	normal_map = (VECTOR4*)malloc(size_x*size_y*sizeof(VECTOR4));
	memset(surface_normals, 0, (size_x+1)*(size_y+1)*2*sizeof(VECTOR3));
	n_index = size_x*2+2;
	
	for (i = 0; i < size_y-1; i++)
	{
		n_index += 2;
		h1 = h_map[i*size_x]*h_scale;
		h3 = h_map[(i+1)*size_x]*h_scale;
		for (j = 1; j < size_x; j++)
		{
			h0 = h1;
			h1 = h_map[i*size_x+j]*h_scale;
			h2 = h3;
			h3 = h_map[(i+1)*size_x+j]*h_scale;
			VMake(v1, h0 - h2, h0 - h1, 1.0f);
			VMake(v2, h1 - h3, h2 - h3, 1.0f);
			Normalize(v1, v1);
			Normalize(v2, v2);
			VAssign(surface_normals[n_index++], v1);
			VAssign(surface_normals[n_index++], v2);
		}
		n_index += 2;
	}
	n_row = ((size_x+1)*2)-1;
	n_index = 1;
	for (i = 0; i < size_y; i++)
	{	
		VAssign(v3, surface_normals[n_index]);
		VAssign(v6, surface_normals[n_index+n_row]);
		for (j = 0; j < size_x; j++)
		{
			VAssign(v1, v3);
			VAssign(v2, surface_normals[n_index+1]);
			VAssign(v3, surface_normals[n_index+2]);
			VAssign(v4, v6);
			VAssign(v5, surface_normals[n_index+n_row+1]);
			VAssign(v6, surface_normals[n_index+n_row+2]);
			VAddEq(v1, v2);
			VAddEq(v1, v3);
			VAddEq(v1, v4);
			VAddEq(v1, v5);
			VAddEq(v1, v6);
			Normalize(v1, v1);
			VAssign4(normal_map[i*size_x+j], v1, 0.0f);
			n_index += 2;
		}
		n_index += 2;
	}

	free(surface_normals);
	build_normal_texures(normal_map, size_x, size_y);
	free(normal_map);
}

/*!
 * \ingroup 	display_utils
 * \brief 	Calculates the current terrain vertex normals of the terrain height map.
 *
 * Calculates the current terrain vertex normals of the terrain height map.
 * \param 	h_map The terrain height map.
 * \param 	size_x The size of the terrain height map in x direction.
 * \param 	size_y The size of the terrain height map in y direction.
 * \param	h_scale The scale of the terrain height (z direction).
 *  
 * \callgraph
 */
static inline void calc_terrain_vertex_normals(unsigned short* h_map, unsigned int size_x, 
		unsigned int size_y, float h_scale)
{
	int i, j, n_index, n_row;
	float h0, h1, h2, h3;
	VECTOR3 v1, v2, v3, v4, v5, v6;
	VECTOR3* surface_normals;
	
	surface_normals = (VECTOR3*)malloc((size_x+1)*(size_y+1)*2*sizeof(VECTOR3));
	terrain_vertex_normals = (VECTOR3*)malloc(size_x*size_y*sizeof(VECTOR3));
	memset(surface_normals, 0, (size_x+1)*(size_y+1)*2*sizeof(VECTOR3));
	n_index = size_x*2+2;
	
	for (i = 0; i < size_y-1; i++)
	{
		n_index += 2;
		h1 = h_map[i*size_x]*h_scale;
		h3 = h_map[(i+1)*size_x]*h_scale;
		for (j = 1; j < size_x; j++)
		{
			h0 = h1;
			h1 = h_map[i*size_x+j]*h_scale;
			h2 = h3;
			h3 = h_map[(i+1)*size_x+j]*h_scale;
			VMake(v1, h0 - h2, h0 - h1, 1.0f);
			VMake(v2, h1 - h3, h2 - h3, 1.0f);
			Normalize(v1, v1);
			Normalize(v2, v2);
			VAssign(surface_normals[n_index++], v1);
			VAssign(surface_normals[n_index++], v2);
		}
		n_index += 2;
	}
	n_row = ((size_x+1)*2)-1;
	n_index = 1;
	for (i = 0; i < size_y; i++)
	{	
		VAssign(v3, surface_normals[n_index]);
		VAssign(v6, surface_normals[n_index+n_row]);
		for (j = 0; j < size_x; j++)
		{
			VAssign(v1, v3);
			VAssign(v2, surface_normals[n_index+1]);
			VAssign(v3, surface_normals[n_index+2]);
			VAssign(v4, v6);
			VAssign(v5, surface_normals[n_index+n_row+1]);
			VAssign(v6, surface_normals[n_index+n_row+2]);
			VAddEq(v1, v2);
			VAddEq(v1, v3);
			VAddEq(v1, v4);
			VAddEq(v1, v5);
			VAddEq(v1, v6);
			Normalize(v1, v1);
			VAssign(terrain_vertex_normals[i*size_x+j], v1);
			n_index += 2;
		}
		n_index += 2;
	}
	free(surface_normals);
}

void init_normal_mapping(unsigned short *h_map, const unsigned int size_x, 
		const unsigned int size_y, const float h_scale)
{
#ifdef	USE_SHADER
	unsigned int normal_texture_count_x;
	unsigned int normal_texture_count_y;

	normal_texture_count_x = (size_x+NORMAL_MAP_MAX_X-1)/NORMAL_MAP_MAX_X;
	normal_texture_count_y = (size_y+NORMAL_MAP_MAX_Y-1)/NORMAL_MAP_MAX_Y;
	normal_map_IDs = create_normal_texture_IDs(normal_texture_count_x*normal_texture_count_y*16);

	if (use_low_mem) calc_normal_map_lo_mem(h_map, size_x, size_y, h_scale);
#ifdef		USE_SSE	
	else 
	{
		init_sse();
		if (use_sse) calc_normal_map_sse(h_map, size_x, size_y, h_scale);
		else calc_normal_map(h_map, size_x, size_y, h_scale);
	}
#else	
	else calc_normal_map(h_map, size_x, size_y, h_scale);	
#endif
	normal_mapping_shader = init_normal_mapping_shader();
#endif
	calc_terrain_vertex_normals(h_map, size_x, size_y, h_scale);
	normal_map_size_x = size_x;
	normal_map_size_x = size_y;
}

void free_normal_mapping()
{
#ifdef	USE_SHADER
	unsigned int normal_texture_count_x;
	unsigned int normal_texture_count_y;

	normal_texture_count_x = (normal_map_size_x+NORMAL_MAP_MAX_X-1)/NORMAL_MAP_MAX_X;
	normal_texture_count_y = (normal_map_size_y+NORMAL_MAP_MAX_Y-1)/NORMAL_MAP_MAX_Y;
	glDeleteTextures(normal_texture_count_x*normal_texture_count_y, normal_map_IDs);
	free(normal_map_IDs);
	free_shader(normal_mapping_shader);
#endif
	free(terrain_vertex_normals);
}
#endif
