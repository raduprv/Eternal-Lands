#include <limits>
#include "actor_init.h"
#include "load_gl_extensions.h"
#include <cal3d/cal3d.h>
#ifdef	USE_BOOST
#include <boost/shared_array.hpp>
#include <boost/foreach.hpp>
#endif	/* USE_BOOST */
#include <map>
#include "bbox_tree.h"
#include "io/elfile.hpp"
#include "exceptions/extendedexception.hpp"
#include "gl_init.h"
#include "shadows.h"
#include "map.h"
#include "optimizer.hpp"
#include "md5.h"

Uint32 use_animation_program = 1;
#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
Uint32 use_display_actors = 1;
Uint32 use_actor_bbox_check = 1;
Uint32 use_render_mesh_shader = 1;
Uint32 use_set_transformation_buffers = 1;
Uint32 use_build_actor_bounding_box = 1;
Uint32 use_model_attach_and_detach_mesh = 1;
Uint32 use_render_attached_meshs = 1;
Uint32 use_ext_gpu_program_parameters = 1;
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
Uint32 max_bones_per_mesh = 27;

class HardwareMeshData
{
	private:
		const Sint32 mesh_index;
		const Uint32 size;
		eternal_lands::memory_ptr buffer;
	public:
		inline HardwareMeshData(const Sint32 mesh_index, const Uint32 size):
			mesh_index(mesh_index), size(size)
		{
			buffer = eternal_lands::memory_ptr(new eternal_lands::memory_buffer(size * 4 * sizeof(float)));
		}

		inline void set_buffer_value(const Uint32 index, const float value)
		{
			*(buffer->get_memory<float*>(index * sizeof(float))) = value;
		}

		inline float* get_buffer(const Uint32 index = 0) const
		{
			return buffer->get_memory<float*>(index * sizeof(float));
		}

		inline Uint32 get_size() const
		{
			return size;
		}

		inline Sint32 get_mesh_index() const
		{
			return mesh_index;
		}
};

struct ActorVertex
{
	float m_vertex[3];
	Uint8 m_weight[4];
	float m_normal[3];
	Uint8 m_index[4];
	float m_texture[2];
//	float m_bone_count;
};

#ifdef	USE_BOOST
typedef boost::shared_array<Uint16> Uint16Array;
typedef boost::shared_array<Uint32> Uint32Array;
typedef boost::shared_array<float> FloatArray;
typedef boost::shared_array<CalIndex> CalIndexArray;
typedef boost::shared_array<ActorVertex> ActorVertexArray;
#endif

typedef std::map<Sint32, HardwareMeshData> IntMap;

int last_actor_type = -1;
bool use_normals;

GLuint vertex_program_ids[5];

static inline GLuint load_vertex_program(const std::string &name)
{
	GLuint id;
	GLint support;
	eternal_lands::el_file f(name, true);
	std::string str;
	std::stringstream s1;
	std::stringstream s2;
	size_t pos;

#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	log_info("start load_vertex_program: %s", name.c_str());
	try
	{
		CHECK_GL_EXCEPTION();
	}
	CATCH_AND_LOG_EXCEPTIONS
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */

	ELglGenProgramsARB(1, &id);
	ELglBindProgramARB(GL_VERTEX_PROGRAM_ARB, id);

	str = std::string(reinterpret_cast<char*>(f.get_pointer()), f.get_size());
	s1 << (max_bones_per_mesh * 3);
	pos = str.find("%d");
	if (pos == str.npos)
	{
		EXTENDED_EXCEPTION(ExtendedException::ec_io_error, "File '" << name <<
			"' is invalid.");
	}
	str.replace(pos, 2, s1.str());
	s2 << (max_bones_per_mesh * 3 - 1);
	pos = str.find("%d");
	if (pos == str.npos)
	{
		EXTENDED_EXCEPTION(ExtendedException::ec_io_error, "File '" << name <<
			"' is invalid.");
	}
	str.replace(pos, 2, s2.str());

	ELglProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
		str.size(), str.c_str());

	if (glGetError() != GL_NO_ERROR)
	{
		log_error("Vertex program:\n %s", str.c_str());
		EXTENDED_EXCEPTION(ExtendedException::ec_opengl_error, "Error: '" <<
			glGetString(GL_PROGRAM_ERROR_STRING_ARB) << "' in file '" << name << "'");
	}

	ELglGetProgramivARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &support);

	if (support != GL_TRUE)
	{
		log_error("Vertex program:\n %s", str.c_str());
		EXTENDED_EXCEPTION(ExtendedException::ec_opengl_error, "Error: vertex program'" <<
			name << "' needs too much resources.");
	}

	ELglBindProgramARB(GL_VERTEX_PROGRAM_ARB, 0);

#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	try
	{
		CHECK_GL_EXCEPTION();
	}
	CATCH_AND_LOG_EXCEPTIONS
	log_info("done load_vertex_program: %s", name.c_str());
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */

	return id;
}

