/*!
 \brief A special effect involving a small flame and a bright halo, useful
 for lanterns and torches.
 */

#ifndef EFFECT_LAMP_H
#define EFFECT_LAMP_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

	// C L A S S E S //////////////////////////////////////////////////////////////

	class LampEffect : public Effect
	{
		public:
			LampEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
				const color_t _hue_adjust, const color_t _saturation_adjust,
				const float scale, const bool halo, const Uint16 _LOD);
			~LampEffect();

			virtual EffectEnum get_type()
			{
				return EC_LAMP;
			}
			;
			bool idle(const Uint64 usec);

			GradientMover* mover;
			GradientMover* mover2;
			ParticleMover* mover3;
			ParticleSpawner* spawner;
			color_t hue_adjust;
			color_t saturation_adjust;
			int big_particles;
			float scale;
			float new_scale;
			float sqrt_scale;
			bool halo;
	};

	class LampParticle : public Particle
	{
		public:
			LampParticle(Effect* _effect, ParticleMover* _mover,
				const Vec3 _pos, const Vec3 _velocity,
				const color_t hue_adjust, const color_t saturation_adjust,
				const float _scale, const Uint16 _LOD);
			~LampParticle()
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
			; // We don't want the particle system lights to be used on the lamp, since it will assumedly already have one.
			virtual light_t get_light_level()
			{
				return 0.0;
			}
			; // Same.

			Uint16 LOD;
	};

	class LampBigParticle : public Particle
	{
		public:
			LampBigParticle(Effect* _effect, ParticleMover* _mover,
				const Vec3 _pos, const Vec3 _velocity,
				const color_t hue_adjust, const color_t saturation_adjust,
				const float _scale, const Uint16 _LOD);
			~LampBigParticle()
			{
				((LampEffect*)effect)->big_particles--;
			}
			;

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
			; // Like above
			virtual light_t get_light_level()
			{
				return 0.0;
			}
			; // Same.

			Uint16 LOD;
	};

	class LampFlareParticle : public Particle
	{
		public:
			LampFlareParticle(Effect* _effect, ParticleMover* _mover,
				const Vec3 _pos, const Vec3 _velocity, const float _scale);
			~LampFlareParticle()
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
			; // Like above
			virtual light_t get_light_level()
			{
				return 0.0;
			}
			; // Same.
			virtual bool deletable()
			{
				return false;
			}
			;

			Vec3 true_pos;
			coord_t true_size;
	};

///////////////////////////////////////////////////////////////////////////////

} // End namespace ec

#endif	// defined EFFECT_LAMP_H
