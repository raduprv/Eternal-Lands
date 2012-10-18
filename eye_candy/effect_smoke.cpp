// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_smoke.h"

namespace ec
{

	// C L A S S   F U N C T I O N S //////////////////////////////////////////////

	SmokeParticle::SmokeParticle(Effect* _effect, ParticleMover* _mover,
		const Vec3 _pos, const Vec3 _velocity, const color_t hue_adjust,
		const color_t saturation_adjust, const coord_t _sqrt_scale,
		const coord_t _max_size, const coord_t size_scalar,
		const alpha_t alpha_scale) :
		Particle(_effect, _mover, _pos, _velocity,
			size_scalar * (0.5f + randcoord()))
	{
		sqrt_scale = _sqrt_scale;
		max_size = _max_size;
		const color_t color_scale= square(randcolor(0.6));
		color_t hue, saturation, value;
		hue = randcolor(1.0);
		//  saturation = 1.0 - (color_scale + 0.15) / (0.15 + color_scale + square(randcolor(0.15)));
		saturation = color_scale;
		value = square(randcolor(0.15)) + color_scale + 0.15;
		//  color[0] = square(randcolor(0.15)) + color_scale + 0.15;
		//  color[1] = square(randcolor(0.15)) + color_scale + 0.15;
		//  color[2] = square(randcolor(0.15)) + color_scale + 0.15;
		hue += hue_adjust;
		if (hue > 1.0)
			hue -= 1.0;
		saturation = std::min(1.0f, saturation * saturation_adjust);
		hsv_to_rgb(hue, saturation, value, color[0], color[1], color[2]);
		alpha = std::min(1.0f, (0.05f + randcoord(0.1f)) * alpha_scale);
		flare_max = 1.0;
		flare_exp = 1.0;
		flare_frequency = 1.0;
		state = 0;
	}

#ifdef	NEW_TEXTURES
	float SmokeParticle::get_burn() const
	{
		return 0.0f;
	}
#else	/* NEW_TEXTURES */
	void SmokeParticle::draw(const Uint64 usec)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		Vec3 shifted_pos = pos - *(((SmokeEffect*)effect)->pos);

		Particle::draw(usec);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}
#endif	/* NEW_TEXTURES */

	bool SmokeParticle::idle(const Uint64 delta_t)
	{
		if (effect->recall)
			return false;

		const alpha_t alpha_scalar =
			1.0 - std::pow(0.5f, (float)delta_t / (60000000
				* sqrt_scale));
		alpha -= alpha_scalar;

		if (alpha < 0.006)
			return false;

		const coord_t size_scalar = std::pow(0.5f, (float)delta_t
			/ (1500000 * sqrt_scale));
		size = std::min(max_size, size / size_scalar * 0.25f + size * 0.75f);

		Vec3 velocity_shift;
		velocity_shift.randomize();
		velocity_shift.y /= 3;
		velocity_shift.normalize(0.00002 * std::sqrt(delta_t));
		velocity += velocity_shift;
		return true;
	}

#ifdef	NEW_TEXTURES
	Uint32 SmokeParticle::get_texture()
	{
		return base->get_texture(EC_SIMPLE);
	}
#else	/* NEW_TEXTURES */
	GLuint SmokeParticle::get_texture(const Uint16 res_index)
	{
		return base->TexSimple.get_texture(res_index);
	}
#endif	/* NEW_TEXTURES */

	SmokeEffect::SmokeEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
		const color_t _hue_adjust, const color_t _saturation_adjust,
		const float _scale, const Uint16 _LOD)
	{
		if (EC_DEBUG)
			std::cout << "SmokeEffect (" << this << ") created (" << *_pos
				<< ", " << _scale << ")" << std::endl;
		base = _base;
		dead = _dead;
		pos = _pos;
		hue_adjust = _hue_adjust;
		saturation_adjust = _saturation_adjust;
		count = 0;
		scale = _scale;
		sqrt_scale = std::sqrt(scale);
		max_size = scale * 270 / (_LOD + 10);
		size_scalar = sqrt_scale * 75 / (_LOD + 5);
		alpha_scalar = 5.5 / (std::sqrt(_LOD) + 1.0);
		count_scalar = 500000 / _LOD;
		LOD = base->last_forced_LOD;
		desired_LOD = _LOD;
		bounds = NULL;
		mover = new GradientMover(this);
		spawner = new FilledDiscSpawner(0.2 * sqrt_scale);

		//  Test code:
		//  Particle* p = new SmokeParticle(this, mover, *pos, Vec3(0.0, 0.0, 0.0), hue_adjust, saturation_adjust, sqrt_scale, max_size, size_scalar, alpha_scalar);
		//  base->push_back_particle(p);
		/*  
		 for (int i = 0; i < LOD * 4; i++)
		 {
		 const Vec3 coords = spawner->get_new_coords() + *pos;
		 Vec3 velocity;
		 velocity.randomize(0.015);
		 velocity.y += 0.25 + randcoord(0.15);
		 Particle* p = new SmokeParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, sqrt_scale, max_size, size_scalar, alpha_scalar);
		 if (!base->push_back_particle(p))
		 break;
		 }
		 */
	}

	SmokeEffect::~SmokeEffect()
	{
		delete mover;
		delete spawner;
		if (EC_DEBUG)
			std::cout << "SmokeEffect (" << this << ") destroyed." << std::endl;
	}

	bool SmokeEffect::idle(const Uint64 usec)
	{
		if ((recall) && (particles.size() == 0))
			return false;

		if (recall)
			return true;

		count += usec;

		while (count > 0)
		{
			const Vec3 coords = spawner->get_new_coords() + *pos;
			Vec3 velocity;
			velocity.randomize(0.015);
			velocity.y += 0.3;
			Particle
				* p =
					new SmokeParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, sqrt_scale, max_size, size_scalar, alpha_scalar);
			if (!base->push_back_particle(p))
			{
				count = 0;
				break;
			}
			count -= count_scalar * std::max(3.0f, 10.0f - LOD);
		}

		return true;
	}

///////////////////////////////////////////////////////////////////////////////

}
;

