/*!
 \brief Special effects for elements focused on a target that don't disappear
 right away.
 */

#ifndef EFFECT_ONGOING_H
#define EFFECT_ONGOING_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

	// C L A S S E S //////////////////////////////////////////////////////////////

	class OngoingEffect : public Effect
	{
		public:
			enum OngoingType
			{
				OG_MAGIC_PROTECTION,
				OG_SHIELD,
				OG_MAGIC_IMMUNITY,
				OG_POISON,
				OG_HARVEST,
			};

			OngoingEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
				const color_t _hue_adjust, const color_t _saturation_adjust,
				const OngoingType _type, const Uint16 _LOD,
				const float _strength, Uint32 _buff_type);
			~OngoingEffect();

			virtual EffectEnum get_type()
			{
				return EC_ONGOING;
			}
			;
			bool idle(const Uint64 usec);

			ParticleSpawner* spawner;
			ParticleMover* mover;
			color_t hue_adjust;
			color_t saturation_adjust;
			Vec3 effect_center;
			OngoingType type;
			float strength;
			Vec3 initial_center;
			Uint32 buff_type;
	};

	class OngoingParticle : public Particle
	{
		public:
			OngoingParticle(Effect* _effect, ParticleMover* _mover,
				const Vec3 _pos, const Vec3 _velocity,
				const color_t hue_adjust, const color_t saturation_adjust,
				const coord_t _size, const alpha_t _alpha, color_t hue,
				color_t saturation, color_t value, TextureEnum _texture,
				const Uint16 _LOD, const OngoingEffect::OngoingType _type);
			OngoingParticle(Effect* _effect, ParticleMover* _mover,
				const Vec3 _pos, const Vec3 _velocity,
				const color_t hue_adjust, const color_t saturation_adjust,
				const coord_t _size, const alpha_t _alpha, color_t hue,
				color_t saturation, color_t value, TextureEnum _texture,
				const Uint16 _LOD, const OngoingEffect::OngoingType _type,
				const angle_t _angle);
			~OngoingParticle()
			{
			}

			virtual bool idle(const Uint64 delta_t);
			virtual Uint32 get_texture();
			virtual float get_burn() const;
			virtual light_t estimate_light_level() const
			{
				return 0.002;
			}
			;

			TextureEnum texture;
			Uint16 LOD;
			OngoingEffect::OngoingType type;
			angle_t angle;
			Vec3 center;
	};

///////////////////////////////////////////////////////////////////////////////

} // End namespace ec

#endif	// defined EFFECT_ONGOING_H
