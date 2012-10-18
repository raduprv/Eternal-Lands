
// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_mines.h"
#include "orbital_mover.h"

namespace ec
{

	// C L A S S   F U N C T I O N S //////////////////////////////////////////////

	MineParticle::MineParticle(Effect* _effect, ParticleMover* _mover,
		const Vec3 _pos, const Vec3 _velocity, const coord_t _size,
		const alpha_t _alpha, const color_t red, const color_t green,
#ifdef	NEW_TEXTURES
		const color_t blue, TextureEnum _texture, const Uint16 _LOD,
#else	/* NEW_TEXTURES */
		const color_t blue, Texture* _texture, const Uint16 _LOD,
#endif	/* NEW_TEXTURES */
		const MineEffect::MineType _type) :
		Particle(_effect, _mover, _pos, _velocity,
			_size * (0.5 + randcoord()) * 10 / _LOD)
	{
		type = _type;
		color[0] = red;
		color[1] = green;
		color[2] = blue;
		texture = _texture;
		alpha = _alpha;
		flare_max = 5.0;
		flare_exp = 0.1;
		flare_frequency = 3.0;
		LOD = _LOD;
		state = 0;
	}

	bool MineParticle::idle(const Uint64 delta_t)
	{
		if (effect->recall)
			return false;

		const interval_t float_time = delta_t / 1000000.0;
		const Uint64 age = get_time() - born;
		switch (type)
		{
			case MineEffect::DETONATE_MAGIC_IMMUNITY_REMOVAL:
			{
				if (pos.y > 20)
					return false;

				velocity.y *= 1.15;
				velocity.x *= 0.82;
				velocity.z *= 0.82;

				const alpha_t scalar =
					pow_randfloat(float_time * 1);
				alpha *= scalar;

				break;
			}
			case MineEffect::DETONATE_UNINVIZIBILIZER:
			{
				if (alpha < 0.01)
					return false;

				const alpha_t scalar =
					pow_randfloat(float_time * 1);
				alpha *= scalar;
				size *= 0.95;

				break;
			}
			case MineEffect::DETONATE_MANA_DRAINER:
			{
				if (pos.y < -2.0)
					return false;

				break;
			}
			case MineEffect::DETONATE_MANA_BURNER:
			{
				if (age < 650000)
					break;

				if (alpha < 0.03)
					return false;

				const alpha_t scalar =
					pow_randfloat(float_time * 6);
				alpha *= scalar;

				break;
			}
			case MineEffect::DETONATE_CALTROP:
			case MineEffect::DETONATE_CALTROP_POISON:
			{
				if (age < 700000)
				{
					velocity *= 0.85;
				}
				else
				{
					const percent_t scalar =
						std::pow(0.5f, float_time * 1);
					alpha *= scalar;

					if (alpha < 0.01)
						return false;
				}
				break;
			}
			case MineEffect::DETONATE_TRAP:
			{
				color[0] = 0.7 + 0.3 * sin(age / 530000.0);
				color[1] = 0.7 + 0.3 * sin(age / 970000.0 + 1.3);
				color[2] = 0.7 + 0.3 * sin(age / 780000.0 + 1.9);

				pos.y += (0.4 - pos.y) * 0.2;

				if (age > 4700000)
					alpha *= 0.5;

				if (alpha < 0.01)
					return false;

				break;
			}
			case MineEffect::DETONATE_TYPE1_SMALL:
			case MineEffect::DETONATE_TYPE1_MEDIUM:
			case MineEffect::DETONATE_TYPE1_LARGE:
			{
				if (alpha < 0.01)
					return false;

				velocity *= 0.5;

				//		const alpha_t scalar = pow_randfloat(float_time * 2);
				if (age > 500000)
					alpha *= 0.8; // scalar

				break;
			}
		}

		return true;
	}

#ifdef	NEW_TEXTURES
	Uint32 MineParticle::get_texture()
	{
		return base->get_texture(texture);
	}

	float MineParticle::get_burn() const
	{
		if ((type == MineEffect::DETONATE_CALTROP) ||
			(type == MineEffect::DETONATE_CALTROP_POISON) ||
			(type == MineEffect::DETONATE_TRAP) ||
			(((type == MineEffect::DETONATE_TYPE1_SMALL) ||
			(type == MineEffect::DETONATE_TYPE1_MEDIUM) ||
			(type == MineEffect::DETONATE_TYPE1_LARGE)) &&
			(state == 0)))
		{
			return 1.0f;
		}
		else
		{
			return 0.0f;
		}
	}
#else	/* NEW_TEXTURES */
	GLuint MineParticle::get_texture(const Uint16 res_index)
	{
		return texture->get_texture(res_index);
	}

