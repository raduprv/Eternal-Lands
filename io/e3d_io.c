#include <float.h>
#include <SDL_endian.h>
#include "e3d_io.h"
#include "../asc.h"
#include "../platform.h"
#include "../textures.h"
#ifdef MAP_EDITOR
 #include "../map_editor/misc.h"
#else
 #include "../misc.h"
#endif
#include "../errors.h"
#include "elfilewrapper.h"
#include "normal.h"
#include "half.h"

static Uint32 has_normal(const Uint32 options)
{
	return (options & 0x01) != 0;
}

static Uint32 has_tangent(const Uint32 options)
{
	return (options & 0x02) != 0;
}

static Uint32 has_secondary_texture_coordinate(const Uint32 options)
{
	return (options & 0x04) != 0;
}

static Uint32 has_color(const Uint32 options)
{
	return (options & 0x08) != 0;
}

static Uint32 half_position(const Uint32 format)
{
	return (format & 0x01) != 0;
}

static Uint32 half_uv(const Uint32 format)
{
	return (format & 0x02) != 0;
}

static Uint32 half_extra_uv(const Uint32 format)
{
	return (format & 0x04) != 0;
}

static Uint32 compressed_normal(const Uint32 format)
{
	return (format & 0x08) != 0;
}

static Uint32 short_index(const Uint32 format)
{
	return (format & 0x10) != 0;
}

static Uint32 check_vertex_size(const Uint32 size, const Uint32 options, const Uint32 format)
{
	Uint32 tmp;

	tmp = 0;

	if (half_position(format))
	{
		tmp += 3 * sizeof(Uint16);
	}
	else
	{
		tmp += 3 * sizeof(float);
	}
	if (half_uv(format))
	{
		tmp += 2 * sizeof(Uint16);
	}
	else
	{
		tmp += 2 * sizeof(float);
	}

	if (has_normal(options))
	{
		if (compressed_normal(format))
		{
			tmp += sizeof(Uint16);
		}
		else
		{
			tmp += 3 * sizeof(float);
		}
	}

	if (has_tangent(options))
	{
		if (compressed_normal(format))
		{
			tmp += sizeof(Uint16);
		}
		else
		{
			tmp += 3 * sizeof(float);
		}
	}

	if (has_secondary_texture_coordinate(options))
	{
		if (half_extra_uv(format))
		{
			tmp += 2 * sizeof(Uint16);
		}
		else
		{
			tmp += 2 * sizeof(float);
		}
	}

	if (has_color(options))
	{
		tmp += 4 * sizeof(Uint8);
	}

	return tmp == size;
}

static Uint32 get_material_size(const Uint32 options)
{
	if (has_secondary_texture_coordinate(options))
	{
		return sizeof(e3d_material) + sizeof(e3d_extra_texture);
	}
	else
	{
		return sizeof(e3d_material);
	}
}

e3d_vertex_data vertex_layout_array[] =
{
	{
		sizeof(float) * 2, 0, 0, 0,
		sizeof(float) * 5, 3, 2, 0, 0,
		GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_UNSIGNED_BYTE
	},
	{
		sizeof(float) * 5, 0, sizeof(float) * 2, 0,
		sizeof(float) * 8, 3, 2, 3, 0,
		GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_UNSIGNED_BYTE
	},
	{
		sizeof(float) * 2, 0, 0, sizeof(float) * 5,
		sizeof(float) * 5 + sizeof(Uint8) * 4, 3, 2, 0, 4,
		GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_UNSIGNED_BYTE
	},
	{
		sizeof(float) * 5, 0, sizeof(float) * 2, sizeof(float) * 8,
		sizeof(float) * 8 + sizeof(Uint8) * 4, 3, 2, 3, 4,
		GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_UNSIGNED_BYTE
	}
};

