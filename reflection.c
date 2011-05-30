#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "reflection.h"
#include "3d_objects.h"
#include "bbox_tree.h"
#include "cal.h"
#include "draw_scene.h"
#include "elconfig.h"
#include "errors.h"
#include "framebuffer.h"
#include "gl_init.h"
#include "interface.h"
#include "io/map_io.h"
#include "lights.h"
#include "load_gl_extensions.h"
#include "map.h"
#include "textures.h"
#include "tiles.h"
#include "weather.h"
#ifdef CLUSTER_INSIDES_OLD
#include "cluster.h"
#endif
#include "shadows.h"
#include "global.h"
#include "shader/shader.h"
#include "sky.h"
#ifdef FSAA
#include "fsaa/fsaa.h"
#endif /* FSAA */

typedef struct
{
	float u;
	float v;
	float z;

}water_vertex;

float water_depth_offset=-0.25f;
float water_movement_u=0;
float water_movement_v=0;
int show_reflection=1;
GLuint water_reflection_fbo = 0;
GLuint water_reflection_fbo_depth_buffer = 0;
GLuint water_reflection_fbo_texture = 0;
int reflection_texture_width = 0;
int reflection_texture_height = 0;
MATRIX4x4 reflect_texgen_mat;
GLfloat* water_tile_buffer = 0;
GLuint water_tile_buffer_object = 0;
int water_buffer_usage = 0;
int water_buffer_reflectiv_index = 0;
int water_shader_quality = 0;

int get_max_supported_water_shader_quality()
{
	if (!use_frame_buffer)
	{
		return 0;
	}

	if (get_shader(st_water, sst_shadow_receiver, sft_disabled, 0) == 0)
	{
		return 0;
	}
	if (get_shader(st_water, sst_no_shadow_receiver, sft_disabled, 0) == 0)
	{
		return 0;
	}
	if (get_shader(st_water, sst_shadow_receiver, sft_enabled, 0) == 0)
	{
		return 0;
	}
	if (get_shader(st_water, sst_no_shadow_receiver, sft_enabled, 0) == 0)
	{
		return 0;
	}
	if (get_shader(st_reflectiv_water, sst_shadow_receiver, sft_disabled, 0) == 0)
	{
		return 0;
	}
	if (get_shader(st_reflectiv_water, sst_no_shadow_receiver, sft_disabled, 0) == 0)
	{
		return 0;
	}
	if (get_shader(st_reflectiv_water, sst_shadow_receiver, sft_enabled, 0) == 0)
	{
		return 0;
	}
	if (get_shader(st_reflectiv_water, sst_no_shadow_receiver, sft_enabled, 0) == 0)
	{
		return 0;
	}

	if (get_shader(st_water, sst_shadow_receiver, sft_disabled, 1) == 0)
	{
		return 1;
	}
	if (get_shader(st_water, sst_no_shadow_receiver, sft_disabled, 1) == 0)
	{
		return 1;
	}
	if (get_shader(st_water, sst_shadow_receiver, sft_enabled, 1) == 0)
	{
		return 1;
	}
	if (get_shader(st_water, sst_no_shadow_receiver, sft_enabled, 1) == 0)
	{
		return 1;
	}
	if (get_shader(st_reflectiv_water, sst_shadow_receiver, sft_disabled, 1) == 0)
	{
		return 1;
	}
	if (get_shader(st_reflectiv_water, sst_no_shadow_receiver, sft_disabled, 1) == 0)
	{
		return 1;
	}
	if (get_shader(st_reflectiv_water, sst_shadow_receiver, sft_enabled, 1) == 0)
	{
		return 1;
	}
	if (get_shader(st_reflectiv_water, sst_no_shadow_receiver, sft_enabled, 1) == 0)
	{
		return 1;
	}

	return 2;
}

void init_water_buffers(int water_buffer_size)
{
	water_tile_buffer = realloc(water_tile_buffer, water_buffer_size * 4 * 2 * sizeof(GLfloat));

	if (have_extension(arb_vertex_buffer_object))
	{
		if (water_tile_buffer_object == 0)
		{
			ELglGenBuffersARB(1, &water_tile_buffer_object);
		}
		else
		{
			ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, water_tile_buffer_object);
			ELglBufferDataARB(GL_ARRAY_BUFFER_ARB, 0, 0, GL_DYNAMIC_DRAW_ARB);
			ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		}
	}
}

