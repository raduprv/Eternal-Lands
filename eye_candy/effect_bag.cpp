// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_bag.h"

namespace ec
{

	// C L A S S   F U N C T I O N S //////////////////////////////////////////////

	BagParticle::BagParticle(Effect* _effect, ParticleMover* _mover,
		const Vec3 _pos, const Vec3 _velocity, const coord_t _size) :
		Particle(_effect, _mover, _pos, _velocity, _size)
	{
		color[0] = randcolor(0.3) + 0.7;
		color[1] = randcolor(0.3) + 0.5;
		color[2] = randcolor(0.3) + 0.3;
		alpha = 0.75;
		flare_max = 1.0;
		flare_exp = 1.0;
		flare_frequency = 1.0;
		state = 0;
	}

	float BagParticle::get_burn() const
	{
		return 0.0f;
	}

	bool BagParticle::idle(const Uint64 delta_t)
	{
		if (effect->recall)
			return false;

		const Uint64 age = get_time() - born;
		const interval_t float_time = delta_t / 1000000.0;
		if (age > 220000)
		{
			const alpha_t alpha_scalar = std::pow(0.5f, float_time
				* 3.0f);
			alpha *= alpha_scalar;

			if (alpha < 0.01)
				return false;
		}

		return true;
	}

	Uint32 BagParticle::get_texture()
	{
		return base->get_texture(EC_FLARE);
	}

	BagEffect::BagEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
		const bool _picked_up, const Uint16 _LOD)
	{
		if (EC_DEBUG)
			std::cout << "BagEffect (" << this << ") created." << std::endl;
		base = _base;
		dead = _dead;
		pos = _pos;
		picked_up = _picked_up;
		effect_center = *pos;
		if (!picked_up)
			effect_center.y += 0.2;
		LOD = base->last_forced_LOD;
		desired_LOD = _LOD;
		bounds = NULL;
		mover = new GravityMover(this, &effect_center, 4e9);
		spawner = new HollowSphereSpawner(0.14 + 0.02 * (float)picked_up);

		const coord_t size = 40 / (LOD + 10);
		for (int i = 0; i < LOD * (30 + 30 * (int)picked_up); i++)
		{
			Vec3 coords = spawner->get_new_coords();
			Vec3 velocity = coords * 1.3;
			coords += effect_center;
			Particle* p = new BagParticle(this, mover, coords, velocity, size);
			if (!base->push_back_particle(p))
				break;
		}

	}

	BagEffect::~BagEffect()
	{
		delete mover;
		delete spawner;
		if (EC_DEBUG)
			std::cout << "BagEffect (" << this << ") destroyed." << std::endl;
	}

	bool BagEffect::idle(const Uint64 usec)
	{
		if (particles.size() == 0)
			return false;

		effect_center.x = pos->x;
		if (picked_up)
			effect_center.y += usec / 2000000.0;
		else
			effect_center.y -= usec / 2000000.0;
		effect_center.z = pos->z;

		return true;
	}

///////////////////////////////////////////////////////////////////////////////

}
;

