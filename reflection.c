#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef MAP_EDITOR2
#include "../map_editor2/global.h"
#else
#include "global.h"
#include "tiles.h"
#include "textures.h"
#endif

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
GLuint water_reflection_fbo = 0;
GLuint water_reflection_fbo_renderbuffer = 0;
GLuint water_reflection_fbo_texture = 0;
int reflection_texture_width = 0;
int reflection_texture_height = 0;
double projectionlMatrixd[16];
double modelMatrixd[16];

float mrandom(float max)
{
	return ((float) max * (rand () % 8 ));
}

#ifndef MAP_EDITOR2
void draw_actor_reflection(actor * actor_id)
{
	double x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;
	int texture_id;

	CHECK_GL_ERRORS();
	if(!actor_id->remapped_colors)texture_id=get_texture_id(actor_id->texture_id);
	else
		{
			//we have remaped colors, we don't store such textures into the cache
			texture_id=actor_id->texture_id;
		}
	bind_texture_id(texture_id);

	//now, go and find the current frame

	glPushMatrix();//we don't want to affect the rest of the scene
	
	x_pos=actor_id->tmp.x_pos;
	y_pos=actor_id->tmp.y_pos;
	z_pos=actor_id->tmp.z_pos;

	if(z_pos==0.0f)//actor is walking, as opposed to flying, get the height underneath
		z_pos=-2.2f+height_map[actor_id->tmp.y_tile_pos*tile_map_size_x*6+actor_id->tmp.x_tile_pos]*0.2f;

	glTranslatef(x_pos+0.25f, y_pos+0.25f, z_pos);

	x_rot=actor_id->tmp.x_rot;
	y_rot=actor_id->tmp.y_rot;
	z_rot=actor_id->tmp.z_rot;
	
	z_rot=-z_rot;
	z_rot+=180;
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	if (actor_id->calmodel!=NULL) cal_render_actor(actor_id);

	glPopMatrix();
	CHECK_GL_ERRORS();
}

void draw_enhanced_actor_reflection(actor * actor_id)
{
	double x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;
	int texture_id;

	CHECK_GL_ERRORS();
	
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
	z_rot=-actor_id->tmp.z_rot;
	z_rot+=180;	//test
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	if (actor_id->calmodel!=NULL) {
		cal_render_actor(actor_id);
	}

	glPopMatrix();	//restore the scene
	CHECK_GL_ERRORS();
}
#endif


#ifndef NEW_FRUSTUM
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

	if(!object_id->display) return;	// not currently on the map, ignore it
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

#ifdef NEW_LIGHTING
	if (
	    ( (use_new_lighting) && (object_id->self_lit && (!(game_minute >= 5 && game_minute < 235) || dungeon))) ||
	    ((!use_new_lighting) && (object_id->self_lit && (!is_day || dungeon)))
	   )
#else
	if (object_id->self_lit && (!is_day || dungeon)) 
#endif
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

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	if(have_vertex_buffers && object_id->e3d_data->vbo[0] && object_id->e3d_data->vbo[1] && object_id->e3d_data->vbo[2]){
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, object_id->e3d_data->vbo[0]);
		glTexCoordPointer(2,GL_FLOAT,0,0);
		
		if(!object_id->e3d_data->is_ground){
			ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, object_id->e3d_data->vbo[1]);
			glEnableClientState(GL_NORMAL_ARRAY);
			glNormalPointer(GL_FLOAT,0,0);
		}
		
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, object_id->e3d_data->vbo[2]);
		glVertexPointer(3,GL_FLOAT,0,0);
	} else {
		glVertexPointer(3,GL_FLOAT,0,array_vertex);
		glTexCoordPointer(2,GL_FLOAT,0,array_uv_main);
		if(!object_id->e3d_data->is_ground)
		{
			glEnableClientState(GL_NORMAL_ARRAY);
			glNormalPointer(GL_FLOAT,0,array_normal);	
		}
	}
	
	if(use_compiled_vertex_array)ELglLockArraysEXT(0, object_id->e3d_data->face_no);
	for(i=0;i<materials_no;i++)
		if(array_order[i].count>0)
			{
				int	idx, max;
				get_and_set_texture_id(array_order[i].texture_id);
				// ATI bug fix for large arrays
				idx= array_order[i].start;
				max= array_order[i].start+array_order[i].count;
				while(idx < max) {
		    		int num;
		   
					num= max-idx;
					if(num > 3000){
						num= 3000;
					}
		    		glDrawArrays(GL_TRIANGLES, idx, num);
		    		idx+= num;
				}
			}
	if(use_compiled_vertex_array)ELglUnlockArraysEXT();
	CHECK_GL_ERRORS();
	glPopMatrix();//restore the scene
	CHECK_GL_ERRORS();

