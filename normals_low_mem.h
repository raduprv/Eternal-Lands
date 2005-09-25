#ifdef	TERRAIN
/*!
 * \file
 * \ingroup 	display_utils
 * \brief 	Normal map calculation with little memory usage.
 */
#ifndef NORMALS_LOW_MEM_H
#define NORMALS_LOW_MEM_H

/*!
 * \ingroup 	display_utils
 * \brief	Calculates the normal textures of the normal map with little memory usage.
 * 
 * Calculates the normal textures of the normal map with little memory usage.
 * \param 	normal_map The normal map.
 * \param 	size_x The size of the normal map in x direction.
 * \param 	size_y The size of the normal map in y direction.
 *  
 * \callgraph
 */
static inline void build_normal_texures_lo_mem(SHORT_VEC3* normal_map, const unsigned int size_x, 
		const unsigned int size_y)
{
	unsigned int i, j, k, l, index;
	unsigned int x_count, y_count;
	SHORT_VEC3* normal_texture;
		
	normal_texture = (SHORT_VEC3*)malloc(NORMAL_TEXTURE_MAX_X*NORMAL_TEXTURE_MAX_Y*sizeof(SHORT_VEC3));

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
								sizeof(SHORT_VEC3));
					}
				}
			}
			else
			{
				index = j*NORMAL_MAP_MAX_X-NORMAL_MAP_PAD_X/2+(i*NORMAL_MAP_MAX_Y-NORMAL_MAP_PAD_Y/2)*size_x;
				for (k = 0; k < NORMAL_TEXTURE_MAX_Y; k++)
				{
					memcpy(normal_texture[k*NORMAL_TEXTURE_MAX_X], normal_map[index+k*size_x], 
							NORMAL_TEXTURE_MAX_X*sizeof(SHORT_VEC3));
				}
			}
			
			index = i*x_count+j;
			
			glBindTexture(GL_TEXTURE_2D, normal_map_IDs[index]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA12, NORMAL_TEXTURE_MAX_X, NORMAL_TEXTURE_MAX_Y, 0, GL_RGB,
				GL_SHORT, normal_texture);
		}
	}

	free(normal_texture);
}

/*!
 * \ingroup 	display_utils
 * \brief	Calculates the normal of the triangle number index.
 * 
 * Calculates the normal of the triangle number index of the terrain height map. 
 * \param	v1 The return value.
 * \param	index The index of the triangle.
 * \param	size_x The size of the normal map in x direction.
 * \param	size_y The size of the normal map in y direction.
 * \param	h_map The terrain height map.
 * \param	h_scale The scale of the terrain height (z direction).
 *  
 * \callgraph
 */
static inline void calc_triangle_normal(VECTOR4 v1, const unsigned int index, 
		const unsigned int size_x, const unsigned int size_y, 
		unsigned short* h_map, const float h_scale)
{
	float h0, h1, h2, h3;
	int x, y, down;

	x = index%((size_x+1)*2);
	y = index/((size_x+1)*2);

	down = x%2;

	x /= 2;
	
	if ((x == 0) || (y == 0) || (x == size_x) || (y == size_y))
	{
		v1[X] = 0.0f;
		v1[Y] = 0.0f;
		v1[Z] = 0.0f;
	}
	else
	{
		x -= 1;
		y -= 1;

		h1 = h_map[y*size_x+x+1]*h_scale;
		h2 = h_map[(y+1)*size_x+x]*h_scale;
				
		if (down == 0)
		{
			h0 = h_map[y*size_x+x]*h_scale;
			VMake(v1, h0 - h2, h0 - h1, 1.0f);
		}
		else
		{
			h3 = h_map[(y+1)*size_x+x+1]*h_scale;
			VMake(v1, h1 - h3, h2 - h3, 1.0f);
		}
		Normalize(v1, v1);
	}
}

/*!
 * \ingroup 	display_utils
 * \brief 	Calculates the current normal map of the terrain height map little memory usage.
 *
 * Calculates the current normal map of the terrain height map with little memory usage.
 * \param 	h_map The terrain height map.
 * \param 	size_x The size of the terrain height map in x direction.
 * \param 	size_y The size of the terrain height map in y direction.
 * \param	h_scale The scale of the terrain height (z direction).
 *  
 * \callgraph
 */
static inline void calc_normal_map_lo_mem(unsigned short* h_map, const unsigned int size_x, 
		const unsigned int size_y, const float h_scale)
{
	int i, j, n_index, n_row;
	VECTOR3 v1, v2, v3, v4, v5, v6;
	SHORT_VEC3* normal_map;
	
	normal_map = (SHORT_VEC3*)malloc(size_x*size_y*sizeof(SHORT_VEC3));

	n_row = ((size_x+1)*2)-1;
	n_index = 1;
	
	for (i = 0; i < size_y; i++)
	{	
		calc_triangle_normal(v3, n_index, size_x, size_y, h_map, h_scale);
		calc_triangle_normal(v6, n_index+n_row, size_x, size_y, h_map, h_scale);
		for (j = 0; j < size_x; j++)
		{
			VAssign(v1, v3);
			calc_triangle_normal(v2, n_index+1, size_x, size_y, h_map, h_scale);
			calc_triangle_normal(v3, n_index+2, size_x, size_y, h_map, h_scale);
			VAssign(v4, v6);
			calc_triangle_normal(v5, n_index+n_row+1, size_x, size_y, h_map, h_scale);
			calc_triangle_normal(v6, n_index+n_row+2, size_x, size_y, h_map, h_scale);
			VAddEq(v1, v2);
			VAddEq(v1, v3);
			VAddEq(v1, v4);
			VAddEq(v1, v5);
			VAddEq(v1, v6);
			Normalize(v1, v1);
			VAssignS3(normal_map[i*size_x+j], v1);
			n_index += 2;
		}
		n_index += 2;
	}

	build_normal_texures_lo_mem(normal_map, size_x, size_y);
	free(normal_map);
}
#endif
#endif
