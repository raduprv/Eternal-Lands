#include "actor_init.h"
#include "load_gl_extensions.h"
#include <cal3d/cal3d.h>
#ifdef	USE_BOOST
#include <boost/shared_array.hpp>
#endif	/* USE_BOOST */
#include <map>
#include "bbox_tree.h"
#include "io/elfile.hpp"
#include "exceptions/extendedexception.hpp"

Uint32 use_animation_program = 1;

typedef std::map<Sint32, Sint32> IntMap;

GLuint vertex_program_ids[5];

static inline GLuint load_vertex_program(const std::string &name)
{
	GLuint id;
	eternal_lands::el_file f(name, true);

	ELglGenProgramsARB(1, &id);
	ELglBindProgramARB(GL_VERTEX_PROGRAM_ARB, id);
	ELglProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
		f.get_size(), f.get_pointer());

	CHECK_GL_EXCEPTION();

	ELglBindProgramARB(GL_VERTEX_PROGRAM_ARB, 0);

	return id;
}

extern "C" int load_vertex_programs()
{
	try
	{
		vertex_program_ids[0] = load_vertex_program("shaders/anim.vert");
		vertex_program_ids[1] = load_vertex_program("shaders/anim_depth.vert");
		vertex_program_ids[2] = load_vertex_program("shaders/anim_shadow.vert");
		vertex_program_ids[3] = load_vertex_program("shaders/anim_ghost.vert");
		vertex_program_ids[4] = load_vertex_program("shaders/anim_ghost_shadow.vert");
	}
	CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(0)

	return 1;
}

extern "C" void unload_vertex_programs()
{
	ELglDeleteProgramsARB(5, vertex_program_ids);
}

static inline void set_transformation(actor_types *a, actor *act, Uint32 index)
{
	Sint32 i, count;
	const std::vector<CalBone *>& vectorBone = act->calmodel->getSkeleton()->getVectorBone();

	count = a->hardware_model->getBoneCount();

	for (i = 0; i < count; i++)
	{
		const CalVector &translationBoneSpace = vectorBone[a->hardware_model->getVectorHardwareMesh()[index].m_vectorBonesIndices[i]]->getTranslationBoneSpace();
		const CalMatrix &rotationMatrix = vectorBone[a->hardware_model->getVectorHardwareMesh()[index].m_vectorBonesIndices[i]]->getTransformMatrix();

		ELglProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, i * 3 + 0,
			rotationMatrix.dxdx, rotationMatrix.dxdy, rotationMatrix.dxdz,
			translationBoneSpace.x);
		ELglProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, i * 3 + 1,
			rotationMatrix.dydx, rotationMatrix.dydy, rotationMatrix.dydz,
			translationBoneSpace.y);
		ELglProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, i * 3 + 2,
			rotationMatrix.dzdx, rotationMatrix.dzdy, rotationMatrix.dzdz,
			translationBoneSpace.z);
	}
}

static float buffer[8192];

static inline void set_transformation_ext(actor_types *a, actor *act, Uint32 index)
{
	Sint32 i, count;
	const std::vector<CalBone *>& vectorBone = act->calmodel->getSkeleton()->getVectorBone();

	count = a->hardware_model->getBoneCount();

	for (i = 0; i < count; i++)
	{
		const CalVector &translationBoneSpace = vectorBone[a->hardware_model->getVectorHardwareMesh()[index].m_vectorBonesIndices[i]]->getTranslationBoneSpace();
		const CalMatrix &rotationMatrix = vectorBone[a->hardware_model->getVectorHardwareMesh()[index].m_vectorBonesIndices[i]]->getTransformMatrix();

		buffer[i * 12 +  0] = rotationMatrix.dxdx;
		buffer[i * 12 +  1] = rotationMatrix.dxdy;
		buffer[i * 12 +  2] = rotationMatrix.dxdz;
		buffer[i * 12 +  3] = translationBoneSpace.x;
		buffer[i * 12 +  4] = rotationMatrix.dydx;
		buffer[i * 12 +  5] = rotationMatrix.dydy;
		buffer[i * 12 +  6] = rotationMatrix.dydz;
		buffer[i * 12 +  7] = translationBoneSpace.y;
		buffer[i * 12 +  8] = rotationMatrix.dzdx;
		buffer[i * 12 +  9] = rotationMatrix.dzdy;
		buffer[i * 12 + 10] = rotationMatrix.dzdz;
		buffer[i * 12 + 11] = translationBoneSpace.z;
	}
	ELglProgramLocalParameters4fvEXT(GL_VERTEX_PROGRAM_ARB, 0, count * 3, buffer);
}