extern "C" void unload_vertex_programs()
{
#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	log_info("start unload_vertex_programs");
	try
	{
		CHECK_GL_EXCEPTION();
	}
	CATCH_AND_LOG_EXCEPTIONS
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
	ELglDeleteProgramsARB(5, vertex_program_ids);
	memset(vertex_program_ids, 0, sizeof(vertex_program_ids));
#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	try
	{
		CHECK_GL_EXCEPTION();
	}
	CATCH_AND_LOG_EXCEPTIONS
	log_info("done unload_vertex_programs");
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
}

static inline void render_mesh_shader(actor_types *a, actor *act, Sint32 index, const HardwareMeshData &hmd, const bool use_glow)
{
	Uint32 element_index, count, i;
	Sint32 bone_id, glow;
	float reverse_scale;
	CalSkeleton *skel;

#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	log_info("start render_mesh_shader");
	try
	{
		CHECK_GL_EXCEPTION();
	}
	CATCH_AND_LOG_EXCEPTIONS
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */

	if (index >= 0)
	{
		bone_id = -1;
		glow = -1;

		if (act->is_enhanced_model)
		{
			if (act->cur_shield >= 0)
			{
				if ((Uint32)a->shield[act->cur_shield].mesh_index == hmd.get_mesh_index())
				{
					bone_id = 21;
					glow = a->shield[act->cur_shield].glow;
				}
			}
			if (act->cur_weapon >= 0)
			{
				if ((Uint32)a->weapon[act->cur_weapon].mesh_index == hmd.get_mesh_index())
				{
					bone_id = 26;
					glow = a->weapon[act->cur_weapon].glow;
				}
			}
		}

		if (use_glow)
		{
			if (glow > 0)
			{
				ELglVertexAttrib4f(4, glow_colors[glow].r * 3.0f,
					glow_colors[glow].g * 3.0f, glow_colors[glow].b * 3.0f, 1.0f);
			}
			else
			{
				if (act->ghost || (act->buffs & BUFF_INVISIBILITY))
				{
					if ((act->buffs & BUFF_INVISIBILITY))
					{
						ELglVertexAttrib4f(4, 1.0f, 1.0f, 1.0f, 0.25f);
					}
					else
					{
						ELglVertexAttrib4f(4, 1.0f, 1.0f, 1.0f, 1.0f);
					}
				}
				else
				{
					ELglVertexAttrib4f(4, -1.0f, -1.0f, -1.0f, -1.0f);
				}
			}
		}

		a->hardware_model->selectHardwareMesh(index);

#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
		log_info("start setting parameter");
		if (have_extension(ext_gpu_program_parameters) && use_ext_gpu_program_parameters)
		{
			log_info("Using GL_EXT_gpu_program_parameters");
			ELglProgramLocalParameters4fvEXT(GL_VERTEX_PROGRAM_ARB, 0,
				a->hardware_model->getBoneCount() * 3, hmd.get_buffer());
		}
		else
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
		{
			count = a->hardware_model->getBoneCount() * 3;
			for (i = 0; i < count; i++)
			{
				ELglProgramLocalParameter4fvARB(GL_VERTEX_PROGRAM_ARB, i,
					hmd.get_buffer(i * 4));
			}
		}

#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
		try
		{
			CHECK_GL_EXCEPTION();
		}
		CATCH_AND_LOG_EXCEPTIONS
		log_info("done setting parameter");
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
	
		if (bone_id != -1)
		{
			glPushMatrix();
			reverse_scale = 1.0f / a->skel_scale;

			skel = act->calmodel->getSkeleton();

			const CalVector &point = skel->getBone(bone_id)->getTranslationAbsolute();

			glTranslatef(point[0], point[1], point[2]);
			glScalef(reverse_scale, reverse_scale, reverse_scale);
			glTranslatef(-point[0], -point[1], -point[2]);

		}

		element_index = a->hardware_model->getStartIndex() * a->index_size;

#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
		if (use_render_mesh_shader)
		{
			glDrawElements(GL_TRIANGLES, a->hardware_model->getFaceCount() * 3,
				a->index_type, reinterpret_cast<void*>(element_index));
		}
		try
		{
			CHECK_GL_EXCEPTION();
		}
		CATCH_AND_LOG_EXCEPTIONS
#else	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
		glDrawElements(GL_TRIANGLES, a->hardware_model->getFaceCount() * 3, a->index_type,
			reinterpret_cast<void*>(element_index));
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */

		if (bone_id != -1)
		{
			glPopMatrix();
		}
	}

#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	try
	{
		CHECK_GL_EXCEPTION();
	}
	CATCH_AND_LOG_EXCEPTIONS
	log_info("done render_mesh_shader");
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
}

