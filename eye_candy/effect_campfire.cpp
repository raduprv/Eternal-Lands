// I N C L U D E S ////////////////////////////////////////////////////////////

#ifdef DEBUG_POINT_PARTICLES
#include <iostream>
#include <sstream>
#endif

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_campfire.h"

namespace ec
{

	// C L A S S   F U N C T I O N S //////////////////////////////////////////////

	CampfireParticle::CampfireParticle(Effect* _effect, ParticleMover* _mover,
		const Vec3 _pos, const Vec3 _velocity, const color_t hue_adjust,
		const color_t saturation_adjust, const float _scale,
		const float _sqrt_scale, const int _state, const Uint16 _LOD) :
		Particle(_effect, _mover, _pos, _velocity)
	{
		state = _state;
		color_t hue, saturation, value;
		if (state != 2)
		{
			hue = 0.03 + randcolor(0.1);
			saturation = 0.8;
			value = 1.0;
		}
		else
		{
			hue = 0.9 + randfloat(0.2);
			saturation = randfloat(0.2);
			value = randfloat(0.2);
		}
		hue += hue_adjust;
		if (hue > 1.0)
			hue -= 1.0;
		saturation = std::min(1.0f, saturation * saturation_adjust);
		hsv_to_rgb(hue, saturation, value, color[0], color[1], color[2]);
		LOD = _LOD;
		size = _sqrt_scale * 9.5 * (1.0 + 4 * randfloat()) / (_LOD + 5.0);
		size_max = 270 * _scale / (_LOD + 10);
		alpha = randfloat(0.5) + (1.0 - (_LOD + 20.0) / 60.0);
		if (state == 2)
		{
			size *= 1.9;
			alpha *= 2.0;
			alpha = std::min(alpha, 1.0f - LOD * 0.07f);
		}
		while (alpha > 1.0)
		{
			alpha *= 0.7;
			size /= 0.7f;
		}
		velocity /= size;
		flare_max = 1.5;
		flare_exp = 0.3;
		flare_frequency = 5.0;
		if (state)
			size *= 0.7;

#ifdef DEBUG_POINT_PARTICLES
		size = 10.0f;
#endif
	}

	bool CampfireParticle::idle(const Uint64 delta_t)
	{
		if (effect->recall)
			return false;

#ifdef DEBUG_POINT_PARTICLES
		coord_t tempsize = base->temp_sprite_scalar * size * invsqrt(square(pos.x - base->camera.x) + square(pos.y - base->camera.y) + square(pos.z - base->camera.z));
		tempsize *= flare();
		std::stringstream strstr;
		strstr << "Position: " << pos << "; Camera: " << base->camera << "; Distance: " << (pos - base->camera).magnitude() << "; Size: " << size << "; Draw size: " << tempsize << "; Base sprite scalar: " << base->temp_sprite_scalar;
		char buffer[1024];
		strstr.get(buffer, sizeof(buffer));
		buffer[sizeof(buffer) - 1] = '\0';
		std::string s(buffer);
		std::cout << s << std::endl;
		log_warning(s);
		return true;
#endif

		if (alpha < 0.12 - LOD * 0.01)
			return false;

		const float scalar= square(std::pow(0.5f, (interval_t)delta_t / (3000000 - LOD * 200000)));

		if (state == 0)
			alpha = alpha * square(scalar) * 0.6 + alpha * 0.4;
		else if (state == 1)
			alpha *= scalar;
		else
			alpha = alpha * scalar * 0.45 + alpha * 0.55;

		if (state != 2)
		{
			velocity.x = velocity.x * scalar;
			velocity.z = velocity.z * scalar;
		}
		size = std::min(size_max, size / scalar * 0.3f + size * 0.7f);

		return true;
	}

#ifdef	NEW_TEXTURES
	Uint32 CampfireParticle::get_texture()
	{
		if (state == 0)
		{
			return base->get_texture(EC_FLARE);
		}
		else
		{
			return base->get_texture(EC_SIMPLE);
		}
	}

	float CampfireParticle::get_burn() const
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
	GLuint CampfireParticle::get_texture(const Uint16 res_index)
	{
		if (state == 0)
			return base->TexFlare.get_texture(res_index);
		else
			return base->TexSimple.get_texture(res_index);
	}

	void CampfireParticle::draw(const Uint64 usec)
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

	CampfireBigParticle::CampfireBigParticle(Effect* _effect,
		ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity,
		const color_t hue_adjust, const color_t saturation_adjust,
		const float _sqrt_scale, const Uint16 _LOD) :
		Particle(_effect, _mover, _pos, _velocity)
	{
		const float LOD = _LOD / 10.0;
		color[0] = 1.0;
		color[1] = 0.5 + randfloat() / 2;
		color[2] = 0.3;
		color_t hue, saturation, value;
		hue = 0.03 + randcolor(0.1);
		saturation = 0.8;
		value = 1.0;
		hue += hue_adjust;
		if (hue > 1.0)
			hue -= 1.0;
		saturation = std::min(1.0f, saturation * saturation_adjust);
		hsv_to_rgb(hue, saturation, value, color[0], color[1], color[2]);
		size = 7.5 * (2.0 + randfloat()) / 2.5;
		alpha = std::min(1.0f, 7.0f / size / (LOD + 5));
		size *= _sqrt_scale;
		velocity = Vec3(0.0, 0.0, 0.0);
		flare_max = 1.0;
		flare_exp = 0.0;
		flare_frequency = 2.0;
	}

