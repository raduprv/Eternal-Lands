/*!
 \brief A special effect that creates a column of black smoke in the air.
 */

#ifndef EFFECT_SMOKE_H
#define EFFECT_SMOKE_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

	// C L A S S E S //////////////////////////////////////////////////////////////

	class SmokeParticle : public Particle
	{
		public:
			SmokeParticle(Effect* _effect, ParticleMover* _mover,
				const Vec3 _pos, const Vec3 _velocity,
				const color_t hue_adjust, const color_t saturation_adjust,
				const coord_t _sqrt_scale, const coord_t _max_size,
				const coord_t size_scalar, const alpha_t alpha_scale);
			~SmokeParticle()
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

			coord_t sqrt_scale;
			coord_t max_size;
	};

	class SmokeEffect : public Effect
	{
		public:
			SmokeEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
				const color_t _hue_adjust, const color_t _saturation_adjust,
				const float _scale, const Uint16 _LOD);
			~SmokeEffect();

			virtual EffectEnum get_type()
			{
				return EC_SMOKE;
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
				max_size = scale * 270 / (_LOD + 10);
				size_scalar = sqrt_scale * 75 / (_LOD + 5);
				alpha_scalar = 6.5 / (std::sqrt(_LOD) + 1.0);
				count_scalar = 500000 / LOD;
			}
			;

			ParticleMover* mover;
			ParticleSpawner* spawner;
			color_t hue_adjust;
			color_t saturation_adjust;
			interval_t count;
			float scale;
			coord_t sqrt_scale;
			coord_t max_size;
			coord_t size_scalar;
			alpha_t alpha_scalar;
			Uint32 count_scalar;
	};

///////////////////////////////////////////////////////////////////////////////

} // End namespace ec

#endif	// defined EFFECT_SMOKE_H