	void MineParticle::draw(const Uint64 usec)
	{
		if ((type == MineEffect::DETONATE_CALTROP) || (type
			== MineEffect::DETONATE_CALTROP_POISON) || (type
			== MineEffect::DETONATE_TRAP) || ((type
			== MineEffect::DETONATE_TYPE1_SMALL || type
			== MineEffect::DETONATE_TYPE1_MEDIUM || type
			== MineEffect::DETONATE_TYPE1_LARGE) && (state == 0)))
			Particle::draw(usec);
		else
		{
			glEnable(GL_LIGHTING);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glNormal3f(0.0, 1.0, 0.0);
			Particle::draw(usec);

			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glDisable(GL_LIGHTING);
		}
	}
#endif	/* NEW_TEXTURES */

	light_t MineParticle::get_light_level()
	{
		if ((type == MineEffect::DETONATE_CALTROP) || (type
			== MineEffect::DETONATE_CALTROP_POISON) || (type
			== MineEffect::DETONATE_TRAP))
			return alpha * size / 1500;
		else if (type == MineEffect::DETONATE_UNINVIZIBILIZER)
			return alpha * size / 1000;
		else
			return 0.0;
	}
	;

	MineEffect::MineEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
		const MineType _type, const Uint16 _LOD)
	{
		if (EC_DEBUG)
			std::cout << "MineEffect (" << this << ") created (" << type
				<< ")." << std::endl;
		base = _base;
		dead = _dead;
		pos = _pos;
		effect_center = *pos;
		type = _type;
		LOD = base->last_forced_LOD;
		desired_LOD = _LOD;
		spawner = NULL;
		bounds = NULL;
		mover = NULL;
		spawner2 = NULL;
		mover2 = NULL;

		switch (type)
		{
			case DETONATE_MAGIC_IMMUNITY_REMOVAL:
			{
				spawner = new FilledSphereSpawner(0.1);
				mover = new ParticleMover(this);
				while ((int)particles.size() < LOD * 150)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center;
					Vec3 velocity;
					velocity.randomize(0.5);
					velocity.y = 0.2;
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new MineParticle(this, mover, coords, velocity, 0.2, 1.0, 3.0, 3.0, 3.0, EC_SIMPLE, LOD, type);
#else	/* NEW_TEXTURES */
							new MineParticle(this, mover, coords, velocity, 0.2, 1.0, 3.0, 3.0, 3.0, &(base->TexSimple), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case DETONATE_UNINVIZIBILIZER:
			{
				effect_center.y += 1.8;
				spawner = new HollowDiscSpawner(0.3);
				mover = new ParticleMover(this);
				while ((int)particles.size() < LOD * 200)
				{
					Vec3 coords = spawner->get_new_coords();
					Vec3 velocity;
					velocity.randomize();
					velocity.normalize(0.2);
					coords += effect_center;
					coords.y = randfloat(1.8);
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new MineParticle(this, mover, coords, velocity, 1.2, 1.0, 3.0, 3.0, 3.0, EC_SIMPLE, LOD, type);
#else	/* NEW_TEXTURES */
							new MineParticle(this, mover, coords, velocity, 1.2, 1.0, 3.0, 3.0, 3.0, &(base->TexSimple), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case DETONATE_MANA_DRAINER:
			{
				effect_center.y += 1.6;
				spawner = new HollowDiscSpawner(0.3);
				mover = new SimpleGravityMover(this);
				while ((int)particles.size() < LOD * 50)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center;
					Vec3 velocity;
					velocity.randomize();
					velocity.normalize(0.2);
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new MineParticle(this, mover, coords, velocity, 0.3, 1.0, 0.8, 0.35, 0.7, EC_SIMPLE, LOD, type);
#else	/* NEW_TEXTURES */
							new MineParticle(this, mover, coords, velocity, 0.3, 1.0, 0.8, 0.35, 0.7, &(base->TexSimple), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case DETONATE_MANA_BURNER:
			{
				effect_center.y += 1.0;
				spawner = new FilledSphereSpawner(0.5);
				mover = new GravityMover(this, &effect_center, 8e9);
				while ((int)particles.size() < LOD * 100)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center;
					Vec3 velocity;
					velocity.randomize();
					velocity.normalize(0.9);
					Particle
						* p =
#ifdef	NEW_TEXTURES
							new MineParticle(this, mover, coords, velocity, 0.5, 0.5, 0.8, 0.35, 0.7, EC_TWINFLARE, LOD, type);
#else	/* NEW_TEXTURES */
							new MineParticle(this, mover, coords, velocity, 0.5, 0.5, 0.8, 0.35, 0.7, &(base->TexTwinflare), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case DETONATE_CALTROP:
			case DETONATE_CALTROP_POISON:
			{
				effect_center.y += 0.05;
				mover = new SimpleGravityMover(this);
				spawner = new HollowSphereSpawner(0.1);

				for (int i = 0; i < LOD * 10; i++)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_center;
					const Vec3 velocity(0.0, 5.0, 0.0);
#ifdef	NEW_TEXTURES
					Particle* p = new MineParticle(this, mover, coords, velocity, 0.75, 0.6, 0.4, (type == DETONATE_CALTROP ? 0.3 : 0.5), 0.3, EC_TWINFLARE, LOD, type);
#else	/* NEW_TEXTURES */
					Particle* p = new MineParticle(this, mover, coords, velocity, 0.75, 0.6, 0.4, (type == DETONATE_CALTROP ? 0.3 : 0.5), 0.3, &(base->TexTwinflare), LOD, type);
#endif	/* NEW_TEXTURES */
					p->state = 1;
					if (!base->push_back_particle(p))
						break;
				}

				break;
			}
			case DETONATE_TRAP:
			{
				effect_center.y += 1.25;
				mover = new OrbitalMover(this, effect_center);
				spawner = new HollowSphereSpawner(1.25);
				Particle* p;

				for (int i = 0; i < LOD * 100; i++)
				{
					Vec3 c = effect_center;
					c.y = -0.3 + (i * 0.05);
					Vec3 vel;
					vel.randomize();
					vel.normalize(2.0);
					vel *= randfloat() * 4.0;
#ifdef	NEW_TEXTURES
					p = new MineParticle(this, mover, c, vel, 0.2, 1.0, 1.0, 1.0, 1.0, EC_VOID, LOD, type);
#else	/* NEW_TEXTURES */
					p = new MineParticle(this, mover, c, vel, 0.2, 1.0, 1.0, 1.0, 1.0, &(base->TexVoid), LOD, type);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;

					dynamic_cast<OrbitalMover*>(mover)->setParticleData(p, OrbitalParticleData(i, 10,
						0.45, 10) );
				}

				break;
			}
			case DETONATE_TYPE1_SMALL:
			case DETONATE_TYPE1_MEDIUM:
			case DETONATE_TYPE1_LARGE:
			{
				spawner = new FilledSphereSpawner(0.1);
				mover = new ParticleMover(this);
				const float scale = (type == DETONATE_TYPE1_SMALL ? 0.75 : type
					== DETONATE_TYPE1_MEDIUM ? 1.25 : 2.0);
				Vec3 wind;
				wind.randomize();
				wind.normalize(0.25);
				wind.y = 0;
				for (int i = 0; i < LOD * 100 * scale; i++)
				{
					Vec3 coords = spawner->get_new_coords();
					Vec3 velocity = coords * 10.0 * sqrt(scale);
					velocity.y = fabs(velocity.y) * 3.0f;
					coords += effect_center;
					coords.y -= 0.1;
#ifdef	NEW_TEXTURES
					Particle * p = new MineParticleFire(this, mover, coords, velocity, 0.5, 1.0, 1.0, randcolor(0.75), 0.0, EC_FLARE, LOD);
#else	/* NEW_TEXTURES */
					Particle * p = new MineParticleFire(this, mover, coords, velocity, 0.5, 1.0, 1.0, randcolor(0.75), 0.0, &(base->TexFlare), LOD);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				spawner = new FilledSphereSpawner(0.5 * std::sqrt(scale));
				for (int i = 0; i < LOD * 32 * scale; i++)
				{
					Vec3 coords = spawner->get_new_coords();
					Vec3 velocity;
					coords += effect_center;
					float grey = randcolor(0.5);
					velocity.randomize();
					velocity.normalize(0.25 * scale);
					velocity.y = fabs(velocity.y) * 6.0f;
					velocity += wind;
#ifdef	NEW_TEXTURES
					Particle *p = new MineParticleSmoke(this, mover, coords, velocity, 3.0 + randcoord(3.0 * scale), 0.0, grey, grey, grey, EC_SIMPLE, LOD);
#else	/* NEW_TEXTURES */
					Particle *p = new MineParticleSmoke(this, mover, coords, velocity, 3.0 + randcoord(3.0 * scale), 0.0, grey, grey, grey, &(base->TexSimple), LOD);
#endif	/* NEW_TEXTURES */
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
		}
	}

	MineEffect::~MineEffect()
	{
		if (spawner)
			delete spawner;
		if (mover)
			delete mover;
		if (spawner2)
			delete spawner2;
		if (mover2)
			delete mover2;
		if (EC_DEBUG)
			std::cout << "MineEffect (" << this << ") destroyed." << std::endl;
	}

	bool MineEffect::idle(const Uint64 usec)
	{
		if (particles.size() == 0)
			return false;

		effect_center.x = pos->x;
		effect_center.y += usec / 3000000.0;
		effect_center.z = pos->z;

		gravity_center.y += usec / 10000000.0;

		return true;
	}
	
	MineParticleFire::MineParticleFire(Effect* _effect, ParticleMover* _mover,
		const Vec3 _pos, const Vec3 _velocity, const coord_t _size,
		const alpha_t _alpha, const color_t red, const color_t green,
#ifdef	NEW_TEXTURES
		const color_t blue, TextureEnum _texture, const Uint16 _LOD):
#else	/* NEW_TEXTURES */
		const color_t blue, Texture* _texture, const Uint16 _LOD):
#endif	/* NEW_TEXTURES */
		Particle(_effect, _mover, _pos, _velocity, _size)
	{
		color[0] = red;
		color[1] = green;
		color[2] = blue;
		texture = _texture;
		alpha = _alpha;
		flare_max = 5.0;
		flare_exp = 0.5;
		flare_frequency = 0.01;
		LOD = _LOD;
		state = 0;
	}

	bool MineParticleFire::idle(const Uint64 delta_t)
	{
		const interval_t float_time = delta_t / 1000000.0;
		const Uint64 age = get_time() - born;
		const float age_f = (float)(age)/1000000.0f;
		const alpha_t scalar = pow_randfloat(float_time * 2);
		if (age_f > 1.5)
			alpha = 2.5 - age_f;
		if (alpha < 0.01)
			return false;
		velocity *= scalar;
		return true;
	}

#ifdef	NEW_TEXTURES
	Uint32 MineParticleFire::get_texture()
	{
		return base->get_texture(texture);
	}
#else	/* NEW_TEXTURES */
	GLuint MineParticleFire::get_texture(const Uint16 res_index)
	{
		return texture->get_texture(res_index);
	}
#endif	/* NEW_TEXTURES */

	MineParticleSmoke::MineParticleSmoke(Effect* _effect, ParticleMover* _mover,
		const Vec3 _pos, const Vec3 _velocity, const coord_t _size,
		const alpha_t _alpha, const color_t red, const color_t green,
#ifdef	NEW_TEXTURES
		const color_t blue, TextureEnum _texture, const Uint16 _LOD):
#else	/* NEW_TEXTURES */
		const color_t blue, Texture* _texture, const Uint16 _LOD):
#endif	/* NEW_TEXTURES */
		Particle(_effect, _mover, _pos, _velocity, _size)
	{
		color[0] = red;
		color[1] = green;
		color[2] = blue;
		texture = _texture;
		alpha = _alpha;
		flare_max = 1.0;
		flare_exp = 1.0;
		flare_frequency = 100.0;
		LOD = _LOD;
		state = 0;
	}

	bool MineParticleSmoke::idle(const Uint64 delta_t)
	{
		if (state == 0) {
			const Uint64 age = get_time() - born;
			const float age_f = (float)(age)/1000000.0f;
			alpha = age_f * 0.125;
			if (age_f > 0.25) {
				state = 1;
				alpha = randalpha(0.375);
			}
			return true;
		}
		const interval_t float_time = delta_t / 1000000.0;
		const alpha_t scalar = pow_randfloat(float_time);
		alpha *= std::sqrt(scalar);
		if (alpha < 0.01)
			return false;
		return true;
	}

#ifdef	NEW_TEXTURES
	Uint32 MineParticleSmoke::get_texture()
	{
		return base->get_texture(texture);
	}

	float MineParticleSmoke::get_burn() const
	{
		return 0.0f;
	}
#else	/* NEW_TEXTURES */
	GLuint MineParticleSmoke::get_texture(const Uint16 res_index)
	{
		return texture->get_texture(res_index);
	}

	void MineParticleSmoke::draw(const Uint64 usec)
	{
		glEnable(GL_LIGHTING);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glNormal3f(0.0, 1.0, 0.0);
		Particle::draw(usec);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glDisable(GL_LIGHTING);
	}
#endif	/* NEW_TEXTURES */

///////////////////////////////////////////////////////////////////////////////

}
;

