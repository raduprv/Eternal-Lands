#ifdef NEW_E3D_FORMAT
#include "e3d_io.h"
#include <float.h>

e3d_object* load_e3d_detail(e3d_object* cur_object)
{
	e3d_header header;
	e3d_T2F_N3F_V3F_vertex* vertex_list;
	e3d_T2F_V3F_vertex* vertex_ground_list;
	e3d_material material;
	FILE* file;
	char cur_dir[1024];
	int i, j, l, mem_size, vertex_size, size, file_pos, triangle_strips_no, indicies_size;
	char text_file_name[1024];
	unsigned int* buffer;
	unsigned int* index_buffer;
	unsigned char* char_list;
	unsigned short* short_list;
	unsigned int* int_list;
	float* float_mem;
	void* index_pointer;
	
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

	file = my_fopen(cur_object->file_name, "rb");
	if (file == NULL)
	{
		LOG_ERROR("Can't open file \"%s\"!", cur_object->file_name);
		free(cur_object);
		return NULL;
	}
		
	if (read_and_check_elc_header(file, EL3D_FILE_MAGIC_NUMBER, EL3D_FILE_VERSION_NUMBER, cur_object->file_name) != 0)
	{
		LOG_ERROR("File \"%s\" has wrong header!", cur_object->file_name);
		free(cur_object);
		fclose(file);
		return NULL;
	}
	
	fread((char*)&header, 1, sizeof(e3d_header), file);

	cur_object->vertex_no = SDL_SwapLE32(header.vertex_no);
	cur_object->index_no = SDL_SwapLE32(header.index_no);
	cur_object->material_no = SDL_SwapLE32(header.material_no);
	cur_object->is_ground = header.is_ground;
	vertex_size = SDL_SwapLE32(header.vertex_size);

	// They have the size we expected
	if (SDL_SwapLE32(header.index_size) != sizeof(unsigned int))
	{
		LOG_ERROR("File \"%s\" has wrong index size!", cur_object->file_name);
		fclose(file);
		return NULL;
	}
	
	if (cur_object->is_ground == 0) size = sizeof(e3d_T2F_N3F_V3F_vertex);
	else size = sizeof(e3d_T2F_V3F_vertex);

	// Now reading the vertices
	fseek(file, SDL_SwapLE32(header.vertex_offset), SEEK_SET);
	
	cur_object->vertex_data = malloc(cur_object->vertex_no*size);
	
	mem_size = cur_object->vertex_no*size;

	if (vertex_size == size)
	{
		fread(cur_object->vertex_data, cur_object->vertex_no, vertex_size, file);
	}
	else
	{
		if (cur_object->is_ground == 0)
		{
			vertex_list = cur_object->vertex_data;
			for (i = 0; i < cur_object->vertex_no; i++)
			{
				fread(&vertex_list[i], 1, size, file);
				fseek(file, vertex_size-size, SEEK_CUR);
			}
		}
		else
		{
			vertex_ground_list = cur_object->vertex_data;
			for (i = 0; i < cur_object->vertex_no; i++)
			{
				fread(&vertex_ground_list[i], 1, size, file);
				fseek(file, vertex_size-size, SEEK_CUR);
			}
		}
	}
	float_mem = cur_object->vertex_data;

	vertex_size = size;
#ifdef EL_BIG_ENDIAN
	for (i = 0; i < cur_object->vertex_no*(size/sizeof(float)); i++)
	{
		float_mem[i] = SwapLEFloat(float_mem[i]);
	}
#endif
	
	// Now reading the indicies
	fseek(file, SDL_SwapLE32(header.index_offset), SEEK_SET);
	
	size = SDL_SwapLE32(header.index_size);
	// They have the size we expected
	if (size != sizeof(unsigned int))
	{
		LOG_ERROR("File \"%s\" has wrong index size!", cur_object->file_name);
		free(cur_object->vertex_data);
		free(cur_object->indicies);
		free(cur_object);
		fclose(file);
		return NULL;
	}

	index_buffer = (unsigned int*)malloc(cur_object->index_no*sizeof(unsigned int));
	fread(index_buffer, cur_object->index_no, sizeof(unsigned int), file);
	
	// Now reading the materials
	fseek(file, SDL_SwapLE32(header.material_offset), SEEK_SET);
	
	// only allocate the materials structure if it doesn't exist (on initial load)
	if(cur_object->materials == NULL){
		cur_object->materials = (e3d_draw_list*)malloc(cur_object->material_no*sizeof(e3d_draw_list));
		memset(cur_object->materials, 0, cur_object->material_no*sizeof(e3d_draw_list));

		mem_size += cur_object->material_no*sizeof(e3d_draw_list);
	}

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

	cur_object->indicies = malloc(cur_object->index_no*indicies_size);

	if (have_vertex_buffers) index_pointer = 0;
	else index_pointer = cur_object->indicies;
	
	mem_size += cur_object->index_no*indicies_size;
	
	// Optimize the storage format for better use with the OpenGL glMultiDrawElements function
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

	for (i = 0; i < cur_object->material_no; i++)
	{
		file_pos = ftell(file);
		
		fread(&material, 1, sizeof(e3d_material), file);
		snprintf(text_file_name, sizeof(text_file_name), "%s%s", cur_dir, material.material_name);
		cur_object->materials[i].texture_id = load_texture_cache_deferred(text_file_name, 0);

		cur_object->materials[i].options = SDL_SwapLE32(material.options);
		cur_object->materials[i].min_x = SwapLEFloat(material.min_x);
		cur_object->materials[i].min_y = SwapLEFloat(material.min_y);
		cur_object->materials[i].min_z = SwapLEFloat(material.min_z);
		cur_object->materials[i].max_x = SwapLEFloat(material.max_x);
		cur_object->materials[i].max_y = SwapLEFloat(material.max_y);
		cur_object->materials[i].max_z = SwapLEFloat(material.max_z);
		// calculate the max size for cruse LOD processing
		cur_object->materials[i].max_size= max(max(cur_object->materials[i].max_x-cur_object->materials[i].min_x, cur_object->materials[i].max_y-cur_object->materials[i].min_y), cur_object->materials[i].max_z-cur_object->materials[i].min_z);
		cur_object->materials[i].triangles_indicies_index = indicies_size*SDL_SwapLE32(material.triangles.index) + index_pointer;
		cur_object->materials[i].triangles_indicies_count = SDL_SwapLE32(material.triangles.count);
		cur_object->materials[i].triangles_indicies_min = SDL_SwapLE32(material.triangles_min_index);
		cur_object->materials[i].triangles_indicies_max = SDL_SwapLE32(material.triangles_max_index);
		triangle_strips_no = SDL_SwapLE32(material.triangle_strips_no);

		cur_object->materials[i].triangle_strips_no = triangle_strips_no;

		if (triangle_strips_no > 0)
		{
			cur_object->materials[i].triangle_strips_indicies_index = (void**)malloc(triangle_strips_no*sizeof(void*));
			cur_object->materials[i].triangle_strips_indicies_count = (int*)malloc(triangle_strips_no*sizeof(int));
		
			mem_size += triangle_strips_no*(sizeof(int)+sizeof(void*));
		
			buffer = (unsigned int*)malloc(triangle_strips_no*sizeof(e3d_triangle_list));
		
			fread(buffer, triangle_strips_no, sizeof(e3d_triangle_list), file);
			for (j = 0; j < triangle_strips_no; j++)
			{
				cur_object->materials[i].triangle_strips_indicies_index[j] = indicies_size*SDL_SwapLE32(buffer[j*2+0]) + index_pointer;
				cur_object->materials[i].triangle_strips_indicies_count[j] = SDL_SwapLE32(buffer[j*2+1]);
			}
			free(buffer);
		}

		file_pos += SDL_SwapLE32(material.material_size);
		fseek(file, file_pos, SEEK_SET);
	}
	fclose(file);

	if (have_vertex_buffers)
	{
		//Generate the buffers
		ELglGenBuffersARB(2, cur_object->vbo);
		
		if (cur_object->vbo[0] && cur_object->vbo[1])
		{
			ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, cur_object->vbo[0]);
			ELglBufferDataARB(GL_ARRAY_BUFFER_ARB, cur_object->vertex_no*vertex_size, cur_object->vertex_data, GL_STATIC_DRAW_ARB);
		
			ELglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, cur_object->vbo[1]);
			ELglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, cur_object->index_no*indicies_size, cur_object->indicies, GL_STATIC_DRAW_ARB);
				
			ELglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
			ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		} 
		else
		{
			if (cur_object->vbo[0]) ELglDeleteBuffersARB(1, &cur_object->vbo[0]);
			if (cur_object->vbo[1]) ELglDeleteBuffersARB(1, &cur_object->vbo[1]);
			cur_object->vbo[0] = 0;
			cur_object->vbo[1] = 0;

			LOG_ERROR("We could not create all 2 vertex buffers! This is a major bug, so report it to the developers!");
		}
	}
	else
	{
		cur_object->vbo[0] = 0;
		cur_object->vbo[1] = 0;
	}

	cache_adj_size(cache_e3d, mem_size, cur_object);

	return cur_object;
}
#endif