static void read_vertex_buffer(el_file_ptr file, float* buffer, const Uint32 vertex_count,
	const Uint32 vertex_size, const Uint32 options, const Uint32 format)
{
	float temp[3];
	Uint16 tmp[3];
	Uint32 i, idx, offset;
	Uint8 color[4];

	idx = 0;

	offset = el_tell(file);

	for (i = 0; i < vertex_count; i++)
	{
		el_seek(file, offset + i * vertex_size, SEEK_SET);

		if (half_uv(format))
		{
			el_read(file, 2 * sizeof(Uint16), tmp);

			temp[0] = half_to_float(SDL_SwapLE16(tmp[0]));
			temp[1] = half_to_float(SDL_SwapLE16(tmp[1]));
		}
		else
		{
			el_read(file, 2 * sizeof(float), temp);

			temp[0] = SwapLEFloat(temp[0]);
			temp[1] = SwapLEFloat(temp[1]);
		}

		buffer[idx + 0] = temp[0];
		buffer[idx + 1] = temp[1];

		idx += 2;

		if (has_secondary_texture_coordinate(options))
		{
			if (half_extra_uv(format))
			{
				el_seek(file, 2 * sizeof(Uint16), SEEK_CUR);
			}
			else
			{
				el_seek(file, 2 * sizeof(float), SEEK_CUR);
			}
		}

		if (has_normal(options))
		{
			if (compressed_normal(format))
			{
				el_read(file, sizeof(Uint16), tmp);

				uncompress_normal(SDL_SwapLE16(tmp[0]), temp);
			}
			else
			{
				el_read(file, 3 * sizeof(float), temp);

				temp[0] = SwapLEFloat(temp[0]);
				temp[1] = SwapLEFloat(temp[1]);
				temp[2] = SwapLEFloat(temp[2]);
			}

			buffer[idx + 0] = temp[0];
			buffer[idx + 1] = temp[1];
			buffer[idx + 2] = temp[2];

			idx += 3;
		}

		if (has_tangent(options))
		{
			if (compressed_normal(format))
			{
				el_seek(file, sizeof(Uint16), SEEK_CUR);
			}
			else
			{
				el_seek(file, 3 * sizeof(float), SEEK_CUR);
			}
		}

		if (half_position(format))
		{
			el_read(file, 3 * sizeof(Uint16), tmp);

			temp[0] = half_to_float(SDL_SwapLE16(tmp[0]));
			temp[1] = half_to_float(SDL_SwapLE16(tmp[1]));
			temp[2] = half_to_float(SDL_SwapLE16(tmp[2]));
		}
		else
		{
			el_read(file, 3 * sizeof(float), temp);

			temp[0] = SwapLEFloat(temp[0]);
			temp[1] = SwapLEFloat(temp[1]);
			temp[2] = SwapLEFloat(temp[2]);
		}

		buffer[idx + 0] = temp[0];
		buffer[idx + 1] = temp[1];
		buffer[idx + 2] = temp[2];

		idx += 3;

		if (has_color(options))
		{
			el_read(file, 4 * sizeof(Uint8), color);
			memcpy(&buffer[idx], color, 4 * sizeof(Uint8));
			idx += 1;
		}
	}
}

static void free_e3d_pointer(e3d_object* cur_object)
{
	if (cur_object != 0)
	{
		if (cur_object->vertex_data != 0)
		{
			free(cur_object->vertex_data);
			cur_object->vertex_data = 0;
		}
		if (cur_object->indices != 0)
		{
			free(cur_object->indices);
			cur_object->indices = 0;
		}
		if (cur_object->materials != 0)
		{
			free(cur_object->materials);
			cur_object->materials = 0;
		}
		free(cur_object);
	}
}

static int check_pointer(void* ptr, e3d_object* cur_object, const char* str, el_file_ptr file)
{
	if (ptr == 0)
	{
		LOG_ERROR("Can't allocate memory for %s!", str);
		free_e3d_pointer(cur_object);
		el_close(file);
		return 0;
	}
	else
	{
		return 1;
	}
}

#define CHECK_POINTER(ptr, str) check_pointer((ptr), cur_object, (str), file)

