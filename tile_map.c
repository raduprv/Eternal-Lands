#include <math.h>
#include <stdlib.h>
#include "2d_objects.h"
#include "asc.h"
#include "bbox_tree.h"
#include "draw_scene.h"
#include "gl_init.h"
#include "init.h"
#include "load_gl_extensions.h"
#include "reflection.h"
#include "shadows.h"
#include "textures.h"
#include "tiles.h"
#ifdef CLUSTER_INSIDES_OLD
#include "cluster.h"
#endif
#ifdef FSAA
#include "fsaa/fsaa.h"
#endif /* FSAA */

#ifdef MAP_EDITOR2
img_struct map_tiles[256];
#endif

unsigned char *tile_map;
unsigned char *height_map;
int tile_map_size_x;
int tile_map_size_y;
int tile_list[256];
char dungeon=0;//no sun
float ambient_r=0;
float ambient_g=0;
float ambient_b=0;
char map_file_name[256];
GLfloat* terrain_tile_buffer = 0;
GLuint terrain_tile_buffer_object = 0;
int terrain_buffer_usage = 0;

void init_terrain_buffers(int terrain_buffer_size)
{
	terrain_tile_buffer = realloc(terrain_tile_buffer, terrain_buffer_size * 4 * 2 * sizeof(GLfloat));

	if (have_extension(arb_vertex_buffer_object))
	{
		if (terrain_tile_buffer_object == 0)
		{
			ELglGenBuffersARB(1, &terrain_tile_buffer_object);
		}
		else
		{
			ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, terrain_tile_buffer_object);
			ELglBufferDataARB(GL_ARRAY_BUFFER_ARB, 0, 0, GL_DYNAMIC_DRAW_ARB);
			ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		}
	}

}

static __inline__ void build_terrain_buffer()
{
	unsigned int i, j, l, x, y, start, stop;
	float x_scaled,y_scaled;
#ifdef CLUSTER_INSIDES_OLD
	short cluster = get_actor_cluster ();
	short tile_cluster;
#endif

	if (get_bbox_intersect_flag(main_bbox_tree, TYPE_TERRAIN, ide_changed))
	{
		clear_bbox_intersect_flag(main_bbox_tree, TYPE_TERRAIN, ide_changed);
	}
	else
	{
		return;
	}

	j = 0;

	get_intersect_start_stop(main_bbox_tree, TYPE_TERRAIN, &start, &stop);

	for (i = start; i < stop; i++)
	{
		l = get_intersect_item_ID(main_bbox_tree, i);
		x = get_terrain_x (l);
		y = get_terrain_y (l);

#ifdef CLUSTER_INSIDES_OLD
		tile_cluster = get_cluster (6*x, 6*y);
		if (tile_cluster && tile_cluster != cluster)
			continue;
#endif

		x_scaled = x * 3.0f;
		y_scaled = y * 3.0f;

		terrain_tile_buffer[j * 8 + 0] = x_scaled;
		terrain_tile_buffer[j * 8 + 1] = y_scaled + 3.0f;
		terrain_tile_buffer[j * 8 + 2] = x_scaled;
		terrain_tile_buffer[j * 8 + 3] = y_scaled;
		terrain_tile_buffer[j * 8 + 4] = x_scaled + 3.0f;
		terrain_tile_buffer[j * 8 + 5] = y_scaled;
		terrain_tile_buffer[j * 8 + 6] = x_scaled + 3.0f;
		terrain_tile_buffer[j * 8 + 7] = y_scaled + 3.0f;
		j++;
	}

	terrain_buffer_usage = j;

	if (have_extension(arb_vertex_buffer_object))
	{
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, terrain_tile_buffer_object);
		ELglBufferDataARB(GL_ARRAY_BUFFER_ARB, terrain_buffer_usage * 4 * 2 * sizeof(GLfloat),
			terrain_tile_buffer, GL_DYNAMIC_DRAW_ARB);
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	}
}