static __inline__ void build_water_buffer()
{
	unsigned int i, j, l, x, y, start, stop;
	float x_scaled,y_scaled;
#ifdef CLUSTER_INSIDES_OLD
	short cluster = get_actor_cluster ();
	short tile_cluster;
#endif

	if (get_bbox_intersect_flag(main_bbox_tree, TYPE_REFLECTIV_WATER, ide_changed) ||
		get_bbox_intersect_flag(main_bbox_tree, TYPE_NO_REFLECTIV_WATER, ide_changed))
	{
		clear_bbox_intersect_flag(main_bbox_tree, TYPE_REFLECTIV_WATER, ide_changed);
		clear_bbox_intersect_flag(main_bbox_tree, TYPE_NO_REFLECTIV_WATER, ide_changed);
	}
	else
	{
		return;
	}

	j = 0;

	get_intersect_start_stop(main_bbox_tree, TYPE_NO_REFLECTIV_WATER, &start, &stop);

	for (i = start; i < stop; i++)
	{
		l = get_intersect_item_ID(main_bbox_tree, i);
		x = get_terrain_x (l);
		y = get_terrain_y (l);

#ifdef CLUSTER_INSIDES_OLD
		tile_cluster = get_cluster (6*x, 6*y);
		if (tile_cluster && tile_cluster != cluster)
			continue;
#endif

		x_scaled = x * 3.0f;
		y_scaled = y * 3.0f;

		water_tile_buffer[j * 8 + 0] = x_scaled;
		water_tile_buffer[j * 8 + 1] = y_scaled;
		water_tile_buffer[j * 8 + 2] = x_scaled + 3.0f;
		water_tile_buffer[j * 8 + 3] = y_scaled;
		water_tile_buffer[j * 8 + 4] = x_scaled + 3.0f;
		water_tile_buffer[j * 8 + 5] = y_scaled + 3.0f;
		water_tile_buffer[j * 8 + 6] = x_scaled;
		water_tile_buffer[j * 8 + 7] = y_scaled + 3.0f;

		if (x == 0)
		{
			water_tile_buffer[j * 8 + 0] -= water_tiles_extension;
			water_tile_buffer[j * 8 + 6] -= water_tiles_extension;
		}
		else if (x == tile_map_size_x-1)
		{
			water_tile_buffer[j * 8 + 2] += water_tiles_extension;
			water_tile_buffer[j * 8 + 4] += water_tiles_extension;
		}
		if (y == 0)
		{
			water_tile_buffer[j * 8 + 1] -= water_tiles_extension;
			water_tile_buffer[j * 8 + 3] -= water_tiles_extension;
		}
		else if (y == tile_map_size_y-1)
		{
			water_tile_buffer[j * 8 + 5] += water_tiles_extension;
			water_tile_buffer[j * 8 + 7] += water_tiles_extension;
		}

		j++;
	}

	water_buffer_reflectiv_index = j;

	get_intersect_start_stop(main_bbox_tree, TYPE_REFLECTIV_WATER, &start, &stop);

	for (i = start; i < stop; i++)
	{
		l = get_intersect_item_ID(main_bbox_tree, i);
		x = get_terrain_x (l);
		y = get_terrain_y (l);

#ifdef CLUSTER_INSIDES_OLD
		tile_cluster = get_cluster (6*x, 6*y);
		if (tile_cluster && tile_cluster != cluster)
			continue;
#endif

		x_scaled = x * 3.0f;
		y_scaled = y * 3.0f;

		water_tile_buffer[j * 8 + 0] = x_scaled;
		water_tile_buffer[j * 8 + 1] = y_scaled;
		water_tile_buffer[j * 8 + 2] = x_scaled + 3.0f;
		water_tile_buffer[j * 8 + 3] = y_scaled;
		water_tile_buffer[j * 8 + 4] = x_scaled + 3.0f;
		water_tile_buffer[j * 8 + 5] = y_scaled + 3.0f;
		water_tile_buffer[j * 8 + 6] = x_scaled;
		water_tile_buffer[j * 8 + 7] = y_scaled + 3.0f;

		if (x == 0)
		{
			water_tile_buffer[j * 8 + 0] -= water_tiles_extension;
			water_tile_buffer[j * 8 + 6] -= water_tiles_extension;
		}
		else if (x == tile_map_size_x-1)
		{
			water_tile_buffer[j * 8 + 2] += water_tiles_extension;
			water_tile_buffer[j * 8 + 4] += water_tiles_extension;
		}
		if (y == 0)
		{
			water_tile_buffer[j * 8 + 1] -= water_tiles_extension;
			water_tile_buffer[j * 8 + 3] -= water_tiles_extension;
		}
		else if (y == tile_map_size_y-1)
		{
			water_tile_buffer[j * 8 + 5] += water_tiles_extension;
			water_tile_buffer[j * 8 + 7] += water_tiles_extension;
		}

		j++;
	}
	water_buffer_usage = j;

	if (have_extension(arb_vertex_buffer_object) && water_buffer_usage > 0)
	{
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, water_tile_buffer_object);
		ELglBufferDataARB(GL_ARRAY_BUFFER_ARB, water_buffer_usage * 4 * 2 * sizeof(GLfloat),
			water_tile_buffer, GL_DYNAMIC_DRAW_ARB);
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	}
}

