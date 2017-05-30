/*!
 \brief Special effects for magic cast on one's self
 */

#ifndef EFFECT_SELFMAGIC_H
#define EFFECT_SELFMAGIC_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

	// C L A S S E S //////////////////////////////////////////////////////////////

	class SelfMagicEffect : public Effect
	{
		public:
			enum SelfMagicType
			{
				HEAL,
				MAGIC_PROTECTION,
				SHIELD,
				HEATSHIELD,
				COLDSHIELD,
				RADIATIONSHIELD,
				RESTORATION,
				BONES_TO_GOLD,
				TELEPORT_TO_THE_PORTALS_ROOM,
				MAGIC_IMMUNITY,
				ALERT // Not really a spell, but functions like one.
			};

			SelfMagicEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
				const SelfMagicType _type, const Uint16 _LOD);
			~SelfMagicEffect();

			virtual EffectEnum get_type()
			{
				return EC_SELFMAGIC;
			}
			;
			bool idle(const Uint64 usec);
			void draw(const Uint64 usec);
			static Uint64 get_max_end_time()
			{
				return 4000000;
			}
			;
			virtual Uint64 get_expire_time()
			{
				return 4000000 + born;
			}
			;

			ParticleSpawner* spawner;
			ParticleMover* mover;
			ParticleSpawner* spawner2;
			ParticleMover* mover2;
			Vec3 effect_center;
			Vec3 shift;
			Sint64 count;
			Uint64 count_scalar;
			SelfMagicType type;
			CaplessCylinders* capless_cylinders;
			float alpha_scale;
			coord_t size_scalar;
			float* target_alpha;
	};

	class SelfMagicParticle : public Particle
	{
		public:
			SelfMagicParticle(Effect* _effect, ParticleMover* _mover,
				const Vec3 _pos, const Vec3 _velocity, const coord_t _size,
				const alpha_t _alpha, const color_t red, const color_t green,
				const color_t blue, TextureEnum _texture, const Uint16 _LOD,
				const SelfMagicEffect::SelfMagicType _type);
			~SelfMagicParticle()
			{
			}

			virtual bool idle(const Uint64 delta_t);
			virtual Uint32 get_texture();
			virtual light_t estimate_light_level() const
			{
				return 0.002;
			}
			;

			TextureEnum texture;
			Uint16 LOD;
			SelfMagicEffect::SelfMagicType type;
	};

///////////////////////////////////////////////////////////////////////////////

} // End namespace ec

#endif	// defined EFFECT_SELFMAGIC_H