#ifdef	NEW_TEXTURES
void draw_quad_tiles(const unsigned int start, const unsigned int stop,
	unsigned int idx, const unsigned int zero_id)
{
	Uint32 i, l, size;
	Sint32 x, y;
	Uint32 cur_id, last_id;
#ifdef CLUSTER_INSIDES_OLD
	short cluster = get_actor_cluster ();
	short tile_cluster;
#endif

	size = 0;

	if (start < stop)
	{
		l = get_intersect_item_ID(main_bbox_tree, start);
		x = get_terrain_x(l);
		y = get_terrain_y(l);

		if (tile_map[y * tile_map_size_x + x] == 0)
		{
			cur_id = zero_id;
		}
		else
		{
			cur_id = tile_list[tile_map[y*tile_map_size_x+x]];
		}

		bind_texture(cur_id);
		last_id = cur_id;
	}
	else
	{
		return;
	}

	for (i = start; i < stop; i++)
	{
		l = get_intersect_item_ID(main_bbox_tree, i);
		x = get_terrain_x(l);
		y = get_terrain_y(l);

#ifdef CLUSTER_INSIDES_OLD
		tile_cluster = get_cluster (6*x, 6*y);
		if (tile_cluster && tile_cluster != cluster)
			continue;
#endif

		if (tile_map[y * tile_map_size_x + x] == 0)
		{
			cur_id = zero_id;
		}
		else
		{
			cur_id = tile_list[tile_map[y*tile_map_size_x+x]];
		}

		if (cur_id != last_id)
		{
			glDrawArrays(GL_QUADS, idx * 4, size * 4);
			bind_texture(cur_id);
			last_id = cur_id;
			idx += size;
			size = 0;
		}
		size++;
	}
	glDrawArrays(GL_QUADS, idx * 4, size * 4);
}
#else	/* NEW_TEXTURES */
void draw_terrain_quad_tiles(unsigned int start, unsigned int stop)
{
	unsigned int i, l, size, idx;
	int x, y, cur_texture;
#ifdef CLUSTER_INSIDES_OLD
	short cluster = get_actor_cluster ();
	short tile_cluster;
#endif

	idx = 0;
	size = 0;

	for (i = start; i < stop; i++)
	{
		l = get_intersect_item_ID(main_bbox_tree, i);
		x = get_terrain_x(l);
		y = get_terrain_y(l);

#ifdef CLUSTER_INSIDES_OLD
		tile_cluster = get_cluster (6*x, 6*y);
		if (tile_cluster && tile_cluster != cluster)
			continue;
#endif

		cur_texture = get_texture_id(tile_list[tile_map[y*tile_map_size_x+x]]);
		if (cur_texture != last_texture)
		{
			glDrawArrays(GL_QUADS, idx * 4, size * 4);
			bind_texture_id(cur_texture);
			cur_texture = last_texture;
			idx += size;
			size = 0;
		}
		size++;
	}
	glDrawArrays(GL_QUADS, idx * 4, size * 4);
}
#endif	/* NEW_TEXTURES */