#ifdef NEW_LIGHTING
	if (
	    ( (use_new_lighting) && (object_id->self_lit && (!(game_minute >= 5 && game_minute < 235) || dungeon))) ||
	    ((!use_new_lighting) && (object_id->self_lit && (!is_day || dungeon)))
	   )
#else
	if (object_id->self_lit && (!is_day || dungeon)) 
#endif
		glEnable(GL_LIGHTING);
	if(is_transparent)
		{
			glDisable(GL_ALPHA_TEST);
		}

	if(have_vertex_buffers){
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	}

	CHECK_GL_ERRORS();
}
#endif  //NEW_FRUSTUM

//if there is any reflecting tile, returns 1, otherwise 0
int find_reflection()
{
#ifndef	NEW_FRUSTUM
	int x_start,x_end,y_start,y_end;
	int x,y;
	float x_scaled,y_scaled;
	int found_water=0;

	//get only the tiles around the camera
	//we have the axes inverted, btw the go from 0 to -255
	if(camera_x<0)x=(camera_x*-1)/3;
	else x=camera_x/3;
	if(camera_y<0)y=(camera_y*-1)/3;
	else y=camera_y/3;
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
#else
	unsigned int idx;
	
	idx = main_bbox_tree->cur_intersect_type;
	if (main_bbox_tree->intersect[idx].start[TYPE_REFLECTIV_WATER] < main_bbox_tree->intersect[idx].stop[TYPE_REFLECTIV_WATER]) return 2;
	else 
	{
		if (main_bbox_tree->intersect[idx].start[TYPE_NO_REFLECTIV_WATER] < main_bbox_tree->intersect[idx].stop[TYPE_NO_REFLECTIV_WATER]) return 1;
		else return 0;
	}
#endif
}

#ifndef NEW_FRUSTUM
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
#endif  //NEW_FRUSTUM

static __inline__ int adapt_size(int size)
{
	int i, j;
	
	glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE_EXT, &i);

	size = min2i(size, i);
	
	if (have_texture_non_power_of_two) return size;
	{
		j = 1;
		while (j < size) j += j;
		return min2i(j, i);
	}
}

void free_reflection_framebuffer()
{
	free_color_framebuffer(&water_reflection_fbo, &water_reflection_fbo_renderbuffer, 
			&water_reflection_fbo_texture);
}

void make_reflection_framebuffer(int width, int height)
{
	reflection_texture_width = adapt_size(width);
	reflection_texture_height = adapt_size(height);
	free_reflection_framebuffer();
	make_color_framebuffer(reflection_texture_width, reflection_texture_height, &water_reflection_fbo, 
			&water_reflection_fbo_renderbuffer, &water_reflection_fbo_texture);
}

void change_reflection_framebuffer_size(int width, int height)
{
	reflection_texture_width = adapt_size(width);
	reflection_texture_height = adapt_size(height);
	change_color_framebuffer_size(reflection_texture_width, reflection_texture_height, &water_reflection_fbo, 
			&water_reflection_fbo_renderbuffer, &water_reflection_fbo_texture);
}

static __inline__ void init_texturing()
{
	glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrixd);
	glGetDoublev(GL_PROJECTION_MATRIX, projectionlMatrixd);
}

