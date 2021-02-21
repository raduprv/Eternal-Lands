// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"
#include "effect_wind.h"
#include "../textures.h"

namespace ec
{

	const float range_scalar = 1.0;

	// C L A S S   F U N C T I O N S //////////////////////////////////////////////

	WindParticle::WindParticle(Effect* _effect, ParticleMover* _mover,
		const Vec3 _pos, const Vec3 _velocity, const color_t hue_adjust,
		const color_t saturation_adjust, const coord_t scalar,
		const coord_t _min_height, const coord_t _max_height,
		const WindEffect::WindType _type) :
		Particle(_effect, _mover, _pos, _velocity)
	{
		state = 0;
		flare_max = 1.0;
		flare_exp = 1.0;
		flare_frequency = 1.0;
		min_height = _min_height;
		max_height = _max_height;
		type = _type;
		angle_t angle, rise;
		color_t hue = 0.0, saturation = 0.0, value = 0.0;
		switch (type)
		{
			case WindEffect::LEAVES:
			{
				hue = 0.0 + randcolor(0.08);
				saturation = 0.5 + randcolor(0.4);
				value = 0.35 + randcolor(0.20);
				size = 0.08 * scalar;
				alpha = 1.0;
				subtype = rand() % 3; // Store it in case we need it later -- say, for a texture.
				switch (subtype)
				{
					case 0: // Maple
					{
						angle = randangle(30 * (PI / 180)) + (240 * (PI / 180));
						rise = randangle(30 * (PI / 180)) + (-10 * (PI / 180));
						rotation_axes[0] = Vec3(sin(angle) * cos(rise),
							sin(rise), cos(angle) * cos(rise));
						axis_weights[0] = (sin(rise) + 0.2) * (randangle(0.3) + 0.85);
						angle = -randangle(30 * (PI / 180)) + (120 * (PI / 180));
						rise = randangle(30 * (PI / 180)) + (-10 * (PI / 180));
						rotation_axes[1] = Vec3(sin(angle) * cos(rise),
							sin(rise), cos(angle) * cos(rise));
						axis_weights[1] = (sin(rise) + 0.2) * (randangle(0.3) + 0.85);
						angle = randangle(30 * (PI / 180)) + (165 * (PI / 180));
						rise = randangle(30 * (PI / 180)) + (-10 * (PI / 180));
						rotation_axes[2] = Vec3(sin(angle) * cos(rise),
							sin(rise), cos(angle) * cos(rise));
						axis_weights[2] = (sin(rise) + 0.2) * (randangle(0.3) + 1.0);
						break;
					}
					case 1: // Oak
					{
						angle = randangle(30 * (PI / 180)) + (220 * (PI / 180));
						rise = randangle(40 * (PI / 180)) + (-12 * (PI / 180));
						rotation_axes[0] = Vec3(sin(angle) * cos(rise),
							sin(rise), cos(angle) * cos(rise));
						axis_weights[0] = (sin(rise) + 0.2) * (randangle(0.2) + 0.5);
						angle = -randangle(30 * (PI / 180)) + (140 * (PI / 180));
						rise = randangle(40 * (PI / 180)) + (-12 * (PI / 180));
						rotation_axes[1] = Vec3(sin(angle) * cos(rise),
							sin(rise), cos(angle) * cos(rise));
						axis_weights[1] = (sin(rise) + 0.2) * (randangle(0.2) + 0.5);
						angle = randangle(20 * (PI / 180)) + (170 * (PI / 180));
						rise = randangle(20 * (PI / 180)) + (-7 * (PI / 180));
						rotation_axes[2] = Vec3(sin(angle) * cos(rise),
							sin(rise), cos(angle) * cos(rise));
						axis_weights[2] = (sin(rise) + 0.2) * (randangle(0.3) + 1.0);
						break;
					}
					case 2: // Ash
					{
						angle = randangle(30 * (PI / 180)) + (200 * (PI / 180));
						rise = randangle(40 * (PI / 180)) + (-12 * (PI / 180));
						rotation_axes[0] = Vec3(sin(angle) * cos(rise),
							sin(rise), cos(angle) * cos(rise));
						axis_weights[0] = (sin(rise) + 0.2) * (randangle(0.15) + 0.3);
						angle = -randangle(30 * (PI / 180)) + (160 * (PI / 180));
						rise = randangle(40 * (PI / 180)) + (-12 * (PI / 180));
						rotation_axes[1] = Vec3(sin(angle) * cos(rise),
							sin(rise), cos(angle) * cos(rise));
						axis_weights[1] = (sin(rise) + 0.2) * (randangle(0.15) + 0.3);
						angle = randangle(20 * (PI / 180)) + (170 * (PI / 180));
						rise = randangle(15 * (PI / 180)) + (-6 * (PI / 180));
						rotation_axes[2] = Vec3(sin(angle) * cos(rise),
							sin(rise), cos(angle) * cos(rise));
						axis_weights[2] = (sin(rise) + 0.2) * (randangle(0.3) + 1.0);
						break;
					}
				}
				break;
			}
			case WindEffect::FLOWER_PETALS:
			{
				hue = 0.0;
				saturation = 0.41 + randcolor(0.26);
				value = 0.6;
				size = (0.035 + randangle(0.025)) * scalar;
				alpha = 0.9;

				angle = randangle(60 * (PI / 180)) + (210 * (PI / 180));
				rise = randangle(60 * (PI / 180)) + (-20 * (PI / 180));
				rotation_axes[0] = Vec3(sin(angle) * cos(rise), sin(rise),
					cos(angle) * cos(rise));
				axis_weights[0] = (sin(rise) + 0.2) * (randangle(0.3) + 0.5);
				angle = -randangle(60 * (PI / 180)) + (90 * (PI / 180));
				rise = randangle(60 * (PI / 180)) + (-20 * (PI / 180));
				rotation_axes[1] = Vec3(sin(angle) * cos(rise), sin(rise),
					cos(angle) * cos(rise));
				axis_weights[1] = (sin(rise) + 0.2) * (randangle(0.3) + 0.5);
				angle = randangle(60 * (PI / 180)) + (150 * (PI / 180));
				rise = randangle(60 * (PI / 180)) + (-20 * (PI / 180));
				rotation_axes[2] = Vec3(sin(angle) * cos(rise), sin(rise),
					cos(angle) * cos(rise));
				axis_weights[2] = (sin(rise) + 0.2) * (randangle(0.3) + 0.5);
				break;
			}
			case WindEffect::SNOW:
			{
				hue = 0.67;
				saturation = 0.05;
				value = 0.85 + randcolor(0.15);
				size = (0.006 + randcolor(0.004)) * scalar;
				alpha = 0.7;

				angle = 240 * (PI / 180);
				rotation_axes[0] = Vec3(sin(angle), 0.0, cos(angle));
				axis_weights[2] = randangle(0.1) + 0.5;
				angle = 120 * (PI / 180);
				rotation_axes[1] = Vec3(sin(angle), 0.0, cos(angle));
				axis_weights[2] = randangle(0.1) + 0.5;
				angle = 180 * (PI / 180);
				rotation_axes[2] = Vec3(sin(angle), 0.0, cos(angle));
				axis_weights[2] = randangle(0.1) + 0.5;
				break;
			}
		}

		//  std::cout << this << ", " << pos << ": " << "Leaf created." <<
		//  std::endl;

		/*
		 // Normalize axis weights
		 percent_t sum = 0;
		 for (int i = 0; i < 3; i++)
		 coord_t distance_squared = (camera - *(e->pos)).magnitude_squared();
		 sum += axis_weights[i];
		 for (int i = 0; i < 3; i++)
		 axis_weights[i] /= sum;
		 */
		hue += hue_adjust;
		if (hue > 1.0)
			hue -= 1.0;
		saturation = std::min(1.0f, saturation * saturation_adjust);
		hsv_to_rgb(hue, saturation, value, color[0], color[1], color[2]);
	}

