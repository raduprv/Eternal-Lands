#include "e3d_io.h"
#include <float.h>
#ifdef	NEW_FILE_IO
#include "elfilewrapper.h"
#endif	//NEW_FILE_IO

static void free_e3d_pointer(e3d_object* cur_object)
{
	if (cur_object != NULL)
	{
		if (cur_object->vertex_data != NULL)
		{
			free(cur_object->vertex_data);
			cur_object->vertex_data = NULL;
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

#ifdef	NEW_FILE_IO
static int check_pointer(void* ptr, e3d_object* cur_object, const char* str, el_file_ptr file)
#else	//NEW_FILE_IO
static int check_pointer(void* ptr, e3d_object* cur_object, const char* str, FILE* file)
#endif	//NEW_FILE_IO
{
	if (ptr == NULL)
	{
		LOG_ERROR("Can't allocate memory for %s!", str);
		free_e3d_pointer(cur_object);
#ifdef	NEW_FILE_IO
		el_close(file);
#else	//NEW_FILE_IO
		fclose(file);
#endif	//NEW_FILE_IO
		return 0;
	}
	else
	{
		return 1;
	}
}

#define CHECK_POINTER(ptr, str) check_pointer((ptr), cur_object, (str), file)

#ifdef	NEW_FILE_IO
static int check_size(int read_size, int size, e3d_object* cur_object, const char* filename, const char* str, el_file_ptr file)
#else	//NEW_FILE_IO
static int check_size(int read_size, int size, e3d_object* cur_object, const char* filename, const char* str, FILE* file)
#endif	//NEW_FILE_IO
{
	if (read_size < size)
	{
		LOG_ERROR("File \"%s\" has too small %s size!", filename, str);
		free_e3d_pointer(cur_object);
#ifdef	NEW_FILE_IO
		el_close(file);
#else	//NEW_FILE_IO
		fclose(file);
#endif	//NEW_FILE_IO
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
#ifdef USE_EXTRA_TEXTURE
	e3d_extra_texture extra_texture;
#endif //USE_EXTRA_TEXTURE
	char cur_dir[1024];
	int i, j, l, idx, mem_size, vertex_size, material_size;
	int file_pos, indicies_size, float_count, index_size, v_size, m_size;
	char text_file_name[1024];
	unsigned int* index_buffer;
	unsigned short* short_list;
	unsigned int* int_list;
	float* float_buffer;
	char* tmp_buffer;
	Uint8* index_pointer;
	float* float_pointer;
#ifdef	NEW_FILE_IO
	el_file_ptr file;
#else	//NEW_FILE_IO
	FILE* file;
#endif	//NEW_FILE_IO
	
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

#ifdef	NEW_FILE_IO
	file = el_open(cur_object->file_name);
#else	//NEW_FILE_IO
	file= my_fopen(cur_object->file_name, "rb");
#endif	//NEW_FILE_IO
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
#ifdef	NEW_FILE_IO
		el_close(file);
#else	//NEW_FILE_IO
		fclose(file);
#endif	//NEW_FILE_IO
		return NULL;
	}
	
#ifdef	NEW_FILE_IO
	el_read(file, sizeof(e3d_header), &header);
#else	//NEW_FILE_IO
	fread((char*)&header, 1, sizeof(e3d_header), file);
#endif	//NEW_FILE_IO

	cur_object->vertex_no = SDL_SwapLE32(header.vertex_no);
	cur_object->index_no = SDL_SwapLE32(header.index_no);
	cur_object->material_no = SDL_SwapLE32(header.material_no);
	cur_object->vertex_options = header.vertex_options;
	vertex_size = SDL_SwapLE32(header.vertex_size);
	material_size = SDL_SwapLE32(header.material_size);
	index_size = SDL_SwapLE32(header.index_size);

	v_size = get_vertex_size(cur_object->vertex_options);
	m_size = get_material_size(cur_object->vertex_options);
	float_count = get_vertex_float_count(cur_object->vertex_options);

	// They have at least the size we expected
	if (!CHECK_SIZE(vertex_size, v_size, "vertex")) return NULL;
	if (!CHECK_SIZE(material_size, m_size, "material")) return NULL;
	if (index_size != sizeof(unsigned int))
	{
		LOG_ERROR("File \"%s\" has wrong index size!", cur_object->file_name);
		free_e3d_pointer(cur_object);
#ifdef	NEW_FILE_IO
		el_close(file);
#else	//NEW_FILE_IO
		fclose(file);
#endif	//NEW_FILE_IO
		return NULL;
	}
	
	// Now reading the vertices
#ifdef	NEW_FILE_IO
	el_seek(file, SDL_SwapLE32(header.vertex_offset), SEEK_SET);
#else	//NEW_FILE_IO
	fseek(file, SDL_SwapLE32(header.vertex_offset), SEEK_SET);
#endif	//NEW_FILE_IO

	cur_object->vertex_data = malloc(cur_object->vertex_no * v_size);
	mem_size = cur_object->vertex_no * v_size;
	if (!CHECK_POINTER(cur_object->vertex_data, "vertex data")) return NULL;

	float_buffer = (float*) malloc(v_size);
	if (!CHECK_POINTER(float_buffer, "float buffer")) return NULL;
	tmp_buffer = (char*) malloc(cur_object->vertex_no * vertex_size);
	if (!CHECK_POINTER(tmp_buffer, "tmp buffer"))
	{
		free(float_buffer);
		return NULL;
	}

#ifdef	NEW_FILE_IO
	el_read(file, cur_object->vertex_no * vertex_size, tmp_buffer);
#else	//NEW_FILE_IO
	fread(tmp_buffer, cur_object->vertex_no, vertex_size, file);
#endif	//NEW_FILE_IO

	idx = 0;
	float_pointer = cur_object->vertex_data;
	for (i = 0; i < cur_object->vertex_no; i++)
	{
		memcpy(float_buffer, &tmp_buffer[idx], v_size);
		idx += vertex_size;
		for (j = 0; j < float_count; j++)
		{
			float_pointer[i * float_count + j] = SwapLEFloat(float_buffer[j]);
		}
	}

	free(float_buffer);
	free(tmp_buffer);

	// Now reading the indicies
#ifdef	NEW_FILE_IO
	el_seek(file, SDL_SwapLE32(header.index_offset), SEEK_SET);
#else	//NEW_FILE_IO
	fseek(file, SDL_SwapLE32(header.index_offset), SEEK_SET);
#endif	//NEW_FILE_IO

	index_buffer = (unsigned int*)malloc(cur_object->index_no * sizeof(unsigned int));
	if (!CHECK_POINTER(index_buffer, "index buffer")) return NULL;
#ifdef	NEW_FILE_IO
	el_read(file, cur_object->index_no * sizeof(unsigned int), index_buffer);
#else	//NEW_FILE_IO
	fread(index_buffer, cur_object->index_no, sizeof(unsigned int), file);
#endif	//NEW_FILE_IO

	if (cur_object->index_no < 65536)
	{
		indicies_size = 2;
		cur_object->index_type = GL_UNSIGNED_SHORT;
	}
	else
	{
		indicies_size = 4;
		cur_object->index_type = GL_UNSIGNED_INT;
	}

	cur_object->indicies = malloc(cur_object->index_no * indicies_size);
	mem_size += cur_object->index_no * indicies_size;
	if (!CHECK_POINTER(cur_object->indicies, "indicies"))
	{
		free(index_buffer);
		return NULL;
	}

	if (use_vertex_buffers) index_pointer = 0;
	else index_pointer = cur_object->indicies;
	
	// Optimize the storage format for better use with the OpenGL glDrawElements function
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
		else
		{
			LOG_ERROR("This should never happen!");
			free_e3d_pointer(cur_object);
#ifdef	NEW_FILE_IO
			el_close(file);
#else	//NEW_FILE_IO
			fclose(file);
#endif	//NEW_FILE_IO
			return NULL;
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
#ifdef	NEW_FILE_IO
	el_seek(file, SDL_SwapLE32(header.material_offset), SEEK_SET);
#else	//NEW_FILE_IO
	fseek(file, SDL_SwapLE32(header.material_offset), SEEK_SET);
#endif	//NEW_FILE_IO
	
	cur_object->min_x = 1e10f;
	cur_object->min_y = 1e10f;
	cur_object->min_z = 1e10f;
	cur_object->max_x = -1e10f;
	cur_object->max_y = -1e10f;
	cur_object->max_z = -1e10f;
	cur_object->max_size = -1e10f;

	for (i = 0; i < cur_object->material_no; i++)
	{
		
#ifdef	NEW_FILE_IO
		file_pos = el_tell(file);
		el_read(file, sizeof(e3d_material), &material);
#else	//NEW_FILE_IO
		file_pos = ftell(file);
		fread(&material, 1, sizeof(e3d_material), file);
#endif	//NEW_FILE_IO
		snprintf(text_file_name, sizeof(text_file_name), "%s%s", cur_dir, material.material_name);

		cur_object->materials[i].options = SDL_SwapLE32(material.options);
#ifdef	MAP_EDITOR
		cur_object->materials[i].diffuse_map = load_texture_cache(text_file_name,0);
#else	//MAP_EDITOR
#ifdef	NEW_ALPHA
		// prepare to load the textures depending on if it is transparent or not (diff alpha handling)
		if (material_is_transparent(cur_object->materials[i].options))
		{	// is this object transparent?
			cur_object->materials[i].diffuse_map= load_texture_cache_deferred(text_file_name, -1);
		}
		else
		{
			cur_object->materials[i].diffuse_map= load_texture_cache_deferred(text_file_name, -1);	//255);
		}
#else	//NEW_ALPHA
//		cur_object->materials[i].diffuse_map = load_texture_cache_deferred(text_file_name, 255);
		cur_object->materials[i].diffuse_map = load_texture_cache_deferred(text_file_name, 0);
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

		cur_object->min_x = min2f(cur_object->min_x, cur_object->materials[i].min_x);
		cur_object->min_y = min2f(cur_object->min_y, cur_object->materials[i].min_y);
		cur_object->min_z = min2f(cur_object->min_z, cur_object->materials[i].min_z);
		cur_object->max_x = max2f(cur_object->max_x, cur_object->materials[i].max_x);
		cur_object->max_y = max2f(cur_object->max_y, cur_object->materials[i].max_y);
		cur_object->max_z = max2f(cur_object->max_z, cur_object->materials[i].max_z);
		cur_object->max_size = max2f(cur_object->max_size, cur_object->materials[i].max_size);

		cur_object->materials[i].triangles_indicies_index = indicies_size*SDL_SwapLE32(material.index) + index_pointer;
		cur_object->materials[i].triangles_indicies_count = SDL_SwapLE32(material.count);
		cur_object->materials[i].triangles_indicies_min = SDL_SwapLE32(material.triangles_min_index);
		cur_object->materials[i].triangles_indicies_max = SDL_SwapLE32(material.triangles_max_index);

#ifdef	USE_EXTRA_TEXTURE
		if (has_extra_texture(cur_object->vertex_options))		
		{
#ifdef	NEW_FILE_IO
			el_read(file, sizeof(e3d_extra_texture), &extra_texture);
#else	//NEW_FILE_IO
			fread(&extra_texture, 1, sizeof(e3d_extra_texture), file);
#endif	//NEW_FILE_IO
			snprintf(text_file_name, sizeof(text_file_name), "%s%s", cur_dir, extra_texture.material_name);
#ifdef	MAP_EDITOR
			cur_object->materials[i].extra_diffuse_map = load_texture_cache(text_file_name,0);
#else	//MAP_EDITOR
#ifdef	NEW_ALPHA
			cur_object->materials[i].extra_diffuse_map = load_texture_cache_deferred(text_file_name, -1);
#else	//NEW_ALPHA
			cur_object->materials[i].extra_diffuse_map = load_texture_cache_deferred(text_file_name, 0);
#endif	//NEW_ALPHA
#endif	//MAP_EDITOR

		}
#endif	//USE_EXTRA_TEXTURE
		file_pos += SDL_SwapLE32(material_size);

#ifdef	NEW_FILE_IO
		el_seek(file, file_pos, SEEK_SET);
#else	//NEW_FILE_IO
		fseek(file, file_pos, SEEK_SET);
#endif	//NEW_FILE_IO
	}
#ifdef	NEW_FILE_IO
	el_close(file);
#else	//NEW_FILE_IO
	fclose(file);
#endif	//NEW_FILE_IO

	if (use_vertex_buffers)
	{
		//Generate the buffers
		ELglGenBuffersARB(1, &cur_object->vertex_vbo);
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB,
			cur_object->vertex_vbo);
		ELglBufferDataARB(GL_ARRAY_BUFFER_ARB,
			cur_object->vertex_no * v_size,
			cur_object->vertex_data, GL_STATIC_DRAW_ARB);
#ifndef	MAP_EDITOR
		free(cur_object->vertex_data);
		cur_object->vertex_data = NULL;
#endif	//MAP_EDITOR
		
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
	else
	{
		cur_object->vertex_vbo = 0;
		cur_object->indicies_vbo = 0;
	}

#ifndef	MAP_EDITOR
	cache_adj_size(cache_e3d, mem_size, cur_object);
#endif	//MAP_EDITOR
	return cur_object;
}