static e3d_object* do_load_e3d_detail(e3d_object* cur_object)
{
	e3d_header header;
	e3d_material material;
	char cur_dir[1024];
	int i, idx, l, mem_size, vertex_size, material_size;
	int file_pos, indices_size, index_size;
	char text_file_name[1024];
	Uint32 tmp;
	Uint16 tmp_16;
	Uint8* index_pointer;
	el_file_ptr file;
	version_number version;
	
	if (cur_object == 0) return 0;

	memset(cur_dir, 0, sizeof(cur_dir));
	//get the current directory
	l = strlen(cur_object->file_name);
	//parse the string backwards, until we find a /
	while (l > 0)
	{
		if ((cur_object->file_name[l] == '/') || (cur_object->file_name[l] == '\\')) break;
		l--;
	}

	i = 0;
	if (l)//prevent invalid dir names
	{
		while (l >= 0)
		{
			cur_dir[i] = cur_object->file_name[i];
			i++;
			l--;
		}
		cur_dir[i+1] = 0;
	}

	LOG_DEBUG("Loading e3d file '%s'.", cur_object->file_name);

	file = el_open(cur_object->file_name);
	if (file == 0)
	{
		LOG_ERROR("Can't open file '%s'!", cur_object->file_name);

		free_e3d_pointer(cur_object);

		return 0;
	}

	if (read_and_check_elc_header(file, EL3D_FILE_MAGIC_NUMBER, &version, cur_object->file_name) != 0)
	{
		LOG_ERROR("File '%s' has wrong header!", cur_object->file_name);

		free_e3d_pointer(cur_object);
		el_close(file);

		return 0;
	}
	
	el_read(file, sizeof(e3d_header), &header);

	cur_object->vertex_no = SDL_SwapLE32(header.vertex_no);
	cur_object->index_no = SDL_SwapLE32(header.index_no);
	cur_object->material_no = SDL_SwapLE32(header.material_no);
	vertex_size = SDL_SwapLE32(header.vertex_size);
	material_size = SDL_SwapLE32(header.material_size);
	index_size = SDL_SwapLE32(header.index_size);

	LOG_DEBUG("E3d file vertex count %d and size %d.",
		cur_object->vertex_no, vertex_size);

	LOG_DEBUG("E3d file index count %d and size %d.",
		cur_object->index_no, index_size);

	LOG_DEBUG("E3d file material count %d and size %d.",
		cur_object->material_no, material_size);

	LOG_DEBUG("E3d file version %d.%d.%d.%d.", version[0],
		version[1], version[2], version[3]);

	if ((version[0] == 1) && (version[1] == 1))
	{
		if ((header.vertex_options & 0xF0) != 0)
		{
			LOG_ERROR("Unknow options (%d) for file %s.",
				header.vertex_options, cur_object->file_name);
		}

		header.vertex_options &= 0x0F;

		if ((header.vertex_format & 0xE0) != 0)
		{
			LOG_ERROR("Unknow format (%d) for file %s.",
				header.vertex_format, cur_object->file_name);
		}

		header.vertex_format &= 0x1F;
	}
	else
	{
		if ((version[0] == 1) && (version[1] == 0))
		{
			header.vertex_format = 0;
			header.vertex_options ^= 0x01;
			header.vertex_options &= 0x07;
		}
		else
		{
			LOG_ERROR("File '%s' has wrong version number!",
				cur_object->file_name);

			free_e3d_pointer(cur_object);
			el_close(file);

			return 0;
		}
	}

	idx = 0;

	if (has_normal(header.vertex_options))
	{
		idx += 1;
	}

	if (has_color(header.vertex_options))
	{
		idx += 2;
	}

	cur_object->vertex_layout = &(vertex_layout_array[idx]);

	// They have at least the size we expected

	if (check_vertex_size(vertex_size, header.vertex_options, header.vertex_format) == 0)
	{
		LOG_ERROR("File '%s' has wrong vertex size!", cur_object->file_name);

		free_e3d_pointer(cur_object);
		el_close(file);

		return 0;
	}

	if (material_size != get_material_size(header.vertex_options))
	{
		LOG_ERROR("File '%s' has wrong material size! Expected size %d, found size %d.",
			cur_object->file_name, get_material_size(header.vertex_options), material_size);
		free_e3d_pointer(cur_object);
		el_close(file);
		return 0;
	}

	if (short_index(header.vertex_format))
	{
		if (index_size != sizeof(Uint16))
		{
			LOG_ERROR("File '%s' has wrong index size! Expected size %d, found size %d.",
				cur_object->file_name, sizeof(Uint16), index_size);
			free_e3d_pointer(cur_object);
			el_close(file);
			return 0;
		}
	}
	else
	{
		if (index_size != sizeof(Uint32))
		{
			LOG_ERROR("File '%s' has wrong index size! Expected size %d, found size %d.",
				cur_object->file_name, sizeof(Uint32), index_size);
			free_e3d_pointer(cur_object);
			el_close(file);
			return 0;
		}
	}

	LOG_DEBUG("Reading vertices at %d from e3d file '%s'.",
		SDL_SwapLE32(header.vertex_offset), cur_object->file_name);
	// Now reading the vertices
	el_seek(file, SDL_SwapLE32(header.vertex_offset), SEEK_SET);

	cur_object->vertex_data = malloc(cur_object->vertex_no * cur_object->vertex_layout->size);
	mem_size = cur_object->vertex_no * cur_object->vertex_layout->size;
	if (!CHECK_POINTER(cur_object->vertex_data, "vertex data")) return 0;

	read_vertex_buffer(file, (float*)(cur_object->vertex_data), cur_object->vertex_no,
		vertex_size, header.vertex_options, header.vertex_format);

	LOG_DEBUG("Reading indices at %d from e3d file '%s'.",
		SDL_SwapLE32(header.index_offset), cur_object->file_name);
	// Now reading the indices
	el_seek(file, SDL_SwapLE32(header.index_offset), SEEK_SET);

	if (cur_object->index_no < 65536)
	{
		indices_size = 2;
		cur_object->index_type = GL_UNSIGNED_SHORT;
	}
	else
	{
		indices_size = 4;
		cur_object->index_type = GL_UNSIGNED_INT;
	}

	cur_object->indices = malloc(cur_object->index_no * indices_size);

	if (use_vertex_buffers) index_pointer = 0;
	else index_pointer = cur_object->indices;

	for (i = 0; i < cur_object->index_no; i++)
	{
		if (index_size == 2)
		{
			el_read(file, sizeof(Uint16), &tmp_16);
			tmp = SDL_SwapLE16(tmp_16);
		}
		else
		{
			el_read(file, sizeof(Uint32), &tmp);
			tmp = SDL_SwapLE32(tmp);
		}

		if (indices_size == 2)
		{
			((Uint16*)(cur_object->indices))[i] = tmp;
		}
		else
		{
			((Uint32*)(cur_object->indices))[i] = tmp;
		}
	}

	// only allocate the materials structure if it doesn't exist (on initial load)
	if (cur_object->materials == 0)
	{
		cur_object->materials = (e3d_draw_list*)malloc(cur_object->material_no*sizeof(e3d_draw_list));
		if (!CHECK_POINTER(cur_object->materials, "materials")) return 0;
		memset(cur_object->materials, 0, cur_object->material_no * sizeof(e3d_draw_list));
	}
	mem_size += cur_object->material_no * sizeof(e3d_draw_list);

	LOG_DEBUG("Reading materials at %d from e3d file '%s'.",
		SDL_SwapLE32(header.material_offset), cur_object->file_name);
	// Now reading the materials
	el_seek(file, SDL_SwapLE32(header.material_offset), SEEK_SET);
	
	cur_object->min_x = 1e10f;
	cur_object->min_y = 1e10f;
	cur_object->min_z = 1e10f;
	cur_object->max_x = -1e10f;
	cur_object->max_y = -1e10f;
	cur_object->max_z = -1e10f;
	cur_object->max_size = -1e10f;

	for (i = 0; i < cur_object->material_no; i++)
	{
		
		file_pos = el_tell(file);
		el_read(file, sizeof(e3d_material), &material);
		safe_snprintf(text_file_name, sizeof(text_file_name), "%s%s", cur_dir, material.material_name);

		cur_object->materials[i].options = SDL_SwapLE32(material.options);
#ifdef	MAP_EDITOR
#ifdef	NEW_TEXTURES
		cur_object->materials[i].texture = load_texture_cached(text_file_name, tt_mesh);
#else	/* NEW_TEXTURES */
		cur_object->materials[i].texture = load_texture_cache(text_file_name,0);
#endif	/* NEW_TEXTURES */
#else	//MAP_EDITOR
#ifdef	NEW_TEXTURES
		cur_object->materials[i].texture = load_texture_cached(text_file_name, tt_mesh);
#else	/* NEW_TEXTURES */
#ifdef	NEW_ALPHA
		// prepare to load the textures depending on if it is transparent or not (diff alpha handling)
		if (material_is_transparent(cur_object->materials[i].options))
		{	// is this object transparent?
			cur_object->materials[i].texture= load_texture_cache_deferred(text_file_name, -1);
		}
		else
		{
			cur_object->materials[i].texture= load_texture_cache_deferred(text_file_name, -1);	//255);
		}
#else	//NEW_ALPHA
//		cur_object->materials[i].texture = load_texture_cache_deferred(text_file_name, 255);
		cur_object->materials[i].texture = load_texture_cache_deferred(text_file_name, 0);
#endif	//NEW_ALPHA
#endif	/* NEW_TEXTURES */
#endif	//MAP_EDITOR

		cur_object->materials[i].min_x = SwapLEFloat(material.min_x);
		cur_object->materials[i].min_y = SwapLEFloat(material.min_y);
		cur_object->materials[i].min_z = SwapLEFloat(material.min_z);
		cur_object->materials[i].max_x = SwapLEFloat(material.max_x);
		cur_object->materials[i].max_y = SwapLEFloat(material.max_y);
		cur_object->materials[i].max_z = SwapLEFloat(material.max_z);
		// calculate the max size for cruse LOD processing
		cur_object->materials[i].max_size= max2f(max2f(cur_object->materials[i].max_x-cur_object->materials[i].min_x, cur_object->materials[i].max_y-cur_object->materials[i].min_y), cur_object->materials[i].max_z-cur_object->materials[i].min_z);

		cur_object->min_x = min2f(cur_object->min_x, cur_object->materials[i].min_x);
		cur_object->min_y = min2f(cur_object->min_y, cur_object->materials[i].min_y);
		cur_object->min_z = min2f(cur_object->min_z, cur_object->materials[i].min_z);
		cur_object->max_x = max2f(cur_object->max_x, cur_object->materials[i].max_x);
		cur_object->max_y = max2f(cur_object->max_y, cur_object->materials[i].max_y);
		cur_object->max_z = max2f(cur_object->max_z, cur_object->materials[i].max_z);
		cur_object->max_size = max2f(cur_object->max_size, cur_object->materials[i].max_size);

		cur_object->materials[i].triangles_indices_index = indices_size*SDL_SwapLE32(material.index) + index_pointer;
		cur_object->materials[i].triangles_indices_count = SDL_SwapLE32(material.count);
		cur_object->materials[i].triangles_indices_min = SDL_SwapLE32(material.triangles_min_index);
		cur_object->materials[i].triangles_indices_max = SDL_SwapLE32(material.triangles_max_index);

		file_pos += SDL_SwapLE32(material_size);

		el_seek(file, file_pos, SEEK_SET);

	}
	el_close(file);

	LOG_DEBUG("Building vertex buffers (%d) for e3d file '%s'.",
		use_vertex_buffers, cur_object->file_name);

	if (use_vertex_buffers)
	{
		//Generate the buffers
		ELglGenBuffersARB(1, &cur_object->vertex_vbo);
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB,
			cur_object->vertex_vbo);
		ELglBufferDataARB(GL_ARRAY_BUFFER_ARB,
			cur_object->vertex_no * cur_object->vertex_layout->size,
			cur_object->vertex_data, GL_STATIC_DRAW_ARB);
#ifndef	MAP_EDITOR
		free(cur_object->vertex_data);
		cur_object->vertex_data = 0;
#endif	//MAP_EDITOR
		
		ELglGenBuffersARB(1, &cur_object->indices_vbo);
		ELglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
			cur_object->indices_vbo);
		ELglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
			cur_object->index_no * indices_size,
			cur_object->indices, GL_STATIC_DRAW_ARB);
#ifndef	MAP_EDITOR
		free(cur_object->indices);
		cur_object->indices = 0;
#endif	//MAP_EDITOR
				
		ELglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	}
	else
	{
		cur_object->vertex_vbo = 0;
		cur_object->indices_vbo = 0;
	}

#ifndef	MAP_EDITOR
	LOG_DEBUG("Adding e3d file '%s' to cache.",
		cur_object->file_name);

	cache_adj_size(cache_e3d, mem_size, cur_object);
#endif	//MAP_EDITOR
	return cur_object;
}

e3d_object* load_e3d_detail(e3d_object* cur_object)
{
	e3d_object* result;

	ENTER_DEBUG_MARK("load e3d");

	result = do_load_e3d_detail(cur_object);

	LEAVE_DEBUG_MARK("load e3d");

	return result;
}