extern "C" void set_actor_animation_program(Uint32 pass, Uint32 ghost)
{
	Uint32 index, i;
	VECTOR4 zero;
	VECTOR4 one;

#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	log_info("start set_actor_animation_program: %d", pass);
	try
	{
		CHECK_GL_EXCEPTION();
	}
	CATCH_AND_LOG_EXCEPTIONS
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */

	switch (pass)
	{
		case DEFAULT_RENDER_PASS:
		{
			if (ghost != 0)
			{
				index = 3;
			}
			else
			{
				index = 0;
			}

			use_normals = ghost == 0;

			break;
		}
		case REFLECTION_RENDER_PASS:
		{
			assert(ghost == 0);

			index = 0;

			use_normals = ghost == 0;

			break;
		}
		case DEPTH_RENDER_PASS:
		{
			assert(ghost == 0);

			index = 1;

			use_normals = false;

			break;
		}
		case SHADOW_RENDER_PASS:
		{
			if (ghost != 0)
			{
				index = 4;
			}
			else
			{
				index = 2;
			}

			use_normals = ghost == 0;

			break;
		}
		default:
		{
			index = 0;
			break;
		}
	}

	zero[0] = 0.0f;
	zero[1] = 0.0f;
	zero[2] = 0.0f;
	zero[3] = 0.0f;

	one[0] = 1.0f;
	one[1] = 1.0f;
	one[2] = 1.0f;
	one[3] = 1.0f;

#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	try
	{
		CHECK_GL_EXCEPTION();
	}
	CATCH_AND_LOG_EXCEPTIONS
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */

	for (i = 0; i < 8; i++)
	{
		if (glIsEnabled(GL_LIGHT0 + i) == GL_FALSE)
		{
			glLightfv(GL_LIGHT0 + i, GL_POSITION, one);
			glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, zero);
			glLightfv(GL_LIGHT0 + i, GL_SPECULAR, zero);
			glLightfv(GL_LIGHT0 + i, GL_AMBIENT, zero);
		}
		glLightf(GL_LIGHT0 + i, GL_CONSTANT_ATTENUATION, 1.0f);
		glLightf(GL_LIGHT0 + i, GL_QUADRATIC_ATTENUATION, 0.0f);
	}

	glEnable(GL_VERTEX_PROGRAM_ARB);

	ELglEnableVertexAttribArrayARB(0);
	ELglEnableVertexAttribArrayARB(1);
	ELglEnableVertexAttribArrayARB(3);

	ELglBindProgramARB(GL_VERTEX_PROGRAM_ARB, vertex_program_ids[index]);

#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	try
	{
		CHECK_GL_EXCEPTION();
	}
	CATCH_AND_LOG_EXCEPTIONS
	log_info("end set_actor_animation_program: %d", pass);
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
}

extern "C" void disable_actor_animation_program()
{
#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	log_info("start disable_actor_animation_program");
	try
	{
		CHECK_GL_EXCEPTION();
	}
	CATCH_AND_LOG_EXCEPTIONS
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */

	ELglBindProgramARB(GL_VERTEX_PROGRAM_ARB, 0);

	ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	ELglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

	ELglDisableVertexAttribArrayARB(0);
	ELglDisableVertexAttribArrayARB(1);
	ELglDisableVertexAttribArrayARB(2);
	ELglDisableVertexAttribArrayARB(3);
	ELglDisableVertexAttribArrayARB(8);

	glDisable(GL_VERTEX_PROGRAM_ARB);

	last_actor_type = -1;

#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	try
	{
		CHECK_GL_EXCEPTION();
	}
	CATCH_AND_LOG_EXCEPTIONS
	log_info("done disable_actor_animation_program");
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
}

#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
static HardwareMeshData hmd(-1, 8196);
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */

extern "C" int load_vertex_programs()
{
	GLint max_parameters, max_instructions;
#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	int i;
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
	try
	{
		CHECK_GL_EXCEPTION();
		ELglGetProgramivARB(GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_PARAMETERS_ARB,
			&max_parameters);
		log_info("Max parameters per program: %d", max_parameters);
		if (max_parameters > 256)
		{
			max_parameters = 256;
			log_info("Using only 256 parameters to be save.");
		}
		ELglGetProgramivARB(GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_INSTRUCTIONS_ARB,
			&max_instructions);
		log_info("Max instructions per program: %d", max_instructions);
		max_bones_per_mesh = (max_parameters - 43) / 3;
		log_info("Max bones per mesh: %d", max_bones_per_mesh);
		if (max_parameters < 46)
		{
			EXTENDED_EXCEPTION(ExtendedException::ec_opengl_error, "Not enought " << 
				"parameters available.");			
		}
		vertex_program_ids[0] = load_vertex_program("shaders/anim.vert");
		vertex_program_ids[1] = load_vertex_program("shaders/anim_depth.vert");
		vertex_program_ids[2] = load_vertex_program("shaders/anim_shadow.vert");
		vertex_program_ids[3] = load_vertex_program("shaders/anim_ghost.vert");
		vertex_program_ids[4] = load_vertex_program("shaders/anim_ghost_shadow.vert");
#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
		for (i = 0; i < 2049; i++)
		{
			hmd.set_buffer_value(i * 12 +  0, 1.0f);
			hmd.set_buffer_value(i * 12 +  1, 0.0f);
			hmd.set_buffer_value(i * 12 +  2, 0.0f);
			hmd.set_buffer_value(i * 12 +  3, 0.0f);
			hmd.set_buffer_value(i * 12 +  4, 0.0f);
			hmd.set_buffer_value(i * 12 +  5, 1.0f);
			hmd.set_buffer_value(i * 12 +  6, 0.0f);
			hmd.set_buffer_value(i * 12 +  7, 0.0f);
			hmd.set_buffer_value(i * 12 +  8, 0.0f);
			hmd.set_buffer_value(i * 12 +  9, 0.0f);
			hmd.set_buffer_value(i * 12 + 10, 1.0f);
			hmd.set_buffer_value(i * 12 + 11, 0.0f);
		}
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
	}
	CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(0)
#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	try
	{
		CHECK_GL_EXCEPTION();
	}
	CATCH_AND_LOG_EXCEPTIONS
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */

	return 1;
}