static __inline__ void init_depth()
{
	unsigned int start, stop;

	glClearDepth(0.0f);
	glClear(GL_DEPTH_BUFFER_BIT);
	glClearDepth(1.0f);
	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_ALWAYS);
	glDepthRange(1.0f, 1.0f);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	/* Now drawing the water */
	get_intersect_start_stop(main_bbox_tree, TYPE_REFLECTIV_WATER, &start, &stop);
	
	if (use_vertex_buffers)
	{
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, water_tile_buffer_object);
		glInterleavedArrays(GL_V2F, 0, 0);
	}
	else
	{
		glInterleavedArrays(GL_V2F, 0, water_tile_buffer);
	}
	glDrawArrays(GL_QUADS, water_buffer_reflectiv_index * 4, (water_buffer_usage - water_buffer_reflectiv_index) * 4);
	
	if (use_vertex_buffers)
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	glDisableClientState(GL_VERTEX_ARRAY);

	/* Re-enable update of color and depth. */
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthRange(0.0f, 1.0f);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

//if there is any reflecting tile, returns 1, otherwise 0
int find_reflection()
{
	unsigned int idx;
	
	idx = main_bbox_tree->cur_intersect_type;
	if (main_bbox_tree->intersect[idx].start[TYPE_REFLECTIV_WATER] < main_bbox_tree->intersect[idx].stop[TYPE_REFLECTIV_WATER]) return 2;
	else 
	{
		if (main_bbox_tree->intersect[idx].start[TYPE_NO_REFLECTIV_WATER] < main_bbox_tree->intersect[idx].stop[TYPE_NO_REFLECTIV_WATER]) return 1;
		else return 0;
	}
}

static __inline__ int adapt_size(int size)
{
	GLint i, j;
	
	glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE_EXT, &i);

	size = min2i(size, i);

	if (have_extension(arb_texture_non_power_of_two) || supports_gl_version(2, 0))
	{
		return size;
	}
	else
	{
		j = 1;
		while (j < size)
		{
			j += j;
		}
		return j;
	}
}

void free_reflection_framebuffer()
{
	free_color_framebuffer(&water_reflection_fbo, &water_reflection_fbo_depth_buffer, 
		NULL, &water_reflection_fbo_texture);
}

void make_reflection_framebuffer(int width, int height)
{
	reflection_texture_width = adapt_size(width);
	reflection_texture_height = adapt_size(height);
	free_reflection_framebuffer();
	make_color_framebuffer(reflection_texture_width, reflection_texture_height,
		&water_reflection_fbo, &water_reflection_fbo_depth_buffer, 
		NULL, &water_reflection_fbo_texture);
}

void change_reflection_framebuffer_size(int width, int height)
{
	reflection_texture_width = adapt_size(width);
	reflection_texture_height = adapt_size(height);
	change_color_framebuffer_size(reflection_texture_width, reflection_texture_height,
		&water_reflection_fbo, &water_reflection_fbo_depth_buffer, 
		NULL, &water_reflection_fbo_texture);
}

static __inline__ void init_texturing()
{
	MATRIX4x4 reflect_view_mat;
	MATRIX4x4 reflect_proj_mat;

	glGetFloatv(GL_MODELVIEW_MATRIX, reflect_view_mat);
	glGetFloatv(GL_PROJECTION_MATRIX, reflect_proj_mat);

	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0.5f, 0.5f, 0.5f);
	glScalef(0.5f, 0.5f, 0.5f);
	glMultMatrixf(reflect_proj_mat);
	glMultMatrixf(reflect_view_mat);
	glTranslatef(0.0f, 0.0f, water_depth_offset);
	glGetFloatv(GL_MODELVIEW_MATRIX, reflect_texgen_mat);
	glPopMatrix();
#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

