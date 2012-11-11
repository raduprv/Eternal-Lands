// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_candle.h"

namespace ec
{

	// C L A S S   F U N C T I O N S //////////////////////////////////////////////

	CandleParticle::CandleParticle(Effect* _effect, ParticleMover* _mover,
		const Vec3 _pos, const Vec3 _velocity, const color_t hue_adjust,
		const color_t saturation_adjust, const float _scale, const Uint16 _LOD) :
		Particle(_effect, _mover, _pos, _velocity)
	{
		color_t hue, saturation, value;
		LOD = _LOD;
		hue = 0.03 + randcolor(0.08);
		saturation = 0.78;
		value = 0.9;
		hue += hue_adjust;
		if (hue > 1.0)
			hue -= 1.0;
		saturation = std::min(1.0f, saturation * saturation_adjust);
		hsv_to_rgb(hue, saturation, value, color[0], color[1], color[2]);
		size = 6.0 * (2.0 + randcoord()) / (LOD + 2);
		alpha = 0.4 * 5 / size / (LOD + 2);
		alpha = std::min(1.0f, alpha);
		size *= _scale;
		flare_max = 1.0;
		flare_exp = 0.0;
		flare_frequency = 2.0;
		state = ((rand() % 3) == 0);
	}

	bool CandleParticle::idle(const Uint64 delta_t)
	{
		if (effect->recall)
			return false;

		const float scalar = 1.0 - std::pow(0.5f, (interval_t)delta_t
			* LOD / 42000000.0f);
		alpha -= scalar;

		if (alpha < 0.02)
			return false;

		return true;
	}

#ifdef	NEW_TEXTURES
	Uint32 CandleParticle::get_texture()
	{
		return base->get_texture(EC_FLARE);
	}

	float CandleParticle::get_burn() const
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
	GLuint CandleParticle::get_texture(const Uint16 res_index)
	{
		return base->TexFlare.get_texture(res_index);
	}

	void CandleParticle::draw(const Uint64 usec)
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

	CandleEffect::CandleEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
		const color_t _hue_adjust, const color_t _saturation_adjust,
		const float _scale, const Uint16 _LOD)
	{
		if (EC_DEBUG)
			std::cout << "CandleEffect (" << this << ") created." << std::endl;
		base = _base;
		dead = _dead, pos = _pos;
		hue_adjust = _hue_adjust;
		saturation_adjust = _saturation_adjust;
		scale = _scale;
		sqrt_scale = std::sqrt(scale);
		LOD = base->last_forced_LOD;
		desired_LOD = _LOD;
		bounds = NULL;
		mover = new SmokeMover(this, sqrt_scale);
		spawner = new FilledSphereSpawner(0.015 * sqrt_scale);
	}

	CandleEffect::~CandleEffect()
	{
		delete mover;
		delete spawner;
		if (EC_DEBUG)
			std::cout << "CandleEffect (" << this << ") destroyed."
				<< std::endl;
	}

	bool CandleEffect::idle(const Uint64 usec)
	{
		if ((recall) && (particles.size() == 0))
			return false;

		if (recall)
			return true;

		while (((int)particles.size() < LOD * 20)
			&& ((pow_randfloat((LOD * 20 - particles.size()) * (interval_t)usec / 80 / square(LOD)) < 0.5) || ((int)particles.size() < LOD * 10)))
		{
			Vec3 coords = spawner->get_new_coords();
			coords.y += 0.1 * sqrt_scale;
			coords += *pos;
			Vec3 velocity;
			velocity.randomize(0.02 * sqrt_scale);
			velocity.y *= 5.0;
			velocity.y += 0.04 * sqrt_scale;
			Particle
				* p =
					new CandleParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, scale, LOD);
			if (!base->push_back_particle(p))
				break;
		}

		return true;
	}

///////////////////////////////////////////////////////////////////////////////

}
;

