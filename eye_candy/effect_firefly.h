/*!
 \brief A special effect that spawns a region full of fireflies.
 */

#ifndef EFFECT_FIREFLY_H
#define EFFECT_FIREFLY_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

	// C L A S S E S //////////////////////////////////////////////////////////////

	class FireflyParticle : public Particle
	{
		public:
			FireflyParticle(Effect* _effect, ParticleMover* _mover,
				const Vec3 _pos, const Vec3 _velocity,
				const color_t hue_adjust, const color_t saturation_adjust,
				const coord_t _size, const coord_t _min_height,
				const coord_t _max_height);
			~FireflyParticle()
			{
			}

			virtual bool idle(const Uint64 delta_t);
#ifdef	NEW_TEXTURES
			virtual Uint32 get_texture();
#else	/* NEW_TEXTURES */
			virtual GLuint get_texture(const Uint16 res_index);
#endif	/* NEW_TEXTURES */
			virtual light_t estimate_light_level() const
			{
				return 0.0;
			}
			; // We don't want the particle system lights to be used on the pos, since it will assumedly already have one.
			virtual light_t get_light_level()
			{
				return 0.0;
			}
			;
			virtual bool deletable()
			{
				return false;
			}
			;

			coord_t min_height;
			coord_t max_height;
	};

	class FireflyEffect : public Effect
	{
		public:
			FireflyEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
				std::vector<ec::Obstruction*>* _obstructions,
				const color_t _hue_adjust, const color_t _saturation_adjust,
				const float _density, const float _size,
				BoundingRange* bounding_range);
			~FireflyEffect();

			virtual EffectEnum get_type()
			{
				return EC_FIREFLY;
			}
			;
			bool idle(const Uint64 usec);

			BoundingMover* mover;
			NoncheckingFilledBoundingSpawner* spawner;
			color_t hue_adjust;
			color_t saturation_adjust;
			Vec3 center;
			int firefly_count;
			float size;
	};

///////////////////////////////////////////////////////////////////////////////

} // End namespace ec

#endif	// defined EFFECT_FIREFLY_H