	bool WindParticle::idle(const Uint64 delta_t)
	{
		if (effect->recall)
			return false;

		WindEffect* wind_effect = (WindEffect*)effect;
		const Uint64 age = get_time() - born;
		const interval_t usec = delta_t / 1000000.0;
		const Vec3 cur_wind = get_wind_vec();

		//  std::cout << "Wind vec: " << cur_wind.magnitude() << ", " << cur_wind << std::endl;
		float divisor;
		switch (type)
		{
			case WindEffect::LEAVES:
			{
				divisor = 100000;
				break;
			}
			case WindEffect::FLOWER_PETALS:
			{
				divisor = 200000;
				break;
			}
			case WindEffect::SNOW:
			{
				divisor = 300000;
				break;
			}
			default: // Should never reach.
			{
				divisor = 0;
				break;
			}
		}

		const float scalar = std::pow(0.5f, (interval_t)delta_t
			/ divisor);
		velocity = velocity * scalar + cur_wind * (1.0 - scalar);

		if (pos.y < min_height)
		{
			velocity /= ((min_height - pos.y + 1.0) * 8);
			pos.y = min_height;
			if (!(rand() % 3))
				velocity.y = -velocity.y * 1.5;
		}
		else
		{
			pos.y -= 0.4 * usec;
			if (pos.y < min_height)
				pos.y = min_height;
		}

		//  std::cout << "Pos: " << pos << std::endl;
		//  std::cout << "Velocity: " << velocity.magnitude() << ", " << velocity << std::endl;

		if (!state)
		{
			if (age > 50000000)
			{
				//      std::cout << this << ": Too old." << std::endl;
				state = 1;
			}

			if ((pos - base->center).planar_magnitude_squared()
				> MAX_DRAW_DISTANCE_SQUARED * 2)
			{
				//      std::cout << this << ": " << pos << " too far from " << base->center << std::endl;
				state = 1;
			}
		}

		if (state == 0)
		{
			if (pos.y > max_height)
				velocity.y -= delta_t / 2000000.0;

			Vec3 shifted_pos = pos - wind_effect->center;
			const coord_t radius= std::sqrt(square(shifted_pos.x) + square(shifted_pos.z));
			const angle_t angle = atan2(shifted_pos.x, shifted_pos.z);
			const coord_t max_radius =
				wind_effect->bounding_range->get_radius(angle);
			if (radius > max_radius)
			{ // Pass it off to a neighboring effect.
				if (wind_effect->neighbors.size() == 0)
				{
					state = 1;
					return true;
				}
				std::vector<WindEffect::WindNeighbor>::iterator iter;
				for (iter = wind_effect->neighbors.begin(); iter != wind_effect->neighbors.end(); ++iter)
				{
					if ((angle > iter->start_angle) && (angle <= iter->end_angle))
						break;
				}
				if (iter == wind_effect->neighbors.end())
					iter = wind_effect->neighbors.begin();
				if (iter->neighbor == NULL)
				{
					state = 1;
					return true;
				}
				effect->particles.erase(this);
				effect = iter->neighbor;
				effect->particles[this] = true;
			}
		}
		else
		{
			pos.y += delta_t / 800000.0;
			if (pos.y > max_height * 2)
			{
				//      std::cout << this << ": " << pos << " too high." << std::endl;
				return false;
			}
		}

		// Rotate the blowing object.
		angle_t rot_scalar;
		switch (type)
		{
			case WindEffect::LEAVES:
			{
				rot_scalar = 2.0;
				break;
			}
			case WindEffect::FLOWER_PETALS:
			{
				rot_scalar = 3.5;
				break;
			}
			case WindEffect::SNOW:
			{
				rot_scalar = 30.0;
				break;
			}
			default: // Should never reach.
			{
				rot_scalar = 0;
				break;
			}
		}
		if (pos.y >= min_height + 0.05)
		{
			for (int i = 0; i < 3; i++)
			{
				const angle_t rot = rot_scalar * randangle(10.0) * usec * axis_weights[i]
					* (rotation_axes[i].dot(cur_wind)) * (3.0 + velocity.y)
					+ randangle(0.6) * usec;
				Quaternion cur_rotation;
				cur_rotation.from_axis_and_angle(rotation_axes[i], rot);
				quaternion *= cur_rotation;
			}
		}

		return true;
	}