extern "C" void cal_render_actor_shader(actor *act, Uint32 use_lightning, Uint32 use_textures, Uint32 use_glow)
{
	actor_types* a;
	IntMap* im;
	float s;
#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	int i;
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */

#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	log_info("start cal_render_actor_shader");
	try
	{
		CHECK_GL_EXCEPTION();
	}
	CATCH_AND_LOG_EXCEPTIONS
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */

	assert(act->calmodel);

	s = get_actor_scale(act);

	if (s != 1.0f)
	{
		glScalef(s, s, s);
	}

	a = &actors_defs[act->actor_type];

	if (last_actor_type != act->actor_type)
	{
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, a->vertex_buffer);
		ELglVertexAttribPointerARB(0, 3, GL_FLOAT, GL_FALSE, sizeof(ActorVertex),
			reinterpret_cast<void*>(0 * sizeof(float) + 0 * sizeof(Uint8)));
		ELglVertexAttribPointerARB(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ActorVertex),
			reinterpret_cast<void*>(3 * sizeof(float) + 0 * sizeof(Uint8)));
		if (use_normals && use_lightning)
		{
			ELglEnableVertexAttribArrayARB(2);
			ELglVertexAttribPointerARB(2, 3, GL_FLOAT, GL_FALSE, sizeof(ActorVertex),
				reinterpret_cast<void*>(3 * sizeof(float) + 4 * sizeof(Uint8)));
		}
		else
		{
			ELglDisableVertexAttribArrayARB(2);
		}
		ELglVertexAttribPointerARB(3, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(ActorVertex),
			reinterpret_cast<void*>(6 * sizeof(float) + 4 * sizeof(Uint8)));
		if (use_textures)
		{
			ELglEnableVertexAttribArrayARB(8);
			ELglVertexAttribPointerARB(8, 2, GL_FLOAT, GL_FALSE, sizeof(ActorVertex),
				reinterpret_cast<void*>(6 * sizeof(float) + 8 * sizeof(Uint8)));
		}
		else
		{
			ELglDisableVertexAttribArrayARB(8);
		}
		ELglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, a->index_buffer);
		last_actor_type = act->actor_type;
	}
#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	try
	{
		CHECK_GL_EXCEPTION();
	}
	CATCH_AND_LOG_EXCEPTIONS
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */

	im = reinterpret_cast<IntMap*>(act->calmodel->getUserData());

	assert(im);

#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	if (use_render_attached_meshs)
	{
		for (IntMap::iterator it = im->begin(); it != im->end(); it++)
		{
			render_mesh_shader(a, act, it->first, it->second, use_glow);
		}
	}
	else
	{
		for (i = 0; i < a->hardware_model->getHardwareMeshCount(); i++)
		{
			render_mesh_shader(a, act, i, hmd, use_glow);
		}
	}
#else	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
#ifdef	USE_BOOST
	BOOST_FOREACH(IntMap::value_type it, *im)
	{
		render_mesh_shader(a, act, it.first, it.second, use_glow);
	}
#else	/* USE_BOOST */
	for (IntMap::iterator it = im->begin(); it != im->end(); it++)
	{
		render_mesh_shader(a, act, it->first, it->second, use_glow);
	}
#endif	/* USE_BOOST */
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	try
	{
		CHECK_GL_EXCEPTION();
	}
	CATCH_AND_LOG_EXCEPTIONS
	log_info("done cal_render_actor_shader");
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
}

static inline void calculate_face_and_vertex_count(CalCoreModel* core_model, Uint32 &face_count,
	Uint32 &vertex_count)
{
	CalCoreMesh *core_mesh;
	Sint32 i, j;
	Sint32 count;

	face_count = 0;
	vertex_count = 0;

#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	log_info("start calculate_face_and_vertex_count");
	try
	{
		CHECK_GL_EXCEPTION();
	}
	CATCH_AND_LOG_EXCEPTIONS
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */

	for (i = 0; i < core_model->getCoreMeshCount(); i++)
	{
		core_mesh = core_model->getCoreMesh(i);
		count = core_mesh->getCoreSubmeshCount();
		for (j = 0; j < count; j++)
		{		
			CalCoreSubmesh *core_sub_mesh = core_mesh->getCoreSubmesh(j);
			face_count += core_sub_mesh->getFaceCount();
			vertex_count += core_sub_mesh->getVertexCount();
		}
	}

#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	try
	{
		CHECK_GL_EXCEPTION();
	}
	CATCH_AND_LOG_EXCEPTIONS
	log_info("done calculate_face_and_vertex_count");
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
}

