// I N C L U D E S ////////////////////////////////////////////////////////////

#include <SDL.h>
#include <SDL_image.h>
#include <errno.h>

#include "eye_candy.h"
#include "../platform.h"
#ifdef CLUSTER_INSIDES
#include "../cluster.h"
#endif
#include "../io/elfilewrapper.h"
#include "../textures.h"
#ifdef	NEW_TEXTURES
#include "../load_gl_extensions.h"
#include "../weather.h"
#endif	/* NEW_TEXTURES */

namespace ec
{

#ifdef	NEW_TEXTURES
	namespace
	{

		const float scale_u = 1023.0f / (1024.0f * 1024.0f);
		const float scale_v = 511.0f / (512.0f * 512.0f);
		const float offset_u = 0.5f / 1024.0f;
		const float offset_v = 0.5f / 512.0f;

		#include "eye_candy.tat"

		#define BUILD_TEXTURE_COORDINATES(u, v, u_size, v_size)	\
			u, v, u + u_size, v, u + u_size, v + v_size, u, v + v_size, 

		#define BUILD_EYE_CANDY_TEXTURE_COORDINATES(index, img, name, x, y, w, h)	\
			BUILD_TEXTURE_COORDINATES(x * scale_u + offset_u, y * scale_v + offset_v, w * scale_u, h * scale_v)

		const float texture_coordinates[] =
		{
			LIST_EYE_CANDY_ATLAS(BUILD_EYE_CANDY_TEXTURE_COORDINATES)
			0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
		};

		const float* get_texture_coordinates(const Uint32 texture, const Uint32 index)
		{
			return &texture_coordinates[texture * 8 + index * 2];
		}

		Uint32 get_texture_index(const TextureEnum type)
		{
			switch (type)
			{
				case EC_CRYSTAL:
					return randint(3);
				case EC_FLARE:
					return randint(3) + 3;
				case EC_INVERSE:
					return randint(4) + 6;
				case EC_SHIMMER:
					return randint(3) + 10;
				case EC_SIMPLE:
					return 13;
				case EC_TWINFLARE:
					return randint(5) + 14;
				case EC_VOID:
					return randint(3) + 19;
				case EC_WATER:
					return randint(4) + 22;
				case EC_LEAF_ASH:
					return 26;
				case EC_LEAF_MAPLE:
					return 27;
				case EC_LEAF_OAK:
					return 28;
				case EC_PETAL:
					return 29;
				case EC_SNOWFLAKE:
					return 30;
			}

			return 31;
		}

		float get_texture_coordinate(const float burn)
		{
			return 0.25f + burn * 0.5f;
		}

	}
#endif	/* NEW_TEXTURES */

	const float MIN_SAFE_ALPHA = 0.02942f;

	// G L O B A L S //////////////////////////////////////////////////////////////

	MathCache math_cache;
	std::vector<Obstruction*> null_obstructions;
	bool ec_error_status = false;
	Logger logger;

	// C L A S S   F U N C T I O N S //////////////////////////////////////////////

#ifndef	NEW_TEXTURES
	Texture::Texture()
	{
	}

	Texture::~Texture()
	{
		clear();
	}

	GLuint Texture::get_texture(const Uint16 res_index) const
	{
		return (GLuint)get_texture(res_index, randint(texture_ids[res_index].size()));
	}

	GLuint Texture::get_texture(const Uint16 res_index, const int frame) const
	{
		return texture_ids[res_index][frame];
	}

	GLuint Texture::get_texture(const Uint16 res_index, const Uint64 born,
		const Uint64 changerate) const
	{
		return (GLuint)get_texture(res_index,
			((get_time() - born) / changerate) % texture_ids[res_index].size());
	}

	void Texture::clear()
	{
		for (int i = 0; i < 4; i++)
		{
			for (std::vector<GLuint>::iterator iter = texture_ids[i].begin(); iter
				!= texture_ids[i].end(); iter++)
			{
				const GLuint texture = *iter;
				glDeleteTextures(1, &texture);
			}
			texture_ids[i].clear();
		}
	}

	void Texture::push_texture(const std::string filename)
	{
		SDL_Surface* tex;
		GLuint texture_id;

		el_file_ptr file;

		file = el_open(filename.c_str());

		if (file == NULL)
			tex = NULL;
		else
		{
			tex = IMG_Load_RW(SDL_RWFromMem(el_get_pointer(file),
				el_get_size(file)), 1);
			el_close(file);
		}

		if (!tex)
		{
			logger.log_error("Cannot load texture '" + filename + "'.");
			return;
		}
		if (tex->format->palette)
		{
			logger.log_error("Cannot use paletted texture '" + filename + "'.");
			return;
		}
		if (tex->w != tex->h)
		{
			logger.log_error("Textures must be square; please fix '" + filename
				+ "'.");
			return;
		}
		if ((tex->w != 16) && (tex->w != 32) && (tex->w != 64) && (tex->w
			!= 128))
		{
			logger.log_error("Only 16x16, 32x32, 64x64, and 128x128 textures supportex; fix '"
				+ filename + "'.");
			return;
		}

		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, tex->format->BytesPerPixel, tex->w,
			tex->h, 0, (tex->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB),
			GL_UNSIGNED_BYTE, tex->pixels);

		if (tex->w == 16)
			texture_ids[0].push_back(texture_id);
		if (tex->w == 32)
			texture_ids[1].push_back(texture_id);
		if (tex->w == 64)
			texture_ids[2].push_back(texture_id);
		if (tex->w == 128)
			texture_ids[3].push_back(texture_id);

		SDL_FreeSurface(tex);
	}
#else	/* NEW_TEXTURES */
	void Effect::draw_particle(const coord_t size,
		const Uint32 texture, const color_t r, const color_t g,
		const color_t b, const alpha_t alpha, const Vec3 pos,
		const alpha_t burn)
	{
		Vec3 corner[4];
		Uint32 i, index;

		corner[0] = Vec3(pos - base->corner_offset1 * size);
		corner[1] = Vec3(pos + base->corner_offset2 * size);
		corner[2] = Vec3(pos + base->corner_offset1 * size);
		corner[3] = Vec3(pos - base->corner_offset2 * size);

		for (i = 0; i < 4; i++)
		{
			index = particle_count * 4 + i;

			buffer[index * 10 + 0] = r;
			buffer[index * 10 + 1] = g;
			buffer[index * 10 + 2] = b;
			buffer[index * 10 + 3] = alpha;
			buffer[index * 10 + 4] = corner[i].x;
			buffer[index * 10 + 5] = corner[i].y;
			buffer[index * 10 + 6] = corner[i].z;
			buffer[index * 10 + 7] = get_texture_coordinates(texture, i)[0];
			buffer[index * 10 + 8] = get_texture_coordinates(texture, i)[1];
			buffer[index * 10 + 9] = get_texture_coordinate(burn);
		}

		particle_count++;
	}

	void Effect::build_particle_buffer(const Uint64 time_diff)
	{
		std::map<Particle*, bool>::const_iterator iter;
		const Vec3 center(base->center);
		Uint32 size;

		particle_count = 0;

		particle_max_count = particles.size() * (1 + motion_blur_points);

		if (particle_max_count == 0)
		{
			return;
		}

		particle_max_count = (particle_max_count + 0xF) & 0xFFFFFFF0;
		size = particle_max_count * 40 * sizeof(float);

		if (particle_vertex_buffer.get_size() < size)
		{
			particle_vertex_buffer.bind(el::hbt_vertex);

			particle_vertex_buffer.set_size(el::hbt_vertex, size,
				el::hbut_dynamic_draw);
		}

		particle_vertex_buffer.bind(el::hbt_vertex);

		buffer = static_cast<float*>(particle_vertex_buffer.map(
			el::hbt_vertex, el::hbat_write_only));

		if (bounds)
		{
			for (iter = particles.begin(); iter != particles.end(); iter++)
			{
				Particle* p = iter->first;
				const coord_t dist_squared = (p->pos - center).magnitude_squared();
				if (dist_squared < MAX_DRAW_DISTANCE_SQUARED)
					p->draw(time_diff);
			}
		}
		else
		{
			for (iter = particles.begin(); iter != particles.end(); iter++)
			{
				Particle* p = iter->first;
				p->draw(time_diff);
			}
		}

		particle_vertex_buffer.unmap(el::hbt_vertex);
	}

	void Effect::draw_particle_buffer()
	{
		if (particle_count <= 0)
		{
			return;
		}

		particle_vertex_buffer.bind(el::hbt_vertex);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

		glColorPointer(4, GL_FLOAT, 10 * sizeof(float),
			static_cast<char*>(0) + 0 * sizeof(float));
		glVertexPointer(3, GL_FLOAT, 10 * sizeof(float),
			static_cast<char*>(0) + 4 * sizeof(float));

		ELglClientActiveTextureARB(GL_TEXTURE0);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 10 * sizeof(float),
			static_cast<char*>(0) + 7 * sizeof(float));

		ELglClientActiveTextureARB(GL_TEXTURE1);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(1, GL_FLOAT, 10 * sizeof(float),
			static_cast<char*>(0) + 9 * sizeof(float));

		glDrawArrays(GL_QUADS, 0, particle_count * 4);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		ELglClientActiveTextureARB(GL_TEXTURE1);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		ELglClientActiveTextureARB(GL_TEXTURE0);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
#endif	/* NEW_TEXTURES */

	Shape::~Shape()
	{
#ifndef	NEW_TEXTURES
		delete[] vertices;
		delete[] facets;
		delete[] normals;
#endif	/* NEW_TEXTURES */
	}

	void Shape::draw()
	{
		if (!base->draw_shapes)
			return;

		glColor4f(color.x, color.y, color.z, alpha);

		glPushMatrix();
		glTranslated(pos.x, pos.y, pos.z);
#ifdef	NEW_TEXTURES
		base->set_shape_texture_combiner(1.0f);

		vertex_buffer.bind(el::hbt_vertex);
		index_buffer.bind(el::hbt_index);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);

		ELglMultiTexCoord1f(GL_TEXTURE1, get_texture_coordinate(1.0f));
		glNormalPointer(GL_FLOAT, 3 * sizeof(float),
			static_cast<char*>(0) + vertex_count * 3 * sizeof(float));
		glVertexPointer(3, GL_FLOAT, 3 * sizeof(float),
			static_cast<char*>(0));

		glDrawElements(GL_TRIANGLES, facet_count * 3, GL_UNSIGNED_SHORT, 0);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);

		base->set_particle_texture_combiner();
#else	/* NEW_TEXTURES */
		glDisable(GL_TEXTURE_2D);

		if (base->poor_transparency_resolution)
#ifdef X86_64
			srand((Uint32)(Uint64)(void*)this);
#else
			srand((Uint32)(void*)this);
#endif
		glBegin(GL_TRIANGLES);
		{
			for (int i = 0; i < facet_count; i++)
			{
				if (base->poor_transparency_resolution && (alpha
					< MIN_SAFE_ALPHA)) // Ttlanhil recommends this number.
				{
					if (randfloat() < (alpha / MIN_SAFE_ALPHA))
						glColor4f(color.x, color.y, color.z, MIN_SAFE_ALPHA);
					else
						continue;
				}
				glNormal3f(normals[facets[i * 3] * 3],
					normals[facets[i * 3] * 3 + 1],
					normals[facets[i * 3] * 3 + 2]);
				glVertex3f(vertices[facets[i * 3] * 3],
					vertices[facets[i * 3] * 3 + 1],
					vertices[facets[i * 3] * 3 + 2]);
				glNormal3f(normals[facets[i * 3 + 1] * 3],
					normals[facets[i * 3 + 1] * 3 + 1],
					normals[facets[i * 3 + 1] * 3 + 2]);
				glVertex3f(vertices[facets[i * 3 + 1] * 3],
					vertices[facets[i * 3 + 1] * 3 + 1],
					vertices[facets[i * 3 + 1] * 3 + 2]);
				glNormal3f(normals[facets[i * 3 + 2] * 3],
					normals[facets[i * 3 + 2] * 3 + 1],
					normals[facets[i * 3 + 2] * 3 + 2]);
				glVertex3f(vertices[facets[i * 3 + 2] * 3],
					vertices[facets[i * 3 + 2] * 3 + 1],
					vertices[facets[i * 3 + 2] * 3 + 2]);
			}
		}
		glEnd();
		glEnable(GL_TEXTURE_2D);