/* Calculate the texture coordinates for a vertex and send them to OpenGL */
static __inline__ void RenderVertex(GLdouble vX,GLdouble vY, GLdouble vZ)
{
	GLdouble tX, tY, tZ;
	/* Dummy viewport */
	int vp[4] = {0, 0, 1, 1};
	
	/* Calculate the window coordinates of the vertex */
	gluProject(vX, vY, vZ, modelMatrixd, projectionlMatrixd, vp, &tX, &tY, &tZ);

	/* Use the window coords as texture coords */
	ELglMultiTexCoord2fARB(detail_unit, tX, tY);
}

static __inline__ void draw_lake_water_tile_framebuffer(float x_pos, float y_pos)
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
	for(y=0,fy=y_pos;y<16;fy+=y_step,y++)
	{
		for(x=0,fx=x_pos;x<17;fx+=x_step,x++)
		{
			ELglMultiTexCoord2fARB(base_unit,fx*u_step+noise_array[((y+1)&15)*16+(x&15)].u+water_movement_u, 
				(fy+y_step)*v_step+noise_array[((y+1)&15)*16+(x&15)].v+water_movement_v);
			RenderVertex(fx, fy+y_step, water_deepth_offset);
			glVertex3f(fx, fy+y_step, water_deepth_offset);

			ELglMultiTexCoord2fARB(base_unit,fx*u_step+noise_array[y*16+(x&15)].u+water_movement_u, 
				fy*v_step+noise_array[y*16+(x&15)].v+water_movement_v);
			RenderVertex(fx, fy, water_deepth_offset);
			glVertex3f(fx, fy, water_deepth_offset);
		}
	}
	glEnd();
}

#ifdef NEW_FRUSTUM
static __inline__ void init_depth()
{
	float x, y, x_scaled, y_scaled;
	int i, l;
	unsigned int start, stop;

	glDepthFunc(GL_ALWAYS);
	glDepthRange(1.0f, 1.0f);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	/* Now drawing the water */
	get_intersect_start_stop(main_bbox_tree, TYPE_REFLECTIV_WATER, &start, &stop);
	
	glBegin(GL_QUADS);
	for (i = start; i < stop; i++)
	{
		l = get_intersect_item_ID(main_bbox_tree, i);
		x = get_terrain_x(l);
		y = get_terrain_y(l);
		y_scaled = y*3.0f;
		x_scaled = x*3.0f;

		glVertex3f(x_scaled,        y_scaled + 3.0f, 0.0f);
		glVertex3f(x_scaled,        y_scaled,        0.0f);
		glVertex3f(x_scaled + 3.0f, y_scaled,        0.0f);
		glVertex3f(x_scaled + 3.0f, y_scaled + 3.0f, 0.0f);
	}
	glEnd();

	/* Re-enable update of color and depth. */
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthRange(0.0f, 1.0f);
	glDepthFunc(GL_LESS);
}
#endif // NEW_FRUSTUM

void display_3d_reflection()
{
#ifndef	NEW_FRUSTUM
	double water_clipping_p[4]={0.0, 0.0, -1.0, water_deepth_offset};
#endif	//NEW_FRUSTUM
	int view_port[4];
#ifdef	NEW_FRUSTUM
	unsigned int cur_intersect_type;
#endif	//NEW_FRUSTUM
#ifndef NEW_FRUSTUM
	if (regenerate_near_objects)
	{
		if (!get_near_3d_objects()) return;
	}
#endif	//NEW_FRUSTUM
	if (use_frame_buffer)
	{
		glGetIntegerv(GL_VIEWPORT, view_port);
		ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, water_reflection_fbo);
		glViewport(0, 0, reflection_texture_width, reflection_texture_height);
	}
	
	glCullFace(GL_FRONT);
#ifndef	NEW_FRUSTUM
	glEnable(GL_CLIP_PLANE0);
	glClipPlane(GL_CLIP_PLANE0, water_clipping_p);
