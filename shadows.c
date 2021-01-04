#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL.h>
#include "shadows.h"
#include "2d_objects.h"
#include "3d_objects.h"
#include "bbox_tree.h"
#include "cal.h"
#include "cursors.h"
#include "draw_scene.h"
#include "e3d.h"
#include "elconfig.h"
#include "events.h"
#include "framebuffer.h"
#include "lights.h"
#include "map.h"
#include "new_actors.h"
#include "reflection.h"
#include "skeletons.h"
#include "textures.h"
#include "tiles.h"
#include "weather.h"
#include "io/e3d_io.h"

#ifdef OSX
#define GL_EXT_texture_env_combine 1
#endif

// TODO: pbuffers according to Mesa/progs/xdemos/glxpbdemo.c

float proj_on_ground[16];
float ground_plane[4]={0,0,1,0};
float sun_position[4]={400.0, 400.0, 500.0, 0.0};
double light_view_mat[16],light_proj_mat[16],shadow_texgen_mat[16];
int shadows_on=0;
int is_day=0;
int use_shadow_mapping=1;

//TODO: Would like to use TEXTURE_RECTANGLE for cards that support it, but for some reason it doesn't work??
GLenum depth_texture_target=GL_TEXTURE_2D;
int shadow_map_size;
GLuint depth_map_id = 0;
/* Good values:
#define depth_map_scale 15.0
#define light_view_near -30.0
#define light_view_far 6.0*/
GLuint shadow_fbo = 0;

GLfloat light_view_hscale=13.0;
GLfloat light_view_top=10.0;
GLfloat light_view_bottom=-10.0;
GLfloat light_view_near=-30.0;
GLfloat light_view_far=6.0;

#ifdef  DEBUG
extern int e3d_count, e3d_total;    // LRNR:stats testing only
extern int cur_e3d_count;
#endif
extern e3d_object   *cur_e3d;

void free_shadow_framebuffer()
{
	free_depth_framebuffer(&shadow_fbo, &depth_map_id);
}

void make_shadow_framebuffer()
{
	change_shadow_framebuffer_size();
}

void change_shadow_framebuffer_size()
{
	change_depth_framebuffer_size(shadow_map_size, shadow_map_size, &shadow_fbo, &depth_map_id);
}

void calc_light_frustum(float light_xrot)
{
	float window_ratio=(GLfloat)window_width/(GLfloat)window_height;
	float max_height=30.0; //TODO: Really calculate this from positions and heights of objects
	float x,y;
	float slight, clight;

	light_xrot=-light_xrot;
	clight=cos(light_xrot);
	slight=sin(light_xrot);
	//TODO: Optimize this function a bit.
	//Assuming a max zoom_level of 3.75 and near/far distances of 20.0, we'll set the hscale to the radius of a circle that
	//can just contain the view frustum of the player. To simplify things, we'll assume the view frustum is horizontal.
	//light_view_hscale=sqrt(window_ratio*window_ratio*3.75f*3.75f+12.0f*12.0f);
	light_view_hscale=sqrt(window_ratio*window_ratio+30.0f*30.0f);
	// For the others, we can just use the parametric ellipse formula to find the value for this angle
	x=light_view_hscale*slight;
	y=max_height*clight;
	light_view_top=sqrt(x*x+y*y);
	y=3.75f*clight;
	light_view_far=sqrt(x*x+y*y);
	x=light_view_hscale*clight;
	y=3.75f*slight;
	light_view_bottom=-sqrt(x*x+y*y);
	x=100.0f*slight;  // A bit better than the real value (infinity)
	y=max_height*clight;
	light_view_near=-sqrt(x*x+y*y);
}