	Uint32 WindParticle::get_texture() // Shouldn't be needed.  But just in case...
	{
		switch (type)
		{
			case WindEffect::LEAVES:
				switch (subtype)
				{
					case 0: // Maple
						return base->get_texture(EC_LEAF_MAPLE);
					case 1: // Oak
						return base->get_texture(EC_LEAF_OAK);
					case 2: // Ash
						return base->get_texture(EC_LEAF_ASH);
					default: // Should never reach.
						return 0;
				}
			case WindEffect::FLOWER_PETALS:
				return base->get_texture(EC_PETAL);
			case WindEffect::SNOW:
				return base->get_texture(EC_SNOWFLAKE);
			default: // Should never reach.
				return 0;
		}

		return 0; // Control should never reach here.
	}

	float WindParticle::get_burn() const
	{
		return 0.0f;
	}

	Vec3 WindParticle::get_wind_vec() const
	{
		const WindEffect* e = (WindEffect*)effect;
		const float time_offset = (float)((unsigned short)(get_time() / 10000))
			* PI / 2000.0; // Translation: Convert to milliseconds, truncate the higher-order digits, convert to a float, make it wraparound in radians, and scale it down some.
		const unsigned short individual_offset =
			(unsigned short)(uintptr_t)(void*)(this); // Based on the memory address in order to give each particle a unique bias.
		srand(individual_offset);
		const float offset= randfloat() * 0.5;

		const coord_t x = 1.0 * sin(offset + pos.x * 0.5283 + pos.z * 0.7111
			+ time_offset * 0.6817) * sin(offset + pos.x * 1.2019 + pos.z
			* 0.5985 + time_offset * 1.5927) * e->max_adjust / (fabs(pos.y
			- e->center.y) + 1);
		const coord_t y = 1.0 * sin(offset + pos.x * 0.4177 + pos.z * 1.3127
			+ time_offset * 1.1817) * sin(offset + pos.x * 0.5828 + pos.z
			* 0.6888 + time_offset * 2.1927) * e->max_adjust * 2.0;
		const coord_t z = 1.0 * sin(offset + pos.x * 1.1944 + pos.z * 0.9960
			+ time_offset * 1.6817) * sin(offset + pos.x * 0.6015 + pos.z
			* 1.4809 + time_offset * 1.4927) * e->max_adjust / (fabs(pos.y
			- e->center.y) + 1);

		//  Vec3 random_component;
		//  random_component.randomize(max_adjust / 4);
		//  std::cout << this << ",\t" << pos << ",\t" << velocity << ":\t" << Vec3(x, y, z) << ",\t" << (e->overall_wind + Vec3(x, y, z)) << std::endl;
		//  std::cout << "  **\t" << offset << ",\t" << (pos.x * 1.9283) << ",\t" << (pos.z * 2.4111) << ",\t" << (time_offset * 2.2817) << " =>\t" << sin(offset + pos.x * 1.9283 + pos.z * 2.4111 + time_offset * 2.2817) << std::endl;
		//  std::cout << "  **\t" << offset << ",\t" << (pos.x * 3.4019) << ",\t" << (pos.z * 2.0985) << ",\t" << (time_offset * 4.1927) << " =>\t" << sin(offset + pos.x * 3.4019 + pos.z * 2.0985 + time_offset * 4.1927) << std::endl;
		return e->overall_wind + Vec3(x, y, z);// + random_component;
	}

