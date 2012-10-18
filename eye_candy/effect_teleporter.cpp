// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_teleporter.h"

namespace ec
{

	// C L A S S   F U N C T I O N S //////////////////////////////////////////////

	TeleporterParticle::TeleporterParticle(Effect* _effect,
		ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity,
		const color_t hue_adjust, const color_t saturation_adjust,
		const coord_t size_scalar) :
		Particle(_effect, _mover, _pos, _velocity,
			size_scalar * (0.5 + 1.5 * randcoord()))
	{
		color_t hue, saturation, value;
		hue = randcolor(1.0);
		saturation = randfloat(0.2);
		value = 0.9;
		hue += hue_adjust;
		if (hue > 1.0)
			hue -= 1.0;
		saturation = std::min(1.0f, saturation * saturation_adjust);
		hsv_to_rgb(hue, saturation, value, color[0], color[1], color[2]);
		alpha = std::min(1.0f, 5.0f / size);
		velocity /= size;
		flare_max = 1.6;
		flare_exp = 0.2;
		flare_frequency = 2.0;
	}

	bool TeleporterParticle::idle(const Uint64 delta_t)
	{
		if (effect->recall)
			return false;

		if (alpha < 0.03)
			return false;

		const alpha_t scalar =
			std::pow(0.5f, (float)delta_t / 800000);
		alpha *= scalar;

		return true;
	}

#ifdef	NEW_TEXTURES
	Uint32 TeleporterParticle::get_texture()
	{
		return base->get_texture(EC_SHIMMER);
	}
#else	/* NEW_TEXTURES */
	GLuint TeleporterParticle::get_texture(const Uint16 res_index)
	{
		return base->TexShimmer.get_texture(res_index);
	}
#endif	/* NEW_TEXTURES */

