/*!
 \brief A special effect that creates a persistant column of light with
 sparkles for teleportation.
 */

#ifndef EFFECT_TELEPORTER_H
#define EFFECT_TELEPORTER_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

	// C L A S S E S //////////////////////////////////////////////////////////////

	class TeleporterParticle : public Particle
	{
		public:
			TeleporterParticle(Effect* _effect, ParticleMover* _mover,
				const Vec3 _pos, const Vec3 _velocity,
				const color_t hue_adjust, const color_t saturation_adjust,
				const coord_t size_scalar);
			~TeleporterParticle()
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
			;
			virtual light_t get_light_level()
			{
				return 0.0;
			}
			;
	};

	class TeleporterEffect : public Effect
	{
		public:
			TeleporterEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
				const color_t _hue_adjust, const color_t _saturation_adjust,
				const float _scale, const Uint16 _LOD);
			~TeleporterEffect();

			virtual EffectEnum get_type()
			{
				return EC_TELEPORTER;
			}
			;
			bool idle(const Uint64 usec);
			void draw(const Uint64 usec);
			virtual void request_LOD(const float _LOD);
			void add_actor_alpha_pointer(float* ptr);

			ParticleMover* mover;
			ParticleSpawner* spawner;
#ifdef	NEW_TEXTURES
			CaplessCylinders* capless_cylinders;
#else	/* NEW_TEXTURES */
			std::vector<Shape*> capless_cylinders;
#endif	/* NEW_TEXTURES */
			color_t hue_adjust;
			color_t saturation_adjust;
			float sqrt_LOD;
			coord_t size_scalar;
			coord_t radius;
			coord_t scale;
			Vec3 teleporter_color;
			std::vector< std::pair<float*, Uint64> > targets;
	};

///////////////////////////////////////////////////////////////////////////////

} // End namespace ec

#endif	// defined EFFECT_TELEPORTER_H
