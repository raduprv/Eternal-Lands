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

#ifdef	TERRAIN
unsigned int etr = 0;

static __inline__ void draw_tile_map_normal_mapping(const unsigned int x, const unsigned int y)
{
	int i, j;
	float x_scaled, y_scaled;
	float u, v;

	y_scaled = y;
	y_scaled *= UNITS_PER_TILE_Y;
	v = 0.0f;

	for (i = 0; i < VERTEXES_PER_TILE_Y; i++)
	{
		x_scaled = x;
		x_scaled *= UNITS_PER_TILE_X;
		u = 0.0f;

		glBegin(GL_TRIANGLE_STRIP);
		for (j = 0; j < VERTEXES_PER_TILE_X+1; j++)
		{
			ELglMultiTexCoord2fARB(base_unit, u, v);
			ELglMultiTexCoord2fARB(detail_unit, x_scaled/texture_scale+clouds_movement_u, y_scaled/texture_scale+clouds_movement_v);
			ELglMultiTexCoord2fARB(normal_map_unit, get_texture_coord_u(x, j), get_texture_coord_v(y, i));
			ELglMultiTexCoord2fARB(extra_unit, extra_texture_coordinates[get_extra_texcoord(x, y, j, i)][0],
					extra_texture_coordinates[get_extra_texcoord(x, y, j, i)][1]);
			glVertex3f(x_scaled, y_scaled, get_vertex_height(x, y, j, i));

			ELglMultiTexCoord2fARB(base_unit, u, v+1.0f/VERTEXES_PER_TILE_Y);
			ELglMultiTexCoord2fARB(detail_unit, x_scaled/texture_scale+clouds_movement_u,
					(y_scaled+UNITS_PER_VERTEX_Y)/texture_scale+clouds_movement_v);
			ELglMultiTexCoord2fARB(normal_map_unit, get_texture_coord_u(x, j), get_texture_coord_v(y, i+1));
			ELglMultiTexCoord2fARB(extra_unit, extra_texture_coordinates[get_extra_texcoord(x, y, j, i+1)][0],
					extra_texture_coordinates[get_extra_texcoord(x, y, j, i+1)][1]);
			glVertex3f(x_scaled, y_scaled+UNITS_PER_VERTEX_Y, get_vertex_height(x, y, j, i+1));
			x_scaled += UNITS_PER_VERTEX_X;
			u += 1.0f/VERTEXES_PER_TILE_X;
		}
		glEnd();
		y_scaled += UNITS_PER_VERTEX_Y;
		v += 1.0f/VERTEXES_PER_TILE_Y;	
	}
}

static __inline__ void draw_tile_map_multitexture(const unsigned int x, const unsigned int y)
{
	int i, j;
	float x_scaled, y_scaled;
	float u, v;

	y_scaled = y;
	y_scaled *= UNITS_PER_TILE_Y;
	v = 0.0f;

	for (i = 0; i < VERTEXES_PER_TILE_Y; i++)
	{
		x_scaled = x;
		x_scaled *= UNITS_PER_TILE_X;
		u = 0.0f;

		glBegin(GL_TRIANGLE_STRIP);
		for (j = 0; j < VERTEXES_PER_TILE_X+1; j++)
		{
			ELglMultiTexCoord2fARB(base_unit, u, v);
			ELglMultiTexCoord2fARB(detail_unit, x_scaled/texture_scale+clouds_movement_u, y_scaled/texture_scale+clouds_movement_v);
			ELglMultiTexCoord2fARB(extra_unit, extra_texture_coordinates[get_extra_texcoord(x, y, j, i)][0],
					extra_texture_coordinates[get_extra_texcoord(x, y, j, i)][1]);
			glNormal3fv(terrain_vertex_normals[get_vertex_normal(x, y, j, i)]);
			glVertex3f(x_scaled, y_scaled, get_vertex_height(x, y, j, i));

			ELglMultiTexCoord2fARB(base_unit, u, v+1.0f/VERTEXES_PER_TILE_Y);
			ELglMultiTexCoord2fARB(detail_unit, x_scaled/texture_scale+clouds_movement_u,
					(y_scaled+UNITS_PER_VERTEX_Y)/texture_scale+clouds_movement_v);
			ELglMultiTexCoord2fARB(extra_unit, extra_texture_coordinates[get_extra_texcoord(x, y, j, i)][0],
					extra_texture_coordinates[get_extra_texcoord(x, y, j, i)][1]);
			glNormal3fv(terrain_vertex_normals[get_vertex_normal(x, y, j, i+1)]);
			glVertex3f(x_scaled, y_scaled+UNITS_PER_VERTEX_Y, get_vertex_height(x, y, j, i+1));
			x_scaled += UNITS_PER_VERTEX_X;
			u += 1.0f/VERTEXES_PER_TILE_X;
		}
		glEnd();
		y_scaled += UNITS_PER_VERTEX_Y;
		v += 1.0f/VERTEXES_PER_TILE_Y;	
	}
}

