/*!
 \brief Special effects for summoning creatures.
 */

#ifndef EFFECT_SUMMON_H
#define EFFECT_SUMMON_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

	// C L A S S E S //////////////////////////////////////////////////////////////

	class OuterSummonParticle : public Particle
	{
		public:
			OuterSummonParticle(Effect* _effect, ParticleMover* _mover,
				const Vec3 _pos, const Vec3 _velocity, const coord_t _size,
				const alpha_t _alpha, const color_t red, const color_t green,
				const color_t blue, const Uint16 _LOD);
			~OuterSummonParticle()
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
				return 0.002;
			}
			;

			Uint16 LOD;
	};

	class InnerSummonParticle : public Particle
	{
		public:
			InnerSummonParticle(Effect* _effect, ParticleMover* _mover,
				const Vec3 _pos, const Vec3 _velocity, const coord_t _size,
				const alpha_t _alpha, const color_t red, const color_t green,
#ifdef	NEW_TEXTURES
				const color_t blue, TextureEnum _texture, const Uint16 _LOD);
#else	/* NEW_TEXTURES */
				const color_t blue, Texture* _texture, const Uint16 _LOD);
#endif	/* NEW_TEXTURES */
			~InnerSummonParticle()
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
				return 0.002;
			}
			;
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
			virtual bool deletable()
			{
				return false;
			}
			;

#ifdef	NEW_TEXTURES
			TextureEnum texture;
#else	/* NEW_TEXTURES */
			Texture* texture;
#endif	/* NEW_TEXTURES */
			Uint16 LOD;
	};

	class SummonEffect : public Effect
	{
		public:
			enum SummonType
			{
				RABBIT,
				RAT,
				BEAVER,
				SKUNK,
				RACOON,
				DEER,
				GREEN_SNAKE,
				RED_SNAKE,
				BROWN_SNAKE,
				FOX,
				BOAR,
				WOLF,
				SPIDER,
				SKELETON,
				SMALL_GARGOYLE,
				MEDIUM_GARGOYLE,
				LARGE_GARGOYLE,
				PUMA,
				FEMALE_GOBLIN,
				POLAR_BEAR,
				BEAR,
				ARMED_MALE_GOBLIN,
				ARMED_SKELETON,
				FEMALE_ORC,
				MALE_ORC,
				ARMED_FEMALE_ORC,
				ARMED_MALE_ORC,
				TIGER,
				CYCLOPS,
				FLUFFY,
				GIANT_SNAKE,
				PHANTOM_WARRIOR,
				MOUNTAIN_CHIMERAN,
				YETI,
				ARCTIC_CHIMERAN,
				GIANT
			};

			SummonEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
				const SummonType _type, const Uint16 _LOD);
			~SummonEffect();

			virtual EffectEnum get_type()
			{
				return EC_SUMMON;
			}
			;
			bool idle(const Uint64 usec);
			virtual void request_LOD(const float _LOD);

			IFSParticleSpawner* inner_spawner;
			IFSParticleSpawner* outer_spawner;
			SmokeMover* smoke_mover;
			GravityMover* gravity_mover;
			Vec3 gravity_center;
			coord_t outer_size;
			coord_t outer_radius;
			coord_t inner_size;
			alpha_t outer_alpha;
			alpha_t inner_alpha;
			color_t outer_color[3];
			color_t inner_color[3];
#ifdef	NEW_TEXTURES
			TextureEnum texture;
#else	/* NEW_TEXTURES */
			Texture* texture;
#endif	/* NEW_TEXTURES */
			Sint64 count;
			Uint32 count_scalar;
			SummonType type;
	};

///////////////////////////////////////////////////////////////////////////////

} // End namespace ec

#endif	// defined EFFECT_SUMMON_H