static __inline__ void setup_water_fbo_texgen()
{
	GLfloat plane[4];

	ELglActiveTextureARB(detail_unit);

	glEnable(GL_TEXTURE_GEN_S);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	plane[0] = reflect_texgen_mat[0];
	plane[1] = reflect_texgen_mat[4];
	plane[2] = reflect_texgen_mat[8];
	plane[3] = reflect_texgen_mat[12];
	glTexGenfv(GL_S, GL_EYE_PLANE, plane);

	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	plane[0] = reflect_texgen_mat[1];
	plane[1] = reflect_texgen_mat[5];
	plane[2] = reflect_texgen_mat[9];
	plane[3] = reflect_texgen_mat[13];
	glTexGenfv(GL_T, GL_EYE_PLANE, plane);

	glEnable(GL_TEXTURE_GEN_R);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	plane[0] = reflect_texgen_mat[2];
	plane[1] = reflect_texgen_mat[6];
	plane[2] = reflect_texgen_mat[10];
	plane[3] = reflect_texgen_mat[14];
	glTexGenfv(GL_R, GL_EYE_PLANE, plane);

	glEnable(GL_TEXTURE_GEN_Q);
	glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	plane[0] = reflect_texgen_mat[3];
	plane[1] = reflect_texgen_mat[7];
	plane[2] = reflect_texgen_mat[11];
	plane[3] = reflect_texgen_mat[15];
	glTexGenfv(GL_Q, GL_EYE_PLANE, plane);

	ELglActiveTextureARB(base_unit);

#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

static __inline__ void setup_water_texgen()
{
	GLfloat plane[4];

	glEnable(GL_TEXTURE_GEN_S);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	plane[0] = 3.0f / 50.0f;
	plane[1] = 0.0f;
	plane[2] = 0.0f;
	plane[3] = water_movement_u;
	glTexGenfv(GL_S, GL_OBJECT_PLANE, plane);

	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	plane[0] = 0.0f;
	plane[1] = 3.0f / 50.0f;
	plane[2] = 0.0f;
	plane[3] = water_movement_v;
	glTexGenfv(GL_T, GL_OBJECT_PLANE, plane);

#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

static __inline__ void setup_noise_texgen()
{
	GLfloat plane[4];

	ELglActiveTextureARB(GL_TEXTURE3);

	glEnable(GL_TEXTURE_GEN_S);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	plane[0] = 1.0f / 3.0f;
	plane[1] = 0.0f;
	plane[2] = 0.0f;
	plane[3] = 0.0f;
	glTexGenfv(GL_S, GL_OBJECT_PLANE, plane);

	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	plane[0] = 0.0f;
	plane[1] = 1.0f / 3.0f;
	plane[2] = 0.0f;
	plane[3] = 0.0f;
	glTexGenfv(GL_T, GL_OBJECT_PLANE, plane);

	ELglActiveTextureARB(base_unit);

#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

static __inline__ void disable_water_fbo_texgen()
{
	ELglActiveTextureARB(detail_unit);

	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
	glDisable(GL_TEXTURE_GEN_Q);

	ELglActiveTextureARB(base_unit);

#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

static __inline__ void disable_water_texgen()
{
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

static __inline__ void disable_noise_texgen()
{
	ELglActiveTextureARB(GL_TEXTURE3);

	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

	ELglActiveTextureARB(base_unit);

#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void display_3d_reflection()
{
	GLint view_port[4];
	unsigned int cur_intersect_type;
	int clip_sky = 0;

	CalculateFrustum();

	cur_intersect_type = get_cur_intersect_type(main_bbox_tree);
	set_cur_intersect_type(main_bbox_tree, INTERSECTION_TYPE_DEFAULT);
	build_water_buffer();

	if (water_buffer_usage == 0) return;

	init_depth();
	set_cur_intersect_type(main_bbox_tree, cur_intersect_type);

	if (use_frame_buffer && water_shader_quality > 0)
	{
		CHECK_GL_ERRORS();
		CHECK_FBO_ERRORS();
		glGetIntegerv(GL_VIEWPORT, view_port);
		CHECK_GL_ERRORS();
		ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, water_reflection_fbo);
		glViewport(0, 0, reflection_texture_width, reflection_texture_height);
		CHECK_GL_ERRORS();
		CHECK_FBO_ERRORS();

		glPushMatrix();
		glTranslatef(0.0f, 0.0f, water_depth_offset);
	}
	else
    {
		glPushMatrix();
		glTranslatef(0.0f, 0.0f, water_depth_offset);

        if (have_stencil)
        {
            unsigned int start, stop;
            
            glClearStencil(0);
            glClear(GL_STENCIL_BUFFER_BIT);
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_ALWAYS, 1, 1);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			glDepthMask(GL_FALSE);
            
            if (use_vertex_buffers)
            {
                ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, water_tile_buffer_object);
                glInterleavedArrays(GL_V2F, 0, 0);
            }
            else
            {
                glInterleavedArrays(GL_V2F, 0, water_tile_buffer);
            }
            
            get_intersect_start_stop(main_bbox_tree, TYPE_REFLECTIV_WATER, &start, &stop);
            glDrawArrays(GL_QUADS, water_buffer_reflectiv_index*4, (stop-start) * 4);
            
            if (use_vertex_buffers)
            {
                ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
                glDisableClientState(GL_VERTEX_ARRAY);
            }
        
            glStencilFunc(GL_EQUAL, 1, 1);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glDepthMask(GL_TRUE);
        }
		else
		{
			clip_sky = 1;
		}
	}

	glCullFace(GL_FRONT);
	glScalef(1.0f, 1.0f, -1.0f);
	glTranslatef(0.0f, 0.0f, -water_depth_offset);
	glNormal3f(0.0f, 0.0f, 1.0f);

	glLightfv(GL_LIGHT7, GL_POSITION, sun_position);
	if (skybox_show_sky)
	{
        glPushMatrix();
        glTranslatef(0.0, 0.0, -skybox_get_z_position());
		if (!clip_sky)
		{
			skybox_display();
		}
		else
		{
			GLdouble clip_plane[] = {0.0, 0.0, -water_depth_offset, 0.0};
			glClipPlane(GL_CLIP_PLANE0, clip_plane);
			glEnable(GL_CLIP_PLANE0);
			skybox_display();
			glDisable(GL_CLIP_PLANE0);
		}
        glPopMatrix();
	}

	if (far_reflection_plane > 0.0)
	{
		weather_init_lightning_light();

	cur_intersect_type = get_cur_intersect_type(main_bbox_tree);
	set_cur_intersect_type(main_bbox_tree, INTERSECTION_TYPE_REFLECTION);
	calculate_reflection_frustum(water_depth_offset);

	enable_reflection_clip_planes();

//	draw_tile_map();
//	display_2d_objects();
	display_objects();
	display_ground_objects();
#ifndef MAP_EDITOR2
	display_actors(0, REFLECTION_RENDER_PASS);
#endif
	display_alpha_objects();
//	display_blended_objects();
	set_cur_intersect_type(main_bbox_tree, cur_intersect_type);
#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixd(skybox_view);
	glMatrixMode(GL_MODELVIEW);

	weather_render_lightning();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	disable_reflection_clip_planes();

	}

	glPopMatrix();
	glCullFace(GL_BACK);
	CHECK_GL_ERRORS();

	if (use_frame_buffer && water_shader_quality > 0)
	{
		CHECK_GL_ERRORS();
		CHECK_FBO_ERRORS();
		ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glViewport(view_port[0], view_port[1], view_port[2], view_port[3]);
		CHECK_GL_ERRORS();
		CHECK_FBO_ERRORS();
	}
    else if (have_stencil)
    {
		glDisable(GL_STENCIL_TEST);
	}
	glLightfv(GL_LIGHT7, GL_POSITION, sun_position);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

#ifndef MAP_EDITOR2
void blend_reflection_fog()
{
	static GLfloat blendColor[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
	if (use_frame_buffer && water_shader_quality > 0)
	{
		return;
	}

	build_water_buffer();

	if (water_buffer_usage == 0) return;

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, water_depth_offset);

	// we write to the depth buffer later, for now keep it as it is to avoid flickering
	glDepthMask(GL_FALSE);

	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glColor3f(0.0f, 0.0f, 0.0f); 
	
	glFogfv(GL_FOG_COLOR, blendColor);
	glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);

	if (use_vertex_buffers)
	{
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, water_tile_buffer_object);
		glInterleavedArrays(GL_V2F, 0, 0);
	}
	else
	{
		glInterleavedArrays(GL_V2F, 0, water_tile_buffer);
	}
	glDrawArrays(GL_QUADS, water_buffer_reflectiv_index * 4, (water_buffer_usage - water_buffer_reflectiv_index) * 4);

	// now add the fog by additive blending
	glFogfv(GL_FOG_COLOR, skybox_fog_color);
	glBlendFunc(GL_ONE, GL_ONE);
	
	glDrawArrays(GL_QUADS, water_buffer_reflectiv_index * 4, (water_buffer_usage - water_buffer_reflectiv_index) * 4);

	if (use_vertex_buffers)
	{
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	}
	glDisableClientState(GL_VERTEX_ARRAY);

	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	// ok, now we can write depth values
	glDepthMask(GL_TRUE);

	glPopMatrix();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}
#endif

#ifndef	NEW_TEXTURES
void draw_water_quad_tiles(unsigned int start, unsigned int stop, unsigned int idx, int water_id)
{
	unsigned int i, l, size;
	int x, y, cur_texture;
#ifdef CLUSTER_INSIDES_OLD
	short cluster = get_actor_cluster ();
	short tile_cluster;
#endif

	size = 0;
	cur_texture = last_texture;

	for (i = start; i < stop; i++)
	{
		l = get_intersect_item_ID(main_bbox_tree, i);
		x = get_terrain_x(l);
		y = get_terrain_y(l);

#ifdef CLUSTER_INSIDES_OLD
		tile_cluster = get_cluster (6*x, 6*y);
		if (tile_cluster && tile_cluster != cluster)
			continue;
#endif

		if (!tile_map[y * tile_map_size_x + x])
		{
			cur_texture = get_texture_id(water_id);
		}
		else
		{
			cur_texture = get_texture_id(tile_list[tile_map[y * tile_map_size_x + x]]);
		}
		if (cur_texture != last_texture)
		{
			glDrawArrays(GL_QUADS, idx * 4, size * 4);
			bind_texture_id(cur_texture);
			cur_texture = last_texture;
			idx += size;
			size = 0;
		}
		size++;
	}
	glDrawArrays(GL_QUADS, idx * 4, size * 4);
}
#endif	/* NEW_TEXTURES */

void draw_lake_tiles()
{
	unsigned int start, stop;
	int water_id;
	float noise_scale[4] = {0.125f, 0.125f, 0.0625f, 0.0625f};
	GLint idx;
	GLhandleARB cur_shader;

	build_water_buffer();
	CHECK_GL_ERRORS();

	if (water_buffer_usage == 0) return;

	glEnable(GL_CULL_FACE);

	if (dungeon) water_id = tile_list[231];
	else water_id = tile_list[0];

	if (use_frame_buffer && (water_shader_quality > 0) && show_reflection)
	{
		if (!dungeon && shadows_on && (is_day || lightning_falling))
		{
			cur_shader = get_shader(st_water, sst_shadow_receiver, use_fog, water_shader_quality - 1);
		}
		else
		{
			cur_shader = get_shader(st_water, sst_no_shadow_receiver, use_fog, water_shader_quality - 1);
		}
		ELglUseProgramObjectARB(cur_shader);
		CHECK_GL_ERRORS();

		if (water_shader_quality > 1)
		{
			ELglClientActiveTextureARB(GL_TEXTURE3);
			ELglActiveTextureARB(GL_TEXTURE3);
			glEnable(GL_TEXTURE_3D);
			glBindTexture(GL_TEXTURE_3D, noise_tex);
			ELglActiveTextureARB(base_unit);
			setup_noise_texgen();
			CHECK_GL_ERRORS();

			ELglUniform1iARB(ELglGetUniformLocationARB(cur_shader, "noise_texture"), 3);
			ELglUniform4fvARB(ELglGetUniformLocationARB(cur_shader, "noise_scale"), 1, noise_scale);
			ELglUniform1fARB(ELglGetUniformLocationARB(cur_shader, "time"), cur_time / 23725.0f);
			CHECK_GL_ERRORS();
		}
		if (!dungeon && shadows_on && (is_day || lightning_falling))
		{
			ELglUniform1iARB(ELglGetUniformLocationARB(cur_shader, "shadow_texture"), shadow_unit - GL_TEXTURE0);
		}
		ELglUniform1iARB(ELglGetUniformLocationARB(cur_shader, "tile_texture"), base_unit - GL_TEXTURE0);
		CHECK_GL_ERRORS();
	}
	CHECK_GL_ERRORS();

	setup_water_texgen();

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, water_depth_offset);

	if (use_vertex_buffers)
	{
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, water_tile_buffer_object);
		glInterleavedArrays(GL_V2F, 0, 0);
	}
	else
	{
		glInterleavedArrays(GL_V2F, 0, water_tile_buffer);
	}

	get_intersect_start_stop(main_bbox_tree, TYPE_NO_REFLECTIV_WATER, &start, &stop);
#ifdef	FSAA
	if (fsaa > 1)
	{
		glEnable(GL_MULTISAMPLE);
	}
#endif	/* FSAA */
#ifdef	NEW_TEXTURES
	draw_quad_tiles(start, stop, 0, water_id);
#else	/* NEW_TEXTURES */
	draw_water_quad_tiles(start, stop, 0, water_id);
#endif	/* NEW_TEXTURES */
#ifdef	FSAA
	if (fsaa > 1)
	{
		glDisable(GL_MULTISAMPLE);
	}
#endif	/* FSAA */

	if (use_frame_buffer && (water_shader_quality > 0) && show_reflection)
	{
		setup_water_fbo_texgen();
		CHECK_GL_ERRORS();

		ELglActiveTextureARB(detail_unit);
		glBindTexture(GL_TEXTURE_2D, water_reflection_fbo_texture);
		ELglActiveTextureARB(base_unit);
		CHECK_GL_ERRORS();

		if (!dungeon && shadows_on && (is_day || lightning_falling))
		{
			cur_shader = get_shader(st_reflectiv_water, sst_shadow_receiver, use_fog, water_shader_quality - 1);
		}
		else
		{
			cur_shader = get_shader(st_reflectiv_water, sst_no_shadow_receiver, use_fog, water_shader_quality - 1);
		}
		ELglUseProgramObjectARB(cur_shader);
		CHECK_GL_ERRORS();

		if (water_shader_quality > 1)
		{
			ELglClientActiveTextureARB(GL_TEXTURE3);
			ELglActiveTextureARB(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_3D, noise_tex);
			ELglActiveTextureARB(base_unit);
			CHECK_GL_ERRORS();
			setup_noise_texgen();

			CHECK_GL_ERRORS();
			ELglUniform1iARB(ELglGetUniformLocationARB(cur_shader, "noise_texture"), 3);
			ELglUniform4fvARB(ELglGetUniformLocationARB(cur_shader, "noise_scale"), 1, noise_scale);
			ELglUniform1fARB(ELglGetUniformLocationARB(cur_shader, "time"), cur_time / 23725.0f);
			CHECK_GL_ERRORS();
		}
		idx = ELglGetUniformLocationARB(cur_shader, "texel_size_x");
		if (idx >= 0)
		{
			ELglUniform2fARB(idx, 1.0f / reflection_texture_width, 0.0f);
		}
		idx = ELglGetUniformLocationARB(cur_shader, "texel_size_y");
		if (idx >= 0)
		{
			ELglUniform2fARB(idx, 0.0f, 1.0f / reflection_texture_width);
		}
		idx = ELglGetUniformLocationARB(cur_shader, "size");
		if (idx >= 0)
		{
			ELglUniform2fARB(idx, reflection_texture_width, reflection_texture_height);
		}
		idx = ELglGetUniformLocationARB(cur_shader, "hg_texture");
		if (idx >= 0)
		{
			ELglActiveTextureARB(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_1D, filter_lut);
			ELglActiveTextureARB(base_unit);
			ELglUniform1iARB(idx, 4);
		}

		if (!dungeon && shadows_on && (is_day || lightning_falling))
		{
			ELglUniform1iARB(ELglGetUniformLocationARB(cur_shader, "shadow_texture"), shadow_unit - GL_TEXTURE0);
		}
		ELglUniform1iARB(ELglGetUniformLocationARB(cur_shader, "reflection_texture"), detail_unit - GL_TEXTURE0);
		ELglUniform1iARB(ELglGetUniformLocationARB(cur_shader, "tile_texture"), base_unit - GL_TEXTURE0);
		ELglUniform1fARB(ELglGetUniformLocationARB(cur_shader, "blend"), 0.75f);
		CHECK_GL_ERRORS();
	}
	else /* if (show_reflection) */
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	}

	get_intersect_start_stop(main_bbox_tree, TYPE_REFLECTIV_WATER, &start, &stop);