static inline void render_mesh_shader(actor_types *a, actor *act, Sint32 index, Sint32 mesh_index)
{
	Uint32 element_index;
	Sint32 bone_id, glow;
	float reverse_scale;
	CalSkeleton *skel;

	if (index >= 0)
	{
		a->hardware_model->selectHardwareMesh(index);

#ifdef	SHADER_EXTRA_DEBUG
		log_info("Actor type name '%s'", a->actor_name);
#endif
		if (have_extension(ext_gpu_program_parameters))
		{
			set_transformation_ext(a, act, index);
		}
		else
		{
			set_transformation(a, act, index);
		}

		bone_id = -1;
#ifdef	SHADER_EXTRA_DEBUG
		log_info("Checking for enganced model");
#endif
		if (act->is_enhanced_model)
		{
#ifdef	SHADER_EXTRA_DEBUG
			log_info("Checking for shield");
			log_info("cur_shield: %d", act->cur_shield);
#endif
			if (act->cur_shield >= 0)
			{
				if (a->shield[act->cur_shield].mesh_index == mesh_index)
				{
					bone_id = 21;
					glow = a->shield[act->cur_shield].glow;
				}
			}
#ifdef	SHADER_EXTRA_DEBUG
			log_info("Checking for weapon");
			log_info("cur_weapon: %d", act->cur_weapon);
#endif
			if (act->cur_weapon >= 0)
			{
				if (a->weapon[act->cur_weapon].mesh_index == mesh_index)
				{
					bone_id = 26;
					glow = a->weapon[act->cur_weapon].glow;
				}
			}
		}

#ifdef	SHADER_EXTRA_DEBUG
		log_info("Checking for scale");
		log_info("bone_id: %d", bone_id);
#endif
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

#ifdef	SHADER_EXTRA_DEBUG
		log_info("Drawing mesh");
#endif
		glDrawElements(GL_TRIANGLES, a->hardware_model->getFaceCount() * 3, a->index_type,
			reinterpret_cast<void*>(element_index));

		if (bone_id != -1)
		{
			glPopMatrix();
		}
	}
}

typedef struct ActorVertex
{
	float m_vertex[3];
	Uint8 m_weight[4];
	float m_normal[3];
	Uint8 m_index[4];
	float m_texture[2];
};

#ifdef	USE_BOOST
typedef boost::shared_array<Uint16> Uint16Array;
typedef boost::shared_array<Uint32> Uint32Array;
typedef boost::shared_array<float> FloatArray;
typedef boost::shared_array<CalIndex> CalIndexArray;
typedef boost::shared_array<ActorVertex> ActorVertexArray;
#endif

extern "C" void set_actor_animation_program(Uint32 pass, Uint32 ghost)
{
	Uint32 index;

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

			break;
		}
		case REFLECTION_RENDER_PASS:
		{
			assert(ghost == 0);

			index = 0;

			break;
		}
		case DEPTH_RENDER_PASS:
		{
			assert(ghost == 0);

			index = 1;

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

			break;
		}
		default:
		{
			index = 0;
			break;
		}
	}

	int i;
	VECTOR4 zero;
	VECTOR4 zero_one;

	zero[0] = 0.0f;
	zero[1] = 0.0f;
	zero[2] = 0.0f;
	zero[3] = 0.0f;

	zero_one[0] = 0.0f;
	zero_one[1] = 0.0f;
	zero_one[2] = 0.0f;
	zero_one[3] = 1.0f;

#ifdef	SHADER_EXTRA_DEBUG
	log_info("Clearing lights");
#endif
	for (i = 0; i < 8; i++)
	{
#ifdef	SHADER_EXTRA_DEBUG
		log_info("Clearing light %d", i);
#endif
		if (glIsEnabled(GL_LIGHT0 + i) == GL_FALSE)
		{
			glLightfv(GL_LIGHT0 + i, GL_POSITION, zero_one);
			glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, zero);
			glLightfv(GL_LIGHT0 + i, GL_SPECULAR, zero);
			glLightfv(GL_LIGHT0 + i, GL_AMBIENT, zero);
		}
	}
#ifdef	SHADER_EXTRA_DEBUG
	log_info("Done Clearing lights");
#endif

	ELglBindProgramARB(GL_VERTEX_PROGRAM_ARB, vertex_program_ids[index]);
}

