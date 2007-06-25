#include "shader.h"
#include "noise.h"
#include "../io/elfilewrapper.h"

GLuint noise_tex;

typedef struct
{
	const char* vertex_shader_file_name;
	const char* vertex_shader_defines;
	const char* fragment_shader_file_name;
	const char* fragment_shader_defines;
} shader_data;

shader_data shader_data_list[] = {
	{ NULL, NULL, "./shader/water_fs.glsl", "" }, 
	{ NULL, NULL, "./shader/water_fs.glsl", "#define\tUSE_SHADOW\n" },
	{ NULL, NULL, "./shader/reflectiv_water_fs.glsl", "" },
	{ NULL, NULL, "./shader/reflectiv_water_fs.glsl", "#define\tUSE_SHADOW\n" },
	{ NULL, NULL, "./shader/water_fs.glsl", "#define\tUSE_NOISE\n" },
	{ NULL, NULL, "./shader/water_fs.glsl", "#define\tUSE_NOISE\n#define\tUSE_SHADOW\n" },
	{ NULL, NULL, "./shader/reflectiv_water_fs.glsl", "#define\tUSE_NOISE\n" },
	{ NULL, NULL, "./shader/reflectiv_water_fs.glsl", "#define\tUSE_NOISE\n#define\tUSE_SHADOW\n" }
};

#define	shader_data_size (sizeof(shader_data_list) / sizeof(shader_data))

#define MAX_SHADER_COUNT shader_data_size

GLhandleARB shader[MAX_SHADER_COUNT];

static __inline__ int load_shader(GLhandleARB object, const char* file_name,
	const char* defines)
{
	int size[2];
	el_file_ptr file;
	const char* buffer[2];
	
	file = el_open(file_name);

	if (file == NULL)
	{
		return 0;
	}
	else
	{
		size[0] = strlen(defines);
		size[1] = el_get_size(file);
		
		buffer[0] = defines;
		buffer[1] = (char*)el_get_pointer(file);
	
		ELglShaderSourceARB(object, 2, buffer, size);

		el_close(file);

		return 1;
	}
}

static __inline__ void log_shader_log(GLhandleARB object)
{
	GLint blen, slen;
	GLcharARB* info_log;
	
	ELglGetObjectParameterivARB(object, GL_OBJECT_INFO_LOG_LENGTH_ARB , &blen);
	
	if (blen > 1)
	{
		info_log = (GLcharARB*)malloc(blen * sizeof(GLcharARB));
		ELglGetInfoLogARB(object, blen, &slen, info_log);
		LOG_ERROR(info_log);
		free(info_log);
	}
}

static __inline__ GLhandleARB build_shader(const char* vertex_shader_file_name,
	const char* vertex_shader_defines, const char* fragment_shader_file_name,
	const char* fragment_shader_defines)
{
	GLint ret, error = 0;

	GLhandleARB shader_object, vertex_shader_object, fragment_shader_object;

	if ((vertex_shader_file_name == NULL) && (fragment_shader_file_name == NULL))
	{
		return 0;
	}

	shader_object = ELglCreateProgramObjectARB();

	if (vertex_shader_file_name != NULL)
	{
		vertex_shader_object = ELglCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
		if (load_shader(vertex_shader_object, vertex_shader_file_name, vertex_shader_defines) == 1)
		{
			ELglCompileShaderARB(vertex_shader_object);
			log_shader_log(vertex_shader_object);
			ELglGetObjectParameterivARB(vertex_shader_object, GL_OBJECT_COMPILE_STATUS_ARB, &ret);

			if (ret == 1)
			{
				ELglAttachObjectARB(shader_object, vertex_shader_object);
			}
			else
			{
				error = 1;
			}
			ELglDeleteObjectARB(vertex_shader_object);
		}
		else
		{
			error = 1;
		}
	}

	if ((fragment_shader_file_name != NULL) && (error == 0))
	{
		fragment_shader_object = ELglCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
		if (load_shader(fragment_shader_object, fragment_shader_file_name, fragment_shader_defines) == 1)
		{
			ELglCompileShaderARB(fragment_shader_object);
			log_shader_log(fragment_shader_object);

			ELglGetObjectParameterivARB(fragment_shader_object, GL_OBJECT_COMPILE_STATUS_ARB, &ret);

			if (ret == 1)
			{
				ELglAttachObjectARB(shader_object, fragment_shader_object);
			}
			else
			{
				error = 1;
			}
			ELglDeleteObjectARB(fragment_shader_object);
		}
		else
		{
			error = 1;
		}
	}

	if (error == 1)
	{
		ELglDeleteProgramsARB(1, &shader_object);

		return 0;
	}

	ELglLinkProgramARB(shader_object);
	log_shader_log(shader_object);

	ELglGetObjectParameterivARB(shader_object, GL_OBJECT_LINK_STATUS_ARB, &ret);

	if (ret == 1)
	{
		return shader_object;
	}
	else
	{
		ELglDeleteProgramsARB(1, &shader_object);

		return 0;
	}
}

int is_shader_supported()
{
	if (have_extension(arb_fragment_program) && have_extension(arb_vertex_program) &&
		have_extension(arb_fragment_shader) && have_extension(arb_vertex_shader) &&
		have_extension(arb_shader_objects) && have_extension(arb_shading_language_100))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void free_shaders()
{
	glDeleteTextures(1, &noise_tex);
	if (is_shader_supported())
	{
		ELglDeleteProgramsARB(MAX_SHADER_COUNT, shader);
	}
}

static __inline__ int get_shader_index(shader_type type, shader_shadow_type shadow_type, Uint32 quality)
{
	int ret;

	ret = type * 2;
	ret += shadow_type;
	ret += max2i(min2i(quality, 1), 0) * 4;

	return ret;
}

void init_shaders()
{
	int i;

	noise_tex = build_3d_noise_texture(64, 8, 2);

	memset(shader, 0, sizeof(shader));

	if (is_shader_supported())
	{
		for (i = 0; i < shader_data_size; i++)
		{
			shader[i] = build_shader(shader_data_list[i].vertex_shader_file_name,
				shader_data_list[i].vertex_shader_defines,
				shader_data_list[i].fragment_shader_file_name,
				shader_data_list[i].fragment_shader_defines);
		}
	}
}

GLhandleARB get_shader(shader_type type, shader_shadow_type shadow_type, Uint32 quality)
{
	int index;

	index = get_shader_index(type, shadow_type, quality);

//	printf("Index: %d\n", index);

	return shader[index];
}

