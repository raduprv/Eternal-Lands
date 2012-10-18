// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_fountain.h"

namespace ec
{

	// C L A S S   F U N C T I O N S //////////////////////////////////////////////

	FountainParticle::FountainParticle(Effect* _effect, ParticleMover* _mover,
		const Vec3 _pos, const Vec3 _velocity, const color_t hue_adjust,
		const color_t saturation_adjust, const coord_t _base_height,
		const bool _backlight, const float _sqrt_scale,
		const coord_t _max_size, const coord_t size_scalar) :
		Particle(_effect, _mover, _pos, _velocity,
			size_scalar * (0.5 + 5 * randcoord()))
	{
		base_height = _base_height;
		backlight = _backlight;
		sqrt_scale = _sqrt_scale;
		max_size = _max_size;
		color_t hue, saturation, value;
		hue = 0.6;
		saturation = 0.3;
		value = 0.85;
		hue += hue_adjust;
		if (hue > 1.0)
			hue -= 1.0;
		saturation = std::min(1.0f, saturation * saturation_adjust);
		hsv_to_rgb(hue, saturation, value, color[0], color[1], color[2]);
		color[0] *= 2.0;
		color[1] *= 2.0;
		color[2] *= 2.0;
		alpha = sqrt_scale * 3.5 / size;
		if (backlight)
			alpha /= 9;
		alpha = std::min(1.0f, alpha);
		flare_max = 1.5;
		flare_exp = 0.3;
		flare_frequency = 5.0;
		state = 0;
	}

#ifdef	NEW_TEXTURES
	float FountainParticle::get_burn() const
	{
		if (backlight)
		{
			return 1.0f;
		}
		else
		{
			return 0.0f;
		}
	}
#else	/* NEW_TEXTURES */
	void FountainParticle::draw(const Uint64 usec)
	{
		if (!backlight)
		{
			glEnable(GL_LIGHTING);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			Vec3 shifted_pos = pos - *(((FountainEffect*)effect)->pos);
#if 0	// Clear, but slow.  Consider this a comment.
			Vec3 velocity2 = velocity;
			//    velocity2.normalize();
			shifted_pos.normalize();
			Vec3 cross = velocity2.cross(shifted_pos).cross(velocity2);
			glNormal3f(cross.x, cross.y, cross.z);
#else	// Faster, but obfuscated.  NOTE: For some reason, it's not producing the same output!  However, I like the new output better.
			const coord_t vel_mag_squared = velocity.magnitude_squared();
			const coord_t sp_mag = shifted_pos.magnitude();
			const coord_t scalar = vel_mag_squared * sp_mag;
			const coord_t vx_squared = velocity.x * velocity.x;
			const coord_t vy_squared = velocity.y * velocity.y;
			const coord_t vz_squared = velocity.z * velocity.z;
			const coord_t x = (shifted_pos.x * (vz_squared - vy_squared)
				- velocity.x * (shifted_pos.z * velocity.z + shifted_pos.y
					* velocity.y)) / scalar;
			const coord_t y = (shifted_pos.y * (vx_squared - vz_squared)
				- velocity.y * (shifted_pos.x * velocity.x + shifted_pos.z
					* velocity.z)) / scalar;
			const coord_t z = (shifted_pos.z * (vy_squared - vx_squared)
				- velocity.z * (shifted_pos.y * velocity.y + shifted_pos.x
					* velocity.x)) / scalar;
			glNormal3f(-x, -y, -z);
#endif
		}
		Particle::draw(usec);
		if (!backlight)
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glDisable(GL_LIGHTING);
		}
	}
#endif	/* NEW_TEXTURES */

	bool FountainParticle::idle(const Uint64 delta_t)
	{
		if (effect->recall)
			return false;

		if (state == 0)
		{
			const float scalar = std::pow(0.5f, (float)delta_t
				/ 2500000);
			alpha *= scalar;
			size = std::min(max_size, size / scalar * 0.25f + size * 0.75f);

			if (pos.y < base_height)
			{
				state = 1;
				velocity.randomize(sqrt_scale);
				velocity.y = -velocity.y / 10;
				pos.y = base_height;
			}
		}
		else
		{
			if (alpha < 0.02)
				return false;

			const float scalar = std::pow(0.5f, (float)delta_t
				/ 200000);
			alpha *= scalar;
			size = std::min(max_size, size / scalar * 0.25f + size * 0.75f);
		}

		return true;
	}

#ifdef	NEW_TEXTURES
	Uint32 FountainParticle::get_texture()
	{
		if (state == 1)
		{
			return base->get_texture(EC_WATER);
		}
		else
		{
			return base->get_texture(EC_FLARE);
		}
	}
#else	/* NEW_TEXTURES */
	GLuint FountainParticle::get_texture(const Uint16 res_index)
	{
		if (state == 1)
			return base->TexWater.get_texture(res_index);
		else
			return base->TexFlare.get_texture(res_index);
	}
#endif	/* NEW_TEXTURES */

	FountainEffect::FountainEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
		const color_t _hue_adjust, const color_t _saturation_adjust,
		const bool _backlight, const coord_t _base_height, const float _scale,
		const Uint16 _LOD)
	{
		if (EC_DEBUG)
			std::cout << "FountainEffect (" << this << ") created."
				<< std::endl;
		base = _base;
		dead = _dead;
		pos = _pos;
		count = 0;
		hue_adjust = _hue_adjust;
		saturation_adjust = _saturation_adjust;
		backlight = _backlight;
		scale = _scale;
		sqrt_scale = std::sqrt(scale);
		max_size = 3 * scale * 90 / (_LOD + 10);
		size_scalar = sqrt_scale * 4.5 / (_LOD + 5);
		base_height = _base_height + 0.1;
		count_scalar = 15000 / _LOD;
		LOD = base->last_forced_LOD;
		desired_LOD = _LOD;
		bounds = NULL;
		mover = new SimpleGravityMover(this);
		basic_mover = new ParticleMover(this);
		spawner = new FilledSphereSpawner(0.05 * sqrt_scale);

		/*  
		 for (int i = 0; i < LOD * 4; i++)
		 {
		 const Vec3 coords = spawner->get_new_coords() + *pos + Vec3(0.0, 0.1 * sqrt_scale, 0.0);
		 Vec3 velocity;
		 velocity.randomize(0.1 * sqrt_scale);
		 velocity += Vec3(0.0, 0.8 * scale, 0.0);
		 Particle* p = new FountainParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, pos->y, backlight, sqrt_scale, max_size, size_scalar);
		 if (!base->push_back_particle(p))
		 break;
		 }
		 */
	}

	FountainEffect::~FountainEffect()
	{
		delete mover;
		delete basic_mover;
		delete spawner;
		if (EC_DEBUG)
			std::cout << "FountainEffect (" << this << ") destroyed."
				<< std::endl;
	}

	bool FountainEffect::idle(const Uint64 usec)
	{
		if ((recall) && (particles.size() == 0))
			return false;

		if (recall)
			return true;

		count += usec;

		while (count > 0)
		{
			const Vec3 coords = spawner->get_new_coords() + *pos + Vec3(0.0,
				0.1 * sqrt_scale, 0.0);
			Vec3 velocity;
			velocity.randomize(0.2 * sqrt_scale);
			velocity.y = 0.0;
			velocity.normalize((0.15 + randfloat(0.1)) * sqrt_scale);
			velocity += Vec3(0.0, 1.2 * scale, 0.0);
			Particle
				* p =
					new FountainParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, base_height, backlight, sqrt_scale, max_size, size_scalar);
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