#ifdef	FSAA
	if (fsaa > 1)
	{
		glEnable(GL_MULTISAMPLE);
	}
#endif	/* FSAA */
#ifdef	NEW_TEXTURES
	draw_quad_tiles(start, stop, water_buffer_reflectiv_index, water_id);
#else	/* NEW_TEXTURES */
	draw_water_quad_tiles(start, stop, water_buffer_reflectiv_index, water_id);
#endif	/* NEW_TEXTURES */
#ifdef	FSAA
	if (fsaa > 1)
	{
		glDisable(GL_MULTISAMPLE);
	}
#endif	/* FSAA */

	if (use_frame_buffer && (water_shader_quality > 0) && show_reflection)
	{
		CHECK_GL_ERRORS();
		disable_water_fbo_texgen();

		ELglActiveTextureARB(detail_unit);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		ELglClientActiveTextureARB(base_unit);
		ELglActiveTextureARB(base_unit);
		glEnable(GL_TEXTURE_2D);
		last_texture = -1;
		CHECK_GL_ERRORS();

		ELglUseProgramObjectARB(0);

		if (water_shader_quality > 1)
		{
			ELglActiveTextureARB(GL_TEXTURE3);
			glDisable(GL_TEXTURE_3D);
			glBindTexture(GL_TEXTURE_3D, 0);
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
			ELglActiveTextureARB(base_unit);

		}
 		CHECK_GL_ERRORS();
	}
	else /* if (show_reflection) */
	{
		glDisable(GL_BLEND);
	}

	glDisable(GL_CULL_FACE);

	if (use_vertex_buffers)
	{
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	}
	glDisableClientState(GL_VERTEX_ARRAY);

	glPopMatrix();

	disable_water_texgen();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void draw_sky_background()
{
	static GLfloat lights_c[4][3];
#ifdef MAP_EDITOR2
	int i;
#else // MAP_EDITOR2
	int i;
	float weather_bias = (1.0-weather_get_density());
#endif // MAP_EDITOR2
	GLint view_port[4];

	glDisable(GL_TEXTURE_2D);
	if (use_frame_buffer && (water_shader_quality > 0) && show_reflection)
	{
		CHECK_GL_ERRORS();
		CHECK_FBO_ERRORS();
		init_texturing();
		CHECK_GL_ERRORS();
		glGetIntegerv(GL_VIEWPORT, view_port);
		CHECK_GL_ERRORS();
		ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, water_reflection_fbo);
		CHECK_GL_ERRORS();
		CHECK_FBO_ERRORS();
		glViewport(0, 0, reflection_texture_width, reflection_texture_height);
		CHECK_GL_ERRORS();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		CHECK_GL_ERRORS();
		Enter2DModeExtended(reflection_texture_width, reflection_texture_height);
		CHECK_GL_ERRORS();
		CHECK_FBO_ERRORS();
	}
	else
	{
		Enter2DMode();
	}

	for (i = 0; i < 3; i++) {
		// get the sky color
		lights_c[0][i] = sky_lights_c1[light_level][i]*weather_bias;
		lights_c[1][i] = sky_lights_c2[light_level][i]*weather_bias;
		lights_c[2][i] = sky_lights_c3[light_level][i]*weather_bias;
		lights_c[3][i] = sky_lights_c4[light_level][i]*weather_bias;
	}

	if (!skybox_show_sky)
	{
		glBegin(GL_QUADS);

		if (use_frame_buffer && (water_shader_quality > 0) && show_reflection)
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
	}

	Leave2DMode();
	if (use_frame_buffer && (water_shader_quality > 0) && show_reflection)
	{
		CHECK_GL_ERRORS();
		CHECK_FBO_ERRORS();
		ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glViewport(view_port[0], view_port[1], view_port[2], view_port[3]);
		CHECK_GL_ERRORS();
		CHECK_FBO_ERRORS();
	}
	glEnable(GL_TEXTURE_2D);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void draw_dungeon_sky_background()
{
	static const GLfloat baseColor[3] = { 0.00f, 0.21f, 0.34f };
#ifndef MAP_EDITOR2
	static GLfloat color[3];
	int i;
	float weather_density = weather_get_density();
#endif // MAP_EDITOR2
	GLint view_port[4];

	glDisable(GL_TEXTURE_2D);
	if (use_frame_buffer && (water_shader_quality > 0) && show_reflection)
	{
		CHECK_GL_ERRORS();
		CHECK_FBO_ERRORS();
		init_texturing();
		CHECK_GL_ERRORS();
		glGetIntegerv(GL_VIEWPORT, view_port);
		CHECK_GL_ERRORS();
		ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, water_reflection_fbo);
		CHECK_GL_ERRORS();
		CHECK_FBO_ERRORS();
		glViewport(0, 0, reflection_texture_width, reflection_texture_height);
		CHECK_GL_ERRORS();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		CHECK_GL_ERRORS();
		Enter2DModeExtended(reflection_texture_width, reflection_texture_height);
		CHECK_GL_ERRORS();
		CHECK_FBO_ERRORS();
	}
	else
	{
		Enter2DMode();
	}