#ifdef	USE_BOOST
static inline void convert_indices(Uint32Array data, const CalIndexArray indices, const Uint32 count)
#else	/* USE_BOOST */
static inline void convert_indices(Uint32* data, const CalIndex* indices, const Uint32 count)
#endif	/* USE_BOOST */
{
	Uint32 i;

	for (i = 0; i < count * 3; i++)
	{ 
		data[i] = indices[i];
	}
}

#ifdef	USE_BOOST
static inline void pack_indices(Uint16Array data, const Uint32Array indices, const Uint32 count)
#else	/* USE_BOOST */
static inline void pack_indices(Uint16* data, const Uint32* indices, const Uint32 count)
#endif	/* USE_BOOST */
{
	Uint32 i;

	for (i = 0; i < count * 3; i++)
	{ 
		data[i] = indices[i];
	}
}

extern "C" void build_buffers(actor_types* a)
{
#ifdef	USE_BOOST
	FloatArray vertex_buffer;
	FloatArray normal_buffer;
	FloatArray weight_buffer;
	FloatArray matrix_index_buffer;
	FloatArray texture_coordinate_buffer;
	CalIndexArray indices;
	ActorVertexArray buffer;
	Uint32Array data32;
#else	/* USE_BOOST */
	float* vertex_buffer;
	float* normal_buffer;
	float* weight_buffer;
	float* matrix_index_buffer;
	float* texture_coordinate_buffer;
	CalIndex* indices;
	ActorVertex* buffer;
	Uint32* data32;
#endif	/* USE_BOOST */
	Uint32 face_count, vertex_count, max_index;
	Sint32 i, j;
	Uint32 idx, offset, count;
#ifdef	USE_ACTORS_OPTIMIZER
	MD5_DIGEST digest;
	Uint32 size, tmp;
	bool loaded;
#endif	/* USE_ACTORS_OPTIMIZER */

	face_count = 0;
	vertex_count = 0;

#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	log_info("start build_buffers: %s", a->actor_name);
	try
	{
		CHECK_GL_EXCEPTION();
	}
	CATCH_AND_LOG_EXCEPTIONS
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */

	calculate_face_and_vertex_count(a->coremodel, face_count, vertex_count);

	a->hardware_model = new CalHardwareModel(a->coremodel);

#ifdef	USE_BOOST
	vertex_buffer = FloatArray(new float[32768 * 3]);
	normal_buffer = FloatArray(new float[32768 * 3]);
	weight_buffer = FloatArray(new float[32768 * 4]);
	matrix_index_buffer = FloatArray(new float[32768 * 4]);
	texture_coordinate_buffer = FloatArray(new float[32768 * 2]);
	indices = CalIndexArray(new CalIndex[65536 * 3]);

	a->hardware_model->setVertexBuffer(reinterpret_cast<char*>(vertex_buffer.get()),
		3 * sizeof(float));
	a->hardware_model->setNormalBuffer(reinterpret_cast<char*>(normal_buffer.get()),
		3 * sizeof(float));
	a->hardware_model->setWeightBuffer(reinterpret_cast<char*>(weight_buffer.get()),
		4 * sizeof(float));
	a->hardware_model->setMatrixIndexBuffer(reinterpret_cast<char*>(matrix_index_buffer.get()),
		4 * sizeof(float));
	a->hardware_model->setTextureCoordNum(1);
	a->hardware_model->setTextureCoordBuffer(0,
		reinterpret_cast<char*>(texture_coordinate_buffer.get()), 2 * sizeof(float));
	a->hardware_model->setIndexBuffer(indices.get());
#else	/* USE_BOOST */
	vertex_buffer = new float[32768 * 3];
	normal_buffer = new float[32768 * 3];
	weight_buffer = new float[32768 * 4];
	matrix_index_buffer = new float[32768 * 4];
	texture_coordinate_buffer = new float[32768 * 2];
	indices = new CalIndex[65536 * 3];

	a->hardware_model->setVertexBuffer(reinterpret_cast<char*>(vertex_buffer),
		3 * sizeof(float));
	a->hardware_model->setNormalBuffer(reinterpret_cast<char*>(normal_buffer),
		3 * sizeof(float));
	a->hardware_model->setWeightBuffer(reinterpret_cast<char*>(weight_buffer),
		4 * sizeof(float));
	a->hardware_model->setMatrixIndexBuffer(reinterpret_cast<char*>(matrix_index_buffer),
		4 * sizeof(float));
	a->hardware_model->setTextureCoordNum(1);
	a->hardware_model->setTextureCoordBuffer(0,
		reinterpret_cast<char*>(texture_coordinate_buffer), 2 * sizeof(float));
	a->hardware_model->setIndexBuffer(indices);
#endif	/* USE_BOOST */

	a->hardware_model->load(0, 0, max_bones_per_mesh);

#ifdef	USE_BOOST
	buffer = ActorVertexArray(new ActorVertex[a->hardware_model->getTotalVertexCount()]);
#else	/* USE_BOOST */
	buffer = new ActorVertex[a->hardware_model->getTotalVertexCount()];
#endif	/* USE_BOOST */

	idx = 0;

	for (i = 0; i < a->hardware_model->getTotalVertexCount(); i++)
	{
//		buffer[i].m_bone_count = 0;
		for (j = 0; j < 3; j++)
		{
			buffer[i].m_vertex[j] = vertex_buffer[i * 3 + j];
		}
		for (j = 0; j < 4; j++)
		{
			buffer[i].m_weight[j] = weight_buffer[i * 4 + j] * 255.0f;
/*			if (weight_buffer[i * 4 + j] > 0.0f)
			{
				buffer[i].m_bone_count = j + 1;
			}*/
		}
		for (j = 0; j < 3; j++)
		{
			buffer[i].m_normal[j] = normal_buffer[i * 3 + j];
		}
		for (j = 0; j < 4; j++)
		{
			buffer[i].m_index[j] = matrix_index_buffer[i * 4 + j];
		}
		for (j = 0; j < 2; j++)
		{
			buffer[i].m_texture[j] = texture_coordinate_buffer[i * 2 + j];
		}
	}

	ELglGenBuffersARB(1, &a->vertex_buffer);
	ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, a->vertex_buffer);
