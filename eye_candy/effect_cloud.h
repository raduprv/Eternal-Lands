/*!
 \brief A special effect that cloud; when low down, it looks like slowly
 drifting fog.
 */

#ifndef EFFECT_CLOUD_H
#define EFFECT_CLOUD_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

	// C L A S S E S //////////////////////////////////////////////////////////////

	class CloudParticle : public Particle
	{
		public:
			CloudParticle(Effect* _effect, ParticleMover* _mover,
				const Vec3 _pos, const Vec3 _velocity,
				const color_t hue_adjust, const color_t saturation_adjust,
				const coord_t _min_height, const coord_t _max_height,
				const coord_t _size, const alpha_t _alpha);
			~CloudParticle()
			{
			}
			;

			virtual bool idle(const Uint64 delta_t);
			void remove_neighbor(const CloudParticle*const p);
			void add_incoming_neighbor(CloudParticle*const p);
			void remove_incoming_neighbor(const CloudParticle*const p);
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
			; // Clouds don't glow.  :)
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
			std::vector<CloudParticle*> neighbors;
			std::vector<CloudParticle*> incoming_neighbors;
			Vec3 normal;
			light_t brightness;
	};

	class CloudEffect : public Effect
	{
		public:
			CloudEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
				const color_t _hue_adjust, const color_t _saturation_adjust,
				const float _density, BoundingRange* bounding_range,
				const Uint16 _LOD);
			~CloudEffect();

			virtual EffectEnum get_type()
			{
				return EC_CLOUD;
			}
			;
			bool idle(const Uint64 usec);

			BoundingMover* mover;
			NoncheckingFilledBoundingSpawner* spawner;
			color_t hue_adjust;
			color_t saturation_adjust;
			Vec3 center;
			alpha_t alpha;
			coord_t size_scalar;
			int count;
	};

///////////////////////////////////////////////////////////////////////////////

} // End namespace ec

#endif	// defined EFFECT_CLOUD_H
