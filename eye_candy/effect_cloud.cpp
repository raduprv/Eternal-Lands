// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_cloud.h"

namespace ec
{

	// C L A S S   F U N C T I O N S //////////////////////////////////////////////

	CloudParticle::CloudParticle(Effect* _effect, ParticleMover* _mover,
		const Vec3 _pos, const Vec3 _velocity, const color_t hue_adjust,
		const color_t saturation_adjust, const coord_t _min_height,
		const coord_t _max_height, const coord_t _size, const alpha_t _alpha) :
		Particle(_effect, _mover, _pos, _velocity, _size)
	{
		color_t hue, saturation, value;
		hue = 0.67;
		saturation = 0.05;
		value = 1.0;
		hue += hue_adjust;
		if (hue > 1.0)
			hue -= 1.0;
		saturation = std::min(1.0f, saturation * saturation_adjust);
		hsv_to_rgb(hue, saturation, value, color[0], color[1], color[2]);
		alpha = _alpha;
		flare_max = 2.0;
		flare_exp = 0.7;
		flare_frequency = 30.0;
		min_height = _min_height;
		max_height = _max_height;
		normal = Vec3(0.0, 1.0, 0.0);
	}

	bool CloudParticle::idle(const Uint64 delta_t)
	{
		if (effect->recall)
			return false;

		if (base->particles.size() > 1)
		{
			if ((pos - base->center).magnitude_squared()
				> MAX_DRAW_DISTANCE_SQUARED)
			{
				//Find everything that sees this as a neighbor and delete the references to this.
				for (auto incoming: incoming_neighbors)
					incoming->remove_neighbor(this);
				for (auto neighbor: neighbors)
					neighbor->remove_incoming_neighbor(this);
				return false;
			}
		}

		Vec3 velocity_shift;
		velocity_shift.randomize();
		velocity_shift.y /= 3;
		velocity_shift.normalize(0.00002 * std::sqrt(delta_t));
		velocity += velocity_shift;
		const coord_t magnitude = velocity.magnitude();
		if (magnitude > 0.15)
			velocity /= (magnitude / 0.15);

		if (fabs(velocity.y) > 0.1)
			velocity.y *= std::pow(0.5f, delta_t / 300000.0f);

		if (pos.y - size / 40 < min_height)
			velocity.y += delta_t / 500000.0;

		if (pos.y + size / 40 > max_height)
			velocity.y -= delta_t / 2500000.0;

		if (effect->particles.size() <= 1)
			return true;

		if (!neighbors.size())
		{
			std::map<Particle*, bool>::iterator next_iter;
			int offset;
			CloudParticle* next;
			while (true)
			{
				next_iter = effect->particles.begin();
				offset = randint((int)effect->particles.size());
				for (int j = 0; j < offset; j++)
					++next_iter;
				next = (CloudParticle*)next_iter->first;
				if (next != this)
					break;
			}
			neighbors.push_back(next);
			next->add_incoming_neighbor(this);
		}

		// Adjust our neighbors -- try a few points to see if they're closer.
		// First, create the map

		CloudEffect* eff = (CloudEffect*)effect;
		std::map<coord_t, CloudParticle*> neighbors_map;
		for (int i = 0; i < (int)neighbors.size(); i++)
		{
			const coord_t distsquared = (neighbors[i]->pos - pos).magnitude_squared();
			neighbors_map[distsquared] = neighbors[i];
		}

		// Now, try to replace elements.
		coord_t maxdist = neighbors_map.rbegin()->first;
		for (int i = 0; (i < 1) || ((neighbors_map.size() < 20) && (i < 40)); i++)
		{
			std::map<Particle*, bool>::iterator iter;
			int offset;
			CloudParticle* neighbor;
			while (true)
			{
				iter = eff->particles.begin();
				offset = randint((int)eff->particles.size());
				for (int j = 0; j < offset; j++)
					++iter;
				neighbor = (CloudParticle*)iter->first;
				if (neighbor != this)
					break;
			}
			const coord_t distsquared = (neighbor->pos - pos).magnitude_squared();
			if (neighbors_map.size() >= 20)
			{
				if (distsquared > maxdist)
					continue;
				if (neighbors_map.count(distsquared))
					continue;
			}
			if (neighbors_map.size() >= 20)
			{
				std::map<coord_t, CloudParticle*>::iterator iter =
					neighbors_map.begin();
				for (int j = 0; j < (int)neighbors_map.size() - 1; j++)
					++iter;
				neighbors_map.erase(iter);
			}
			neighbors_map[distsquared] = neighbor;
			maxdist = neighbors_map.rbegin()->first;
		}

		// Set our color based on how deep into the cloud we are, based on our neighbors.  Also rebuild the neighbors vector.
		coord_t distsquaredsum = 0;
		Vec3 centerpoint(0.0, 0.0, 0.0);
		for (auto neighbor: neighbors)
			neighbor->remove_incoming_neighbor(this);
		neighbors.clear();

		for (const auto& iter: neighbors_map)
		{
			distsquaredsum += iter.first;
			centerpoint += iter.second->pos; //Should really be (pos - iter->second->pos); will correct this below for speed.
			neighbors.push_back(iter.second);
			iter.second->add_incoming_neighbor(this);
		}
		centerpoint = (pos * 20) - centerpoint;
		Vec3 new_normal = centerpoint;
		const coord_t magnitude_squared = centerpoint.magnitude_squared();
		const coord_t scale= std::sqrt(magnitude_squared);
		new_normal /= scale; // Normalize
		//  light_t new_brightness = 1.0 - (25.0 / (scale + 50.0));
		//  new_normal.x = (new_normal.x < 0 ? -1 : 1) * math_cache.powf_0_1_rough_close(fabs(new_normal.x), new_brightness * 2.0 - 1.0);
		//  new_normal.y = (new_normal.y < 0 ? -1 : 1) * math_cache.powf_0_1_rough_close(fabs(new_normal.y), new_brightness * 2.0 - 1.0);
		//  new_normal.z = (new_normal.z < 0 ? -1 : 1) * math_cache.powf_0_1_rough_close(fabs(new_normal.z), new_brightness * 2.0 - 1.0);
		const percent_t change_rate = std::pow(0.5f, delta_t
			/ 2000000.0f);
		normal = normal * change_rate + new_normal * (1.0 - change_rate);
		normal.normalize();
		//  color[0] = color[0] * change_rate + new_brightness * (1.0 - change_rate);
		//  color[1] = color[0];
		//  color[2] = color[0];
		//  std::cout << "  " << centerpoint << std::endl;
		//  std::cout << "  " << normal << std::endl;
		//  std::cout << "  " << brightness << std::endl;

		return true;
	}

