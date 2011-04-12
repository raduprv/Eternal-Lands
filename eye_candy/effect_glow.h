/*!
 \brief Special effects for glows.
 */

#ifndef EFFECT_GLOW_H
#define EFFECT_GLOW_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

	// C L A S S E S //////////////////////////////////////////////////////////////

	class GlowEffect : public Effect
	{
		public:
			enum GlowType
			{
				REMOTE_HEAL_GLOW,
				HARM_GLOW,
				POISON_GLOW,
				LEVEL_UP_DEFAULT_GLOW,
				LEVEL_UP_OA_GLOW,
				LEVEL_UP_ATT_GLOW,
				LEVEL_UP_DEF_GLOW,
				LEVEL_UP_HAR_GLOW,
				LEVEL_UP_ALC_GLOW_L,
				LEVEL_UP_ALC_GLOW_R,
				LEVEL_UP_MAG_GLOW,
				LEVEL_UP_POT_GLOW_L,
				LEVEL_UP_POT_GLOW_R,
				LEVEL_UP_SUM_GLOW,
				LEVEL_UP_MAN_GLOW_L,
				LEVEL_UP_MAN_GLOW_R,
				LEVEL_UP_CRA_GLOW_L,
				LEVEL_UP_CRA_GLOW_R,
				LEVEL_UP_ENG_GLOW_L,
				LEVEL_UP_ENG_GLOW_R,
				LEVEL_UP_TAI_GLOW_L,
				LEVEL_UP_TAI_GLOW_R,
				LEVEL_UP_RAN_GLOW,
			};

			GlowEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
				const GlowType _type, const Uint16 _LOD);
			~GlowEffect();

			virtual EffectEnum get_type()
			{
				return EC_GLOW;
			}
			;
			bool idle(const Uint64 usec);
			static Uint64 get_max_end_time()
			{
				return 7500000;
			}
			;
			virtual Uint64 get_expire_time()
			{
				return 7500000 + born;
			}
			;

			ParticleSpawner* spawner;
			ParticleSpawner* spawner2;
			ParticleSpawner* spawner3;
			ParticleMover* mover;
			ParticleMover* mover2;
			ParticleMover* mover3;
			Vec3 effect_center;
			GlowType type;
			Vec3 shift;
			color_t red, green, blue;
	};

	class GlowParticle : public Particle
	{
		public:
			GlowParticle(Effect* _effect, ParticleMover* _mover,
				const Vec3 _pos, const Vec3 _velocity, const coord_t _size,
				const alpha_t _alpha, const color_t red, const color_t green,
#ifdef	NEW_TEXTURES
				const color_t blue, TextureEnum _texture, const Uint16 _LOD,
#else	/* NEW_TEXTURES */
				const color_t blue, Texture* _texture, const Uint16 _LOD,
#endif	/* NEW_TEXTURES */
				const GlowEffect::GlowType _type);
			~GlowParticle()
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
			GlowEffect::GlowType type;
	};

///////////////////////////////////////////////////////////////////////////////

} // End namespace ec

#endif	// defined EFFECT_GLOW_H
