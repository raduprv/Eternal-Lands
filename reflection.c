#include <stdlib.h>
#include <string.h>
#include "global.h"
#include <math.h>

typedef struct
{
	float u;
	float v;
	float z;

}water_vertex;

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
	double x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;
	int texture_id;
	char *cur_frame;

	CHECK_GL_ERRORS();
	if(!actor_id->remapped_colors)texture_id=get_texture_id(actor_id->texture_id);
	else
		{
			//we have remaped colors, we don't store such textures into the cache
			texture_id=actor_id->texture_id;
		}
	bind_texture_id(texture_id);

	cur_frame=actor_id->tmp.cur_frame;

	//now, go and find the current frame

	glPushMatrix();//we don't want to affect the rest of the scene
	
	x_pos=actor_id->tmp.x_pos;
	y_pos=actor_id->tmp.y_pos;
	z_pos=actor_id->tmp.z_pos;

	if(z_pos==0.0f)//actor is walking, as opposed to flying, get the height underneath
		z_pos=-2.2f+height_map[actor_id->tmp.y_tile_pos*tile_map_size_x*6+actor_id->tmp.x_tile_pos]*0.2f;
	z_pos+=-water_deepth_offset*2;

	glTranslatef(x_pos+0.25f, y_pos+0.25f, z_pos);

	x_rot=actor_id->tmp.x_rot;
	y_rot=actor_id->tmp.y_rot;
	z_rot=actor_id->tmp.z_rot;
	
	z_rot=-z_rot;
	z_rot+=180;
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	if (actors_defs[actor_id->actor_type].coremodel!=NULL) cal_render_actor(actor_id);

	glPopMatrix();
	CHECK_GL_ERRORS();
}

