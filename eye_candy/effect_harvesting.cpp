// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_harvesting.h"

namespace ec
{

	// C L A S S   F U N C T I O N S //////////////////////////////////////////////

	HarvestingParticle::HarvestingParticle(Effect* _effect,
		ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity,
		const coord_t _size, const alpha_t _alpha, const color_t red,
#ifdef	NEW_TEXTURES
		const color_t green, const color_t blue, TextureEnum _texture,
#else	/* NEW_TEXTURES */
		const color_t green, const color_t blue, Texture* _texture,
#endif	/* NEW_TEXTURES */
		const Uint16 _LOD, const HarvestingEffect::HarvestingType _type) :
		Particle(_effect, _mover, _pos, _velocity,
			_size * (0.5 + randcoord()) * 15 / (_LOD + 5))
	{
		type = _type;
		color[0] = red;
		color[1] = green;
		color[2] = blue;
		texture = _texture;
		alpha = _alpha;
		flare_max = 5.0;
		flare_exp = 0.1;
		flare_frequency = 3.0;
		LOD = _LOD;
		state = 0;
	}

	bool HarvestingParticle::idle(const Uint64 delta_t)
	{
		if (effect->recall)
			return false;

		const interval_t float_time = delta_t / 1000000.0;
		const Uint64 age = get_time() - born;
		switch (type)
		{
			case HarvestingEffect::RADON_POUCH:
			{
				if (alpha < 0.01)
					return false;

				const alpha_t scalar =
					pow_randfloat(float_time * 2);
				alpha *= scalar;

				break;
			}
			case HarvestingEffect::TOOL_BREAK:
			{
				if (alpha < 0.01)
					return false;

				const alpha_t scalar =
					pow_randfloat(float_time * 2);
				const float age_f = (float)(age)/1000000.0f;
				alpha *= scalar;
				velocity.x *= 1.0f / (1.0f + age_f);
				velocity.z *= 1.0f / (1.0f + age_f);
				velocity.y = -age_f * 0.25f;

				break;
			}
			case HarvestingEffect::CAVERN_WALL:
			{
				if (pos.y < -2.0)
					return false;

				break;
			}
			case HarvestingEffect::MOTHER_NATURE:
			{
				if (alpha < 0.01)
					return false;

				const alpha_t scalar =
					pow_randfloat(float_time * 3);
				if (size < 10)
					size /= scalar;
				alpha *= scalar;

				break;
			}
			case HarvestingEffect::QUEEN_OF_NATURE:
			{
				if (alpha < 0.01)
					return false;

				const alpha_t scalar =
					pow_randfloat(float_time);
				alpha *= scalar;

				break;
			}
			case HarvestingEffect::BEES:
			{
				size = 0.5f + randfloat(0.5f);
				const float age_f = (float)(age)/1000000.0f;
				if (age_f < 2.5f)
					break;
				if (2.5f + randfloat(3.75f) < age_f)
					return false;
				break;
			}
			case HarvestingEffect::BAG_OF_GOLD:
			{
				if (age > 220000)
				{
					const alpha_t alpha_scalar =
						std::pow(0.5f, float_time * 6.0f);
					alpha *= alpha_scalar;

					if (alpha < 0.02)
						return false;
				}

				break;
			}
			case HarvestingEffect::RARE_STONE:
			{
				color[0] = 0.7 + 0.3 * std::sin(age / 530000.0);
				color[1] = 0.7 + 0.3 * std::sin(age / 970000.0 + 1.3);
				color[2] = 0.7 + 0.3 * std::sin(age / 780000.0 + 1.9);

				if (age < 700000)
				{
					if (state == 0)
						size = age / 45000.0;
				}
				else
				{
					const percent_t scalar =
						std::pow(0.5f, float_time * 0.5f);
					size *= scalar;
					alpha *= scalar;

					if (alpha < 0.01)
						return false;
				}
				break;
			}
		}

		return true;
	}

#ifdef	NEW_TEXTURES
	Uint32 HarvestingParticle::get_texture()
	{
		return base->get_texture(texture);
	}

