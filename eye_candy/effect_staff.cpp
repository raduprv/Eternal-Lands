// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_staff.h"

namespace ec
{

	// C L A S S   F U N C T I O N S //////////////////////////////////////////////

	StaffParticle::StaffParticle(Effect* _effect, ParticleMover* _mover,
		const Vec3 _pos, const Vec3 _velocity, const coord_t _size,
		const alpha_t _alpha, const color_t red, const color_t green,
		const color_t blue, Texture* _texture, const Uint16 _LOD) :
		Particle(_effect, _mover, _pos, _velocity)
	{
		color[0] = red + randcolor(0.25) - 0.125;
		if (color[0] > 1.0)
			color[0] = 1.0;
		else if (color[0] < 0.0)
			color[0] = 0.0;
		color[1] = green + randcolor(0.25) - 0.125;
		if (color[1] > 1.0)
			color[1] = 1.0;
		else if (color[1] < 0.0)
			color[1] = 0.0;
		color[2] = blue + randcolor(0.25) - 0.125;
		if (color[2] > 1.0)
			color[2] = 1.0;
		else if (color[2] < 0.0)
			color[2] = 0.0;
		texture = _texture;
		size = _size * (0.2 + randcoord());
		if (size > 1.0)
			size = 1.0f;
		alpha = _alpha;
		velocity /= size;
		flare_max = 1.6;
		flare_exp = 0.2;
		flare_frequency = 2.0;
		LOD = _LOD;
	}

	bool StaffParticle::idle(const Uint64 delta_t)
	{
		if (effect->recall)
			return false;

		if (alpha < 0.01)
			return false;

		const alpha_t scalar =
			math_cache.powf_05_close((float)delta_t / 300000);
		alpha *= fastsqrt(scalar);

		return true;
	}

	GLuint StaffParticle::get_texture(const Uint16 res_index)
	{
		return texture->get_texture(res_index);
	}

	StaffEffect::StaffEffect(EyeCandy* _base, bool* _dead,
		Vec3* _end, const StaffType _type, const Uint16 _LOD)
	{
		if (EC_DEBUG)
			std::cout << "StaffEffect (" << this << ") created (" << _type
				<< ")." << std::endl;
		base = _base;
		dead = _dead;
		pos = _end;
		type = _type;
		bounds = NULL;
		mover = new ParticleMover(this);

		switch (type)
		{
			case STAFF_OF_THE_MAGE:
			{
				color[0] = 1.0;
				color[1] = 0.6;
				color[2] = 0.3;
				texture = &(base->TexFlare);
				break;
			}
			case STAFF_OF_PROTECTION:
			{
				color[0] = 0.4;
				color[1] = 0.5;
				color[2] = 1.0;
				texture = &(base->TexCrystal);
				break;
			}
		}

		old_end = *pos;
		LOD = 100;
		desired_LOD = _LOD;
		request_LOD((float)base->last_forced_LOD);
	}

	StaffEffect::~StaffEffect()
	{
		delete mover;
		if (EC_DEBUG)
			std::cout << "StaffEffect (" << this << ") destroyed." << std::endl;
	}

	void StaffEffect::request_LOD(const float _LOD)
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
			case STAFF_OF_THE_MAGE:
			{
				alpha = 1.0;
				size = 1.2;
				break;
			}
			case STAFF_OF_PROTECTION:
			{
				alpha = 1.0;
				size = 1.25;
				break;
			}
		}

		size *= 40.0 / (LOD + 17);
		alpha /= 13.0 / (LOD + 3);
	}

	bool StaffEffect::idle(const Uint64 usec)
	{
		if ((recall) && (particles.size() == 0))
			return false;

		if (recall)
			return true;

		const Vec3 pos_change = old_end - *pos;
		float speed= square(pos_change.magnitude() * 1000000.0 / usec) * 0.666667;
		float bias = 0.5f;
		if (speed > 3.0f)
			speed = 3.0f;
		else if (speed < 0.25f)
			speed = 0.25f;

		while (math_cache.powf_0_1_rough_close(randfloat(), (float)usec * 0.0000083 * speed) < bias)
		{
			const Vec3 coords = *pos;
			const Vec3 velocity = Vec3(0.0, -randcoord(0.25), 0.0);
			Particle* p = new StaffParticle(this, mover, coords, velocity, size - 0.25 + randfloat(0.75), 0.25 + randalpha(0.75f), color[0], color[1], color[2], texture, LOD);
			if (!base->push_back_particle(p))
				break;
			if (randfloat(2.0f) < 0.1f) {
				p = new StaffParticle(this, mover, coords, velocity, 1.5, 1.0, 2.0, 2.0, 2.0, &(base->TexTwinflare), LOD);
				base->push_back_particle(p);
			}
		}

		old_end = *pos;

		return true;
	}

///////////////////////////////////////////////////////////////////////////////

}
;