extern "C" void cal_render_actor_shader(actor *act)
{
	actor_types* a;

	assert(act->calmodel);

	glPushMatrix();

	glScalef(get_actor_scale(act), get_actor_scale(act), get_actor_scale(act));

	a = &actors_defs[act->actor_type];

	glEnable(GL_VERTEX_PROGRAM_ARB);

	ELglEnableVertexAttribArrayARB(0);
	ELglEnableVertexAttribArrayARB(1);
	ELglEnableVertexAttribArrayARB(2);
	ELglEnableVertexAttribArrayARB(3);
	ELglEnableVertexAttribArrayARB(8);

	ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, a->vertex_buffer);
	ELglVertexAttribPointerARB(0, 3, GL_FLOAT, GL_FALSE, sizeof(ActorVertex),
		reinterpret_cast<void*>(0 * sizeof(float) + 0 * sizeof(Uint8)));
	ELglVertexAttribPointerARB(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ActorVertex),
		reinterpret_cast<void*>(3 * sizeof(float) + 0 * sizeof(Uint8)));
	ELglVertexAttribPointerARB(2, 3, GL_FLOAT, GL_FALSE, sizeof(ActorVertex),
		reinterpret_cast<void*>(3 * sizeof(float) + 4 * sizeof(Uint8)));
	ELglVertexAttribPointerARB(3, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(ActorVertex),
		reinterpret_cast<void*>(6 * sizeof(float) + 4 * sizeof(Uint8)));
	ELglVertexAttribPointerARB(8, 2, GL_FLOAT, GL_FALSE, sizeof(ActorVertex),
		reinterpret_cast<void*>(6 * sizeof(float) + 8 * sizeof(Uint8)));
	ELglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, a->index_buffer);

	if (act->is_enhanced_model)
	{
		IntMap* im;
		IntMap::const_iterator it;

		im = reinterpret_cast<IntMap*>(act->calmodel->getUserData());

		if (im != 0)
		{
			for (it = im->begin(); it != im->end(); it++)
			{
				render_mesh_shader(a, act, it->first, it->second);
			}
		}
	}
	else
	{
		int i;

		for (i = 0; i < a->hardware_model->getHardwareMeshCount(); i++)
		{
			render_mesh_shader(a, act, i, -1);
		}
	}

	ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	ELglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

	ELglDisableVertexAttribArrayARB(0);
	ELglDisableVertexAttribArrayARB(1);
	ELglDisableVertexAttribArrayARB(2);
	ELglDisableVertexAttribArrayARB(3);
	ELglDisableVertexAttribArrayARB(8);

	glDisable(GL_VERTEX_PROGRAM_ARB);

	glPopMatrix();
}

static inline void calculate_face_and_vertex_count(CalCoreModel* core_model, Uint32 &face_count,
	Uint32 &vertex_count)
{
	CalCoreMesh *core_mesh;
	Sint32 i, j;
	Sint32 count;

	face_count = 0;
	vertex_count = 0;

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
}

template <typename T>
#ifdef	USE_BOOST
static inline void convert_indices(boost::shared_array<T> data, const CalIndexArray indices, const Uint32 count)
#else	/* USE_BOOST */
static inline void convert_indices(T* data, const CalIndex* indices, const Uint32 count)
#endif	/* USE_BOOST */
{
	Uint32 i;

	for (i = 0; i < count * 3; i++)
	{ 
		data[i] = indices[i];
	}
}

extern "C" void build_buffers(actor_types* a, const Uint32 max_bones_per_mesh)
{
#ifdef	USE_BOOST
	FloatArray vertex_buffer;
	FloatArray normal_buffer;
	FloatArray weight_buffer;
	FloatArray matrix_index_buffer;
	FloatArray texture_coordinate_buffer;
	CalIndexArray indices;
	ActorVertexArray buffer;
#else	/* USE_BOOST */
	float* vertex_buffer;
	float* normal_buffer;
	float* weight_buffer;
	float* matrix_index_buffer;
	float* texture_coordinate_buffer;
	CalIndex* indices;
	ActorVertex* buffer;
#endif	/* USE_BOOST */
	Uint32 face_count, vertex_count, max_index;
	Sint32 i, j;
	Uint32 idx, offset, count;

	face_count = 0;
	vertex_count = 0;

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
		for (j = 0; j < 3; j++)
		{
			buffer[i].m_vertex[j] = vertex_buffer[i * 3 + j];
		}
		for (j = 0; j < 4; j++)
		{
			buffer[i].m_weight[j] = weight_buffer[i * 4 + j] * 255.0f;
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

	if (max_index <= std::numeric_limits<Uint16>::max())
	{
#ifdef	USE_BOOST
		Uint16Array data16;

		data16 = Uint16Array(new Uint16[a->hardware_model->getTotalFaceCount() * 3]);
#else	/* USE_BOOST */
		Uint16* data16;

		data16 = new Uint16[a->hardware_model->getTotalFaceCount() * 3];
#endif	/* USE_BOOST */
		convert_indices<Uint16>(data16, indices, a->hardware_model->getTotalFaceCount());
		a->index_type = GL_UNSIGNED_SHORT;
		a->index_size = 2;
#ifdef	USE_BOOST
		ELglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
			a->hardware_model->getTotalFaceCount() * 3 * 2, data16.get(),
			GL_STATIC_DRAW_ARB);
#else	/* USE_BOOST */
		ELglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
			a->hardware_model->getTotalFaceCount() * 3 * 2, data16,
			GL_STATIC_DRAW_ARB);
#endif	/* USE_BOOST */
	}
	else
	{
#ifdef	USE_BOOST
		Uint32Array data32;

		data32 = Uint32Array(new Uint32[a->hardware_model->getTotalFaceCount() * 3]);
#else	/* USE_BOOST */
		Uint32* data32;

		data32 = new Uint32[a->hardware_model->getTotalFaceCount() * 3];
#endif	/* USE_BOOST */
		convert_indices<Uint32>(data32, indices, a->hardware_model->getTotalFaceCount());
		a->index_type = GL_UNSIGNED_INT;
		a->index_size = 4;
#ifdef	USE_BOOST
		ELglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
			a->hardware_model->getTotalFaceCount() * 3 * 4, data32.get(),
			GL_STATIC_DRAW_ARB);