#endif	//NEW_FRUSTUM
	glPushMatrix();	

	glTranslatef(0.0f, 0.0f, water_deepth_offset);
	glScalef(1.0f, 1.0f, -1.0f);
	glTranslatef(0.0f, 0.0f, -water_deepth_offset);
	glNormal3f(0.0f, 0.0f, 1.0f);
#ifdef	NEW_FRUSTUM
	init_depth();
	cur_intersect_type = get_cur_intersect_type(main_bbox_tree);
	set_cur_intersect_type(main_bbox_tree, INTERSECTION_TYPE_REFLECTION);
	calculate_reflection_frustum(water_deepth_offset);
	enable_reflection_clip_planes();
#endif	//NEW_FRUSTUM

//	draw_tile_map();
//	display_2d_objects();
	display_objects();
	display_ground_objects();
#ifndef MAP_EDITOR2
	display_actors(0, 1);
#endif
	display_alpha_objects();
//	display_blended_objects();
#ifdef NEW_FRUSTUM
	set_cur_intersect_type(main_bbox_tree, cur_intersect_type);
#endif

	glPopMatrix();
#ifndef	NEW_FRUSTUM
	glDisable(GL_CLIP_PLANE0);
#else	//NEW_FRUSTUM
	disable_reflection_clip_planes();
#endif	//NEW_FRUSTUM
	glCullFace(GL_BACK);
	CHECK_GL_ERRORS();
	reset_material();

	if (use_frame_buffer)
	{
		ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glViewport(view_port[0], view_port[1], view_port[2], view_port[3]);
	}
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
						ELglMultiTexCoord2fARB(base_unit,fx*u_step+noise_array[((y+1)&15)*16+(x&15)].u+water_movement_u, 
							(fy+y_step)*v_step+noise_array[((y+1)&15)*16+(x&15)].v+water_movement_v);
						glVertex3f(fx, fy+y_step, water_deepth_offset);

						ELglMultiTexCoord2fARB(base_unit,fx*u_step+noise_array[y*16+(x&15)].u+water_movement_u, 
							fy*v_step+noise_array[y*16+(x&15)].v+water_movement_v);
						glVertex3f(fx, fy, water_deepth_offset);

					}
			}
	else
		for(y=0,fy=y_pos;y<16;fy+=y_step,y++)
			{
				for(x=0,fx=x_pos;x<17;fx+=x_step,x++)
					{
						glTexCoord2f(fx*u_step+noise_array[((y+1)&15)*16+(x&15)].u+water_movement_u,
							(fy+y_step)*v_step+noise_array[((y+1)&15)*16+(x&15)].v+water_movement_v);
						glVertex3f(fx,fy+y_step, water_deepth_offset);

						glTexCoord2f(fx*u_step+noise_array[y*16+(x&15)].u+water_movement_u,
							fy*v_step+noise_array[y*16+(x&15)].v+water_movement_v);
						glVertex3f(fx,fy, water_deepth_offset);
					}
			}
	glEnd();
}


