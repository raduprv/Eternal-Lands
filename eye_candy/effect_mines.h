/*!
 \brief Special effects for all of the harvesting events.
 */

#ifndef EFFECT_MINES_H
#define EFFECT_MINES_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

	// C L A S S E S //////////////////////////////////////////////////////////////

	class MineEffect : public Effect
	{
		public:
			enum MineType
			{
				DETONATE_TYPE1_SMALL,
				DETONATE_TYPE1_MEDIUM,
				DETONATE_TYPE1_LARGE,
				DETONATE_TRAP,
				DETONATE_CALTROP,
				DETONATE_CALTROP_POISON,
				DETONATE_MANA_DRAINER,
				DETONATE_MANA_BURNER,
				DETONATE_UNINVIZIBILIZER,
				DETONATE_MAGIC_IMMUNITY_REMOVAL
			};

			MineEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
				const MineType _type, const Uint16 _LOD);
			~MineEffect();

			virtual EffectEnum get_type()
			{
				return EC_MINES;
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
			MineType type;
	};

	class MineParticle : public Particle
	{
		public:
			MineParticle(Effect* _effect, ParticleMover* _mover,
				const Vec3 _pos, const Vec3 _velocity, const coord_t _size,
				const alpha_t _alpha, const color_t red, const color_t green,
				const color_t blue, TextureEnum _texture, const Uint16 _LOD,
				const MineEffect::MineType _type);
			~MineParticle()
			{
			}

			virtual bool idle(const Uint64 delta_t);
			virtual Uint32 get_texture();
			virtual float get_burn() const;
			virtual light_t estimate_light_level() const
			{
				return 0.0015;
			}
			;
			virtual light_t get_light_level();

			TextureEnum texture;
			Uint16 LOD;
			MineEffect::MineType type;
	};

	class MineParticleFire : public Particle
	{
		public:
			MineParticleFire(Effect* _effect, ParticleMover* _mover,
				const Vec3 _pos, const Vec3 _velocity, const coord_t _size,
				const alpha_t _alpha, const color_t red, const color_t green,
				const color_t blue, TextureEnum _texture, const Uint16 _LOD);
			~MineParticleFire()
			{
			}

			virtual bool idle(const Uint64 delta_t);
			virtual Uint32 get_texture();
			virtual light_t estimate_light_level() const
			{
				return 0.0015;
			}

			TextureEnum texture;
			Uint16 LOD;
	};

	class MineParticleSmoke : public Particle
	{
		public:
			MineParticleSmoke(Effect* _effect, ParticleMover* _mover,
				const Vec3 _pos, const Vec3 _velocity, const coord_t _size,
				const alpha_t _alpha, const color_t red, const color_t green,
				const color_t blue, TextureEnum _texture, const Uint16 _LOD);
			~MineParticleSmoke()
			{
			}

			virtual bool idle(const Uint64 delta_t);
			virtual Uint32 get_texture();
			virtual float get_burn() const;
			virtual light_t estimate_light_level() const
			{
				return 0.0015;
			}

			TextureEnum texture;
			Uint16 LOD;
	};

///////////////////////////////////////////////////////////////////////////////

} // End namespace ec

#endif	// defined EFFECT_MINES_H

