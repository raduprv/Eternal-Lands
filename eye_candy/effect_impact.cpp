// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_impact.h"

namespace ec
{

	// C L A S S   F U N C T I O N S //////////////////////////////////////////////

	ImpactParticle::ImpactParticle(Effect* _effect, ParticleMover* _mover,
		const Vec3 _pos, const Vec3 _velocity, const coord_t _size,
		const alpha_t _alpha, const color_t red, const color_t green,
#ifdef	NEW_TEXTURES
		const color_t blue, TextureEnum _texture, const Uint16 _LOD,
#else	/* NEW_TEXTURES */
		const color_t blue, Texture* _texture, const Uint16 _LOD,
#endif	/* NEW_TEXTURES */
		const ImpactEffect::ImpactType _type) :
		Particle(_effect, _mover, _pos, _velocity,
			(0.3 + randcoord()) * 15 / 3.16 / std::sqrt(_LOD))
	{
		type = _type;
		color[0] = red;
		color[1] = green;
		color[2] = blue;
		texture = _texture;
		alpha = _alpha;
		velocity /= size;
		size *= _size;
		//  std::cout << ": " << velocity << std::endl;
		flare_max = 1.0;
		flare_exp = 0.1;
		flare_frequency = 50.0;
		LOD = _LOD;
		state = 0;
	}

	bool ImpactParticle::idle(const Uint64 delta_t)
	{
		const float float_time = delta_t / 1000000.0;
		switch (type)
		{
			case ImpactEffect::MAGIC_PROTECTION:
			{
				const alpha_t scalar = 1.0
					- pow_randfloat(float_time * 4.0f);
				alpha -= scalar;
				if (alpha < 0.02)
					return false;
				break;
			}
			case ImpactEffect::SHIELD:
			{
				const alpha_t scalar = 1.0
					- pow_randfloat(float_time * 4.0f);
				alpha -= scalar;
				if (alpha < 0.02)
					return false;
				break;
			}
			case ImpactEffect::MAGIC_IMMUNITY:
			{
				const alpha_t scalar = 1.0
					- pow_randfloat(float_time * 2.0f);
				alpha -= scalar;
				if (alpha < 0.02)
					return false;
				break;
			}
			case ImpactEffect::POISON:
			{
				const alpha_t scalar = 1.0
					- pow_randfloat(float_time * 1.4f);
				alpha -= scalar;
				if (alpha < 0.02)
					return false;
				break;
			}
			case ImpactEffect::BLOOD:
			{
				const alpha_t scalar = 1.0
					- pow_randfloat(float_time * 0.8f);
				alpha -= scalar;
				if (alpha < 0.02)
					return false;
				break;
			}
		}

		return true;
	}

#ifdef	NEW_TEXTURES
	Uint32 ImpactParticle::get_texture()
	{
		return base->get_texture(texture);
	}

	float ImpactParticle::get_burn() const
	{
		if (state != 1)
		{
			return 1.0f;
		}
		else
		{
			return 0.0f;
		}
	}
#else	/* NEW_TEXTURES */
	GLuint ImpactParticle::get_texture(const Uint16 res_index)
	{
		return texture->get_texture(res_index);
	}

	void ImpactParticle::draw(const Uint64 usec)
	{
		if (state == 1)
		{
			glEnable(GL_LIGHTING);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			Vec3 normal;
			normal.randomize();
			normal.normalize();
			glNormal3f(normal.x, normal.y, normal.z);
		}

		Particle::draw(usec);

		if (state == 1)
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glDisable(GL_LIGHTING);
		}
	}
