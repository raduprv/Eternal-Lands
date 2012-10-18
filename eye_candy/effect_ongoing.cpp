// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_ongoing.h"

namespace ec
{

	// C L A S S   F U N C T I O N S //////////////////////////////////////////////

	OngoingParticle::OngoingParticle(Effect* _effect, ParticleMover* _mover,
		const Vec3 _pos, const Vec3 _velocity, const color_t hue_adjust,
		const color_t saturation_adjust, const coord_t _size,
		const alpha_t _alpha, color_t hue, color_t saturation, color_t value,
#ifdef	NEW_TEXTURES
		TextureEnum _texture, const Uint16 _LOD,
#else	/* NEW_TEXTURES */
		Texture* _texture, const Uint16 _LOD,
#endif	/* NEW_TEXTURES */
		const OngoingEffect::OngoingType _type) :
		Particle(_effect, _mover, _pos, _velocity, _size)
	{
		type = _type;
		hue += hue_adjust;
		if (hue > 1.0)
			hue -= 1.0;
		saturation = std::min(1.0f, saturation * saturation_adjust);
		hsv_to_rgb(hue, saturation, value, color[0], color[1], color[2]);
		if (type == OngoingEffect::OG_HARVEST)
		{
			color[0] = hue;
			color[1] = saturation;
			color[2] = value;
		}
		texture = _texture;
		alpha = _alpha;
		velocity /= size;
		flare_max = 1.0;
		flare_exp = 0.1;
		flare_frequency = 50.0;
		LOD = _LOD;
		state = 0;
		angle = 0.0f;
		center = _pos;
	}

	OngoingParticle::OngoingParticle(Effect* _effect, ParticleMover* _mover,
		const Vec3 _pos, const Vec3 _velocity, const color_t hue_adjust,
		const color_t saturation_adjust, const coord_t _size,
		const alpha_t _alpha, color_t hue, color_t saturation, color_t value,
#ifdef	NEW_TEXTURES
		TextureEnum _texture, const Uint16 _LOD,
#else	/* NEW_TEXTURES */
		Texture* _texture, const Uint16 _LOD,
#endif	/* NEW_TEXTURES */
		const OngoingEffect::OngoingType _type, const angle_t _angle) :
		Particle(_effect, _mover, _pos, _velocity, _size)
	{
		type = _type;
		hue += hue_adjust;
		if (hue > 1.0)
			hue -= 1.0;
		saturation *= saturation_adjust;
		if (saturation > 1.0)
			saturation = 1.0;
		hsv_to_rgb(hue, saturation, value, color[0], color[1], color[2]);
		if (type == OngoingEffect::OG_HARVEST)
		{
			color[0] = hue;
			color[1] = saturation;
			color[2] = value;
		}
		texture = _texture;
		alpha = _alpha;
		velocity /= size;
		flare_max = 1.0;
		flare_exp = 0.1;
		flare_frequency = 50.0;
		LOD = _LOD;
		state = 0;
		angle = _angle;
		center = _pos;
	}

