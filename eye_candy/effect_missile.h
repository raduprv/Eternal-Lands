/*!
 \brief Special effects for special missiles (fire, ice, explosive...)
 */

#ifndef EFFECT_MISSILE_H
#define EFFECT_MISSILE_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

	// C L A S S E S //////////////////////////////////////////////////////////////

	class MissileEffect : public Effect
	{
		public:
			enum MissileType
			{
				MAGIC,
				FIRE,
				ICE,
				EXPLOSIVE,
			};

				MissileEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
					const MissileType _type, const Uint16 _LOD,
					const int _hitOrMiss);
			~MissileEffect();

			virtual EffectEnum get_type()
			{
				return EC_MISSILE;
			}
			;
			bool idle(const Uint64 usec);
			virtual void request_LOD(const float _LOD);

			ParticleMover* mover;
			Vec3 old_pos;
			coord_t size;
			alpha_t alpha;
			color_t color[3];
#ifdef	NEW_TEXTURES
			TextureEnum texture;
#else	/* NEW_TEXTURES */
			Texture* texture;
#endif	/* NEW_TEXTURES */
			MissileType type;
			// keep hitOrMiss in sync with missiles.h!
			/*
			 typedef enum {
			 MISSED_SHOT = 0,
			 NORMAL_SHOT = 1,
			 CRITICAL_SHOT = 2
			 } MissileShotType;
			 */
			int hitOrMiss;
	};

	class MissileParticle : public Particle
	{
		public:
			MissileParticle(Effect* _effect, ParticleMover* _mover,
				const Vec3 _pos, const Vec3 _velocity, const coord_t _size,
				const alpha_t _alpha, const color_t red, const color_t green,
#ifdef	NEW_TEXTURES
				const color_t blue, TextureEnum _texture, const Uint16 _LOD,
#else	/* NEW_TEXTURES */
				const color_t blue, Texture* _texture, const Uint16 _LOD,
#endif	/* NEW_TEXTURES */
				const MissileEffect::MissileType _type);
			~MissileParticle()
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

#ifdef	NEW_TEXTURES
			TextureEnum texture;
#else	/* NEW_TEXTURES */
			Texture* texture;
#endif	/* NEW_TEXTURES */
			Uint16 LOD;
			MissileEffect::MissileType type;
	};

///////////////////////////////////////////////////////////////////////////////

} // End namespace ec

#endif	// defined EFFECT_MISSILE_H
