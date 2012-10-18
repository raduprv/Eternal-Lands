// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_lamp.h"

namespace ec
{

	// C L A S S   F U N C T I O N S //////////////////////////////////////////////

	LampParticle::LampParticle(Effect* _effect, ParticleMover* _mover,
		const Vec3 _pos, const Vec3 _velocity, const color_t hue_adjust,
		const color_t saturation_adjust, const float _scale, const Uint16 _LOD) :
		Particle(_effect, _mover, _pos, _velocity,
			4 * (0.15 + 2.3 * randcoord() * randcoord()) / (LOD + 2))
	{
		LOD = _LOD;
		color_t hue, saturation, value;
		hue = 0.04 + randcolor(0.09);
		saturation = 0.4;
		value = 1.0;
		hue += hue_adjust;
		if (hue > 1.0)
			hue -= 1.0;
		saturation = std::min(1.0f, saturation * saturation_adjust);
		hsv_to_rgb(hue, saturation, value, color[0], color[1], color[2]);
		alpha = std::min(1.0f, 7.0f / size);
		velocity /= size;
		size *= _scale;
		flare_max = 1.0;
		flare_exp = 0.0;
		flare_frequency = 2.0;
	}

	bool LampParticle::idle(const Uint64 delta_t)
	{
		if (effect->recall)
			return false;

		if (alpha < 0.02)
		{
			return false;
		}

		const float scalar = std::pow(0.5f, (float)delta_t / 400000);
		//  color[0] = color[0] * scalar * 0.5 + color[0] * 0.5;
		//  color[1] = color[1] * scalar * 0.5 + color[1] * 0.5;
		//  color[2] = color[2] * scalar * 0.5 + color[2] * 0.5;
		alpha *= scalar;

		return true;
	}

#ifdef	NEW_TEXTURES
	Uint32 LampParticle::get_texture()
	{
		return base->get_texture(EC_FLARE);
	}
#else	/* NEW_TEXTURES */
	GLuint LampParticle::get_texture(const Uint16 res_index)
	{
		return base->TexFlare.get_texture(res_index);
	}
#endif	/* NEW_TEXTURES */

	LampBigParticle::LampBigParticle(Effect* _effect, ParticleMover* _mover,
		const Vec3 _pos, const Vec3 _velocity, const color_t hue_adjust,
		const color_t saturation_adjust, const float _scale, const Uint16 _LOD) :
		Particle(_effect, _mover, _pos, _velocity)
	{
		LOD = _LOD;
		color_t hue = 0.02 + randcolor(0.07);
		color_t saturation = 0.7;
		color_t value = 1.0;
		saturation = std::min(1.0f, saturation * saturation_adjust);
		hsv_to_rgb(hue, saturation, value, color[0], color[1], color[2]);
		size = 9 * (2.0 + randcoord()) / (LOD + 2);
		alpha = std::min(1.0f, 1.4f * 5 / size / (LOD + 2));
		size *= _scale;
		velocity = Vec3(0.0, 0.0, 0.0);
		flare_max = 1.0;
		flare_exp = 0.0;
		flare_frequency = 2.0;
		state = ((rand() % 3) == 0);
	}

	bool LampBigParticle::idle(const Uint64 delta_t)
	{
		if (effect->recall)
			return false;

		const float scalar = 1.0 - std::pow(0.5f, (interval_t)delta_t
			* LOD / 32000000.0f);
		//  color[0] = color[0] * scalar * 0.5 + color[0] * 0.5;
		//  color[1] = color[1] * scalar * 0.5 + color[1] * 0.5;
		//  color[2] = color[2] * scalar * 0.5 + color[2] * 0.5;
		alpha -= scalar;

		if (alpha < 0.03)
			return false;

		return true;
	}

#ifdef	NEW_TEXTURES
	Uint32 LampBigParticle::get_texture()
	{
		return base->get_texture(EC_FLARE);
	}

	float LampBigParticle::get_burn() const
	{
		if (state == 0)
		{
			return 1.0f;
		}
		else
		{
			return 0.0f;
		}
	}
#else	/* NEW_TEXTURES */
	GLuint LampBigParticle::get_texture(const Uint16 res_index)
	{
		return base->TexFlare.get_texture(res_index);
	}

	void LampBigParticle::draw(const Uint64 usec)
	{
		if (state == 0)
		{
			Particle::draw(usec);
		}
		else
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			Particle::draw(usec);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		}
	}