	bool OngoingParticle::idle(const Uint64 delta_t)
	{
		const interval_t float_time = delta_t / 1000000.0;
		const Uint64 age = get_time() - born;
		switch (type)
		{
			case OngoingEffect::OG_MAGIC_PROTECTION:
			{
				const alpha_t scalar = (1.0
					- pow_randfloat(float_time * 1.0f)) * 0.25f;
				alpha -= scalar;
				velocity.y -= scalar;
				if (alpha < 0.01)
					return false;
				break;
			}
			case OngoingEffect::OG_SHIELD:
			{
				const alpha_t scalar = (1.0
					- pow_randfloat(float_time * 1.0f)) * 0.5f;
				alpha -= scalar;
				velocity.y -= scalar;
				if (alpha < 0.01)
					return false;
				break;
			}
			case OngoingEffect::OG_MAGIC_IMMUNITY:
			{
				const alpha_t scalar = (1.0
					- pow_randfloat(float_time * 0.75f)) * 0.25f;
				alpha -= scalar;
				velocity.y -= scalar * 0.25f;
				if (alpha < 0.01)
					return false;
				break;
			}
			case OngoingEffect::OG_POISON:
			{
				const alpha_t scalar = 1.0
					- pow_randfloat(float_time * 0.5f);
				alpha -= scalar;
				if (alpha < 0.02)
					return false;
				break;
			}
			case OngoingEffect::OG_HARVEST:
			{
				if (((OngoingEffect*)effect)->recall == false)
				{
					//center = ((OngoingEffect*)effect)->effect_center;
				}
				const float age_f = (float)(age)/1000000;
				pos.x = center.x + cos(angle + M_PI * age_f) * std::max((age_f
					< 0.75f ? 0.0f : 0.0625f), (float)(age_f * 2.5f / exp(age_f
					* 4.0f)));
				pos.z = center.z + sin(angle + M_PI * age_f) * std::max((age_f
					< 0.75f ? 0.0f : 0.0625f), (float)(age_f * 2.5f / exp(age_f
					* 4.0f)));
				pos.y = center.y - 0.0625f + pow(age_f, 2.0f) * 0.25f;
				const alpha_t scalar = 1.0f
					- pow_randfloat(float_time * 0.5f);
				alpha -= scalar * 0.5f;
				if (alpha < 0.01)
					return false;
				size -= scalar * 0.0625f;
				break;
			}
		}

		return true;
	}

#ifdef	NEW_TEXTURES
	Uint32 OngoingParticle::get_texture()
	{
		return base->get_texture(texture);
	}

	float OngoingParticle::get_burn() const
	{
		if ((type == OngoingEffect::OG_POISON) && (state == 1))
		{
			return 0.0f;
		}
		else
		{
			return 1.0f;
		}
	}
#else	/* NEW_TEXTURES */
	GLuint OngoingParticle::get_texture(const Uint16 res_index)
	{
		return texture->get_texture(res_index);
	}

	void OngoingParticle::draw(const Uint64 usec)
	{
		if ((type == OngoingEffect::OG_POISON) && (state == 1))
		{
			glEnable(GL_LIGHTING);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			Vec3 normal;
			normal.randomize();
			normal.normalize();
			glNormal3f(normal.x, normal.y, normal.z);
		}
		Particle::draw(usec);

		if ((type == OngoingEffect::OG_POISON) && (state == 1))
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glDisable(GL_LIGHTING);
		}
	}
