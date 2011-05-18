
#include "shader.h"
#include "noise.h"
#include "../errors.h"
#include "../gl_init.h"
#include "../load_gl_extensions.h"
#include "../misc.h"
#include "../io/elfilewrapper.h"

GLuint noise_tex;
GLuint filter_lut;

typedef struct
{
	const char* vertex_shader_file_name;
	const char* vertex_shader_defines;
	const char* fragment_shader_file_name;
	const char* fragment_shader_defines;
} shader_data;

shader_data shader_data_list[] = {
	{ NULL, NULL, "./shaders/water_fs.glsl", " " },
	{ NULL, NULL, "./shaders/water_fs.glsl", "#define\tUSE_SHADOW\n" },
	{ NULL, NULL, "./shaders/water_fs.glsl", "#define\tUSE_FOG\n" },
	{ NULL, NULL, "./shaders/water_fs.glsl", "#define\tUSE_FOG\n#define\tUSE_SHADOW\n" },
	{ NULL, NULL, "./shaders/reflectiv_water_fs.glsl", "#define\tUSE_CUBIC_FILTER\n" },
	{ NULL, NULL, "./shaders/reflectiv_water_fs.glsl", "#define\tUSE_CUBIC_FILTER\n#define\tUSE_SHADOW\n" },
	{ NULL, NULL, "./shaders/reflectiv_water_fs.glsl", "#define\tUSE_CUBIC_FILTER\n#define\tUSE_FOG\n" },
	{ NULL, NULL, "./shaders/reflectiv_water_fs.glsl", "#define\tUSE_CUBIC_FILTER\n#define\tUSE_FOG\n#define\tUSE_SHADOW\n" },
	{ NULL, NULL, "./shaders/water_fs.glsl", "#define\tUSE_NOISE\n" },
	{ NULL, NULL, "./shaders/water_fs.glsl", "#define\tUSE_NOISE\n#define\tUSE_SHADOW\n" },
	{ NULL, NULL, "./shaders/water_fs.glsl", "#define\tUSE_NOISE\n#define\tUSE_FOG\n" },
	{ NULL, NULL, "./shaders/water_fs.glsl", "#define\tUSE_NOISE\n#define\tUSE_FOG\n#define\tUSE_SHADOW\n" },
	{ NULL, NULL, "./shaders/reflectiv_water_fs.glsl", "#define\tUSE_NOISE\n#define\tUSE_CUBIC_FILTER\n" },
	{ NULL, NULL, "./shaders/reflectiv_water_fs.glsl", "#define\tUSE_NOISE\n#define\tUSE_CUBIC_FILTER\n#define\tUSE_SHADOW\n" },
	{ NULL, NULL, "./shaders/reflectiv_water_fs.glsl", "#define\tUSE_NOISE\n#define\tUSE_CUBIC_FILTER\n#define\tUSE_FOG\n" },
	{ NULL, NULL, "./shaders/reflectiv_water_fs.glsl", "#define\tUSE_NOISE\n#define\tUSE_CUBIC_FILTER\n#define\tUSE_FOG\n#define\tUSE_SHADOW\n" }
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
		CHECK_GL_ERRORS();

		el_close(file);

		return 1;
	}
}

static __inline__ void log_shader_compile_log(GLhandleARB object, const char* shader_file_name,
	GLint error)
{
	GLint blen, slen;
	GLcharARB* info_log;

	ELglGetObjectParameterivARB(object, GL_OBJECT_INFO_LOG_LENGTH_ARB , &blen);
	CHECK_GL_ERRORS();

	if (blen > 1)
	{
		info_log = (GLcharARB*)malloc(blen * sizeof(GLcharARB));
		ELglGetInfoLogARB(object, blen, &slen, info_log);
		CHECK_GL_ERRORS();
		if (error == 1)
		{
			LOG_DEBUG("Compiling shader '%s' successful: %s", shader_file_name, info_log);
		}
		else
		{
			LOG_ERROR("Compiling shader '%s' failed: %s", shader_file_name, info_log);
		}
		free(info_log);
	}
	else
	{
		if (error == 1)
		{
			LOG_DEBUG("Compiling shader '%s' successful", shader_file_name);
		}
		else
		{
			LOG_ERROR("Compiling shader '%s' failed", shader_file_name);
		}
	}
}

