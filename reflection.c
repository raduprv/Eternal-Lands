#include "global.h"
#include <math.h>

float mrandom(float max)
{
  return ((float) max * (rand () % 8 ));
}

void draw_3d_reflection(object3d * object_id)
{
	float x,y,z,u,v;
	float x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;
	float noise_x,noise_y,noise_z;

	int faces_no,materials_no,texture_id,a,b,c;
	int i,k,j;
	e3d_face *faces_list;
	e3d_vertex *vertex_list;
	e3d_material *material_list;
	char is_transparent;


	faces_no=object_id->e3d_data->face_no;
	faces_list=object_id->e3d_data->faces;
	vertex_list=object_id->e3d_data->vertexes;
	is_transparent=object_id->e3d_data->is_transparent;

	x_pos=object_id->x_pos;
	y_pos=object_id->y_pos;
	z_pos=object_id->z_pos+-water_deepth_offset*2;

	x_rot=object_id->x_rot;
	y_rot=object_id->y_rot;
	z_rot=object_id->z_rot;

  if(object_id->self_lit && (night_shadows_on || dungeon))
  {
 	glDisable(GL_LIGHTING);
    glColor3f(object_id->r,object_id->g,object_id->b);
  }

  if(is_transparent)
  	{
	    glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
	    glAlphaFunc(GL_GREATER,0.05f);
	    //glDisable(GL_CULL_FACE);

	}


	glPushMatrix();//we don't want to affect the rest of the scene
	glTranslatef (x_pos, y_pos,z_pos);
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	j=0;//j is used for the noise table
	glBegin(GL_TRIANGLES);
	for(i=0;i<faces_no;i++)
		{
			a=faces_list[i].a;
			b=faces_list[i].b;
			c=faces_list[i].c;

			if(j>255)j=0;
			noise_x=noise_array[j].u;
			noise_y=noise_array[j].v;
			j++;

			texture_id=get_texture_id(faces_list[i].material);


    		if(last_texture!=texture_id)
   			 	{
					glEnd();
					glBindTexture(GL_TEXTURE_2D, texture_id);
					glBegin(GL_TRIANGLES);
					last_texture=texture_id;
				}


			glNormal3fv(&vertex_list[a].nx);

			glTexCoord2f(faces_list[i].au+noise_x,faces_list[i].av+noise_y);
			glVertex3fv(&vertex_list[a].x);

			glNormal3fv(&vertex_list[b].nx);

			glTexCoord2f(faces_list[i].bu+noise_x,faces_list[i].bv+noise_y);
			glVertex3fv(&vertex_list[b].x);

			glNormal3fv(&vertex_list[c].nx);

			glTexCoord2f(faces_list[i].cu+noise_x,faces_list[i].cv+noise_y);
			glVertex3fv(&vertex_list[c].x);
		}


	glEnd();
	glPopMatrix();//restore the scene


  if(object_id->self_lit && (night_shadows_on || dungeon))glEnable(GL_LIGHTING);
  if(is_transparent)
  	{
  		glDisable(GL_ALPHA_TEST);
  		//glEnable(GL_CULL_FACE);
	}

}