void calc_shadow_matrix()
{
	float light_pos[4];

	if (!is_day && lightning_falling)
		memcpy(light_pos, lightning_position, 4*sizeof(float));
	else
		memcpy(light_pos, sun_position, 4*sizeof(float));

	if(use_shadow_mapping)
		{
			float xrot,zrot;

			float div_length=1.0f/sqrt(light_pos[0]*light_pos[0]+light_pos[1]*light_pos[1]+light_pos[2]*light_pos[2]);
			light_pos[0]*=div_length;
			light_pos[1]*=div_length;
			light_pos[2]*=div_length;
			// Grumble, Old version of OS X don't have *f trig functions but I'm compiling on a version so I can't just #define my way out
#ifdef OSX
			xrot=-acos(light_pos[2]);
#else
			xrot=-acosf(light_pos[2]);
#endif
			//xrot=-atan2f(light_pos[2],light_pos[0])*180.0f/(float)M_PI;
#ifdef OSX
			zrot=-90.0f-atan2(light_pos[1],light_pos[0])*180.0f/(float)M_PI;
#else
			zrot=-90.0f-atan2f(light_pos[1],light_pos[0])*180.0f/(float)M_PI;
#endif

			glPushMatrix();
			glLoadIdentity();
			calc_light_frustum(xrot);
			xrot*=180.0f/(float)M_PI;
			glOrtho(-light_view_hscale,light_view_hscale,
				light_view_bottom,light_view_top,light_view_near,light_view_far);
			glGetDoublev(GL_MODELVIEW_MATRIX,light_proj_mat);
			glLoadIdentity();
			glRotatef(xrot,1.0f,0.0f,0.0f);
			glRotatef(zrot,0.0f,0.0f,1.0f);
			glGetDoublev(GL_MODELVIEW_MATRIX,light_view_mat);
			glLoadIdentity();
			if(depth_texture_target!=GL_TEXTURE_2D)glScalef(shadow_map_size,shadow_map_size,0);
			glTranslatef(0.5,0.5,0.5);   // This...
			glScalef(0.5,0.5,0.5);       // ...and this == S
			glMultMatrixd(light_proj_mat);     // Plight
			glMultMatrixd(light_view_mat);     // L^-1
			glGetDoublev(GL_MODELVIEW_MATRIX,shadow_texgen_mat);
			glPopMatrix();
		}
	else
		{
			float dot;

			// dot product of plane and light position
			dot = ground_plane[0] * light_pos[0]
			  + ground_plane[1] * light_pos[1]
			  + ground_plane[2] * light_pos[2]
			  + ground_plane[3] * light_pos[3];

			// first column
			proj_on_ground[0] = dot - light_pos[0] * ground_plane[0];
			proj_on_ground[4] = 0.0f - light_pos[0] * ground_plane[1];
			proj_on_ground[8] = 0.0f - light_pos[0] * ground_plane[2];
			proj_on_ground[12] = 0.0f - light_pos[0] * ground_plane[3];

			// second column
			proj_on_ground[1] = 0.0f - light_pos[1] * ground_plane[0];
			proj_on_ground[5] = dot - light_pos[1] * ground_plane[1];
			proj_on_ground[9] = 0.0f - light_pos[1] * ground_plane[2];
			proj_on_ground[13] = 0.0f - light_pos[1] * ground_plane[3];

			// third column
			proj_on_ground[2] = 0.0f - light_pos[2] * ground_plane[0];
			proj_on_ground[6] = 0.0f - light_pos[2] * ground_plane[1];
			proj_on_ground[10] = dot - light_pos[2] * ground_plane[2];
			proj_on_ground[14] = 0.0f - light_pos[2] * ground_plane[3];

			// fourth column
			proj_on_ground[3] = 0.0f - light_pos[3] * ground_plane[0];
			proj_on_ground[7] = 0.0f - light_pos[3] * ground_plane[1];
			proj_on_ground[11] = 0.0f - light_pos[3] * ground_plane[2];
			proj_on_ground[15] = dot - light_pos[3] * ground_plane[3];
		}
	main_bbox_tree->intersect[INTERSECTION_TYPE_SHADOW].intersect_update_needed = 1;
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void draw_3d_object_shadows(unsigned int object_type)
{
	unsigned int    start, stop;
	unsigned int    i, j, l;
	int is_transparent;
#ifdef SIMPLE_LOD
	int x, y;
	int dist;

	x= -camera_x;
	y= -camera_y;
#endif //SIMPLE_LOD

	cur_e3d= NULL;
#ifdef  DEBUG
	cur_e3d_count= 0;
#endif  //DEBUG

	get_intersect_start_stop(main_bbox_tree, object_type, &start, &stop);
	// nothing to draw?
	if(start >= stop){
		return;
	}

	// find the modes we need
	is_transparent= is_alpha_3d_object(object_type);
	if(is_transparent)
		{
			glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
			glAlphaFunc(GL_GREATER,0.05f);
//			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		}
//	else glDisable(GL_TEXTURE_2D);//we don't need textures for non transparent objects

	glEnable(GL_TEXTURE_2D);
	// now loop through each object
	for (i=start; i<stop; i++)
	{
		j = get_intersect_item_ID(main_bbox_tree, i);
		l = get_3dobject_index(j);
		if (objects_list[l] == NULL) continue;
		//track the usage
		cache_use(objects_list[l]->e3d_data->cache_ptr);
		if(!objects_list[l]->display) continue;	// not currently on the map, ignore it
#ifdef  SIMPLE_LOD
		// simple size/distance culling
		dist= (x-objects_list[l]->x_pos)*(x-objects_list[l]->x_pos) + (y-objects_list[l]->y_pos)*(y-objects_list[l]->y_pos);
		if(objects_list[l]->e3d_data->materials && (10000*objects_list[l]->e3d_data->materials[get_3dobject_material(j)].max_size)/(dist) < ((is_transparent)?15:10)) continue;
#endif  //SIMPLE_LOD
		draw_3d_object_detail(objects_list[l], get_3dobject_material(j), 0, is_transparent, 0);
	}

	if (use_compiled_vertex_array && (cur_e3d != NULL))
	{
		ELglUnlockArraysEXT();
	}
	if (use_vertex_buffers)
	{
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		ELglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	}
	if(is_transparent)
	{
		glDisable(GL_ALPHA_TEST);
	}
	else glEnable(GL_TEXTURE_2D);

	CHECK_GL_ERRORS();
#ifdef  DEBUG
	// final statistics
	if(cur_e3d_count > 0){
		e3d_count++;
		e3d_total+= cur_e3d_count;
	}
	cur_e3d_count= 0;
#endif  //DEBUG
	cur_e3d= NULL;
}

void display_shadows()
{
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.05f, 2.0f);
	glEnable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_COLOR_MATERIAL);

	draw_3d_object_shadows(TYPE_3D_NO_BLEND_NO_GROUND_ALPHA_SELF_LIT_OBJECT);
	draw_3d_object_shadows(TYPE_3D_NO_BLEND_NO_GROUND_ALPHA_NO_SELF_LIT_OBJECT);
	draw_3d_object_shadows(TYPE_3D_NO_BLEND_NO_GROUND_NO_ALPHA_SELF_LIT_OBJECT);
	draw_3d_object_shadows(TYPE_3D_NO_BLEND_NO_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT);

	if(!use_shadow_mapping)
	{
		draw_3d_object_shadows(TYPE_3D_NO_BLEND_GROUND_ALPHA_SELF_LIT_OBJECT);
		draw_3d_object_shadows(TYPE_3D_NO_BLEND_GROUND_ALPHA_NO_SELF_LIT_OBJECT);
		draw_3d_object_shadows(TYPE_3D_NO_BLEND_GROUND_NO_ALPHA_SELF_LIT_OBJECT);
		draw_3d_object_shadows(TYPE_3D_NO_BLEND_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

#ifndef MAP_EDITOR2
	display_actors(0, DEPTH_RENDER_PASS);
#endif
	glCullFace(GL_BACK);
	glDisable(GL_POLYGON_OFFSET_FILL);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void display_3d_ground_objects()
{
	glEnable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_COLOR_MATERIAL);

	if (!dungeon && clouds_shadows)
	{
		//bind the detail texture
		ELglActiveTextureARB(detail_unit);
		glEnable(GL_TEXTURE_2D);
		bind_texture_unbuffered(ground_detail_text);
		ELglActiveTextureARB(base_unit);
		glEnable(GL_TEXTURE_2D);
	}

	draw_3d_objects(TYPE_3D_NO_BLEND_GROUND_ALPHA_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_NO_BLEND_GROUND_ALPHA_NO_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_NO_BLEND_GROUND_NO_ALPHA_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_NO_BLEND_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT);

	if (!dungeon && clouds_shadows)
	{
		//disable the second texture unit
		ELglActiveTextureARB(detail_unit);
		glDisable(GL_TEXTURE_2D);
		ELglActiveTextureARB(base_unit);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisable(GL_CULL_FACE);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void display_3d_non_ground_objects()
{
	//we don't want to be affected by 2d objects and shadows
	anything_under_the_mouse(0,UNDER_MOUSE_NO_CHANGE);

	glEnable(GL_CULL_FACE);
	glEnable(GL_COLOR_MATERIAL);
	glEnableClientState(GL_VERTEX_ARRAY);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	if (!dungeon && clouds_shadows)
	{
		//bind the detail texture
		ELglActiveTextureARB(detail_unit);
		glEnable(GL_TEXTURE_2D);
		bind_texture_unbuffered(ground_detail_text);
		ELglActiveTextureARB(base_unit);
		glEnable(GL_TEXTURE_2D);
	}

	draw_3d_objects(TYPE_3D_NO_BLEND_NO_GROUND_ALPHA_SELF_LIT_OBJECT);
	draw_3d_objects( TYPE_3D_NO_BLEND_NO_GROUND_ALPHA_NO_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_NO_BLEND_NO_GROUND_NO_ALPHA_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_NO_BLEND_NO_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT);

	if (!dungeon && clouds_shadows)
	{
		//disable the second texture unit
		ELglActiveTextureARB(detail_unit);
		glDisable(GL_TEXTURE_2D);
		ELglActiveTextureARB(base_unit);
	}
	glDisable(GL_COLOR_MATERIAL);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_CULL_FACE);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void render_light_view()
{
	unsigned int cur_intersect_type;
	if(use_shadow_mapping)
		{
			if (!use_frame_buffer && !depth_map_id)
				{
					GLint depthbits=16;
					GLenum internalformat=GL_DEPTH_COMPONENT16_ARB;

					glGetIntegerv(GL_DEPTH_BITS,&depthbits);
					if(depthbits==24)internalformat=GL_DEPTH_COMPONENT24_ARB;
					else if(depthbits==32)internalformat=GL_DEPTH_COMPONENT32_ARB;

					glGenTextures(1,&depth_map_id);
					glBindTexture(depth_texture_target,depth_map_id);
					CHECK_GL_ERRORS();
					glTexImage2D(depth_texture_target,0,internalformat,
						     shadow_map_size,shadow_map_size,
						     0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
					glTexParameteri(depth_texture_target,GL_TEXTURE_COMPARE_MODE_ARB,
							GL_COMPARE_R_TO_TEXTURE_ARB);
					glTexParameteri(depth_texture_target,GL_TEXTURE_COMPARE_FUNC_ARB,GL_LEQUAL);

					glTexParameteri(depth_texture_target,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
					glTexParameteri(depth_texture_target,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
					//TODO: Might want to use CLAMP_TO_BORDER for cards that support it?
					glTexParameteri(depth_texture_target,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
					glTexParameteri(depth_texture_target,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
					CHECK_GL_ERRORS();
				}

			if (use_frame_buffer)
			{
				CHECK_GL_ERRORS();
				CHECK_FBO_ERRORS();
				ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, shadow_fbo);
			        ELglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, depth_map_id, 0);
				glClear(GL_DEPTH_BUFFER_BIT);
        			glDrawBuffer(GL_NONE);
	        		glReadBuffer(GL_NONE);
				CHECK_GL_ERRORS();
				CHECK_FBO_ERRORS();
			}
			CHECK_GL_ERRORS();

			glPushAttrib(GL_ALL_ATTRIB_BITS);

			glViewport(0,0,shadow_map_size,shadow_map_size);
			CHECK_GL_ERRORS();

			glEnable(GL_SCISSOR_TEST);
			glScissor(1, 1, shadow_map_size-2, shadow_map_size-2);

			glDisable(GL_LIGHTING);
			glEnable(GL_DEPTH_TEST);
#ifndef MAP_EDITOR2
			if (use_fog) glDisable(GL_FOG);
#endif
			glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
			CHECK_GL_ERRORS();
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadMatrixd(light_proj_mat);
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadMatrixd(light_view_mat);
			glTranslatef((int)camera_x,(int)camera_y,(int)camera_z);
			cur_intersect_type = get_cur_intersect_type(main_bbox_tree);
			set_cur_intersect_type(main_bbox_tree, INTERSECTION_TYPE_SHADOW);
			calculate_shadow_frustum();
			display_shadows();
			set_cur_intersect_type(main_bbox_tree, cur_intersect_type);

			if (!use_frame_buffer)
			{
				glBindTexture(depth_texture_target, depth_map_id);
				glCopyTexSubImage2D(depth_texture_target, 0, 0, 0, 0, 0, shadow_map_size, shadow_map_size);
				glClear(GL_DEPTH_BUFFER_BIT);
			}

			CHECK_GL_ERRORS();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
			glPopAttrib();
			glBindTexture(GL_TEXTURE_2D,0);
			last_texture=-1;
			CHECK_GL_ERRORS();
			if (use_frame_buffer)
			{
				CHECK_GL_ERRORS();
				CHECK_FBO_ERRORS();
				ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        			glDrawBuffer(GL_BACK);
	      			glReadBuffer(GL_BACK);
				CHECK_GL_ERRORS();
				CHECK_FBO_ERRORS();
			}
			CHECK_GL_ERRORS();
		}
}

void setup_2d_texgen()
{
	GLfloat plane[4];

	glEnable(GL_TEXTURE_GEN_S);
	glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
	plane[0]=shadow_texgen_mat[0];plane[1]=shadow_texgen_mat[4];plane[2]=shadow_texgen_mat[8];plane[3]=shadow_texgen_mat[12];
	glTexGenfv(GL_S,GL_EYE_PLANE,plane);

	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
	plane[0]=shadow_texgen_mat[1];plane[1]=shadow_texgen_mat[5];plane[2]=shadow_texgen_mat[9];plane[3]=shadow_texgen_mat[13];
	glTexGenfv(GL_T,GL_EYE_PLANE,plane);

	glEnable(GL_TEXTURE_GEN_R);
	glTexGeni(GL_R,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
	plane[0]=shadow_texgen_mat[2];plane[1]=shadow_texgen_mat[6];plane[2]=shadow_texgen_mat[10];plane[3]=shadow_texgen_mat[14];
	glTexGenfv(GL_R,GL_EYE_PLANE,plane);

	glEnable(GL_TEXTURE_GEN_Q);
	glTexGeni(GL_Q,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
	plane[0]=shadow_texgen_mat[3];plane[1]=shadow_texgen_mat[7];plane[2]=shadow_texgen_mat[11];plane[3]=shadow_texgen_mat[15];
	glTexGenfv(GL_Q,GL_EYE_PLANE,plane);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void disable_texgen()
{
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
	glDisable(GL_TEXTURE_GEN_Q);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void setup_shadow_mapping()
{
    GLfloat shadow_color[] = {ambient_light[0]+0.2,
							  ambient_light[1]+0.2,
							  ambient_light[2]+0.2,
							  1.0};

	if (!is_day && lightning_falling)
	{
		if (lightning_ambient_color[0]+0.2 > shadow_color[0]) shadow_color[0] = lightning_ambient_color[0]+0.2;
		if (lightning_ambient_color[1]+0.2 > shadow_color[1]) shadow_color[1] = lightning_ambient_color[1]+0.2;
		if (lightning_ambient_color[2]+0.2 > shadow_color[2]) shadow_color[2] = lightning_ambient_color[2]+0.2;
	}

	glPushMatrix();
	glLoadIdentity();
	if (!first_person)
	glTranslatef(0.0f, 0.0f, -zoom_level*camera_distance);
	glRotatef(rx, 1.0f, 0.0f, 0.0f);
	if (first_person)
	{
		float head_pos[3];
        cal_get_actor_bone_local_position(get_our_actor(), get_actor_bone_id(get_our_actor(), head_bone), NULL, head_pos);
		glTranslatef(head_pos[0], head_pos[1], 0.0);
	}
	glRotatef(rz, 0.0f, 0.0f, 1.0f);
	glTranslatef(camera_x-(int)camera_x,camera_y-(int)camera_y,camera_z-(int)camera_z);

	glBindTexture(depth_texture_target,depth_map_id);
	setup_2d_texgen();

#ifdef OSX
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_COMBINE_ARB);
	glTexEnvi(GL_TEXTURE_ENV,GL_COMBINE_RGB_ARB,GL_INTERPOLATE_ARB);
	glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE0_RGB_ARB,GL_PREVIOUS_ARB);
	glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND0_RGB_ARB,GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE1_RGB_ARB,GL_CONSTANT_ARB);
	glTexEnvfv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR,shadow_color);
	glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND1_RGB_ARB,GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE2_RGB_ARB,GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND2_RGB_ARB,GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV,GL_COMBINE_ALPHA_ARB,GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE0_ALPHA_ARB,GL_PREVIOUS_ARB);
	glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND0_ALPHA_ARB,GL_SRC_ALPHA);
#else
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_COMBINE_EXT);
	glTexEnvi(GL_TEXTURE_ENV,GL_COMBINE_RGB_EXT,GL_INTERPOLATE_EXT);
	glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE0_RGB_EXT,GL_PREVIOUS_EXT);
	glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND0_RGB_EXT,GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE1_RGB_EXT,GL_CONSTANT_EXT);
	glTexEnvfv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR,shadow_color);
	glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND1_RGB_EXT,GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE2_RGB_EXT,GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND2_RGB_EXT,GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV,GL_COMBINE_ALPHA_EXT,GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE0_ALPHA_EXT,GL_PREVIOUS_EXT);
	glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND0_ALPHA_EXT,GL_SRC_ALPHA);