static __inline__ void log_shader_linking_log(GLhandleARB object, GLint error)
{
	GLint blen, slen;
	GLcharARB* info_log;

	ELglGetObjectParameterivARB(object, GL_OBJECT_INFO_LOG_LENGTH_ARB , &blen);
	CHECK_GL_ERRORS();

	if (blen > 1)
	{
		info_log = (GLcharARB*)malloc(blen * sizeof(GLcharARB));
		ELglGetInfoLogARB(object, blen, &slen, info_log);
		CHECK_GL_ERRORS();
		if (error == 1)
		{
			LOG_DEBUG("Linking shaders successful: %s", info_log);
		}
		else
		{
			LOG_ERROR("Linking shaders failed: %s", info_log);
		}
		free(info_log);
	}
	else
	{
		if (error == 1)
		{
			LOG_DEBUG("Linking shaders successful");
		}
		else
		{
			LOG_ERROR("Linking shaders failed");
		}
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

	CHECK_GL_ERRORS();
	shader_object = ELglCreateProgramObjectARB();
	CHECK_GL_ERRORS();

	if (vertex_shader_file_name != NULL)
	{
		vertex_shader_object = ELglCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
		CHECK_GL_ERRORS();
		if (load_shader(vertex_shader_object, vertex_shader_file_name, vertex_shader_defines) == 1)
		{
			ELglCompileShaderARB(vertex_shader_object);
			CHECK_GL_ERRORS();
			ELglGetObjectParameterivARB(vertex_shader_object, GL_OBJECT_COMPILE_STATUS_ARB, &ret);
			CHECK_GL_ERRORS();
			log_shader_compile_log(vertex_shader_object, vertex_shader_file_name, ret);

			if (ret == 1)
			{
				ELglAttachObjectARB(shader_object, vertex_shader_object);
				CHECK_GL_ERRORS();
			}
			else
			{
				error = 1;
			}
			ELglDeleteObjectARB(vertex_shader_object);
			CHECK_GL_ERRORS();
		}
		else
		{
			error = 1;
		}
	}

	if ((fragment_shader_file_name != NULL) && (error == 0))
	{
		fragment_shader_object = ELglCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
		CHECK_GL_ERRORS();
		if (load_shader(fragment_shader_object, fragment_shader_file_name, fragment_shader_defines) == 1)
		{
			ELglCompileShaderARB(fragment_shader_object);
			CHECK_GL_ERRORS();
			ELglGetObjectParameterivARB(fragment_shader_object, GL_OBJECT_COMPILE_STATUS_ARB, &ret);
			CHECK_GL_ERRORS();
			log_shader_compile_log(fragment_shader_object, fragment_shader_file_name, ret);

			if (ret == 1)
			{
				ELglAttachObjectARB(shader_object, fragment_shader_object);
				CHECK_GL_ERRORS();
			}
			else
			{
				error = 1;
			}
			ELglDeleteObjectARB(fragment_shader_object);
			CHECK_GL_ERRORS();
		}
		else
		{
			error = 1;
		}
	}

	if (error == 1)
	{
		ELglDeleteProgramsARB(1, &shader_object);
		CHECK_GL_ERRORS();

		return 0;
	}

	ELglLinkProgramARB(shader_object);
	CHECK_GL_ERRORS();
	ELglGetObjectParameterivARB(shader_object, GL_OBJECT_LINK_STATUS_ARB, &ret);
	CHECK_GL_ERRORS();
	log_shader_linking_log(shader_object, ret);

	if (ret == 1)
	{
		return shader_object;
	}
	else
	{
		ELglDeleteProgramsARB(1, &shader_object);
		CHECK_GL_ERRORS();

		return 0;
	}
}

int is_shader_supported()
{
	if (have_extension(arb_fragment_program) && have_extension(arb_vertex_program) &&
		have_extension(arb_fragment_shader) && have_extension(arb_vertex_shader) &&
		have_extension(arb_shader_objects) && have_extension(arb_shading_language_100) &&
		supports_gl_version(1, 3))
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
	CHECK_GL_ERRORS();
	glDeleteTextures(1, &noise_tex);
	CHECK_GL_ERRORS();
	if (is_shader_supported())
	{
		ELglDeleteProgramsARB(MAX_SHADER_COUNT, shader);
		CHECK_GL_ERRORS();
	}
}

static __inline__ int get_shader_index(shader_type type, shader_shadow_type shadow_type, shader_fog_type fog_type, Uint32 quality)
{
	int ret;

	ret = type * 4;
	ret += fog_type * 2;
	ret += shadow_type;
	ret += max2i(min2i(quality, 1), 0) * 8;

	return ret;
}

static __inline__ GLuint build_filter_lut(const Uint32 size)
{
	GLfloat *data;
	float w[4];
	float x3, x2, x;
	Uint32 i;
	GLuint texture;

	data = (GLfloat*)malloc(size * 3 * sizeof(GLfloat));

	x = 0.0f;

	for (i = 0; i < size; i++)
	{
		x2 = x * x;
		x3 = x2 * x;

		w[0] = (-x3 + 3 * x2 - 3 * x + 1) / 6.0f;
		w[1] = (3 * x3 - 6 * x2 + 4) / 6.0f;
		w[2] = (-3 * x3 + 3 * x2 + 3 * x + 1) / 6.0f;
		w[3] = x3 / 6.0f;

		data[i * 3 + 0] = 1.0 + x - w[1] / (w[0] + w[1]);
		data[i * 3 + 1] = 1.0 - x + w[3] / (w[2] + w[3]);
		data[i * 3 + 2] = w[2] + w[3];

		x += 1.0f / (size - 1);
	}

	LOG_DEBUG("Filter lookup texture\n");
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_1D, texture);

	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB10_A2, size, 0, GL_RGB, GL_FLOAT, data);
	glBindTexture(GL_TEXTURE_1D, 0);
	free(data);

	return texture;
}

void init_shaders()
{
	int i;

	memset(shader, 0, sizeof(shader));

	if (is_shader_supported())
	{
		ENTER_DEBUG_MARK("init shaders");

		noise_tex = build_3d_noise_texture(64, 3, 2);
		filter_lut = build_filter_lut(128);

		for (i = 0; i < shader_data_size; i++)
		{
			shader[i] = build_shader(shader_data_list[i].vertex_shader_file_name,
				shader_data_list[i].vertex_shader_defines,
				shader_data_list[i].fragment_shader_file_name,
				shader_data_list[i].fragment_shader_defines);
		}

		LEAVE_DEBUG_MARK("init shaders");
	}
}

GLhandleARB get_shader(shader_type type, shader_shadow_type shadow_type, shader_fog_type fog_type, Uint32 quality)
{
	int index;

	index = get_shader_index(type, shadow_type, fog_type, quality);

	return shader[index];
}