static __inline__ void setup_terrain_clous_texgen()
{
	GLfloat plane[4];

	ELglActiveTextureARB(detail_unit);

	glEnable(GL_TEXTURE_GEN_S);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	plane[0] = 1.0f / texture_scale;
	plane[1] = 0.0f;
	plane[2] = 0.0f;
	plane[3] = clouds_movement_u;
	glTexGenfv(GL_S, GL_OBJECT_PLANE, plane);

	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	plane[0] = 0.0f;
	plane[1] = 1.0f / texture_scale;
	plane[2] = 0.0f;
	plane[3] = clouds_movement_v;
	glTexGenfv(GL_T, GL_OBJECT_PLANE, plane);

	ELglActiveTextureARB(base_unit);

#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

static __inline__ void setup_terrain_texgen()
{
	GLfloat plane[4];

	glEnable(GL_TEXTURE_GEN_S);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	plane[0] = 1.0f / 3.0f;
	plane[1] = 0.0f;
	plane[2] = 0.0f;
	plane[3] = 0.0f;
	glTexGenfv(GL_S, GL_OBJECT_PLANE, plane);

	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	plane[0] = 0.0f;
	plane[1] = 1.0f / 3.0f;
	plane[2] = 0.0f;
	plane[3] = 0.0f;
	glTexGenfv(GL_T, GL_OBJECT_PLANE, plane);

#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

static __inline__ void disable_terrain_clous_texgen()
{
	ELglActiveTextureARB(detail_unit);

	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

	ELglActiveTextureARB(base_unit);

#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

static __inline__ void disable_terrain_texgen()
{
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void draw_tile_map()
{
	unsigned int start, stop;

	glEnable(GL_CULL_FACE);

	build_terrain_buffer();

	glEnable(GL_TEXTURE_2D);
	if (!dungeon && clouds_shadows)
	{
		//bind the detail texture
		ELglActiveTextureARB(detail_unit);
		glEnable(GL_TEXTURE_2D);
#ifdef	NEW_TEXTURES
		bind_texture_unbuffered(ground_detail_text);
#else	/* NEW_TEXTURES */
		glBindTexture(GL_TEXTURE_2D, get_texture_id(ground_detail_text));
#endif	/* NEW_TEXTURES */
		ELglActiveTextureARB(base_unit);
		glEnable(GL_TEXTURE_2D);
	}

	if (!dungeon && (clouds_shadows || use_shadow_mapping))
	{
		setup_terrain_clous_texgen();
	}
	setup_terrain_texgen();

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -0.001f);

	if (use_vertex_buffers)
	{
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, terrain_tile_buffer_object);
		glInterleavedArrays(GL_V2F, 0, 0);
	}
	else
	{
		glInterleavedArrays(GL_V2F, 0, terrain_tile_buffer);
	}

	get_intersect_start_stop(main_bbox_tree, TYPE_TERRAIN, &start, &stop);
#ifdef	FSAA
	if (fsaa > 1)
	{
		glEnable(GL_MULTISAMPLE);
	}
#endif	/* FSAA */
#ifdef	NEW_TEXTURES
	draw_quad_tiles(start, stop, 0, tile_list[0]);
#else	/* NEW_TEXTURES */
	draw_terrain_quad_tiles(start, stop);
#endif	/* NEW_TEXTURES */
#ifdef	FSAA
	if (fsaa > 1)
	{
		glDisable(GL_MULTISAMPLE);
	}
#endif	/* FSAA */

	glDisable(GL_CULL_FACE);

	if (use_vertex_buffers)
	{
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	}

	glDisableClientState(GL_VERTEX_ARRAY);

	glPopMatrix();

	if (!dungeon && (clouds_shadows || use_shadow_mapping))
	{
		disable_terrain_clous_texgen();
	}
	disable_terrain_texgen();

	if (!dungeon && clouds_shadows)
	{
		//disable the second texture unit
		ELglActiveTextureARB(detail_unit);
		glDisable(GL_TEXTURE_2D);
		ELglActiveTextureARB(base_unit);
	}
	glEnable(GL_TEXTURE_2D);

#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

//load only the tiles that are on the map
void load_map_tiles()
{
	int i;
#ifdef MAP_EDITOR2
	char str[200];
	
	for(i=0;i<255;i++){
#ifdef OLD_MISC_OBJ_DIR
		safe_snprintf(str,sizeof(str), "./tiles/tile%i.bmp",i);
#else
		safe_snprintf(str,sizeof(str), "./3dobjects/tile%i.dds",i);
#endif
		if(IS_WATER_TILE(i) && IS_REFLECTING(i))
			tile_list[i]=load_texture_cache(str,70);
		else 
			tile_list[i]=load_texture_cache(str,255);

		if(get_texture_id(tile_list[i]) == 0)
			tile_list[i]=0;
	}
#else
#ifdef	NEW_TEXTURES
	char str[128];

	for (i = 0; i < 255; i++)
	{
		safe_snprintf(str, sizeof(str), "./3dobjects/tile%i", i);
		tile_list[i] = load_texture_cached(str, tt_mesh);
	}
#else	/* NEW_TEXTURES */
	int cur_tile;
	char str[80];
	for(i=0;i<tile_map_size_x*tile_map_size_y;i++)
		{
			cur_tile=tile_map[i];
			if(!cur_tile && dungeon)cur_tile=231;
			//check to see if we already have the current tile loaded
			if(!tile_list[cur_tile] && cur_tile!=255)//if it is 255, it's a null tile, don't load it
				{
					//tile not loaded, so load it
#ifdef OLD_MISC_OBJ_DIR
					safe_snprintf(str, sizeof(str), "./tiles/tile%i.bmp",cur_tile);
#else
					safe_snprintf(str, sizeof(str), "./3dobjects/tile%i.dds",cur_tile);
#endif
					if(IS_WATER_TILE(cur_tile) && IS_REFLECTING(cur_tile))
						tile_list[cur_tile]=load_texture_cache(str,70);
					else
						tile_list[cur_tile]=load_texture_cache(str,255);
			
				}
		}
#endif	/* NEW_TEXTURES */
#endif
}

#ifdef NEW_SOUND
int get_tile_type(int x, int y)
{
	int tile_num = (y / 6) * tile_map_size_x + (x / 6);
//	printf("X: %d, Y: %d, tile_num: %d\n", x, y, tile_num);
	if (x >= 0 && (x / 6) <= tile_map_size_x && y >= 0 && (y / 6) <= tile_map_size_y)
		return tile_map[tile_num];
	else
		return -1;
}
#endif // NEW_SOUND

int get_tile_walkable(const int x, const int y)
{
	if (!get_tile_valid(x, y))
	{
		return 0;
	}

	return (height_map[x * tile_map_size_x * 6 + y] & 0x3F) != 0;
}

int get_tile_valid(const int x, const int y)
{
	return (x >= 0) && (x < (tile_map_size_x * 6)) &&
		(y >= 0) && (y < (tile_map_size_y * 6));
}

float get_tile_height(const float x, const float y)
{
	float z;
	int pos_x, pos_y, i, j, tmp, count;

	if (!get_tile_valid(x, y))
	{
		return 0.0f;
	}

	pos_x = x;
	pos_y = y;

	tmp = height_map[pos_y * tile_map_size_x * 6 + pos_x];

	if (tmp != 0)
	{
		return tmp * 0.2f - 2.2f;
	}

	tmp = 0;
	count = 0;

	for (j = pos_y - 1; j <= (pos_y + 1); ++j)
	{
		for (i = pos_x - 1; i <= (pos_x + 1); ++i)
		{
			if (get_tile_walkable(i, j))
			{
				tmp += height_map[j * tile_map_size_x * 6 + i];
				count++;
			}
		}
	}

	z = tmp;

	if (count > 1)
	{
		z /= count;
	}

	return z * 0.2f - 2.2f;
}

