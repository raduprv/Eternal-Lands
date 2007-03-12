#ifdef NEW_E3D_FORMAT
#include "e3d_io.h"
#include <float.h>
#ifdef	ZLIB
#include	<zlib.h>
#endif

static void free_e3d_pointer(e3d_object* cur_object)
{
	if (cur_object != NULL)
	{
		if (cur_object->vertex_data != NULL)
		{
			free(cur_object->vertex_data);
			cur_object->vertex_data = NULL;
		}
		if (cur_object->normal_data != NULL)
		{
			free(cur_object->normal_data);
			cur_object->normal_data = NULL;
		}
		if (cur_object->texture_data != NULL)
		{
			free(cur_object->texture_data);
			cur_object->texture_data = NULL;
		}
		if (cur_object->tangent_data != NULL)
		{
			free(cur_object->tangent_data);
			cur_object->tangent_data = NULL;
		}
		if (cur_object->extra_uv_data != NULL)
		{
			free(cur_object->extra_uv_data);
			cur_object->extra_uv_data = NULL;
		}
		if (cur_object->indicies != NULL)
		{
			free(cur_object->indicies);
			cur_object->indicies = NULL;
		}
		if (cur_object->materials != NULL)
		{
			free(cur_object->materials);
			cur_object->materials = NULL;
		}
		free(cur_object);
	}
}

#ifdef	ZLIB
static int check_pointer(void* ptr, e3d_object* cur_object, const char* str, gzFile* file)
#else	//ZLIB
static int check_pointer(void* ptr, e3d_object* cur_object, const char* str, FILE* file)
#endif	//ZLIB
{
	if (ptr == NULL)
	{
		LOG_ERROR("Can't allocate memory for %s!", str);
		free_e3d_pointer(cur_object);
#ifdef	ZLIB
		gzclose(file);
#else	//ZLIB
		fclose(file);
#endif	//ZLIB
		return 0;
	}
	else
	{
		return 1;
	}
}

#define CHECK_POINTER(ptr, str) check_pointer((ptr), cur_object, (str), file)

#ifdef	ZLIB
static int check_size(int read_size, int size, e3d_object* cur_object, const char* filename, const char* str, gzFile* file)
#else	//ZLIB
static int check_size(int read_size, int size, e3d_object* cur_object, const char* filename, const char* str, FILE* file)
#endif	//ZLIB
{
	if (read_size < size)
	{
		LOG_ERROR("File \"%s\" has too small %s size!", filename, str);
		free_e3d_pointer(cur_object);
#ifdef	ZLIB
		gzclose(file);
#else	//ZLIB
		fclose(file);
#endif	//ZLIB
		return 0;
	}
	else
	{
		return 1;
	}
}

#define CHECK_SIZE(rsize, size, str) check_size((rsize), (size), cur_object, cur_object->file_name, (str), file)

