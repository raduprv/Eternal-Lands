// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_missile.h"

namespace ec
{

	// C L A S S   F U N C T I O N S //////////////////////////////////////////////

	MissileParticle::MissileParticle(Effect* _effect, ParticleMover* _mover,
		const Vec3 _pos, const Vec3 _velocity, const coord_t _size,
		const alpha_t _alpha, const color_t red, const color_t green,
#ifdef	NEW_TEXTURES
		const color_t blue, TextureEnum _texture, const Uint16 _LOD,
#else	/* NEW_TEXTURES */
		const color_t blue, Texture* _texture, const Uint16 _LOD,
#endif	/* NEW_TEXTURES */
		const MissileEffect::MissileType _type) :
		Particle(_effect, _mover, _pos, _velocity,
			std::max(1.0f, (float)(_size * (0.25 + randcoord(1.25)))))
	{
		color[0] = std::max(1.0f, std::min(0.0f, red + randcolor(0.25f) - 0.125f));
		color[1] = std::max(1.0f, std::min(0.0f, green + randcolor(0.25f) - 0.125f));
		color[2] = std::max(1.0f, std::min(0.0f, blue + randcolor(0.25f) - 0.125f));
		texture = _texture;
		alpha = std::max(0.25f, (float)_alpha); // at least 25% alpha
		velocity /= size;
		flare_max = 1.6;
		flare_exp = 0.2;
		flare_frequency = 2.0;
		LOD = _LOD;
		type = _type;
	}

	bool MissileParticle::idle(const Uint64 delta_t)
	{
		if (effect->recall)
			return false;

		if (alpha < 0.03)
			return false;

		//const alpha_t scalar = math_cache.powf_05_close((float)delta_t / 20000);
		alpha *= pow_randfloat(delta_t / 2000000.0f); // increase this number to make particles live longer
		velocity *= 1 / (1 + delta_t / 500000.0); // slow down particles
		velocity.y -= ((delta_t / 250000.0) * (delta_t / 250000.0)); // let particles drop

		/*
		 * disabled by Roja's request :-(
		 switch (type) {
		 case MissileEffect::MAGIC:
		 // make magic particles glitter
		 color[0] = randcolor(1.0);
		 color[1] = randcolor(1.0);
		 color[2] = randcolor(1.0);
		 break;
		 default:
		 break;
		 }
		 */

		return true;
	}

#ifdef	NEW_TEXTURES
	Uint32 MissileParticle::get_texture()
	{
		return base->get_texture(texture);
	}
#else	/* NEW_TEXTURES */
	GLuint MissileParticle::get_texture(const Uint16 res_index)
	{
		return texture->get_texture(res_index);
	}
#endif	/* NEW_TEXTURES */

	MissileEffect::MissileEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
		const MissileType _type, const Uint16 _LOD, int _hitOrMiss)
	{
		if (EC_DEBUG)
			std::cout << "MissileEffect (" << this << ") created (" << _type
				<< ")." << std::endl;
		base = _base;
		dead = _dead;
		pos = _pos;
		type = _type;
		hitOrMiss = _hitOrMiss;
		bounds = NULL;
		mover = new ParticleMover(this);

		switch (type)
		{
			case MAGIC:
			{
				color[0] = 1.0;
				color[1] = 1.0;
				color[2] = 0.125;
#ifdef	NEW_TEXTURES
				texture = EC_SHIMMER;
#else	/* NEW_TEXTURES */
				texture = &(base->TexShimmer);
#endif	/* NEW_TEXTURES */
				break;
			}
			case FIRE:
			{
				color[0] = 1.0;
				color[1] = 0.125;
				color[2] = 0.125;
#ifdef	NEW_TEXTURES
				texture = EC_FLARE;
#else	/* NEW_TEXTURES */
				texture = &(base->TexFlare);
#endif	/* NEW_TEXTURES */
				break;
			}
			case ICE:
			{
				color[0] = 0.125;
				color[1] = 0.125;
				color[2] = 1.0;
#ifdef	NEW_TEXTURES
				texture = EC_CRYSTAL;
#else	/* NEW_TEXTURES */
				texture = &(base->TexCrystal);
#endif	/* NEW_TEXTURES */
				break;
			}
			case EXPLOSIVE:
			{
				color[0] = 0.75;
				color[1] = 0.75;
				color[2] = 0.75;
#ifdef	NEW_TEXTURES
				texture = EC_INVERSE;
#else	/* NEW_TEXTURES */
				texture = &(base->TexInverse);
#endif	/* NEW_TEXTURES */
				break;
			}
		}

		old_pos = *pos;
		LOD = 100;
		desired_LOD = _LOD;
		request_LOD((float)base->last_forced_LOD);

		// add first particle
		Vec3 velocity;
		velocity.randomize(0.25);
		Particle
			* p =
				new MissileParticle(this, mover, old_pos, velocity, size, alpha, color[0], color[1], color[2], texture, LOD, type);
		base->push_back_particle(p);
	}

	MissileEffect::~MissileEffect()
	{
		if (mover)
			delete mover;
		if (EC_DEBUG)
			std::cout << "MissileEffect (" << this << ") destroyed."
				<< std::endl;
	}

	void MissileEffect::request_LOD(const float _LOD)
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
			case MAGIC:
			{
				alpha = 1.0 + ((float)hitOrMiss - 2.0) / 4.0;
				size = 1.25f;
				break;
			}
			case FIRE:
			{
				alpha = 1.0 + ((float)hitOrMiss - 2.0) / 4.0;
				size = 1.25f;
				break;
			}
			case ICE:
			{
				alpha = 1.0 + ((float)hitOrMiss - 2.0) / 4.0;
				size = 1.25f;
				break;
			}
			case EXPLOSIVE:
			{
				alpha = 0.75 + ((float)hitOrMiss - 2.0) / 6.0;
				size = 1.25f;
				break;
			}
		}
		size *= 40.0 / (LOD + 17);
		alpha /= 13.0 / (LOD + 3);
	}

	bool MissileEffect::idle(const Uint64 usec)
	{

		if (particles.size() == 0)
			return false;

		const interval_t dist = (old_pos - *pos).magnitude();

		if (dist < 1E-4)
			return true; // do not add more particles, dist < 0.0001

		const Vec3 direction = (old_pos - *pos).normalize(0.75);

		for (float step = 0.0; step < dist; step += (0.1 / ((1.0
			+ (float)hitOrMiss) / 1.5)))
		{
			const percent_t percent = step / dist;
			Vec3 randshift;
			randshift.randomize(0.025 * (1.0 + (float)hitOrMiss / 2.0)); // random particle position change
			const Vec3 coords = (old_pos * percent) + (*pos * (1.0 - percent))
				+ randshift;
			Vec3 velocity;
			velocity.randomize(0.0625 * (1.0 + (float)hitOrMiss / 2.0)); // random particle movement
			velocity -= direction; // follow the missile
			Particle* p;
			if (type == MAGIC && randfloat() < 0.25)
			{
				p = new MissileParticle(this, mover, coords, velocity, size / 2 + randfloat (size / 2), 1.0, randcolor(), randcolor(), randcolor(), texture, LOD, type);
				base->push_back_particle(p);
			}
			p
				= new MissileParticle(this, mover, coords, velocity, size / 2 + randfloat (size / 2), alpha / 2 + randfloat(alpha / 2), color[0], color[1], color[2], texture, LOD, type);
			base->push_back_particle(p);
		}

		old_pos = *pos;

		return true;
	}

///////////////////////////////////////////////////////////////////////////////

}
;