#endif
	glPopMatrix();
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void setup_cloud_texturing(void)
{
	ELglActiveTextureARB(detail_unit);
	if (!dungeon && clouds_shadows)
	{
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	ELglActiveTextureARB(base_unit);
}

void draw_sun_shadowed_scene(int any_reflection)
{
	if(use_shadow_mapping)
		{
			reset_material();

			shadow_unit=GL_TEXTURE0_ARB;
			base_unit=GL_TEXTURE1_ARB;
			detail_unit=GL_TEXTURE2_ARB;

#ifndef MAP_EDITOR2
			if (use_fog) glDisable(GL_FOG);
#endif
			ELglActiveTextureARB(shadow_unit);
			glEnable(depth_texture_target);
			setup_shadow_mapping();

			setup_cloud_texturing();

			ELglClientActiveTextureARB(base_unit);
			ELglActiveTextureARB(base_unit);
			glEnable(GL_TEXTURE_2D);
			last_texture=-1;
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			CHECK_GL_ERRORS();
#ifndef MAP_EDITOR2
			if (use_fog) glEnable(GL_FOG);
#endif
			glNormal3f(0.0f,0.0f,1.0f);
			if(any_reflection)draw_lake_tiles();
			draw_tile_map();
#ifdef MAP_EDITOR2
			get_world_x_y ();
			display_mode();
#endif
			CHECK_GL_ERRORS();
			display_2d_objects();
			CHECK_GL_ERRORS();
			anything_under_the_mouse(0, UNDER_MOUSE_NOTHING);

			display_objects();
			display_ground_objects();
#ifndef MAP_EDITOR2
			display_actors(1, SHADOW_RENDER_PASS);  // Affects other textures ????????? (FPS etc., unless there's a particle system...)
#endif
			display_alpha_objects();
			display_blended_objects();

#ifndef MAP_EDITOR2
			if (use_fog) glDisable(GL_FOG);
#endif

			ELglActiveTextureARB(shadow_unit);
			glDisable(depth_texture_target);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glBindTexture(GL_TEXTURE_2D,0);
			disable_texgen();

			ELglActiveTextureARB(detail_unit);
			glDisable(GL_TEXTURE_2D);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glBindTexture(GL_TEXTURE_2D,0);
			disable_texgen();

			ELglActiveTextureARB(base_unit);
			glDisable(GL_TEXTURE_2D);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glBindTexture(GL_TEXTURE_2D,0);
			disable_texgen();

			shadow_unit=GL_TEXTURE2_ARB;
			base_unit=GL_TEXTURE0_ARB;
			detail_unit=GL_TEXTURE1_ARB;
			ELglActiveTextureARB(base_unit);
			ELglClientActiveTextureARB(base_unit);
			last_texture=-1;
			glBindTexture(GL_TEXTURE_2D,0);
			glEnable(GL_TEXTURE_2D);
		}
	else
		{
			glNormal3f(0.0f,0.0f,1.0f);
			if(any_reflection)draw_lake_tiles();

			setup_cloud_texturing();

			draw_tile_map();
#ifdef MAP_EDITOR2
			get_world_x_y ();
			display_mode();
#endif
			CHECK_GL_ERRORS();
			display_2d_objects();
			CHECK_GL_ERRORS();
			anything_under_the_mouse(0, UNDER_MOUSE_NOTHING);
			display_3d_ground_objects();
			// turning off writing to the color buffer and depth buffer
			glDisable(GL_DEPTH_TEST);

			glDisable(GL_LIGHTING);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

			glEnable(GL_STENCIL_TEST);
			// write a one to the stencil buffer everywhere we are about to draw
			glStencilFunc(GL_ALWAYS, 1, 0xffffffff);
			// this is to always pass a one to the stencil buffer where we draw
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

			display_shadows();

			glStencilFunc(GL_EQUAL, 1, 0xFFFFFFFF);
			// don't modify the contents of the stencil buffer
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

			// Lachesis: drawing in 3D mode in order to get the fog correct
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_LIGHTING);
			glDepthMask(GL_FALSE);
			glDisable(GL_DEPTH_TEST);

#ifndef MAP_EDITOR2
			if (use_fog) glEnable(GL_FOG);
#endif

			glEnable(GL_BLEND);
			// need this function (or both flipped) for correctly working fog too
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

			glColor4f(0.0f, 0.0f, 0.0f, 0.7f*((diffuse_light[0] + diffuse_light[1] + diffuse_light[2])/3.0 - 0.0f));

			glBegin(GL_QUADS);
				glVertex4f(-camera_x+20.0f,-camera_y+20.0f,0.0f,1.0f);
				glVertex4f(-camera_x+20.0f,-camera_y-20.0f,0.0f,1.0f);
				glVertex4f(-camera_x-20.0f,-camera_y-20.0f,0.0f,1.0f);
				glVertex4f(-camera_x-20.0f,-camera_y+20.0f,0.0f,1.0f);
			glEnd();

			glDisable(GL_BLEND);

			glEnable(GL_TEXTURE_2D);
			glDepthMask(GL_TRUE);

			glEnable(GL_DEPTH_TEST);
			glColor4f(1.0f,1.0f,1.0f,1.0f);
			glEnable(GL_LIGHTING);
			glDisable(GL_STENCIL_TEST);

			display_3d_non_ground_objects();
#ifndef MAP_EDITOR2
			display_actors(1, DEFAULT_RENDER_PASS);
#endif
			display_blended_objects();

		}
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}
