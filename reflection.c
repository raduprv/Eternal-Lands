#include "global.h"
#include <math.h>

float mrandom(float max)
{
  return ((float) max * (rand () % 8 ));
}


void draw_3d_reflection(object3d * object_id)
{
	float x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;

	int materials_no,texture_id;
	int i;

	e3d_array_vertex *array_vertex;
	e3d_array_normal *array_normal;
	e3d_array_uv_main *array_uv_main;
	e3d_array_order *array_order;

	int is_transparent;

	check_gl_errors();
	is_transparent=object_id->e3d_data->is_transparent;
	materials_no=object_id->e3d_data->materials_no;

	array_vertex=object_id->e3d_data->array_vertex;
	array_normal=object_id->e3d_data->array_normal;
	array_uv_main=object_id->e3d_data->array_uv_main;
	array_order=object_id->e3d_data->array_order;

	if(object_id->self_lit && (night_shadows_on || dungeon))
		{
			glDisable(GL_LIGHTING);
			glColor3f(object_id->r,object_id->g,object_id->b);
		}

	if(is_transparent)
		{
			glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
			glAlphaFunc(GL_GREATER,0.05f);

		}


	check_gl_errors();
	glPushMatrix();//we don't want to affect the rest of the scene
	x_pos=object_id->x_pos;
	y_pos=object_id->y_pos;
	z_pos=object_id->z_pos;
	if(z_pos<0)z_pos+=-water_deepth_offset*2;

	glTranslatef (x_pos, y_pos,z_pos);
	x_rot=object_id->x_rot;
	y_rot=object_id->y_rot;
	z_rot=object_id->z_rot;
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	check_gl_errors();
	glVertexPointer(3,GL_FLOAT,0,array_vertex);
	glTexCoordPointer(2,GL_FLOAT,0,array_uv_main);
	glNormalPointer(GL_FLOAT,0,array_normal);
	for(i=0;i<materials_no;i++)
		{
			texture_id=get_texture_id(array_order[i].texture_id);
			if(last_texture!=texture_id)
				{
					last_texture=texture_id;
					glBindTexture(GL_TEXTURE_2D, texture_id);
				}
			check_gl_errors();
			//if(have_compiled_vertex_array)ELglLockArraysEXT(array_order[i].start, array_order[i].count);
			glDrawArrays(GL_TRIANGLES,array_order[i].start,array_order[i].count);
			//if(have_compiled_vertex_array)ELglUnlockArraysEXT();
		}

	check_gl_errors();
	glPopMatrix();//restore the scene
	check_gl_errors();


	if(object_id->self_lit && (night_shadows_on || dungeon))glEnable(GL_LIGHTING);
	if(is_transparent)
		{
			glDisable(GL_ALPHA_TEST);
		}

	check_gl_errors();
}

//if there is any reflecting tile, returns 1, otherwise 0
int find_reflection()
{
	int x_start,x_end,y_start,y_end;
	int x,y;
	float x_scaled,y_scaled;

	//get only the tiles around the camera
	//we have the axes inverted, btw the go from 0 to -255
	if(cx<0)x=(int)(cx*-1)/3;
	else x=(int)cx/3;
	if(cy<0)y=(int)(cy*-1)/3;
	else y=(int)cy/3;
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
					if(!tile_map[y*tile_map_size_x+x])return 1;
				}
		}
	return 0;
}

int find_local_reflection(int x_pos,int y_pos,int range)
{
	int x_start,x_end,y_start,y_end;
	int x,y;

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
					if(!tile_map[y*tile_map_size_x+x])return 1;
				}
		}
	return 0;
}


void display_3d_reflection()
{
	int i;
	int x,y;
	double water_clipping_p[4]={0,0,-1,water_deepth_offset};

	x=(int)-cx;
	y=(int)-cy;

	glEnable(GL_CLIP_PLANE0);
	glClipPlane(GL_CLIP_PLANE0, water_clipping_p);
	
	glDisable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	set_material(0.1f,0.2f,0.3f);
	glPushMatrix();
	glScalef(1.0f, 1.0f, -1.0f);
	for(i=0;i<max_obj_3d;i++)
		{
			if(objects_list[i])
			     {
					 if(!objects_list[i]->e3d_data->is_ground)
					 	{
			         		int dist1;
			         		int dist2;

			         		dist1=x-(int)objects_list[i]->x_pos;
			         		dist2=y-(int)objects_list[i]->y_pos;
			         		if(dist1*dist1+dist2*dist2<=21*21)
			         			{
									float x_len, y_len, z_len;
									float radius;

									z_len=objects_list[i]->e3d_data->max_z-objects_list[i]->e3d_data->min_z;
									x_len=objects_list[i]->e3d_data->max_x-objects_list[i]->e3d_data->min_x;
									y_len=objects_list[i]->e3d_data->max_y-objects_list[i]->e3d_data->min_y;
									//do some checks, to see if we really have to display this object
									if(x_len<5 && y_len<5 && z_len<4 && !find_local_reflection((int)objects_list[i]->x_pos,(int)objects_list[i]->y_pos,1))continue;

									radius=x_len/2;
									if(radius<y_len/2)radius=y_len/2;
									if(radius<z_len)radius=z_len;
									//not in the middle of the air
									if(SphereInFrustum(objects_list[i]->x_pos,objects_list[i]->y_pos,
										objects_list[i]->z_pos,radius))
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
	int x,y;
	float x_scaled,y_scaled;
	int cur_texture;

	//glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	bind_texture_id(get_texture_id(sky_text_1));

	//get only the tiles around the camera
	//we have the axes inverted, btw the go from 0 to -255
	if(cx<0)x=(int)(cx*-1)/3;
	else x=(int)cx/3;
	if(cy<0)y=(int)(cy*-1)/3;
	else y=(int)cy/3;
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
					if(!tile_map[y*tile_map_size_x+x])draw_lake_water_tile(x_scaled,y_scaled);
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