void draw_enhanced_actor_reflection(actor * actor_id)
{
	double x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;
	int texture_id;
	char *cur_frame;

	CHECK_GL_ERRORS();
	
	cur_frame=actor_id->tmp.cur_frame;
	
	texture_id=actor_id->texture_id;

	bind_texture_id(texture_id);

	glPushMatrix();//we don't want to affect the rest of the scene
	
	x_pos=actor_id->tmp.x_pos;
	y_pos=actor_id->tmp.y_pos;
	z_pos=actor_id->tmp.z_pos;

	if(z_pos==0.0f)//actor is walking, as opposed to flying, get the height underneath
		z_pos=-2.2f+height_map[actor_id->tmp.y_tile_pos*tile_map_size_x*6+actor_id->tmp.x_tile_pos]*0.2f;
	z_pos+=-water_deepth_offset*2;

	glTranslatef(x_pos+0.25f, y_pos+0.25f, z_pos);
	
	x_rot=actor_id->tmp.x_rot;
	y_rot=actor_id->tmp.y_rot;
	z_rot=actor_id->tmp.z_rot;
	
	z_rot+=180;//test
	z_rot=-z_rot;
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	if (actors_defs[actor_id->actor_type].coremodel!=NULL) {
		cal_render_actor(actor_id);
	}

	glPopMatrix();//restore the scene
	CHECK_GL_ERRORS();
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

	CHECK_GL_ERRORS();
	is_transparent=object_id->e3d_data->is_transparent;
	materials_no=object_id->e3d_data->materials_no;

	cache_use(cache_e3d, object_id->e3d_data->cache_ptr);
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


	CHECK_GL_ERRORS();
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

	CHECK_GL_ERRORS();

	if(have_vertex_buffers && object_id->e3d_data->vbo[0] && object_id->e3d_data->vbo[1] && object_id->e3d_data->vbo[2]){
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, object_id->e3d_data->vbo[0]);
		glTexCoordPointer(2,GL_FLOAT,0,0);
		
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, object_id->e3d_data->vbo[1]);
		glNormalPointer(GL_FLOAT,0,0);
		
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, object_id->e3d_data->vbo[2]);
		glVertexPointer(3,GL_FLOAT,0,0);
	} else {
		glVertexPointer(3,GL_FLOAT,0,array_vertex);
		glTexCoordPointer(2,GL_FLOAT,0,array_uv_main);
		glNormalPointer(GL_FLOAT,0,array_normal);	
	}
	
	if(have_compiled_vertex_array)ELglLockArraysEXT(0, object_id->e3d_data->face_no);
	for(i=0;i<materials_no;i++)
		if(array_order[i].count>0)
			{
				get_and_set_texture_id(array_order[i].texture_id);
				glDrawArrays(GL_TRIANGLES,array_order[i].start,array_order[i].count);
			}
	if(have_compiled_vertex_array)ELglUnlockArraysEXT();
	CHECK_GL_ERRORS();
	glPopMatrix();//restore the scene
	CHECK_GL_ERRORS();

	if(object_id->self_lit && (!is_day || dungeon))glEnable(GL_LIGHTING);
	if(is_transparent)
		{
			glDisable(GL_ALPHA_TEST);
		}

	if(have_vertex_buffers){
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
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
					if(IS_WATER_TILE(tile_map[y*tile_map_size_x+x]))
						{
							if(IS_REFLECTING(tile_map[y*tile_map_size_x+x])) return 2;
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
					if(IS_WATER_TILE(tile_map[y*tile_map_size_x+x]))
						{
							if(IS_REFLECTING(tile_map[y*tile_map_size_x+x])) return 2;
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
	float window_ratio;
	struct near_3d_object * nobj;
	
	window_ratio=(GLfloat)window_width/(GLfloat)window_height;

	if(regenerate_near_objects)if(!get_near_3d_objects())return;
	
	x=-cx;
	y=-cy;
	
	CHECK_GL_ERRORS();

	glEnable(GL_CLIP_PLANE0);
	glClipPlane(GL_CLIP_PLANE0, water_clipping_p);

	glDisable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	set_material(0.1f,0.2f,0.3f);
	glPushMatrix();
	glScalef(1.0f, 1.0f, -1.0f);

	for(nobj=first_near_3d_object;nobj;nobj=nobj->next){
        	if(!objects_list[nobj->pos])
			regenerate_near_objects=1;
		else if(!objects_list[nobj->pos]->e3d_data->is_ground && nobj->dist<=442)
       	 		draw_3d_object(objects_list[nobj->pos]);
	}

	glDisableClientState(GL_NORMAL_ARRAY);
	glNormal3f(0.0f,0.0f,1.0f);
	for(i=0;i<no_near_actors;i++) {
		if(near_actors[i].dist<=100 && !near_actors[i].ghost){ 
			actor * act=actors_list[near_actors[i].actor];

			if(act){
				if(act->is_enhanced_model)
					draw_enhanced_actor_reflection(act);
				else 
					draw_actor_reflection(act);
			}
		}
	}
	glPopMatrix();
	reset_material();

	glDisable(GL_CLIP_PLANE0);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_CULL_FACE);
	CHECK_GL_ERRORS();
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
					if(IS_WATER_TILE(tile_map[actualy+actualx]) && check_tile_in_frustrum(x_scaled,y_scaled))
						{
							if(!tile_map[actualy+actualx])
								{
									if(dungeon)
										get_and_set_texture_id(tile_list[231]);
									else
										get_and_set_texture_id(tile_list[0]);
								}
							else
								get_and_set_texture_id(tile_list[tile_map[actualx+actualy]]);
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
#define SCALE_FACTOR 100

	glColor3f(sky_lights_c1[light_level][0]-(float)weather_light_offset/SCALE_FACTOR,
			  sky_lights_c1[light_level][1]-(float)weather_light_offset/SCALE_FACTOR,
			  sky_lights_c1[light_level][2]-(float)weather_light_offset/SCALE_FACTOR);
	glVertex3i(0,0,0);

	glColor3f(sky_lights_c2[light_level][0]-(float)weather_light_offset/SCALE_FACTOR,
			  sky_lights_c2[light_level][1]-(float)weather_light_offset/SCALE_FACTOR,
			  sky_lights_c2[light_level][2]-(float)weather_light_offset/SCALE_FACTOR);
	glVertex3i(0,window_height,0);

	glColor3f(sky_lights_c3[light_level][0]-(float)weather_light_offset/SCALE_FACTOR,
			  sky_lights_c3[light_level][1]-(float)weather_light_offset/SCALE_FACTOR,
			  sky_lights_c3[light_level][2]-(float)weather_light_offset/SCALE_FACTOR);
	glVertex3i(window_width,window_height,0);

	glColor3f(sky_lights_c4[light_level][0]-(float)weather_light_offset/SCALE_FACTOR,
			  sky_lights_c4[light_level][1]-(float)weather_light_offset/SCALE_FACTOR,
			  sky_lights_c4[light_level][2]-(float)weather_light_offset/SCALE_FACTOR);
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