static __inline__ void draw_tile_map_singletexture(const unsigned int x, const unsigned int y)
{
	int i, j;
	float x_scaled, y_scaled;
	float u, v;

	y_scaled = y;
	y_scaled *= UNITS_PER_TILE_Y;
	v = 0.0f;

	for (i = 0; i < VERTEXES_PER_TILE_Y; i++)
	{
		x_scaled = x;
		x_scaled *= UNITS_PER_TILE_X;
		u = 0.0f;

		glBegin(GL_TRIANGLE_STRIP);
		for (j = 0; j < VERTEXES_PER_TILE_X+1; j++)
		{
			glTexCoord2f(u, v);
			glNormal3fv(terrain_vertex_normals[get_vertex_normal(x, y, j, i+1)]);
			glVertex3f(x_scaled, y_scaled, get_vertex_height(x, y, j, i));

			glTexCoord2f(u, v+1.0f/VERTEXES_PER_TILE_Y);
			glNormal3fv(terrain_vertex_normals[get_vertex_normal(x, y, j, i+1)]);
			glVertex3f(x_scaled, y_scaled+UNITS_PER_VERTEX_Y, get_vertex_height(x, y, j, i+1));
			x_scaled += UNITS_PER_VERTEX_X;
			u += 1.0f/VERTEXES_PER_TILE_X;
		}
		glEnd();
		y_scaled += UNITS_PER_VERTEX_Y;
		v += 1.0f/VERTEXES_PER_TILE_Y;	
	}
}