#endif	/* NEW_TEXTURES */

	OngoingEffect::OngoingEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
		const color_t _hue_adjust, const color_t _saturation_adjust,
		const OngoingType _type, const Uint16 _LOD, const float _strength, Uint32 _buff_type)
	{
		if (EC_DEBUG)
			std::cout << "OngoingEffect (" << this << ") created." << std::endl;
		base = _base;
		dead = _dead;
		pos = _pos;
		initial_center = *pos;
		hue_adjust = _hue_adjust;
		saturation_adjust = _saturation_adjust;
		effect_center = *pos;
		//effect_center.y += 0.5; // don't! it's linked to a bone position now
		type = _type;
		LOD = base->last_forced_LOD;
		desired_LOD = _LOD;
		spawner = NULL;
		bounds = NULL;
		mover = NULL;
		strength = _strength;
		buff_type = _buff_type;

		switch (type)
		{
			case OG_MAGIC_PROTECTION:
			{
				spawner = new HollowDiscSpawner(0.25);
				mover = new ParticleMover(this);
				break;
			}
			case OG_MAGIC_IMMUNITY:
			{
				spawner = new FilledSphereSpawner(0.25);
				mover = new ParticleMover(this);
				break;
			}
			case OG_SHIELD:
			{
				spawner = new HollowDiscSpawner(0.5);
				mover = new ParticleMover(this);
				break;
			}
			case OG_POISON:
			{
				spawner = new HollowDiscSpawner(0.25);
				mover = new SpiralMover(this, &effect_center, -0.9, 0.8);
				break;
			}
			case OG_HARVEST:
			{
				spawner = new FilledSphereSpawner(0.05);
				mover = new ParticleMover(this);
				break;
			}
		}
	}

	OngoingEffect::~OngoingEffect()
	{
		if (spawner)
			delete spawner;
		if (mover)
			delete mover;
		if (EC_DEBUG)
			std::cout << "OngoingEffect (" << this << ") destroyed."
				<< std::endl;
	}

	bool OngoingEffect::idle(const Uint64 usec)
	{
		if ((recall) && (particles.size() == 0))
		{
			return false;
		}

		if (recall)
		{
			return true;
		}

		effect_center = *pos;

		const interval_t float_time = usec / 1000000.0;
		const Uint64 age = get_time() - born;
		const float age_f = (float)(age)/1000000;
		switch (type)
		{
			case OG_MAGIC_PROTECTION:
			{
				while (pow_randfloat(float_time * 6.0f * LOD * strength) < 0.5)
				{
					Vec3 coords = spawner->get_new_coords() + effect_center;
					coords += (coords - effect_center).normalize() * sin(age_f * 2.5f)
						* 0.125f;
					coords.y += sin(age_f * 1.5f) * 0.125f;
					const Vec3 velocity(0.0, 0.0, 0.0);
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new OngoingParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, 0.75, 1.0, 0.93, 0.72, 0.7, EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
							new OngoingParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, 0.75, 1.0, 0.93, 0.72, 0.7, &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case OG_SHIELD:
			{
				while (pow_randfloat(float_time * 12.0f * LOD * strength) < 0.75)
				{
					Vec3 coords = spawner->get_new_coords() + effect_center;
					coords.y += sin(age_f * 2.5f) * 0.33f - 0.125f;
					const Vec3 velocity(0.0, 0.0, 0.0);
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new OngoingParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, 0.5, 1.0, 0.55, 0.05, 0.9, EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
							new OngoingParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, 0.5, 1.0, 0.55, 0.05, 0.9, &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case OG_MAGIC_IMMUNITY:
			{
				while (pow_randfloat(float_time * 6.0f * LOD * strength) < 0.5)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center;
					const Vec3 velocity(0.0, 0.0, 0.0);
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new OngoingParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, 1.5, 1.0, 0.93, 0.72, 0.7, EC_SHIMMER, LOD, type);
#else	/* NEW_TEXTURES */
							new OngoingParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, 1.5, 1.0, 0.93, 0.72, 0.7, &(base->TexShimmer), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case OG_POISON: //The odd one out.  ;)
			{
				while (pow_randfloat(float_time * 4.0f * LOD * strength) < 0.5)
				{
					Vec3 coords = spawner->get_new_coords();
					Vec3 velocity;
					velocity.randomize(0.13);
					velocity.y += 0.08;
					coords += effect_center;
					coords.y = 0.3 + randcoord(0.8);
					Particle* p;
					if (randfloat() < 0.4)
					{
						p
#ifdef	NEW_TEXTURES
							= new OngoingParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, 1.45, 0.5, 0.27 + randcolor(0.06), 0.6 + randcolor(0.15), 0.5 + randcolor(0.3), EC_VOID, LOD, type);
#else	/* NEW_TEXTURES */
							= new OngoingParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, 1.45, 0.5, 0.27 + randcolor(0.06), 0.6 + randcolor(0.15), 0.5 + randcolor(0.3), &(base->TexVoid), LOD, type);
#endif	/* NEW_TEXTURES */
						p->state = 1;
					}
					else
					{
						p
#ifdef	NEW_TEXTURES
							= new OngoingParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, 0.85, 1.0, randcolor(0.5), 0.33 + randcolor(0.67), 0.2 + randcolor(0.1), EC_WATER, LOD, type);
#else	/* NEW_TEXTURES */
							= new OngoingParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, 0.85, 1.0, randcolor(0.5), 0.33 + randcolor(0.67), 0.2 + randcolor(0.1), &(base->TexWater), LOD, type);
#endif	/* NEW_TEXTURES */
						p->state = 0;
					}
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case OG_HARVEST:
			{
				while (pow_randfloat(float_time * 2.0f * LOD * strength) < 0.6)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center;
					const Vec3 velocity = Vec3(0.0, 0.0, 0.0);
					Particle* p;
					if (randfloat() < 0.5)
					{
						p
#ifdef	NEW_TEXTURES
							= new OngoingParticle(this, mover, coords, velocity * 0.95, hue_adjust, saturation_adjust, 0.5 + randcolor(0.75), 1.0, 0.75 + randcolor(0.25), 0.5 + randcolor(0.1), randcolor(0.1), EC_FLARE, LOD, type, randfloat(2.0 * M_PI));
#else	/* NEW_TEXTURES */
							= new OngoingParticle(this, mover, coords, velocity * 0.95, hue_adjust, saturation_adjust, 0.5 + randcolor(0.75), 1.0, 0.75 + randcolor(0.25), 0.5 + randcolor(0.1), randcolor(0.1), &(base->TexFlare), LOD, type, randfloat(2.0 * M_PI));
#endif	/* NEW_TEXTURES */
					}
					else
					{
						p
#ifdef	NEW_TEXTURES
							= new OngoingParticle(this, mover, coords, velocity * 1.05, hue_adjust, saturation_adjust, 0.75 + randcolor(0.5), 1.0, 0.75 + randcolor(0.25), 0.5 + randcolor(0.1), randcolor(0.1), EC_SHIMMER, LOD, type, randfloat(2.0 * M_PI));
#else	/* NEW_TEXTURES */
							= new OngoingParticle(this, mover, coords, velocity * 1.05, hue_adjust, saturation_adjust, 0.75 + randcolor(0.5), 1.0, 0.75 + randcolor(0.25), 0.5 + randcolor(0.1), randcolor(0.1), &(base->TexShimmer), LOD, type, randfloat(2.0 * M_PI));
#endif	/* NEW_TEXTURES */
					}
					if (!base->push_back_particle(p))
						break;
					if (randfloat() < 0.5)
					{
						p
#ifdef	NEW_TEXTURES
							= new OngoingParticle(this, mover, coords, velocity * 0.9, hue_adjust, saturation_adjust, 0.25 + randcolor(), 1.0, 0.75 + randcolor(0.25), 0.5 + randcolor(0.1), randcolor(0.1), EC_VOID, LOD, type, randfloat(2.0 * M_PI));
#else	/* NEW_TEXTURES */
							= new OngoingParticle(this, mover, coords, velocity * 0.9, hue_adjust, saturation_adjust, 0.25 + randcolor(), 1.0, 0.75 + randcolor(0.25), 0.5 + randcolor(0.1), randcolor(0.1), &(base->TexVoid), LOD, type, randfloat(2.0 * M_PI));
#endif	/* NEW_TEXTURES */
					}
					else
					{
						p
#ifdef	NEW_TEXTURES
							= new OngoingParticle(this, mover, coords, velocity * 1.1, hue_adjust, saturation_adjust, 0.5 + randcolor(0.5), 1.0, 0.75 + randcolor(0.25), 0.5 + randcolor(0.1), randcolor(0.1), EC_TWINFLARE, LOD, type, randfloat(2.0 * M_PI));
#else	/* NEW_TEXTURES */
							= new OngoingParticle(this, mover, coords, velocity * 1.1, hue_adjust, saturation_adjust, 0.5 + randcolor(0.5), 1.0, 0.75 + randcolor(0.25), 0.5 + randcolor(0.1), randcolor(0.1), &(base->TexTwinflare), LOD, type, randfloat(2.0 * M_PI));
#endif	/* NEW_TEXTURES */
					}
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
		}

		return true;
	}

///////////////////////////////////////////////////////////////////////////////

}
;

