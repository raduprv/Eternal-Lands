/*!
 \brief Special effects for all of the harvesting events.
 */

#ifndef EFFECT_HARVESTING_H
#define EFFECT_HARVESTING_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

	// C L A S S E S //////////////////////////////////////////////////////////////

	class HarvestingEffect : public Effect
	{
		public:
			enum HarvestingType
			{
				RADON_POUCH,
				CAVERN_WALL,
				MOTHER_NATURE,
				QUEEN_OF_NATURE,
				BEES,
				BAG_OF_GOLD,
				RARE_STONE,
				TOOL_BREAK,
			};

			HarvestingEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
				const HarvestingType _type, const Uint16 _LOD);
			HarvestingEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, Vec3* _pos2,
				const HarvestingType _type, const Uint16 _LOD);
			~HarvestingEffect();

			virtual EffectEnum get_type()
			{
				return EC_HARVESTING;
			}
			;
			bool idle(const Uint64 usec);
			static Uint64 get_max_end_time()
			{
				return 5000000;
			}
			;
			virtual Uint64 get_expire_time()
			{
				return 5000000 + born;
			}
			;

			ParticleSpawner* spawner;
			ParticleMover* mover;
			ParticleSpawner* spawner2;
			ParticleMover* mover2;
			Vec3 effect_center;
			Vec3 gravity_center;
			Vec3 direction;
			Vec3* pos2;
			HarvestingType type;
	};

	class HarvestingParticle : public Particle
	{
		public:
			HarvestingParticle(Effect* _effect, ParticleMover* _mover,
				const Vec3 _pos, const Vec3 _velocity, const coord_t _size,
				const alpha_t _alpha, const color_t red, const color_t green,
#ifdef	NEW_TEXTURES
				const color_t blue, TextureEnum _texture, const Uint16 _LOD,
#else	/* NEW_TEXTURES */
				const color_t blue, Texture* _texture, const Uint16 _LOD,
#endif	/* NEW_TEXTURES */
				const HarvestingEffect::HarvestingType _type);
			~HarvestingParticle()
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
				return 0.0015;
			}
			;
			virtual light_t get_light_level();

#ifdef	NEW_TEXTURES
			TextureEnum texture;
#else	/* NEW_TEXTURES */
			Texture* texture;
#endif	/* NEW_TEXTURES */
			Uint16 LOD;
			HarvestingEffect::HarvestingType type;
	};

///////////////////////////////////////////////////////////////////////////////

} // End namespace ec

#endif	// defined EFFECT_HARVESTING_H