#endif	/* NEW_TEXTURES */
		glPopMatrix();
	}

	CaplessCylinder::CaplessCylinder(EyeCandy* _base, const Vec3 _start,
		const Vec3 _end, const Vec3 _color, const alpha_t _alpha,
		const coord_t _radius, const int polys) :
		Shape(_base)
	{
#ifdef	NEW_TEXTURES
		float* vertices;
		float* normals;
		GLushort* facets;
		Uint32 size;
#endif	/* NEW_TEXTURES */
		radius = _radius;
		start = _start;
		end = _end;
		orig_start = _start;
		orig_end = _end;
		pos = (start + end) / 2;
		start -= pos;
		end -= pos;
		color = _color;
		alpha = _alpha;

		Vec3 normalized = start;
		normalized.normalize();

		const int subdivisions = ((polys - 1) / 2) + 1;
		vertex_count = subdivisions * 2;
#ifdef	NEW_TEXTURES
		if (vertex_count == 0)
		{
			return;
		}

		size = 6 * sizeof(float) * vertex_count;

		if (vertex_buffer.get_size() < size)
		{
			vertex_buffer.bind(el::hbt_vertex);

			vertex_buffer.set_size(el::hbt_vertex, size,
				el::hbut_dynamic_draw);
		}

		vertex_buffer.bind(el::hbt_vertex);
		vertices = static_cast<float*>(vertex_buffer.map(el::hbt_vertex,
			el::hbat_write_only));
		normals = &vertices[vertex_count * 3];
#else	/* NEW_TEXTURES */
		vertices = new coord_t[vertex_count * 3];
		normals = new coord_t[vertex_count * 3];
#endif	/* NEW_TEXTURES */

		// Get the coordinates.
		const angle_t radian_increment = 2 * PI / subdivisions;
		int i = 0;
		for (angle_t rad = 0; rad < 2 * PI - 0.0001; rad += radian_increment,
			i++)
		{
			vertices[i * 3] = end.x + radius * (cos(rad) * normalized.z
				+ sin(rad) * normalized.y);
			vertices[i * 3 + 1] = end.y + radius * (cos(rad) * normalized.x
				+ sin(rad) * normalized.z);
			vertices[i * 3 + 2] = end.z + radius * (cos(rad) * normalized.y
				+ sin(rad) * normalized.x);
			normals[i * 3] = cos(rad) * normalized.z + sin(rad) * normalized.y;
			normals[i * 3 + 1] = cos(rad) * normalized.x + sin(rad)
				* normalized.z;
			normals[i * 3 + 2] = cos(rad) * normalized.y + sin(rad)
				* normalized.x;
			vertices[i * 3 + subdivisions * 3] = start.x + radius * (cos(rad
				- radian_increment / 2) * normalized.z + sin(rad
				- radian_increment / 2) * normalized.y);
			vertices[i * 3 + 1 + subdivisions * 3] = start.y + radius
				* (cos(rad - radian_increment / 2) * normalized.x + sin(rad
					- radian_increment / 2) * normalized.z);
			vertices[i * 3 + 2 + subdivisions * 3] = start.z + radius
				* (cos(rad - radian_increment / 2) * normalized.y + sin(rad
					- radian_increment / 2) * normalized.x);
			normals[i * 3 + subdivisions * 3] = cos(rad - radian_increment / 2)
				* normalized.z + sin(rad - radian_increment / 2) * normalized.y;
			normals[i * 3 + subdivisions * 3 + 1] = cos(rad - radian_increment
				/ 2) * normalized.x + sin(rad - radian_increment / 2)
				* normalized.z;
			normals[i * 3 + subdivisions * 3 + 2] = cos(rad - radian_increment
				/ 2) * normalized.y + sin(rad - radian_increment / 2)
				* normalized.x;
		}

#ifdef	NEW_TEXTURES
		vertex_buffer.unmap(el::hbt_vertex);
		vertex_buffer.unbind(el::hbt_vertex);
#endif	/* NEW_TEXTURES */

		facet_count = subdivisions * 2;
#ifdef	NEW_TEXTURES
		size = 3 * sizeof(GLushort) * facet_count;

		if (index_buffer.get_size() < size)
		{
			index_buffer.bind(el::hbt_index);

			index_buffer.set_size(el::hbt_index, size,
				el::hbut_dynamic_draw);
		}

		index_buffer.bind(el::hbt_index);
		facets = static_cast<GLushort*>(index_buffer.map(el::hbt_index,
			el::hbat_write_only));
#else	/* NEW_TEXTURES */
		facets = new GLuint[facet_count * 3];
#endif	/* NEW_TEXTURES */

		// Add in the sides.
		for (int i = 0; i < subdivisions; i++)
		{
			facets[i * 6] = i;
			facets[i * 6 + 1] = i + subdivisions;
			facets[i * 6 + 2] = i + subdivisions + 1;
			facets[i * 6 + 3] = i + subdivisions + 1;
			facets[i * 6 + 4] = i + 1;
			facets[i * 6 + 5] = i;
		}

		// Wraparound.
		facets[(subdivisions - 1) * 6 + 2] = subdivisions;
		facets[(subdivisions - 1) * 6 + 3] = subdivisions;
		facets[(subdivisions - 1) * 6 + 4] = 0;
#ifdef	NEW_TEXTURES
		index_buffer.unmap(el::hbt_index);
		index_buffer.unbind(el::hbt_index);
#endif	/* NEW_TEXTURES */
	}

	Cylinder::Cylinder(EyeCandy* _base, const Vec3 _start, const Vec3 _end,
		const Vec3 _color, const alpha_t _alpha, const coord_t _radius,
		const int polys) :
		Shape(_base)
	{
#ifdef	NEW_TEXTURES
		float* vertices;
		float* normals;
		GLushort* facets;
		Uint32 size;
#endif	/* NEW_TEXTURES */
		radius = _radius;
		start = _start;
		end = _end;
		orig_start = _start;
		orig_end = _end;
		pos = (start + end) / 2;
		start -= pos;
		end -= pos;
		color = _color;
		alpha = _alpha;

		Vec3 normalized = start;
		normalized.normalize();

		const int subdivisions = ((polys - 1) / 4) + 1;
		vertex_count = subdivisions * 4 + 2; //+2 is for the centerpoints of the caps.
#ifdef	NEW_TEXTURES
		if (vertex_count == 0)
		{
			return;
		}

		size = 6 * sizeof(float) * vertex_count;

		if (vertex_buffer.get_size() < size)
		{
			vertex_buffer.bind(el::hbt_vertex);

			vertex_buffer.set_size(el::hbt_vertex, size,
				el::hbut_dynamic_draw);
		}

		vertex_buffer.bind(el::hbt_vertex);
		vertices = static_cast<float*>(vertex_buffer.map(el::hbt_vertex,
			el::hbat_write_only));
		normals = &vertices[vertex_count * 3];
#else	/* NEW_TEXTURES */
		vertices = new coord_t[vertex_count * 3];
		normals = new coord_t[vertex_count * 3];
#endif	/* NEW_TEXTURES */

		// Get the coordinates.
		const angle_t radian_increment = 2 * PI / subdivisions;
		int i = 0;
		for (angle_t rad = 0; rad < 2 * PI - 0.0001; rad += radian_increment,
			i++)
		{
			vertices[i * 3] = end.x + radius * (cos(rad) * normalized.z
				+ sin(rad) * normalized.y);
			vertices[i * 3 + 1] = end.y + radius * (cos(rad) * normalized.x
				+ sin(rad) * normalized.z);
			vertices[i * 3 + 2] = end.z + radius * (cos(rad) * normalized.y
				+ sin(rad) * normalized.x);
			normals[i * 3] = normalized.x;
			normals[i * 3 + 1] = normalized.y;
			normals[i * 3 + 2] = normalized.z;
			vertices[i * 3 + subdivisions * 3] = start.x + radius * (cos(rad
				- radian_increment / 2) * normalized.z + sin(rad
				- radian_increment / 2) * normalized.y);
			vertices[i * 3 + 1 + subdivisions * 3] = start.y + radius
				* (cos(rad - radian_increment / 2) * normalized.x + sin(rad
					- radian_increment / 2) * normalized.z);
			vertices[i * 3 + 2 + subdivisions * 3] = start.z + radius
				* (cos(rad - radian_increment / 2) * normalized.y + sin(rad
					- radian_increment / 2) * normalized.x);
			normals[i * 3 + subdivisions * 3] = -normalized.x;
			normals[i * 3 + 1 + subdivisions * 3] = -normalized.y;
			normals[i * 3 + 2 + subdivisions * 3] = -normalized.z;
			vertices[i * 3 + subdivisions * 6] = vertices[i * 3];
			vertices[i * 3 + subdivisions * 6 + 1] = vertices[i * 3 + 1];
			vertices[i * 3 + subdivisions * 6 + 2] = vertices[i * 3 + 2];
			normals[i * 3 + subdivisions * 6] = cos(rad) * normalized.z
				+ sin(rad) * normalized.y;
			normals[i * 3 + subdivisions * 6 + 1] = cos(rad) * normalized.x
				+ sin(rad) * normalized.z;
			normals[i * 3 + subdivisions * 6 + 2] = cos(rad) * normalized.y
				+ sin(rad) * normalized.x;
			vertices[i * 3 + subdivisions * 9]
				= vertices[i * 3 + subdivisions * 3];
			vertices[i * 3 + subdivisions * 9 + 1]
				= vertices[i * 3 + subdivisions * 3 + 1];
			vertices[i * 3 + subdivisions * 9 + 2]
				= vertices[i * 3 + subdivisions * 3 + 2];
			normals[i * 3 + subdivisions * 9] = cos(rad - radian_increment / 2)
				* normalized.z + sin(rad - radian_increment / 2) * normalized.y;
			normals[i * 3 + subdivisions * 9 + 1] = cos(rad - radian_increment
				/ 2) * normalized.x + sin(rad - radian_increment / 2)
				* normalized.z;
			normals[i * 3 + subdivisions * 9 + 2] = cos(rad - radian_increment
				/ 2) * normalized.y + sin(rad - radian_increment / 2)
				* normalized.x;
		}

		// Don't forget the centerpoints of the caps.
		vertices[subdivisions * 12] = end.x;
		vertices[subdivisions * 12 + 1] = end.y;
		vertices[subdivisions * 12 + 2] = end.z;
		normals[subdivisions * 12] = normalized.x;
		normals[subdivisions * 12 + 1] = normalized.y;
		normals[subdivisions * 12 + 2] = normalized.z;
		vertices[subdivisions * 12 + 3] = start.x;
		vertices[subdivisions * 12 + 4] = start.y;
		vertices[subdivisions * 12 + 5] = start.z;
		normals[subdivisions * 12 + 3] = -normalized.x;
		normals[subdivisions * 12 + 4] = -normalized.y;
		normals[subdivisions * 12 + 5] = -normalized.z;

#ifdef	NEW_TEXTURES
		vertex_buffer.unmap(el::hbt_vertex);
		vertex_buffer.unbind(el::hbt_vertex);
#endif	/* NEW_TEXTURES */

		facet_count = subdivisions * 4;
#ifdef	NEW_TEXTURES
		size = 3 * sizeof(GLushort) * facet_count;

		if (index_buffer.get_size() < size)
		{
			index_buffer.bind(el::hbt_index);

			index_buffer.set_size(el::hbt_index, size,
				el::hbut_dynamic_draw);
		}

		index_buffer.bind(el::hbt_index);
		facets = static_cast<GLushort*>(index_buffer.map(el::hbt_index,
			el::hbat_write_only));
#else	/* NEW_TEXTURES */
		facets = new GLuint[facet_count * 3];
#endif	/* NEW_TEXTURES */

		// First, add in the caps.
		for (int i = 0; i < subdivisions; i++)
		{
			facets[i * 3] = subdivisions * 4;
			facets[i * 3 + 1] = i;
			facets[i * 3 + 2] = i + 1;
			facets[i * 3 + subdivisions * 3] = subdivisions * 4 + 1;
			facets[i * 3 + 1 + subdivisions * 3] = i + subdivisions;
			facets[i * 3 + 2 + subdivisions * 3] = i + 1 + subdivisions;
		}
		// Wraparound.
		facets[(subdivisions - 1) * 3 + 2] = 0;
		facets[(subdivisions - 1) * 3 + 2 + subdivisions * 3] = subdivisions;

		// Now, add in the sides.
		for (int i = 0; i < subdivisions; i++)
		{
			facets[subdivisions * 3 * 2 + i * 6] = i + subdivisions * 2;
			facets[subdivisions * 3 * 2 + i * 6 + 1] = i + subdivisions * 3;
			facets[subdivisions * 3 * 2 + i * 6 + 2] = i + subdivisions * 3 + 1;
			facets[subdivisions * 3 * 2 + i * 6 + 3] = i + subdivisions * 3 + 1;
			facets[subdivisions * 3 * 2 + i * 6 + 4] = i + subdivisions * 2 + 1;
			facets[subdivisions * 3 * 2 + i * 6 + 5] = i + subdivisions * 2;
		}

		// Wraparound.
		facets[subdivisions * 3 * 2 + (subdivisions - 1) * 6 + 2] = subdivisions * 3;
		facets[subdivisions * 3 * 2 + (subdivisions - 1) * 6 + 3] = subdivisions * 3;
		facets[subdivisions * 3 * 2 + (subdivisions - 1) * 6 + 4] = subdivisions * 2;
#ifdef	NEW_TEXTURES
		index_buffer.unmap(el::hbt_index);
		index_buffer.unbind(el::hbt_index);
#endif	/* NEW_TEXTURES */
	}

	Sphere::Sphere(EyeCandy* _base, const Vec3 _pos, const Vec3 _color,
		const alpha_t _alpha, const coord_t _radius, const int polys) :
		Shape(_base)
	{
#ifdef	NEW_TEXTURES
		float* vertices;
		float* normals;
		GLushort* facets;
		Uint32 size;
#endif	/* NEW_TEXTURES */
		radius = _radius;
		pos = _pos;
		color = _color;
		alpha = _alpha;

		// Start with an octahedron.
		typedef std::pair<angle_t, angle_t> SphericalCoord;
		std::vector<SphericalCoord> spherical_vertices;

		spherical_vertices.push_back(SphericalCoord(0.0, 0.0));
		spherical_vertices.push_back(SphericalCoord(0.0, PI * 0.5));
		spherical_vertices.push_back(SphericalCoord(PI * 0.5, PI * 0.5));
		spherical_vertices.push_back(SphericalCoord(PI, PI * 0.5));
		spherical_vertices.push_back(SphericalCoord(PI * 1.5, PI * 0.5));
		spherical_vertices.push_back(SphericalCoord(0.0, PI));

		std::vector<Facet> spherical_facets;
		spherical_facets.push_back(Facet(0, 1, 2));
		spherical_facets.push_back(Facet(0, 2, 3));
		spherical_facets.push_back(Facet(0, 3, 4));
		spherical_facets.push_back(Facet(0, 4, 1));
		spherical_facets.push_back(Facet(5, 1, 4));
		spherical_facets.push_back(Facet(5, 2, 1));
		spherical_facets.push_back(Facet(5, 3, 2));
		spherical_facets.push_back(Facet(5, 4, 3));

		// Tesselate it until we've reached our subdivision limit.
		while ((int)spherical_facets.size() < polys)
		{
			std::vector<Facet> spherical_facets2;
			for (std::vector<Facet>::iterator iter = spherical_facets.begin(); iter
				!= spherical_facets.end(); iter++)
			{
				const int p1_index = iter->f[0];
				const int p2_index = iter->f[1];
				const int p3_index = iter->f[2];
				const SphericalCoord p1(spherical_vertices[p1_index]);
				const SphericalCoord p2(spherical_vertices[p2_index]);
				const SphericalCoord p3(spherical_vertices[p3_index]);
				coord_t p4, q4;
				average_points(p1.first, p2.first, p1.second, p2.second, p4, q4);
				const int p4_index = (int)spherical_vertices.size();
				spherical_vertices.push_back(SphericalCoord(p4, q4));
				coord_t p5, q5;
				average_points(p2.first, p3.first, p2.second, p3.second, p5, q5);
				const int p5_index = (int)spherical_vertices.size();
				spherical_vertices.push_back(SphericalCoord(p5, q5));
				coord_t p6, q6;
				average_points(p3.first, p1.first, p3.second, p1.second, p6, q6);
				const int p6_index = (int)spherical_vertices.size();
				spherical_vertices.push_back(SphericalCoord(p6, q6));
				spherical_facets2.push_back(Facet(p1_index, p4_index, p6_index));
				spherical_facets2.push_back(Facet(p2_index, p5_index, p4_index));
				spherical_facets2.push_back(Facet(p3_index, p6_index, p5_index));
				spherical_facets2.push_back(Facet(p4_index, p5_index, p6_index));
			}
			spherical_facets = spherical_facets2;
		}

		// Convert spherical to rectangular.
		vertex_count = (int)spherical_vertices.size();
#ifdef	NEW_TEXTURES
		if (vertex_count == 0)
		{
			return;
		}

		size = 6 * sizeof(float) * vertex_count;

		if (vertex_buffer.get_size() < size)
		{
			vertex_buffer.bind(el::hbt_vertex);

			vertex_buffer.set_size(el::hbt_vertex, size,
				el::hbut_dynamic_draw);
		}

		vertex_buffer.bind(el::hbt_vertex);
		vertices = static_cast<float*>(vertex_buffer.map(el::hbt_vertex,
			el::hbat_write_only));
		normals = &vertices[vertex_count * 3];
#else	/* NEW_TEXTURES */
		vertices = new coord_t[vertex_count * 3];
		normals = new coord_t[vertex_count * 3];
#endif	/* NEW_TEXTURES */

		for (int i = 0; i < vertex_count; i++)
		{
			const coord_t p = spherical_vertices[i].first;
			const coord_t q = spherical_vertices[i].second;
			normals[i * 3] = sin(p) * sin(q);
			normals[i * 3 + 1] = cos(q);
			normals[i * 3 + 2] = cos(p) * sin(q);
			vertices[i * 3] = normals[i * 3] * radius;
			vertices[i * 3 + 1] = normals[i * 3 + 1] * radius;
			vertices[i * 3 + 2] = normals[i * 3 + 2] * radius;
		}
#ifdef	NEW_TEXTURES
		vertex_buffer.unmap(el::hbt_vertex);
		vertex_buffer.unbind(el::hbt_vertex);
#endif	/* NEW_TEXTURES */

		// Convert facets to OpenGL-suitable array.
		facet_count = (int)spherical_facets.size();
#ifdef	NEW_TEXTURES
		size = 3 * sizeof(GLushort) * facet_count;

		if (index_buffer.get_size() < size)
		{
			index_buffer.bind(el::hbt_index);

			index_buffer.set_size(el::hbt_index, size,
				el::hbut_dynamic_draw);
		}

		index_buffer.bind(el::hbt_index);
		facets = static_cast<GLushort*>(index_buffer.map(el::hbt_index,
			el::hbat_write_only));
#else	/* NEW_TEXTURES */
		facets = new GLuint[facet_count * 3];
#endif	/* NEW_TEXTURES */
		for (int i = 0; i < facet_count; i++)
		{
			facets[i * 3] = spherical_facets[i].f[0];
			facets[i * 3 + 1] = spherical_facets[i].f[1];
			facets[i * 3 + 2] = spherical_facets[i].f[2];
		}
#ifdef	NEW_TEXTURES
		index_buffer.unmap(el::hbt_index);
		index_buffer.unbind(el::hbt_index);
#endif	/* NEW_TEXTURES */
	}

	void Sphere::average_points(const coord_t p1_first, const coord_t p2_first,
		const coord_t p1_second, const coord_t p2_second, coord_t& p, coord_t& q)
	{
		if ((fabs(p2_second) < 0.00001) || (fabs(p2_second - PI) < 0.00001))
			p = p1_first;
		else if ((fabs(p1_second) < 0.00001)
			|| (fabs(p1_second - PI) < 0.00001))
			p = p2_first;
		else if (fabs(p1_first - p2_first) > PI)
		{
			p = (p1_first + p2_first - 2 * PI) / 2;
			if (p < 0)
				p += 2 * PI;
		}
		else
			p = (p1_first + p2_first) / 2;

		q = (p1_second + p2_second) / 2;
	}

