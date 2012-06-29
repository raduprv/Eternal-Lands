#include <math.h>
#include <stdlib.h>
#include "tiles.h"
#include "global.h"

void destroy_map_tiles()
{
#ifdef	NEW_TEXTURES
	int i;

	for (i = 0; i < 256; i++)
	{
		if (map_tiles[i].image)
		{
			free(map_tiles[i].image);
		}
	}
#else	/* NEW_TEXTURES */
	int i=0;
	for(;i<256;i++) if(map_tiles[i].texture) free(map_tiles[i].texture);
#endif	/* NEW_TEXTURES */
}


void draw_tile_map()
{
	int x_start,x_end,y_start,y_end;
	int x,y;
	float x_scaled,y_scaled;

	//get only the tiles around the camera
	//we have the axes inverted, btw the go from 0 to -255
	if(camera_x<0)x=(int)(camera_x*-1)/3;
	else x=(int)camera_x/3;
	if(camera_y<0)y=(int)(camera_y*-1)/3;
	else y=(int)camera_y/3;
	x_start=x-(int)zoom_level;
	y_start=y-(int)zoom_level;
	x_end=x+(int)zoom_level;
	y_end=y+(int)zoom_level;
	if(x_start<0)x_start=0;
	if(x_end>=tile_map_size_x)x_end=tile_map_size_x-1;
	if(y_start<0)y_start=0;
	if(y_end>=tile_map_size_y)y_end=tile_map_size_y-1;
	if(!have_multitexture || poor_man)
		{
			for(y=y_start;y<=y_end;y++)
				{
					y_scaled=y*3.0f;
					for(x=x_start;x<=x_end;x++)
						{
							x_scaled=x*3.0f;
							if(is_water_tile(tile_map[y*tile_map_size_x+x]))continue;//lake, skip
							if(tile_map[y*tile_map_size_x+x]==255){
								/*glDisable(GL_TEXTURE_2D);
								glDisable(GL_LIGHTING);
								glBegin(GL_QUADS);
								glColor3f(0.2,0.2,0.2);
				 				glVertex3f(x_scaled,y_scaled+3, -3.0f);
								glVertex3f(x_scaled,y_scaled, -3.0f);
								glVertex3f(x_scaled+3, y_scaled,-3.0f);
								glVertex3f(x_scaled+3, y_scaled+3,-3.0f);
								glColor3f(1.0,1.0,1.0);
				 				glEnd();
								glEnable(GL_LIGHTING);
								glEnable(GL_TEXTURE_2D);*/
								continue;
							}//null, skip
#ifdef	NEW_TEXTURES
							bind_texture(tile_list[tile_map[y*tile_map_size_x+x]]);
#else	/* NEW_TEXTURES */
							get_and_set_texture_id(tile_list[tile_map[y*tile_map_size_x+x]]);
#endif	/* NEW_TEXTURES */

							glBegin(GL_QUADS);
 							glTexCoord2f(0, 0.0f);
			 				glVertex3f(x_scaled,y_scaled+3, 0.0f);
							glTexCoord2f(0, 1.0f);
							glVertex3f(x_scaled,y_scaled, 0.0f);
							glTexCoord2f(1.0f, 1.0f);
							glVertex3f(x_scaled+3, y_scaled,0.0f);
							glTexCoord2f(1.0f, 0.0f);
							glVertex3f(x_scaled+3, y_scaled+3,0.0f);
							glEnd();
						}
				}
		}
	else//we draw the ground details
		{
			//bind the detail texture
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glEnable(GL_TEXTURE_2D);
#ifdef	NEW_TEXTURES
			bind_texture_unbuffered(ground_detail_text);
#else	/* NEW_TEXTURES */
			glBindTexture(GL_TEXTURE_2D,  texture_cache[ground_detail_text].texture_id);
#endif	/* NEW_TEXTURES */
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_2D);

			for(y=y_start;y<=y_end;y++)
				{
					y_scaled=y*3.0f;
					for(x=x_start;x<=x_end;x++)
						{
							x_scaled=x*3.0f;
							if(is_water_tile(tile_map[y*tile_map_size_x+x]))continue;//lake, skip
							if(tile_map[y*tile_map_size_x+x]==255){
								/*glDisable(GL_TEXTURE_2D);
								glDisable(GL_LIGHTING);
								glBegin(GL_QUADS);
								glColor3f(0.2,0.2,0.2);
				 				glVertex3f(x_scaled,y_scaled+3, -3.0f);
								glVertex3f(x_scaled,y_scaled, -3.0f);
								glVertex3f(x_scaled+3, y_scaled,-3.0f);
								glVertex3f(x_scaled+3, y_scaled+3,-3.0f);
								glColor3f(1.0,1.0,1.0);
				 				glEnd();
								glEnable(GL_LIGHTING);
								glEnable(GL_TEXTURE_2D);*/
								continue;
							}//null, skip
#ifdef	NEW_TEXTURES
							bind_texture(tile_list[tile_map[y*tile_map_size_x+x]]);
#else	/* NEW_TEXTURES */
							get_and_set_texture_id(tile_list[tile_map[y*tile_map_size_x+x]]);
#endif	/* NEW_TEXTURES */
							//draw our normal tile
							glBegin(GL_QUADS);
 							glMultiTexCoord2fARB(GL_TEXTURE0_ARB,0, 0.0f);
 							glMultiTexCoord2fARB(GL_TEXTURE1_ARB,x_scaled/texture_scale+clouds_movement_u, (y_scaled+3.0)/texture_scale+clouds_movement_v);
			 				glVertex3f(x_scaled,y_scaled+3, 0.0f);

							glMultiTexCoord2fARB(GL_TEXTURE0_ARB,0, 1.0f);
							glMultiTexCoord2fARB(GL_TEXTURE1_ARB,x_scaled/texture_scale+clouds_movement_u, y_scaled/texture_scale+clouds_movement_v);
							glVertex3f(x_scaled,y_scaled, 0.0f);

							glMultiTexCoord2fARB(GL_TEXTURE0_ARB,1.0f, 1.0f);
							glMultiTexCoord2fARB(GL_TEXTURE1_ARB,(x_scaled+3.0f)/texture_scale+clouds_movement_u, y_scaled/texture_scale+clouds_movement_v);
							glVertex3f(x_scaled+3, y_scaled,0.0f);

							glMultiTexCoord2fARB(GL_TEXTURE0_ARB,1.0f, 0.0f);
							glMultiTexCoord2fARB(GL_TEXTURE1_ARB,(x_scaled+3.0)/texture_scale+clouds_movement_u, (y_scaled+3.0)/texture_scale+clouds_movement_v);
							glVertex3f(x_scaled+3, y_scaled+3,0.0f);
							glEnd();
						}
				}
			//disable the second texture unit
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glDisable(GL_TEXTURE_2D);
			glActiveTextureARB(GL_TEXTURE0_ARB);

		}

}

//load only the tiles that are on the map
void load_map_tiles()
{
	int i;
	int cur_tile;
	char str[80];
	for(i=0;i<tile_map_size_x*tile_map_size_y;i++)
	{
		cur_tile=tile_map[i];
		//check to see if we already have the current tile loaded
		if(!tile_list[cur_tile] && cur_tile!=255)//if it is 255, it's a null tile, don't load it
			{
				//tile not loaded, so load it
				if(!cur_tile && dungeon) cur_tile=231;
				sprintf(str,"./3dobjects/tile%i.dds",cur_tile);
#ifdef	NEW_TEXTURES
				tile_list[cur_tile] = load_texture_cached(str, tt_mesh);
#else	/* NEW_TEXTURES */
				if(is_water_tile(cur_tile) && is_reflecting(cur_tile))
					tile_list[cur_tile]=load_texture_cache(str,70);
				else
					tile_list[cur_tile]=load_texture_cache(str,255);
#endif	/* NEW_TEXTURES */
			}
	}

}

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