#endif	/* NEW_TEXTURES */

	LampFlareParticle::LampFlareParticle(Effect* _effect,
		ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity,
		const float _scale) :
		Particle(_effect, _mover, _pos, _velocity, _scale * 9.5)
	{
		color[0] = 1.0;
		color[1] = 0.5;
		color[2] = 0.1;
		true_size = size;
		alpha = 1.0;
		velocity = Vec3(0.0, 0.0, 0.0);
		flare_max = 1.2;
		flare_exp = 0.5;
		flare_frequency = 0.01;
		true_pos = _pos;
	}

	bool LampFlareParticle::idle(const Uint64 delta_t)
	{
		if (effect->recall)
			return false;

		return true;
	}

#ifdef	NEW_TEXTURES
	Uint32 LampFlareParticle::get_texture()
	{
		return base->get_texture(EC_VOID);
	}
#else	/* NEW_TEXTURES */
	GLuint LampFlareParticle::get_texture(const Uint16 res_index)
	{
		return base->TexVoid.get_texture(res_index);
	}
#endif	/* NEW_TEXTURES */

	LampEffect::LampEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
		const color_t _hue_adjust, const color_t _saturation_adjust,
		const float _scale, const bool _halo, const Uint16 _LOD)
	{
		if (EC_DEBUG)
			std::cout << "LampEffect (" << this << ") created." << std::endl;
		base = _base;
		dead = _dead, pos = _pos;
		hue_adjust = _hue_adjust;
		saturation_adjust = _saturation_adjust;
		scale = _scale;
		halo = _halo;
		if (!halo)
			new_scale = scale * 1.25;
		else
			new_scale = scale;
		sqrt_scale = std::sqrt(new_scale);
		LOD = base->last_forced_LOD;
		desired_LOD = _LOD;
		bounds = NULL;
		mover = new SmokeMover(this);
		mover2 = new SmokeMover(this, 1.2);
		mover3 = new ParticleMover(this);
		spawner = new FilledSphereSpawner(0.065 * sqrt_scale);

		/*
		 for (int i = 0; i < LOD * 1.5; i++)
		 {
		 const Vec3 coords = spawner->get_new_coords() + *pos + Vec3(0.0, 0.05, 0.0);
		 Vec3 velocity;
		 velocity.randomize(0.2);
		 Particle* p = new LampParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, scale, LOD);
		 if (!base->push_back_particle(p))
		 break;
		 }
		 */
		big_particles = 0;
		for (int i = 0; i < LOD * 0.5; i++)
		{
			Vec3 coords = spawner->get_new_coords();
			coords.x *= 0.5;
			coords.z *= 0.5;
			coords.y += 0.1 * sqrt_scale;
			coords += *pos;
			Vec3 velocity;
			velocity.randomize(0.10 * sqrt_scale);
			Particle
				* p =
					new LampBigParticle(this, mover2, coords, velocity, hue_adjust, saturation_adjust, new_scale, LOD);
			if (!base->push_back_particle(p))
				break;
			big_particles++;
		}

		if (halo)
			base->push_back_particle(new LampFlareParticle(this, mover3, *pos + Vec3(0.0, 0.15 * sqrt_scale, 0.0), Vec3(0.0, 0.0, 0.0), 0.65 * sqrt_scale));
	}

	LampEffect::~LampEffect()
	{
		delete mover;
		delete mover2;
		delete mover3;
		delete spawner;
		if (EC_DEBUG)
			std::cout << "LampEffect (" << this << ") destroyed." << std::endl;
	}

	bool LampEffect::idle(const Uint64 usec)
	{
		if ((recall) && (particles.size() == 0))
			return false;

		if (recall)
			return true;

		while (((int)particles.size() < LOD * 15)
			&& (pow_randfloat((LOD * 15 - particles.size()) * (interval_t)usec / 20000 / square(LOD)) < 0.5))
		{
			Vec3 coords = spawner->get_new_coords() + *pos;
			coords.y += 0.11 * sqrt_scale;
			Vec3 velocity;
			velocity.randomize(0.01 * sqrt_scale);
			Particle
				* p =
					new LampParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, new_scale, LOD);
			if (!base->push_back_particle(p))
				break;
		}

		while ((big_particles < LOD * 7)
			&& ((pow_randfloat((LOD * 7 - big_particles) * (interval_t)usec / 9000.0f / square(LOD)) < 0.5) || (big_particles < LOD * 4)))
		{
			Vec3 coords = spawner->get_new_coords();
			coords.y *= 1.6;
			coords.y += 0.1 * sqrt_scale;
			coords += *pos;
			Vec3 velocity;
			velocity.randomize(0.8 * sqrt_scale);
			velocity.y *= 3.0;
			Particle
				* p =
					new LampBigParticle(this, mover2, coords, velocity, hue_adjust, saturation_adjust, new_scale, LOD);
			if (!base->push_back_particle(p))
				break;
			big_particles++;
		}

		return true;
	}

///////////////////////////////////////////////////////////////////////////////

}
;