e3d_object* load_e3d_detail(e3d_object* cur_object)
{
	e3d_header header;
	e3d_material material;
	char cur_dir[1024];
	int i, idx, l, mem_size, vertex_size, material_size, index_size, size;
	int file_pos, indicies_size;
	char text_file_name[1024];
	unsigned int* index_buffer;
	unsigned char* char_list;
	unsigned short* short_list;
	unsigned int* int_list;
	float* float_buffer;
	char* tmp_buffer;
	void* index_pointer;
	float* float_pointer;
#ifdef	ZLIB
	gzFile* file;
#else	//ZLIB
	FILE* file;
#endif	//ZLIB
	
	if (cur_object == NULL) return NULL;

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

#ifdef	ZLIB
	file= my_gzopen(cur_object->file_name, "rb");
#else	//ZLIB
	file= my_fopen(cur_object->file_name, "rb");
#endif	//ZLIB
	if (file == NULL)
	{
		LOG_ERROR("Can't open file \"%s\"!", cur_object->file_name);
		free_e3d_pointer(cur_object);
		return NULL;
	}

	if (read_and_check_elc_header(file, EL3D_FILE_MAGIC_NUMBER, EL3D_FILE_VERSION_NUMBER, cur_object->file_name) != 0)
	{
		LOG_ERROR("File \"%s\" has wrong header!", cur_object->file_name);
		free_e3d_pointer(cur_object);
#ifdef	ZLIB
		gzclose(file);
#else	//ZLIB
		fclose(file);
#endif	//ZLIB
		return NULL;
	}
	
#ifdef	ZLIB
	gzread(file, (char*)&header, sizeof(e3d_header));
#else	//ZLIB
	fread((char*)&header, 1, sizeof(e3d_header), file);
#endif	//ZLIB

	cur_object->vertex_no = SDL_SwapLE32(header.vertex_no);
	cur_object->index_no = SDL_SwapLE32(header.index_no);
	cur_object->material_no = SDL_SwapLE32(header.material_no);
	cur_object->vertex_options = header.vertex_options;
	vertex_size = SDL_SwapLE32(header.vertex_size);
	material_size = SDL_SwapLE32(header.material_size);
	index_size = SDL_SwapLE32(header.index_size);

	size = get_vertex_size(cur_object->vertex_options);

	// They have at least the size we expected
	if (!CHECK_SIZE(vertex_size, size, "vertex")) return NULL;
	if (!CHECK_SIZE(material_size, sizeof(e3d_material), "material")) return NULL;
	if (index_size != sizeof(unsigned int))
	{
		LOG_ERROR("File \"%s\" has wrong index size!", cur_object->file_name);
		free_e3d_pointer(cur_object);
#ifdef	ZLIB
		gzclose(file);
#else	//ZLIB
		fclose(file);
#endif	//ZLIB
		return NULL;
	}
	
	// Now reading the vertices
#ifdef	ZLIB
	gzseek(file, SDL_SwapLE32(header.vertex_offset), SEEK_SET);
#else	//ZLIB
	fseek(file, SDL_SwapLE32(header.vertex_offset), SEEK_SET);
#endif	//ZLIB

	cur_object->vertex_data = malloc(cur_object->vertex_no * 3 * sizeof(float));
	mem_size = cur_object->vertex_no * 3 * sizeof(float);
	if (!CHECK_POINTER(cur_object->vertex_data, "vertex data")) return NULL;

	cur_object->texture_data = malloc(cur_object->vertex_no * 2 * sizeof(float));
	mem_size += cur_object->vertex_no * 2 * sizeof(float);
	if (!CHECK_POINTER(cur_object->texture_data, "texture data")) return NULL;

	if (is_ground(cur_object->vertex_options))
	{
		cur_object->normal_data = NULL;
	}
	else
	{
		cur_object->normal_data = malloc(cur_object->vertex_no * 3 * sizeof(float));
		mem_size = cur_object->vertex_no * 3 * sizeof(float);
		if (!CHECK_POINTER(cur_object->normal_data, "normal data")) return NULL;
	}

	if (has_tangen(cur_object->vertex_options))
	{
		cur_object->tangent_data = malloc(cur_object->vertex_no * 3 * sizeof(float));
		mem_size += cur_object->vertex_no * 3 * sizeof(float);
		if (!CHECK_POINTER(cur_object->tangent_data, "tangent data")) return NULL;
	}
	else
	{
		cur_object->tangent_data = NULL;
	}

	if (has_extra_uv(cur_object->vertex_options))
	{
		cur_object->extra_uv_data = malloc(cur_object->vertex_no * 2 * sizeof(float));
		mem_size += cur_object->vertex_no * 2 * sizeof(float);
		if (!CHECK_POINTER(cur_object->extra_uv_data, "extra uv data")) return NULL;
	}
	else
	{
		cur_object->extra_uv_data = NULL;
	}

	float_buffer = (float*) malloc(size);
	if (!CHECK_POINTER(float_buffer, "float buffer")) return NULL;
	tmp_buffer = (char*) malloc(cur_object->vertex_no * vertex_size);
	if (!CHECK_POINTER(tmp_buffer, "tmp buffer"))
	{
		free(float_buffer);
		return NULL;
	}

#ifdef	ZLIB
	gzread(file, tmp_buffer, cur_object->vertex_no * vertex_size);
#else	//ZLIB
	fread(tmp_buffer, cur_object->vertex_no, vertex_size, file);
#endif	//ZLIB

	idx = 0;
	for (i = 0; i < cur_object->vertex_no; i++)
	{
		memcpy(float_buffer, &tmp_buffer[idx], size);
		idx += vertex_size;
		l = 0;
		float_pointer = cur_object->texture_data;
		float_pointer[i * 2 + 0] = SwapLEFloat(float_buffer[l++]);
		float_pointer[i * 2 + 1] = SwapLEFloat(float_buffer[l++]);
		if (!is_ground(cur_object->vertex_options))
		{
			float_pointer = cur_object->normal_data;
			float_pointer[i * 3 + 0] = SwapLEFloat(float_buffer[l++]);
			float_pointer[i * 3 + 1] = SwapLEFloat(float_buffer[l++]);
			float_pointer[i * 3 + 2] = SwapLEFloat(float_buffer[l++]);
		}
		float_pointer = cur_object->vertex_data;
		float_pointer[i * 3 + 0] = SwapLEFloat(float_buffer[l++]);
		float_pointer[i * 3 + 1] = SwapLEFloat(float_buffer[l++]);
		float_pointer[i * 3 + 2] = SwapLEFloat(float_buffer[l++]);
		if (has_tangen(cur_object->vertex_options))
		{
			float_pointer = cur_object->tangent_data;
			float_pointer[i * 3 + 0] = SwapLEFloat(float_buffer[l++]);
			float_pointer[i * 3 + 1] = SwapLEFloat(float_buffer[l++]);
			float_pointer[i * 3 + 2] = SwapLEFloat(float_buffer[l++]);
		}
		if (has_extra_uv(cur_object->vertex_options))
		{
			float_pointer = cur_object->extra_uv_data;
			float_pointer[i * 2 + 0] = SwapLEFloat(float_buffer[l++]);
			float_pointer[i * 2 + 1] = SwapLEFloat(float_buffer[l++]);
		}
	}

	free(float_buffer);
	free(tmp_buffer);

	// Now reading the indicies
#ifdef	ZLIB
	gzseek(file, SDL_SwapLE32(header.index_offset), SEEK_SET);
#else	//ZLIB
	fseek(file, SDL_SwapLE32(header.index_offset), SEEK_SET);
#endif	//ZLIB

	index_buffer = (unsigned int*)malloc(cur_object->index_no * sizeof(unsigned int));
	if (!CHECK_POINTER(index_buffer, "index buffer")) return NULL;
#ifdef	ZLIB
	gzread(file, index_buffer, cur_object->index_no * sizeof(unsigned int));
#else	//ZLIB
	fread(index_buffer, cur_object->index_no, sizeof(unsigned int), file);
#endif	//ZLIB

	if (cur_object->index_no <= 256)
	{
		indicies_size = 1;
		cur_object->index_type = GL_UNSIGNED_BYTE;
	}
	else
	{
		if (cur_object->index_no <= 256*256)
		{
			indicies_size = 2;
			cur_object->index_type = GL_UNSIGNED_SHORT;
		}
		else
		{
			indicies_size = 4;
			cur_object->index_type = GL_UNSIGNED_INT;
		}
	}

	cur_object->indicies = malloc(cur_object->index_no * indicies_size);
	mem_size += cur_object->index_no * indicies_size;
	if (!CHECK_POINTER(cur_object->indicies, "indicies"))
	{
		free(index_buffer);
		return NULL;
	}

	if (have_vertex_buffers) index_pointer = 0;
	else index_pointer = cur_object->indicies;
	
	// Optimize the storage format for better use with the OpenGL glDrawElements function
	if (indicies_size == 1)
	{
		char_list = (unsigned char*)cur_object->indicies;
		for (i = 0; i < cur_object->index_no; i++)
			char_list[i] = SDL_SwapLE32(index_buffer[i]);
	}
	else
	{
		if (indicies_size == 2)
		{
			short_list = (unsigned short*)cur_object->indicies;
			for (i = 0; i < cur_object->index_no; i++)
				short_list[i] = SDL_SwapLE32(index_buffer[i]);
		}
		else
		{
			if (indicies_size == 4)
			{
				int_list = (unsigned int*)cur_object->indicies;
				for (i = 0; i < cur_object->index_no; i++)
					int_list[i] = SDL_SwapLE32(index_buffer[i]);
			}
		}
	}
	free(index_buffer);

	// only allocate the materials structure if it doesn't exist (on initial load)
	if (cur_object->materials == NULL)
	{
		cur_object->materials = (e3d_draw_list*)malloc(cur_object->material_no*sizeof(e3d_draw_list));
		if (!CHECK_POINTER(cur_object->materials, "materials")) return NULL;
		memset(cur_object->materials, 0, cur_object->material_no * sizeof(e3d_draw_list));
	}
	mem_size += cur_object->material_no * sizeof(e3d_draw_list);

	// Now reading the materials
#ifdef	ZLIB
	gzseek(file, SDL_SwapLE32(header.material_offset), SEEK_SET);
#else	//ZLIB
	fseek(file, SDL_SwapLE32(header.material_offset), SEEK_SET);
#endif	//ZLIB
	
	for (i = 0; i < cur_object->material_no; i++)
	{
		
#ifdef	ZLIB
		file_pos = gztell(file);
		gzread(file, &material, sizeof(e3d_material));
#else	//ZLIB
		file_pos = ftell(file);
		fread(&material, 1, sizeof(e3d_material), file);
#endif	//ZLIB
		snprintf(text_file_name, sizeof(text_file_name), "%s%s", cur_dir, material.material_name);

		cur_object->materials[i].options = SDL_SwapLE32(material.options);
#ifdef	MAP_EDITOR
		cur_object->materials[i].texture_id = load_texture_cache(text_file_name,0);
#else	//MAP_EDITOR
#ifdef	NEW_ALPHA
		// prepare to load the textures depending on if it is transparent or not (diff alpha handling)
		if(cur_object->materials[i].options & 0x00000001){	// is this object transparent?
			cur_object->materials[i].texture_id= load_texture_cache_deferred(text_file_name, -1);
		} else {
			cur_object->materials[i].texture_id= load_texture_cache_deferred(text_file_name, -1);	//255);
		}
#else	//NEW_ALPHA
//		cur_object->materials[i].texture_id = load_texture_cache_deferred(text_file_name, 255);
		cur_object->materials[i].texture_id = load_texture_cache_deferred(text_file_name, 0);
#endif	//NEW_ALPHA
#endif	//MAP_EDITOR

		cur_object->materials[i].min_x = SwapLEFloat(material.min_x);
		cur_object->materials[i].min_y = SwapLEFloat(material.min_y);
		cur_object->materials[i].min_z = SwapLEFloat(material.min_z);
		cur_object->materials[i].max_x = SwapLEFloat(material.max_x);
		cur_object->materials[i].max_y = SwapLEFloat(material.max_y);
		cur_object->materials[i].max_z = SwapLEFloat(material.max_z);
		// calculate the max size for cruse LOD processing
		cur_object->materials[i].max_size= max2f(max2f(cur_object->materials[i].max_x-cur_object->materials[i].min_x, cur_object->materials[i].max_y-cur_object->materials[i].min_y), cur_object->materials[i].max_z-cur_object->materials[i].min_z);
		cur_object->materials[i].triangles_indicies_index = indicies_size*SDL_SwapLE32(material.index) + index_pointer;
		cur_object->materials[i].triangles_indicies_count = SDL_SwapLE32(material.count);
		cur_object->materials[i].triangles_indicies_min = SDL_SwapLE32(material.triangles_min_index);
		cur_object->materials[i].triangles_indicies_max = SDL_SwapLE32(material.triangles_max_index);

		file_pos += SDL_SwapLE32(material_size);
#ifdef	ZLIB
		gzseek(file, file_pos, SEEK_SET);
#else	//ZLIB
		fseek(file, file_pos, SEEK_SET);
#endif	//ZLIB
	}
#ifdef	ZLIB
	gzclose(file);
#else	//ZLIB
	fclose(file);
#endif	//ZLIB

	if (have_vertex_buffers)
	{
		//Generate the buffers
		ELglGenBuffersARB(1, &cur_object->vertex_vbo);
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB,
			cur_object->vertex_vbo);
		ELglBufferDataARB(GL_ARRAY_BUFFER_ARB,
			cur_object->vertex_no * 3 * sizeof(float),
			cur_object->vertex_data, GL_STATIC_DRAW_ARB);
#ifndef	MAP_EDITOR
		free(cur_object->vertex_data);
		cur_object->vertex_data = NULL;
#endif	//MAP_EDITOR

		if (!is_ground(cur_object->vertex_options))
		{
			ELglGenBuffersARB(1, &cur_object->normal_vbo);
			ELglBindBufferARB(GL_ARRAY_BUFFER_ARB,
				cur_object->normal_vbo);
			ELglBufferDataARB(GL_ARRAY_BUFFER_ARB,
				cur_object->vertex_no * 3 * sizeof(float),
				cur_object->normal_data, GL_STATIC_DRAW_ARB);
			free(cur_object->normal_data);
			cur_object->normal_data = NULL;
		}
		
		ELglGenBuffersARB(1, &cur_object->texture_vbo);
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB,
			cur_object->texture_vbo);
		ELglBufferDataARB(GL_ARRAY_BUFFER_ARB,
			cur_object->vertex_no * 2 * sizeof(float),
			cur_object->texture_data, GL_STATIC_DRAW_ARB);
		free(cur_object->texture_data);
		cur_object->texture_data = NULL;
		
		if (has_tangen(cur_object->vertex_options))
		{
			ELglGenBuffersARB(1, &cur_object->tangent_vbo);
			ELglBindBufferARB(GL_ARRAY_BUFFER_ARB,
				cur_object->tangent_vbo);
			ELglBufferDataARB(GL_ARRAY_BUFFER_ARB,
				cur_object->vertex_no * 3 * sizeof(float),
				cur_object->tangent_data, GL_STATIC_DRAW_ARB);
			free(cur_object->tangent_data);
			cur_object->tangent_data = NULL;
		}
		
		if (has_extra_uv(cur_object->vertex_options))
		{
			ELglGenBuffersARB(1, &cur_object->extra_uv_vbo);
			ELglBindBufferARB(GL_ARRAY_BUFFER_ARB,
				cur_object->extra_uv_vbo);
			ELglBufferDataARB(GL_ARRAY_BUFFER_ARB,
				cur_object->vertex_no * 2 * sizeof(float),
				cur_object->extra_uv_data, GL_STATIC_DRAW_ARB);
			free(cur_object->extra_uv_data);
			cur_object->extra_uv_data = NULL;
		}
		
		ELglGenBuffersARB(1, &cur_object->indicies_vbo);
		ELglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
			cur_object->indicies_vbo);
		ELglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
			cur_object->index_no * indicies_size,
			cur_object->indicies, GL_STATIC_DRAW_ARB);
#ifndef	MAP_EDITOR
		free(cur_object->indicies);
		cur_object->indicies = NULL;
#endif	//MAP_EDITOR
				
		ELglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	}

#ifndef	MAP_EDITOR
	cache_adj_size(cache_e3d, mem_size, cur_object);
#endif	//MAP_EDITOR
	return cur_object;
}
#endif
