#include <stdlib.h>
#include <string.h>
#include "global.h"
#include <math.h>

water_vertex noise_array[16*16];
int sky_text_1;
float water_deepth_offset=-0.25f;
int lake_waves_timer=0;
float water_movement_u=0;
float water_movement_v=0;
int show_reflection=1;

float mrandom(float max)
{
	return ((float) max * (rand () % 8 ));
}

void draw_actor_reflection(actor * actor_id)
{
	int i;
	double x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;
	int texture_id;
	char *cur_frame;

	check_gl_errors();
	if(!actor_id->remapped_colors)texture_id=get_texture_id(actor_id->texture_id);
	else
		{
			//we have remaped colors, we don't store such textures into the cache
			texture_id=actor_id->texture_id;
		}
	bind_texture_id(texture_id);

	cur_frame=actor_id->cur_frame;

	//now, go and find the current frame
	i= get_frame_number(actor_id->model_data, cur_frame);
	if(i<0)	return;	//nothing to draw

	glPushMatrix();//we don't want to affect the rest of the scene
	x_pos=actor_id->x_pos;
	y_pos=actor_id->y_pos;
	z_pos=actor_id->z_pos;

	if(z_pos==0.0f)//actor is walking, as opposed to flying, get the height underneath
		z_pos=-2.2f+height_map[actor_id->y_tile_pos*tile_map_size_x*6+actor_id->x_tile_pos]*0.2f;
	z_pos+=-water_deepth_offset*2;

	glTranslatef(x_pos+0.25f, y_pos+0.25f, z_pos);

	x_rot=actor_id->x_rot;
	y_rot=actor_id->y_rot;
	z_rot=actor_id->z_rot;
	z_rot=-z_rot;
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	draw_model(actor_id->model_data, cur_frame, actor_id->ghost);

	glPopMatrix();
	check_gl_errors();
}

void draw_enhanced_actor_reflection(actor * actor_id)
{
	double x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;
	int texture_id;
	char *cur_frame;

	check_gl_errors();
	cur_frame=actor_id->cur_frame;
	texture_id=actor_id->texture_id;

	bind_texture_id(texture_id);

	glPushMatrix();//we don't want to affect the rest of the scene
	x_pos=actor_id->x_pos;
	y_pos=actor_id->y_pos;
	z_pos=actor_id->z_pos;

	if(z_pos==0.0f)//actor is walking, as opposed to flying, get the height underneath
		z_pos=-2.2f+height_map[actor_id->y_tile_pos*tile_map_size_x*6+actor_id->x_tile_pos]*0.2f;
	z_pos+=-water_deepth_offset*2;

	glTranslatef(x_pos+0.25f, y_pos+0.25f, z_pos);
	x_rot=actor_id->x_rot;
	y_rot=actor_id->y_rot;
	z_rot=actor_id->z_rot;
	z_rot+=180;//test
	z_rot=-z_rot;
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	if(actor_id->body_parts->legs)draw_model(actor_id->body_parts->legs,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->torso)draw_model(actor_id->body_parts->torso,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->head)draw_model(actor_id->body_parts->head,cur_frame,actor_id->ghost);

	if(actor_id->body_parts->weapon)
		{
			int glow;
			draw_model(actor_id->body_parts->weapon,cur_frame,actor_id->ghost);
			glow=actor_id->body_parts->weapon_glow;
			if(glow!=GLOW_NONE)draw_model_halo(actor_id->body_parts->weapon,cur_frame,glow_colors[glow].r,glow_colors[glow].g,glow_colors[glow].b);
		}

	if(actor_id->body_parts->shield)draw_model(actor_id->body_parts->shield,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->helmet)draw_model(actor_id->body_parts->helmet,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->cape)draw_model(actor_id->body_parts->cape,cur_frame,actor_id->ghost);


	//////
	glPopMatrix();//restore the scene
	check_gl_errors();
}



