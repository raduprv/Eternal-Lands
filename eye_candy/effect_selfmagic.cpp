// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_selfmagic.h"

namespace ec
{

	// C L A S S   F U N C T I O N S //////////////////////////////////////////////

	SelfMagicParticle::SelfMagicParticle(Effect* _effect,
		ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity,
		const coord_t _size, const alpha_t _alpha, const color_t red,
#ifdef	NEW_TEXTURES
		const color_t green, const color_t blue, TextureEnum _texture,
#else	/* NEW_TEXTURES */
		const color_t green, const color_t blue, Texture* _texture,
#endif	/* NEW_TEXTURES */
		const Uint16 _LOD, const SelfMagicEffect::SelfMagicType _type) :
		Particle(_effect, _mover, _pos, _velocity,
			_size * (0.5 + randcoord()) * 13 / (_LOD + 3))
	{
		type = _type;
		color[0] = red;
		color[1] = green;
		color[2] = blue;
		texture = _texture;
		alpha = _alpha;
		velocity /= size;
		flare_max = 5.0;
		flare_exp = 0.1;
		flare_frequency = 3.0;
		LOD = _LOD;
		state = 0;
	}

	bool SelfMagicParticle::idle(const Uint64 delta_t)
	{
		if (effect->recall)
			return false;

		switch (type)
		{
			case SelfMagicEffect::HEAL:
			{
				const interval_t float_time = delta_t / 1000000.0;

				if (state == 0)
				{
					if ((get_time() - born > 500000)
						|| (pow_randfloat(float_time * 5.0f)) < 0.5)
						state = 1;
				}
				else
				{
					if (alpha < 0.04)
						return false;

					const float scalar = pow_randfloat(float_time * 15.0f);
					energy *= scalar;
					if (size < 10)
						size /= scalar;
					alpha *= scalar;
				}
				break;
			}
			case SelfMagicEffect::MAGIC_PROTECTION:
			{
				const interval_t float_time = delta_t / 1000000.0;

				if (state == 0)
				{
					if ((get_time() - born > 700000)
						|| (pow_randfloat(float_time * 3.0f)) < 0.5)
						state = 1;
				}
				else
				{
					if (alpha < 0.02)
						return false;

					const float scalar = pow_randfloat(float_time * 7.0f);
					energy *= scalar;
					alpha *= scalar;
				}
				break;
			}
			case SelfMagicEffect::HEATSHIELD:
			case SelfMagicEffect::COLDSHIELD:
			case SelfMagicEffect::RADIATIONSHIELD:
			case SelfMagicEffect::SHIELD:
			{
				const interval_t float_time = delta_t / 1000000.0;

				if (state == 0)
				{
					if ((get_time() - born > 700000)
						|| (pow_randfloat(float_time * 3.0f)) < 0.5)
						state = 1;
				}
				else
				{
					if (alpha < 0.02)
						return false;

					const float scalar = std::max(0.0001f,
						pow_randfloat(float_time * 4.5f));
					if (size < 10)
						size /= scalar;
					alpha *= square(scalar);
				}
				break;
			}
			case SelfMagicEffect::RESTORATION:
			{
				const interval_t float_time = delta_t / 1000000.0;

				if (state == 0)
				{
					if ((get_time() - born > 700000)
						|| (pow_randfloat(float_time * 3.0f)) < 0.5)
						state = 1;
				}
				else
				{
					if (alpha < 0.02)
						return false;

					const float scalar = std::max(0.0001f,
						pow_randfloat(float_time * 4.5f));
					if (size < 10)
						size /= scalar;
					alpha *= square(scalar);
				}
				break;
			}
			case SelfMagicEffect::BONES_TO_GOLD:
			{
				const interval_t float_time = delta_t / 1000000.0;

				if (state == 0)
				{
					if ((get_time() - born > 500000)
						|| (pow_randfloat(float_time * 6.0f)) < 0.5)
					{
						for (int i = 0; i < 5; i++)
						{
							Vec3 new_velocity;
							new_velocity.randomize(1.5);
							new_velocity += velocity;
							Particle
								* p =
#ifdef	NEW_TEXTURES
									new SelfMagicParticle(effect, mover, pos, new_velocity, 2.0, 1.0, 1.0, 1.0, 0.5, EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
									new SelfMagicParticle(effect, mover, pos, new_velocity, 2.0, 1.0, 1.0, 1.0, 0.5, &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
							p->state = 1;
							if (!base->push_back_particle(p))
								break;
						}
						return false;
					}
				}
				else
				{
					if (alpha < 0.02)
						return false;

					const float scalar = pow_randfloat(float_time * 10.0f);
					energy *= scalar;
					if (size < 10)
						size /= scalar;
					alpha *= scalar;
				}
				break;
			}
			case SelfMagicEffect::TELEPORT_TO_THE_PORTALS_ROOM:
			{
				if (alpha < 0.005)
					return false;

				const alpha_t scalar = std::pow(0.5f, (float)delta_t
					/ 500000);
				alpha *= scalar;

				break;
			}
			case SelfMagicEffect::MAGIC_IMMUNITY:
			{
				const interval_t float_time = delta_t / 1000000.0;

				if (state == 0)
				{
					if ((get_time() - born > 700000)
						|| (pow_randfloat(float_time * 3.0f)) < 0.5)
						state = 1;
				}
				else
				{
					if (alpha < 0.02)
						return false;

					const float scalar = pow_randfloat(float_time * 7.0f);
					alpha *= scalar;
				}
				break;
			}
			case SelfMagicEffect::ALERT:
			{
				alpha -= delta_t / 5000000.0;

				if (alpha < 0.01)
					return false;

				break;
			}
		}

		pos += ((SelfMagicEffect*)effect)->shift;

		return true;
	}

#ifdef	NEW_TEXTURES
	Uint32 SelfMagicParticle::get_texture()
	{
		return base->get_texture(texture);
	}
#else	/* NEW_TEXTURES */
	GLuint SelfMagicParticle::get_texture(const Uint16 res_index)
	{
		return texture->get_texture(res_index);
	}
#endif	/* NEW_TEXTURES */

	SelfMagicEffect::SelfMagicEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
		const SelfMagicType _type, const Uint16 _LOD)
	{
		if (EC_DEBUG)
			std::cout << "SelfMagicEffect (" << this << ") created (" << _type
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
		target_alpha = NULL;
		shift = Vec3(0.0, 0.0, 0.0);

#ifdef	NEW_TEXTURES
		alpha_scale = 1.0f;
		capless_cylinders = 0;
#endif	/* NEW_TEXTURES */

		switch (type)
		{
			case HEAL:
			{
				//spawner = new SierpinskiIFSParticleSpawner(1.05);
				spawner = new SierpinskiIFSParticleSpawner(1.5);
				mover = new GravityMover(this, &effect_center, 1e10);
				while ((int)particles.size() < LOD * 96)
				{
					Vec3 coords = spawner->get_new_coords() * 0.5;
					Vec3 velocity = -coords * 3;
					coords += effect_center;
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new SelfMagicParticle(this, mover, coords, velocity, 0.7, 0.5, 0.4, 0.7, 0.2, EC_FLARE, LOD, type);
#else	/* NEW_TEXTURES */
							new SelfMagicParticle(this, mover, coords, velocity, 0.7, 0.5, 0.4, 0.7, 0.2, &(base->TexFlare), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case MAGIC_PROTECTION:
			{
				spawner = new HollowSphereSpawner(0.9);
				mover = new GravityMover(this, &effect_center, 3e10);
				while ((int)particles.size() < LOD * 96)
				{
					Vec3 coords = spawner->get_new_coords() + effect_center;
					Vec3 velocity;
					velocity.randomize(0.4);
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new SelfMagicParticle(this, mover, coords, velocity, 1.1, 1.0, 0.7, 0.2, 0.4, EC_VOID, LOD, type);
#else	/* NEW_TEXTURES */
							new SelfMagicParticle(this, mover, coords, velocity, 1.1, 1.0, 0.7, 0.2, 0.4, &(base->TexVoid), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case SHIELD:
			{
				spawner = new HollowDiscSpawner(0.65);
				mover = new SpiralMover(this, &effect_center, 15.0, 14.0);

				while ((int)particles.size() < LOD * 96)
				{
					Vec3 coords = spawner->get_new_coords() + effect_center;
					Vec3 velocity;
					velocity.randomize(0.3);
					velocity.y *= 5;
					velocity.y += 0.7;
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new SelfMagicParticle(this, mover, coords, velocity, 2.0, 1.0, 0.5, 0.5, 0.6, EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
							new SelfMagicParticle(this, mover, coords, velocity, 2.0, 1.0, 0.5, 0.5, 0.6, &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case HEATSHIELD:
			{
				spawner = new SierpinskiIFSParticleSpawner();
				spawner2 = new HollowDiscSpawner(0.65);
				mover = new SpiralMover(this, &effect_center, 10.0, 120.0);
				mover2 = new SpiralMover(this, &effect_center, 15.0, 14.0);

				while ((int)particles.size() < LOD * 48)
				{
					Vec3 coords = spawner->get_new_coords() + effect_center;
					Vec3 velocity;
					velocity.randomize(0.39);
					velocity.y *= 1.5;
					velocity.y += 0.7;
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new SelfMagicParticle(this, mover, coords, velocity, 5.0, 0.9, 1.0, 0.55, 0.05, EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
							new SelfMagicParticle(this, mover, coords, velocity, 5.0, 0.9, 1.0, 0.55, 0.05, &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				while ((int)particles.size() < LOD * 96)
				{
					Vec3 coords = spawner2->get_new_coords() + effect_center;
					Vec3 velocity;
					velocity.randomize(0.39);
					velocity.y *= 1.5;
					velocity.y += 0.7;
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new SelfMagicParticle(this, mover2, coords, velocity, 2.0, 1.0, 1.0, 0.55, 0.05, EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
							new SelfMagicParticle(this, mover2, coords, velocity, 2.0, 1.0, 1.0, 0.55, 0.05, &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case COLDSHIELD:
			{
				spawner = new FilledEllipsoidSpawner(ec::Vec3(2.0, 1.0, 0.1));
				spawner2 = new HollowDiscSpawner(0.65);
				mover = new SpiralMover(this, &effect_center, 5.0, 50.0);
				mover2 = new SpiralMover(this, &effect_center, 15.0, 14.0);

				while ((int)particles.size() < LOD * 36)
				{
					Vec3 coords = spawner->get_new_coords() + effect_center;
					Vec3 velocity;
					velocity.randomize(0.36);
					velocity.y *= 2.5;
					velocity.y -= 0.7;
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new SelfMagicParticle(this, mover, coords, velocity, 8.0, 0.5, 0.05, 0.50, 0.95, EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
							new SelfMagicParticle(this, mover, coords, velocity, 8.0, 0.5, 0.05, 0.50, 0.95, &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				while ((int)particles.size() < LOD * 96)
				{
					Vec3 coords = spawner2->get_new_coords() + effect_center;
					Vec3 velocity;
					velocity.randomize(0.36);
					velocity.y *= 2.5;
					velocity.y -= 0.7;
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new SelfMagicParticle(this, mover2, coords, velocity, 2.0, 1.0, 0.05, 0.50, 0.95, EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
							new SelfMagicParticle(this, mover2, coords, velocity, 2.0, 1.0, 0.05, 0.50, 0.95, &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case RADIATIONSHIELD:
			{
				spawner = new HollowEllipsoidSpawner(Vec3(0.5, 0.9, 0.5));
				spawner2 = new HollowDiscSpawner(0.65);
				mover = new SpiralMover(this, &effect_center, 12.0, 36.0);
				mover2 = new SpiralMover(this, &effect_center, 15.0, 14.0);

				while ((int)particles.size() < LOD * 32)
				{
					Vec3 coords = spawner->get_new_coords() + effect_center;
					Vec3 velocity;
					velocity.randomize(0.33);
					velocity.y *= 4.25;
					velocity.y += 0.3;
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new SelfMagicParticle(this, mover, coords, velocity, 4.0, 0.9, 0.05, 1.0, 0.25, EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
							new SelfMagicParticle(this, mover, coords, velocity, 4.0, 0.9, 0.05, 1.0, 0.25, &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				while ((int)particles.size() < LOD * 96)
				{
					Vec3 coords = spawner2->get_new_coords() + effect_center;
					Vec3 velocity;
					velocity.randomize(0.33);
					velocity.y *= 4.25;
					velocity.y += 0.3;
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new SelfMagicParticle(this, mover2, coords, velocity, 2.0, 1.0, 0.05, 0.8, 0.25, EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
							new SelfMagicParticle(this, mover2, coords, velocity, 2.0, 1.0, 0.05, 0.8, 0.25, &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case RESTORATION:
			{
				spawner = new FilledSphereSpawner(0.8);
				mover = new GravityMover(this, &effect_center, 3e7);
				spawner2 = new HollowDiscSpawner(0.45);
				mover2 = new SpiralMover(this, &effect_center, 10.0, 11.0);
				while ((int)particles.size() < LOD * 48)
				{
					Vec3 coords = spawner->get_new_coords() * 3.5;
					Vec3 velocity = -coords * 3;
					coords += effect_center;
#ifdef	NEW_TEXTURES
					Particle * p = new SelfMagicParticle(this, mover, coords, velocity, 1.9 + randcoord(1.5), 0.85 + randalpha(0.15), 0.25 + randcolor(0.3), 0.7 + randcolor(0.2), 0.3, EC_FLARE, LOD, type);
#else	/* NEW_TEXTURES */
					Particle * p = new SelfMagicParticle(this, mover, coords, velocity, 1.9 + randcoord(1.5), 0.85 + randalpha(0.15), 0.25 + randcolor(0.3), 0.7 + randcolor(0.2), 0.3, &(base->TexFlare), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				while ((int)particles.size() < LOD * 96)
				{
					Vec3 coords = spawner2->get_new_coords() + effect_center;
					Vec3 velocity;
					velocity.randomize(0.3);
					velocity.y *= 5;
					velocity.y -= 0.7;
#ifdef	NEW_TEXTURES
					Particle * p = new SelfMagicParticle(this, mover2, coords, velocity, 1.0 + randcoord(1.5), 0.75 + randalpha(0.25), 0.6 + randcolor(0.3), 0.35 + randcolor(0.45), 0.3, EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
					Particle * p = new SelfMagicParticle(this, mover2, coords, velocity, 1.0 + randcoord(1.5), 0.75 + randalpha(0.25), 0.6 + randcolor(0.3), 0.35 + randcolor(0.45), 0.3, &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case BONES_TO_GOLD:
			{
				count = 0;
				count_scalar = 100000 / _LOD;
				spawner = new FilledDiscSpawner(0.45);
				mover = new SpiralMover(this, &effect_center, 13.0, 12.0);
				Vec3 coords = spawner->get_new_coords() + effect_center;
				Vec3 velocity;
				velocity.randomize(0.4);
				velocity.y += 2.8;
#ifdef	NEW_TEXTURES
				Particle * p = new SelfMagicParticle(this, mover, coords, velocity, 2.0, 1.0, 0.6, 0.5, 0.5, EC_INVERSE, LOD, type);
#else	/* NEW_TEXTURES */
				Particle * p = new SelfMagicParticle(this, mover, coords, velocity, 2.0, 1.0, 0.6, 0.5, 0.5, &(base->TexInverse), LOD, type);
#endif	/* NEW_TEXTURES */
				base->push_back_particle(p);
				break;
			}
			case TELEPORT_TO_THE_PORTALS_ROOM:
			{
				mover = new ParticleMover(this);
				spawner = new FilledDiscSpawner(0.2);
				const float sqrt_LOD= std::sqrt(LOD);
				size_scalar = 1.0;
				for (int i = 0; i < LOD * 96; i++)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center + Vec3(0.0, randcoord() * randcoord() * 8.0 * sqrt_LOD, 0.0);
					Vec3 velocity(0.0, randcoord(0.1), 0.0);
					velocity.randomize(0.25);
					const coord_t size = size_scalar * (0.5 + 1.5 * randcoord());
					velocity /= size;
#ifdef	NEW_TEXTURES
					Particle* p = new SelfMagicParticle(this, mover, coords, velocity, size, 1.0, randcolor(1.0), randcolor(1.0), randcolor(1.0), EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
					Particle* p = new SelfMagicParticle(this, mover, coords, velocity, size, 1.0, randcolor(1.0), randcolor(1.0), randcolor(1.0), &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}

				const float radius = 0.377628;
#ifdef	NEW_TEXTURES
				std::vector<CaplessCylinders::CaplessCylinderItem> cylinders;
#endif	/* NEW_TEXTURES */
				for (int i = 0; i < LOD * 4; i++)
				{
					const percent_t percent = ((percent_t)i + 1) / (LOD * 4);
#ifdef	NEW_TEXTURES
					cylinders.push_back(CaplessCylinders::CaplessCylinderItem(effect_center, effect_center + Vec3(0.0, 10.0 / percent, 0.0), Vec3(1.0, 1.0, 1.0), (0.1 + (1.0 - percent) * 0.05) / (LOD + 2), radius * percent, (int)(25 * (percent + 0.2))));
				}

				if (cylinders.size() > 0)
				{
					capless_cylinders = new CaplessCylinders(base, cylinders);
				}
#else	/* NEW_TEXTURES */
					capless_cylinders.push_back(new CaplessCylinder(base, effect_center, effect_center + Vec3(0.0, 10.0 / percent, 0.0), Vec3(1.0, 1.0, 1.0), (0.1 + (1.0 - percent) * 0.05) / (LOD + 2), radius * percent, (int)(25 * (percent + 0.2))));
				}
#endif	/* NEW_TEXTURES */

				break;
			}
			case MAGIC_IMMUNITY:
			{
				spawner = new FilledSphereSpawner(1.0);
				mover = new GravityMover(this, &effect_center, 4e10);
				while ((int)particles.size() < LOD * 32)
				{
					Vec3 coords = spawner->get_new_coords();
					Vec3 velocity;
					velocity.randomize(2.0);
					coords += effect_center;
#ifdef	NEW_TEXTURES
					Particle * p = new SelfMagicParticle(this, mover, coords, velocity, randcoord(7.0) + 0.3, 1.0, randcolor(), randcolor(), randcolor(), EC_VOID, LOD, type);
#else	/* NEW_TEXTURES */
					Particle * p = new SelfMagicParticle(this, mover, coords, velocity, randcoord(7.0) + 0.3, 1.0, randcolor(), randcolor(), randcolor(), &(base->TexVoid), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case ALERT:
			{
				effect_center.y += 0.5;
				spawner = new HollowEllipsoidSpawner(Vec3(0.2, 0.5, 0.2));
				mover = new ParticleMover(this);
				while ((int)particles.size() < LOD * 32)
				{
					Vec3 coords = spawner->get_new_coords();
					Vec3 velocity = coords * 16;
					coords += effect_center;
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new SelfMagicParticle(this, mover, coords, velocity, 7.0, 0.12, 1.0, 1.0, 1.0, EC_VOID, LOD, type);
#else	/* NEW_TEXTURES */
							new SelfMagicParticle(this, mover, coords, velocity, 7.0, 0.12, 1.0, 1.0, 1.0, &(base->TexVoid), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
		}
	}

	SelfMagicEffect::~SelfMagicEffect()
	{
		if (spawner)
			delete spawner;
		if (mover)
			delete mover;
		if (spawner2)
			delete spawner2;
		if (mover2)
			delete mover2;
#ifdef	NEW_TEXTURES
		delete capless_cylinders;
#else	/* NEW_TEXTURES */
		for (size_t i = 0; i < capless_cylinders.size(); i++)
			delete capless_cylinders[i];
		capless_cylinders.clear();
#endif	/* NEW_TEXTURES */
		if (EC_DEBUG)
			std::cout << "SelfMagicEffect (" << this << ") destroyed."
				<< std::endl;
	}

	bool SelfMagicEffect::idle(const Uint64 usec)
	{
		if (particles.size() == 0)
		{
#ifdef	NEW_TEXTURES
			if (capless_cylinders != 0)
			{
				if (alpha_scale < 0.01f)
				{
					return false;
				}
			}
			else
			{
				return false;
			}
#else	/* NEW_TEXTURES */
			if (capless_cylinders.size())
			{
				if ((*capless_cylinders.rbegin())->alpha < 0.01)
					return false;
			}
			else
				return false;
#endif	/* NEW_TEXTURES */
		}
		else if (recall)
			return true;

		const Vec3 last_effect_center = effect_center;

		effect_center.x = pos->x;
		effect_center.z = pos->z;

		shift = effect_center - last_effect_center;

		effect_center.y += usec / 1500000.0;

		const Uint64 cur_time = get_time();
		const Uint64 age = cur_time - born;
		switch (type)
		{
			case HEAL:
			{
				break;
			}
			case MAGIC_PROTECTION:
			{
				break;
			}
			case SHIELD:
			case HEATSHIELD:
			case COLDSHIELD:
			case RADIATIONSHIELD:
			{
				break;
			}
			case RESTORATION:
			{
				break;
			}
			case BONES_TO_GOLD:
			{
				if (age < 750000)
				{
					count += usec;

					while (count > 0)
					{
						Vec3 coords = spawner->get_new_coords() + effect_center;
						Vec3 velocity;
						velocity.randomize(0.4);
						velocity.y += 2.8;
						Particle
							* p =
#ifdef	NEW_TEXTURES
								new SelfMagicParticle(this, mover, coords, velocity, 2.0, 1.0, 0.6, 0.5, 0.5, EC_INVERSE, LOD, type);
#else	/* NEW_TEXTURES */
								new SelfMagicParticle(this, mover, coords, velocity, 2.0, 1.0, 0.6, 0.5, 0.5, &(base->TexInverse), LOD, type);
#endif	/* NEW_TEXTURES */
						if (!base->push_back_particle(p))
						{
							count = 0;
							break;
						}
						count -= count_scalar;
					}
				}
				break;
			}
			case TELEPORT_TO_THE_PORTALS_ROOM:
			{
				if (age > 500000)
				{
#ifdef	NEW_TEXTURES
					alpha_scale *= std::pow(0.5f, (interval_t)usec / 200000.0f);
#else	/* NEW_TEXTURES */
					const alpha_t scalar =
						std::pow(0.5f, (interval_t)usec / 200000.0f);
					for (int i = 0; i < (int)capless_cylinders.size(); i++)
					{
						float percent = float(i) / float(capless_cylinders.size()) * 0.8;
						std::vector<Shape*>::const_iterator iter =
							capless_cylinders.begin() + i;
						(*iter)->alpha = (*iter)->alpha * percent + (*iter)->alpha * scalar * (1.0 - percent);
					}
					/*
					 for (std::vector<Shape*>::iterator iter = capless_cylinders.begin(); iter != capless_cylinders.end(); iter++)
					 {
					 (*iter)->alpha *= scalar;
					 //          std::coutk << (*iter)->alpha << scalar << ((*iter)->alpha * scalar) << std::endl;
					 if ((*iter)->alpha > 0.005)
					 break;
					 }
					 */
#endif	/* NEW_TEXTURES */
				}

				if (target_alpha)
				{
					if (age < 500000)
					{
						*target_alpha = 1.0 - (age / 500000.0);
					}
					else if (age < 1000000)
					{
						*target_alpha = (age - 500000) / 500000.0;
					}
					else
					{
						*target_alpha = 1.0;
					}
				}

				break;
			}
			case MAGIC_IMMUNITY:
			{
				break;
			}
			case ALERT:
			{
				break;
			}
		}

		return true;
	}

	void SelfMagicEffect::draw(const Uint64 usec)
	{
#ifdef	NEW_TEXTURES
		if (capless_cylinders != 0)
		{
			capless_cylinders->draw(alpha_scale);
		}
#else	/* NEW_TEXTURES */
		for (std::vector<Shape*>::iterator iter = capless_cylinders.begin(); iter
			!= capless_cylinders.end(); iter++)
			(*iter)->draw();
#endif	/* NEW_TEXTURES */
	}

///////////////////////////////////////////////////////////////////////////////

}
;