	float HarvestingParticle::get_burn() const
	{
		if (((type == HarvestingEffect::RADON_POUCH) && (state == 0)) ||
			(type == HarvestingEffect::MOTHER_NATURE) ||
			(type == HarvestingEffect::QUEEN_OF_NATURE) ||
			((type == HarvestingEffect::BAG_OF_GOLD) && (state == 0)) ||
			(type == HarvestingEffect::RARE_STONE))
		{
			return 1.0f;
		}
		else
		{
			return 0.0f;
		}
	}
#else	/* NEW_TEXTURES */
	GLuint HarvestingParticle::get_texture(const Uint16 res_index)
	{
		return texture->get_texture(res_index);
	}

	void HarvestingParticle::draw(const Uint64 usec)
	{
		if (((type == HarvestingEffect::RADON_POUCH) && (state == 0)) || (type
			== HarvestingEffect::MOTHER_NATURE) || (type
			== HarvestingEffect::QUEEN_OF_NATURE) || ((type
			== HarvestingEffect::BAG_OF_GOLD) && (state == 0)) || (type
			== HarvestingEffect::RARE_STONE))
			Particle::draw(usec);
		else
		{
			glEnable(GL_LIGHTING);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glNormal3f(0.0, 1.0, 0.0);
			Particle::draw(usec);

			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glDisable(GL_LIGHTING);
		}
	}
#endif	/* NEW_TEXTURES */

	light_t HarvestingParticle::get_light_level()
	{
		if (((type == HarvestingEffect::RADON_POUCH) && (state == 0)) || (type
			== HarvestingEffect::MOTHER_NATURE) || (type
			== HarvestingEffect::QUEEN_OF_NATURE) || ((type
			== HarvestingEffect::BAG_OF_GOLD) && (state == 0)) || (type
			== HarvestingEffect::RARE_STONE))
			return alpha * size / 1500;
		else
			return 0.0;
	}
	;