#ifdef	NEW_TEXTURES
	CaplessCylinders::CaplessCylinders(EyeCandy* _base,
		const std::vector<CaplessCylinderItem> &items)
	{
		CaplessCylindersVertex* vertices;
		GLushort* facets;
		Vec3 start, end, pos, color;
		float radius, alpha;
		Uint32 idx, count, face_index, vertex_index, size;

		base = _base;

		count = items.size();

		vertex_count = 0;
		facet_count = 0;

		for (idx = 0; idx < count; idx++)
		{
			const int subdivisions = ((items[idx].polys - 1) / 2) + 1;
			vertex_count += subdivisions * 2;
			facet_count += subdivisions * 2;
		}

		if ((vertex_count == 0) || (facet_count == 0))
		{
			return;
		}

		size = sizeof(CaplessCylindersVertex) * vertex_count;

		if (vertex_buffer.get_size() < size)
		{
			vertex_buffer.bind(el::hbt_vertex);

			vertex_buffer.set_size(el::hbt_vertex, size,
				el::hbut_dynamic_draw);
		}

		vertex_buffer.bind(el::hbt_vertex);
		vertices = static_cast<CaplessCylindersVertex*>(vertex_buffer.map(el::hbt_vertex, el::hbat_write_only));

		size = 3 * sizeof(GLushort) * facet_count;

		if (index_buffer.get_size() < size)
		{
			index_buffer.bind(el::hbt_index);

			index_buffer.set_size(el::hbt_index, size,
				el::hbut_dynamic_draw);
		}

		index_buffer.bind(el::hbt_index);
		facets = static_cast<GLushort*>(index_buffer.map(el::hbt_index,
			el::hbat_write_only));

		face_index = 0;
		vertex_index = 0;

		for (idx = 0; idx < count; idx++)
		{
			radius = items[idx].radius;
			start = items[idx].start;
			end = items[idx].end;
			pos = (start + end) / 2;
			color = items[idx].color;
			alpha = items[idx].alpha;

			Vec3 normalized = start - pos;
			normalized.normalize();

			const int subdivisions = ((items[idx].polys - 1) / 2) + 1;
			vertex_count = subdivisions * 2;

			// Get the coordinates.
			const angle_t radian_increment = 2 * PI / subdivisions;
			int i = 0;
			for (angle_t rad = 0; rad < 2 * PI - 0.0001;
				rad += radian_increment, i++)
			{
				vertices[vertex_index + i].r = static_cast<GLubyte>(color.x * 255.0f + 0.5f);
				vertices[vertex_index + i].g = static_cast<GLubyte>(color.y * 255.0f + 0.5f);
				vertices[vertex_index + i].b = static_cast<GLubyte>(color.z * 255.0f + 0.5f);
				vertices[vertex_index + i].a = static_cast<GLubyte>(alpha * 255.0f + 0.5f);

				vertices[vertex_index + i].x = end.x + radius * (cos(rad) * normalized.z
					+ sin(rad) * normalized.y);
				vertices[vertex_index + i].y = end.y + radius * (cos(rad) * normalized.x
					+ sin(rad) * normalized.z);
				vertices[vertex_index + i].z = end.z + radius * (cos(rad) * normalized.y
					+ sin(rad) * normalized.x);
				vertices[vertex_index + i].nx = cos(rad) * normalized.z
					+ sin(rad) * normalized.y;
				vertices[vertex_index + i].ny = cos(rad) * normalized.x
					+ sin(rad) * normalized.z;
				vertices[vertex_index + i].nz = cos(rad) * normalized.y
					+ sin(rad) * normalized.x;

				vertices[vertex_index + i + subdivisions].r = static_cast<GLubyte>(color.x * 255.0f + 0.5f);
				vertices[vertex_index + i + subdivisions].g = static_cast<GLubyte>(color.y * 255.0f + 0.5f);
				vertices[vertex_index + i + subdivisions].b = static_cast<GLubyte>(color.z * 255.0f + 0.5f);
				vertices[vertex_index + i + subdivisions].a = static_cast<GLubyte>(alpha * 255.0f + 0.5f);

				vertices[vertex_index + i + subdivisions].x = start.x + radius * (cos(rad
					- radian_increment / 2) * normalized.z + sin(rad
					- radian_increment / 2) * normalized.y);
				vertices[vertex_index + i + subdivisions].y = start.y + radius
					* (cos(rad - radian_increment / 2) * normalized.x + sin(rad
						- radian_increment / 2) * normalized.z);
				vertices[vertex_index + i + subdivisions].z = start.z + radius
					* (cos(rad - radian_increment / 2) * normalized.y + sin(rad
						- radian_increment / 2) * normalized.x);
				vertices[vertex_index + i + subdivisions].nx = cos(rad - radian_increment / 2)
					* normalized.z + sin(rad - radian_increment / 2) * normalized.y;
				vertices[vertex_index + i + subdivisions].ny = cos(rad - radian_increment
					/ 2) * normalized.x + sin(rad - radian_increment / 2)
					* normalized.z;
				vertices[vertex_index + i + subdivisions].nz = cos(rad - radian_increment
					/ 2) * normalized.y + sin(rad - radian_increment / 2)
					* normalized.x;
			}

			// Add in the sides.
			for (int i = 0; i < subdivisions; i++)
			{
				facets[(face_index + i) * 6] = vertex_index + i;
				facets[(face_index + i) * 6 + 1] = vertex_index + i + subdivisions;
				facets[(face_index + i) * 6 + 2] = vertex_index + i + subdivisions + 1;
				facets[(face_index + i) * 6 + 3] = vertex_index + i + subdivisions + 1;
				facets[(face_index + i) * 6 + 4] = vertex_index + i + 1;
				facets[(face_index + i) * 6 + 5] = vertex_index + i;
			}

			// Wraparound.
			facets[(face_index + (subdivisions - 1)) * 6 + 2] = vertex_index + subdivisions;
			facets[(face_index + (subdivisions - 1)) * 6 + 3] = vertex_index + subdivisions;
			facets[(face_index + (subdivisions - 1)) * 6 + 4] = vertex_index + 0;

			vertex_index += subdivisions * 2;
			face_index += subdivisions;
		}

		vertex_buffer.unmap(el::hbt_vertex);
		vertex_buffer.unbind(el::hbt_vertex);

		index_buffer.unmap(el::hbt_index);
		index_buffer.unbind(el::hbt_index);
	}

	void CaplessCylinders::draw(const float alpha_scale)
	{
		if (!base->draw_shapes)
			return;

		base->set_shape_texture_combiner(alpha_scale);

		vertex_buffer.bind(el::hbt_vertex);
		index_buffer.bind(el::hbt_index);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

		ELglMultiTexCoord1f(GL_TEXTURE1, get_texture_coordinate(1.0f));
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(CaplessCylindersVertex),
			static_cast<char*>(0) + 6 * sizeof(float));
		glNormalPointer(GL_FLOAT, sizeof(CaplessCylindersVertex),
			static_cast<char*>(0) + 3 * sizeof(float));
		glVertexPointer(3, GL_FLOAT, sizeof(CaplessCylindersVertex),
			static_cast<char*>(0));

		glDrawElements(GL_TRIANGLES, facet_count * 3, GL_UNSIGNED_SHORT, 0);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);

		base->set_particle_texture_combiner();
	}
