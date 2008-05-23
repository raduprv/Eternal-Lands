#include "orbital_mover.h"

namespace ec
{
	OrbitalMover::OrbitalMover(Effect* _effect, const Vec3 &center) :
		ParticleMover(_effect)
	{
		m_vec3Center = center;
	}
	OrbitalMover::~OrbitalMover()
	{
	}

	void OrbitalMover::move(Particle& p, Uint64 usec)
	{
		float elapsed = get_time() - p.born;

		p_map_t::iterator i = m_hParticleData.find(&p);
		if (i == m_hParticleData.end() )
			return;

		OrbitalParticleData &pdata = i->second;

		std::pair<float,float> ps = orbit.getCoordsAtTime(elapsed / 100000.0
			+ pdata.start_offset, pdata.semimajor_axis, pdata.eccentricity,
			pdata.period) ;

		p.pos.x = ps.first/30.0 + m_vec3Center.x;
		p.pos.z = ps.second/30.0 + m_vec3Center.z;
	}

	void OrbitalMover::attachParticle(Particle *p)
	{
	}
	void OrbitalMover::detachParticle(Particle *p)
	{
		p_map_t::iterator i = m_hParticleData.find(p);
		if (i != m_hParticleData.end() )
			m_hParticleData.erase(i);
		//m_hParticleData[p] = OrbitalParticleData();

	}
	void OrbitalMover::setParticleData(Particle *p,
		const OrbitalParticleData &data)
	{
		m_hParticleData[p] = data;
	}
}

// EOF