	HarvestingEffect::HarvestingEffect(EyeCandy* _base, bool* _dead,
		Vec3* _pos, const HarvestingType _type, const Uint16 _LOD)
	{
		if (EC_DEBUG)
			std::cout << "HarvestingEffect (" << this << ") created (" << type
				<< ")." << std::endl;
		base = _base;
		dead = _dead;
		pos = _pos;
		effect_center = *pos;
		type = _type;
		LOD = base->last_forced_LOD;
		desired_LOD = _LOD;
		spawner = NULL;
		bounds = NULL;
		mover = NULL;
		spawner2 = NULL;
		mover2 = NULL;
		direction = Vec3(0.0, 0.0, 0.0);

		switch (type)
		{
			case TOOL_BREAK:
			{
				// handled in other constructor
				break;
			}
			case RADON_POUCH:
			{
				effect_center.y += 0.5;
				spawner = new FilledSphereSpawner(0.9);
				mover = new GravityMover(this, &effect_center, 8e9);
				while ((int)particles.size() < LOD * 50)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center;
					Vec3 velocity;
					velocity.randomize();
					velocity.normalize(0.8);
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new HarvestingParticle(this, mover, coords, velocity, 5.25, 0.5, 0.6, 0.7, 0.2, EC_FLARE, LOD, type);
#else	/* NEW_TEXTURES */
							new HarvestingParticle(this, mover, coords, velocity, 5.25, 0.5, 0.6, 0.7, 0.2, &(base->TexFlare), LOD, type);
#endif	/* NEW_TEXTURES */
					p->state = 0;
					if (!base->push_back_particle(p))
						break;
				}
				while ((int)particles.size() < LOD * 100)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center;
					Vec3 velocity;
					velocity.randomize();
					velocity.normalize(1.5);
#ifdef	NEW_TEXTURES
					Particle * p = new HarvestingParticle(this, mover, coords, velocity, 4.5, 0.5 + randalpha(0.4), 0.7, 0.6, 0.5, EC_WATER, LOD, type);
#else	/* NEW_TEXTURES */
					Particle * p = new HarvestingParticle(this, mover, coords, velocity, 4.5, 0.5 + randalpha(0.4), 0.7, 0.6, 0.5, &(base->TexWater), LOD, type);
#endif	/* NEW_TEXTURES */
					p->state = 1;
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case CAVERN_WALL:
			{
				effect_center.y += 15.0;
				spawner = new FilledSphereSpawner(1.0);
				mover = new ParticleMover(this);
				while ((int)particles.size() < LOD * 50)
				{
					Vec3 coords = spawner->get_new_coords();
					coords.y *= 8.0;
					Vec3 velocity;
					velocity.randomize();
					velocity.normalize(0.2);
					velocity.y *= 3.0;
					velocity.y -= 9.0;
					coords += effect_center;
					const color_t scalar= randcolor(0.4);
#ifdef	NEW_TEXTURES
					Particle * p = new HarvestingParticle(this, mover, coords, velocity, 8.0 + randcoord(12.0), 1.0, scalar + randcolor(0.1), scalar + randcolor(0.1), scalar + randcolor(0.1), EC_SIMPLE, LOD, type);
#else	/* NEW_TEXTURES */
					Particle * p = new HarvestingParticle(this, mover, coords, velocity, 8.0 + randcoord(12.0), 1.0, scalar + randcolor(0.1), scalar + randcolor(0.1), scalar + randcolor(0.1), &(base->TexSimple), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				while ((int)particles.size() < LOD * 100)
				{
					Vec3 coords = spawner->get_new_coords();
					coords.y *= 8.0;
					Vec3 velocity;
					velocity.randomize();
					velocity.normalize(0.2);
					velocity.y *= 3.0;
					velocity.y -= 9.0;
					coords += effect_center;
#ifdef	NEW_TEXTURES
					Particle * p = new HarvestingParticle(this, mover, coords, velocity, 3.0 + randcoord(6.0), 0.4 + randalpha(0.4), 0.2 + randcolor(0.2), 0.2 + randcolor(0.2), 0.2 + randcolor(0.2), EC_WATER, LOD, type);
#else	/* NEW_TEXTURES */
					Particle * p = new HarvestingParticle(this, mover, coords, velocity, 3.0 + randcoord(6.0), 0.4 + randalpha(0.4), 0.2 + randcolor(0.2), 0.2 + randcolor(0.2), 0.2 + randcolor(0.2), &(base->TexWater), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case MOTHER_NATURE:
			{
				effect_center.y += 0.2;
				spawner = new HollowDiscSpawner(0.1);
				mover = new SpiralMover(this, &effect_center, 18.0, 11.0);
				while ((int)particles.size() < LOD * 100)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center;
					Vec3 velocity;
					velocity.randomize(0.3);
					velocity.y *= 3;
					velocity.y += 1.4;
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new HarvestingParticle(this, mover, coords, velocity, 3.0, 0.2, 1.0, 0.5 + randcolor(0.5), 0.5, EC_TWINFLARE, LOD, type);
#else	/* NEW_TEXTURES */
							new HarvestingParticle(this, mover, coords, velocity, 3.0, 0.2, 1.0, 0.5 + randcolor(0.5), 0.5, &(base->TexTwinflare), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case QUEEN_OF_NATURE:
			{
				effect_center.y += 0.2;
				spawner = new FilledDiscSpawner(0.5);
				mover = new ParticleMover(this);
				while ((int)particles.size() < LOD * 100)
				{
					Vec3 coords = spawner->get_new_coords() + effect_center;
					coords.y += (coord_t)(randfloat(2.0) * randfloat(2.0) * randfloat(2.0));
					const Vec3 velocity(0.0, 0.0, 0.0);
#ifdef	NEW_TEXTURES
					Particle * p = new HarvestingParticle(this, mover, coords, velocity, 2.0 + randcoord(1.0), 1.0, randcolor(1.0), randcolor(1.0), randcolor(1.0), EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
					Particle * p = new HarvestingParticle(this, mover, coords, velocity, 2.0 + randcoord(1.0), 1.0, randcolor(1.0), randcolor(1.0), randcolor(1.0), &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case BEES:
			{
				spawner = new FilledSphereSpawner(0.75);
				mover = new GravityMover(this, &effect_center, 8e9);
				direction.randomize();
				direction.y = 0;
				while ((int)particles.size() < LOD * 4)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center - direction;
					Vec3 velocity;
					velocity.randomize();
					velocity.normalize(0.75);
					velocity.x += randfloat(direction.x);
					velocity.z += randfloat(direction.z);
#ifdef	NEW_TEXTURES
					Particle * p = new HarvestingParticle(this, mover, coords, velocity, 0.5 + randfloat(0.25), 1.0, 0.9, 0.7, 0.3, EC_TWINFLARE, LOD, type);
#else	/* NEW_TEXTURES */
					Particle * p = new HarvestingParticle(this, mover, coords, velocity, 0.5 + randfloat(0.25), 1.0, 0.9, 0.7, 0.3, &(base->TexTwinflare), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case BAG_OF_GOLD:
			{
				mover = new GravityMover(this, &effect_center, 2e10);
				spawner = new HollowSphereSpawner(0.3);

				for (int i = 0; i < LOD * 60; i++)
				{
					Vec3 coords = spawner->get_new_coords();
					const Vec3 velocity = coords / 10.0;
					coords += effect_center;
#ifdef	NEW_TEXTURES
					Particle* p = new HarvestingParticle(this, mover, coords, velocity, 1.05, 0.75, randcolor(0.3) + 0.7, randcolor(0.3) + 0.5, randcolor(0.3) + 0.3, EC_FLARE, LOD, type);
#else	/* NEW_TEXTURES */
					Particle* p = new HarvestingParticle(this, mover, coords, velocity, 1.05, 0.75, randcolor(0.3) + 0.7, randcolor(0.3) + 0.5, randcolor(0.3) + 0.3, &(base->TexFlare), LOD, type);
#endif	/* NEW_TEXTURES */
					p->state = 1;
					if (!base->push_back_particle(p))
						break;
				}

#ifdef	NEW_TEXTURES
				Particle* p = new HarvestingParticle(this, mover, effect_center, Vec3(0.0, 0.0, 0.0), 8.0, 1.0, 0.8, 0.7, 0.3, EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
				Particle* p = new HarvestingParticle(this, mover, effect_center, Vec3(0.0, 0.0, 0.0), 8.0, 1.0, 0.8, 0.7, 0.3, &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
				base->push_back_particle(p);
				break;
			}
			case RARE_STONE:
			{
				mover = new ParticleMover(this);
				spawner = new HollowSphereSpawner(0.3);

				for (int i = 0; i < LOD * 60; i++)
				{
					Vec3 coords = spawner->get_new_coords();
					const Vec3 velocity = coords / 10.0;
					coords += effect_center;
#ifdef	NEW_TEXTURES
					Particle* p = new HarvestingParticle(this, mover, coords, velocity, 0.75, 0.05, randcolor(0.3) + 0.7, randcolor(0.3) + 0.5, randcolor(0.3) + 0.3, EC_FLARE, LOD, type);
#else	/* NEW_TEXTURES */
					Particle* p = new HarvestingParticle(this, mover, coords, velocity, 0.75, 0.05, randcolor(0.3) + 0.7, randcolor(0.3) + 0.5, randcolor(0.3) + 0.3, &(base->TexFlare), LOD, type);
#endif	/* NEW_TEXTURES */
					p->state = 1;
					if (!base->push_back_particle(p))
						break;
				}

#ifdef	NEW_TEXTURES
				Particle* p = new HarvestingParticle(this, mover, effect_center, Vec3(0.0, 0.0, 0.0), 7.5, 1.0, 1.0, 1.0, 1.0, EC_VOID, LOD, type);
#else	/* NEW_TEXTURES */
				Particle* p = new HarvestingParticle(this, mover, effect_center, Vec3(0.0, 0.0, 0.0), 7.5, 1.0, 1.0, 1.0, 1.0, &(base->TexVoid), LOD, type);
#endif	/* NEW_TEXTURES */
				if (!base->push_back_particle(p))
					break;
#ifdef	NEW_TEXTURES
				p = new HarvestingParticle(this, mover, effect_center, Vec3(0.0, 0.01, 0.0), 7.5, 1.0, 1.0, 1.0, 1.0, EC_VOID, LOD, type);
#else	/* NEW_TEXTURES */
				p = new HarvestingParticle(this, mover, effect_center, Vec3(0.0, 0.01, 0.0), 7.5, 1.0, 1.0, 1.0, 1.0, &(base->TexVoid), LOD, type);
#endif	/* NEW_TEXTURES */
				base->push_back_particle(p);
				break;
			}
		}
	}

	HarvestingEffect::HarvestingEffect(EyeCandy* _base, bool* _dead,
		Vec3* _pos, Vec3* _pos2, const HarvestingType _type, const Uint16 _LOD)
	{
		if (EC_DEBUG)
			std::cout << "HarvestingEffect (" << this << ") created (" << type
				<< ")." << std::endl;
		base = _base;
		dead = _dead;
		pos = _pos;
		pos2 = _pos2;
		effect_center = *pos + (*pos2 - *pos) * 0.5f;
		type = _type;
		LOD = base->last_forced_LOD;
		desired_LOD = _LOD;
		spawner = NULL;
		bounds = NULL;
		mover = NULL;
		spawner2 = NULL;
		mover2 = NULL;
		direction = Vec3(0.0, 0.0, 0.0);

		switch (type)
		{
			case TOOL_BREAK:
			{
				spawner = new FilledSphereSpawner(0.05);
				mover = new ParticleMover(this);
				while ((int)particles.size() < LOD * 32)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center;
					Vec3 velocity;
					velocity.randomize();
					velocity.normalize();
					velocity.y = 0.0;
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new HarvestingParticle(this, mover, coords, velocity, 1.25, 1.0, 0.5, 0.5, 0.5, EC_FLARE, LOD, type);
#else	/* NEW_TEXTURES */
							new HarvestingParticle(this, mover, coords, velocity, 1.25, 1.0, 0.5, 0.5, 0.5, &(base->TexFlare), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			default:
				break;
		}
	}

	HarvestingEffect::~HarvestingEffect()
	{
		if (spawner)
			delete spawner;
		if (mover)
			delete mover;
		if (spawner2)
			delete spawner2;
		if (mover2)
			delete mover2;
		if (EC_DEBUG)
			std::cout << "HarvestingEffect (" << this << ") destroyed."
				<< std::endl;
	}

	bool HarvestingEffect::idle(const Uint64 usec)
	{
		if (particles.size() == 0)
			return false;

		effect_center.x = pos->x;
		effect_center.z = pos->z;

		switch (type)
		{
			case BEES:
			{
				const Uint64 age = get_time() - born;
				const float age_f = (float)(age)/1000000;
				if (age_f > 1.0f)
					direction.y = randfloat(1.25);
				effect_center = *pos + direction * (-1.0f + age_f);
				gravity_center = *pos + direction * (-1.0f + age_f);
				break;
			}
			default:
			{
				effect_center.y += usec / 3000000.0;
				gravity_center.y += usec / 10000000.0;
				break;
			}
		}

		return true;
	}

///////////////////////////////////////////////////////////////////////////////

}
;