#ifndef MAP_EDITOR2
void blend_reflection_fog()
{
#ifndef	NEW_FRUSTUM
	int x_start,x_end,y_start,y_end;
	int x,y;
#else
	unsigned int i, l, x, y, start, stop;
#endif
	float x_scaled,y_scaled;
	static GLfloat blendColor[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
#ifdef NEW_WEATHER
	GLfloat fogColor[4];
	
	glGetFloatv(GL_FOG_COLOR, fogColor);
#endif

	// we write to the depth buffer later, for now keep it as it is to avoid flickering
	glDepthMask(GL_FALSE);

	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glColor3f(0.0f, 0.0f, 0.0f); 

#ifndef	NEW_FRUSTUM
	//get only the tiles around the camera
	//we have the axes inverted, btw the go from 0 to -255
	if(camera_x<0)x=(camera_x*-1)/3;
	else x=camera_x/3;
	if(camera_y<0)y=(camera_y*-1)/3;
	else y=camera_y/3;
	x_start = (int)x - 8;
	y_start = (int)y - 8;
	x_end   = (int)x + 8;
	y_end   = (int)y + 8;
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
							// scale the colors down to what the fog lets through
							glFogfv(GL_FOG_COLOR, blendColor);
							glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
							glBegin(GL_QUADS);
								glVertex3f(x_scaled       , y_scaled       , water_deepth_offset);
								glVertex3f(x_scaled + 3.0f, y_scaled       , water_deepth_offset);
								glVertex3f(x_scaled + 3.0f, y_scaled + 3.0f, water_deepth_offset);
								glVertex3f(x_scaled       , y_scaled + 3.0f, water_deepth_offset);
							glEnd();

							// now add the fog by additive blending
							glFogfv(GL_FOG_COLOR, fogColor);
							glBlendFunc(GL_ONE, GL_ONE);
							glBegin(GL_QUADS);
								glVertex3f(x_scaled       , y_scaled       , water_deepth_offset);
								glVertex3f(x_scaled + 3.0f, y_scaled       , water_deepth_offset);
								glVertex3f(x_scaled + 3.0f, y_scaled + 3.0f, water_deepth_offset);
								glVertex3f(x_scaled       , y_scaled + 3.0f, water_deepth_offset);
							glEnd();
						}
				}
		}
	
#else
	glFogfv(GL_FOG_COLOR, blendColor);
	glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
	
	get_intersect_start_stop(main_bbox_tree, TYPE_NO_REFLECTIV_WATER, &start, &stop);
	
	for (i = start; i < stop; i++)
	{
		l = get_intersect_item_ID(main_bbox_tree, i);
		x = get_terrain_x(l);
		y = get_terrain_y(l);
		y_scaled = y*3.0f;
		x_scaled = x*3.0f;
		glBegin(GL_QUADS);
			glVertex3f(x_scaled       , y_scaled       , water_deepth_offset);
			glVertex3f(x_scaled + 3.0f, y_scaled       , water_deepth_offset);
			glVertex3f(x_scaled + 3.0f, y_scaled + 3.0f, water_deepth_offset);
			glVertex3f(x_scaled       , y_scaled + 3.0f, water_deepth_offset);
		glEnd();
	}
	
	get_intersect_start_stop(main_bbox_tree, TYPE_REFLECTIV_WATER, &start, &stop);
	
	for (i = start; i < stop; i++)
	{
		l = get_intersect_item_ID(main_bbox_tree, i);
		x = get_terrain_x(l);
		y = get_terrain_y(l);
		y_scaled = y*3.0f;
		x_scaled = x*3.0f;
		glBegin(GL_QUADS);
			glVertex3f(x_scaled       , y_scaled       , water_deepth_offset);
			glVertex3f(x_scaled + 3.0f, y_scaled       , water_deepth_offset);
			glVertex3f(x_scaled + 3.0f, y_scaled + 3.0f, water_deepth_offset);
			glVertex3f(x_scaled       , y_scaled + 3.0f, water_deepth_offset);
		glEnd();
	}
	
	// now add the fog by additive blending
	glFogfv(GL_FOG_COLOR, fogColor);
	glBlendFunc(GL_ONE, GL_ONE);
	
	get_intersect_start_stop(main_bbox_tree, TYPE_NO_REFLECTIV_WATER, &start, &stop);

	for (i = start; i < stop; i++)
	{
		l = get_intersect_item_ID(main_bbox_tree, i);
		x = get_terrain_x(l);
		y = get_terrain_y(l);
		y_scaled = y*3.0f;
		x_scaled = x*3.0f;
		glBegin(GL_QUADS);
			glVertex3f(x_scaled       , y_scaled       , water_deepth_offset);
			glVertex3f(x_scaled + 3.0f, y_scaled       , water_deepth_offset);
			glVertex3f(x_scaled + 3.0f, y_scaled + 3.0f, water_deepth_offset);
			glVertex3f(x_scaled       , y_scaled + 3.0f, water_deepth_offset);
		glEnd();
	}
	
	get_intersect_start_stop(main_bbox_tree, TYPE_REFLECTIV_WATER, &start, &stop);
	
	for (i = start; i < stop; i++)
	{
		l = get_intersect_item_ID(main_bbox_tree, i);
		x = get_terrain_x(l);
		y = get_terrain_y(l);
		y_scaled = y*3.0f;
		x_scaled = x*3.0f;
		glBegin(GL_QUADS);
			glVertex3f(x_scaled       , y_scaled       , water_deepth_offset);
			glVertex3f(x_scaled + 3.0f, y_scaled       , water_deepth_offset);
			glVertex3f(x_scaled + 3.0f, y_scaled + 3.0f, water_deepth_offset);
			glVertex3f(x_scaled       , y_scaled + 3.0f, water_deepth_offset);
		glEnd();
	}
