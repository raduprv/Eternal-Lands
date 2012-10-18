// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_glow.h"

namespace ec
{

	// C L A S S   F U N C T I O N S //////////////////////////////////////////////

	GlowParticle::GlowParticle(Effect* _effect, ParticleMover* _mover,
		const Vec3 _pos, const Vec3 _velocity, const coord_t _size,
		const alpha_t _alpha, const color_t red, const color_t green,
#ifdef	NEW_TEXTURES
		const color_t blue, TextureEnum _texture, const Uint16 _LOD,
#else	/* NEW_TEXTURES */
		const color_t blue, Texture* _texture, const Uint16 _LOD,
#endif	/* NEW_TEXTURES */
		const GlowEffect::GlowType _type) :
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
		pos = _pos;
	}

	bool GlowParticle::idle(const Uint64 delta_t)
	{
		if (effect->recall)
			return false;

		const interval_t float_time = delta_t / 1000000.0;
		const Uint64 age = get_time() - born;
		switch (type)
		{
			case GlowEffect::REMOTE_HEAL_GLOW:
			{
				color[0] = 0.4 + randcolor(0.15) * std::sin(age / 530000.0f);
				color[1] = 0.7;
				color[2] = 0.2 + 0.15 * std::sin(age / 780000.0f + 1.9f);

				const percent_t scalar = std::pow(0.5f, float_time
					* 0.5f);
				const float age_f = (float)(age)/1000000.0f;
				size = 32.0f * age_f / std::exp(1.5f * age_f);
				alpha *= scalar;

				if (alpha < 0.01)
					return false;
				break;
			}
			case GlowEffect::HARM_GLOW:
			{
				color[0] = 1.0;
				color[1] = 0.4 + randcolor(0.15) * std::sin(age / 530000.0f);
				color[2] = 0.2 + 0.15 * std::sin(age / 780000.0f + 1.9f);

				const percent_t scalar = std::pow(0.5f, float_time
					* 0.5f);
				const float age_f = (float)(age)/1000000.0f;
				size = 32.0f * age_f / std::exp(1.5f * age_f);
				alpha *= scalar;

				if (alpha < 0.01)
					return false;
				break;
			}
			case GlowEffect::POISON_GLOW:
			{
				color[0] = randcolor(0.3);
				color[1] = 0.5 + randcolor(0.3);
				color[2] = randcolor(0.5);

				const percent_t scalar = std::pow(0.5f, float_time
					* 0.5f);
				const float age_f = (float)(age)/1000000.0f;
				size = 32.0f * age_f / std::exp(1.5f * age_f);
				alpha *= scalar;

				if (alpha < 0.01)
					return false;
				break;
			}
			case GlowEffect::LEVEL_UP_DEFAULT_GLOW:
			{
				const Uint64 age = get_time() - born;
				pos.y += float_time * (1.0 - (float)age * 0.000001 * (float)age
					* 0.000001);
				if (age < 950000)
					break;
				if (alpha < 0.01)
					return false;
				const alpha_t scalar =
					pow_randfloat(float_time * 1.0f); // smaller numbers -> longer effect
				alpha *= scalar;
				break;
			}
			case GlowEffect::LEVEL_UP_OA_GLOW:
			case GlowEffect::LEVEL_UP_ATT_GLOW:
			case GlowEffect::LEVEL_UP_DEF_GLOW:
			{
				const Uint64 age = get_time() - born;
				if (age < 950000)
					break;
				if (alpha < 0.01)
					return false;
				const alpha_t scalar =
					pow_randfloat(float_time * 1.0f); // smaller numbers -> longer effect
				alpha *= scalar;
				break;
			}
			case GlowEffect::LEVEL_UP_HAR_GLOW:
			{
				if (alpha < 0.01)
					return false;

				const alpha_t scalar =
					pow_randfloat(float_time);
				alpha *= scalar;

				velocity.y -= ((delta_t / 250000.0) * (delta_t / 250000.0)
					+ randfloat(0.125)); // let particles drop

				break;
			}
			case GlowEffect::LEVEL_UP_ALC_GLOW_L:
			case GlowEffect::LEVEL_UP_ALC_GLOW_R:
			case GlowEffect::LEVEL_UP_POT_GLOW_L:
			case GlowEffect::LEVEL_UP_POT_GLOW_R:
			case GlowEffect::LEVEL_UP_MAN_GLOW_L:
			case GlowEffect::LEVEL_UP_MAN_GLOW_R:
			case GlowEffect::LEVEL_UP_CRA_GLOW_L:
			case GlowEffect::LEVEL_UP_CRA_GLOW_R:
			case GlowEffect::LEVEL_UP_ENG_GLOW_L:
			case GlowEffect::LEVEL_UP_ENG_GLOW_R:
			case GlowEffect::LEVEL_UP_TAI_GLOW_L:
			case GlowEffect::LEVEL_UP_TAI_GLOW_R:
			{
				velocity.x *= 0.025 * float_time;
				velocity.z *= 0.025 * float_time;
				velocity.y += float_time;
				const alpha_t scalar = 1.0
					- pow_randfloat(float_time * 0.75f);
				alpha -= scalar;
				if (alpha < 0.01)
					return false;
				break;
			}
			case GlowEffect::LEVEL_UP_MAG_GLOW:
			{
				alpha *= pow_randfloat(delta_t / 1500000.0f); // increase this number to make particles live longer
				if (alpha < 0.01)
					return false;
				const Vec3 velshift = (*(effect->pos) - pos).normalize(20.0) * float_time;
				velocity += velshift;
				if (velocity.magnitude() > 7.5)
				{
					velocity.normalize(7.5);
				}
				break;
			}
			default:
			{
				alpha *= pow_randfloat(delta_t / 1000000.0f); // increase this number to make particles live longer
				if (alpha < 0.01)
					return false;
				break;
			}
		}

		// if the effect moved, shift the particle
		pos += ((GlowEffect*)effect)->shift;

		// rotate particle around effect's y achsis
		switch (type)
		{
			case GlowEffect::LEVEL_UP_OA_GLOW:
			{
				// relative position of the particle to the effect center
				Vec3 relpos;
				relpos.y = 0;
				relpos.x = pos.x - ((GlowEffect*)effect)->pos->x;
				relpos.z = pos.z - ((GlowEffect*)effect)->pos->z;

				// relative position to rotate
				Vec3 rotrelpos = relpos;
				const angle_t angle= M_PI * float_time;
				// rotate it around y achsis
				rotrelpos.x = relpos.x * cos(angle) + relpos.z * sin(angle);
				rotrelpos.z = -relpos.x * sin(angle) + relpos.z * cos(angle);

				// move particle
				pos += (relpos - rotrelpos);
				if (relpos.magnitude() > 1.25)
				{
					for (int i = 0; i < 32; i++)
					{
						relpos.y = 0;
						relpos.x = pos.x - ((GlowEffect*)effect)->pos->x;
						relpos.z = pos.z - ((GlowEffect*)effect)->pos->z;
						if (relpos.magnitude() < 1.25)
						{
							break;
						}
						relpos.normalize(0.025);
						pos -= relpos;
					}
				}
			}
			default:
			{
				break;
			}
		}

		return true;
	}