#ifdef	USE_BOOST
	ELglBufferDataARB(GL_ARRAY_BUFFER_ARB, a->hardware_model->getTotalVertexCount() *
		sizeof(ActorVertex), buffer.get(), GL_STATIC_DRAW_ARB);
#else	/* USE_BOOST */
	ELglBufferDataARB(GL_ARRAY_BUFFER_ARB, a->hardware_model->getTotalVertexCount() *
		sizeof(ActorVertex), buffer, GL_STATIC_DRAW_ARB);
#endif	/* USE_BOOST */
	ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	max_index = 0;

	for (i = 0; i < a->hardware_model->getHardwareMeshCount(); i++)
	{
		a->hardware_model->selectHardwareMesh(i);

		offset = a->hardware_model->getStartIndex();
		count = a->hardware_model->getBaseVertexIndex();

		for (j = 0; j < a->hardware_model->getFaceCount(); j++)
		{
			indices[j * 3 + 0 + offset] += count;
			indices[j * 3 + 1 + offset] += count;
			indices[j * 3 + 2 + offset] += count;
			max_index = std::max(max_index, static_cast<Uint32>(indices[j * 3 + 0 + offset]));
			max_index = std::max(max_index, static_cast<Uint32>(indices[j * 3 + 1 + offset]));
			max_index = std::max(max_index, static_cast<Uint32>(indices[j * 3 + 2 + offset]));
		}
	}

	ELglGenBuffersARB(1, &a->index_buffer);
	ELglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, a->index_buffer);

#ifdef	USE_BOOST
	data32 = Uint32Array(new Uint32[a->hardware_model->getTotalFaceCount() * 3]);
#else	/* USE_BOOST */
	data32 = new Uint32[a->hardware_model->getTotalFaceCount() * 3];
#endif	/* USE_BOOST */
	convert_indices(data32, indices, a->hardware_model->getTotalFaceCount());

