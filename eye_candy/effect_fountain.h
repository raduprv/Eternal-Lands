/*!
 \brief A special effect that sprays up water, which then falls and splatters.
 */

#ifndef EFFECT_FOUNTAIN_H
#define EFFECT_FOUNTAIN_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

	// C L A S S E S //////////////////////////////////////////////////////////////

	class FountainParticle : public Particle
	{
		public:
			FountainParticle(Effect* _effect, ParticleMover* _mover,
				const Vec3 _pos, const Vec3 _velocity,
				const color_t hue_adjust, const color_t saturation_adjust,
				const coord_t _base_height, const bool _backlight,
				const float _sqrt_scale, const coord_t _max_size,
				const coord_t size_scalar);
			~FountainParticle()
			{
			}

			virtual bool idle(const Uint64 delta_t);
#ifdef	NEW_TEXTURES
			virtual Uint32 get_texture();
			virtual float get_burn() const;
#else	/* NEW_TEXTURES */
			virtual GLuint get_texture(const Uint16 res_index);
			virtual void draw(const Uint64 usec);
#endif	/* NEW_TEXTURES */
			virtual light_t estimate_light_level() const
			{
				return 0.0;
			}
			; // No glow.
			virtual light_t get_light_level()
			{
				return 0.0;
			}
			; // Same.

			coord_t base_height;
			bool backlight;
			float sqrt_scale;
			coord_t max_size;
	};

	class FountainEffect : public Effect
	{
		public:
			FountainEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
				const color_t _hue_adjust, const color_t _saturation_adjust,
				const bool _backlight, const coord_t _base_height,
				const float _scale, const Uint16 _LOD);
			~FountainEffect();

			virtual EffectEnum get_type()
			{
				return EC_FOUNTAIN;
			}
			;
			bool idle(const Uint64 usec);
			virtual void request_LOD(const float _LOD)
			{
				if (fabs(_LOD - (float)LOD) < 1.0)
					return;
				const Uint16 rounded_LOD = (Uint16)round(_LOD);
				if (rounded_LOD <= desired_LOD)
					LOD = rounded_LOD;
				else
					LOD = desired_LOD;
				max_size = 3 * scale * 90 / (_LOD + 10);
				size_scalar = sqrt_scale * 6 / (_LOD + 5);
				count_scalar = 15000 / LOD;
			}
			;

			GradientMover* mover;
			ParticleMover* basic_mover;
			ParticleSpawner* spawner;
			color_t hue_adjust;
			color_t saturation_adjust;
			int big_particles;
			interval_t count;
			coord_t base_height;
			bool backlight;
			float scale;
			float sqrt_scale;
			coord_t max_size;
			coord_t size_scalar;
			Uint32 count_scalar;
	};

///////////////////////////////////////////////////////////////////////////////

} // End namespace ec

#endif	// defined EFFECT_FOUNTAIN_H