#endif	/* NEW_TEXTURES */

	Obstruction::Obstruction(const coord_t _max_distance, const coord_t _force)
	{
		max_distance = _max_distance;
		max_distance_squared = square(max_distance);
		force = _force;
	}

	Vec3 SimpleCylinderObstruction::get_force_gradient(Particle& p)
	{ //Vertical cylinder, infinite height.
		const Vec3 translated_pos = p.pos - *(pos);

		const coord_t distsquared= square(translated_pos.x) + square(translated_pos.z);
		if (distsquared < max_distance_squared)
		{
			p.pos -= p.velocity * (p.base->time_diff / 1000000.0) * 0.5;
			return translated_pos * (force / (distsquared + 0.0001));
		}
		else
			return Vec3(0.0, 0.0, 0.0);
	}

	Vec3 CappedSimpleCylinderObstruction::get_force_gradient(Particle& p)
	{ //Vertical cylinder, infinite height.
		const Vec3 translated_pos = p.pos - *(pos);

		if ((p.pos.y < bottom) || (p.pos.y > top))
			return Vec3(0.0, 0.0, 0.0);

		const coord_t distsquared= square(translated_pos.x) + square(translated_pos.z);
		if (distsquared < max_distance_squared)
		{
			p.pos -= p.velocity * (p.base->time_diff / 1000000.0) * 0.5;
			return translated_pos * (force / (distsquared + 0.0001));
		}
		else
			return Vec3(0.0, 0.0, 0.0);
	}

	CylinderObstruction::CylinderObstruction(Vec3* _start, Vec3* _end,
		const coord_t _max_distance, const coord_t _force) :
		Obstruction(_max_distance, _force)
	{
		start = _start;
		end = _end;
		length_vec = *end - *start; // Assume that this doesn't change.
		length_vec_mag = length_vec.magnitude();
	}
	;

	Vec3 CylinderObstruction::get_force_gradient(Particle& p)
	{
		Vec3 v_offset;
		const Vec3 v1 = p.pos - *start;
		angle_t dotprod1;
		if ((dotprod1 = length_vec.dot(v1)) <= 0)
			v_offset = v1;
		else if (length_vec_mag <= dotprod1)
			v_offset = p.pos - *end;
		else
		{
			const coord_t scalar = dotprod1 / length_vec_mag;
			v_offset = *start + (length_vec * scalar);
		}
		const coord_t distsquared = v_offset.magnitude_squared();

		if (distsquared < max_distance_squared)
		{
			p.pos -= p.velocity * (p.base->time_diff / 1000000.0) * 0.5;
			return v_offset * (force / (distsquared + 0.0001));
		}
		else
			return Vec3(0.0, 0.0, 0.0);
	}

	Vec3 SphereObstruction::get_force_gradient(Particle& p)
	{
		const Vec3 translated_pos = p.pos - *(pos);

		const coord_t distsquared= square(translated_pos.x) + square(translated_pos.y) + square(translated_pos.z);
		if (distsquared < square(max_distance))
		{
			p.pos -= p.velocity * (p.base->time_diff / 1000000.0) * 0.5;
			return translated_pos * (force / (distsquared + 0.0001));
		}
		else
			return Vec3(0.0, 0.0, 0.0);
	}

	Vec3 BoxObstruction::get_force_gradient(Particle& p)
	{ // Arbitrary-rotation box.
		const Vec3 translated_pos = p.pos - *center;

		// Is it anywhere close?
		const coord_t distsquared = translated_pos.planar_magnitude_squared();
		if (distsquared >= max_distance_squared)
			return Vec3(0.0, 0.0, 0.0); // Nope.

		// So, it's close.  Is it actually bounded?
		const float s_rx = *sin_rot_x;
		const float c_rx = *cos_rot_x;
		const float s_ry = *sin_rot_y;
		const float c_ry = *cos_rot_y;
		const float s_rz = *sin_rot_z;
		const float c_rz = *cos_rot_z;

		Vec3 roty_position;
		roty_position.x = translated_pos.z * s_ry + translated_pos.x * c_ry;
		roty_position.y = translated_pos.y;
		roty_position.z = translated_pos.z * c_ry - translated_pos.x * s_ry;

		Vec3 rotx_position;
		rotx_position.x = roty_position.x;
		rotx_position.y = roty_position.y * c_rx - roty_position.z * s_rx;
		rotx_position.z = roty_position.y * s_rx + roty_position.z * c_rx;

		Vec3 rotz_position;
		rotz_position.x = rotx_position.x * c_rz - rotx_position.y * s_rz;
		rotz_position.y = rotx_position.x * s_rz + rotx_position.y * c_rz;
		rotz_position.z = rotx_position.z;

		//  std::cout << position << " | " << translated_pos << " | " << rotz_position << std::endl;

		if ((rotz_position.x < start.x) || (rotz_position.y < start.y)
			|| (rotz_position.z < start.z) || (rotz_position.x > end.x)
			|| (rotz_position.y > end.y) || (rotz_position.z > end.z))
		{
			//    if ((start - end).magnitude_squared() > 60.0)
			//    {
			//      if (translated_pos.magnitude_squared() < 150.0)
			//    if ((center->x > 41.74) && (center->x < 41.75))
			//        std::cout << "B1: " << p.pos << ", " << translated_pos << ", " << rotz_position << ": " << start << ", " << end << ": " << Vec3(0.0, 0.0, 0.0) << std::endl;
			//    }
			/*
			 glColor4f(0.0, 1.0, 0.0, 1.0);
			 glBegin(GL_TRIANGLES);
			 glVertex3f(center->x, center->y, center->z);
			 glVertex3f(center->x, 0, center->z);
			 glVertex3f(p.pos.x, p.pos.y, p.pos.z);
			 glEnd();

			 glColor4f(0.0, 0.0, 1.0, 0.2);
			 glBegin(GL_QUADS);
			 glVertex3f(start.x, start.y, start.z);
			 glVertex3f(end.x, start.y, start.z);
			 glVertex3f(end.x, end.y, start.z);
			 glVertex3f(start.x, end.y, start.z);
			 glVertex3f(start.x, start.y, start.z);
			 glVertex3f(end.x, start.y, start.z);
			 glVertex3f(end.x, start.y, end.z);
			 glVertex3f(start.x, start.y, end.z);
			 glVertex3f(start.x, start.y, end.z);
			 glVertex3f(start.x, end.y, end.z);
			 glVertex3f(start.x, end.y, start.z);
			 glVertex3f(start.x, start.y, start.z);
			 glVertex3f(start.x, start.y, end.z);
			 glVertex3f(end.x, start.y, end.z);
			 glVertex3f(end.x, end.y, end.z);
			 glVertex3f(start.x, end.y, end.z);
			 glVertex3f(start.x, end.y, start.z);
			 glVertex3f(end.x, end.y, start.z);
			 glVertex3f(end.x, end.y, end.z);
			 glVertex3f(start.x, end.y, end.z);
			 glVertex3f(end.x, start.y, end.z);
			 glVertex3f(end.x, end.y, end.z);
			 glVertex3f(end.x, end.y, start.z);
			 glVertex3f(end.x, start.y, start.z);
			 glEnd();
			 
			 if (distsquared < 0.1)
			 std::cout << p.pos << ", " << translated_pos << ", " << rotz_position << " : " << *center << " / " << start << ", " << end << std::endl;
			 */
			return Vec3(0.0, 0.0, 0.0);
		}
		else
		{
			p.pos -= p.velocity * (p.base->time_diff / 1000000.0) * 0.5;

			Vec3 ret;
			ret.x = 3 * force * (rotz_position.x - midpoint.x) / size.x;
			ret.y = 3 * force * (rotz_position.y - start.y) / square(size.y + 0.7);
			ret.z = 3 * force * (rotz_position.z - midpoint.z) / size.z;

			const float s_rx2 = *sin_rot_x2;
			const float c_rx2 = *cos_rot_x2;
			const float s_ry2 = *sin_rot_y2;
			const float c_ry2 = *cos_rot_y2;
			const float s_rz2 = *sin_rot_z2;
			const float c_rz2 = *cos_rot_z2;

			Vec3 rotz_ret;
			rotz_ret.x = ret.x * c_rz2 - ret.y * s_rz2;
			rotz_ret.y = ret.x * s_rz2 + ret.y * c_rz2;
			rotz_ret.z = ret.z;

			Vec3 rotx_ret;
			rotx_ret.x = rotz_ret.x;
			rotx_ret.y = rotz_ret.y * c_rx2 - rotz_ret.z * s_rx2;
			rotx_ret.z = rotz_ret.y * s_rx2 + rotz_ret.z * c_rx2;

			Vec3 roty_ret;
			roty_ret.x = rotx_ret.z * s_ry2 + rotx_ret.x * c_ry2;
			roty_ret.y = rotx_ret.y;
			roty_ret.z = rotx_ret.z * c_ry2 - rotx_ret.x * s_ry2;

			//    if ((center->x > 41.74) && (center->x < 41.75))
			//      std::cout << "B2: " << p.pos << ", " << translated_pos << ", " << rotz_position << ": " << (*center) << ": " << start << ", " << end << ": " << ret << ", " << roty_ret << std::endl;
			/*
			 glColor4f(1.0, 0.0, 0.0, 1.0);
			 glBegin(GL_TRIANGLES);
			 glVertex3f(center->x, center->y, center->z);
			 glVertex3f(center->x, 0.2, center->z);
			 glVertex3f(center->x + roty_ret.x, center->y + roty_ret.y, center->z + roty_ret.z);
			 glEnd();
			 */
			return roty_ret;
		}
		// get rid of compiler warning, this code should never be reached
		return Vec3(0.0, 0.0, 0.0);
	}

	PolarCoordElement::PolarCoordElement(const coord_t _frequency,
		const coord_t _offset, const coord_t _scalar, const coord_t _power)
	{
		frequency = _frequency;
		offset = _offset;
		scalar = _scalar;
		power = _power;
	}

	coord_t PolarCoordElement::get_radius(const angle_t angle) const
	{
		const angle_t temp_cos = cos((angle + offset) * frequency);
		coord_t ret = std::pow(fabsf(temp_cos), power)
			* scalar + scalar;
		ret = copysign(ret, temp_cos);
		return ret;
	}

	Particle::Particle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos,
		const Vec3 _velocity, const coord_t _size)
	{
		effect = _effect;
		base = effect->base;
		cur_motion_blur_point = 0;
		motion_blur = new ParticleHistory[effect->motion_blur_points];
		for (int i = 0; i < effect->motion_blur_points; i++)
			motion_blur[i].alpha = 0;
		mover = _mover;
		pos = _pos;
		velocity = _velocity;
		energy = mover->calculate_energy(*this);
		born = get_time();
		mover->attachParticle(this);
		size = _size;
	}

	Particle::~Particle()
	{
		delete[] motion_blur;
		if (mover)
			mover->detachParticle(this);
	}
	;

#ifdef	NEW_TEXTURES
	void Particle::draw(const Uint64 usec)
	{
		alpha_t burn = get_burn();
		alpha_t tempalpha = alpha;

		coord_t tempsize = base->billboard_scalar * size;
		tempsize *= flare();

		assert(std::isfinite(burn));
		assert(std::isfinite(tempalpha));
		assert(std::isfinite(base->billboard_scalar));
		assert(std::isfinite(size));
		assert(std::isfinite(tempsize));

		Uint32 texture = get_texture(); // Always hires, since we're not checking distance.

		effect->draw_particle(tempsize, texture, color[0], color[1],
			color[2], tempalpha, pos, burn);

		if (effect->motion_blur_points > 0)
		{
			const alpha_t faderate = std::pow(
				effect->motion_blur_fade_rate, (float)usec / 1000000);

			for (int i = 0; i < effect->motion_blur_points; i++)
				effect->draw_particle(motion_blur[i].size,
					motion_blur[i].texture,
					motion_blur[i].color[0],
					motion_blur[i].color[1],
					motion_blur[i].color[2],
					motion_blur[i].alpha,
					motion_blur[i].pos, burn);

			motion_blur[cur_motion_blur_point] = ParticleHistory(
				tempsize, texture, color[0], color[1], color[2],
				alpha, pos);
			cur_motion_blur_point++;

			for (int i = 0; i < effect->motion_blur_points; i++)
				motion_blur[i].alpha *= faderate;

			if (cur_motion_blur_point == effect->motion_blur_points)
				cur_motion_blur_point = 0;
		}
	}
#else	/* NEW_TEXTURES */
	void Particle::draw(const Uint64 usec)
	{
		switch (base->draw_method)
		{
			case EyeCandy::POINT_SPRITES:
			{
				coord_t tempsize = base->temp_sprite_scalar * size / std::sqrt(square(pos.x - base->camera.x) + square(pos.y - base->camera.y) + square(pos.z - base->camera.z));
				tempsize *= flare();

				if (tempsize > base->max_allowable_point_size)
				{
					//Fall through.
				}
				else
				{
					alpha_t tempalpha = alpha;
					if (tempsize < 5.0) // Pseudo-antialiasing.  Makes the particles look nice.
					{
						tempalpha /= square(5.0 / tempsize);
						tempsize = 5.0;
					}
					else if (tempsize > base->max_point_size)
						tempsize = base->max_point_size;

					if (base->poor_transparency_resolution)
					{
						if (tempalpha < MIN_SAFE_ALPHA)
						{
							if (randfloat() < (tempalpha / MIN_SAFE_ALPHA))
								tempalpha = MIN_SAFE_ALPHA;
							else
								return;
						}
					}

					int res_index;
					if (tempsize <= 16)
						res_index = 0;
					else if (tempsize <= 32)
						res_index = 1;
					else if (tempsize <= 64)
						res_index = 2;
					else
						res_index = 3;
					const GLuint texture = get_texture(res_index);
					base->draw_point_sprite_particle(tempsize, texture,
						color[0], color[1], color[2], tempalpha, pos);
					if (effect->motion_blur_points > 0)
					{
						const alpha_t faderate =
							std::pow(
								effect->motion_blur_fade_rate, (float)usec
									/ 1000000);

						for (int i = 0; i < effect->motion_blur_points; i++)
							base->draw_point_sprite_particle(
								motion_blur[i].size, motion_blur[i].texture,
								motion_blur[i].color[0],
								motion_blur[i].color[1],
								motion_blur[i].color[2], motion_blur[i].alpha,
								motion_blur[i].pos);

						motion_blur[cur_motion_blur_point] = ParticleHistory(
							tempsize, texture, color[0], color[1], color[2],
							tempalpha, pos);
						cur_motion_blur_point++;

						for (int i = 0; i < effect->motion_blur_points; i++)
							motion_blur[i].alpha *= faderate;

						if (cur_motion_blur_point == effect->motion_blur_points)
							cur_motion_blur_point = 0;
					}
					break;
				}
			}
			default:
			{
				alpha_t tempalpha = alpha;
				if (base->poor_transparency_resolution)
				{
					if (tempalpha < MIN_SAFE_ALPHA)
					{
						if (randfloat() < (tempalpha / MIN_SAFE_ALPHA))
							tempalpha = MIN_SAFE_ALPHA;
						else
							return;
					}
				}

				coord_t tempsize = base->billboard_scalar * size;
				tempsize *= flare();

				const GLuint texture = get_texture(3); // Always hires, since we're not checking distance.
				//    std::cout << this << ": " << tempsize << ", " << size << ": " << pos << std::endl;
				if (base->draw_method != EyeCandy::ACCURATE_BILLBOARDS)
					base->draw_fast_billboard_particle(tempsize, texture,
						color[0], color[1], color[2], tempalpha, pos);
				else
					base->draw_accurate_billboard_particle(tempsize, texture,
						color[0], color[1], color[2], tempalpha, pos);
				if (effect->motion_blur_points > 0)
				{
					const alpha_t faderate = std::pow(
						effect->motion_blur_fade_rate, (float)usec / 1000000);

					if (base->draw_method != EyeCandy::ACCURATE_BILLBOARDS)
					{
						for (int i = 0; i < effect->motion_blur_points; i++)
							base->draw_fast_billboard_particle(
								motion_blur[i].size, motion_blur[i].texture,
								motion_blur[i].color[0],
								motion_blur[i].color[1],
								motion_blur[i].color[2], motion_blur[i].alpha,
								motion_blur[i].pos);
					}
					else
					{
						for (int i = 0; i < effect->motion_blur_points; i++)
							base->draw_accurate_billboard_particle(
								motion_blur[i].size, motion_blur[i].texture,
								motion_blur[i].color[0],
								motion_blur[i].color[1],
								motion_blur[i].color[2], motion_blur[i].alpha,
								motion_blur[i].pos);
					}

					motion_blur[cur_motion_blur_point] = ParticleHistory(
						tempsize, texture, color[0], color[1], color[2], alpha,
						pos);
					cur_motion_blur_point++;

					for (int i = 0; i < effect->motion_blur_points; i++)
						motion_blur[i].alpha *= faderate;

					if (cur_motion_blur_point == effect->motion_blur_points)
						cur_motion_blur_point = 0;
				}
			}
		}
	}