#endif
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	// ok, now we can write depth values
	glDepthMask(GL_TRUE);
}
#endif

void draw_lake_tiles()
{
#ifdef	NEW_FRUSTUM
	unsigned int i, l, start, stop;
#else
	int x_start,x_end,y_start,y_end;
#endif
	int x,y;
	float x_scaled,y_scaled;
	int water_id;
#ifdef NEW_FRUSTUM
	float blend_float = 0.75f;
	float blend_vec[4] = {blend_float, blend_float, blend_float, blend_float};
#endif
	glEnable(GL_CULL_FACE);
	
#ifndef	NEW_FRUSTUM
	//get only the tiles around the camera
	//we have the axes inverted, btw the go from 0 to -255
	if(camera_x<0)x=(camera_x*-1)/3;
	else x=camera_x/3;
	if(camera_y<0)y=(camera_y*-1)/3;
	else y=camera_y/3;
	x_start = (int)x - 8;
	y_start = (int)y - 8;
	x_end   = (int)x + 8;
	y_end   = (int)y + 8;
#endif
#ifndef	NEW_FRUSTUM
	if(x_start < 0) x_start = 0;
	if(x_end >= tile_map_size_x) x_end = tile_map_size_x - 1;
	if(y_start < 0) y_start = 0;
	if(y_end >= tile_map_size_y) y_end = tile_map_size_y - 1;

	if(dungeon) water_id = tile_list[231];
	else water_id = tile_list[0];
	
	if (use_frame_buffer)
	{
		for(y = y_start; y <= y_end; y++)
		{
			y_scaled=y*3.0f;
			for(x = x_start; x <= x_end; x++)
			{
				x_scaled = x*3.0f;
				if(IS_WATER_TILE(tile_map[y*tile_map_size_x+x]) && check_tile_in_frustrum(x_scaled, y_scaled))
				{
					if(!tile_map[y*tile_map_size_x+x])
					{
						get_and_set_texture_id(water_id);
					}
					else get_and_set_texture_id(tile_list[tile_map[y*tile_map_size_x+x]]);
					draw_lake_water_tile_framebuffer(x_scaled, y_scaled);
				}
			}
		}
	}
	else
	{
		for(y = y_start; y <= y_end; y++)
		{
			y_scaled=y*3.0f;
			for(x = x_start; x <= x_end; x++)
			{
				x_scaled = x*3.0f;
				if(IS_WATER_TILE(tile_map[y*tile_map_size_x+x]) && check_tile_in_frustrum(x_scaled, y_scaled))
				{
					if(!tile_map[y*tile_map_size_x+x])
					{
						get_and_set_texture_id(water_id);
					}
					else get_and_set_texture_id(tile_list[tile_map[y*tile_map_size_x+x]]);
					draw_lake_water_tile(x_scaled, y_scaled);
				}
			}
		}
	}
#else
	if(dungeon) water_id = tile_list[231];
	else water_id = tile_list[0];

	get_intersect_start_stop(main_bbox_tree, TYPE_NO_REFLECTIV_WATER, &start, &stop);
	for (i = start; i < stop; i++)
	{
		l = get_intersect_item_ID(main_bbox_tree, i);
		x = get_terrain_x(l);
		y = get_terrain_y(l);
		y_scaled = y*3.0f;
		x_scaled = x*3.0f;
		if(!tile_map[y*tile_map_size_x+x]) get_and_set_texture_id(water_id);
		else get_and_set_texture_id(tile_list[tile_map[y*tile_map_size_x+x]]);
		draw_lake_water_tile(x_scaled,y_scaled);
	}
	if (use_frame_buffer && show_reflection)
	{
		ELglActiveTextureARB(base_unit);
		glEnable(GL_TEXTURE_2D);
		
		ELglActiveTextureARB(detail_unit);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, water_reflection_fbo_texture);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_CONSTANT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR);
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, blend_vec);
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 1);

		ELglActiveTextureARB(base_unit);
		
		get_intersect_start_stop(main_bbox_tree, TYPE_REFLECTIV_WATER, &start, &stop);
		for (i = start; i < stop; i++)
		{
			l = get_intersect_item_ID(main_bbox_tree, i);
			x = get_terrain_x(l);
			y = get_terrain_y(l);
			y_scaled = y*3.0f;
			x_scaled = x*3.0f;
			if(!tile_map[y*tile_map_size_x+x]) get_and_set_texture_id(water_id);
			else get_and_set_texture_id(tile_list[tile_map[y*tile_map_size_x+x]]);
			draw_lake_water_tile_framebuffer(x_scaled,y_scaled);
		}
	}
	else
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		get_intersect_start_stop(main_bbox_tree, TYPE_REFLECTIV_WATER, &start, &stop);
		for (i = start; i < stop; i++)
		{
			l = get_intersect_item_ID(main_bbox_tree, i);
			x = get_terrain_x(l);
			y = get_terrain_y(l);
			y_scaled = y*3.0f;
			x_scaled = x*3.0f;
			if(!tile_map[y*tile_map_size_x+x]) get_and_set_texture_id(water_id);
			else get_and_set_texture_id(tile_list[tile_map[y*tile_map_size_x+x]]);
			draw_lake_water_tile(x_scaled,y_scaled);
		}
	}
