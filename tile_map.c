#include <math.h>
#include <stdlib.h>
#include "global.h"

unsigned char *tile_map;
unsigned char *height_map;
int tile_map_size_x;
int tile_map_size_y;
int tile_list[256];
char dungeon=0;//no sun
float ambient_r=0;
float ambient_g=0;
float ambient_b=0;
char map_file_name[60];

void draw_tile_map()
{
	int x_start,x_end,y_start,y_end;
	int x,y;
	float x_scaled,y_scaled;
	int cur_texture;

	if(have_multitexture && clouds_shadows)
		{
			//bind the detail texture
			ELglActiveTextureARB(detail_unit);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, get_texture_id(ground_detail_text));
			ELglActiveTextureARB(base_unit);
			glEnable(GL_TEXTURE_2D);
		}

	//get only the tiles around the camera
	//we have the axes inverted, btw the go from 0 to -255
	if(cx<0)x=(cx*-1)/3;
	else x=cx/3;
	if(cy<0)y=(cy*-1)/3;
	else y=cy/3;
	x_start=(int)x-5;
	y_start=(int)y-5;
	x_end=(int)x+5;
	y_end=(int)y+5;
	if(x_start<0)x_start=0;
	if(x_end>=tile_map_size_x)x_end=tile_map_size_x-1;
	if(y_start<0)y_start=0;
	if(y_end>=tile_map_size_y)y_end=tile_map_size_y-1;
	if(!have_multitexture || (!clouds_shadows && !use_shadow_mapping))
		{
			glBegin(GL_QUADS);
			for(y=y_start;y<=y_end;y++)
				{
					y_scaled=y*3.0f;
					for(x=x_start;x<=x_end;x++)
						{
							x_scaled=x*3.0f;
							if(!tile_map[y*tile_map_size_x+x])continue;//lake, skip
							if(tile_map[y*tile_map_size_x+x]==255)continue;//null, skip
							if(!check_tile_in_frustrum(x_scaled,y_scaled))continue;//outside of the frustrum
							cur_texture=get_texture_id(tile_list[tile_map[y*tile_map_size_x+x]]);
							if(last_texture!=cur_texture)
								{
									glEnd();
									bind_texture_id(cur_texture);
									glBegin(GL_QUADS);
								}

 							glTexCoord2f(0, 1.0f);
			 				glVertex3f(x_scaled,y_scaled+3, 0.0f);
							glTexCoord2f(0, 0);
							glVertex3f(x_scaled,y_scaled, 0.0f);
							glTexCoord2f(1.0f, 0);
							glVertex3f(x_scaled+3, y_scaled,0.0f);
							glTexCoord2f(1.0f, 1.0f);
							glVertex3f(x_scaled+3, y_scaled+3,0.0f);
						}
				}
			glEnd();
		}
	else//we draw the ground details
		{

			glBegin(GL_QUADS);
			for(y=y_start;y<=y_end;y++)
				{
					y_scaled=y*3.0f;
					for(x=x_start;x<=x_end;x++)
						{
							x_scaled=x*3.0f;
							if(!tile_map[y*tile_map_size_x+x])continue;//lake, skip
							if(tile_map[y*tile_map_size_x+x]==255)continue;//null, skip
							if(!check_tile_in_frustrum(x_scaled,y_scaled))continue;//outside of the frustrum
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
			 				glVertex3f(x_scaled,y_scaled+3, 0.0f);

							ELglMultiTexCoord2fARB(base_unit,0, 0);
							ELglMultiTexCoord2fARB(detail_unit,x_scaled/texture_scale+clouds_movement_u, y_scaled/texture_scale+clouds_movement_v);
							glVertex3f(x_scaled,y_scaled, 0.0f);

							ELglMultiTexCoord2fARB(base_unit,1.0f, 0);
							ELglMultiTexCoord2fARB(detail_unit,(x_scaled+3.0f)/texture_scale+clouds_movement_u, y_scaled/texture_scale+clouds_movement_v);
							glVertex3f(x_scaled+3, y_scaled,0.0f);

							ELglMultiTexCoord2fARB(base_unit,1.0f, 1.0f);
							ELglMultiTexCoord2fARB(detail_unit,(x_scaled+3.0)/texture_scale+clouds_movement_u, (y_scaled+3.0)/texture_scale+clouds_movement_v);
							glVertex3f(x_scaled+3, y_scaled+3,0.0f);
						}
				}
			glEnd();

		}
	if(have_multitexture && clouds_shadows)
		{
			//disable the second texture unit
			ELglActiveTextureARB(detail_unit);
			glDisable(GL_TEXTURE_2D);
			ELglActiveTextureARB(base_unit);
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
			if(!tile_list[cur_tile] && cur_tile && cur_tile!=255)//if it is 255, it's a null tile, don't load it
				//if it is 0, it's a lake tile, don't load it
				{
					//tile not loaded, so load it
					sprintf(str,"./tiles/tile%i.bmp",cur_tile);
					tile_list[cur_tile]=load_texture_cache(str,255);
				}
		}

}