#endif	/* NEW_TEXTURES */

	coord_t Particle::flare() const
	{
		float exp_base, tmp;

		assert(flare_frequency);

		if (flare_max == 1.0)
		{
			return 1.0;
		}

#ifdef DEBUG_NANS
		//  std::cout << "Beginning test." << std::endl;
		if (!pos.is_valid())
		std::cout << "ERROR: Invalid particle " << this << ": pos=" << pos << "; velocity=" << velocity << "; effect=" << effect << std::endl;
#endif
		assert(pos.is_valid());

		const short offset = (short)long(&alpha); //Unique to the particle.

		tmp = offset;

		if (pos.is_valid())
		{
			tmp += pos.x + pos.y + pos.z;
		}

		exp_base = fabs(sin(tmp / flare_frequency));

		const coord_t exp = std::pow(exp_base, flare_exp);
		const coord_t flare_val = 1.0 / (exp + 0.00001);
		if (flare_val > flare_max)
			return flare_max;
		else
			return flare_val;
	}

	ParticleMover::ParticleMover(Effect* _effect)
	{
		effect = _effect;
		base = effect->base;
	}

	Vec3 ParticleMover::vec_shift(const Vec3 src, const Vec3 dest,
		const percent_t percent) const
	{
#if 0	// Slow but clear version.  Consider this a comment.
		const coord_t magnitude = src.magnitude();
		Vec3 ret = nonpreserving_vec_shift(src, dest, percent);
		ret.normalize(magnitude);
#else	// Fast but obfuscated
		const coord_t magnitude_squared = src.magnitude_squared();
		Vec3 ret = nonpreserving_vec_shift(src, dest, percent);
		ret /= std::sqrt(ret.magnitude_squared() / magnitude_squared);
#endif
		return ret;
	}

	Vec3 ParticleMover::vec_shift_amount(const Vec3 src, const Vec3 dest,
		const coord_t amount) const
	{
		const Vec3 diff = dest - src;
		const coord_t magnitude = diff.magnitude();
		if (magnitude > amount)
			return dest;
		const percent_t percent = magnitude / amount;
		return vec_shift(src, dest, percent);
	}

	Vec3 ParticleMover::nonpreserving_vec_shift(const Vec3 src,
		const Vec3 dest, const percent_t percent) const
	{
		return (dest - src) * percent + src * (1.0 - percent);
	}

	Vec3 ParticleMover::nonpreserving_vec_shift_amount(const Vec3 src,
		const Vec3 dest, const coord_t amount) const
	{
		const Vec3 diff = dest - src;
		const coord_t magnitude = diff.magnitude();
		if (magnitude > amount)
			return dest;
		const percent_t percent = magnitude / amount;
		return nonpreserving_vec_shift(src, dest, percent);
	}

	void GradientMover::move(Particle& p, Uint64 usec)
	{
		const coord_t scalar = usec / 1000000.0;
		Vec3 gradient_velocity = p.velocity + get_force_gradient(p) * scalar;
		if (gradient_velocity.magnitude_squared() > 10000.0)
			gradient_velocity.normalize(100.0);
		p.velocity = gradient_velocity + get_obstruction_gradient(p) * scalar;
#if 0	// Slow but clear version.  Consider this a comment.
		p.velocity.normalize(gradient_velocity.magnitude() + 0.000001);
#else	// Fast but obfuscated
		p.velocity /= std::sqrt(p.velocity.magnitude_squared() / (gradient_velocity.magnitude_squared() + 0.000001));
#endif
		p.pos += p.velocity * scalar;
	}

	Vec3 GradientMover::get_force_gradient(Particle& p) const
	{
		return Vec3(0.0, 0.0, 0.0);
	}

	Vec3 SmokeMover::get_force_gradient(Particle& p) const
	{
		return Vec3(0.0, 0.2 * strength, 0.0);
	}

	Vec3 SpiralMover::get_force_gradient(Particle& p) const
	{
		Vec3 shifted_pos = p.pos - *center;
		return Vec3(shifted_pos.z * spiral_speed - shifted_pos.x * pinch_rate,
			0.0, shifted_pos.x * spiral_speed - shifted_pos.z * pinch_rate);
	}

	coord_t PolarCoordsBoundingRange::get_radius(const angle_t angle) const
	{
		float radius = 0.0;
		for (std::vector<PolarCoordElement>::const_iterator iter =
			elements.begin(); iter != elements.end(); iter++)
			radius += iter->get_radius(angle);
		return radius;
	}

	coord_t SmoothPolygonBoundingRange::get_radius(const angle_t angle) const
	{
		const float angle2 = (angle < 0 ? angle + 2 * PI : angle);
		std::vector<SmoothPolygonElement>::const_iterator lower, upper;
		lower = elements.begin() + (elements.size() - 1);
		for (upper = elements.begin(); upper != elements.end(); upper++)
		{
			if (upper->angle >= angle2)
				break;
			lower = upper;
		}
		if (upper == elements.end())
			upper = elements.begin();
		float upper_percent;
		if (upper->angle > lower->angle)
			upper_percent = (angle2 - lower->angle) / (upper->angle
				- lower->angle);
		else if (angle2 > lower->angle)
			upper_percent = (angle2 - lower->angle) / (upper->angle + 2 * PI
				- lower->angle);
		else
			upper_percent = (angle2 + 2 * PI - lower->angle) / (upper->angle
				+ 2 * PI - lower->angle);
		return upper_percent * upper->radius + (1.0 - upper_percent)
			* lower->radius;
	}

	BoundingMover::BoundingMover(Effect* _effect, const Vec3 _center_pos,
		BoundingRange* _bounding_range, const coord_t _force) :
		GradientMover(_effect)
	{
		center_pos = _center_pos;
		bounding_range = _bounding_range;
		force = _force;
	}

	Vec3 BoundingMover::get_force_gradient(Particle& p) const
	{
		Vec3 shifted_pos = p.pos - center_pos;
		const coord_t radius= std::sqrt(square(shifted_pos.x) + square(shifted_pos.z));
		const angle_t angle = atan2(shifted_pos.x, shifted_pos.z);
		const coord_t max_radius = bounding_range->get_radius(angle);
		if (radius > max_radius)
		{
			const coord_t diff = (radius - max_radius) / radius;
			return -shifted_pos * diff;
		}
		else
			return Vec3(0.0, 0.0, 0.0);
	}

	Vec3 SimpleGravityMover::get_force_gradient(Particle& p) const
	{
		return Vec3(0.0, -1.6, 0.0);
	}

	Vec3 GradientMover::get_obstruction_gradient(Particle& p) const
	{ //Unlike normal force gradients, obstruction gradients are used in a magnitude-preserving fashion.
		Vec3 ret(0.0, 0.0, 0.0);
		for (std::vector<Obstruction*>::iterator iter =
			effect->obstructions->begin(); iter != effect->obstructions->end(); iter++)
			ret += (*iter)->get_force_gradient(p);
		return ret;
	}

	GravityMover::GravityMover(Effect* _effect, Vec3* _center) :
		GradientMover(_effect)
	{
		mass = 2e10;
		max_gravity = 20.0;
		gravity_center_ptr = _center;
		gravity_center = *gravity_center_ptr;
		old_gravity_center = gravity_center;
	}

	GravityMover::GravityMover(Effect* _effect, Vec3* _center,
		const energy_t _mass) :
		GradientMover(_effect)
	{
		mass = _mass;
		max_gravity = mass / 1e9;
		gravity_center_ptr = _center;
		gravity_center = *gravity_center_ptr;
		old_gravity_center = gravity_center;
	}

	void GravityMover::set_gravity_center(Vec3* _gravity_center)
	{
		gravity_center_ptr = _gravity_center;
	}

	void GravityMover::move(Particle& p, Uint64 usec)
	{
		old_gravity_center = gravity_center;
		gravity_center = *gravity_center_ptr;

		if (usec >= base->max_usec_per_particle_move)
			usec = base->max_usec_per_particle_move;

		const coord_t dist = gravity_dist(p, gravity_center);
		if (gravity_center != old_gravity_center)
		{
			const coord_t old_dist = gravity_dist(p, old_gravity_center);
			p.energy += G * mass * (dist - old_dist);
		}
		Vec3 gravity_vec(gravity_center.x - p.pos.x,
			gravity_center.y - p.pos.y, gravity_center.z - p.pos.z);

		// Simulated point gravity sources tend to promote extreme forces that, even if you fix the energy balance, will throw off angles.  Cancel them out.
		energy_t scalar = G * mass / square(dist) + 0.00001;

		scalar = std::min(scalar, max_gravity);
		gravity_vec.normalize(scalar);
		p.pos += p.velocity * ((coord_t)usec / 1000000.0);
		p.velocity += gravity_vec * ((coord_t)usec / 1000000.0);

		// Simulated point gravity sources tend to create/destroy energy when a particle passes near the center.  Cancel it out.
		const energy_t velocity_energy = calculate_velocity_energy(p);
		const energy_t new_energy = G * mass * dist + velocity_energy;
		const energy_t energy_difference = p.energy - new_energy;
		const energy_t new_velocity_energy = velocity_energy
			+ energy_difference;
		if (new_velocity_energy >= 0)
		{
#if 0	// Slow but clear.  Consider this a comment.
			const coord_t new_velocity = std::sqrt(2.0 * new_velocity_energy);
			if (new_velocity)
			p.velocity.normalize(new_velocity + 0.000001);
			else
			p.velocity = Vec3(0.0, 0.0, 0.0);
#else	// Fast but obfuscated
			const coord_t new_velocity_squared = 2.0 * new_velocity_energy;
			if (std::abs(new_velocity_squared) > 0.00001)
				p.velocity /= std::sqrt(p.velocity.magnitude_squared() / new_velocity_squared + 0.000001);
			else
				p.velocity = Vec3(0.0, 0.0, 1.0);
#endif
		}

		// Factor in the force gradient, if any.
		Vec3 gradient_velocity = p.velocity + get_force_gradient(p);
		Vec3 obstruction_velocity = gradient_velocity
			+ get_obstruction_gradient(p);
		const coord_t grad_mag_squared = gradient_velocity.magnitude_squared();
#if 0	// Slow but clear.  Consider this a comment.
		if (grad_mag_squared)
		obstruction_velocity.normalize(gradient_velocity.magnitude() + 0.00001);
		else
		obstruction_velocity = Vec3(0.0, 0.0, 0.0);
#else	// Fast but obfuscated.
		if (std::abs(grad_mag_squared) > 0.00001)
			obstruction_velocity /= std::sqrt(obstruction_velocity.magnitude_squared() / (grad_mag_squared + 0.00001) + 0.00001);
		else
			obstruction_velocity = Vec3(0.0, 0.0, 1.0);
#endif
		const coord_t obstruction_velocity_energy = 0.5
			* obstruction_velocity.magnitude_squared();
		p.energy += obstruction_velocity_energy - new_velocity_energy;
		p.velocity = obstruction_velocity;
	}

	energy_t GravityMover::calculate_velocity_energy(const Particle& p) const
	{
		return 0.5 * p.velocity.magnitude_squared();
	}

	energy_t GravityMover::calculate_position_energy(const Particle& p) const
	{
		return G * mass * gravity_dist(p, gravity_center);
	}

	coord_t GravityMover::gravity_dist(const Particle& p, const Vec3& center) const
	{
		return std::sqrt(square(p.pos.x - center.x) + square(p.pos.y - center.y) + square(p.pos.z - center.z));
	}

	energy_t GravityMover::calculate_energy(const Particle& p) const
	{
		return calculate_velocity_energy(p) + calculate_position_energy(p);
	}

	Vec3 IFSLinearElement::get_new_coords(const Vec3& pos)
	{
		return (pos * inv_scale) + (center * scale);
	}

	Vec3 IFSSinusoidalElement::get_new_coords(const Vec3& pos)
	{
		const Vec3 result(sin(pos.x * scalar.x + offset.x) * scalar2.x,
			sin(pos.y * scalar.y + offset.y) * scalar2.y, sin(pos.z * scalar.z
				+ offset.z) * scalar2.z);
		return (pos * inv_scale) + (result * scale);
	}

	Vec3 IFSSphericalElement::get_new_coords(const Vec3& pos)
	{
		const coord_t r = pos.magnitude();
		const Vec3 result((pos.x + numerator_adjust.x) / (r
			+ denominator_adjust.x), (pos.y + numerator_adjust.y) / (r
			+ denominator_adjust.y), (pos.z + numerator_adjust.z) / (r
			+ denominator_adjust.z));
		return (pos * inv_scale) + (result * scale);
	}

	Vec3 IFSRingElement::get_new_coords(const Vec3& pos)
	{
		const coord_t r= std::sqrt(square(pos.x) + square(pos.z));
		const Vec3 result((pos.x + numerator_adjust.x) / (r
			+ denominator_adjust.x), 0.0, (pos.z + numerator_adjust.z) / (r
			+ denominator_adjust.z));
		return (pos * inv_scale) + (result * scale);
	}

	Vec3 IFSSwirlElement::get_new_coords(const Vec3& pos)
	{
		const coord_t r = pos.magnitude();
		const angle_t theta_x = atan2(pos.z, pos.y);
		const angle_t theta_y = atan2(pos.x, pos.z);
		const angle_t theta_z = atan2(pos.y, pos.x);
		const Vec3 result(r * cos(theta_x + r), r * cos(theta_y + r), r
			* cos(theta_z + r));
		return (pos * inv_scale) + (result * scale);
	}

	Vec3 IFS2DSwirlElement::get_new_coords(const Vec3& pos)
	{
		const coord_t r= std::sqrt(square(pos.x) + square(pos.z));
		const angle_t theta_x = atan(pos.z);
		const angle_t theta_z = atan(pos.x);
		const Vec3 result(r * cos(theta_x + r), 0.0, r * cos(theta_z + r));
		return (pos * inv_scale) + (result * scale);
	}

	Vec3 IFSHorseshoeElement::get_new_coords(const Vec3& pos)
	{
		const coord_t r = pos.magnitude();
		const angle_t theta_x = atan2(pos.z, pos.y + 0.0000001f);
		const angle_t theta_y = atan2(pos.x, pos.z + 0.0000001f);
		const angle_t theta_z = atan2(pos.y, pos.x + 0.0000001f);
		const Vec3 result(r * cos(2 * theta_x), r * cos(2 * theta_y), r * cos(2 * theta_z));
		return (pos * inv_scale) + (result * scale);
	}

	Vec3 IFS2DHorseshoeElement::get_new_coords(const Vec3& pos)
	{
		const coord_t r = pos.magnitude();
		const angle_t theta_x = atan(pos.z);
		const angle_t theta_z = atan(pos.y);
		const Vec3 result(r * cos(2 * theta_x), 0.0, r * cos(2 * theta_z));
		return (pos * inv_scale) + (result * scale);
	}

	IFSParticleSpawner::IFSParticleSpawner(const int count, const coord_t size)
	{
		generate(count, Vec3(size, size, size));
		pos = Vec3(0.0, 0.0, 0.0);
	}

	IFSParticleSpawner::IFSParticleSpawner(const int count, const Vec3 scale)
	{
		generate(count, scale);
		pos = Vec3(0.0, 0.0, 0.0);
	}

	IFSParticleSpawner::~IFSParticleSpawner()
	{
		for (std::vector<IFSParticleElement*>::iterator iter =
			ifs_elements.begin(); iter != ifs_elements.end(); iter++)
			delete *iter;
	}

	void IFSParticleSpawner::generate(const int count, const Vec3 scale)
	{
		for (int i = 0; i < count; i++)
		{
			Vec3 v;
			v.randomize();
			v.x *= scale.x;
			v.y *= scale.y;
			v.z *= scale.z;
			ifs_elements.push_back(new IFSLinearElement(v, randcoord()));
		}
	}

	Vec3 IFSParticleSpawner::get_new_coords()
	{
		pos = ifs_elements[randint((int)ifs_elements.size())]->get_new_coords(pos);
		if (!pos.is_valid())
			pos = Vec3(0.0, 0.0, 0.0);
		return pos;
	}

	Vec3 FilledSphereSpawner::get_new_coords()
	{
		Vec3 ret;
		ret.randomize();
		ret *= randcoord_non_zero() * radius;
		return ret;
	}

	Vec3 FilledEllipsoidSpawner::get_new_coords()
	{
		Vec3 ret;
		ret.randomize();
		ret *= randcoord_non_zero();
		ret.x *= radius.x;
		ret.y *= radius.y;
		ret.z *= radius.z;

		return ret;
	}

	Vec3 HollowSphereSpawner::get_new_coords()
	{
		Vec3 ret;
		ret.randomize();
		ret *= radius;

		return ret;
	}

	Vec3 HollowEllipsoidSpawner::get_new_coords()
	{
		Vec3 ret;
		ret.randomize();
		ret.x *= radius.x;
		ret.y *= radius.y;
		ret.z *= radius.z;

		return ret;
	}

	Vec3 FilledDiscSpawner::get_new_coords()
	{
		const angle_t angle= randangle(2 * PI);
		const coord_t scale = (1.0 - square(randcoord())) * radius;
		const Vec3 ret(scale * sin(angle), 0.0, scale * cos(angle));
		return ret;
	}

	Vec3 HollowDiscSpawner::get_new_coords()
	{
		const angle_t angle= randangle(2 * PI);
		const Vec3 ret(radius * sin(angle), 0.0, radius * cos(angle));
		return ret;
	}

	Vec3 FilledBoundingSpawner::get_new_coords()
	{
		Vec3 cur_pos;
		const Vec3 camera_center_difference = *center - *base_center;
		//  int i;
		//  for (i = 0; i < 30; i++)
		{
			const angle_t angle= randangle(2 * PI);
			const coord_t scalar= randcoord() * MAX_DRAW_DISTANCE * range_scalar;
			cur_pos = Vec3(sin(angle) * scalar, 0.0, cos(angle) * scalar);
			const angle_t angle_to_center = atan2(cur_pos.x, cur_pos.z);
			const coord_t radius = bounding_range->get_radius(angle_to_center);
			//    std::cout << cur_pos << " - " << camera_center_difference << ".magnitude_squared() (" << (cur_pos - camera_center_difference).magnitude_squared() << ") < " << square(radius) << std::endl;
			//    if ((cur_pos - camera_center_difference).magnitude_squared() < square(radius))
			if ((cur_pos - camera_center_difference).magnitude_squared()
				> square(radius))
				//      break;
				return Vec3(-32768.0, 0.0, 0.0);
		}
		//  if (i == 10)
		//    return Vec3(-32768.0, 0.0, 0.0);
		//  else
		return cur_pos;
	}

	coord_t FilledBoundingSpawner::get_area() const
	{ // Not 100% accurate, but goot enough.  :)
		coord_t avg_radius = 0;
		for (float f = 0; f < 2 * PI; f += (2 * PI / 256.0))
			avg_radius += bounding_range->get_radius(f);
		avg_radius /= 256.0;
		return PI * square(avg_radius);
	}

	Vec3 NoncheckingFilledBoundingSpawner::get_new_coords()
	{
		const angle_t angle= randangle(2 * PI);
		const coord_t radius = bounding_range->get_radius(angle);
		const coord_t scalar= std::sqrt(randcoord());
		return Vec3(sin(angle) * scalar * radius, 0.0, cos(angle) * scalar
			* radius);
	}

	coord_t NoncheckingFilledBoundingSpawner::get_area() const
	{ // Not 100% accurate, but goot enough.  :)
		coord_t avg_radius = 0;
		for (float f = 0; f < 2 * PI; f += (2 * PI / 256.0))
			avg_radius += bounding_range->get_radius(f);
		avg_radius /= 256.0;
		return PI * square(avg_radius);
	}

	Vec3 HollowBoundingSpawner::get_new_coords()
	{
		const angle_t angle= randangle(2 * PI);
		const coord_t radius = bounding_range->get_radius(angle);
		return Vec3(sin(angle) * radius, 0.0, cos(angle) * radius);
	}

	coord_t HollowBoundingSpawner::get_area() const
	{ // Not 100% accurate, but goot enough.  :)
		coord_t avg_radius = 0;
		for (float f = 0; f < 2 * PI; f += (2 * PI / 256.0))
			avg_radius += bounding_range->get_radius(f);
		avg_radius /= 256.0;
		return PI * square(avg_radius);
	}

	EyeCandy::EyeCandy()
	{
		set_thresholds(10000, 13, 37);
		max_usec_per_particle_move = 100000;
		sprite_scalar = 0.11;
		max_point_size = 500.0;
		lighting_scalar = 1000.0;
		light_estimate = 0.0;
		use_lights = true;
#ifndef	NEW_TEXTURES
		draw_method = FAST_BILLBOARDS;
#endif	/* NEW_TEXTURES */
		billboard_scalar = 0.2;
		width = 800;
		height = 600;
		temp_sprite_scalar = sprite_scalar * height;
		last_forced_LOD = 10;
		framerate = 100.0;
		max_fps = 255.0;
#ifndef	NEW_TEXTURES
		poor_transparency_resolution = false;
#endif	/* NEW_TEXTURES */
		draw_shapes = true;
	}

	EyeCandy::EyeCandy(int _max_particles)
	{
		set_thresholds(_max_particles, 13, 37);
		max_usec_per_particle_move = 100000;
		sprite_scalar = 0.11;
		max_point_size = 500.0;
		lighting_scalar = 1000.0;
		light_estimate = 0.0;
		use_lights = true;
#ifndef	NEW_TEXTURES
		draw_method = FAST_BILLBOARDS;
#endif	/* NEW_TEXTURES */
		billboard_scalar = 0.2;
		width = 800;
		height = 600;
		temp_sprite_scalar = sprite_scalar * height;
		last_forced_LOD = 10;
		framerate = 100.0;
		max_fps = 255.0;
#ifndef	NEW_TEXTURES
		poor_transparency_resolution = false;
#endif	/* NEW_TEXTURES */
		draw_shapes = true;
	}

	EyeCandy::~EyeCandy()
	{
		for (std::vector<Particle*>::iterator iter = particles.begin(); iter
			!= particles.end(); iter++)
			delete *iter;
		for (std::vector<Effect*>::iterator iter = effects.begin(); iter
			!= effects.end(); iter++)
			delete *iter;
		for (std::vector<GLenum>::iterator iter = lights.begin(); iter
			!= lights.end(); iter++)
			glDisable(*iter);
	}

	void EyeCandy::set_thresholds(const int _max_particles,
		const float min_framerate, const float max_framerate)
	{
		max_particles = _max_particles;
		const float range = max_framerate - min_framerate;
		LOD_9_threshold = max_particles * 73 / 100;
		LOD_8_threshold = max_particles * 76 / 100;
		LOD_7_threshold = max_particles * 79 / 100;
		LOD_6_threshold = max_particles * 82 / 100;
		LOD_5_threshold = max_particles * 85 / 100;
		LOD_4_threshold = max_particles * 88 / 100;
		LOD_3_threshold = max_particles * 91 / 100;
		LOD_2_threshold = max_particles * 94 / 100;
		LOD_1_threshold = max_particles * 97 / 100;
		LOD_10_time_threshold = min_framerate + range * (9.0 / 9.0);
		LOD_9_time_threshold = min_framerate + range * (8.0 / 9.0);
		LOD_8_time_threshold = min_framerate + range * (7.0 / 9.0);
		LOD_7_time_threshold = min_framerate + range * (6.0 / 9.0);
		LOD_6_time_threshold = min_framerate + range * (5.0 / 9.0);
		LOD_5_time_threshold = min_framerate + range * (4.0 / 9.0);
		LOD_4_time_threshold = min_framerate + range * (3.0 / 9.0);
		LOD_3_time_threshold = min_framerate + range * (2.0 / 9.0);
		LOD_2_time_threshold = min_framerate + range * (1.0 / 9.0);
		LOD_1_time_threshold = min_framerate + range * (0.0 / 9.0);
		//  allowable_particles_to_add = max_particles;
	}