#endif
	
	if (use_frame_buffer && show_reflection)
	{
		ELglActiveTextureARB(detail_unit);
		glDisable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glBindTexture(GL_TEXTURE_2D,0);

		ELglClientActiveTextureARB(base_unit);
		ELglActiveTextureARB(base_unit);
		glEnable(GL_TEXTURE_2D);
		last_texture=-1;
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		CHECK_GL_ERRORS();
	}
	else
	{
		glDisable(GL_BLEND);
	}
	glDisable(GL_CULL_FACE);
}

void draw_sky_background()
{
	static GLfloat lights_c[4][3];
#ifdef MAP_EDITOR2
	int i;
#else
#ifdef NEW_WEATHER
	int i;
#else
	int i, j;
#endif
#endif
	int view_port[4];
	
	if (use_frame_buffer && show_reflection)
	{
		glGetIntegerv(GL_VIEWPORT, view_port);
		ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, water_reflection_fbo);
		glViewport(0, 0, reflection_texture_width, reflection_texture_height);
		init_texturing();
		glClearDepth(0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearDepth(1.0f);
		Enter2DModeExtended(reflection_texture_width, reflection_texture_height);
	}
	else
	{
		Enter2DMode();
	}

#ifdef NEW_WEATHER
	for (i = 0; i < 3; i++) {
		// get the sky color
		lights_c[0][i] = weather_bias_light(sky_lights_c1[light_level][i]);
		lights_c[1][i] = weather_bias_light(sky_lights_c2[light_level][i]);
		lights_c[2][i] = weather_bias_light(sky_lights_c3[light_level][i]);
		lights_c[3][i] = weather_bias_light(sky_lights_c4[light_level][i]);
	}

	for (i = 0; i < 4; i++) {
		weather_color_bias(lights_c[i], lights_c[i]);
	}
#else
	for (i=0; i<3; i++) {
		// get the sky color
		lights_c[0][i] = sky_lights_c1[light_level][i];
		lights_c[1][i] = sky_lights_c2[light_level][i];
		lights_c[2][i] = sky_lights_c3[light_level][i];
		lights_c[3][i] = sky_lights_c4[light_level][i];
		
#ifndef MAP_EDITOR2		
		for (j=0; j<4; j++) {
			// make it darker according to weather
			GLfloat tmp = lights_c[j][i] - (float)weather_light_offset/100.0f;
			// blend it with fog color according to fog density
			lights_c[j][i] = (1.0f - fogAlpha)*tmp + fogAlpha*fogColor[i];
		}
#endif
	}
#endif
	
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);

	if (use_frame_buffer && show_reflection)
	{
		glColor3fv(lights_c[0]);
		glVertex3i(0, 0, 0);
		glColor3fv(lights_c[1]);
		glVertex3i(0, reflection_texture_height, 0);
		glColor3fv(lights_c[2]);
		glVertex3i(reflection_texture_width, reflection_texture_height, 0);
		glColor3fv(lights_c[3]);
		glVertex3i(reflection_texture_width, 0, 0);
	}
	else
	{
		glColor3fv(lights_c[0]);
		glVertex3i(0, 0, 0);
		glColor3fv(lights_c[1]);
		glVertex3i(0, window_height, 0);
		glColor3fv(lights_c[2]);
		glVertex3i(window_width, window_height, 0);
		glColor3fv(lights_c[3]);
		glVertex3i(window_width, 0, 0);
	}

	glEnd();
	glEnable(GL_TEXTURE_2D);
	Leave2DMode();
	if (use_frame_buffer && show_reflection)
	{
		ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glViewport(view_port[0], view_port[1], view_port[2], view_port[3]);
	}
}