	WindEffect::WindEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
		std::vector<ec::Obstruction*>* _obstructions, const color_t _hue_adjust,
		const color_t _saturation_adjust, const coord_t _scalar,
		const float _density, BoundingRange* _bounding_range,
		const WindType _type, const Vec3 _prevailing_wind)
	{
		if (EC_DEBUG)
			std::cout << "WindEffect (" << this << ") created." << std::endl;
		base = _base;
		dead = _dead;
		pos = _pos;
		center = *pos;
		obstructions = _obstructions;
		type = _type;
		hue_adjust = _hue_adjust;
		saturation_adjust = _saturation_adjust;
		scalar = _scalar;
		prevailing_wind = _prevailing_wind;
		overall_wind = prevailing_wind;
		max_adjust = 0.2 + prevailing_wind.magnitude() * 0.5;
		overall_wind_adjust = Vec3(0.0, 0.0, 0.0);
		bounding_range = _bounding_range;
		bounds = bounding_range;
		mover = new GradientMover(this);
		spawner = new FilledBoundingSpawner(_bounding_range, pos, &(base->center), range_scalar);
		//  max_LOD1_count = (int)(spawner->get_area() * _density * 1.0) / 10;
		//  std::cout << "2: " <<  hue_adjust << " / " << saturation_adjust << " / " << scalar << " / " << _density << std::endl;
		max_LOD1_count = (int)(MAX_DRAW_DISTANCE_SQUARED * PI * _density
			* range_scalar * 2.0) / 25;
		LOD = base->last_forced_LOD;
		count = LOD * max_LOD1_count;
		switch (type)
		{
			case LEAVES:
			{
				count *= 1;
				break;
			}
			case FLOWER_PETALS:
			{
				count *= 3;
				break;
			}
			case SNOW:
			{
				count *= 8;
				break;
			}
		}
	}

	WindEffect::~WindEffect()
	{
		delete mover;
		delete spawner;
		if (EC_DEBUG)
			std::cout << "WindEffect (" << this << ") destroyed." << std::endl;
	}

	void WindEffect::set_pass_off(std::vector<Effect*> pass_off_to)
	{
		std::vector<WindEffect*> new_vec;
		for (auto effect: pass_off_to)
			new_vec.push_back((WindEffect*)effect);
		set_pass_off(new_vec);
	}

	void WindEffect::set_pass_off(std::vector<WindEffect*> pass_off_to)
	{
		// Get the max extents for each neighbor.
		for (auto eff: pass_off_to)
		{
			WindNeighbor n;
			n.neighbor = eff;
			const coord_t dist = (center - eff->center).magnitude();
			const angle_t angle = center.angle_to(eff->center);
			const angle_t opposite_angle = remainderf(angle + PI, 2 * PI);
			const coord_t radius1 = bounding_range->get_radius(angle);
			const coord_t radius2 =
				eff->bounding_range->get_radius(opposite_angle);
			if (dist > radius1 + radius2)
				continue;

			angle_t angle_shift = 0.01;
			angle_t start_angle = angle - angle_shift;
			for (;; angle_shift += 0.01, start_angle -= 0.01)
			{
				if (start_angle < 0)
					start_angle += 2 * PI;
				const percent_t distance_penalty = cos(angle_shift);
				const coord_t radius1 = bounding_range->get_radius(angle);
				const coord_t radius2 =
					eff->bounding_range->get_radius(opposite_angle);
				const coord_t newdist = (radius1 + radius2) / distance_penalty;
				if (dist < newdist)
					break;
			}
			n.start_angle = start_angle;
			if (n.start_angle < 0)
				n.start_angle += 2 * PI;

			angle_shift = 0.01;
			float end_angle = angle - angle_shift;
			for (;; angle_shift += 0.01, end_angle += 0.01)
			{
				if (end_angle >= 2 * PI)
					end_angle -= 2 * PI;
				const percent_t distance_penalty = cos(angle_shift);
				const coord_t radius1 = bounding_range->get_radius(angle);
				const coord_t radius2 =
					eff->bounding_range->get_radius(opposite_angle);
				const coord_t newdist = (radius1 + radius2) / distance_penalty;
				if (dist < newdist)
					break;
			}
			n.end_angle = end_angle;
			if (n.end_angle >= 2 * PI)
				n.end_angle -= 2 * PI;

			neighbors.push_back(n);
		}

		// Eliminate overlap and insert nulls between extents that don't reach each other.
		if (neighbors.size())
		{
			WindNeighbor* previous = &(*(neighbors.begin() + (neighbors.size()
				- 1)));
			for (int i = 0; i < (int)neighbors.size(); i++)
			{
				std::vector<WindNeighbor>::iterator iter = neighbors.begin() + i;
				WindNeighbor* current = &(*iter);
				angle_t end_angle = previous->end_angle;
				if (end_angle - current->start_angle > PI)
					end_angle -= 2 * PI;
				if (end_angle > current->start_angle)
				{ // Narrow down the angles.
					angle_t average = (end_angle + current->start_angle) / 2;
					if (average < 0)
						average += 2 * PI;
					previous->end_angle = average;
					current->start_angle = average;
				}
				else
				{ // Insert a null
					WindNeighbor n;
					n.neighbor = NULL;
					n.start_angle = previous->end_angle;
					n.end_angle = current->start_angle;
					neighbors.insert(iter, n);
					i++;
				}
				previous = current;
			}
		}

		// Spawn leaves
		for (int i = count - (int)particles.size(); i >= 0; i--)
		{
			Vec3 coords = spawner->get_new_coords();
			if (coords.x == -32768.0)
				continue;
			coords += base->center + Vec3(0.0, 0.05, 0.0);
			Vec3 velocity;
			velocity.randomize(max_adjust / 4);
			velocity.y /= 2;
			Particle
				* p =
					new WindParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, scalar, center.y + 0.05, center.y + 1.0, type);
			if (!base->push_back_particle(p))
				break;
		}
	}

	bool WindEffect::idle(const Uint64 usec)
	{
		if ((recall) && (particles.size() == 0))
			return false;

		if (recall)
			return true;

		center = *pos;

		Vec3 velocity_shift;
		velocity_shift.randomize();
		velocity_shift.y /= 2;
		velocity_shift.normalize(0.00004 * 4 * std::sqrt(usec));
		overall_wind_adjust += velocity_shift;
		const coord_t magnitude = overall_wind_adjust.magnitude();
		if (magnitude > max_adjust * 3.5)
		{
			//    std::cout << "Adjusting down." << std::endl;
			overall_wind_adjust /= (magnitude / (max_adjust * 3.5));
		}
		//  std::cout << "Wind adjust: " << overall_wind_adjust.magnitude() << ", " << overall_wind_adjust << std::endl;

		if (fabs(overall_wind_adjust.y) > 0.15)
			overall_wind_adjust.y *= std::pow(0.5f, usec / 300000.0f);

		overall_wind = prevailing_wind + overall_wind_adjust;

		count = LOD * max_LOD1_count;
		switch (type)
		{
			case LEAVES:
			{
				count *= 1;
				break;
			}
			case FLOWER_PETALS:
			{
				count *= 3;
				break;
			}
			case SNOW:
			{
				count *= 8;
				break;
			}
		}

		for (int i = count - (int)particles.size(); i >= 0; i--)
		{
			Vec3 coords = spawner->get_new_coords();
			if (coords.x == -32768.0)
				continue;
			//    std::cout << coords << ", ";
			coords += base->center + Vec3(0.0, 0.05, 0.0);
			//    std::cout << coords << std::endl;
			Vec3 velocity;
			velocity.randomize(max_adjust / 2);
			velocity.y /= 2;
			Particle
				* p =
					new WindParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, scalar, center.y + 0.05, center.y + 4.0, type);
			if (!base->push_back_particle(p))
			{
				break;
			}
		}

		return true;
	}

///////////////////////////////////////////////////////////////////////////////

}
;