//if there is any reflecting tile, returns 1, otherwise 0
int find_reflection()
{
	int x_start,x_end,y_start,y_end;
	int x,y;
	float x_scaled,y_scaled;

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


	x=-cx;
	y=-cy;

	glEnable(GL_CLIP_PLANE0);
	glClipPlane(GL_CLIP_PLANE0, water_clipping_p);
	glDisable(GL_CULL_FACE);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);

	set_material(0.1f,0.2f,0.3f);
	glPushMatrix();
	glScalef(1.0f, 1.0f, -1.0f);
	for(i=0;i<max_obj_3d;i++)
		{
			if(objects_list[i])
			     {
			         int dist1;
			         int dist2;
			         float dist;

					 if(!objects_list[i]->e3d_data->is_ground)
					 	{
			         		dist1=x-objects_list[i]->x_pos;
			         		dist2=y-objects_list[i]->y_pos;
			         		dist=dist1*dist1+dist2*dist2;
			         		if(dist<=21*21)
			         			{
									float x_len;
									float y_len;
									float z_len;
									float radius;

									z_len=objects_list[i]->e3d_data->max_z-objects_list[i]->e3d_data->min_z;
									x_len=objects_list[i]->e3d_data->max_x-objects_list[i]->e3d_data->min_x;
									y_len=objects_list[i]->e3d_data->max_y-objects_list[i]->e3d_data->min_y;
									//do some checks, to see if we really have to display this object
									if(x_len<5 && y_len<5 && z_len<4 && !find_local_reflection(objects_list[i]->x_pos,objects_list[i]->y_pos,1))continue;

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
	//glDisable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
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
	float x_step,y_step;
	float u_step,v_step;

	float u_noise_start;
	float u_noise_end;
	float v_noise_start;
	float v_noise_end;

	float uv_tile=50;

	x_step=3.0f/16.0f;
	y_step=3.0f/16.0f;

	u_step=3.0f/uv_tile;
	v_step=3.0f/uv_tile;

	glBegin(GL_QUADS);
	for(y=0;y<16;y++)
		{
			for(x=0;x<16;x++)
				{

					if(y==15 && x!=15)
					{
 					glTexCoord2f((x_pos+x*x_step)*u_step+noise_array[(y-15)*16+x].u+water_movement_u, (y_pos+y*y_step+y_step)*v_step+noise_array[(y-15)*16+x].v+water_movement_v);
	 				glVertex3f(x_pos+x*x_step,y_pos+y*y_step+y_step, water_deepth_offset);

					glTexCoord2f((x_pos+x*x_step)*u_step+noise_array[y*16+x].u+water_movement_u, (y_pos+y*y_step)*v_step+noise_array[y*16+x].v+water_movement_v);
					glVertex3f(x_pos+x*x_step,y_pos+y*y_step, water_deepth_offset);

					glTexCoord2f((x_pos+x*x_step+x_step)*u_step+noise_array[y*16+x+1].u+water_movement_u, (y_pos+y*y_step)*v_step+noise_array[y*16+x+1].v+water_movement_v);
					glVertex3f(x_pos+x*x_step+x_step, y_pos+y*y_step,water_deepth_offset);

					glTexCoord2f((x_pos+x*x_step+x_step)*u_step+noise_array[(y-15)*16+x+1].u+water_movement_u, (y_pos+y*y_step+y_step)*v_step+noise_array[(y-15)*16+x+1].v+water_movement_v);
					glVertex3f(x_pos+x*x_step+x_step, y_pos+y*y_step+y_step,water_deepth_offset);
					}
					else if(y!=15 && x==15)
					{
 					glTexCoord2f((x_pos+x*x_step)*u_step+noise_array[(y+1)*16+x].u+water_movement_u, (y_pos+y*y_step+y_step)*v_step+noise_array[(y+1)*16+x].v+water_movement_v);
	 				glVertex3f(x_pos+x*x_step,y_pos+y*y_step+y_step, water_deepth_offset);

					glTexCoord2f((x_pos+x*x_step)*u_step+noise_array[y*16+x].u+water_movement_u, (y_pos+y*y_step)*v_step+noise_array[y*16+x].v+water_movement_v);
					glVertex3f(x_pos+x*x_step,y_pos+y*y_step, water_deepth_offset);

					glTexCoord2f((x_pos+x*x_step+x_step)*u_step+noise_array[y*16+x-15].u+water_movement_u, (y_pos+y*y_step)*v_step+noise_array[y*16+x-15].v+water_movement_v);
					glVertex3f(x_pos+x*x_step+x_step, y_pos+y*y_step,water_deepth_offset);

					glTexCoord2f((x_pos+x*x_step+x_step)*u_step+noise_array[(y+1)*16+x-15].u+water_movement_u, (y_pos+y*y_step+y_step)*v_step+noise_array[(y+1)*16+x-15].v+water_movement_v);
					glVertex3f(x_pos+x*x_step+x_step, y_pos+y*y_step+y_step,water_deepth_offset);
					}
					else if(y==15 && x==15)
					{
 					glTexCoord2f((x_pos+x*x_step)*u_step+noise_array[(y-15)*16+x].u+water_movement_u, (y_pos+y*y_step+y_step)*v_step+noise_array[(y-15)*16+x].v+water_movement_v);
	 				glVertex3f(x_pos+x*x_step,y_pos+y*y_step+y_step, water_deepth_offset);

					glTexCoord2f((x_pos+x*x_step)*u_step+noise_array[y*16+x].u+water_movement_u, (y_pos+y*y_step)*v_step+noise_array[y*16+x].v+water_movement_v);
					glVertex3f(x_pos+x*x_step,y_pos+y*y_step, water_deepth_offset);

					glTexCoord2f((x_pos+x*x_step+x_step)*u_step+noise_array[y*16+x-15].u+water_movement_u, (y_pos+y*y_step)*v_step+noise_array[y*16+x-15].v+water_movement_v);
					glVertex3f(x_pos+x*x_step+x_step, y_pos+y*y_step,water_deepth_offset);

					glTexCoord2f((x_pos+x*x_step+x_step)*u_step+noise_array[(y-15)*16+x-15].u+water_movement_u, (y_pos+y*y_step+y_step)*v_step+noise_array[(y-15)*16+x-15].v+water_movement_v);
					glVertex3f(x_pos+x*x_step+x_step, y_pos+y*y_step+y_step,water_deepth_offset);
					}
					else
					{
 					glTexCoord2f((x_pos+x*x_step)*u_step+noise_array[(y+1)*16+x].u+water_movement_u, (y_pos+y*y_step+y_step)*v_step+noise_array[(y+1)*16+x].v+water_movement_v);
	 				glVertex3f(x_pos+x*x_step,y_pos+y*y_step+y_step, water_deepth_offset);

					glTexCoord2f((x_pos+x*x_step)*u_step+noise_array[y*16+x].u+water_movement_u, (y_pos+y*y_step)*v_step+noise_array[y*16+x].v+water_movement_v);
					glVertex3f(x_pos+x*x_step,y_pos+y*y_step, water_deepth_offset);

					glTexCoord2f((x_pos+x*x_step+x_step)*u_step+noise_array[y*16+x+1].u+water_movement_u, (y_pos+y*y_step)*v_step+noise_array[y*16+x+1].v+water_movement_v);
					glVertex3f(x_pos+x*x_step+x_step, y_pos+y*y_step,water_deepth_offset);

					glTexCoord2f((x_pos+x*x_step+x_step)*u_step+noise_array[(y+1)*16+x+1].u+water_movement_u, (y_pos+y*y_step+y_step)*v_step+noise_array[(y+1)*16+x+1].v+water_movement_v);
					glVertex3f(x_pos+x*x_step+x_step, y_pos+y*y_step+y_step,water_deepth_offset);
					}


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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);


	if(last_texture!=texture_cache[sky_text_1].texture_id)
		{
			glBindTexture(GL_TEXTURE_2D, texture_cache[sky_text_1].texture_id);
			last_texture=texture_cache[sky_text_1].texture_id;
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

	//glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
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
