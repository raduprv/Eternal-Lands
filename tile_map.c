#include <math.h>
#include <stdlib.h>
#include "global.h"

void draw_tile_map()
{
	int x_start,x_end,y_start,y_end;
	int x,y;
	float x_scaled,y_scaled;
	int cur_texture;

	if(have_multitexture && clouds_shadows)
		{
			//bind the detail texture
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,  texture_cache[ground_detail_text].texture_id);
			glActiveTextureARB(GL_TEXTURE0_ARB);
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
	if(!have_multitexture || !clouds_shadows)
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
									glBindTexture(GL_TEXTURE_2D, cur_texture);
									glBegin(GL_QUADS);
									last_texture=cur_texture;
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
									glBindTexture(GL_TEXTURE_2D, cur_texture);
									glBegin(GL_QUADS);
									last_texture=cur_texture;
								}
							//draw our normal tile
 							glMultiTexCoord2fARB(GL_TEXTURE0_ARB,0, 1.0f);
 							glMultiTexCoord2fARB(GL_TEXTURE1_ARB,x_scaled/texture_scale+clouds_movement_u, (y_scaled+3.0)/texture_scale+clouds_movement_v);
			 				glVertex3f(x_scaled,y_scaled+3, 0.0f);

							glMultiTexCoord2fARB(GL_TEXTURE0_ARB,0, 0);
							glMultiTexCoord2fARB(GL_TEXTURE1_ARB,x_scaled/texture_scale+clouds_movement_u, y_scaled/texture_scale+clouds_movement_v);
							glVertex3f(x_scaled,y_scaled, 0.0f);

							glMultiTexCoord2fARB(GL_TEXTURE0_ARB,1.0f, 0);
							glMultiTexCoord2fARB(GL_TEXTURE1_ARB,(x_scaled+3.0f)/texture_scale+clouds_movement_u, y_scaled/texture_scale+clouds_movement_v);
							glVertex3f(x_scaled+3, y_scaled,0.0f);

							glMultiTexCoord2fARB(GL_TEXTURE0_ARB,1.0f, 1.0f);
							glMultiTexCoord2fARB(GL_TEXTURE1_ARB,(x_scaled+3.0)/texture_scale+clouds_movement_u, (y_scaled+3.0)/texture_scale+clouds_movement_v);
							glVertex3f(x_scaled+3, y_scaled+3,0.0f);
						}
				}
			glEnd();

		}
	if(have_multitexture && clouds_shadows)
		{
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
			if(!tile_list[cur_tile] && cur_tile && cur_tile!=255)//if it is 255, it's a null tile, don't load it
				//if it is 0, it's a lake tile, don't load it
				{
					//tile not loaded, so load it
					sprintf(str,"./tiles/tile%i.bmp",cur_tile);
					tile_list[cur_tile]=load_texture_cache(str,255);
				}
		}

}


