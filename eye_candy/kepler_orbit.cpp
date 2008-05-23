#include "kepler_orbit.h"

namespace ec
{
	KeplerOrbit::KeplerOrbit()
	{
	}

	float KeplerOrbit::computeEccentricAnomaly(float time, float eccentricity,
		float period) const
	{

		float E;
		float M = getMeanAnomaly(time, period);
		float e = eccentricity;

		E = M + sin(M)*(e-(1/8)*e*e*e) + sin(2.0*M)*(0.5f*e*e) + sin(3.0*M)*((3
			/8)*e*e*e);
		return E;
	}

	float KeplerOrbit::computeTrueAnomaly(float time, float eccentricity,
		float period) const
	{
		float E = computeEccentricAnomaly(time, eccentricity, period);

		float T = acos( (eccentricity+cos(E))/ (1+eccentricity*cos(E)));

		int q = (int)(E/(PI));
		if ((q%2)==0)
			return T;
		return T*-1.0;
	}

	float KeplerOrbit::computeDistance(float anomaly, float semimajor_axis,
		float eccentricity) const
	{
		return semimajor_axis*( (1-eccentricity*eccentricity)/(1+eccentricity
			*cos(anomaly)));
	}

	float KeplerOrbit::getMeanAnomaly(float t, float period) const
	{
		return 2*PI*t/period;
	}

	std::pair<float,float> KeplerOrbit::getCoordsAtTime(float time,
		float semimajor_axis, float eccentricity, float period) const
	{
		float A, d;
		if (eccentricity != 0.0f)
		{
			A = computeTrueAnomaly(time, eccentricity, period);
			d = computeDistance(A, semimajor_axis, eccentricity);
		}
		else
		{
			d = semimajor_axis;
			A = getMeanAnomaly(time, period);
		}
		return std::pair<float,float>( d*cos(A), d*sin(A) );
	}
}

// EOF