#ifdef	NEW_TEXTURES
	Uint32 EyeCandy::get_texture(const TextureEnum type) const
	{
		return get_texture_index(type);
	}

	void EyeCandy::set_particle_texture_combiner()
	{
		ELglActiveTextureARB(GL_TEXTURE1);

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 1);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE1);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);

		ELglActiveTextureARB(GL_TEXTURE0);

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE0);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 1);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE0);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);
	}

	void EyeCandy::set_shape_texture_combiner(const float alpha_scale)
	{
		float color[4];

		color[0] = alpha_scale;
		color[1] = alpha_scale;
		color[2] = alpha_scale;
		color[3] = alpha_scale;

		ELglActiveTextureARB(GL_TEXTURE1);

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 1);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE1);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);

		ELglActiveTextureARB(GL_TEXTURE0);

		glTexEnvfv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR, color);

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);

		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 1);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_CONSTANT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);
	}
#else	/* NEW_TEXTURES */
	void EyeCandy::clear_textures()
	{
		//clear any existing textures
		TexCrystal.clear();
		TexFlare.clear();
		TexInverse.clear();
		TexLeafAsh.clear();
		TexLeafMaple.clear();
		TexLeafOak.clear();
		TexPetal.clear();
		TexShimmer.clear();
		TexSimple.clear();
		TexSnowflake.clear();
		TexTwinflare.clear();
		TexVoid.clear();
		TexWater.clear();
	}