#ifdef	USE_ACTORS_OPTIMIZER
	loaded = false;

	size = a->hardware_model->getTotalFaceCount() * 3 * sizeof(Uint32);

	{
		MD5 md5;

		MD5Open(&md5);
#ifdef	USE_BOOST
		MD5Digest(&md5, data32.get(), size);
#else	/* USE_BOOST */
		MD5Digest(&md5, data32, size);
#endif	/* USE_BOOST */
		MD5Close(&md5, digest);
	}

	try
	{
		std::ostringstream file_name;

		file_name << "cache/actor_" << a->actor_type << "_" << max_bones_per_mesh << ".elc";

		if (eternal_lands::el_file::file_exists(file_name.str(), get_path_config_base()))
		{
			eternal_lands::el_file file(file_name.str(), true, get_path_config_base());
			if (file.get_size() != (size + sizeof(MD5_DIGEST) + sizeof(Uint32) * 2))
			{
				EXTENDED_EXCEPTION(ExtendedException::ec_io_error, "File '" <<
					file_name.str() << "' has wrong size. Size " << (size +
					sizeof(MD5_DIGEST) + sizeof(Uint32) * 2) << " expected, "
					<< "but found size " << file.get_size());
			}
			if (memcmp(digest, file.get_pointer(), sizeof(MD5_DIGEST)) != 0)
			{
				EXTENDED_EXCEPTION(ExtendedException::ec_io_error, "File '" <<
					file_name.str() << "' has wrong md5 for data.");
			}
			file.seek(sizeof(MD5_DIGEST), SEEK_SET);
			file.read(sizeof(Uint32), &tmp);
			if (tmp != size)
			{
				EXTENDED_EXCEPTION(ExtendedException::ec_io_error, "File '" <<
					file_name.str() << "' is for wrong number of indices.");
			}
			file.read(sizeof(Uint32), &tmp);
			if (tmp != max_bones_per_mesh)
			{
				EXTENDED_EXCEPTION(ExtendedException::ec_io_error, "File '" <<
					file_name.str() << "' is for wrong number of bones.");
			}
#ifdef	USE_BOOST
			file.read(size, data32.get());
#else	/* USE_BOOST */
			file.read(size, data32);
#endif	/* USE_BOOST */
			loaded = true;
		}
	}
	CATCH_AND_LOG_EXCEPTIONS
	
	if (!loaded)
	{
		std::ostringstream file_name;

		for (i = 0; i < a->hardware_model->getHardwareMeshCount(); i++)
		{
			a->hardware_model->selectHardwareMesh(i);

			count = a->hardware_model->getFaceCount();

			optimize_vertex_cache_order(data32, a->hardware_model->getStartIndex(), count * 3, count * 3);
		}

		file_name << get_path_config_base() << "cache/actor_" << a->actor_type << "_" << max_bones_per_mesh << ".elc";

		log_info("Rebuilding file: %s", file_name.str().c_str());

		mkdir_tree(file_name.str().c_str(), 0);

		std::ofstream file(file_name.str().c_str());

		file.write(reinterpret_cast<char*>(digest), sizeof(MD5_DIGEST));
		file.write(reinterpret_cast<char*>(&size), sizeof(Uint32));
		file.write(reinterpret_cast<char*>(&max_bones_per_mesh), sizeof(Uint32));
#ifdef	USE_BOOST
		file.write(reinterpret_cast<char*>(data32.get()), size);
#else	/* USE_BOOST */
		file.write(reinterpret_cast<char*>(data32), size);
#endif	/* USE_BOOST */
		file.close();
	}
#endif	/* USE_ACTORS_OPTIMIZER */

	if (max_index <= std::numeric_limits<Uint16>::max())
	{
#ifdef	USE_BOOST
		Uint16Array data16;

		data16 = Uint16Array(new Uint16[a->hardware_model->getTotalFaceCount() * 3]);
#else	/* USE_BOOST */
		Uint16* data16;

		data16 = new Uint16[a->hardware_model->getTotalFaceCount() * 3];
#endif	/* USE_BOOST */
		pack_indices(data16, data32, a->hardware_model->getTotalFaceCount());
		a->index_type = GL_UNSIGNED_SHORT;
		a->index_size = sizeof(GLushort);
#ifdef	USE_BOOST
		ELglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
			a->hardware_model->getTotalFaceCount() * 3 * sizeof(GLushort), data16.get(),
			GL_STATIC_DRAW_ARB);
#else	/* USE_BOOST */
		ELglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
			a->hardware_model->getTotalFaceCount() * 3 * sizeof(GLushort), data16,
			GL_STATIC_DRAW_ARB);
#endif	/* USE_BOOST */
	}
	else
	{
		a->index_type = GL_UNSIGNED_INT;
		a->index_size = sizeof(GLuint);
#ifdef	USE_BOOST
		ELglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
			a->hardware_model->getTotalFaceCount() * 3 * sizeof(GLuint), data32.get(),
			GL_STATIC_DRAW_ARB);
#else	/* USE_BOOST */
		ELglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
			a->hardware_model->getTotalFaceCount() * 3 * sizeof(GLuint), data32,
			GL_STATIC_DRAW_ARB);
#endif	/* USE_BOOST */
	}

	ELglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	try
	{
		CHECK_GL_EXCEPTION();
	}
	CATCH_AND_LOG_EXCEPTIONS
	log_info("done build_buffers: %s", a->actor_name);
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
}

static inline void set_transformation_buffer(actor_types *a, actor *act, const Uint32 index,
	HardwareMeshData &hmd)
{
	Sint32 i, count;
	const std::vector<CalBone *>& vectorBone = act->calmodel->getSkeleton()->getVectorBone();

	a->hardware_model->selectHardwareMesh(index);

	count = a->hardware_model->getBoneCount();

	for (i = 0; i < count; i++)
	{
		const CalVector &translationBoneSpace = vectorBone[a->hardware_model->getVectorHardwareMesh()[index].m_vectorBonesIndices[i]]->getTranslationBoneSpace();
		const CalMatrix &rotationMatrix = vectorBone[a->hardware_model->getVectorHardwareMesh()[index].m_vectorBonesIndices[i]]->getTransformMatrix();

		hmd.set_buffer_value(i * 12 +  0, rotationMatrix.dxdx);
		hmd.set_buffer_value(i * 12 +  1, rotationMatrix.dxdy);
		hmd.set_buffer_value(i * 12 +  2, rotationMatrix.dxdz);
		hmd.set_buffer_value(i * 12 +  3, translationBoneSpace.x);
		hmd.set_buffer_value(i * 12 +  4, rotationMatrix.dydx);
		hmd.set_buffer_value(i * 12 +  5, rotationMatrix.dydy);
		hmd.set_buffer_value(i * 12 +  6, rotationMatrix.dydz);
		hmd.set_buffer_value(i * 12 +  7, translationBoneSpace.y);
		hmd.set_buffer_value(i * 12 +  8, rotationMatrix.dzdx);
		hmd.set_buffer_value(i * 12 +  9, rotationMatrix.dzdy);
		hmd.set_buffer_value(i * 12 + 10, rotationMatrix.dzdz);
		hmd.set_buffer_value(i * 12 + 11, translationBoneSpace.z);
	}
}