	Uint32 CloudParticle::get_texture()
	{
		return base->get_texture(EC_SIMPLE);
	}

	float CloudParticle::get_burn() const
	{
		return 0.0f;
	}

	void CloudParticle::remove_neighbor(const CloudParticle*const p)
	{
		for (int i = 0; i < (int)neighbors.size();)
		{
			std::vector<CloudParticle*>::iterator iter = neighbors.begin() + i;
			if (*iter == p)
			{
				neighbors.erase(iter);
				continue;
			}
			i++;
		}

		if (effect->particles.size() <= 2)
			return;

		if (!neighbors.size())
		{
			std::map<Particle*, bool>::iterator next_iter;
			int offset;
			CloudParticle* next;
			while (true)
			{
				next_iter = effect->particles.begin();
				offset = randint((int)effect->particles.size());
				for (int j = 0; j < offset; j++)
					++next_iter;
				next = (CloudParticle*)next_iter->first;
				if ((next != this) && (next != p))
					break;
			}
			neighbors.push_back(next);
			next->add_incoming_neighbor(this);
		}
	}

	void CloudParticle::add_incoming_neighbor(CloudParticle*const p)
	{
		incoming_neighbors.push_back(p);
	}

	void CloudParticle::remove_incoming_neighbor(const CloudParticle*const p)
	{
		for (int i = 0; i < (int)incoming_neighbors.size();)
		{
			std::vector<CloudParticle*>::iterator iter =
				incoming_neighbors.begin() + i;
			if (*iter == p)
			{
				incoming_neighbors.erase(iter);
				continue;
			}
			i++;
		}
	}