void draw_tile_map()
{
	int x_start,x_end,y_start,y_end;
	int x,y;
	float x_scaled,y_scaled;
	unsigned int cur_texture, cur_normal_map, last_normal_map;

	last_normal_map = 0;

	//get only the tiles around the camera
	//we have the axes inverted, btw the go from 0 to -255
	if(cx<0)x=(cx*-1)/3;
	else x=cx/3;
	if(cy<0)y=(cy*-1)/3;
	else y=cy/3;
	
	x_start = (int)x - 8;
	y_start = (int)y - 8;
	x_end   = (int)x + 8;
	y_end   = (int)y + 8;

	if(x_start < 0) x_start = 0;
	if(x_end >= tile_map_size_x) x_end = tile_map_size_x - 1;
	if(y_start < 0) y_start = 0;
	if(y_end >= tile_map_size_y) y_end = tile_map_size_y - 1;
	
	if(have_multitexture && !dungeon && (clouds_shadows || use_normal_mapping))
	{
		if (clouds_shadows)
		{
			//bind the detail texture
			ELglActiveTextureARB(detail_unit);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, get_texture_id(ground_detail_text));
		}
		if (use_normal_mapping)
		{
			int num;
			int i;

			//bind the normal map texture
			last_normal_map = get_normal_texture_ID(x_start, y_start);
			ELglActiveTextureARB(normal_map_unit);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, last_normal_map);
			//enable shader
			ELglUseProgramObjectARB(normal_mapping_shader);
			
			ELglUniform1iARB(ELglGetUniformLocationARB(normal_mapping_shader, "base_texture"), base_unit-GL_TEXTURE0_ARB);
			ELglUniform1iARB(ELglGetUniformLocationARB(normal_mapping_shader, "detail_texture"), detail_unit-GL_TEXTURE0_ARB);
			ELglUniform1iARB(ELglGetUniformLocationARB(normal_mapping_shader, "normal_texture"), normal_map_unit-GL_TEXTURE0_ARB);
			ELglUniform1iARB(ELglGetUniformLocationARB(normal_mapping_shader, "shadow_texture"), shadow_unit-GL_TEXTURE0_ARB);

			num = -1;
			
			for (i = 0; i < 7; i++)
				if (glIsEnabled(GL_LIGHT0+i)) num++;
			
			if (etr == 0)
			{
				float fv[4];
				
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
				etr = 1;
			}			
			ELglUniform1iARB(ELglGetUniformLocationARB(normal_mapping_shader, "NumEnabledLights"), num);//show_lights);
			ELglUniform1iARB(ELglGetUniformLocationARB(normal_mapping_shader, "show_Lights"), show_lights);
			/*
			ELglUniform1iARB(ELglGetUniformLocationARB(normal_mapping_shader, "base_texture_ID"), base_unit-GL_TEXTURE0_ARB);
			ELglUniform1iARB(ELglGetUniformLocationARB(normal_mapping_shader, "detail_texture_ID"), detail_unit-GL_TEXTURE0_ARB);
			ELglUniform1iARB(ELglGetUniformLocationARB(normal_mapping_shader, "normal_texture_ID"), normal_map_unit-GL_TEXTURE0_ARB);
			*/
		}
		ELglActiveTextureARB(base_unit);
		glEnable(GL_TEXTURE_2D);
	}
	else etr = 0;
	
	if(!have_multitexture || dungeon || (!clouds_shadows && !use_shadow_mapping && !use_normal_mapping))
	{
		for(y=y_start;y<=y_end;y++)
		{
			y_scaled=y*3.0f;
			for(x=x_start;x<=x_end;x++)
			{
				x_scaled=x*3.0f;
				if(IS_WATER_TILE(tile_map[y*tile_map_size_x+x]))continue;//lake, skip
				if(tile_map[y*tile_map_size_x+x]==255)continue;//null, skip
				if(!check_tile_in_frustrum(x_scaled,y_scaled))continue;//outside of the frustrum
				cur_texture=get_texture_id(tile_list[tile_map[y*tile_map_size_x+x]]);
				if(last_texture!=cur_texture)
				{
					bind_texture_id(cur_texture);
				}
				draw_tile_map_singletexture(x, y);
			}
		}
	}
	else//we draw the ground details
	{
		if (use_normal_mapping)
		{
			for(y=y_start;y<=y_end;y++)
			{
				y_scaled=y*3.0f;
				for(x=x_start;x<=x_end;x++)
				{
					x_scaled=x*3.0f;
					if(IS_WATER_TILE(tile_map[y*tile_map_size_x+x]))continue;//lake, skip
					if(tile_map[y*tile_map_size_x+x]==255)continue;//null, skip
					if(!check_tile_in_frustrum(x_scaled,y_scaled))continue;//outside of the frustrum
					cur_texture=get_texture_id(tile_list[tile_map[y*tile_map_size_x+x]]);
					if(last_texture!=cur_texture)
					{
						bind_texture_id(cur_texture);
					}
					cur_normal_map = get_normal_texture_ID(x, y);
					if (cur_normal_map != last_normal_map)
					{
						last_normal_map = cur_normal_map;
						ELglActiveTextureARB(normal_map_unit);
						glBindTexture(GL_TEXTURE_2D, cur_normal_map);
						ELglActiveTextureARB(base_unit);
					}
					draw_tile_map_normal_mapping(x, y);
				}
			}

		}
		else
		{
			for(y=y_start;y<=y_end;y++)
			{
				y_scaled=y*3.0f;
				for(x=x_start;x<=x_end;x++)
				{
					x_scaled=x*3.0f;
					if(IS_WATER_TILE(tile_map[y*tile_map_size_x+x]))continue;//lake, skip
					if(tile_map[y*tile_map_size_x+x]==255)continue;//null, skip
					if(!check_tile_in_frustrum(x_scaled,y_scaled))continue;//outside of the frustrum
					cur_texture=get_texture_id(tile_list[tile_map[y*tile_map_size_x+x]]);
					if(last_texture!=cur_texture)
					{
						bind_texture_id(cur_texture);
					}
					draw_tile_map_multitexture(x, y);
				}
			}
		}
	}
	if(have_multitexture && !dungeon && (clouds_shadows || use_normal_mapping))
	{
		if (clouds_shadows)
		{
			//disable the second texture unit
			ELglActiveTextureARB(detail_unit);
			glDisable(GL_TEXTURE_2D);
		}
		if (use_normal_mapping)
		{
			//disable the normal map texture unit
			ELglActiveTextureARB(normal_map_unit);
			glDisable(GL_TEXTURE_2D);
			
			//disable shader
			ELglUseProgramObjectARB(0);
		}
		ELglActiveTextureARB(base_unit);
	}
	glEnable(GL_TEXTURE_2D);
}
#else
void draw_tile_map()
{
#ifndef	NEW_FRUSTUM
	int x_start,x_end,y_start,y_end;
#else
	unsigned int i, l, start, stop;
#endif
	int x,y;
	float x_scaled,y_scaled;
	int cur_texture;

	if(have_multitexture && !dungeon && clouds_shadows)
		{
			//bind the detail texture
			ELglActiveTextureARB(detail_unit);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, get_texture_id(ground_detail_text));
			ELglActiveTextureARB(base_unit);
			glEnable(GL_TEXTURE_2D);
		}

