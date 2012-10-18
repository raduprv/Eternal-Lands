// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_sword.h"

namespace ec
{

	// C L A S S   F U N C T I O N S //////////////////////////////////////////////

	SwordParticle::SwordParticle(Effect* _effect, ParticleMover* _mover,
		const Vec3 _pos, const Vec3 _velocity, const coord_t _size,
		const alpha_t _alpha, const color_t red, const color_t green,
#ifdef	NEW_TEXTURES
		const color_t blue, TextureEnum _texture, const Uint16 _LOD) :
#else	/* NEW_TEXTURES */
		const color_t blue, Texture* _texture, const Uint16 _LOD) :
#endif	/* NEW_TEXTURES */
		Particle(_effect, _mover, _pos, _velocity,
			std::min(1.0f, _size * (0.2f + randcoord())))
	{
		color[0] = std::max(0.0f, std::min(1.0f, red + randcolor(0.25f) - 0.125f));
		color[1] = std::max(0.0f, std::min(1.0f, green + randcolor(0.25f) - 0.125f));
		color[2] = std::max(0.0f, std::min(1.0f, blue + randcolor(0.25f) - 0.125f));
		texture = _texture;
		alpha = _alpha;
		velocity /= size;
		flare_max = 1.6;
		flare_exp = 0.2;
		flare_frequency = 2.0;
		LOD = _LOD;
	}

	bool SwordParticle::idle(const Uint64 delta_t)
	{
		if (effect->recall)
			return false;

		if (alpha < 0.01)
			return false;

		const alpha_t scalar =
			std::pow(0.5f, (float)delta_t / 300000);
		alpha *= std::sqrt(scalar);

		return true;
	}

#ifdef	NEW_TEXTURES
	Uint32 SwordParticle::get_texture()
	{
		return base->get_texture(texture);
	}
#else	/* NEW_TEXTURES */
	GLuint SwordParticle::get_texture(const Uint16 res_index)
	{
		return texture->get_texture(res_index);
	}
