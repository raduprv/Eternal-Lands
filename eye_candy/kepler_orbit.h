#include "eye_candy.h"

namespace ec
{
	/*
	 For more information, please see
	 http://en.wikipedia.org/wiki/Kepler's_laws_of_planetary_motion#Position_as_a_function_of_time
	 */

	class KeplerOrbit
	{
		public:
			KeplerOrbit();
			float computeEccentricAnomaly(float time, float eccentricity,
				float period) const;
			float computeTrueAnomaly(float time, float eccentricity,
				float period) const;
			float computeDistance(float anomaly, float semimajor_axis,
				float eccentricity) const;
			float getMeanAnomaly(float t, float period) const;
			std::pair<float,float> getCoordsAtTime(float time,
				float semimajor_axis, float eccentricity, float period) const;
	};
}