#else	/* USE_BOOST */
		ELglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
			a->hardware_model->getTotalFaceCount() * 3 * 4, data32,
			GL_STATIC_DRAW_ARB);
#endif	/* USE_BOOST */
	}

	ELglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}

extern "C" void build_actor_bounding_box(actor_types* a)
{
	CalCoreSkeleton* ccs;
	CalVector p[8];
	Uint32 i, j, k;

	ccs = a->coremodel->getCoreSkeleton();
	ccs->calculateBoundingBoxes(a->coremodel);
	std::vector<CalCoreBone *>& v = ccs->getVectorCoreBone();

	for (i = 0; i < 3; i++)
	{
		a->bbox.bbmin[i] = 1.0e30f;
		a->bbox.bbmax[i] = -1.0e30f;
	}

	for (i = 0; i < v.size(); i++)
	{
		v[i]->getBoundingBox().computePoints(p);
		for (j = 0; j < 8; j++)
		{
			for (k = 0; k < 3; k++)
			{
				a->bbox.bbmin[k] = std::min(a->bbox.bbmin[k], p[j][k]);
				a->bbox.bbmax[k] = std::max(a->bbox.bbmax[k], p[j][k]);
			}
		}
	}

	for (i = 0; i < 3; i++)
	{
		a->bbox.bbmin[i] -= 0.5f;
		a->bbox.bbmax[i] += 0.5f;
	}
}

extern "C" CalModel *model_new(CalCoreModel* pCoreModel)
{
	CalModel* tmp;

	tmp = new CalModel(pCoreModel);

	tmp->setUserData(new IntMap());

	return tmp;
}

extern "C" void model_delete(CalModel *self)
{
	if (self)
	{
		delete reinterpret_cast<IntMap*>(self->getUserData());
		self->setUserData(0);
	}
	delete self;
}

extern "C" void model_attach_mesh(actor *act, int mesh_id)
{
	IntMap* im;
	int i, count;

	assert(act);

	assert(act->calmodel);

	act->calmodel->attachMesh(mesh_id);

	im = reinterpret_cast<IntMap*>(act->calmodel->getUserData());

	assert(im);

	if (actors_defs[act->actor_type].hardware_model)
	{
		count = actors_defs[act->actor_type].hardware_model->getVectorHardwareMesh().size();

		for (i = 0; i < count; i++)
		{
			if (actors_defs[act->actor_type].hardware_model->getVectorHardwareMesh()[i].meshId
				== mesh_id)
			{
				im->insert(std::pair<Sint32, Sint32>(i, mesh_id));
			}
		}
	}
}

extern "C" void model_detach_mesh(actor *act, int mesh_id)
{
	IntMap* im;
	int i, count;

	assert(act);

	assert(act->calmodel);

	act->calmodel->detachMesh(mesh_id);

	im = reinterpret_cast<IntMap*>(act->calmodel->getUserData());

	assert(im);

	if (actors_defs[act->actor_type].hardware_model)
	{
		count = actors_defs[act->actor_type].hardware_model->getVectorHardwareMesh().size();

		for (i = 0; i < count; i++)
		{
			if (actors_defs[act->actor_type].hardware_model->getVectorHardwareMesh()[i].meshId
				== mesh_id)
			{
				im->erase(i);
			}
		}
	}
}

extern "C" float cal_get_maxz(actor *act)
{
	CalSkeleton *skel;
	CalBone *bone;
	int index;

	skel= act->calmodel->getSkeleton();
	index = skel->getVectorBone().size();
	bone = skel->getBone(index - 1);

	return bone->getTranslationAbsolute().z;
}

