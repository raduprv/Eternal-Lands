/**
 * @file
 * @ingroup load_3d
 * @brief the E3D data format and supporting functions.
 */

#include "e3d.h"
#include "gl_init.h"
#include "shadows.h"
#include "map_io.h"

float zero[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

void e3d_enable_vertex_arrays(e3d_object *e3d_data, Uint32 use_lightning, Uint32 use_textures)
{
	e3d_vertex_data* vertex_layout;
	Uint8 * data_ptr;

	if (have_vertex_buffers)
	{
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, e3d_data->vertex_vbo);
		data_ptr = 0;
	}
	else
	{
		data_ptr = e3d_data->vertex_data;
	}

	vertex_layout = e3d_data->vertex_layout;

	if ((vertex_layout->normal_count > 0) && use_lightning)
	{
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(vertex_layout->normal_type, vertex_layout->size,
			data_ptr + vertex_layout->normal_offset);
	}
	else
	{
		glDisableClientState(GL_NORMAL_ARRAY);
		glNormal3f(0.0f, 0.0f, 1.0f);
	}

	if (vertex_layout->color_count > 0)
	{
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(vertex_layout->color_count, vertex_layout->color_type,
			vertex_layout->size, data_ptr + vertex_layout->color_offset);
	}
	else
	{
		glDisableClientState(GL_COLOR_ARRAY);
	}

	if (use_textures)
	{
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(vertex_layout->texture_count, vertex_layout->texture_type,
			vertex_layout->size, data_ptr + vertex_layout->texture_offset);
	}
	else
	{
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(vertex_layout->position_count, vertex_layout->position_type,
		vertex_layout->size, data_ptr + vertex_layout->position_offset);

	if (have_vertex_buffers)
	{
		ELglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
			e3d_data->indices_vbo);
	}
}

void e3d_disable_vertex_arrays()
{
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void set_emission(object3d * object_id)
{
	if(object_id->self_lit && (night_shadows_on || dungeon))
	{
		glDisable(GL_LIGHTING);
		glMaterialfv(GL_FRONT, GL_EMISSION, object_id->color);
	}
	else
	{
		glMaterialfv(GL_FRONT, GL_EMISSION, zero);
	}

	glEnable(GL_COLOR_MATERIAL);
}