extern "C" void set_transformation_buffers(actor* act)
{
	IntMap* im;
	IntMap::iterator it;
	actor_types* a;

#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	if (!use_set_transformation_buffers)
	{
		return;
	}
	log_info("start set_transformation_buffers");
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
	im = reinterpret_cast<IntMap*>(act->calmodel->getUserData());

	a = &actors_defs[act->actor_type];

	assert(im);

	for (it = im->begin(); it != im->end(); it++)
	{
		set_transformation_buffer(a, act, it->first, it->second);
	}
#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	log_info("done set_transformation_buffers");
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
}

extern "C" void build_actor_bounding_box(actor* a)
{
	CalSkeleton* cs;
	Uint32 i;
	float t;

#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	if (!use_build_actor_bounding_box)
	{
		return;
	}
	log_info("start build_actor_bounding_box");
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
	if (a->calmodel)
	{
		cs = a->calmodel->getSkeleton();
		cs->getBoneBoundingBox(a->bbox.bbmin, a->bbox.bbmax);

		for (i = 0; i < 3; i++)
		{
			t = a->bbox.bbmax[i] - a->bbox.bbmin[i];
			a->bbox.bbmin[i] -= std::max(t * 0.25f - 0.25f, 0.1f);
			a->bbox.bbmax[i] += std::max(t * 0.25f - 0.25f, 0.1f);
		}
	}
#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	log_info("done build_actor_bounding_box");
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
}

extern "C" CalModel *model_new(CalCoreModel* pCoreModel)
{
	CalModel* tmp;

#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	log_info("start model_new");
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
	tmp = new CalModel(pCoreModel);

	tmp->setUserData(new IntMap());

#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	log_info("done model_new");
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
	return tmp;
}

extern "C" void model_delete(CalModel *self)
{
#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	log_info("start model_delete");
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
	if (self)
	{
		delete reinterpret_cast<IntMap*>(self->getUserData());
		self->setUserData(0);
	}
	delete self;
#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	log_info("done model_delete");
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
}

extern "C" void model_attach_mesh(actor *act, int mesh_id)
{
	IntMap* im;
	actor_types* a;
	int i, count;

#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	if (!use_model_attach_and_detach_mesh)
	{
		return;
	}
	log_info("start model_attach_mesh");
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
	assert(act);

	assert(act->calmodel);

	act->calmodel->attachMesh(mesh_id);

	im = reinterpret_cast<IntMap*>(act->calmodel->getUserData());

	assert(im);

	a = &actors_defs[act->actor_type];

	if (a->hardware_model)
	{
		count = a->hardware_model->getVectorHardwareMesh().size();

		for (i = 0; i < count; i++)
		{
			if (a->hardware_model->getVectorHardwareMesh()[i].meshId == mesh_id)
			{
				std::pair<Sint32, HardwareMeshData> p = std::pair<Sint32,
					HardwareMeshData>(i, HardwareMeshData(mesh_id,
					max_bones_per_mesh * 3));
				im->insert(p);
				set_transformation_buffer(a, act, p.first, p.second);
			}
		}
	}
#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	log_info("done model_attach_mesh");
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
}

extern "C" void model_detach_mesh(actor *act, int mesh_id)
{
	IntMap* im;
	actor_types* a;
	int i, count;

#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	if (!use_model_attach_and_detach_mesh)
	{
		return;
	}
	log_info("start model_detach_mesh");
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */

	assert(act);

	assert(act->calmodel);

	act->calmodel->detachMesh(mesh_id);

	im = reinterpret_cast<IntMap*>(act->calmodel->getUserData());

	assert(im);

	a = &actors_defs[act->actor_type];

	if (a->hardware_model)
	{
		count = a->hardware_model->getVectorHardwareMesh().size();

		for (i = 0; i < count; i++)
		{
			if (a->hardware_model->getVectorHardwareMesh()[i].meshId == mesh_id)
			{
				im->erase(i);
			}
		}
	}
#ifdef	VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG
	log_info("done model_detach_mesh");
#endif	/* VERTEX_PROGRAM_ACTOR_ANIMATION_DEBUG */
}