	CloudEffect::CloudEffect(EyeCandy* _base, bool* _dead, Vec3* _pos,
		const color_t _hue_adjust, const color_t _saturation_adjust,
		const float _density, BoundingRange* bounding_range, const Uint16 _LOD)
	{
		if (EC_DEBUG)
			std::cout << "CloudEffect (" << this << ") created (" << *_pos
				<< ", " << bounding_range->get_radius(0.0) << ")." << std::endl;
		base = _base;
		dead = _dead;
		pos = _pos;
		center = *pos;
		hue_adjust = _hue_adjust;
		saturation_adjust = _saturation_adjust;
		LOD = base->last_forced_LOD;
		desired_LOD = _LOD;
		bounds = bounding_range;
		mover = new BoundingMover(this, center, bounding_range, 1.0);
		spawner = new NoncheckingFilledBoundingSpawner(bounding_range);
		//  count = (int)(spawner->get_area() * 0.03 * (LOD  + 1));
		count = (int)(MAX_DRAW_DISTANCE_SQUARED * PI * 0.03 * (LOD + 1));
		if (count < 21)
			count = 21;

		alpha = 0.1725 / (1.0 / _density + 0.15);
		size_scalar = 110.0 / std::sqrt(LOD + 1);

		for (int i = 0; i < count; i++)
		{
			Vec3 coords = spawner->get_new_coords();
			if (coords.x == -32768.0)
				break;
			coords += center + Vec3(0.0, randcoord(5.0), 0.0);
			Vec3 velocity;
			velocity.randomize(0.15);
			velocity.y /= 3;
			const coord_t size = size_scalar + randcoord(size_scalar);
			Particle
				* p =
					new CloudParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, center.y, center.y + 20.0, size, alpha);
			if (!base->push_back_particle(p))
				break;
		}

		// Load one neighbor for each one.  It'll get more on its own.
		for (std::map<Particle*, bool>::iterator iter = particles.begin(); iter != particles.end(); ++iter)
		{
			CloudParticle* p = (CloudParticle*)iter->first;
			CloudParticle* next;
			std::map<Particle*, bool>::iterator iter2 = iter;
			++iter2;
			if (iter2 != particles.end())
				next = (CloudParticle*)iter2->first;
			else
				next = (CloudParticle*)particles.begin()->first;
			p->neighbors.push_back(next);
			next->add_incoming_neighbor(p);
		}
	}

	CloudEffect::~CloudEffect()
	{
		delete mover;
		delete spawner;
		if (EC_DEBUG)
			std::cout << "CloudEffect (" << this << ") destroyed." << std::endl;
	}

	bool CloudEffect::idle(const Uint64 usec)
	{
		if (recall && (particles.size() == 0))
			return false;

		if (recall)
			return true;

		const int start_count = particles.size();
		if (start_count == count)
			return true;

		if (particles.size())
		{
			CloudParticle* last = (CloudParticle*)(particles.rbegin()->first);
			for (int i = count - (int)particles.size(); i >= 0; i--)
			{
				Vec3 coords = spawner->get_new_coords();
				if (coords.x == -32768.0)
					continue;
				coords += center + Vec3(0.0, randcoord(5.0), 0.0);
				Vec3 velocity;
				velocity.randomize(0.15);
				velocity.y /= 3;
				const coord_t size = size_scalar + randcoord(size_scalar);
				CloudParticle
					* p =
						new CloudParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, center.y, center.y + 20.0, size, alpha);
				if (!base->push_back_particle(p))
					break;
				p->neighbors.push_back(last);
				last->add_incoming_neighbor(p);
			}
		}
		else
		{
			for (int i = count - (int)particles.size(); i >= 0; i--)
			{
				Vec3 coords = spawner->get_new_coords();
				if (coords.x == -32768.0)
					continue;
				coords += center + Vec3(0.0, randcoord(5.0), 0.0);
				Vec3 velocity;
				velocity.randomize(0.15);
				velocity.y /= 3;
				const coord_t size = size_scalar + randcoord(size_scalar);
				Particle
					* p =
						new CloudParticle(this, mover, coords, velocity, hue_adjust, saturation_adjust, center.y, center.y + 20.0, size, alpha);
				if (!base->push_back_particle(p))
					break;
			}

			// Load one neighbor for each one.  It'll get more on its own.
			for (std::map<Particle*, bool>::iterator iter = particles.begin(); iter != particles.end(); ++iter)
			{
				CloudParticle* p = (CloudParticle*)iter->first;
				CloudParticle* next;
				std::map<Particle*, bool>::iterator iter2 = iter;
				++iter2;
				if (iter2 != particles.end())
					next = (CloudParticle*)iter2->first;
				else
					next = (CloudParticle*)particles.begin()->first;
				p->neighbors.push_back(next);
				next->add_incoming_neighbor(p);
			}
		}

		return true;
	}

///////////////////////////////////////////////////////////////////////////////

}
;

