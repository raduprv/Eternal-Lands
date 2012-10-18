// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_breath.h"

namespace ec
{

	// C L A S S   F U N C T I O N S //////////////////////////////////////////////

	BreathParticle::BreathParticle(Effect* _effect, ParticleMover* _mover,
		const Vec3 _pos, const Vec3 _velocity, const coord_t _size,
		const alpha_t _alpha, const color_t red, const color_t green,
#ifdef	NEW_TEXTURES
		const color_t blue, TextureEnum _texture, const Uint16 _LOD,
#else	/* NEW_TEXTURES */
		const color_t blue, Texture* _texture, const Uint16 _LOD,
#endif	/* NEW_TEXTURES */
		const BreathEffect::BreathType _type) :
		Particle(_effect, _mover, _pos, _velocity,
			_size * (0.2 + randcoord()) * 15 / _LOD)
	{
		type = _type;
		color[0] = red;
		color[1] = green;
		color[2] = blue;
		texture = _texture;
		//  alpha = _alpha;
		alpha = _alpha * 2 / size;
		flare_max = 8.0;
		flare_exp = 0.5;
		flare_frequency = 3.0;
		LOD = _LOD;
		state = 0;
	}

	bool BreathParticle::idle(const Uint64 delta_t)
	{
		if (effect->recall)
			return false;

		const interval_t float_time = delta_t / 1000000.0f;
		velocity *= std::pow(0.5f, float_time * velocity.magnitude()
			/ 8.0f);

		switch (type)
		{
			case BreathEffect::ICE:
			case BreathEffect::POISON:
			case BreathEffect::MAGIC:
			case BreathEffect::FIRE:
			{
				if (state == 0)
				{
					if ((get_time() - born
						> (type == BreathEffect::POISON ? 100000 : 400000))
						|| (pow_randfloat(float_time * 5.0f)) < 0.5)
						state = 1;
				}
				else
				{
					if (alpha < 0.02)
						return false;

					if ((state == 1) && (alpha < 0.05))
					{
						state = 2;
						if (type == BreathEffect::FIRE)
						{
							Vec3 velocity_offset;
							velocity_offset.randomize(0.2);
							base->push_back_particle(new BreathSmokeParticle(effect, mover, pos, velocity + velocity_offset, size * 1.25, 0.06, texture, LOD, type));
						}
					}

					const alpha_t scalar = pow_randfloat(float_time * 5.0f);
					alpha *= scalar;

					const coord_t size_scalar =
						std::pow(0.5f, (float)delta_t / 1500000);
					size = std::min(3.0f, size / size_scalar * 0.125f + size * 0.5f);
				}
				break;
			}
			case BreathEffect::WIND:
			{
				if (state == 0)
				{
					if ((get_time() - born > 400000)
						|| (pow_randfloat(float_time * 70.0f)) < 0.5)
						state = 1;
				}
				else
				{
					if (alpha < 0.02)
						return false;

					if ((state == 1) && (alpha < 0.04))
					{
						state = 2;
						if ((rand() & 0x07) == 0x07)
						{
							Vec3 velocity_offset;
							velocity_offset.randomize(1.0);
							base->push_back_particle(new BreathSmokeParticle(effect, mover, pos, velocity + velocity_offset, size * 5.0, 1.0, texture, LOD, type));
						}
					}

					const alpha_t scalar = pow_randfloat(float_time * 20.0f);
					alpha *= scalar;

					const coord_t size_scalar =
						std::pow(0.5f, (float)delta_t / 1500000);
					size = std::min(3.0f, size / size_scalar * 0.125f + size * 0.5f);
				}
				break;
			}
			case BreathEffect::LIGHTNING:
			{
				if (state == 0)
				{
					if ((get_time() - born > 400000)
						|| (pow_randfloat(float_time * 10.0f)) < 0.5)
						state = 1;
				}
				else
				{
					if (alpha < 0.02)
						return false;

					if ((state == 1) && (alpha < 0.04))
					{
						state = 2;
						if ((rand() & 0x3F) == 0x3F)
						{
							Vec3 velocity_offset;
							velocity_offset.randomize();
							velocity_offset.normalize();
							Particle
								* p =
#ifdef	NEW_TEXTURES
									new BreathParticle(effect, mover, pos, velocity_offset * 3.6, size / 2, 1.0, color[0], color[1], color[2], EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
									new BreathParticle(effect, mover, pos, velocity_offset * 3.6, size / 2, 1.0, color[0], color[1], color[2], &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
							p->state = 2;
							base->push_back_particle(p);
							p
#ifdef	NEW_TEXTURES
								= new BreathParticle(effect, mover, pos, velocity_offset * 3.4, size / 2, 1.0, color[0], color[1], color[2], EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
								= new BreathParticle(effect, mover, pos, velocity_offset * 3.4, size / 2, 1.0, color[0], color[1], color[2], &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
							p->state = 2;
							base->push_back_particle(p);
							p
#ifdef	NEW_TEXTURES
								= new BreathParticle(effect, mover, pos, velocity_offset * 3.2, size / 2, 1.0, color[0], color[1], color[2], EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
								= new BreathParticle(effect, mover, pos, velocity_offset * 3.2, size / 2, 1.0, color[0], color[1], color[2], &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
							p->state = 2;
							base->push_back_particle(p);
							p
#ifdef	NEW_TEXTURES
								= new BreathParticle(effect, mover, pos, velocity_offset * 3.0, size / 2, 1.0, color[0], color[1], color[2], EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
								= new BreathParticle(effect, mover, pos, velocity_offset * 3.0, size / 2, 1.0, color[0], color[1], color[2], &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
							p->state = 2;
							base->push_back_particle(p);
							p
#ifdef	NEW_TEXTURES
								= new BreathParticle(effect, mover, pos, velocity_offset * 2.8, size / 2, 1.0, color[0], color[1], color[2], EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
								= new BreathParticle(effect, mover, pos, velocity_offset * 2.8, size / 2, 1.0, color[0], color[1], color[2], &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
							p->state = 2;
							base->push_back_particle(p);
							p
#ifdef	NEW_TEXTURES
								= new BreathParticle(effect, mover, pos, velocity_offset * 2.6, size / 2, 1.0, color[0], color[1], color[2], EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
								= new BreathParticle(effect, mover, pos, velocity_offset * 2.6, size / 2, 1.0, color[0], color[1], color[2], &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
							p->state = 2;
							base->push_back_particle(p);
							p
#ifdef	NEW_TEXTURES
								= new BreathParticle(effect, mover, pos, velocity_offset * 2.4, size / 2, 1.0, color[0], color[1], color[2], EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
								= new BreathParticle(effect, mover, pos, velocity_offset * 2.4, size / 2, 1.0, color[0], color[1], color[2], &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
							p->state = 2;
							base->push_back_particle(p);
							p
#ifdef	NEW_TEXTURES
								= new BreathParticle(effect, mover, pos, velocity_offset * 2.2, size / 2, 1.0, color[0], color[1], color[2], EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
								= new BreathParticle(effect, mover, pos, velocity_offset * 2.2, size / 2, 1.0, color[0], color[1], color[2], &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
							p->state = 2;
							base->push_back_particle(p);
							p
#ifdef	NEW_TEXTURES
								= new BreathParticle(effect, mover, pos, velocity_offset * 2.0, size / 2, 1.0, color[0], color[1], color[2], EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
								= new BreathParticle(effect, mover, pos, velocity_offset * 2.0, size / 2, 1.0, color[0], color[1], color[2], &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
							p->state = 2;
							base->push_back_particle(p);
							p
#ifdef	NEW_TEXTURES
								= new BreathParticle(effect, mover, pos, velocity_offset * 1.8, size / 2, 1.0, color[0], color[1], color[2], EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
								= new BreathParticle(effect, mover, pos, velocity_offset * 1.8, size / 2, 1.0, color[0], color[1], color[2], &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
							p->state = 2;
							base->push_back_particle(p);
							p
#ifdef	NEW_TEXTURES
								= new BreathParticle(effect, mover, pos, velocity_offset * 1.6, size / 2, 1.0, color[0], color[1], color[2], EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
								= new BreathParticle(effect, mover, pos, velocity_offset * 1.6, size / 2, 1.0, color[0], color[1], color[2], &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
							p->state = 2;
							base->push_back_particle(p);
							p
#ifdef	NEW_TEXTURES
								= new BreathParticle(effect, mover, pos, velocity_offset * 1.4, size / 2, 1.0, color[0], color[1], color[2], EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
								= new BreathParticle(effect, mover, pos, velocity_offset * 1.4, size / 2, 1.0, color[0], color[1], color[2], &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
							p->state = 2;
							base->push_back_particle(p);
						}
					}

					const alpha_t scalar = pow_randfloat(float_time * 5.0f);
					alpha *= scalar;
					if (state == 2)
						alpha *= square(scalar);

					const coord_t size_scalar =
						std::pow(0.5f, (float)delta_t / 1500000);
					size = std::min(2.0f, size / size_scalar * 0.125f + size * 0.5f);
				}
				break;
			}
		}

		return true;
	}

#ifdef	NEW_TEXTURES
	Uint32 BreathParticle::get_texture()
	{
		return base->get_texture(texture);
	}
#else	/* NEW_TEXTURES */
	GLuint BreathParticle::get_texture(const Uint16 res_index)
	{
		return texture->get_texture(res_index);
	}
#endif	/* NEW_TEXTURES */

	BreathSmokeParticle::BreathSmokeParticle(Effect* _effect,
		ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity,
#ifdef	NEW_TEXTURES
		const coord_t _size, const alpha_t _alpha, TextureEnum _texture,
#else	/* NEW_TEXTURES */
		const coord_t _size, const alpha_t _alpha, Texture* _texture,
#endif	/* NEW_TEXTURES */
		const Uint16 _LOD, const BreathEffect::BreathType _type) :
		Particle(_effect, _mover, _pos, _velocity)
	{
		texture = _texture;
		type = _type;
		switch (type)
		{
			case BreathEffect::FIRE:
			{
				const color_t color_scale= randcolor(0.1);
				color[0] = randcolor(0.1) + color_scale;
				color[1] = randcolor(0.1) + color_scale;
				color[2] = randcolor(0.1) + color_scale;
				break;
			}
			case BreathEffect::ICE:
			{
				const color_t color_scale= randcolor(0.2);
				color[0] = 0.3 + randcolor(0.1) + color_scale;
				color[1] = 0.3 + randcolor(0.1) + color_scale;
				color[2] = 1.0;
				break;
			}
			case BreathEffect::POISON:
			{
				const color_t color_scale= randcolor(0.1);
				color[0] = randcolor(0.0) + color_scale;
				color[1] = randcolor(0.0) + color_scale;
				color[2] = randcolor(0.0) + color_scale;
#ifdef	NEW_TEXTURES
				texture = EC_SIMPLE;
#else	/* NEW_TEXTURES */
				texture = &(base->TexSimple);
#endif	/* NEW_TEXTURES */
				break;
			}
			case BreathEffect::MAGIC:
			{
				color[0] = randcolor(0.35);
				color[1] = randcolor(0.35);
				color[2] = randcolor(0.35);
				break;
			}
			case BreathEffect::LIGHTNING: // Impossible; lightning doesn't use smoke.
			{
				break;
			}
			case BreathEffect::WIND:
			{
				const color_t color_scale= randcolor(0.4);
				color[0] = randcolor(0.1) + color_scale;
				color[1] = randcolor(0.1) + color_scale;
				color[2] = randcolor(0.1) + color_scale;
				break;
			}
		}
		size = _size * (0.5 + randcoord()) * 10 / _LOD;
		alpha = _alpha;
		flare_max = 1.0;
		flare_exp = 1.0;
		flare_frequency = 1.0;
		state = 0;
	}

#ifdef	NEW_TEXTURES
	float BreathSmokeParticle::get_burn() const
	{
		return 0.0f;
	}
#else	/* NEW_TEXTURES */
	void BreathSmokeParticle::draw(const Uint64 usec)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		Particle::draw(usec);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}
#endif	/* NEW_TEXTURES */

	bool BreathSmokeParticle::idle(const Uint64 delta_t)
	{
		if (effect->recall)
			return false;

		const interval_t float_time = delta_t / 1000000.0;
		velocity *= std::pow(0.5f, float_time * velocity.magnitude()
			/ 2.0f);

		if (state == 0)
		{
			alpha += float_time * 2.0;

			if (alpha >= 0.4)
			{
				alpha = 0.4;
				state = 1;
			}
		}
		else
		{
			const alpha_t alpha_scalar = 1.0
				- std::pow(0.5f, (float)delta_t / 1000000);
			alpha -= alpha_scalar * 0.25;

			if (alpha < 0.005)
				return false;

			const coord_t size_scalar = std::pow(0.5f, (float)delta_t
				/ 1500000);
			size = std::min(3.0f, size / size_scalar * 0.125f + size * 0.5f);
		}

		return true;
	}

#ifdef	NEW_TEXTURES
	Uint32 BreathSmokeParticle::get_texture()
	{
		return base->get_texture(texture);
	}
#else	/* NEW_TEXTURES */
	GLuint BreathSmokeParticle::get_texture(const Uint16 res_index)
	{
		return texture->get_texture(res_index);
	}
#endif	/* NEW_TEXTURES */

	BreathEffect::BreathEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
		Vec3* _target, std::vector<ec::Obstruction*>* _obstructions,
		const BreathType _type, const Uint16 _LOD, const percent_t _scale)
	{
		if (EC_DEBUG)
			std::cout << "BreathEffect (" << this << ") created." << std::endl;
		base = _base;
		dead = _dead;
		pos = _pos;
		target = _target;
		obstructions = _obstructions;
		type = _type;
		LOD = base->last_forced_LOD;
		desired_LOD = _LOD;
		scale = _scale;
		bounds = NULL;
		spawner = NULL;
		mover = NULL;
		count = 0;
		count_scalar = 3000 / LOD;
		size_scalar = scale * std::sqrt(LOD) / sqrt(10.0);

		spawner = new FilledSphereSpawner(scale / 3.0);
		mover = new SmokeMover(this, 10.0);
		while ((int)particles.size() < LOD * 64)
		{
			const Vec3 coords = spawner->get_new_coords() + *pos;
			Vec3 velocity;
			velocity.randomize(0.5);
			velocity += (*target - *pos) * 2.0;
			switch (type)
			{
				case FIRE:
				{
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new BreathParticle(this, mover, coords, velocity, 0.5 * size_scalar, 0.6, 0.8 + randcolor(0.2), 0.4 + randcolor(0.4), randcolor(0.4), EC_FLARE, LOD, type);
#else	/* NEW_TEXTURES */
							new BreathParticle(this, mover, coords, velocity, 0.5 * size_scalar, 0.6, 0.8 + randcolor(0.2), 0.4 + randcolor(0.4), randcolor(0.4), &(base->TexFlare), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						return;
					break;
				}
				case ICE:
				{
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new BreathParticle(this, mover, coords, velocity, 0.5 * size_scalar, 0.6, randcolor(0.4), 0.4 + randcolor(0.4), 0.8 + randcolor(0.2), EC_CRYSTAL, LOD, type);
#else	/* NEW_TEXTURES */
							new BreathParticle(this, mover, coords, velocity, 0.5 * size_scalar, 0.6, randcolor(0.4), 0.4 + randcolor(0.4), 0.8 + randcolor(0.2), &(base->TexCrystal), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						return;
					break;
				}
				case POISON:
				{
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new BreathParticle(this, mover, coords, velocity, 0.5 * size_scalar, 0.1, randcolor(0.3), 0.8 + randcolor(0.2), randcolor(0.3), EC_WATER, LOD, type);
#else	/* NEW_TEXTURES */
							new BreathParticle(this, mover, coords, velocity, 0.5 * size_scalar, 0.1, randcolor(0.3), 0.8 + randcolor(0.2), randcolor(0.3), &(base->TexWater), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						return;
					break;
				}
				case MAGIC:
				{
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new BreathParticle(this, mover, coords, velocity, 0.5 * size_scalar, 0.6, randcolor(1.0), randcolor(1.0), randcolor(1.0), EC_TWINFLARE, LOD, type);
#else	/* NEW_TEXTURES */
							new BreathParticle(this, mover, coords, velocity, 0.5 * size_scalar, 0.6, randcolor(1.0), randcolor(1.0), randcolor(1.0), &(base->TexTwinflare), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						return;
					break;
				}
				case LIGHTNING:
				{
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new BreathParticle(this, mover, coords, velocity, 0.25 * size_scalar, 0.6, 0.9 + randcolor(0.1), 0.85 + randcolor(0.15), 0.8 + randcolor(0.2), EC_FLARE, LOD, type);
#else	/* NEW_TEXTURES */
							new BreathParticle(this, mover, coords, velocity, 0.25 * size_scalar, 0.6, 0.9 + randcolor(0.1), 0.85 + randcolor(0.15), 0.8 + randcolor(0.2), &(base->TexFlare), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						return;
					break;
				}
				case WIND:
				{
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new BreathParticle(this, mover, coords, velocity, 0.5 * size_scalar, 0.6, 0.8 + randcolor(0.2), 0.8 + randcolor(0.2), 0.8 + randcolor(0.2), EC_FLARE, LOD, type);
#else	/* NEW_TEXTURES */
							new BreathParticle(this, mover, coords, velocity, 0.5 * size_scalar, 0.6, 0.8 + randcolor(0.2), 0.8 + randcolor(0.2), 0.8 + randcolor(0.2), &(base->TexFlare), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						return;
					break;
				}
			}
		}
	}

	BreathEffect::~BreathEffect()
	{
		if (spawner)
			delete spawner;
		if (mover)
			delete mover;
		if (EC_DEBUG)
			std::cout << "BreathEffect (" << this << ") destroyed."
				<< std::endl;
	}

	bool BreathEffect::idle(const Uint64 usec)
	{
		if (particles.size() == 0)
			return false;

		if (recall)
			return true;

		const Uint64 cur_time = get_time();
		const Uint64 age = cur_time - born;
		if (age > 1100000)
			return true;

		count += usec;

		while (count > 0)
		{
			count -= count_scalar * (14 - LOD);

			Vec3 coords;
			Vec3 velocity;
			if (type == WIND)
			{
				coords = spawner->get_new_coords() * 2 + *pos;
				velocity.randomize(1.0 * scale);
				velocity += (*target - *pos) * 10.0;
			}
			else
			{
				coords = spawner->get_new_coords() + *pos;
				velocity.randomize(0.5 * scale);
				velocity += (*target - *pos) * 2.0;
			}

			switch (type)
			{
				case FIRE:
				{
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new BreathParticle(this, mover, coords, velocity, 0.5 * size_scalar, 0.6, 0.8 + randcolor(0.2), 0.4 + randcolor(0.4), randcolor(0.4), EC_FLARE, LOD, type);
#else	/* NEW_TEXTURES */
							new BreathParticle(this, mover, coords, velocity, 0.5 * size_scalar, 0.6, 0.8 + randcolor(0.2), 0.4 + randcolor(0.4), randcolor(0.4), &(base->TexFlare), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						return true;
					break;
				}
				case ICE:
				{
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new BreathParticle(this, mover, coords, velocity, 0.5 * size_scalar, 0.6, randcolor(0.4), 0.4 + randcolor(0.4), 0.8 + randcolor(0.2), EC_CRYSTAL, LOD, type);
#else	/* NEW_TEXTURES */
							new BreathParticle(this, mover, coords, velocity, 0.5 * size_scalar, 0.6, randcolor(0.4), 0.4 + randcolor(0.4), 0.8 + randcolor(0.2), &(base->TexCrystal), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						return true;
					break;
				}
				case POISON:
				{
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new BreathParticle(this, mover, coords, velocity, 0.5 * size_scalar, 0.1, 0.9 + randcolor(0.1), 0.8 + randcolor(0.2), 0.6 + randcolor(0.2), EC_FLARE, LOD, type);
#else	/* NEW_TEXTURES */
							new BreathParticle(this, mover, coords, velocity, 0.5 * size_scalar, 0.1, 0.9 + randcolor(0.1), 0.8 + randcolor(0.2), 0.6 + randcolor(0.2), &(base->TexFlare), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						return true;
					break;
				}
				case MAGIC:
				{
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new BreathParticle(this, mover, coords, velocity, 0.5 * size_scalar, 0.6, randcolor(1.0), randcolor(1.0), randcolor(1.0), EC_TWINFLARE, LOD, type);
#else	/* NEW_TEXTURES */
							new BreathParticle(this, mover, coords, velocity, 0.5 * size_scalar, 0.6, randcolor(1.0), randcolor(1.0), randcolor(1.0), &(base->TexTwinflare), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						return true;
					break;
				}
				case LIGHTNING:
				{
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new BreathParticle(this, mover, coords, velocity, 0.25 * size_scalar, 0.6, 0.9 + randcolor(0.1), 0.85 + randcolor(0.15), 0.8 + randcolor(0.2), EC_FLARE, LOD, type);
#else	/* NEW_TEXTURES */
							new BreathParticle(this, mover, coords, velocity, 0.25 * size_scalar, 0.6, 0.9 + randcolor(0.1), 0.85 + randcolor(0.15), 0.8 + randcolor(0.2), &(base->TexFlare), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						return true;
					break;
				}
				case WIND:
				{
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new BreathParticle(this, mover, coords, velocity, 0.5 * size_scalar, 0.1, 0.8 + randcolor(0.2), 0.8 + randcolor(0.2), 0.8 + randcolor(0.2), EC_FLARE, LOD, type);
#else	/* NEW_TEXTURES */
							new BreathParticle(this, mover, coords, velocity, 0.5 * size_scalar, 0.1, 0.8 + randcolor(0.2), 0.8 + randcolor(0.2), 0.8 + randcolor(0.2), &(base->TexFlare), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						return true;
					break;
				}
			}
		}

		return true;
	}

///////////////////////////////////////////////////////////////////////////////

}
;