	bool CampfireBigParticle::idle(const Uint64 delta_t)
	{
		if (effect->recall)
			return false;

		return true;
	}

#ifdef	NEW_TEXTURES
	Uint32 CampfireBigParticle::get_texture()
	{
		return base->get_texture(EC_FLARE);
	}
#else	/* NEW_TEXTURES */
	GLuint CampfireBigParticle::get_texture(const Uint16 res_index)
	{
		return base->TexFlare.get_texture(res_index);
	}
#endif	/* NEW_TEXTURES */

	CampfireEffect::CampfireEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
		std::vector<ec::Obstruction*>* _obstructions, const color_t _hue_adjust,
		const color_t _saturation_adjust, const float _scale, const Uint16 _LOD)
	{
		if (EC_DEBUG)
			std::cout << "CampfireEffect (" << this << ") created."
				<< std::endl;
		base = _base;
		dead = _dead;
		pos = _pos;
		obstructions = _obstructions;
		hue_adjust = _hue_adjust;
		saturation_adjust = _saturation_adjust;
		scale = _scale * 0.7;
		sqrt_scale = std::sqrt(scale);
		LOD = base->last_forced_LOD;
		desired_LOD = _LOD;
		bounds = NULL;
#ifdef DEBUG_POINT_PARTICLES
		mover = new ParticleMover(this);
#else
		mover = new SmokeMover(this);
#endif
		stationary = new ParticleMover(this);
		spawner = new FilledSphereSpawner(0.15 * sqrt_scale);
		active = true;
		/*
		 for (int i = 0; i < LOD * 10; i++)
		 {
		 const Vec3 coords = spawner->get_new_coords() + *pos + Vec3(0.0, 0.10, 0.0);
		 Vec3 velocity;
		 velocity.randomize(0.15 * sqrt_scale);
		 velocity += Vec3(0.0, 0.15 * scale, 0.0);
		 Particle* p = new CampfireParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, scale, sqrt_scale, 0, LOD);
		 if (!base->push_back_particle(p))
		 break;
		 }
		 */
#ifdef DEBUG_POINT_PARTICLES
		Particle* p = new CampfireParticle(this, mover, *pos + Vec3(0.0, 0.2, 0.0), Vec3(0.0, 0.0, 0.0), hue_adjust, saturation_adjust, 10.0, sqrt(10.0), 0, 10);
		base->push_back_particle(p);
#else
#ifndef MAP_EDITOR
		big_particles = 0;
		for (int i = 0; i < 20; i++)
		{
			const Vec3 coords = spawner->get_new_coords() * 1.3 + *pos + Vec3(
				0.0, 0.15, 0.0);
			Particle
				* p =
					new CampfireBigParticle(this, stationary, coords, Vec3(0.0, 0.0, 0.0), hue_adjust, saturation_adjust, sqrt_scale, LOD);
			if (!base->push_back_particle(p))
				break;
			big_particles++;
		}
#endif
#endif
	}

	CampfireEffect::~CampfireEffect()
	{
		delete mover;
		delete spawner;
		delete stationary;
		if (EC_DEBUG)
			std::cout << "CampfireEffect (" << this << ") destroyed."
				<< std::endl;
	}

	bool CampfireEffect::idle(const Uint64 usec)
	{
		if ((recall) && (particles.size() == 0))
			return false;

#ifdef DEBUG_POINT_PARTICLES
		return true;
#endif

		if (recall)
			return true;

		while (((int)particles.size() < LOD * 100)
			&& (pow_randfloat((interval_t)usec / 80000 * LOD) < 0.5))
		{
			int state = 0;
			if (rand() & 1)
				state = 1;
			else if (randfloat() < 0.15) // Smoke
				state = 2;

			Vec3 coords;
			if (state == 0)
				coords = spawner->get_new_coords() + *pos
					+ Vec3(0.0, 0.07, 0.0);
			else if (state == 1)
				coords = spawner->get_new_coords() * 0.3 + *pos + Vec3(0.0,
					0.12, 0.0);
			else
				coords = spawner->get_new_coords() + *pos + Vec3(0.0, 0.2
					+ randfloat(0.5), 0.0);
			Vec3 velocity;
			velocity.randomize(0.3 * sqrt_scale);
			velocity.y = velocity.y * 0.5 + 0.2 * scale;
			if (state == 2)
			{
				velocity.x *= 1.5;
				velocity.y *= 1.5;
				velocity.z *= 1.5;
			}
			Particle
				* p =
					new CampfireParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, scale, sqrt_scale, state, LOD);
			if (!base->push_back_particle(p))
				break;
		}

		return true;
	}

///////////////////////////////////////////////////////////////////////////////

}
;

