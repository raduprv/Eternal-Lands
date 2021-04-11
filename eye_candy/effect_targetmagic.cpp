// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_targetmagic.h"

namespace ec
{

	// C L A S S   F U N C T I O N S //////////////////////////////////////////////

	TargetMagicParticle::TargetMagicParticle(Effect* _effect,
		ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity,
		const coord_t _size, const alpha_t _alpha, const color_t red,
		const color_t green, const color_t blue, TextureEnum _texture,
		const Uint16 _LOD, const TargetMagicEffect::TargetMagicType _type,
		ParticleSpawner* _spawner2, ParticleMover* _mover2, Vec3* _target,
		Uint16 _effect_id, Uint16 _state) :
		Particle(_effect, _mover, _pos, _velocity,
			_size * (0.5 + randcoord()))
	{
		type = _type;
		color[0] = red;
		color[1] = green;
		color[2] = blue;
		texture = _texture;
		state = _state;
		if (state != 0)
			size *= 15.0 / (_LOD + 5);
		alpha = _alpha;
		if ((type != TargetMagicEffect::HARM) && (type
			!= TargetMagicEffect::SMITE_SUMMONED))
			velocity /= size;
		else
			velocity /= 6.0;
		flare_max = 15.0;
		flare_exp = 0.4;
		flare_frequency = 30.0;
		LOD = _LOD;
		spawner2 = _spawner2;
		mover2 = _mover2;
		target = _target;
		effect_id = _effect_id;
	}

