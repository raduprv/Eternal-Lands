#include "eye_candy.h"
#include <map>

#include "kepler_orbit.h"

namespace ec
{

	struct OrbitalParticleData
	{
			OrbitalParticleData(float off, float p, float e, float d) :
				start_offset(off), period(p), eccentricity(e),
					semimajor_axis(d)
			{
			}
			OrbitalParticleData()
			{
			}
			float start_offset;
			float period;
			float eccentricity;
			float semimajor_axis;
	};

	class OrbitalMover : public ParticleMover
	{
		public:
			OrbitalMover(Effect* _effect, const Vec3 &center);
			virtual ~OrbitalMover();
			virtual void move(Particle& p, Uint64 usec);
			virtual void attachParticle(Particle *);
			virtual void detachParticle(Particle *);
			virtual void setParticleData(Particle *,
				const OrbitalParticleData &data);
		private:
			float m_fSpeedY;
			KeplerOrbit orbit;
			Vec3 m_vec3Center;
			// Private data for the particles
			typedef std::map< Particle *, OrbitalParticleData> p_map_t;
			p_map_t m_hParticleData;
	};
}

// EOF
