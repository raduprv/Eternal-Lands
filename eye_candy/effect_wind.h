/*!
 \brief Special effects that have objects blow around under the wind.
 */

#ifndef EFFECT_WIND_H
#define EFFECT_WIND_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

	// C L A S S E S //////////////////////////////////////////////////////////////


	class WindEffect : public Effect
	{
		public:
			enum WindType
			{
				LEAVES,
				FLOWER_PETALS,
				SNOW
			};

			struct WindNeighbor
			{
					WindEffect* neighbor;
					angle_t start_angle;
					angle_t end_angle;
			};

			WindEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
				std::vector<ec::Obstruction*>* _obstructions,
				const color_t _hue_adjust, const color_t _saturation_adjust,
				const coord_t _scalar, const float _density,
				BoundingRange* _bounding_range, const WindType _type,
				const Vec3 _prevailing_wind);
			~WindEffect();

			void set_pass_off(std::vector<WindEffect*> pass_off_to); // Required!
			void set_pass_off(std::vector<Effect*> pass_off_to); // Required!

			EffectEnum get_type()
			{
				return EC_WIND;
			}
			;
			bool idle(const Uint64 usec);
			virtual void request_LOD(const float _LOD)
			{
				if (fabs(_LOD - (float)LOD) < 1.0)
					return;
				const Uint16 rounded_LOD = (Uint16)round(_LOD);
				if (rounded_LOD <= desired_LOD)
					LOD = rounded_LOD;
				else
					LOD = desired_LOD;
				count = LOD * max_LOD1_count;
			}
			;

			ParticleMover* mover;
			FilledBoundingSpawner* spawner;
			WindType type;
			Vec3 center;
			color_t hue_adjust;
			color_t saturation_adjust;
			float scalar;
			Vec3 prevailing_wind;
			coord_t max_adjust;
			Vec3 overall_wind_adjust;
			Vec3 overall_wind;
			int max_LOD1_count;
			int count;
			std::vector<WindNeighbor> neighbors; // Where to pass particles off to.
			BoundingRange* bounding_range;
	};

	class WindParticle : public Particle
	{
		public:
			WindParticle(Effect* _effect, ParticleMover* _mover,
				const Vec3 _pos, const Vec3 _velocity,
				const color_t hue_adjust, const color_t saturation_adjust,
				const coord_t scalar, const coord_t _min_height,
				const coord_t _max_height, const WindEffect::WindType _type);
			~WindParticle()
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
				return 0.0;
			}
			; // We don't want the particle system lights to be used on the pos, since it will assumedly already have one.
			virtual light_t get_light_level()
			{
				return 0.0;
			}
			;
			Vec3 get_wind_vec() const;

			coord_t min_height;
			coord_t max_height;
			WindEffect::WindType type;
			Uint8 subtype;
			Vec3 rotation_axes[3];
			percent_t axis_weights[3];
			Quaternion quaternion;
	};

///////////////////////////////////////////////////////////////////////////////

} // End namespace ec

#endif	// defined EFFECT_WIND_H