#endif	/* NEW_TEXTURES */

	void EyeCandy::load_textures()
	{
		// Load the textures.
#ifdef	NEW_TEXTURES
		texture_atlas = load_texture_cached("./textures/eye_candy.dds", tt_atlas);
		texture_burn = load_texture_cached("./textures/eye_candy_burn.dds", tt_atlas);
#else	/* NEW_TEXTURES */
		TexSimple.push_texture("./textures/eye_candy/16x16/simple.png");
		TexFlare.push_texture("./textures/eye_candy/16x16/flare1.png");
		TexFlare.push_texture("./textures/eye_candy/16x16/flare2.png");
		TexFlare.push_texture("./textures/eye_candy/16x16/flare3.png");
		TexVoid.push_texture("./textures/eye_candy/16x16/void1.png");
		TexVoid.push_texture("./textures/eye_candy/16x16/void2.png");
		TexVoid.push_texture("./textures/eye_candy/16x16/void3.png");
		TexTwinflare.push_texture("./textures/eye_candy/16x16/twinflare1.png");
		TexTwinflare.push_texture("./textures/eye_candy/16x16/twinflare2.png");
		TexTwinflare.push_texture("./textures/eye_candy/16x16/twinflare3.png");
		TexTwinflare.push_texture("./textures/eye_candy/16x16/twinflare4.png");
		TexTwinflare.push_texture("./textures/eye_candy/16x16/twinflare5.png");
		TexInverse.push_texture("./textures/eye_candy/16x16/inverse1.png");
		TexInverse.push_texture("./textures/eye_candy/16x16/inverse2.png");
		TexInverse.push_texture("./textures/eye_candy/16x16/inverse3.png");
		TexInverse.push_texture("./textures/eye_candy/16x16/inverse4.png");
		TexShimmer.push_texture("./textures/eye_candy/16x16/shimmer1.png");
		TexShimmer.push_texture("./textures/eye_candy/16x16/shimmer2.png");
		TexShimmer.push_texture("./textures/eye_candy/16x16/shimmer3.png");
		TexCrystal.push_texture("./textures/eye_candy/16x16/crystal1.png");
		TexCrystal.push_texture("./textures/eye_candy/16x16/crystal2.png");
		TexCrystal.push_texture("./textures/eye_candy/16x16/crystal3.png");
		TexWater.push_texture("./textures/eye_candy/16x16/water1.png");
		TexWater.push_texture("./textures/eye_candy/16x16/water2.png");
		TexWater.push_texture("./textures/eye_candy/16x16/water3.png");
		TexLeafMaple.push_texture("./textures/eye_candy/16x16/leaf_maple.png");
		TexLeafOak.push_texture("./textures/eye_candy/16x16/leaf_oak.png");
		TexLeafAsh.push_texture("./textures/eye_candy/16x16/leaf_ash.png");
		TexPetal.push_texture("./textures/eye_candy/16x16/petal.png");
		TexSnowflake.push_texture("./textures/eye_candy/16x16/snowflake.png");
		TexSimple.push_texture("./textures/eye_candy/32x32/simple.png");
		TexFlare.push_texture("./textures/eye_candy/32x32/flare1.png");
		TexFlare.push_texture("./textures/eye_candy/32x32/flare2.png");
		TexFlare.push_texture("./textures/eye_candy/32x32/flare3.png");
		TexVoid.push_texture("./textures/eye_candy/32x32/void1.png");
		TexVoid.push_texture("./textures/eye_candy/32x32/void2.png");
		TexVoid.push_texture("./textures/eye_candy/32x32/void3.png");
		TexTwinflare.push_texture("./textures/eye_candy/32x32/twinflare1.png");
		TexTwinflare.push_texture("./textures/eye_candy/32x32/twinflare2.png");
		TexTwinflare.push_texture("./textures/eye_candy/32x32/twinflare3.png");
		TexTwinflare.push_texture("./textures/eye_candy/32x32/twinflare4.png");
		TexTwinflare.push_texture("./textures/eye_candy/32x32/twinflare5.png");
		TexInverse.push_texture("./textures/eye_candy/32x32/inverse1.png");
		TexInverse.push_texture("./textures/eye_candy/32x32/inverse2.png");
		TexInverse.push_texture("./textures/eye_candy/32x32/inverse3.png");
		TexInverse.push_texture("./textures/eye_candy/32x32/inverse4.png");
		TexShimmer.push_texture("./textures/eye_candy/32x32/shimmer1.png");
		TexShimmer.push_texture("./textures/eye_candy/32x32/shimmer2.png");
		TexShimmer.push_texture("./textures/eye_candy/32x32/shimmer3.png");
		TexCrystal.push_texture("./textures/eye_candy/32x32/crystal1.png");
		TexCrystal.push_texture("./textures/eye_candy/32x32/crystal2.png");
		TexCrystal.push_texture("./textures/eye_candy/32x32/crystal3.png");
		TexWater.push_texture("./textures/eye_candy/32x32/water1.png");
		TexWater.push_texture("./textures/eye_candy/32x32/water2.png");
		TexWater.push_texture("./textures/eye_candy/32x32/water3.png");
		TexLeafMaple.push_texture("./textures/eye_candy/32x32/leaf_maple.png");
		TexLeafOak.push_texture("./textures/eye_candy/32x32/leaf_oak.png");
		TexLeafAsh.push_texture("./textures/eye_candy/32x32/leaf_ash.png");
		TexPetal.push_texture("./textures/eye_candy/32x32/petal.png");
		TexSnowflake.push_texture("./textures/eye_candy/32x32/snowflake.png");
		TexSimple.push_texture("./textures/eye_candy/64x64/simple.png");
		TexFlare.push_texture("./textures/eye_candy/64x64/flare1.png");
		TexFlare.push_texture("./textures/eye_candy/64x64/flare2.png");
		TexFlare.push_texture("./textures/eye_candy/64x64/flare3.png");
		TexVoid.push_texture("./textures/eye_candy/64x64/void1.png");
		TexVoid.push_texture("./textures/eye_candy/64x64/void2.png");
		TexVoid.push_texture("./textures/eye_candy/64x64/void3.png");
		TexTwinflare.push_texture("./textures/eye_candy/64x64/twinflare1.png");
		TexTwinflare.push_texture("./textures/eye_candy/64x64/twinflare2.png");
		TexTwinflare.push_texture("./textures/eye_candy/64x64/twinflare3.png");
		TexTwinflare.push_texture("./textures/eye_candy/64x64/twinflare4.png");
		TexTwinflare.push_texture("./textures/eye_candy/64x64/twinflare5.png");
		TexInverse.push_texture("./textures/eye_candy/64x64/inverse1.png");
		TexInverse.push_texture("./textures/eye_candy/64x64/inverse2.png");
		TexInverse.push_texture("./textures/eye_candy/64x64/inverse3.png");
		TexInverse.push_texture("./textures/eye_candy/64x64/inverse4.png");
		TexShimmer.push_texture("./textures/eye_candy/64x64/shimmer1.png");
		TexShimmer.push_texture("./textures/eye_candy/64x64/shimmer2.png");
		TexShimmer.push_texture("./textures/eye_candy/64x64/shimmer3.png");
		TexCrystal.push_texture("./textures/eye_candy/64x64/crystal1.png");
		TexCrystal.push_texture("./textures/eye_candy/64x64/crystal2.png");
		TexCrystal.push_texture("./textures/eye_candy/64x64/crystal3.png");
		TexWater.push_texture("./textures/eye_candy/64x64/water1.png");
		TexWater.push_texture("./textures/eye_candy/64x64/water2.png");
		TexWater.push_texture("./textures/eye_candy/64x64/water3.png");
		TexLeafMaple.push_texture("./textures/eye_candy/64x64/leaf_maple.png");
		TexLeafOak.push_texture("./textures/eye_candy/64x64/leaf_oak.png");
		TexLeafAsh.push_texture("./textures/eye_candy/64x64/leaf_ash.png");
		TexPetal.push_texture("./textures/eye_candy/64x64/petal.png");
		TexSnowflake.push_texture("./textures/eye_candy/64x64/snowflake.png");
		TexSimple.push_texture("./textures/eye_candy/128x128/simple.png");
		TexFlare.push_texture("./textures/eye_candy/128x128/flare1.png");
		TexFlare.push_texture("./textures/eye_candy/128x128/flare2.png");
		TexFlare.push_texture("./textures/eye_candy/128x128/flare3.png");
		TexVoid.push_texture("./textures/eye_candy/128x128/void1.png");
		TexVoid.push_texture("./textures/eye_candy/128x128/void2.png");
		TexVoid.push_texture("./textures/eye_candy/128x128/void3.png");
		TexTwinflare.push_texture("./textures/eye_candy/128x128/twinflare1.png");
		TexTwinflare.push_texture("./textures/eye_candy/128x128/twinflare2.png");
		TexTwinflare.push_texture("./textures/eye_candy/128x128/twinflare3.png");
		TexTwinflare.push_texture("./textures/eye_candy/128x128/twinflare4.png");
		TexTwinflare.push_texture("./textures/eye_candy/128x128/twinflare5.png");
		TexInverse.push_texture("./textures/eye_candy/128x128/inverse1.png");
		TexInverse.push_texture("./textures/eye_candy/128x128/inverse2.png");
		TexInverse.push_texture("./textures/eye_candy/128x128/inverse3.png");
		TexInverse.push_texture("./textures/eye_candy/128x128/inverse4.png");
		TexShimmer.push_texture("./textures/eye_candy/128x128/shimmer1.png");
		TexShimmer.push_texture("./textures/eye_candy/128x128/shimmer2.png");
		TexShimmer.push_texture("./textures/eye_candy/128x128/shimmer3.png");
		TexCrystal.push_texture("./textures/eye_candy/128x128/crystal1.png");
		TexCrystal.push_texture("./textures/eye_candy/128x128/crystal2.png");
		TexCrystal.push_texture("./textures/eye_candy/128x128/crystal3.png");
		TexWater.push_texture("./textures/eye_candy/128x128/water1.png");
		TexWater.push_texture("./textures/eye_candy/128x128/water2.png");
		TexWater.push_texture("./textures/eye_candy/128x128/water3.png");
		TexLeafMaple.push_texture("./textures/eye_candy/128x128/leaf_maple.png");
		TexLeafOak.push_texture("./textures/eye_candy/128x128/leaf_oak.png");
		TexLeafAsh.push_texture("./textures/eye_candy/128x128/leaf_ash.png");
		TexPetal.push_texture("./textures/eye_candy/128x128/petal.png");
		TexSnowflake.push_texture("./textures/eye_candy/128x128/snowflake.png");
#endif	/* NEW_TEXTURES */
	}

	void EyeCandy::push_back_effect(Effect* e)
	{
		effects.push_back(e);
	}

	bool EyeCandy::push_back_particle(Particle* p)
	{
		if /*(*/((int)particles.size() >= max_particles)/* || (!allowable_particles_to_add))*/
		{
			delete p;
			return false;
		}
		else
		{
			particles.push_back(p);
			p->effect->register_particle(p);
			light_estimate += p->estimate_light_level();
			//    allowable_particles_to_add--;
			return true;
		}
	}

	void EyeCandy::start_draw()
	{
		glDisable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(false);
#ifdef	NEW_TEXTURES
		float modelview[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, modelview);

		const Vec3 right(modelview[0], modelview[4], modelview[8]);
		const Vec3 up(modelview[1], modelview[5], modelview[9]);
		corner_offset1 = (right + up) * billboard_scalar;
		corner_offset2 = (right - up) * billboard_scalar;

		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		ELglActiveTextureARB(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		bind_texture(texture_atlas);

		ELglActiveTextureARB(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		bind_texture(texture_burn);

		set_particle_texture_combiner();

#ifndef MAP_EDITOR
		/* Fog hurts blending with 5 color blending
		 * (red, green, blue, alpha and burn)
		 */
		if (use_fog)
		{
			glDisable(GL_FOG);
		}
#endif
#else	/* NEW_TEXTURES */
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		if ((draw_method == FAST_BILLBOARDS) || (draw_method == POINT_SPRITES)) //We need to do this for point sprites as well because if the particle is too big, it falls through to fast billboards.
		{
			float modelview[16];
			glGetFloatv(GL_MODELVIEW_MATRIX, modelview);

			const Vec3 right(modelview[0], modelview[4], modelview[8]);
			const Vec3 up(modelview[1], modelview[5], modelview[9]);
			corner_offset1 = (right + up) * billboard_scalar;
			corner_offset2 = (right - up) * billboard_scalar;
		}

		if (draw_method == POINT_SPRITES)
		{
			// Scale
			//    glPointSize(100);	// Max on some vidcards
			//    float quadratic[] = {0.0f, 0.0f, 0.01f};
			//    glPointParameterfvARB(GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic);
			//    glPointParameterfARB(GL_POINT_SIZE_MAX_ARB, 200.0f);
			//    glPointParameterfARB(GL_POINT_SIZE_MIN_ARB, 1.0f);

			glEnable(GL_POINT_SPRITE_ARB);
			glTexEnvf(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);
			glGetFloatv(GL_POINT_SIZE_MAX_ARB, &max_allowable_point_size);
		}
#endif	/* NEW_TEXTURES */
	}

	void EyeCandy::end_draw()
	{
#ifdef	NEW_TEXTURES
		ELglActiveTextureARB(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		ELglActiveTextureARB(GL_TEXTURE0);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

#ifndef MAP_EDITOR
		if (use_fog)
		{
			glEnable(GL_FOG);
		}
#endif
#else	/* NEW_TEXTURES */
		if (draw_method == POINT_SPRITES)
		{
			glDisable(GL_POINT_SPRITE_ARB);
		}
#endif	/* NEW_TEXTURES */
		glColor4f(1.0, 1.0, 1.0, 1.0);
		glDisable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(true);
		glDisable(GL_COLOR_MATERIAL);
		glEnable(GL_LIGHTING);
	}

	void EyeCandy::draw()
	{
		if (ec_error_status)
			return;

		start_draw();

#ifdef	NEW_TEXTURES
		Uint32 i, count;

		count = effects.size();

		// Draw effects (any special drawing functionality) and their particles.
		for (i = 0; i < count; i++)
		{
			Effect* e = effects[i];

			if (e->active)
			{
				e->draw(time_diff);
				e->draw_particle_buffer();
			}
		}

		el::HardwareBuffer::unbind(el::hbt_index);
		el::HardwareBuffer::unbind(el::hbt_vertex);
#else	/* NEW_TEXTURES */
		// Draw effects (any special drawing functionality) and their particles.
		for (std::vector<Effect*>::const_iterator iter = effects.begin(); iter
			!= effects.end(); iter++)
		{
			Effect* e = *iter;
			if (!e->active)
				continue;

			e->draw(time_diff);

			// Draw particles
			if (e->bounds)
			{
				for (std::map<Particle*, bool>::const_iterator iter2 = (*iter)->particles.begin(); iter2 != (*iter)->particles.end(); iter2++)
				{
					Particle* p = iter2->first;
					const coord_t dist_squared = (p->pos - center).magnitude_squared();
					if (dist_squared < MAX_DRAW_DISTANCE_SQUARED)
						p->draw(time_diff);
				}
			}
			else
			{
				for (std::map<Particle*, bool>::const_iterator iter2 = (*iter)->particles.begin(); iter2 != (*iter)->particles.end(); iter2++)
				{
					Particle* p = iter2->first;
					p->draw(time_diff);
				}
			}
		}
#endif	/* NEW_TEXTURES */

		end_draw();
		// Draw lights.
		if ((use_lights) && (particles.size() > 0))
		{
			while (light_particles.size() < lights.size())
				light_particles.push_back(std::pair<Particle*, float>(particles[randint((int)particles.size())], 0.0));
				for (int i = 0; i < (int)light_particles.size(); i++)
				{
					Particle* p = light_particles[i].first;
					if (p->get_light_level() < 0.00005)
					{
						int j;
						for (j = 0; j < 40; j++)
						{
							light_particles[i] = std::pair<Particle*, float>(*(particles.begin() + randint((int)particles.size())), 0.0);
							Particle* p = light_particles[i].first;
							if (p->get_light_level()> 0.0001)
							break;
						}
						if (j == 40)
						continue;
					}

					light_particles[i].second += time_diff / 100000.0;
					if (light_particles[i].second >= 1.0)
					light_particles[i].second = 1.0;

					const light_t light_level = p->get_light_level() * light_particles[i].second;
					const GLenum light_id = lights[i];
					const GLfloat light_pos[4] =
					{	p->pos.x, p->pos.y, p->pos.z, 1.0};
					const light_t brightness = light_estimate * lighting_scalar * light_level;
					const GLfloat light_color[4] =
					{	p->color[0] * brightness, p->color[1] * brightness, p->color[2] * brightness, 0.0};
					glEnable(light_id);
					glLightfv(light_id, GL_POSITION, light_pos);
					glLightfv(light_id, GL_DIFFUSE, light_color);
				}
				for (int i = (int)light_particles.size(); i < (int)lights.size(); i++)
				glDisable(lights[i]);
		}
		else
		{
			for (int i = 0; i < (int)lights.size(); i++)
			glDisable(lights[i]); // Save the graphics card some work when rendering the rest of the scene, ne? :)
		}
	}

		void EyeCandy::idle()
		{
			if (ec_error_status)
			return;

			const Uint64 cur_time = get_time();
			if (time_diff < 10)
			time_diff = 10;

#if defined CLUSTER_INSIDES && !defined MAP_EDITOR
			short cluster = get_actor_cluster ();
#endif
			for (int i = 0; i < (int)effects.size(); )
			{
				std::vector<Effect*>::iterator iter = effects.begin() + i;
				Effect* e = *iter;

				//    std::cout << e << ": " << e->get_expire_time() << ", " << cur_time << std::endl;
				if (e->get_expire_time() < cur_time)
				{
					e->recall = true;
				}

				Vec3 shifted_pos = *(e->pos) - center;
				coord_t distance_squared = shifted_pos.planar_magnitude_squared();
				//    std::cout << e << ": " << center << ", " << *e->pos << ": " << (center - *(e->pos)).magnitude_squared() << " <? " << MAX_DRAW_DISTANCE_SQUARED << std::endl;
#if defined CLUSTER_INSIDES && !defined MAP_EDITOR
				bool same_cluster = e->belongsToCluster (cluster);
#endif
				if (!e->active)
				{
					if (e->bounds)
					{
						const angle_t angle = atan2(shifted_pos.x, shifted_pos.z);
						if (distance_squared < MAX_DRAW_DISTANCE_SQUARED + e->bounds->get_radius(angle)
#if defined CLUSTER_INSIDES && !defined MAP_EDITOR
						&& same_cluster
#endif
						)
						{
							if (EC_DEBUG)
							std::cout << "Activating effect(2) " << e << " (" << distance_squared << " < " << (MAX_DRAW_DISTANCE_SQUARED + e->bounds->get_radius(angle)) << ")" << std::endl;
							e->active = true;
						}
						else if (!e->recall)
						{
							i++;
							continue;
						}
					}
					else
					{
						if (distance_squared < MAX_DRAW_DISTANCE_SQUARED
#if defined CLUSTER_INSIDES && !defined MAP_EDITOR
						&& same_cluster
#endif
						)
						{
							if (EC_DEBUG)
							std::cout << "Activating effect " << e << " (" << distance_squared << " < " << MAX_DRAW_DISTANCE_SQUARED << ")" << std::endl;
							e->active = true;
						}
						else if (!e->recall)
						{
							i++;
							continue;
						}
					}
				}
				else
				{
					if (e->bounds)
					{
						const angle_t angle = atan2(shifted_pos.x, shifted_pos.z);
						if (distance_squared > e->bounds->get_radius(angle) + MAX_DRAW_DISTANCE_SQUARED
#if defined CLUSTER_INSIDES && !defined MAP_EDITOR
						|| !same_cluster
#endif
						)
						{
							if (EC_DEBUG)
							std::cout << "Deactivating effect(2) " << e << " (" << distance_squared << " > " << e->bounds->get_radius(angle) + MAX_DRAW_DISTANCE_SQUARED << ": " << *(e->pos) << ", " << center << ", " << e->bounds->get_radius(angle)
#if defined CLUSTER_INSIDES && !defined MAP_EDITOR
								<< ", " << same_cluster
#endif
							  	<< ")" << std::endl;
							e->active = false;
						}
					}
					else
					{
						if (distance_squared > MAX_DRAW_DISTANCE_SQUARED
#if defined CLUSTER_INSIDES && !defined MAP_EDITOR
						|| !same_cluster
#endif
						)
						{
							if (e->get_type() != EC_MISSILE) // don't deactivate missed missiles

							{
								if (EC_DEBUG)
								std::cout << "Deactivating effect " << e << " (" << distance_squared << " > " << MAX_DRAW_DISTANCE_SQUARED << ")" << std::endl;
								e->active = false;
							}
						}
					}
				}

				const bool ret = e->idle(time_diff);
				if (!ret)
				{
					e->recall = true;
					effects.erase(iter);
					delete e;
				}
				else
				i++;
			}

			// If we're nearing our particle limit, lower our level of detail.
			float change_LOD;
			if (particles.size() < LOD_9_threshold)
			change_LOD = 10.0;
			else if (particles.size() < LOD_8_threshold)
			change_LOD = 10 - float(particles.size() - LOD_9_threshold) / float(LOD_8_threshold - LOD_9_threshold);
			else if (particles.size() < LOD_7_threshold)
			change_LOD = 9.0 - float(particles.size() - LOD_8_threshold) / float(LOD_7_threshold - LOD_8_threshold);
			else if (particles.size() < LOD_6_threshold)
			change_LOD = 8.0 - float(particles.size() - LOD_7_threshold) / float(LOD_6_threshold - LOD_7_threshold);
			else if (particles.size() < LOD_5_threshold)
			change_LOD = 7.0 - float(particles.size() - LOD_6_threshold) / float(LOD_5_threshold - LOD_6_threshold);
			else if (particles.size() < LOD_4_threshold)
			change_LOD = 6.0 - float(particles.size() - LOD_5_threshold) / float(LOD_4_threshold - LOD_5_threshold);
			else if (particles.size() < LOD_3_threshold)
			change_LOD = 5.0 - float(particles.size() - LOD_4_threshold) / float(LOD_3_threshold - LOD_4_threshold);
			else if (particles.size() < LOD_2_threshold)
			change_LOD = 4.0 - float(particles.size() - LOD_3_threshold) / float(LOD_2_threshold - LOD_3_threshold);
			else if (particles.size() < LOD_1_threshold)
			change_LOD = 3.0 - float(particles.size() - LOD_2_threshold) / float(LOD_1_threshold - LOD_2_threshold);
			else if ((int)particles.size() < max_particles)
			change_LOD = 2.0 - float(particles.size() - LOD_1_threshold) / float(max_particles - LOD_1_threshold);
			else
			change_LOD = 1.0;

			float change_LOD2;
			if (framerate >= LOD_10_time_threshold)
			change_LOD2 = 10.0;
			else if (framerate >= LOD_9_time_threshold)
			change_LOD2 = 10.0 - float(framerate - LOD_9_time_threshold) / float(LOD_10_time_threshold - LOD_9_time_threshold);
			else if (framerate >= LOD_8_time_threshold)
			change_LOD2 = 9.0 - float(framerate - LOD_8_time_threshold) / float(LOD_9_time_threshold - LOD_8_time_threshold);
			else if (framerate >= LOD_7_time_threshold)
			change_LOD2 = 8.0 - float(framerate - LOD_7_time_threshold) / float(LOD_8_time_threshold - LOD_7_time_threshold);
			else if (framerate >= LOD_6_time_threshold)
			change_LOD2 = 7.0 - float(framerate - LOD_6_time_threshold) / float(LOD_7_time_threshold - LOD_6_time_threshold);
			else if (framerate >= LOD_5_time_threshold)
			change_LOD2 = 6.0 - float(framerate - LOD_5_time_threshold) / float(LOD_6_time_threshold - LOD_5_time_threshold);
			else if (framerate >= LOD_4_time_threshold)
			change_LOD2 = 5.0 - float(framerate - LOD_4_time_threshold) / float(LOD_5_time_threshold - LOD_4_time_threshold);
			else if (framerate >= LOD_3_time_threshold)
			change_LOD2 = 4.0 - float(framerate - LOD_3_time_threshold) / float(LOD_4_time_threshold - LOD_3_time_threshold);
			else if (framerate >= LOD_2_time_threshold)
			change_LOD2 = 3.0 - float(framerate - LOD_2_time_threshold) / float(LOD_3_time_threshold - LOD_2_time_threshold);
			else if (framerate >= LOD_1_time_threshold)
			change_LOD2 = 2.0 - float(framerate - LOD_1_time_threshold) / float(LOD_2_time_threshold - LOD_1_time_threshold);
			else
			change_LOD2 = 1.0;

			//  std::cout << framerate << ", " << LOD_10_time_threshold << " : " << last_forced_LOD << ": " << change_LOD << " / " << change_LOD2 << " (" << particles.size() << ")" << std::endl;

			if (change_LOD> change_LOD2) //Pick whichever one is lower.
			change_LOD = change_LOD2;

			for (std::vector<Effect*>::iterator iter = effects.begin(); iter != effects.end(); iter++)
			(*iter)->request_LOD(change_LOD);

			const float particle_cleanout_rate = (1.0 - std::pow(0.5f, 5.0f / (framerate * square(change_LOD))));
			//  std::cout << (1.0 / particle_cleanout_rate) << std::endl;
			float counter = randfloat();
			for (int i = 0; i < (int)particles.size(); ) //Iterate using an int, not an iterator, because we may be adding/deleting entries, and that messes up iterators.

			{
				std::vector<Particle*>::iterator iter = particles.begin() + i;
				Particle* p = *iter;

				counter -= particle_cleanout_rate;
				if (counter < 0) // Kill off a random particle.

				{
					counter++;
					if ((p->deletable()) && (!p->effect->active))
					{
						particles.erase(iter);
						for (int j = 0; j < (int)light_particles.size(); )
						{
							std::vector< std::pair<Particle*, light_t> >::iterator iter2 = light_particles.begin() + j;
							if (iter2->first == p)
							{
								light_particles.erase(iter2);
								continue;
							}
							j++;
						}
						p->effect->unregister_particle(p);
						light_estimate -= p->estimate_light_level();
						delete p;
					}

					i++;
					continue;
				}

				if ((!p->effect->active) && (!p->effect->recall))
				{
					i++;
					continue;
				}

				p->mover->move(*p, time_diff);
				const bool ret = p->idle(time_diff);
				if (!ret)
				{
					iter = particles.begin() + i; //Why the heck do I need to redo this just because I've push_back'ed entries to the vector in idle()?  :P  Makes no sense.  My best guess: array resizing.
					particles.erase(iter);
					for (int j = 0; j < (int)light_particles.size(); )
					{
						std::vector< std::pair<Particle*, light_t> >::iterator iter2 = light_particles.begin() + j;
						if (iter2->first == p)
						{
							light_particles.erase(iter2);
							continue;
						}
						j++;
					}
					p->effect->unregister_particle(p);
					light_estimate -= p->estimate_light_level();
					delete p;
				}
				else
				i++;
			}
			last_forced_LOD = (Uint16)round(change_LOD);

			//  allowable_particles_to_add = 1 + (int)(particles.size() * 0.00005 * time_diff / 1000000.0 * (max_particles - particles.size()) * change_LOD);
			//  std::cout << "Current: " << particles.size() << "; Allowable new: " << allowable_particles_to_add << std::endl;
#ifdef	NEW_TEXTURES
			Uint32 i, count;

			count = effects.size();

			for (i = 0; i < count; i++)
			{
				Effect* e = effects[i];

				if (e->active)
				{
					e->build_particle_buffer(time_diff);
				}
			}

			el::HardwareBuffer::unbind(el::hbt_index);
			el::HardwareBuffer::unbind(el::hbt_vertex);
#endif	/* NEW_TEXTURES */
		}

		void EyeCandy::add_light(GLenum light_id)
		{
			glDisable(light_id);
			lights.push_back(light_id);
			GLfloat light_pos[4] =
			{	0.0, 0.0, 0.0, 1.0};
			const GLfloat light_white[4] =
			{	0.0, 0.0, 0.0, 0.0};
			glLightfv(light_id, GL_SPECULAR, light_white);
			glLightfv(light_id, GL_POSITION, light_pos);
			glLightf(light_id, GL_LINEAR_ATTENUATION, 1.0);
		}

#ifndef	NEW_TEXTURES
		void EyeCandy::draw_point_sprite_particle(coord_t size, const GLuint texture, const color_t r, const color_t g, const color_t b, const alpha_t alpha, const Vec3 pos)
		{
			//  std::cout << "A: " << size << ", " << texture << ", " << Vec3(r, g, b) << ", " << alpha << std::endl;
			glPointSize(size);

			glBindTexture(GL_TEXTURE_2D, texture);

			glBegin(GL_POINTS);
			{
				glColor4f(r, g, b, alpha);
				glVertex3d(pos.x, pos.y, pos.z);
			}
			glEnd();
		}

		void EyeCandy::draw_fast_billboard_particle(coord_t size, const GLuint texture, const color_t r, const color_t g, const color_t b, const alpha_t alpha, const Vec3 pos)
		{
			//  std::cout << "B: " << size << ", " << texture << ", " << Vec3(r, g, b) << ", " << alpha << std::endl;
			const Vec3 corner1(pos - corner_offset1 * size);
			const Vec3 corner2(pos + corner_offset2 * size);
			const Vec3 corner3(pos + corner_offset1 * size);
			const Vec3 corner4(pos - corner_offset2 * size);

			//  std::cout << corner1 << ", " << corner2 << ", " << corner3 << ", " << corner4 << std::endl;
			//  std::cout << size << ", " << texture << ", " << Vec3(r, g, b) << ", " << alpha << std::endl;

			glBindTexture(GL_TEXTURE_2D, texture);

			glBegin(GL_QUADS);
			{
				glColor4f(r, g, b, alpha);
				glTexCoord2f(0.0, 0.0);
				glVertex3f(corner1.x, corner1.y, corner1.z);
				glTexCoord2f(1.0, 0.0);
				glVertex3f(corner2.x, corner2.y, corner2.z);
				glTexCoord2f(1.0, 1.0);
				glVertex3f(corner3.x, corner3.y, corner3.z);
				glTexCoord2f(0.0, 1.0);
				glVertex3f(corner4.x, corner4.y, corner4.z);
			}
			glEnd();
		}

		void EyeCandy::draw_accurate_billboard_particle(coord_t size, const GLuint texture, const color_t r, const color_t g, const color_t b, const alpha_t alpha, const Vec3 pos)
		{
			glPushMatrix();
			glTranslatef(pos.x, pos.y, pos.z);

			Vec3 object_to_camera_projection(camera.x - pos.x, 0, camera.z - pos.z);
			object_to_camera_projection.normalize();

			const Vec3 look_at(0, 0, 1);
			const Vec3 up = look_at.cross(object_to_camera_projection);
			const float cos_angle = look_at.dot(object_to_camera_projection);

			if ((cos_angle < 0.9999) && (cos_angle> -0.9999))
			glRotatef(acos(cos_angle) * 180.0 / PI, up.x, up.y, up.z);

			Vec3 object_to_camera = camera - pos;
			object_to_camera.normalize();
			const float cos_angle2 = object_to_camera_projection.dot(object_to_camera);

			if ((cos_angle2 < 0.9999) && (cos_angle2> -0.9999))
			{
				if (object_to_camera.y < 0)
				glRotatef(acos(cos_angle2) * 180.0 / PI, 1, 0, 0);
				else
				glRotatef(acos(cos_angle2) * 180.0 / PI, -1, 0, 0);
			}

			const float scaled_size = size * billboard_scalar;

			glBindTexture(GL_TEXTURE_2D, texture);
			/*
			 glBegin(GL_QUADS);
			 {
			 glColor4f(r, g, b, alpha);
			 glTexCoord2f(0.0, 0.0);
			 glVertex3f(pos.x - scaled_size, pos.y - scaled_size, pos.z);
			 glTexCoord2f(1.0, 0.0);
			 glVertex3f(pos.x + scaled_size, pos.y - scaled_size, pos.z);
			 glTexCoord2f(1.0, 1.0);
			 glVertex3f(pos.x + scaled_size, pos.y + scaled_size, pos.z);
			 glTexCoord2f(0.0, 1.0);
			 glVertex3f(pos.x - scaled_size, pos.y + scaled_size, pos.z);
			 }
			 glEnd();
			 */
			glBegin(GL_QUADS);
			{
				glColor4f(r, g, b, alpha);
				glTexCoord2f(0.0, 0.0);
				glVertex3f(-scaled_size, -scaled_size, 0.0);
				glTexCoord2f(1.0, 0.0);
				glVertex3f(scaled_size, -scaled_size, 0.0);
				glTexCoord2f(1.0, 1.0);
				glVertex3f(scaled_size, scaled_size, 0.0);
				glTexCoord2f(0.0, 1.0);
				glVertex3f(-scaled_size, scaled_size, 0.0);
			}
			glEnd();
			glPopMatrix();
		}
#endif	/* NEW_TEXTURES */

		// F U N C T I O N S //////////////////////////////////////////////////////////

		Uint64 get_time()
		{
#if defined(_WIN32) || defined(_WIN64)
			FILETIME ft;
			GetSystemTimeAsFileTime(&ft);
			Uint64 ret = ft.dwHighDateTime;
			ret <<= 32;
			ret |= ft.dwLowDateTime;
			ret /= 10;
			return ret;
#else
			struct timeval t;
			gettimeofday(&t, NULL);
			return ((Uint64)t.tv_sec)*1000000ul + (Uint64)t.tv_usec;
#endif
		}

		void hsv_to_rgb(const color_t h, const color_t s, const color_t v, color_t& r, color_t& g, color_t& b)
		{
			if(!s) // Greyscale

			{
				r = g = b = v;
				return;
			}

			const color_t float_sector = h * 6;
			const int sector = (int)floor(float_sector);
			const float sector_percent = float_sector - sector;
			const float inv_sector_percent = 1.0 - sector_percent;
			const float p = v * (1 - s);
			const float q = v * (1 - s * sector_percent);
			const float t = v * (1 - s * inv_sector_percent);
			switch (sector)
			{
				case 0:
				{
					r = v;
					g = t;
					b = p;
					break;
				}
				case 1:
				{
					r = q;
					g = v;
					b = p;
					break;
				}
				case 2:
				{
					r = p;
					g = v;
					b = t;
					break;
				}
				case 3:
				{
					r = p;
					g = q;
					b = v;
					break;
				}
				case 4:
				{
					r = t;
					g = p;
					b = v;
					break;
				}
				default:
				{
					r = v;
					g = p;
					b = q;
					break;
				}
			}
		}

		///////////////////////////////////////////////////////////////////////////////

	};