void draw_3d_reflection(object3d * object_id)
{
	float x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;

	int materials_no;
	int i;

	e3d_array_vertex *array_vertex;
	e3d_array_normal *array_normal;
	e3d_array_uv_main *array_uv_main;
	e3d_array_order *array_order;

	int is_transparent;

	check_gl_errors();
	is_transparent=object_id->e3d_data->is_transparent;
	materials_no=object_id->e3d_data->materials_no;

	// check for having to load the arrays
	if(!object_id->e3d_data->array_vertex || !object_id->e3d_data->array_normal || !object_id->e3d_data->array_uv_main || !object_id->e3d_data->array_order)
		{
			load_e3d_detail(object_id->e3d_data);
		}
	array_vertex=object_id->e3d_data->array_vertex;
	array_normal=object_id->e3d_data->array_normal;
	array_uv_main=object_id->e3d_data->array_uv_main;
	array_order=object_id->e3d_data->array_order;

	if(object_id->self_lit && (!is_day || dungeon))
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
	//if(z_pos<0)z_pos+=-water_deepth_offset*2;
	z_pos+=-water_deepth_offset*2;

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
		if(array_order[i].count>0)
			{
				get_and_set_texture_id(array_order[i].texture_id);
				if(have_compiled_vertex_array)ELglLockArraysEXT(array_order[i].start, array_order[i].count);
				glDrawArrays(GL_TRIANGLES,array_order[i].start,array_order[i].count);
				if(have_compiled_vertex_array)ELglUnlockArraysEXT();
			}
	check_gl_errors();
	glPopMatrix();//restore the scene
	check_gl_errors();


	if(object_id->self_lit && (!is_day || dungeon))glEnable(GL_LIGHTING);
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
	if(cx<0)x=(cx*-1)/3;
	else x=cx/3;
	if(cy<0)y=(cy*-1)/3;
	else y=cy/3;
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
	float window_ratio;

	window_ratio=(GLfloat)window_width/(GLfloat)window_height;

	check_gl_errors();
	x=-cx;
	y=-cy;

	glEnable(GL_CLIP_PLANE0);
	glClipPlane(GL_CLIP_PLANE0, water_clipping_p);

	glDisable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	set_material(0.1f,0.2f,0.3f);
	glPushMatrix();
	glScalef(1.0f, 1.0f, -1.0f);
	//first, render only the submerged objects, with the clipping plane enabled
	for(i=0;i<highest_obj_3d;i++)
		{
			if(objects_list[i])
				{
					if(!objects_list[i]->e3d_data->is_ground)
					 	{
							int dist1;
							int dist2;

			         		dist1=x-objects_list[i]->x_pos;
			         		dist2=y-objects_list[i]->y_pos;
			         		if(dist1*dist1+dist2*dist2<=21*21)
			         			{
									float x_len, y_len, z_len;
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

	glDisableClientState(GL_NORMAL_ARRAY);
	glNormal3f(0.0f,0.0f,1.0f);
	for(i=0;i<max_actors;i++)
		{
			if(actors_list[i])
			if(!actors_list[i]->ghost)
				{
					int dist1;
					int dist2;

					dist1=x-actors_list[i]->x_pos;
					dist2=y-actors_list[i]->y_pos;
					if(dist1*dist1+dist2*dist2<=100)
							{
								if(actors_list[i]->is_enhanced_model)
									draw_enhanced_actor_reflection(actors_list[i]);
								else draw_actor_reflection(actors_list[i]);
							}
				}
		}
	glPopMatrix();
	reset_material();

	glDisable(GL_CLIP_PLANE0);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	check_gl_errors();
}

void make_lake_water_noise()
{
	int x,y;
	float noise_u,noise_v,noise_z;

	for(x=0;x<16;x++)
		for(y=0;y<16;y++)
			{

				noise_u=mrandom(0.001f);
				noise_v=mrandom(0.001f);
				noise_z=mrandom(0.005f);
				if(noise_z<=0)noise_z=-noise_z;

				noise_array[y*16+x].u=noise_u;
				noise_array[y*16+x].v=noise_v;
				noise_array[y*16+x].z=noise_z;

			}

}

void draw_lake_water_tile(float x_pos, float y_pos)
{
	int x,y;
	float fx,fy;
	float x_step,y_step;
	float u_step,v_step;
	float uv_tile=1.0f/50.0f;

	x_step=3.0f/16.0f;
	y_step=3.0f/16.0f;

	u_step=3.0f*uv_tile;
	v_step=3.0f*uv_tile;

	glBegin(GL_TRIANGLE_STRIP);
	if(have_multitexture)
		for(y=0,fy=y_pos;y<16;fy+=y_step,y++)
			{
				for(x=0,fx=x_pos;x<17;fx+=x_step,x++)
					{
						ELglMultiTexCoord2fARB(base_unit,fx*u_step+noise_array[((y+1)&15)*16+(x&15)].u+water_movement_u, (fy+y_step)*v_step+noise_array[((y+1)&15)*16+(x&15)].v+water_movement_v);
						glVertex3f(fx,fy+y_step, water_deepth_offset);

						ELglMultiTexCoord2fARB(base_unit,fx*u_step+noise_array[y*16+(x&15)].u+water_movement_u, fy*v_step+noise_array[y*16+(x&15)].v+water_movement_v);
						glVertex3f(fx,fy, water_deepth_offset);
					}
			}
	else
		for(y=0,fy=y_pos;y<16;fy+=y_step,y++)
			{
				for(x=0,fx=x_pos;x<17;fx+=x_step,x++)
					{
						glTexCoord2f(fx*u_step+noise_array[((y+1)&15)*16+(x&15)].u+water_movement_u, (fy+y_step)*v_step+noise_array[((y+1)&15)*16+(x&15)].v+water_movement_v);
						glVertex3f(fx,fy+y_step, water_deepth_offset);

						glTexCoord2f(fx*u_step+noise_array[y*16+(x&15)].u+water_movement_u, fy*v_step+noise_array[y*16+(x&15)].v+water_movement_v);
						glVertex3f(fx,fy, water_deepth_offset);
					}
			}
	glEnd();
}


void draw_lake_tiles()
{
	int x_start,x_end,y_start,y_end;
	int x,y;
	float x_scaled,y_scaled;

	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	bind_texture_id(get_texture_id(sky_text_1));

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
	for(y=y_start;y<=y_end;y++)
		{
			int actualy=y;
			if(actualy<0)actualy=0;
			else if(actualy>=tile_map_size_y)actualy=tile_map_size_y-1;
			actualy*=tile_map_size_x;
			y_scaled=y*3.0f;
			for(x=x_start;x<=x_end;x++)
				{
					int actualx=x;
					if(actualx<0)actualx=0;
					else if(actualx>=tile_map_size_x)actualx=tile_map_size_x-1;
					x_scaled=x*3.0f;
					if(!tile_map[actualy+actualx] && check_tile_in_frustrum(x_scaled,y_scaled))draw_lake_water_tile(x_scaled,y_scaled);
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
#define scale_factor 100

	glColor3f(sky_lights_c1[light_level][0]-(float)weather_light_offset/scale_factor,
			  sky_lights_c1[light_level][1]-(float)weather_light_offset/scale_factor,
			  sky_lights_c1[light_level][2]-(float)weather_light_offset/scale_factor);
	glVertex3i(0,0,0);

	glColor3f(sky_lights_c2[light_level][0]-(float)weather_light_offset/scale_factor,
			  sky_lights_c2[light_level][1]-(float)weather_light_offset/scale_factor,
			  sky_lights_c2[light_level][2]-(float)weather_light_offset/scale_factor);
	glVertex3i(0,window_height,0);

	glColor3f(sky_lights_c3[light_level][0]-(float)weather_light_offset/scale_factor,
			  sky_lights_c3[light_level][1]-(float)weather_light_offset/scale_factor,
			  sky_lights_c3[light_level][2]-(float)weather_light_offset/scale_factor);
	glVertex3i(window_width,window_height,0);

	glColor3f(sky_lights_c4[light_level][0]-(float)weather_light_offset/scale_factor,
			  sky_lights_c4[light_level][1]-(float)weather_light_offset/scale_factor,
			  sky_lights_c4[light_level][2]-(float)weather_light_offset/scale_factor);
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



