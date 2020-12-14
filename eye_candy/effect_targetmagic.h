/*!
 \brief Special effects for magic that requires a target.
 */

#ifndef EFFECT_TARGETMAGIC_H
#define EFFECT_TARGETMAGIC_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

	// C L A S S E S //////////////////////////////////////////////////////////////

	class TargetMagicEffect : public Effect
	{
		public:
			enum TargetMagicType
			{
				REMOTE_HEAL,
				POISON,
				TELEPORT_TO_RANGE,
				HARM,
				LIFE_DRAIN,
				HEAL_SUMMONED,
				SMITE_SUMMONED,
				DRAIN_MANA
			};

			TargetMagicEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
				Vec3* _target, const TargetMagicType _type,
				std::vector<ec::Obstruction*>* _obstructions, const Uint16 _LOD);
			TargetMagicEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
				const std::vector<Vec3*>& _targets, const TargetMagicType _type,
				std::vector<ec::Obstruction*>* _obstructions, const Uint16 _LOD);
			~TargetMagicEffect();

			void initialize(EyeCandy* _base, bool* _dead, Vec3* _pos,
				const std::vector<Vec3*>& _targets, const TargetMagicType _type,
				std::vector<ec::Obstruction*>* _obstructions, const Uint16 _LOD);

			virtual EffectEnum get_type()
			{
				return EC_TARGETMAGIC;
			}
			;
			bool idle(const Uint64 usec);
			void draw(const Uint64 usec);
			static Uint64 get_max_end_time()
			{
				return 6000000;
			}
			;
			virtual Uint64 get_expire_time()
			{
				return 6000000 + born;
			}
			;

			ParticleSpawner* spawner;
			ParticleMover* mover;
			ParticleSpawner* spawner2;
			ParticleMover* mover2;
			std::vector<Vec3*> targets;
			std::vector<Vec3> effect_centers;
			Vec3 target;
			Uint16 effect_count;
			TargetMagicType type;
			std::vector<Shape*> capless_cylinders;
			float *target_alpha;
			int particle_count;
	};

	class TargetMagicEffect2 : public Effect
	{
		public:
			TargetMagicEffect2(EyeCandy* _base, TargetMagicEffect* _effect,
				Vec3* _pos, const TargetMagicEffect::TargetMagicType _type,
				ParticleSpawner* _spawner, ParticleMover* _mover,
				float* _target_alpha, Uint16 _effect_id, const Uint16 _LOD);
			~TargetMagicEffect2();

			virtual EffectEnum get_type()
			{
				return EC_TARGETMAGIC;
			}
			;
			bool idle(const Uint64 usec);
			virtual Uint64 get_max_end_time()
			{
				return 6000000;
			}
			;

			TargetMagicEffect* effect;
			ParticleSpawner* spawner;
			ParticleMover* mover;
			Vec3 center;
			Vec3 gravity_center;
			Vec3 shift;
			Uint16 LOD;
			TargetMagicEffect::TargetMagicType type;
			Uint16 effect_id;
			float *target_alpha;
			bool dummy_dead;
	};

	class TargetMagicParticle : public Particle
	{
		public:
			TargetMagicParticle(Effect* _effect, ParticleMover* _mover,
				const Vec3 _pos, const Vec3 _velocity, const coord_t _size,
				const alpha_t _alpha, const color_t red, const color_t green,
				const color_t blue, TextureEnum _texture, const Uint16 _LOD,
				const TargetMagicEffect::TargetMagicType _type,
				ParticleSpawner* _spawner2, ParticleMover* _mover2,
				Vec3* _target, Uint16 _effect_id, Uint16 _state);
			~TargetMagicParticle()
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
			TargetMagicEffect::TargetMagicType type;
			ParticleSpawner* spawner2;
			ParticleMover* mover2;
			Vec3* target;
			Uint16 effect_id;
	};

///////////////////////////////////////////////////////////////////////////////

} // End namespace ec

#endif	// defined EFFECT_TARGETMAGIC_H