	bool TargetMagicParticle::idle(const Uint64 delta_t)
	{
		if (effect->recall)
			return false;

		const Uint64 age = get_time() - born;
		const interval_t float_time = delta_t / 1000000.0;
		Vec3 cur_target = *target;
		cur_target.y += 0.5;

		//  std::cout << "A) " << this << ", " << state << ": " << pos << ", " << alpha << std::endl;
		//  std::cout << "A) " << this << ": " << velocity << ", " << pos << std::endl;

		if ((state == 0) && (age < 500000))
		{
			switch (type)
			{
				case TargetMagicEffect::REMOTE_HEAL:
				{
					break;
				}
				case TargetMagicEffect::POISON:
				{
					const alpha_t scalar =
						pow_randfloat(float_time * 2.0f);
					alpha *= scalar;
					break;
				}
				case TargetMagicEffect::TELEPORT_TO_RANGE:
				{
					break;
				}
				case TargetMagicEffect::HARM:
				{
					pos.y = ((TargetMagicEffect*)effect)->effect_centers[0].y;
					energy = ((GravityMover*)mover)->calculate_energy(*this);
					break;
				}
				case TargetMagicEffect::LIFE_DRAIN:
				{
					const coord_t scalar = std::pow(0.5f, float_time
						* 0.8f);
					if (size < 4.0)
						size /= scalar;
					break;
				}
				case TargetMagicEffect::HEAL_SUMMONED:
				{
					break;
				}
				case TargetMagicEffect::SMITE_SUMMONED:
				{
					pos.y
						= ((TargetMagicEffect*)effect)->effect_centers[effect_id].y;
					energy = ((GravityMover*)mover)->calculate_energy(*this);
					break;
				}
				case TargetMagicEffect::DRAIN_MANA:
				{
					break;
				}
			}
		}
		else if (state == 0)
		{
			const Vec3 temp_pos = pos + velocity * float_time;
			const coord_t speed = fabs(((cur_target - temp_pos).magnitude() * 1.85) / (0.7 - (age
				- 300000) / 1000000.0));
			const Vec3* center = ((TargetMagicEffect*)effect)->pos;
			Vec3 to_source = *center - temp_pos;
			Vec3 to_target = cur_target - temp_pos;
			const coord_t dist_to_source = to_source.magnitude();
			const coord_t dist_to_target = to_target.magnitude();
			to_source /= dist_to_source;
			to_target /= dist_to_target;
			velocity = mover->nonpreserving_vec_shift(velocity, to_target
				* speed, 1.0 - std::pow(0.5f, 3 * float_time));
			energy = mover->calculate_energy(*this);

			if (((dist_to_source > dist_to_target)
				&& (to_source.angle_to_prenormalized(to_target) < PI / 4))
				|| (age > 1200000))
			{
				if (dist_to_target > 0.5)
				{
					// Bring us closer to the center (since we might have just flown past it)
					const percent_t percent = 0.5 / dist_to_target;
					const percent_t inv_percent = 1.0 - percent;
					pos = pos * percent + cur_target * inv_percent;
				}
				((TargetMagicEffect*)effect)->effect_count++;
				base->push_back_effect(new TargetMagicEffect2(base, (TargetMagicEffect*)effect, ((TargetMagicEffect*)effect)->targets[0], type, spawner2, mover2, ((TargetMagicEffect*)effect)->target_alpha, effect_id, LOD));
				return false;
			}

			switch (type)
			{
				case TargetMagicEffect::REMOTE_HEAL:
				case TargetMagicEffect::POISON:
				case TargetMagicEffect::TELEPORT_TO_RANGE:
				case TargetMagicEffect::HARM:
				{
					break;
				}
				case TargetMagicEffect::LIFE_DRAIN:
				{
					const color_t scalar = std::pow(0.5f, float_time
						* 4.0f);
					if (color[0] > 0.2)
						color[0] *= scalar;
					if (color[1] < 1.0)
						color[1] /= scalar;
					break;
				}
				case TargetMagicEffect::HEAL_SUMMONED:
				case TargetMagicEffect::SMITE_SUMMONED:
				case TargetMagicEffect::DRAIN_MANA:
				{
					break;
				}
			}
		}
		else
		{
			switch (type)
			{
				case TargetMagicEffect::REMOTE_HEAL:
				{
					if (alpha < 0.01)
						return false;

					const float scalar =
						pow_randfloat(float_time * 5.0f);
					energy *= scalar;
					if (size < 10)
						size /= scalar;
					alpha *= scalar;
					break;
				}
				case TargetMagicEffect::POISON:
				{
					if (alpha < 0.01)
						return false;

					const float scalar =
						pow_randfloat(float_time * 5.0f);
					energy *= scalar;
					if (size < 6)
						size /= scalar;
					alpha *= scalar;
					break;
				}
				case TargetMagicEffect::TELEPORT_TO_RANGE:
				{
					if (alpha < 0.03)
						return false;

					const alpha_t scalar =
						std::pow(0.5f, (float)delta_t / 500000);
					alpha *= scalar;
					break;
				}
				case TargetMagicEffect::HARM:
				{
					if (pos.y < target->y - 0.5)
						return false;
					break;
				}
				case TargetMagicEffect::LIFE_DRAIN:
				{
					if (alpha < 0.01)
						return false;

					const float scalar =
						pow_randfloat(float_time * 5.0f);
					if (color[0] > 0.2)
						color[0] *= scalar;
					if (color[1] < 1.0)
						color[1] /= scalar;
					energy *= scalar;
					if (size < 5)
						size /= scalar;
					alpha *= scalar;
					break;
				}
				case TargetMagicEffect::HEAL_SUMMONED:
				{
					if (alpha < 0.01)
						return false;

					const float scalar =
						pow_randfloat(float_time * 5.0f);
					energy *= scalar;
					if (size < 5)
						size /= scalar;
					alpha *= scalar;
					break;
				}
				case TargetMagicEffect::SMITE_SUMMONED:
				{
					if (pos.y < target->y - 0.5)
						return false;
					break;
				}
				case TargetMagicEffect::DRAIN_MANA:
				{
					if (alpha < 0.01)
						return false;

					const float scalar =
						pow_randfloat(float_time * 2.5f);
					energy *= scalar;
					alpha *= scalar;
					break;
				}
			}
			pos += ((TargetMagicEffect2*)effect)->shift;
		}

		//  std::cout << "B) " << this << ": " << velocity << ", " << pos << std::endl;

		if (pos.y < effect->pos->y)
		{
			pos.y = effect->pos->y;
		}
		return true;
	}

	Uint32 TargetMagicParticle::get_texture()
	{
		return base->get_texture(texture);
	}

	float TargetMagicParticle::get_burn() const
	{
		if (((type == TargetMagicEffect::POISON) &&
			(state == 2)) ||
			(((type == TargetMagicEffect::HARM) ||
			(type == TargetMagicEffect::SMITE_SUMMONED)) &&
			(state) && (rand() & 1)))
		{
			return 0.0f;
		}
		else
		{
			return 1.0f;
		}
	}

