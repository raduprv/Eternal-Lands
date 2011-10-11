#include "global.h"
#include <math.h>

float mrandom(float max)
{
  return ((float) max * (rand () % 8 ));
}

#include "../e3d_object.h"

void draw_3d_reflection(object3d * object_id)
{
	int i;
	float x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;

	//also, update the last time this object was used
	object_id->last_acessed_time=cur_time;

	set_emission(object_id);

	CHECK_GL_ERRORS();

	glPushMatrix();//we don't want to affect the rest of the scene
	x_pos = object_id->x_pos;
	y_pos = object_id->y_pos;
	z_pos = object_id->z_pos;
	glTranslatef (x_pos, y_pos, z_pos);

	x_rot = object_id->x_rot;
	y_rot = object_id->y_rot;
	z_rot = object_id->z_rot;
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	CHECK_GL_ERRORS();

	e3d_enable_vertex_arrays(object_id->e3d_data, 1, 1);
		
	CHECK_GL_ERRORS();

	for (i = 0; i < object_id->e3d_data->material_no; i++)
	{
		if (object_id->e3d_data->materials[i].options)
		{
			//enable alpha filtering, so we have some alpha key
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0.05f);
			glDisable(GL_CULL_FACE);
		}
		else
		{
			glDisable(GL_ALPHA_TEST);
			glEnable(GL_CULL_FACE);
		}

#ifdef	NEW_TEXTURES
		bind_texture(object_id->e3d_data->materials[i].texture);
#else	/* NEW_TEXTURES */
		get_and_set_texture_id(object_id->e3d_data->materials[i].texture);
#endif	/* NEW_TEXTURES */

		ELglDrawRangeElementsEXT(GL_TRIANGLES,
			object_id->e3d_data->materials[i].triangles_indices_min,
			object_id->e3d_data->materials[i].triangles_indices_max,
			object_id->e3d_data->materials[i].triangles_indices_count,
			object_id->e3d_data->index_type,
			object_id->e3d_data->materials[i].triangles_indices_index);
	}

	glPopMatrix();//restore the scene
	CHECK_GL_ERRORS();

	if (have_multitexture && clouds_shadows)
	{
		glClientActiveTextureARB(GL_TEXTURE1_ARB);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glClientActiveTextureARB(GL_TEXTURE0_ARB);
	}

	e3d_disable_vertex_arrays();
	glDisable(GL_COLOR_MATERIAL);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	if (object_id->self_lit && (night_shadows_on || dungeon)) glEnable(GL_LIGHTING);
	if (object_id->e3d_data->materials[object_id->e3d_data->material_no-1].options)
	{
		glDisable(GL_ALPHA_TEST);
		glEnable(GL_CULL_FACE);
	}
	CHECK_GL_ERRORS();
}


//if there is any reflecting tile, returns 1, otherwise 0
int find_reflection()
{
	int x_start,x_end,y_start,y_end;
	int x,y;
	float x_scaled,y_scaled;
	int found_water=0;

	//get only the tiles around the camera
	//we have the axes inverted, btw the go from 0 to -255
	if(camera_x<0)x=(int)(camera_x*-1)/3;
	else x=(int)camera_x/3;
	if(camera_y<0)y=(int)(camera_y*-1)/3;
	else y=(int)camera_y/3;
	x_start=(int)x-4;
	y_start=(int)y-4;
	x_end=(int)x+4;
	y_end=(int)y+4;
	if(x_start<0)x_start=0;
	if(x_end>=tile_map_size_x)x_end=tile_map_size_x-1;
	if(y_start<0)y_start=0;
	if(y_end>=tile_map_size_y)y_end=tile_map_size_y-1;
	for(y=y_start;y<=y_end;y++)
		{
			y_scaled=y*3.0f;
			for(x=x_start;x<=x_end;x++)
				{
					x_scaled=x*3.0f;
					if(!check_tile_in_frustrum(x_scaled,y_scaled))continue;//outside of the frustrum
					if(is_water_tile(tile_map[y*tile_map_size_x+x]))
						{
							if(is_reflecting(tile_map[y*tile_map_size_x+x]))return 2;
							found_water=1;
						}
				}
		}
	return found_water;
}

int find_local_reflection(int x_pos,int y_pos,int range)
{
	int x_start,x_end,y_start,y_end;
	int x,y;
	int found_water=0;

	//get only the tiles around the camera
	//we have the axes inverted, btw the go from 0 to -255
	if(x_pos<0)x=(x_pos*-1)/3;
	else x=x_pos/3;
	if(y_pos<0)y=(y_pos*-1)/3;
	else y=y_pos/3;
	x_start=(int)x-range;
	y_start=(int)y-range;
	x_end=(int)x+range;
	y_end=(int)y+range;
	if(x_start<0)x_start=0;
	if(x_end>=tile_map_size_x)x_end=tile_map_size_x-1;
	if(y_start<0)y_start=0;
	if(y_end>=tile_map_size_y)y_end=tile_map_size_y-1;
	for(y=y_start;y<=y_end;y++)
		{
			for(x=x_start;x<=x_end;x++)
				{
					if(is_water_tile(tile_map[y*tile_map_size_x+x]))
						{
							if(is_reflecting(tile_map[y*tile_map_size_x+x]))return 2;
							found_water=1;
						}
				}
		}
	return found_water;
}