	TeleporterEffect::TeleporterEffect(EyeCandy* _base, bool* _dead,
		Vec3* _pos, const color_t _hue_adjust,
		const color_t _saturation_adjust, const float _scale, const Uint16 _LOD)
	{
		if (EC_DEBUG)
			std::cout << "TeleporterEffect (" << this << ") created."
				<< std::endl;
		base = _base;
		dead = _dead;
		pos = _pos;
		hue_adjust = _hue_adjust;
		saturation_adjust = _saturation_adjust;
		LOD = base->last_forced_LOD;
		desired_LOD = _LOD;
		sqrt_LOD = std::sqrt(LOD);
		size_scalar = 15 / (LOD + 5);
		scale = _scale;
		bounds = NULL;
		mover = new ParticleMover(this);
		spawner = new FilledDiscSpawner(0.2);

		/*
		 for (int i = 0; i < LOD * 10; i++)
		 {
		 const Vec3 coords = spawner->get_new_coords() + *pos + Vec3(0.0, randcoord() * randcoord() * 8.0 * sqrt_LOD, 0.0);
		 Vec3 velocity(0.0, randcoord(0.1), 0.0);
		 velocity.randomize(0.2);
		 Particle* p = new TeleporterParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, size_scalar);
		 if (!base->push_back_particle(p))
		 break;
		 }
		 */

		//  const float radius = 0.5 * pow(2, 0.18) / 1.5;
		radius = 0.377628 * scale;
		color_t hue, saturation, value;
		hue = 0.67;
		saturation = 0.05;
		value = 1.0;
		hue += hue_adjust;
		if (hue > 1.0)
			hue -= 1.0;
		saturation *= saturation_adjust;
		if (saturation > 1.0)
			saturation = 1.0;
		hsv_to_rgb(hue, saturation, value, teleporter_color.x,
			teleporter_color.y, teleporter_color.z);

#ifdef	NEW_TEXTURES
		capless_cylinders = 0;

		std::vector<CaplessCylinders::CaplessCylinderItem> cylinders;
#endif	/* NEW_TEXTURES */
		for (int i = 0; i < LOD * 4; i++)
		{
			const percent_t percent = ((coord_t)i + 1) / (LOD * 4);
#ifdef	NEW_TEXTURES
			cylinders.push_back(CaplessCylinders::CaplessCylinderItem(*pos, *pos + Vec3(0.0, 10.0 / percent, 0.0), teleporter_color, (0.1 + (1.0 - percent) * 0.05) / (LOD + 2), radius * percent, (int)(25 * (percent + 0.2))));
		}

		if (cylinders.size() > 0)
		{
			capless_cylinders = new CaplessCylinders(base, cylinders);
		}
#else	/* NEW_TEXTURES */
			capless_cylinders.push_back(new CaplessCylinder(base, *pos, *pos + Vec3(0.0, 10.0 / percent, 0.0), teleporter_color, (0.1 + (1.0 - percent) * 0.05) / (LOD + 2), radius * percent, (int)(25 * (percent + 0.2))));
		}
#endif	/* NEW_TEXTURES */
	}

	TeleporterEffect::~TeleporterEffect()
	{
		delete mover;
		delete spawner;
#ifdef	NEW_TEXTURES
		delete capless_cylinders;
#else	/* NEW_TEXTURES */
		for (size_t i = 0; i < capless_cylinders.size(); i++)
			delete capless_cylinders[i];
		capless_cylinders.clear();
#endif	/* NEW_TEXTURES */
		if (EC_DEBUG)
			std::cout << "TeleporterEffect (" << this << ") destroyed."
				<< std::endl;
	}

	bool TeleporterEffect::idle(const Uint64 usec)
	{
		if ((recall) && (particles.size() == 0))
			return false;

		if (recall)
			return true;

		while (((int)particles.size() < LOD * 50)
			&& (pow_randfloat((float)usec / 100000 * LOD) < 0.5))
		{
			const Vec3 coords = spawner->get_new_coords() + *pos + Vec3(0.0,
				randcoord() * randcoord() * 8.0 * sqrt_LOD, 0.0);
			Vec3 velocity;
			velocity.randomize(0.2);
			Particle
				* p =
					new TeleporterParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, size_scalar);
			if (!base->push_back_particle(p))
				break;
		}

		for (int i = 0; i < (int)targets.size();)
		{
			std::vector< std::pair<float *, Uint64> >::iterator iter =
				targets.begin() + i;
			Uint64 age = get_time() - iter->second;
			if (age < 500000)
			{
				*(iter->first) = 1.0 - (age / 500000.0);;
				i++;
			}
			else if (age < 1000000)
			{
				*(iter->first) = (age - 500000.0) / 500000.0;;
				i++;
			}
			else
			{
				*(iter->first) = 1.0;
				targets.erase(iter);
			}
		}

		return true;
	}

	void TeleporterEffect::draw(const Uint64 usec)
	{
#ifdef	NEW_TEXTURES
		capless_cylinders->draw(1.0f);
#else	/* NEW_TEXTURES */
		for (std::vector<Shape*>::iterator iter = capless_cylinders.begin(); iter
			!= capless_cylinders.end(); iter++)
			(*iter)->draw();
#endif	/* NEW_TEXTURES */
	}

	void TeleporterEffect::request_LOD(const float _LOD)
	{
		if (fabs(_LOD - (float)LOD) < 1.5)
			return;
		const Uint16 rounded_LOD = (Uint16)round(_LOD);
		if (rounded_LOD <= desired_LOD)
			LOD = rounded_LOD;
		else
			LOD = desired_LOD;

		sqrt_LOD = std::sqrt(LOD);
		size_scalar = 15 / (LOD + 5);

#ifdef	NEW_TEXTURES
		delete capless_cylinders;
		capless_cylinders = 0;

		std::vector<CaplessCylinders::CaplessCylinderItem> cylinders;
#else	/* NEW_TEXTURES */
		for (size_t i = 0; i < capless_cylinders.size(); i++)
			delete capless_cylinders[i];
		capless_cylinders.clear();
#endif	/* NEW_TEXTURES */

		for (int i = 0; i < LOD * 4; i++)
		{
			const percent_t percent = ((coord_t)i + 1) / (LOD * 4);
#ifdef	NEW_TEXTURES
			cylinders.push_back(CaplessCylinders::CaplessCylinderItem(*pos, *pos + Vec3(0.0, 10.0 / percent, 0.0), teleporter_color, (0.1 + (1.0 - percent) * 0.05) / (LOD + 2), radius * percent, (int)(25 * (percent + 0.2))));
		}

		if (cylinders.size() > 0)
		{
			capless_cylinders = new CaplessCylinders(base, cylinders);
		}
#else	/* NEW_TEXTURES */
			capless_cylinders.push_back(new CaplessCylinder(base, *pos, *pos + Vec3(0.0, 10.0 / percent, 0.0), teleporter_color, (0.1 + (1.0 - percent) * 0.05) / (LOD + 2), radius * percent, (int)(25 * (percent + 0.2))));
		}
#endif	/* NEW_TEXTURES */
	}

	void TeleporterEffect::add_actor_alpha_pointer(float* ptr)
	{
		targets.push_back(std::pair<float*, Uint64>(ptr, get_time()));
	}

	///////////////////////////////////////////////////////////////////////////////

};