	TargetMagicEffect::TargetMagicEffect(EyeCandy* _base, bool* _dead,
		Vec3* _pos, Vec3* _target, const TargetMagicType _type,
		std::vector<ec::Obstruction*>* _obstructions, const Uint16 _LOD)
	{
		if (EC_DEBUG)
			std::cout << "TargetMagicEffect (" << this << ", " << _type
				<< ") created(1)." << std::endl;
		std::vector<Vec3*> new_targets;
		new_targets.push_back(_target);
		//  std::cout << "Target: " << _target << std::endl;
		initialize(_base, _dead, _pos, new_targets, _type, _obstructions, _LOD);
	}

	TargetMagicEffect::TargetMagicEffect(EyeCandy* _base, bool* _dead,
		Vec3* _pos, const std::vector<Vec3*>& _targets,
		const TargetMagicType _type,
		std::vector<ec::Obstruction*>* _obstructions, const Uint16 _LOD)
	{
		if (EC_DEBUG)
			std::cout << "TargetMagicEffect (" << this << ", " << _type
				<< ") created(2)." << std::endl;
		initialize(_base, _dead, _pos, _targets, _type, _obstructions, _LOD);
	}

	void TargetMagicEffect::initialize(EyeCandy* _base, bool* _dead,
		Vec3* _pos, const std::vector<Vec3*>& _targets,
		const TargetMagicType _type,
		std::vector<ec::Obstruction*>* _obstructions, const Uint16 _LOD)
	{
		base = _base;
		dead = _dead;
		targets = _targets;
		pos = _pos;
		type = _type;
		obstructions = _obstructions;
		LOD = _LOD;
		desired_LOD = _LOD;
		bounds = NULL;
		spawner = NULL;
		mover = NULL;
		spawner2 = NULL;
		mover2 = NULL;
		target_alpha = NULL;
		effect_count = 0;
		particle_count = 0;

		switch (type)
		{
			case REMOTE_HEAL:
			{
				effect_centers.push_back(*_pos);

				spawner = new FilledDiscSpawner(0.05);
				spawner2 = new SierpinskiIFSParticleSpawner(1.25);
				mover = new GravityMover(this, &(effect_centers[0]), 1e6);
				const Vec3 direction = (*targets[0] - *pos).normalize();
				const Vec3 coords = spawner->get_new_coords()
					+ effect_centers[0];
				Vec3 velocity;
				velocity.randomize();
				velocity -= direction;
				velocity.y += 10.0;
				Particle * p = new TargetMagicParticle(this, mover, coords, velocity, 7.5, 1.0, 0.3 + randcolor(0.3), 0.7, 0.2, EC_FLARE, LOD, type, spawner2, mover, targets[0], 0, 0);
				base->push_back_particle(p);
				particle_count = 1;
				break;
			}
			case POISON:
			{
				effect_centers.push_back(*_pos);

				spawner = new HollowSphereSpawner(0.15);
				mover = new GravityMover(this, &(effect_centers[0]), 1e6);
				mover2 = new SimpleGravityMover(this);
				while (particles.size() < 16)
				{
					const Vec3 coords = spawner->get_new_coords() * 0.75
						+ effect_centers[0];
					Vec3 velocity;
					velocity.randomize(0.5);
					Particle * p = new TargetMagicParticle(this, mover, coords, velocity, 8.0, 1.0, randcolor(0.3), 0.5 + randcolor(0.3), randcolor(0.5), EC_INVERSE, LOD, type, NULL, mover2, targets[0], 0, 0);
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case TELEPORT_TO_RANGE:
			{
				effect_centers.push_back(*_pos);

				mover = new ParticleMover(this);
				mover2 = new GravityMover(this, &(effect_centers[0]), 6e10);
				spawner = new FilledDiscSpawner(0.2);

				while (particles.size() < 12)
				{
					const Vec3 coords = spawner->get_new_coords() * 0.75
						+ effect_centers[0];
					Vec3 velocity;
					velocity.randomize(0.5);
					Particle * p = new TargetMagicParticle(this, mover2, coords, velocity, 3.75, 0.8 + randcoord(0.2), 1.0, 1.0, 1.0, EC_VOID, LOD, type, spawner, mover, targets[0], 0, 0);
					if (!base->push_back_particle(p))
						break;
				}

				const float sqrt_LOD= std::sqrt(LOD);
				const coord_t size_scalar = 15 / (LOD + 5);
				for (int i = 0; i < LOD * 100; i++)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_centers[0] + Vec3(0.0, -0.7 + randcoord() * randcoord() * 8.0 * sqrt_LOD, 0.0);
					Vec3 velocity;
					velocity.randomize(0.2);
					const coord_t size = size_scalar * (0.5 + 1.5 * randcoord());
					velocity /= size;
					Particle * p = new TargetMagicParticle(this, mover, coords, velocity, size, 1.0, 0.8 + randcolor(0.2), 0.8 + randcolor(0.2), 0.8 + randcolor(0.2), EC_SHIMMER, LOD, type, spawner, mover, targets[0], 0, 0);
					p->state = 1;
					if (!base->push_back_particle(p))
						break;
				}

				//      const float radius = 0.5 * powf(2, 0.18) / 1.5;
				const float radius = 0.377628;
				for (int i = 0; i < LOD * 2; i++)
				{
					const percent_t percent = ((percent_t)i + 1) / (LOD * 2);
					capless_cylinders.push_back(new CaplessCylinder(base, effect_centers[0] - Vec3(0.0, 0.7, 0.0), effect_centers[0] + Vec3(0.0, -0.7 + 10.0 / percent, 0.0), Vec3(1.0, 1.0, 1.0), (0.1 + (1.0 - percent) * 0.05) / (LOD / 2.0 + 2), radius * percent, (int)(25 * (percent + 0.2))));
				}

				break;
			}
			case HARM:
			{
				effect_centers.push_back(*_pos);

				mover = new ParticleMover(this);
				mover2 = new GravityMover(this, &(effect_centers[0]), 1e6);
				spawner = new FilledDiscSpawner(0.2);

				while (particles.size() < 16)
				{
					const Vec3 coords = spawner->get_new_coords() * 0.75
						+ effect_centers[0];
					Vec3 velocity;
					velocity.randomize(0.5);
					Particle * p = new TargetMagicParticle(this, mover2, coords, velocity, 12.0, 1.0, 0.7 + randcolor(0.3), 0.2 + randcolor(0.3), 0.2, EC_FLARE, LOD, type, spawner, mover, targets[0], 0, 0);
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case LIFE_DRAIN:
			{
				pos = targets[0]; //Redoing these variables, since this effect runs backwards.
				targets[0] = _pos;
				target = *targets[0];
				effect_centers.push_back(*pos);

				spawner = new FilledDiscSpawner(0.75);
				mover = new GravityMover(this, &(effect_centers[0]), 1e11);
				mover2 = new GravityMover(this, &target, 5e10);
				const Vec3 direction = (*targets[0] - *pos).normalize();
				for (int i = 0; i < 8; i++)
				{
					const Vec3 coords = spawner->get_new_coords() * 0.75
						+ target;
					Vec3 velocity;
					velocity.randomize(0.5);
					velocity += direction * -2.0 * i;
					velocity.y += i;
					Particle * p = new TargetMagicParticle(this, mover2, coords, velocity, 7.5, 1.0, 0.7 + randcolor(0.3), 0.25 + randcolor(0.25), 0.15 + randcolor(0.15), EC_VOID, LOD, type, spawner, mover2, targets[0], 0, 2);
					if (!base->push_back_particle(p))
						break;
				}
				for (int i = 0; i < 8; i++)
				{
					const Vec3 coords = spawner->get_new_coords() * 0.75
						+ effect_centers[0];
					Vec3 velocity;
					velocity.randomize(0.3);
					velocity += direction * -2.0 * i;
					velocity.y += i;
					Particle * p = new TargetMagicParticle(this, mover, coords, velocity, 15.0, 1.0, 0.7 + randcolor(0.3), 0.15 + randcolor(0.15), 0.25 + randcolor(0.25), EC_TWINFLARE, LOD, type, spawner, mover, targets[0], 0, 0);
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case HEAL_SUMMONED:
			{
				for (int i = 0; i < (int)targets.size(); i++)
					effect_centers.push_back(*_pos);

				spawner = new FilledDiscSpawner(0.75);
				spawner2 = new SierpinskiIFSParticleSpawner();
				mover = new GravityMover(this, &(effect_centers[0]), 1e11);
				for (int i = 0; i < 4 * (int)targets.size(); i++)
				{
					const Vec3 coords = spawner->get_new_coords() * 0.75
						+ effect_centers[i / 4];
					Vec3 velocity;
					velocity.randomize(0.5);
					Particle * p = new TargetMagicParticle(this, mover, coords, velocity, 8.25, 1.0, 0.25 + randcolor(0.4), 0.6 + randcolor(0.3), 0.15 + randcolor(0.1), EC_TWINFLARE, LOD, type, spawner2, mover, targets[i / 4], i / 4, 0);
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case SMITE_SUMMONED:
			{
				for (int i = 0; i < (int)targets.size(); i++)
					effect_centers.push_back(*_pos);

				mover = new ParticleMover(this);
				mover2 = new GravityMover(this, &(effect_centers[0]), 6e10);
				spawner = new FilledDiscSpawner(0.2);

				for (int i = 0; i < 4 * (int)targets.size(); i++)
				{
					const Vec3 coords = spawner->get_new_coords() * 0.75
						+ effect_centers[i / 4];
					Vec3 velocity;
					velocity.randomize(0.5);
					Particle * p = new TargetMagicParticle(this, mover2, coords, velocity, 14.25, 1.0, 0.7 + randcolor(0.3), 0.2 + randcolor(0.3), 0.2, EC_TWINFLARE, LOD, type, spawner, mover, targets[i / 4], i / 4, 0);
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case DRAIN_MANA: //Use crystal particles.
			{
				pos = targets[0]; //Redoing these variables, since this effect runs backwards.
				targets[0] = _pos;
				target = *targets[0];
				effect_centers.push_back(*pos);

				spawner = new FilledDiscSpawner(0.25);
				spawner2 = new HollowSphereSpawner(0.25);
				mover = new GravityMover(this, &(effect_centers[0]), 8e9);
				mover2 = new GravityMover(this, &target, 4e10);
				const Vec3 direction = (*targets[0] - *pos).normalize();
				for (int i = 0; i < 8; i++)
				{
					const Vec3 coords = spawner->get_new_coords() * 0.75
						+ target;
					Vec3 velocity;
					velocity.randomize(0.45);
					velocity += direction * -2.0 * i;
					velocity.y += i;
					Particle * p = new TargetMagicParticle(this, mover2, coords, velocity, 7.5, 1.0, 0.7 + randcolor(0.3), 0.15 + randcolor(0.15), 0.6 + randcolor(0.35), EC_VOID, LOD, type, spawner2, mover2, targets[0], 0, 2);
					if (!base->push_back_particle(p))
						break;
				}
				for (int i = 0; i < 8; i++)
				{
					const Vec3 coords = spawner->get_new_coords()
						+ effect_centers[0];
					Vec3 velocity;
					velocity.randomize(0.85);
					velocity += direction * -2.0 * i;
					velocity.y += i;
					Particle * p = new TargetMagicParticle(this, mover, coords, velocity, 7.0, 1.0, 0.7 + randcolor(0.3), 0.15 + randcolor(0.15), 0.6 + randcolor(0.35), EC_CRYSTAL, LOD, type, spawner2, mover, targets[0], 0, 0);
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
		}
	}

	TargetMagicEffect::~TargetMagicEffect()
	{
		if (spawner)
			delete spawner;
		if (mover)
			delete mover;
		if (spawner2)
			delete spawner2;
		if (mover2)
			delete mover2;
		for (size_t i = 0; i < capless_cylinders.size(); i++)
			delete reinterpret_cast<CaplessCylinder *>(capless_cylinders[i]);
		capless_cylinders.clear();
		if (EC_DEBUG)
			std::cout << "TargetMagicEffect (" << this << ") destroyed."
				<< std::endl << std::flush;
	}

	bool TargetMagicEffect::idle(const Uint64 usec)
	{
		if ((particles.size() == 0) && (effect_count == 0))
			return false;

		if (recall)
			return true;

		const Uint64 cur_time = get_time();
		const Uint64 age = cur_time - born;
		for (int i = 0; i < (int)effect_centers.size(); i++)
		{
			if (age < 500000)
			{
				effect_centers[i].x = pos->x;
				effect_centers[i].y += usec / 1500000.0;
				effect_centers[i].z = pos->z;
			}
			else
			{
				const percent_t percent = fabs((interval_t)(age - 300000)
					/ 700000.0);
				const percent_t inv_percent = 1.0 - percent;
				effect_centers[i].x = (pos->x * inv_percent) + (targets[i]->x
					* percent);
				effect_centers[i].y = (pos->y * inv_percent) + (targets[i]->y
					* percent) + 0.5;
				effect_centers[i].z = (pos->z * inv_percent) + (targets[i]->z
					* percent);
			}
		}

		switch (type)
		{
			case REMOTE_HEAL:
			{
				if ((particle_count == 1) && (age > 200000))
				{
					particle_count = 2;
					const Vec3 direction = (*targets[0] - *pos).normalize();
					const Vec3 coords = spawner->get_new_coords() + *pos;
					Vec3 velocity;
					velocity.randomize(particle_count);
					velocity -= direction * particle_count;
					velocity.y += particle_count * 10.0;
					Particle * p = new TargetMagicParticle(this, mover, coords, velocity, 7.5, 1.0, 0.3 + randcolor(0.3), 0.7, 0.2, EC_FLARE, LOD, type, spawner2, mover, targets[0], 0, 0);
					base->push_back_particle(p);
				}
				if ((particle_count == 2) && (age > 400000))
				{
					particle_count = 3;
					const Vec3 direction = (*targets[0] - *pos).normalize();
					const Vec3 coords = spawner->get_new_coords() + *pos;
					Vec3 velocity;
					velocity.randomize(particle_count);
					velocity -= direction * particle_count;
					velocity.y += particle_count * 10.0;
					Particle * p = new TargetMagicParticle(this, mover, coords, velocity, 7.5, 1.0, 0.3 + randcolor(0.3), 0.7, 0.2, EC_FLARE, LOD, type, spawner2, mover, targets[0], 0, 0);
					base->push_back_particle(p);
				}
				if ((particle_count == 3) && (age > 600000))
				{
					particle_count = 4;
					const Vec3 direction = (*targets[0] - *pos).normalize();
					const Vec3 coords = spawner->get_new_coords() + *pos;
					Vec3 velocity;
					velocity.randomize(particle_count);
					velocity -= direction * particle_count;
					velocity.y += particle_count * 10.0;
					Particle * p = new TargetMagicParticle(this, mover, coords, velocity, 7.5, 1.0, 0.3 + randcolor(0.3), 0.7, 0.2, EC_FLARE, LOD, type, spawner2, mover, targets[0], 0, 0);
					base->push_back_particle(p);
				}
			}
			case POISON:
			{
				break;
			}
			case TELEPORT_TO_RANGE:
			{
				if (age > 1200000)
				{
					const alpha_t scalar =
						std::pow(0.5f, (interval_t)usec / 400000);
					for (int i = 0; i < LOD * 2; i++)
						capless_cylinders[i]->alpha *= scalar;
					if ((age > 2200000) && ((int)capless_cylinders.size()
						== LOD * 4))
					{
						for (int i = LOD * 2; i < LOD * 4; i++)
							capless_cylinders[i]->alpha *= scalar;
					}

					if (target_alpha)
					{
						if (age < 500000)
						{
							*target_alpha = 1.0 - (age / 500000.0);
						}
						else if (age < 600000)
						{
							*target_alpha = 0.0;
						}
					}
				}
				break;
			}
			case HARM:
			{
				effect_centers[0].y = square(age / 100000.0) + 0.5;
				break;
			}
			case LIFE_DRAIN:
			{
				target.x = targets[0]->x;
				target.y = targets[0]->y;
				target.z = targets[0]->z;
				break;
			}
			case HEAL_SUMMONED:
			{
				break;
			}
			case SMITE_SUMMONED:
			{
				for (int i = 0; i < (int)targets.size(); i++)
					effect_centers[i].y = square(age / 100000.0) + 0.5;
				break;
			}
			case DRAIN_MANA:
			{
				target.x = targets[0]->x;
				target.y = targets[0]->y;
				target.z = targets[0]->z;
				break;
			}
		}
		return true;
	}

	void TargetMagicEffect::draw(const Uint64 usec)
	{
		for (auto cylinder: capless_cylinders)
			cylinder->draw();
	}

	TargetMagicEffect2::TargetMagicEffect2(EyeCandy* _base,
		TargetMagicEffect* _effect, Vec3* _pos,
		const TargetMagicEffect::TargetMagicType _type,
		ParticleSpawner* _spawner, ParticleMover* _mover, float* _target_alpha,
		Uint16 _effect_id, const Uint16 _LOD)
	{
		base = _base;
		effect = _effect;
		pos = _pos;
		center = *pos;
		type = _type;
		LOD = base->last_forced_LOD;
		spawner = _spawner;
		mover = _mover;
		effect_id = _effect_id;
		target_alpha = _target_alpha;
		dead = &dummy_dead;
		shift = Vec3(0.0, 0.0, 0.0);

		//  std::cout << "Target center: " << center << std::endl << std::flush;

		switch (type)
		{
			case TargetMagicEffect::REMOTE_HEAL:
			{
				for (int i = 0; i < 3 * LOD; i++)
				{
					Vec3 coords = spawner->get_new_coords() * 0.05;
					Vec3 velocity = -coords * 30;
					velocity.y += 0.5;
					coords += center;
					Particle * p = new TargetMagicParticle(this, mover, coords, velocity, 0.7, 1.0, 0.4 + randcolor(0.3), 0.7, 0.2, EC_FLARE, LOD, type, NULL, NULL, &center, effect_id, 1);
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case TargetMagicEffect::POISON:
			{
				for (int i = 0; i < 3 * LOD; i++)
				{
					Vec3 velocity;
					velocity.randomize(0.2);
					velocity.y += 1.0;
					Particle *p;
					if (randfloat() < 0.4)
						p = new TargetMagicParticle(this, mover, center, velocity, 0.75, 1.0, 0.2 + randcolor(0.2), 0.5 + randcolor(0.3), 0.2, EC_FLARE, LOD, type, NULL, NULL, &center, effect_id, 1);
					else
						p = new TargetMagicParticle(this, mover, center, velocity, 0.75, 1.0, randcolor(0.1), 0.2 + randcolor(0.1), 0.2, EC_WATER, LOD, type, NULL, NULL, &center, effect_id, 2);
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case TargetMagicEffect::TELEPORT_TO_RANGE:
			{
				center = *(effect->targets[0]);
				const float sqrt_LOD= std::sqrt(LOD);
				const coord_t size_scalar = 15 / (LOD + 5);
				for (int i = 0; i < LOD * 3; i++)
				{
					const Vec3 coords = spawner->get_new_coords() + center
						+ Vec3(0.0, randcoord() * randcoord() * 8.0 * sqrt_LOD, 0.0);
					Vec3 velocity(0.0, randcoord(0.1), 0.0);
					velocity.randomize(0.2);
					const coord_t size = size_scalar * (0.5 + 1.5 * randcoord());
					velocity /= size;
					Particle * p = new TargetMagicParticle(this, mover, coords, velocity, size, 1.0, 0.8 + randcolor(0.2), 0.8 + randcolor(0.2), 0.8 + randcolor(0.2), EC_SHIMMER, LOD, type, spawner, mover, effect->targets[0], effect_id, 1);
					if (!base->push_back_particle(p))
						break;
				}

				//      const float radius = 0.5 * powf(2, 0.18) / 1.5;
				const float radius = 0.377628;
				if ((int)effect->capless_cylinders.size() < LOD * 4)
				{
					for (int i = 0; i < LOD * 2; i++)
					{
						const percent_t percent = ((percent_t)i + 1)
							/ (LOD * 2);
						effect->capless_cylinders.push_back(new CaplessCylinder(base, center, center + Vec3(0.0, 10.0 / percent, 0.0), Vec3(1.0, 1.0, 1.0), (0.1 + (1.0 - percent) * 0.05) / (LOD / 2.0 + 2), radius * percent, (int)(25 * (percent + 0.2))));
					}
				}
				break;
			}
			case TargetMagicEffect::HARM:
			{
				center = *(effect->targets[0]);
				motion_blur_points = 3;
				motion_blur_fade_rate = 0.0001;
				for (int i = 0; i < 3 * LOD; i++)
				{
					Vec3 coords = spawner->get_new_coords() * 1.2;
					Vec3 velocity(randcoord(1.0), (-70.0 + randcoord(10.0)) * 10.0 / LOD, randcoord(1.0));
					coords += center;
					coords.y += randcoord(16.0) - 8.0;
					Particle* p;
					if (i & 1)
						p = new TargetMagicParticle(this, mover, coords, velocity, 2.0 + randcoord(6.0), 0.7 + randalpha(0.3), 1.0, 1.0, 1.0, EC_SHIMMER, LOD, type, NULL, NULL, &center, effect_id, 1);
					else
						p = new TargetMagicParticle(this, mover, coords, velocity, 2.0 + randcoord(6.0), 1.0, 0.6 + randcolor(0.4), randcolor(0.5), randcolor(0.5), EC_SHIMMER, LOD, type, NULL, NULL, &center, effect_id, 1);
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case TargetMagicEffect::LIFE_DRAIN:
			{
				center = *(effect->targets[0]);
				for (int i = 0; i < 3 * LOD; i++)
				{
					Vec3 coords = spawner->get_new_coords() * 0.05;
					Vec3 velocity = coords * 30;
					velocity.y += 0.5;
					coords += center;
					Particle * p = new TargetMagicParticle(this, mover, coords, velocity, 0.6, 1.0, 0.4 + randcolor(0.3), 0.7, 0.2, EC_FLARE, LOD, type, NULL, NULL, &center, effect_id, 1);
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case TargetMagicEffect::HEAL_SUMMONED:
			{
				for (int i = 0; i < 3 * LOD; i++)
				{
					Vec3 coords = spawner->get_new_coords() * 0.05;
					Vec3 velocity = coords * 30;
					velocity.y += 0.25;
					coords += center;
					Particle * p = new TargetMagicParticle(this, mover, coords, velocity, 1.0, 1.0, 0.4 + randcolor(0.3), 0.7, 0.2, EC_CRYSTAL, LOD, type, NULL, NULL, &center, effect_id, 1);
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case TargetMagicEffect::SMITE_SUMMONED:
			{
				center = *(effect->targets[effect_id]);
				motion_blur_points = 3;
				motion_blur_fade_rate = 0.0001;
				for (int i = 0; i < 3 * LOD; i++)
				{
					Vec3 coords = spawner->get_new_coords() * 1.2;
					Vec3 velocity(randcoord(1.0), (-100.0 + randcoord(15.0)) * 10.0 / LOD, randcoord(1.0));
					coords += center;
					coords.y += randcoord(10.0) + 8.0;
					Particle* p;
					if (rand() & 1)
						p = new TargetMagicParticle(this, mover, coords, velocity, 3.5 + randcoord(7.0), 0.5 + randalpha(0.3), 1.0, 1.0, 1.0, EC_SHIMMER, LOD, type, NULL, NULL, &center, effect_id, 1);
					else
						p = new TargetMagicParticle(this, mover, coords, velocity, 3.5 + randcoord(7.0), 1.0, 0.3 + randcolor(0.4), 0.3 + randcolor(0.5), randcolor(0.5), EC_SHIMMER, LOD, type, NULL, NULL, &center, effect_id, 1);
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
			case TargetMagicEffect::DRAIN_MANA:
			{
				center = *(effect->targets[0]);
				for (int i = 0; i < 3 * LOD; i++)
				{
					Vec3 coords = spawner->get_new_coords() * 0.05;
					Vec3 velocity = coords * 20;
					coords += center;
					Particle * p = new TargetMagicParticle(this, mover, coords, velocity, 1.6, 1.0, 0.4 + randcolor(0.3), 0.2, 0.7, EC_CRYSTAL, LOD, type, NULL, NULL, &center, effect_id, 1);
					if (!base->push_back_particle(p))
						break;
				}
				break;
			}
		}
	}

	TargetMagicEffect2::~TargetMagicEffect2()
	{
		/*if (spawner)
		 delete spawner;
		 if (mover)
		 delete mover;
		 if (EC_DEBUG)
		 std::cout << "TargetMagicEffect2 (" << this << ") destroyed." << std::endl << std::flush;*/
	}

	bool TargetMagicEffect2::idle(const Uint64 usec)
	{
		if (particles.size() == 0)
		{
			effect->effect_count--;
			return false;
		}

		if (type == TargetMagicEffect::TELEPORT_TO_RANGE)
		{
			if (target_alpha != NULL)
			{
				const Uint64 cur_time = get_time();
				const Uint64 age = cur_time - born;
				if (age < 500000)
				{
					*target_alpha = age / 500000.0;
				}
				else
				{
					*target_alpha = 1.0;
				}
			}
		}

		const Vec3 last_effect_center = center;

		center.x = pos->x;
		center.z = pos->z;

		//  std::cout << "Center: " << center << "; Pos (" << pos << "): " << *pos << std::endl << std::flush;

		shift = center - last_effect_center;

		center.y += usec / 1500000.0;

		return true;
	}

///////////////////////////////////////////////////////////////////////////////

}
;

