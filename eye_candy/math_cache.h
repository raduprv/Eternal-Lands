/*!
 \brief A class for speeding up various mathematics options.

 Why not use built-in, standard math operations?  Quite simply, particle
 systems are CPU intensive, but not that picky about accuracy, while the
 standard calls are accurate but slow.  

 Optimizing this class, in addition to optimizing the Vec3 class, is a nice
 way to get a global performance increase across EyeCandy.  A good target
 would be the powf functions: widely used and still not that fast.  One
 possibility would be, instead of caching powf itself, cache exp and log
 (both of which should cache much more easily, in a smaller cache that could
 readily fit into the CPU's cache) and then use them to recreate powf.
 */

#ifndef MATH_CACHE_H
#define MATH_CACHE_H

#ifdef _MSC_VER
#pragma warning (disable : 4100) // Unreferenced formal parameter
#pragma warning (disable : 4127) // Conditional expression is constant
#pragma warning (disable : 4244) // Conversion from type1 to type2
#pragma warning (disable : 4305) // Truncation from type1 to type2
#endif

// I N C L U D E S ////////////////////////////////////////////////////////////

#ifdef __SSE__
#include <xmmintrin.h>
#endif

#include <SDL.h>
#include <algorithm>

#include "types.h"

namespace ec
{

	// C L A S S E S //////////////////////////////////////////////////////////////

	/*
	 Hirange Performance data:

	 powf_05_close takes about 42% the cpu of powf (recommended).
	 powf_05_rough takes about 40% of the cpu of powf.
	 bm_0_1_close_close takes about 90% of the cpu of powf.
	 bm_0_1_rough_rough takes about 70% of the cpu of powf
	 bm_0_1_rough_close takes about 75% of the cpu of powf, and is almost as accurate as bm_0_1_close_close
	 bm_0_1_close_rough takes about 87% of the cpu of powf.

	 bm_05(): 7132
	 bm_05_rough(): 2868
	 bm_05_close(): 3003
	 bm_0_1(): 157381
	 bm_0_1_rough_rough(): 115168
	 bm_0_1_rough_close(): 123375
	 bm_0_1_close_rough(): 141230
	 bm_0_1_close_close(): 146872

	 sin, cos, and sqrt all provide minimal (<5%) performance increases in "rough"
	 mode, and are notably slower in close mode.  The only upside is, while the
	 sqrt function has only slightly better accuracy than powf function (i.e., not
	 great, but good enough), the sin functions are quite precise.

	 Still, with minimal performance gain, I think that implementing them was a
	 waste.
	 */
	/*
		Removed most MathCache funktions, because they are no speed gain
		and make life for the optimizer harder ;)

		15.10.2012 Xaphier
	*/
	/*
	 class MathCache_Hirange	// Currently unused. 
	 {
	 public:
	 MathCache_Hirange();
	 ~MathCache_Hirange() {};
	 
	 float powf_05_rough(const float power) const;
	 float powf_05_close(const float power) const;
	 float powf_0_1_rough_rough(const float base, const float power) const;
	 float powf_0_1_rough_close(const float base, const float power) const;
	 float powf_0_1_close_rough(const float base, const float power) const;
	 float powf_0_1_close_close(const float base, const float power) const;
	 float sqrt_rough(const float val) const;
	 float sqrt_close(const float val) const;
	 float sin_0_2PI_rough(const float angle) const;
	 float sin_0_2PI_close(const float angle) const;
	 float sin_nPI_PI_rough(const float angle) const;
	 float sin_nPI_PI_close(const float angle) const;
	 float sin_rough(const float angle) const;
	 float sin_close(const float angle) const;
	 float cos_0_2PI_rough(const float angle) const;
	 float cos_0_2PI_close(const float angle) const;
	 float cos_nPI_PI_rough(const float angle) const;
	 float cos_nPI_PI_close(const float angle) const;
	 float cos_rough(const float angle) const;
	 float cos_close(const float angle) const;
	 
	 protected:
	 static int get_lower_index(const float power);
	 static void get_lower_index_and_percent(const float power, int& index, float& percent);

	 float powf_map[10001][129];
	 float sqrt_map[129];
	 float sin_map[10002];
	 float sin_map2[10002];
	 float cos_map[10002];
	 float cos_map2[10002];
	 };
	 */
	class MathCache
	{
		public:

			static int randint(const int upto)
			{
				return rand() % upto;
			}
			;

			static double randdouble()
			{
				return (double)rand() / (double)RAND_MAX;
			}
			;

			static float randfloat()
			{
				return (float)rand() / (float)RAND_MAX;
			}
			;

			static double randdouble(const double scale)
			{
				return scale * randdouble();
			}
			;

			static float randfloat(const float scale)
			{
				return scale * randfloat();
			}
			;

			static float pow_randfloat(const float exponent)
			{
				return std::max(0.0001f, std::pow(randfloat(), exponent));
			}

			static coord_t randcoord_non_zero(void)
			{
				return (coord_t)randfloat() * 0.9999f + 0.0001f;
			}
			;

			static coord_t randcoord(void)
			{
				return (coord_t)randfloat();
			}
			;

			static coord_t randcoord(const coord_t scale)
			{
				return scale * randcoord();
			}
			;

			static color_t randcolor(void)
			{
				return (color_t)randfloat();
			}
			;

			static color_t randcolor(const color_t scale)
			{
				return scale * randcolor();
			}
			;

			static alpha_t randalpha(void)
			{
				return (alpha_t)randfloat();
			}
			;

			static alpha_t randalpha(const alpha_t scale)
			{
				return scale * randalpha();
			}
			;

			static energy_t randenergy(void)
			{
				return (energy_t)randfloat();
			}
			;

			static energy_t randenergy(const energy_t scale)
			{
				return scale * randenergy();
			}
			;

			static light_t randlight(void)
			{
				return (light_t)randfloat();
			}
			;

			static light_t randlight(const light_t scale)
			{
				return scale * randlight();
			}
			;

			static percent_t randpercent(void)
			{
				return (percent_t)randfloat();
			}
			;

			static percent_t randpercent(const percent_t scale)
			{
				return scale * randpercent();
			}
			;

			static angle_t randangle(void)
			{
				return (angle_t)randfloat();
			}
			;

			static angle_t randangle(const angle_t scale)
			{
				return scale * randangle();
			}
			;

			static double square(const double d)
			{
				return d * d;
			}
			;

			static float square(const float f)
			{
				return f * f;
			}
			;

			static int square(const int i)
			{
				return i * i;
			}
			;

	};

////////////////////////////////////////////////////////////////////////////////

}
;

#endif

