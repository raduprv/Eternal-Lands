/*!
 \brief Special effects for making magical staffs sparkle as they're swung.
 */

#ifndef EFFECT_STAFF_H
#define EFFECT_STAFF_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

	// C L A S S E S //////////////////////////////////////////////////////////////

	class StaffParticle : public Particle
	{
		public:
			StaffParticle(Effect* _effect, ParticleMover* _mover,
				const Vec3 _pos, const Vec3 _velocity, const coord_t _size,
				const alpha_t _alpha, const color_t red, const color_t green,
#ifdef	NEW_TEXTURES
				const color_t blue, TextureEnum _texture, const Uint16 _LOD);
#else	/* NEW_TEXTURES */
				const color_t blue, Texture* _texture, const Uint16 _LOD);
#endif	/* NEW_TEXTURES */
			~StaffParticle()
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
	};

	class StaffEffect : public Effect
	{
		public:
			enum StaffType
			{
				STAFF_OF_THE_MAGE,
				STAFF_OF_PROTECTION,
			};

			StaffEffect(EyeCandy* _base, bool* _dead, Vec3* _end,
				const StaffType _type, const Uint16 _LOD);
			~StaffEffect();

			virtual EffectEnum get_type()
			{
				return EC_STAFF;
			}
			;
			bool idle(const Uint64 usec);
			virtual void request_LOD(const float _LOD);

			ParticleMover* mover;
			Vec3 old_end;
			coord_t size;
			alpha_t alpha;
			color_t color[3];
#ifdef	NEW_TEXTURES
			TextureEnum texture;
#else	/* NEW_TEXTURES */
			Texture* texture;
#endif	/* NEW_TEXTURES */
			StaffType type;
	};

///////////////////////////////////////////////////////////////////////////////

} // End namespace ec

#endif	// defined EFFECT_STAFF_H