#ifdef	NEW_TEXTURES
	Uint32 GlowParticle::get_texture()
	{
		return base->get_texture(texture);
	}
#else	/* NEW_TEXTURES */
	GLuint GlowParticle::get_texture(const Uint16 res_index)
	{
		return texture->get_texture(res_index);
	}
#endif	/* NEW_TEXTURES */

	light_t GlowParticle::get_light_level()
	{
		return alpha * size / 1500;
	}
	;

	GlowEffect::GlowEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
		const GlowType _type, const Uint16 _LOD)
	{
		if (EC_DEBUG)
			std::cout << "GlowEffect (" << this << ") created (" << type
				<< ")." << std::endl;
		base = _base;
		dead = _dead;
		pos = _pos;
		effect_center = *pos;
		type = _type;
		LOD = base->last_forced_LOD;
		desired_LOD = _LOD;
		spawner = NULL;
		spawner2 = NULL;
		spawner3 = NULL;
		bounds = NULL;
		mover = NULL;
		mover2 = NULL;
		mover3 = NULL;
		shift = Vec3(0.0, 0.0, 0.0);

		switch (type)
		{
			case GlowEffect::REMOTE_HEAL_GLOW:
			{
				mover = new ParticleMover(this);
				spawner = new FilledEllipsoidSpawner(ec::Vec3(2.0, 1.0, 0.1));
				for (int i = 0; i < LOD * 64; i++)
				{
					Vec3 coords = spawner->get_new_coords();
					const Vec3 velocity = coords / 12.0;
					coords += effect_center;
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new GlowParticle(this, mover, coords, velocity, 0.65, 0.05, 0.4 + randcolor(0.3), 0.7, 0.2, EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
							new GlowParticle(this, mover, coords, velocity, 0.65, 0.05, 0.4 + randcolor(0.3), 0.7, 0.2, &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					p->state = 1;
					if (!base->push_back_particle(p))
						break;
				}
#ifdef	NEW_TEXTURES
				Particle* p = new GlowParticle(this, mover, effect_center, Vec3(0.0, 0.0, 0.0), 7.5, 1.0, 1.0, 1.0, 1.0, EC_CRYSTAL, LOD, type);
#else	/* NEW_TEXTURES */
				Particle* p = new GlowParticle(this, mover, effect_center, Vec3(0.0, 0.0, 0.0), 7.5, 1.0, 1.0, 1.0, 1.0, &(base->TexCrystal), LOD, type);
#endif	/* NEW_TEXTURES */
				if (!base->push_back_particle(p))
					break;
#ifdef	NEW_TEXTURES
				p = new GlowParticle(this, mover, effect_center, Vec3(0.0, 0.01, 0.0), 7.5, 1.0, 1.0, 1.0, 1.0, EC_CRYSTAL, LOD, type);
#else	/* NEW_TEXTURES */
				p = new GlowParticle(this, mover, effect_center, Vec3(0.0, 0.01, 0.0), 7.5, 1.0, 1.0, 1.0, 1.0, &(base->TexCrystal), LOD, type);
#endif	/* NEW_TEXTURES */
				base->push_back_particle(p);
				break;
			}
			case GlowEffect::HARM_GLOW:
			{
				mover = new ParticleMover(this);
				spawner = new FilledEllipsoidSpawner(ec::Vec3(2.0, 1.0, 0.1));
				for (int i = 0; i < LOD * 64; i++)
				{
					Vec3 coords = spawner->get_new_coords();
					const Vec3 velocity = coords / 12.0;
					coords += effect_center;
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new GlowParticle(this, mover, coords, velocity, 0.95, 0.05, 1.0, 0.4 + randcolor(0.3), 0.2, EC_FLARE, LOD, type);
#else	/* NEW_TEXTURES */
							new GlowParticle(this, mover, coords, velocity, 0.95, 0.05, 1.0, 0.4 + randcolor(0.3), 0.2, &(base->TexFlare), LOD, type);
#endif	/* NEW_TEXTURES */
					p->state = 1;
					if (!base->push_back_particle(p))
						break;
				}
#ifdef	NEW_TEXTURES
				Particle* p = new GlowParticle(this, mover, effect_center, Vec3(0.0, 0.0, 0.0), 7.5, 1.0, 1.0, 1.0, 1.0, EC_CRYSTAL, LOD, type);
#else	/* NEW_TEXTURES */
				Particle* p = new GlowParticle(this, mover, effect_center, Vec3(0.0, 0.0, 0.0), 7.5, 1.0, 1.0, 1.0, 1.0, &(base->TexCrystal), LOD, type);
#endif	/* NEW_TEXTURES */
				if (!base->push_back_particle(p))
					break;
#ifdef	NEW_TEXTURES
				p = new GlowParticle(this, mover, effect_center, Vec3(0.0, 0.01, 0.0), 7.5, 1.0, 1.0, 1.0, 1.0, EC_CRYSTAL, LOD, type);
#else	/* NEW_TEXTURES */
				p = new GlowParticle(this, mover, effect_center, Vec3(0.0, 0.01, 0.0), 7.5, 1.0, 1.0, 1.0, 1.0, &(base->TexCrystal), LOD, type);
#endif	/* NEW_TEXTURES */
				base->push_back_particle(p);
				break;
			}
			case GlowEffect::POISON_GLOW:
			{
				mover = new ParticleMover(this);
				spawner = new FilledEllipsoidSpawner(ec::Vec3(2.0, 1.0, 0.1));
				for (int i = 0; i < LOD * 64; i++)
				{
					Vec3 coords = spawner->get_new_coords();
					const Vec3 velocity = coords / 12.0;
					coords += effect_center;
#ifdef	NEW_TEXTURES
					Particle* p = new GlowParticle(this, mover, coords, velocity, 1.95, 0.05, randcolor(0.3), 0.5 + randcolor(0.3), randcolor(0.5), EC_INVERSE, LOD, type);
#else	/* NEW_TEXTURES */
					Particle* p = new GlowParticle(this, mover, coords, velocity, 1.95, 0.05, randcolor(0.3), 0.5 + randcolor(0.3), randcolor(0.5), &(base->TexInverse), LOD, type);
#endif	/* NEW_TEXTURES */
					p->state = 1;
					if (!base->push_back_particle(p))
						break;
				}
#ifdef	NEW_TEXTURES
				Particle* p = new GlowParticle(this, mover, effect_center, Vec3(0.0, 0.0, 0.0), 7.5, 1.0, 1.0, 1.0, 1.0, EC_CRYSTAL, LOD, type);
#else	/* NEW_TEXTURES */
				Particle* p = new GlowParticle(this, mover, effect_center, Vec3(0.0, 0.0, 0.0), 7.5, 1.0, 1.0, 1.0, 1.0, &(base->TexCrystal), LOD, type);
#endif	/* NEW_TEXTURES */
				if (!base->push_back_particle(p))
					break;
#ifdef	NEW_TEXTURES
				p = new GlowParticle(this, mover, effect_center, Vec3(0.0, 0.01, 0.0), 7.5, 1.0, 1.0, 1.0, 1.0, EC_CRYSTAL, LOD, type);
#else	/* NEW_TEXTURES */
				p = new GlowParticle(this, mover, effect_center, Vec3(0.0, 0.01, 0.0), 7.5, 1.0, 1.0, 1.0, 1.0, &(base->TexCrystal), LOD, type);
#endif	/* NEW_TEXTURES */
				base->push_back_particle(p);
				break;
			}
			case LEVEL_UP_DEFAULT_GLOW:
			{
				spawner = new FilledSphereSpawner(0.9);
				mover = new GravityMover(this, &effect_center, 9e3);
				for (int i = 0; i < LOD * 64; i++)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center;
					Vec3 velocity;
					velocity.randomize();
					velocity.normalize(0.9);
#ifdef	NEW_TEXTURES
					Particle * p = new GlowParticle(this, mover, coords, velocity, 1.25, 0.5 + randcolor(0.5), randcolor(1.0), randcolor(1.0), randcolor(1.0), EC_CRYSTAL, LOD, type);
#else	/* NEW_TEXTURES */
					Particle * p = new GlowParticle(this, mover, coords, velocity, 1.25, 0.5 + randcolor(0.5), randcolor(1.0), randcolor(1.0), randcolor(1.0), &(base->TexCrystal), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case GlowEffect::LEVEL_UP_OA_GLOW:
			{
				spawner = new FilledSphereSpawner(0.75);
				spawner2 = new FilledDiscSpawner(0.55);
				spawner3 = new FilledSphereSpawner(0.9);
				mover = new GravityMover(this, &effect_center, 4e9);
				mover2 = new GravityMover(this, &effect_center, 4e9);
				mover3 = new GravityMover(this, &effect_center, 9e3);
				for (int i = 0; i < LOD * 32; i++)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center;
					Vec3 velocity;
					velocity.randomize();
					velocity.normalize(0.75);
#ifdef	NEW_TEXTURES
					Particle * p = new GlowParticle(this, mover, coords, velocity, 3.0 + randcolor(0.75), 0.7 + randcolor(0.3), 254.0/255.0, 254.0/255.0 - 0.1 + randcolor(0.1), 0.0, EC_INVERSE, LOD, type);
#else	/* NEW_TEXTURES */
					Particle * p = new GlowParticle(this, mover, coords, velocity, 3.0 + randcolor(0.75), 0.7 + randcolor(0.3), 254.0/255.0, 254.0/255.0 - 0.1 + randcolor(0.1), 0.0, &(base->TexInverse), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				for (int i = 0; i < LOD * 32; i++)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center;
					Vec3 velocity;
					velocity.randomize();
					velocity.normalize(0.9);
#ifdef	NEW_TEXTURES
					Particle * p = new GlowParticle(this, mover2, coords, velocity, 2.75, 0.7 + randcolor(0.3), 254.0/255.0 - 0.2 + randcolor(0.2), randcolor(1.0), randcolor(0.33), EC_TWINFLARE, LOD, type);
#else	/* NEW_TEXTURES */
					Particle * p = new GlowParticle(this, mover2, coords, velocity, 2.75, 0.7 + randcolor(0.3), 254.0/255.0 - 0.2 + randcolor(0.2), randcolor(1.0), randcolor(0.33), &(base->TexTwinflare), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case LEVEL_UP_ATT_GLOW:
			{
				spawner = new FilledEllipsoidSpawner(ec::Vec3(2.0, 1.0, 0.1));
				spawner2 = new HollowDiscSpawner(0.65);
				mover = new SpiralMover(this, &effect_center, 5.0, 80.0);
				mover2 = new SpiralMover(this, &effect_center, 15.0, 14.0);
				for (int i = 0; i < LOD * 32; i++)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center;
					Vec3 velocity;
					velocity.randomize(0.36);
					velocity.y *= 2.5;
					velocity.y -= 0.7;
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new GlowParticle(this, mover, coords, velocity, 8.0, 0.5, 1.0, 0.0, 0.0, EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
							new GlowParticle(this, mover, coords, velocity, 8.0, 0.5, 1.0, 0.0, 0.0, &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				for (int i = 0; i < LOD * 32; i++)
				{
					const Vec3 coords = spawner2->get_new_coords()
						+ effect_center;
					Vec3 velocity;
					velocity.randomize(0.36);
					velocity.y *= 2.5;
					velocity.y -= 0.7;
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new GlowParticle(this, mover2, coords, velocity, 2.0, 1.0, 1.0, 0.2, 0.2, EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
							new GlowParticle(this, mover2, coords, velocity, 2.0, 1.0, 1.0, 0.2, 0.2, &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case LEVEL_UP_DEF_GLOW:
			{
				spawner = new HollowSphereSpawner(1.25);
				mover = new GravityMover(this, &effect_center, 3e10);
				for (int i = 0; i < LOD * 192; i++)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center;
					Vec3 velocity;
					velocity.randomize(0.1);
					velocity.normalize(0.1);
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new GlowParticle(this, mover, coords, velocity, 2.0, 0.5, 0.0, 0.0, 1.0, EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
							new GlowParticle(this, mover, coords, velocity, 2.0, 0.5, 0.0, 0.0, 1.0, &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case LEVEL_UP_HAR_GLOW:
			{
				spawner = new HollowDiscSpawner(0.5);
				spawner2 = new HollowDiscSpawner(0.3);
				spawner3 = new HollowDiscSpawner(0.1);
				mover = new ParticleMover(this);
				mover2 = new ParticleMover(this);
				mover3 = new ParticleMover(this);
				for (int i = 0; i < LOD * 64; i++)
				{
					Vec3 coords = spawner->get_new_coords() + effect_center;
					coords.y += (coord_t)(randfloat(2.0) * randfloat(2.0) * randfloat(2.0));
					const Vec3 velocity(0.0, -randfloat(0.25), 0.0);
#ifdef	NEW_TEXTURES
					Particle * p = new GlowParticle(this, mover, coords, velocity, 1.0 + randcoord(1.0), 0.8, randcolor(1.0), randcolor(1.0), randcolor(1.0), EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
					Particle * p = new GlowParticle(this, mover, coords, velocity, 1.0 + randcoord(1.0), 0.8, randcolor(1.0), randcolor(1.0), randcolor(1.0), &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				for (int i = 0; i < LOD * 64; i++)
				{
					Vec3 coords = spawner2->get_new_coords() + effect_center;
					coords.y += (coord_t)(randfloat(2.0) * randfloat(2.0) * randfloat(2.0));
					const Vec3 velocity(0.0, -randfloat(0.125), 0.0);
#ifdef	NEW_TEXTURES
					Particle * p = new GlowParticle(this, mover2, coords, velocity, 2.0 + randcoord(1.0), 0.9, randcolor(1.0), randcolor(1.0), randcolor(1.0), EC_FLARE, LOD, type);
#else	/* NEW_TEXTURES */
					Particle * p = new GlowParticle(this, mover2, coords, velocity, 2.0 + randcoord(1.0), 0.9, randcolor(1.0), randcolor(1.0), randcolor(1.0), &(base->TexFlare), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				for (int i = 0; i < LOD * 64; i++)
				{
					Vec3 coords = spawner3->get_new_coords() + effect_center;
					coords.y += (coord_t)(randfloat(2.0) * randfloat(2.0) * randfloat(2.0));
					const Vec3 velocity(0.0, -randfloat(0.0625), 0.0);
#ifdef	NEW_TEXTURES
					Particle * p = new GlowParticle(this, mover3, coords, velocity, 3.0 + randcoord(1.0), 1.0, randcolor(1.0), randcolor(1.0), randcolor(1.0), EC_TWINFLARE, LOD, type);
#else	/* NEW_TEXTURES */
					Particle * p = new GlowParticle(this, mover3, coords, velocity, 3.0 + randcoord(1.0), 1.0, randcolor(1.0), randcolor(1.0), randcolor(1.0), &(base->TexTwinflare), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case LEVEL_UP_ALC_GLOW_L:
			case LEVEL_UP_ALC_GLOW_R:
			{
				spawner = new FilledSphereSpawner(0.1);
				mover = new ParticleMover(this);
				red = 0.1;
				green = 0.1;
				blue = 1.0;
				for (int i = 0; i < LOD * 4; i++)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center;
					Vec3 velocity;
					velocity.randomize(2.0);
					velocity.y = randfloat(0.5);
#ifdef	NEW_TEXTURES
					Particle * p = new GlowParticle(this, mover, coords, velocity, 0.1 + randcoord(0.5), 0.8, red * 0.75 + randcolor(red / 4.0), green * 0.75 + randcolor(green / 4.0), blue * 0.75 + randcolor(blue / 4.0), EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
					Particle * p = new GlowParticle(this, mover, coords, velocity, 0.1 + randcoord(0.5), 0.8, red * 0.75 + randcolor(red / 4.0), green * 0.75 + randcolor(green / 4.0), blue * 0.75 + randcolor(blue / 4.0), &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case LEVEL_UP_POT_GLOW_L:
			case LEVEL_UP_POT_GLOW_R:
			{
				spawner = new FilledSphereSpawner(0.1);
				mover = new ParticleMover(this);
				red = 1.0;
				green = 0.1;
				blue = 0.1;
				for (int i = 0; i < LOD * 4; i++)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center;
					Vec3 velocity;
					velocity.randomize(2.0);
					velocity.y = randfloat(0.5);
#ifdef	NEW_TEXTURES
					Particle * p = new GlowParticle(this, mover, coords, velocity, 0.1 + randcoord(0.5), 0.8, red * 0.75 + randcolor(red / 4.0), green * 0.75 + randcolor(green / 4.0), blue * 0.75 + randcolor(blue / 4.0), EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
					Particle * p = new GlowParticle(this, mover, coords, velocity, 0.1 + randcoord(0.5), 0.8, red * 0.75 + randcolor(red / 4.0), green * 0.75 + randcolor(green / 4.0), blue * 0.75 + randcolor(blue / 4.0), &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case LEVEL_UP_MAN_GLOW_L:
			case LEVEL_UP_MAN_GLOW_R:
			{
				spawner = new FilledSphereSpawner(0.1);
				mover = new ParticleMover(this);
				red = 0.1;
				green = 1.0;
				blue = 0.1;
				for (int i = 0; i < LOD * 4; i++)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center;
					Vec3 velocity;
					velocity.randomize(2.0);
					velocity.y = randfloat(0.5);
#ifdef	NEW_TEXTURES
					Particle * p = new GlowParticle(this, mover, coords, velocity, 0.1 + randcoord(0.5), 0.8, red * 0.75 + randcolor(red / 4.0), green * 0.75 + randcolor(green / 4.0), blue * 0.75 + randcolor(blue / 4.0), EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
					Particle * p = new GlowParticle(this, mover, coords, velocity, 0.1 + randcoord(0.5), 0.8, red * 0.75 + randcolor(red / 4.0), green * 0.75 + randcolor(green / 4.0), blue * 0.75 + randcolor(blue / 4.0), &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case LEVEL_UP_CRA_GLOW_L:
			case LEVEL_UP_CRA_GLOW_R:
			{
				spawner = new FilledSphereSpawner(0.1);
				mover = new ParticleMover(this);
				red = 1.0;
				green = 1.0;
				blue = 0.1;
				for (int i = 0; i < LOD * 4; i++)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center;
					Vec3 velocity;
					velocity.randomize(2.0);
					velocity.y = randfloat(0.5);
#ifdef	NEW_TEXTURES
					Particle * p = new GlowParticle(this, mover, coords, velocity, 0.1 + randcoord(0.5), 0.8, red * 0.75 + randcolor(red / 4.0), green * 0.75 + randcolor(green / 4.0), blue * 0.75 + randcolor(blue / 4.0), EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
					Particle * p = new GlowParticle(this, mover, coords, velocity, 0.1 + randcoord(0.5), 0.8, red * 0.75 + randcolor(red / 4.0), green * 0.75 + randcolor(green / 4.0), blue * 0.75 + randcolor(blue / 4.0), &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case LEVEL_UP_ENG_GLOW_L:
			case LEVEL_UP_ENG_GLOW_R:
			{
				spawner = new FilledSphereSpawner(0.1);
				mover = new ParticleMover(this);
				red = 0.75;
				green = 0.75;
				blue = 1.0;
				for (int i = 0; i < LOD * 4; i++)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center;
					Vec3 velocity;
					velocity.randomize(2.0);
					velocity.y = randfloat(0.5);
#ifdef	NEW_TEXTURES
					Particle * p = new GlowParticle(this, mover, coords, velocity, 0.1 + randcoord(0.5), 0.8, red * 0.75 + randcolor(red / 4.0), green * 0.75 + randcolor(green / 4.0), blue * 0.75 + randcolor(blue / 4.0), EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
					Particle * p = new GlowParticle(this, mover, coords, velocity, 0.1 + randcoord(0.5), 0.8, red * 0.75 + randcolor(red / 4.0), green * 0.75 + randcolor(green / 4.0), blue * 0.75 + randcolor(blue / 4.0), &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case LEVEL_UP_TAI_GLOW_L:
			case LEVEL_UP_TAI_GLOW_R:
			{
				spawner = new FilledSphereSpawner(0.1);
				mover = new ParticleMover(this);
				red = 0.75;
				green = 0.5;
				blue = 0.5;
				for (int i = 0; i < LOD * 4; i++)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center;
					Vec3 velocity;
					velocity.randomize(2.0);
					velocity.y = randfloat(0.5);
#ifdef	NEW_TEXTURES
					Particle * p = new GlowParticle(this, mover, coords, velocity, 0.1 + randcoord(0.5), 0.8, red * 0.75 + randcolor(red / 4.0), green * 0.75 + randcolor(green / 4.0), blue * 0.75 + randcolor(blue / 4.0), EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
					Particle * p = new GlowParticle(this, mover, coords, velocity, 0.1 + randcoord(0.5), 0.8, red * 0.75 + randcolor(red / 4.0), green * 0.75 + randcolor(green / 4.0), blue * 0.75 + randcolor(blue / 4.0), &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case LEVEL_UP_RAN_GLOW:
			{
				mover = new SpiralMover(this, &effect_center, 1.0, 1.0);
				Particle *p;
				const Vec3 center = effect_center;
				for (float i = 0; i < 26; i++)
				{
#ifdef	NEW_TEXTURES
					p = new GlowParticle(this, mover, Vec3(1.25 - (1.25/26.0) * i, 0.0, 0.05 + (0.6/26.0) * i) + center - Vec3(0.7, 0.0, 0.5), Vec3(0.0, 0.0, 0.0), 1.0, 1.0, 1.0, 0.0, 0.0, EC_VOID, LOD, type);
#else	/* NEW_TEXTURES */
					p = new GlowParticle(this, mover, Vec3(1.25 - (1.25/26.0) * i, 0.0, 0.05 + (0.6/26.0) * i) + center - Vec3(0.7, 0.0, 0.5), Vec3(0.0, 0.0, 0.0), 1.0, 1.0, 1.0, 0.0, 0.0, &(base->TexVoid), LOD, type);
#endif	/* NEW_TEXTURES */
					base->push_back_particle(p);
				}
				for (float i = 0; i < 22; i++)
				{
#ifdef	NEW_TEXTURES
					p = new GlowParticle(this, mover, Vec3(0.45 + (0.5/22.0) * i, 0.0, 0.0 + (1.05/22.0) * i) + center - Vec3(0.7, 0.0, 0.5), Vec3(0.0, 0.0, 0.0), 1.0, 1.0, 1.0, 0.0, 0.0, EC_VOID, LOD, type);
#else	/* NEW_TEXTURES */
					p = new GlowParticle(this, mover, Vec3(0.45 + (0.5/22.0) * i, 0.0, 0.0 + (1.05/22.0) * i) + center - Vec3(0.7, 0.0, 0.5), Vec3(0.0, 0.0, 0.0), 1.0, 1.0, 1.0, 0.0, 0.0, &(base->TexVoid), LOD, type);
#endif	/* NEW_TEXTURES */
					base->push_back_particle(p);
				}
				for (float i = 0; i < 5; i++)
				{
#ifdef	NEW_TEXTURES
					p = new GlowParticle(this, mover, Vec3(1.05 - (0.1/5.0) * i, 0.0, 0.85 + (0.25/5.0) * i) + center - Vec3(0.7, 0.0, 0.5), Vec3(0.0, 0.0, 0.0), 1.0, 1.0, 1.0, 0.0, 0.0, EC_VOID, LOD, type);
#else	/* NEW_TEXTURES */
					p = new GlowParticle(this, mover, Vec3(1.05 - (0.1/5.0) * i, 0.0, 0.85 + (0.25/5.0) * i) + center - Vec3(0.7, 0.0, 0.5), Vec3(0.0, 0.0, 0.0), 1.0, 1.0, 1.0, 0.0, 0.0, &(base->TexVoid), LOD, type);
#endif	/* NEW_TEXTURES */
					base->push_back_particle(p);
				}
				for (float i = 0; i < 5; i++)
				{
#ifdef	NEW_TEXTURES
					p = new GlowParticle(this, mover, Vec3(0.75 + (0.25/5.0) * i, 0.0, 0.95 + (0.1/5.0) * i) + center - Vec3(0.7, 0.0, 0.5), Vec3(0.0, 0.0, 0.0), 1.0, 1.0, 1.0, 0.0, 0.0, EC_VOID, LOD, type);
#else	/* NEW_TEXTURES */
					p = new GlowParticle(this, mover, Vec3(0.75 + (0.25/5.0) * i, 0.0, 0.95 + (0.1/5.0) * i) + center - Vec3(0.7, 0.0, 0.5), Vec3(0.0, 0.0, 0.0), 1.0, 1.0, 1.0, 0.0, 0.0, &(base->TexVoid), LOD, type);
#endif	/* NEW_TEXTURES */
					base->push_back_particle(p);
				}
				for (float i = 0; i < 34; i++)
				{
					p
#ifdef	NEW_TEXTURES
						= new GlowParticle(this, mover, Vec3(cos((10.0 + (105.0/32.0) * i)/360 * 2 * 3.14), 0.0, sin((10.0 + (105.0/32.0) * i)/360 * 2 * 3.14)) + center - Vec3(0.35, 0.0, 0.75), Vec3(0.0, 0.0, 0.0), 1.0, 1.0, 1.0, 0.0, 0.0, EC_VOID, LOD, type);
#else	/* NEW_TEXTURES */
						= new GlowParticle(this, mover, Vec3(cos((10.0 + (105.0/32.0) * i)/360 * 2 * 3.14), 0.0, sin((10.0 + (105.0/32.0) * i)/360 * 2 * 3.14)) + center - Vec3(0.35, 0.0, 0.75), Vec3(0.0, 0.0, 0.0), 1.0, 1.0, 1.0, 0.0, 0.0, &(base->TexVoid), LOD, type);
#endif	/* NEW_TEXTURES */
					base->push_back_particle(p);
				}
				break;
			}
			case LEVEL_UP_MAG_GLOW:
			{
				mover = new ParticleMover(this);
				spawner = new HollowSphereSpawner(0.05);
				for (int i = 0; i < LOD * 32; i++)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center;
					Vec3 velocity;
					velocity.randomize(5.0);
					velocity += (effect_center - coords).normalize(5.0);
					velocity.y = fabs(velocity.y);
					if (velocity.magnitude() > 7.5)
					{
						velocity.normalize(7.5);
					}
#ifdef	NEW_TEXTURES
					Particle * p = new GlowParticle(this, mover, coords, velocity, 1.0 + randfloat(0.125), 1.0, randcolor(), randcolor(), randcolor(), EC_VOID, LOD, type);
#else	/* NEW_TEXTURES */
					Particle * p = new GlowParticle(this, mover, coords, velocity, 1.0 + randfloat(0.125), 1.0, randcolor(), randcolor(), randcolor(), &(base->TexVoid), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				for (int i = 0; i < LOD * 24; i++)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center;
					Vec3 velocity;
					velocity.randomize(5.0);
					velocity += (effect_center - coords).normalize(5.0);
					velocity.y = fabs(velocity.y);
					if (velocity.magnitude() > 7.5)
					{
						velocity.normalize(7.5);
					}
#ifdef	NEW_TEXTURES
					Particle * p = new GlowParticle(this, mover, coords, velocity, 1.0 + randfloat(0.25), 1.0, randcolor(), randcolor(), randcolor(), EC_INVERSE, LOD, type);
#else	/* NEW_TEXTURES */
					Particle * p = new GlowParticle(this, mover, coords, velocity, 1.0 + randfloat(0.25), 1.0, randcolor(), randcolor(), randcolor(), &(base->TexInverse), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				for (int i = 0; i < LOD * 16; i++)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center;
					Vec3 velocity;
					velocity.randomize(5.0);
					velocity += (effect_center - coords).normalize(5.0);
					velocity.y = fabs(velocity.y);
					if (velocity.magnitude() > 7.5)
					{
						velocity.normalize(7.5);
					}
#ifdef	NEW_TEXTURES
					Particle * p = new GlowParticle(this, mover, coords, velocity, 1.0 + randfloat(0.5), 1.0, randcolor(), randcolor(), randcolor(), EC_TWINFLARE, LOD, type);
#else	/* NEW_TEXTURES */
					Particle * p = new GlowParticle(this, mover, coords, velocity, 1.0 + randfloat(0.5), 1.0, randcolor(), randcolor(), randcolor(), &(base->TexTwinflare), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				for (int i = 0; i < LOD * 8; i++)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center;
					Vec3 velocity;
					velocity.randomize(5.0);
					velocity += (effect_center - coords).normalize(5.0);
					velocity.y = fabs(velocity.y);
					if (velocity.magnitude() > 7.5)
					{
						velocity.normalize(7.5);
					}
#ifdef	NEW_TEXTURES
					Particle * p = new GlowParticle(this, mover, coords, velocity, 1.0 + randfloat(), 1.0, randcolor(), randcolor(), randcolor(), EC_FLARE, LOD, type);
#else	/* NEW_TEXTURES */
					Particle * p = new GlowParticle(this, mover, coords, velocity, 1.0 + randfloat(), 1.0, randcolor(), randcolor(), randcolor(), &(base->TexFlare), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
			}
			case LEVEL_UP_SUM_GLOW:
			{
				mover = new ParticleMover(this);
#ifdef	NEW_TEXTURES
				Particle * p = new GlowParticle(this, mover, effect_center, Vec3(0.0, 0.0, 0.0), 1.0f, 1.0f, 0.7f, 0.3f, 0.7f, EC_CRYSTAL, LOD, type);
#else	/* NEW_TEXTURES */
				Particle * p = new GlowParticle(this, mover, effect_center, Vec3(0.0, 0.0, 0.0), 1.0f, 1.0f, 0.7f, 0.3f, 0.7f, &(base->TexCrystal), LOD, type);
#endif	/* NEW_TEXTURES */
				if (!base->push_back_particle(p))
					break;
				break;
			}
			default:
			{
				break;
			}
		}
	}

	GlowEffect::~GlowEffect()
	{
		if (spawner)
			delete spawner;
		if (spawner2)
			delete spawner2;
		if (spawner3)
			delete spawner3;
		if (mover)
			delete mover;
		if (mover2)
			delete mover2;
		if (mover3)
			delete mover3;
		if (EC_DEBUG)
			std::cout << "GlowEffect (" << this << ") destroyed." << std::endl;
	}

	bool GlowEffect::idle(const Uint64 usec)
	{
		if (particles.size() == 0)
			return false;

		if (recall)
			return true;

		const Vec3 last_effect_center = effect_center;

		effect_center.x = pos->x;
		effect_center.z = pos->z;

		switch (type)
		{
			case GlowEffect::LEVEL_UP_OA_GLOW:
			{
				effect_center.y += usec / 2500000.0;
				break;
			}
			case GlowEffect::LEVEL_UP_HAR_GLOW:
			{
				break;
			}
			case GlowEffect::LEVEL_UP_ALC_GLOW_L:
			case GlowEffect::LEVEL_UP_ALC_GLOW_R:
			case GlowEffect::LEVEL_UP_POT_GLOW_L:
			case GlowEffect::LEVEL_UP_POT_GLOW_R:
			case GlowEffect::LEVEL_UP_MAN_GLOW_L:
			case GlowEffect::LEVEL_UP_MAN_GLOW_R:
			case GlowEffect::LEVEL_UP_CRA_GLOW_L:
			case GlowEffect::LEVEL_UP_CRA_GLOW_R:
			case GlowEffect::LEVEL_UP_ENG_GLOW_L:
			case GlowEffect::LEVEL_UP_ENG_GLOW_R:
			case GlowEffect::LEVEL_UP_TAI_GLOW_L:
			case GlowEffect::LEVEL_UP_TAI_GLOW_R:
			{
				if (get_time() - born < 4000000)
				{
					for (int i = 0; i < LOD * 4; i++)
					{
						const Vec3 coords = spawner->get_new_coords()
							+ effect_center;
						Vec3 velocity;
						velocity.randomize(2.0);
						velocity.y = randfloat(0.5);
#ifdef	NEW_TEXTURES
						Particle * p = new GlowParticle(this, mover, coords, velocity, 0.1 + randcoord(0.5), 0.8, red * 0.75 + randcolor(red / 4.0), green * 0.75 + randcolor(green / 4.0), blue * 0.75 + randcolor(blue / 4.0), EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
						Particle * p = new GlowParticle(this, mover, coords, velocity, 0.1 + randcoord(0.5), 0.8, red * 0.75 + randcolor(red / 4.0), green * 0.75 + randcolor(green / 4.0), blue * 0.75 + randcolor(blue / 4.0), &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
						if (!base->push_back_particle(p))
							break;
					}
				}
				break;
			}
			case LEVEL_UP_SUM_GLOW:
			{
				effect_center.y = pos->y;
				const Uint64 age = get_time() - born;
				const float age_f = (float)(age)/1000000.0f;
				if (age_f < 4.0)
				{
					for (float f = (age - usec)/1000000.0f; f < age_f; f
						+= 0.0125)
					{
						for (float angle = 0.0f; angle < 2.0f; angle += 0.5f)
						{
							Vec3 coords = effect_center;
							coords.x += cos(M_PI * age_f + M_PI * angle)
								* (age_f + 1.0f) * 0.25;
							coords.z += sin(M_PI * age_f + M_PI * angle)
								* (age_f + 1.0f) * 0.25;
#ifdef	NEW_TEXTURES
							Particle * p = new GlowParticle(this, mover, coords, Vec3(0.0, -randfloat(0.75), 0.0), 0.1 + randcoord(0.5), 1.0f, 0.7f, 0.3f, 0.7f, EC_CRYSTAL, LOD, type);
#else	/* NEW_TEXTURES */
							Particle * p = new GlowParticle(this, mover, coords, Vec3(0.0, -randfloat(0.75), 0.0), 0.1 + randcoord(0.5), 1.0f, 0.7f, 0.3f, 0.7f, &(base->TexCrystal), LOD, type);
#endif	/* NEW_TEXTURES */
							if (!base->push_back_particle(p))
								break;
						}
					}
				}
			}
			default:
			{
				effect_center.y = pos->y;
				break;
			}
		}

		shift = effect_center - last_effect_center;

		return true;
	}

///////////////////////////////////////////////////////////////////////////////

}
;