void draw_dungeon_sky_background()
{
	static const GLfloat baseColor[3] = { 0.00f, 0.21f, 0.34f };
#ifndef MAP_EDITOR2
	static GLfloat color[3];
#ifndef  NEW_WEATHER
	int i;
#endif //  NEW_WEATHER
#endif // MAP_EDITOR2
	int view_port[4];

	if (use_frame_buffer && show_reflection)
	{
		glGetIntegerv(GL_VIEWPORT, view_port);
		ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, water_reflection_fbo);
		glViewport(0, 0, reflection_texture_width, reflection_texture_height);
		init_texturing();
		glClearDepth(0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearDepth(1.0f);
		Enter2DModeExtended(reflection_texture_width, reflection_texture_height);
	}
	else
	{
		Enter2DMode();
	}

#ifdef MAP_EDITOR2
	glColor3fv(baseColor);
#else // MAP EDITOR 2

#ifdef  NEW_WEATHER
	weather_color_bias(baseColor, color);
#else //  NEW_WEATHER
	for (i=0; i<3; i++) {
		color[i] = (1.0f - fogAlpha)*baseColor[i] + fogAlpha*fogColor[i];
	}
#endif //  NEW_WEATHER

	glColor3fv(color);
#endif // MAP_EDITOR
	
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	//draw the sky background

	if (use_frame_buffer && show_reflection)
	{
		glVertex3i(0, 0, 0);
		glVertex3i(0, reflection_texture_height, 0);
		glVertex3i(reflection_texture_width, reflection_texture_height, 0);
		glVertex3i(reflection_texture_width, 0, 0);
	}
	else
	{
		glVertex3i(0, 0, 0);
		glVertex3i(0, window_height, 0);
		glVertex3i(window_width, window_height, 0);
		glVertex3i(window_width, 0, 0);
	}

	glEnd();
	glEnable(GL_TEXTURE_2D);
	Leave2DMode();
	if (use_frame_buffer && show_reflection)
	{
		ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glViewport(view_port[0], view_port[1], view_port[2], view_port[3]);
	}
}