#ifndef	NEW_FRUSTUM

	//get only the tiles around the camera
	//we have the axes inverted, btw the go from 0 to -255
	if(cx<0)x=(cx*-1)/3;
	else x=cx/3;
	if(cy<0)y=(cy*-1)/3;
	else y=cy/3;
	
	x_start = (int)x - 8;
	y_start = (int)y - 8;
	x_end   = (int)x + 8;
	y_end   = (int)y + 8;

	if(x_start<0)x_start=0;
	if(x_end>=tile_map_size_x)x_end=tile_map_size_x-1;
	if(y_start<0)y_start=0;
	if(y_end>=tile_map_size_y)y_end=tile_map_size_y-1;
#endif
	if(!have_multitexture || dungeon || (!clouds_shadows && !use_shadow_mapping))
		{
			glBegin(GL_QUADS);
#ifndef	NEW_FRUSTUM
			for(y=y_start;y<=y_end;y++)
				{
					y_scaled=y*3.0f;
					for(x=x_start;x<=x_end;x++)
						{
							x_scaled=x*3.0f;
							if(IS_WATER_TILE(tile_map[y*tile_map_size_x+x]))continue;//lake, skip
							if(tile_map[y*tile_map_size_x+x]==255)continue;//null, skip
							if(!check_tile_in_frustrum(x_scaled,y_scaled))continue;//outside of the frustrum
#else
			get_intersect_start_stop(main_bbox_tree, TYPE_TERRAIN, &start, &stop);
			for (i = start; i < stop; i++)
			{
				l = get_intersect_item_ID(main_bbox_tree, i);
				x = get_terrain_x(l);
				y = get_terrain_y(l);
				y_scaled = y*3.0f;
				x_scaled = x*3.0f;
#endif
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
#ifndef	NEW_FRUSTUM
						}
#endif
				}
			glEnd();
		}
	else//we draw the ground details
		{

			glBegin(GL_QUADS);
#ifndef	NEW_FRUSTUM
			for(y=y_start;y<=y_end;y++)
				{
					y_scaled=y*3.0f;
					for(x=x_start;x<=x_end;x++)
						{
							x_scaled=x*3.0f;
							if(IS_WATER_TILE(tile_map[y*tile_map_size_x+x]))continue;//lake, skip
							if(tile_map[y*tile_map_size_x+x]==255)continue;//null, skip
							if(!check_tile_in_frustrum(x_scaled,y_scaled))continue;//outside of the frustrum
#else
			get_intersect_start_stop(main_bbox_tree, TYPE_TERRAIN, &start, &stop);
			for (i = start; i < stop; i++)
			{
				l = get_intersect_item_ID(main_bbox_tree, i);
				x = get_terrain_x(l);
				y = get_terrain_y(l);
				y_scaled = y*3.0f;
				x_scaled = x*3.0f;
#endif
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
#ifndef	NEW_FRUSTUM
						}
#endif
				}
			glEnd();

		}
	if(have_multitexture && !dungeon && clouds_shadows)
		{
			//disable the second texture unit
			ELglActiveTextureARB(detail_unit);
			glDisable(GL_TEXTURE_2D);
			ELglActiveTextureARB(base_unit);
		}

	glEnable(GL_TEXTURE_2D);
}
#endif

//load only the tiles that are on the map
void load_map_tiles()
{
	int i;
#ifdef MAP_EDITOR2
	char str[200];
	
	for(i=0;i<255;i++){
		snprintf(str,sizeof(str), "./tiles/tile%i.bmp",i);
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
					sprintf(str,"./tiles/tile%i.bmp",cur_tile);
					if(IS_WATER_TILE(cur_tile) && IS_REFLECTING(cur_tile))
					  tile_list[cur_tile]=load_texture_cache(str,70);
					else tile_list[cur_tile]=load_texture_cache(str,255);
			
				}
		}
#endif
}