void display_3d_reflection()
{
	int i;
	int x,y;
	double water_clipping_p[4]={0,0,-1,water_deepth_offset};

	x=(int)-camera_x;
	y=(int)-camera_y;

	glEnable(GL_CLIP_PLANE0);
	glClipPlane(GL_CLIP_PLANE0, water_clipping_p);
	
	glDisable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	set_material(0.1f,0.2f,0.3f);
	glPushMatrix();
	glScalef(1.0f, 1.0f, -1.0f);
	for (i = 0; i < MAX_OBJ_3D; i++)
		{
			if(objects_list[i] && objects_list[i]->blended!=20)
			     {
					if (objects_list[i]->e3d_data->vertex_layout->normal_count > 0)
					 	{
			         		int dist1;
			         		int dist2;

			         		dist1=x-(int)objects_list[i]->x_pos;
			         		dist2=y-(int)objects_list[i]->y_pos;
			         		if(dist1*dist1+dist2*dist2<=21*21)
			         			{
                     						draw_3d_reflection(objects_list[i]);
								}
						}
                 }
		}
	glPopMatrix();
	reset_material();
	glDisable(GL_CLIP_PLANE0);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void make_lake_water_noise()
{
	int x,y;
	float noise_u,noise_v;

	for(x=0;x<16;x++)
	for(y=0;y<16;y++)
		{

			noise_u=mrandom(0.001f);
			noise_v=mrandom(0.001f);

			noise_array[y*16+x].u=noise_u;
			noise_array[y*16+x].v=noise_v;

		}

}

void draw_lake_water_tile(float x_pos, float y_pos)
{
	int x,y;
	float fx,fy;
	float x_step,y_step;
	float u_step,v_step;
	float uv_tile=50;

	x_step=3.0f/16.0f;
	y_step=3.0f/16.0f;

	u_step=3.0f/uv_tile;
	v_step=3.0f/uv_tile;

	glBegin(GL_QUADS);
	for(y=0,fy=y_pos;y<16;fy+=y_step,y++)
		{
			for(x=0,fx=x_pos;x<16;fx+=x_step,x++)
				{
 					glTexCoord2f((fx)*u_step+noise_array[((y+1)&15)*16+x].u+water_movement_u, (fy+y_step)*v_step+noise_array[((y+1)&15)*16+x].v+water_movement_v);
	 				glVertex3f(fx,fy+y_step, water_deepth_offset);

					glTexCoord2f((fx)*u_step+noise_array[y*16+x].u+water_movement_u, (fy)*v_step+noise_array[y*16+x].v+water_movement_v);
					glVertex3f(fx,fy, water_deepth_offset);

					glTexCoord2f((fx+x_step)*u_step+noise_array[y*16+((x+1)&15)].u+water_movement_u, (fy)*v_step+noise_array[y*16+((x+1)&15)].v+water_movement_v);
					glVertex3f(fx+x_step, fy,water_deepth_offset);

					glTexCoord2f((fx+x_step)*u_step+noise_array[((y+1)&15)*16+((x+1)&15)].u+water_movement_u, (fy+y_step)*v_step+noise_array[((y+1)&15)*16+((x+1)&15)].v+water_movement_v);
					glVertex3f(fx+x_step, fy+y_step,water_deepth_offset);

				}

		}
	glEnd();
}

void draw_lake_tiles()
{
	int x_start,x_end,y_start,y_end;
	int x,y, index;
	float x_scaled,y_scaled;

	//glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

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
	for(y=y_start;y<=y_end;y++)
	{
		y_scaled=y*3.0f;
		for(x=x_start;x<=x_end;x++)
		{
			x_scaled=x*3.0f;
			if(!check_tile_in_frustrum(x_scaled,y_scaled))continue;//outside of the frustrum
			if(is_water_tile(tile_map[y*tile_map_size_x+x]))
			{
				if(!tile_map[y*tile_map_size_x+x])
				{
					if (dungeon)
					{
						index = 231;
					}
					else
					{
						index = 0;
					}
				}
				else
				{
					index = tile_map[y * tile_map_size_x + x];
				}
#ifdef	NEW_TEXTURES
				bind_texture(tile_list[index]);
#else	/* NEW_TEXTURES */
				get_and_set_texture_id(tile_list[index]);
#endif	/* NEW_TEXTURES */
				draw_lake_water_tile(x_scaled,y_scaled);
			}
		}
	}

	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
}

void draw_sky_background()
{
	Enter2DMode();
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	//draw the sky background

			glColor3fv(sky_lights_c1[light_level]);
			glVertex3i(0,0,0);

			glColor3fv(sky_lights_c2[light_level]);
			glVertex3i(0,window_height,0);

			glColor3fv(sky_lights_c3[light_level]);
			glVertex3i(window_width,window_height,0);

			glColor3fv(sky_lights_c4[light_level]);
			glVertex3i(window_width,0,0);

	glEnd();
	glEnable(GL_TEXTURE_2D);
	Leave2DMode();


}

void draw_dungeon_sky_background()
{
	Enter2DMode();
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	//draw the sky background

			glColor3f(0,0.21f,0.34f);
			glVertex3i(0,0,0);

			glColor3f(0,0.21f,0.34f);
			glVertex3i(0,window_height,0);

			glColor3f(0,0.21f,0.34f);
			glVertex3i(window_width,window_height,0);

			glColor3f(0,0.21f,0.34f);
			glVertex3i(window_width,0,0);

	glEnd();
	glEnable(GL_TEXTURE_2D);
	Leave2DMode();


}