#endif	/* NEW_TEXTURES */

	SwordEffect::SwordEffect(EyeCandy* _base, bool* _dead, Vec3* _start,
		Vec3* _end, const SwordType _type, const Uint16 _LOD)
	{
		if (EC_DEBUG)
			std::cout << "SwordEffect (" << this << ") created (" << _type
				<< ")." << std::endl;
		base = _base;
		dead = _dead;
		pos = _start;
		start = _start;
		end = _end;
		type = _type;
		bounds = NULL;
		mover = new ParticleMover(this);

		switch (type)
		{
			case SERPENT:
			{
				color[0] = 0.6;
				color[1] = 0.8;
				color[2] = 0.3;
#ifdef	NEW_TEXTURES
				texture = EC_VOID;
#else	/* NEW_TEXTURES */
				texture = &(base->TexVoid);
#endif	/* NEW_TEXTURES */
				break;
			}
			case CUTLASS:
			{
				color[0] = 1.0;
				color[1] = 1.0;
				color[2] = 1.0;
#ifdef	NEW_TEXTURES
				texture = EC_INVERSE;
#else	/* NEW_TEXTURES */
				texture = &(base->TexInverse);
#endif	/* NEW_TEXTURES */
				break;
			}
			case EMERALD_CLAYMORE:
			{
				color[0] = 0.3;
				color[1] = 1.0;
				color[2] = 0.3;
#ifdef	NEW_TEXTURES
				texture = EC_CRYSTAL;
#else	/* NEW_TEXTURES */
				texture = &(base->TexCrystal);
#endif	/* NEW_TEXTURES */
				break;
			}
			case SUNBREAKER:
			{
				color[0] = 1.0;
				color[1] = 0.8;
				color[2] = 0.3;
#ifdef	NEW_TEXTURES
				texture = EC_VOID;
#else	/* NEW_TEXTURES */
				texture = &(base->TexVoid);
#endif	/* NEW_TEXTURES */
				break;
			}
			case ORC_SLAYER:
			{
				color[0] = 1.0;
				color[1] = 0.1;
				color[2] = 0.1;
#ifdef	NEW_TEXTURES
				texture = EC_WATER;
#else	/* NEW_TEXTURES */
				texture = &(base->TexWater);
#endif	/* NEW_TEXTURES */
				break;
			}
			case EAGLE_WING:
			{
				color[0] = 0.7;
				color[1] = 1.0;
				color[2] = 1.0;
#ifdef	NEW_TEXTURES
				texture = EC_INVERSE;
#else	/* NEW_TEXTURES */
				texture = &(base->TexInverse);
#endif	/* NEW_TEXTURES */
				break;
			}
			case JAGGED_SABER:
			{
				color[0] = 1.0;
				color[1] = 0.3;
				color[2] = 1.0;
#ifdef	NEW_TEXTURES
				texture = EC_TWINFLARE;
#else	/* NEW_TEXTURES */
				texture = &(base->TexTwinflare);
#endif	/* NEW_TEXTURES */
				break;
			}
			case SWORD_OF_FIRE:
			{
				color[0] = 1.0;
				color[1] = 0.6;
				color[2] = 0.3;
#ifdef	NEW_TEXTURES
				texture = EC_FLARE;
#else	/* NEW_TEXTURES */
				texture = &(base->TexFlare);
#endif	/* NEW_TEXTURES */
				break;
			}
			case SWORD_OF_ICE:
			{
				color[0] = 0.4;
				color[1] = 0.5;
				color[2] = 1.0;
#ifdef	NEW_TEXTURES
				texture = EC_CRYSTAL;
#else	/* NEW_TEXTURES */
				texture = &(base->TexCrystal);
#endif	/* NEW_TEXTURES */
				break;
			}
			case SWORD_OF_MAGIC:
			{
				color[0] = 0.7;
				color[1] = 0.6;
				color[2] = 0.4;
#ifdef	NEW_TEXTURES
				texture = EC_SHIMMER;
#else	/* NEW_TEXTURES */
				texture = &(base->TexShimmer);
#endif	/* NEW_TEXTURES */
				break;
			}
		}

		old_end = *end;
		LOD = 100;
		desired_LOD = _LOD;
		request_LOD((float)base->last_forced_LOD);
	}

	SwordEffect::~SwordEffect()
	{
		delete mover;
		if (EC_DEBUG)
			std::cout << "SwordEffect (" << this << ") destroyed." << std::endl;
	}

	void SwordEffect::request_LOD(const float _LOD)
	{
		if (fabs(_LOD - (float)LOD) < 1.0)
			return;

		const Uint16 rounded_LOD = (Uint16)round(_LOD);
		if (rounded_LOD <= desired_LOD)
			LOD = rounded_LOD;
		else
			LOD = desired_LOD;
		switch (type)
		{
			case SERPENT:
			{
				alpha = 0.55;
				size = 1.1;
				break;
			}
			case CUTLASS:
			{
				alpha = 0.1;
				size = 2.0;
				break;
			}
			case EMERALD_CLAYMORE:
			{
				alpha = 0.55;
				size = 0.75;
				break;
			}
			case SUNBREAKER:
			{
				alpha = 0.7;
				size = 1.1;
				break;
			}
			case ORC_SLAYER:
			{
				alpha = 0.1;
				size = 1.3;
				break;
			}
			case EAGLE_WING:
			{
				alpha = 0.2;
				size = 2.25;
				break;
			}
			case JAGGED_SABER:
			{
				alpha = 0.4;
				size = 2.25;
				break;
			}
			case SWORD_OF_FIRE:
			{
				alpha = 1.0;
				size = 1.2;
				break;
			}
			case SWORD_OF_ICE:
			{
				alpha = 1.0;
				size = 1.25;
				break;
			}
			case SWORD_OF_MAGIC:
			{
				alpha = 1.0;
				size = 1.4;
				break;
			}
		}

		size *= 40.0 / (LOD + 17);
		alpha /= 13.0 / (LOD + 3);
	}

	bool SwordEffect::idle(const Uint64 usec)
	{
		if ((recall) && (particles.size() == 0))
			return false;

		if (recall)
			return true;

		const Vec3 pos_change = old_end - *end;
		float speed= square(pos_change.magnitude() * 1000000.0 / usec) * 0.666667;
		float bias = 0.5f;
		switch (type)
		{
			case SERPENT:
			case CUTLASS:
			case EMERALD_CLAYMORE:
			case SUNBREAKER:
			case ORC_SLAYER:
			case EAGLE_WING:
			case JAGGED_SABER:
			{
				bias = randfloat(0.33);
				if (speed > 2.0)
					speed = 2.0f;
				else if (speed < 0.05)
					speed = 0.05;
			}
			case SWORD_OF_FIRE:
			case SWORD_OF_ICE:
			case SWORD_OF_MAGIC:
			{
				if (speed > 3.0f)
					speed = 3.0f;
				else if (speed < 0.25f)
					speed = 0.25f;
			}
		}

		while (pow_randfloat((float)usec * 0.000083f * speed) < bias)
		{
			const percent_t percent= square(randpercent());
			Vec3 randcoords;
			randcoords.randomize(0.0025);
			const Vec3 coords = (*start * percent) + (*end * (1.0 - percent))
				+ randcoords;
			Vec3 velocity;
			velocity.randomize(0.005);
			Vec3 direction = *end - *start;
			direction.normalize(0.05 + randfloat(0.25) * randfloat(0.25));
			velocity += direction;
			Particle* p = new SwordParticle(this, mover, coords, velocity, size - 0.25 + randfloat(0.25), 0.25 + randalpha(percent), color[0], color[1], color[2], texture, LOD);
			if (!base->push_back_particle(p))
				break;
			if (randfloat(2.0f) < 0.1f) {
#ifdef	NEW_TEXTURES
				p = new SwordParticle(this, mover, coords, velocity, 1.5, 1.0, 2.0, 2.0, 2.0, EC_TWINFLARE, LOD);
#else	/* NEW_TEXTURES */
				p = new SwordParticle(this, mover, coords, velocity, 1.5, 1.0, 2.0, 2.0, 2.0, &(base->TexTwinflare), LOD);
#endif	/* NEW_TEXTURES */
				base->push_back_particle(p);
			}
		}

		old_end = *end;

		return true;
	}

///////////////////////////////////////////////////////////////////////////////

}
;