#ifdef MAP_EDITOR2
	glColor3fv(baseColor);
#else // MAP EDITOR 2

	for (i=0; i<3; i++) {
		color[i] = baseColor[i] * ((1.0 - weather_density) + weather_color[i]*weather_density);
	}

	glColor3fv(color);
#endif // MAP_EDITOR
	
	glBegin(GL_QUADS);
	//draw the sky background

	if (use_frame_buffer && (water_shader_quality > 0) && show_reflection)
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

	Leave2DMode();
	if (use_frame_buffer && (water_shader_quality > 0) && show_reflection)
	{
		CHECK_GL_ERRORS();
		CHECK_FBO_ERRORS();
		ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glViewport(view_port[0], view_port[1], view_port[2], view_port[3]);
		CHECK_GL_ERRORS();
		CHECK_FBO_ERRORS();
	}
	glEnable(GL_TEXTURE_2D);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void draw_water_background()
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

    if (dungeon)
        glColor3f(0.00f, 0.21f, 0.34f);
    else
        glColor3fv(skybox_sky_color);

	if (use_frame_buffer && (water_shader_quality > 0) && show_reflection)
	{
		GLint view_port[4];

		CHECK_GL_ERRORS();
		CHECK_FBO_ERRORS();
		init_texturing();
		CHECK_GL_ERRORS();
		glGetIntegerv(GL_VIEWPORT, view_port);
		CHECK_GL_ERRORS();
		ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, water_reflection_fbo);
		CHECK_GL_ERRORS();
		CHECK_FBO_ERRORS();
		glViewport(0, 0, reflection_texture_width, reflection_texture_height);
		CHECK_GL_ERRORS();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		CHECK_GL_ERRORS();

		if (!skybox_show_sky || dungeon)
		{
			Enter2DModeExtended(reflection_texture_width, reflection_texture_height);
			glBegin(GL_QUADS);
			glVertex3i(0, 0, 0);
			glVertex3i(0, reflection_texture_height, 0);
			glVertex3i(reflection_texture_width, reflection_texture_height, 0);
			glVertex3i(reflection_texture_width, 0, 0);
			glEnd();
			Leave2DMode();
		}

		ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glViewport(view_port[0], view_port[1], view_port[2], view_port[3]);
		CHECK_GL_ERRORS();
		CHECK_FBO_ERRORS();
	}
	else
	{
		unsigned int start, stop;

		build_water_buffer();
		CHECK_GL_ERRORS();

		if (water_buffer_usage != 0)
		{
			glPushMatrix();
			glTranslatef(0.0f, 0.0f, water_depth_offset);
			
			if (use_vertex_buffers)
			{
				ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, water_tile_buffer_object);
				glInterleavedArrays(GL_V2F, 0, 0);
			}
			else
			{
				glInterleavedArrays(GL_V2F, 0, water_tile_buffer);
			}
			
			get_intersect_start_stop(main_bbox_tree, TYPE_REFLECTIV_WATER, &start, &stop);
			glDrawArrays(GL_QUADS, water_buffer_reflectiv_index*4, (stop-start) * 4);
            
			if (use_vertex_buffers)
			{
				ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
				glDisableClientState(GL_VERTEX_ARRAY);
			}
			glPopMatrix();
		}
	}
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}
