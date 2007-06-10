#include <math.h>
#include <stdlib.h>
#ifdef MAP_EDITOR2
#include "../map_editor2/global.h"
#else
#include "global.h"
#endif

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

void draw_tile_map()
{
	unsigned int i, l, start, stop;
	int x,y;
	float x_scaled,y_scaled;
	int cur_texture;

#if 0
	{
		int num = -1;
		float fv[4];
			
#ifdef EYE_CANDY
		for (i = 0; i < 4; i++)
#else
		for (i = 0; i < 7; i++)
#endif
			if (glIsEnabled(GL_LIGHT0+i)) num++;

		printf("\nNum:\t%d\n", num);
		printf("\nshow_lights:\t%d\n", show_lights);
		printf("\nnum_lights:\t%d\n", num_lights);

		for (i = 0; i < 8; i++)
		{
			if (glIsEnabled(GL_LIGHT0+i))
			{
				printf("\nLight%d:\n", i);
				glGetLightfv(GL_LIGHT0+i, GL_POSITION, fv);
				printf("\tPosition:\t<%f,\t%f,\t%f,\t%f>\n", fv[0], fv[1], fv[2], fv[3]);
				glGetLightfv(GL_LIGHT0+i, GL_DIFFUSE, fv);
				printf("\tDiffuse:\t<%f,\t%f,\t%f,\t%f>\n", fv[0], fv[1], fv[2], fv[3]);
				glGetLightfv(GL_LIGHT0+i, GL_SPECULAR, fv);
				printf("\tSpecular:\t<%f,\t%f,\t%f,\t%f>\n", fv[0], fv[1], fv[2], fv[3]);
				glGetLightfv(GL_LIGHT0+i, GL_SPOT_DIRECTION, fv);
				printf("\tSpot direction:\t<%f,\t%f,\t%f,\t%f>\n", fv[0], fv[1], fv[2], fv[3]);
				glGetLightfv(GL_LIGHT0+i, GL_AMBIENT, fv);
				printf("\tAmbient:\t<%f,\t%f,\t%f,\t%f>\n", fv[0], fv[1], fv[2], fv[3]);
				glGetLightfv(GL_LIGHT0+i, GL_SPECULAR, fv);
				printf("\tSpecular:\t<%f,\t%f,\t%f,\t%f>\n", fv[0], fv[1], fv[2], fv[3]);
				glGetLightfv(GL_LIGHT0+i, GL_SPOT_CUTOFF, fv);
				printf("\tSpot cutoff:\t%f\n", fv[0]);
				glGetLightfv(GL_LIGHT0+i, GL_CONSTANT_ATTENUATION, fv);
				printf("\tConstant attentuation:\t%f\n", fv[0]);
				glGetLightfv(GL_LIGHT0+i, GL_LINEAR_ATTENUATION, fv);
				printf("\tLinear attentuation:\t%f\n", fv[0]);
				glGetLightfv(GL_LIGHT0+i, GL_QUADRATIC_ATTENUATION, fv);
				printf("\tQuadratic attentuation:\t%f\n", fv[0]);
			}
		}
	}
#endif // Disabled debugging info

	if (!dungeon && clouds_shadows)
		{
			//bind the detail texture
			ELglActiveTextureARB(detail_unit);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, get_texture_id(ground_detail_text));
			ELglActiveTextureARB(base_unit);
			glEnable(GL_TEXTURE_2D);
		}

#ifdef NEW_LIGHTING
	if (use_new_lighting)
		reset_material();
#endif

	if (dungeon || (!clouds_shadows && !use_shadow_mapping))
		{
			glBegin(GL_QUADS);
			get_intersect_start_stop(main_bbox_tree, TYPE_TERRAIN, &start, &stop);
			for (i = start; i < stop; i++)
			{
				l = get_intersect_item_ID(main_bbox_tree, i);
				x = get_terrain_x(l);
				y = get_terrain_y(l);
				y_scaled = y*3.0f;
				x_scaled = x*3.0f;
							cur_texture=get_texture_id(tile_list[tile_map[y*tile_map_size_x+x]]);
							if(last_texture!=cur_texture)
								{
									glEnd();
									bind_texture_id(cur_texture);
									glBegin(GL_QUADS);
								}

 							glTexCoord2f(0, 1.0f);
			 				glVertex3f(x_scaled,y_scaled+3, -0.001f);
							glTexCoord2f(0, 0);
							glVertex3f(x_scaled,y_scaled, -0.001f);
							glTexCoord2f(1.0f, 0);
							glVertex3f(x_scaled+3, y_scaled,-0.001f);
							glTexCoord2f(1.0f, 1.0f);
							glVertex3f(x_scaled+3, y_scaled+3,-0.001f);
				}
			glEnd();
		}
	else//we draw the ground details
		{

			glBegin(GL_QUADS);
			get_intersect_start_stop(main_bbox_tree, TYPE_TERRAIN, &start, &stop);
			for (i = start; i < stop; i++)
			{
				l = get_intersect_item_ID(main_bbox_tree, i);
				x = get_terrain_x(l);
				y = get_terrain_y(l);
				y_scaled = y*3.0f;
				x_scaled = x*3.0f;
							cur_texture=get_texture_id(tile_list[tile_map[y*tile_map_size_x+x]]);
							if(last_texture!=cur_texture)
								{
									glEnd();
									bind_texture_id(cur_texture);
									glBegin(GL_QUADS);
								}
							//draw our normal tile
 							ELglMultiTexCoord2fARB(base_unit,0, 1.0f);
 							ELglMultiTexCoord2fARB(detail_unit,x_scaled/texture_scale+clouds_movement_u, (y_scaled+3.0)/texture_scale+clouds_movement_v);
			 				glVertex3f(x_scaled,y_scaled+3, -0.001f);

							ELglMultiTexCoord2fARB(base_unit,0, 0);
							ELglMultiTexCoord2fARB(detail_unit,x_scaled/texture_scale+clouds_movement_u, y_scaled/texture_scale+clouds_movement_v);
							glVertex3f(x_scaled,y_scaled, -0.001f);

							ELglMultiTexCoord2fARB(base_unit,1.0f, 0);
							ELglMultiTexCoord2fARB(detail_unit,(x_scaled+3.0f)/texture_scale+clouds_movement_u, y_scaled/texture_scale+clouds_movement_v);
							glVertex3f(x_scaled+3, y_scaled,-0.001f);

							ELglMultiTexCoord2fARB(base_unit,1.0f, 1.0f);
							ELglMultiTexCoord2fARB(detail_unit,(x_scaled+3.0)/texture_scale+clouds_movement_u, (y_scaled+3.0)/texture_scale+clouds_movement_v);
							glVertex3f(x_scaled+3, y_scaled+3,-0.001f);
				}
			glEnd();

		}
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
		safe_snprintf(str,sizeof(str), "./tiles/tile%i.bmp",i);
		if(IS_WATER_TILE(i) && IS_REFLECTING(i))
			tile_list[i]=load_texture_cache(str,70);
		else 	tile_list[i]=load_texture_cache(str,255);

		if(get_texture_id(tile_list[i]) == 0)
			tile_list[i]=0;
	}
#else
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
					safe_snprintf(str, sizeof(str), "./tiles/tile%i.bmp",cur_tile);
					if(IS_WATER_TILE(cur_tile) && IS_REFLECTING(cur_tile))
					  tile_list[cur_tile]=load_texture_cache(str,70);
					else tile_list[cur_tile]=load_texture_cache(str,255);
			
				}
		}
#endif
}