#endif	/* NEW_TEXTURES */

	ImpactEffect::ImpactEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
		const Vec3 _angle, const ImpactType _type, const Uint16 _LOD,
		const float _strength)
	{
		if (EC_DEBUG)
			std::cout << "ImpactEffect (" << this << ") created." << std::endl;
		base = _base;
		dead = _dead;
		pos = _pos;
		center = *pos;
		angle = _angle;
		type = _type;
		LOD = base->last_forced_LOD;
		desired_LOD = _LOD;
		spawner = NULL;
		bounds = NULL;
		mover = NULL;
		strength = _strength;
		const coord_t size_scalar = strength * 1.3;
		const coord_t vel_scalar= std::sqrt(strength) * 0.44;

		switch (type)
		{
			case MAGIC_PROTECTION:
			{
				angle.normalize(2.0 * vel_scalar);
				mover = new ParticleMover(this);
				for (int i = 0; i < 50 * LOD; i++)
				{
					Vec3 velocity = -angle;
					Vec3 offset;
					offset.randomize(0.3);
					velocity += offset;
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new ImpactParticle(this, mover, center, velocity, 0.3 * size_scalar, 1.0, 0.7, 0.2, 0.4, EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
							new ImpactParticle(this, mover, center, velocity, 0.3 * size_scalar, 1.0, 0.7, 0.2, 0.4, &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case SHIELD:
			{
				angle.normalize(2.0 * vel_scalar);
				mover = new ParticleMover(this);
				for (int i = 0; i < 50 * LOD; i++)
				{
					Vec3 velocity = -angle;
					Vec3 offset;
					offset.randomize(0.3);
					velocity += offset;
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new ImpactParticle(this, mover, center, velocity, 0.3 * size_scalar, 1.0, 0.9, 0.9, 0.9, EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
							new ImpactParticle(this, mover, center, velocity, 0.3 * size_scalar, 1.0, 0.9, 0.9, 0.9, &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case MAGIC_IMMUNITY:
			{
				angle.normalize(2.5 * vel_scalar);
				mover = new ParticleMover(this);
				for (int i = 0; i < 50 * LOD; i++)
				{
					Vec3 velocity = -angle;
					Vec3 offset;
					offset.randomize(0.4);
					velocity += offset;
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new ImpactParticle(this, mover, center, velocity, 0.35 * size_scalar, 1.0, randcolor(1.0), randcolor(1.0), randcolor(1.0), EC_VOID, LOD, type);
#else	/* NEW_TEXTURES */
							new ImpactParticle(this, mover, center, velocity, 0.35 * size_scalar, 1.0, randcolor(1.0), randcolor(1.0), randcolor(1.0), &(base->TexVoid), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case POISON:
			{
				angle.normalize(1.0 * vel_scalar);
				mover = new ParticleMover(this);
				for (int i = 0; i < 50 * LOD; i++)
				{
					Vec3 velocity = -angle;
					Vec3 offset;
					offset.randomize(0.7);
					velocity += offset;
					Particle* p;
					if (randfloat() < 0.4)
					{
						p
#ifdef	NEW_TEXTURES
							= new ImpactParticle(this, mover, center, velocity, 0.6 * size_scalar, 0.5, 0.2 + randcolor(0.2), 0.5 + randcolor(0.3), 0.2, EC_FLARE, LOD, type);
#else	/* NEW_TEXTURES */
							= new ImpactParticle(this, mover, center, velocity, 0.6 * size_scalar, 0.5, 0.2 + randcolor(0.2), 0.5 + randcolor(0.3), 0.2, &(base->TexFlare), LOD, type);
#endif	/* NEW_TEXTURES */
						p->state = 1;
					}
					else
					{
						p
#ifdef	NEW_TEXTURES
							= new ImpactParticle(this, mover, center, velocity, 0.3 * size_scalar, 1.0, randcolor(0.1), 0.2 + randcolor(0.1), 0.2, EC_WATER, LOD, type);
#else	/* NEW_TEXTURES */
							= new ImpactParticle(this, mover, center, velocity, 0.3 * size_scalar, 1.0, randcolor(0.1), 0.2 + randcolor(0.1), 0.2, &(base->TexWater), LOD, type);
#endif	/* NEW_TEXTURES */
						p->state = 0;
					}
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case BLOOD:
			{
				angle.normalize(0.8 * vel_scalar);
				mover = new SimpleGravityMover(this);
				for (int i = 0; i < 20 * LOD; i++)
				{
					Vec3 velocity = -angle;
					Vec3 offset;
					offset.randomize(0.7);
					//        std::cout << velocity << ", " << angle << ", " << vel_scalar << std::endl;
					velocity += offset;
					velocity.normalize(0.8 * vel_scalar);
					//        std::cout << velocity << std::endl;
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new ImpactParticle(this, mover, center, velocity, square(square(randcoord(0.85))) * size_scalar, 0.5, 0.3 + randcolor(0.7), 0.15 + randcolor(0.1), 0.15 + randcolor(0.1), EC_WATER, LOD, type);
#else	/* NEW_TEXTURES */
							new ImpactParticle(this, mover, center, velocity, square(square(randcoord(0.85))) * size_scalar, 0.5, 0.3 + randcolor(0.7), 0.15 + randcolor(0.1), 0.15 + randcolor(0.1), &(base->TexWater), LOD, type);
#endif	/* NEW_TEXTURES */
					p->state = 1;
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
		}
	}

	ImpactEffect::~ImpactEffect()
	{
		if (spawner)
			delete spawner;
		if (mover)
			delete mover;
		if (EC_DEBUG)
			std::cout << "ImpactEffect (" << this << ") destroyed."
				<< std::endl;
	}

	bool ImpactEffect::idle(const Uint64 usec)
	{
		if (particles.size() == 0)
			return false;

		return true;
	}

///////////////////////////////////////////////////////////////////////////////

}
;

