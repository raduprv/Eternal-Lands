#ifdef EYE_CANDY

#ifndef MATH_CACHE_H
#define MATH_CACHE_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#ifdef __SSE__
 #include <xmmintrin.h>
#endif

#include <SDL.h>

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
  MathCache();
  ~MathCache() {};
  
  float powf_05_rough(const float power) const;
  float powf_05_close(const float power) const;
  float powf_0_1_rough_rough(const float base, const float power) const;
  float powf_0_1_rough_close(const float base, const float power) const;
  float powf_0_1_close_rough(const float base, const float power) const;
  float powf_0_1_close_close(const float base, const float power) const;

  static float invsqrt(float f)
  {
#ifdef __SSE__
    union { __m128 m128; struct { float x, y, z, w; }; } f2;
    f2.m128 = _mm_rsqrt_ss(_mm_set_ss(f));
    return f2.x;
#else
    union { int i; float f; } tmp;
    float half = 0.5f * f;
    tmp.f = f;
    tmp.i = 0x5f3759df - (tmp.i >> 1);   
    f = tmp.f;
    f = f * (1.5f - half * f * f);       
    return f;    
#endif
  };

  static int randint(const int upto)
  {
    return rand() % upto;
  };
  
  static Uint8 rand8()
  {
    return (Uint8)rand();
  };
  
  static Uint16 rand16()
  {
#if RAND_MAX >= 0xFFFF
    return (Uint16)rand();
#elif RAND_MAX > 0xFF
    return (((Uint16)rand()) << 8) | (Uint16)rand();
#else
    return (((Uint16)rand8()) << 8) | (Uint16)rand8();
#endif
  };
  
  static Uint32 rand32()
  {
#if RAND_MAX >= 0xFFFFFFFF
    return (Uint32)rand();
#elif RAND_MAX > 0xFFFF
    return (((Uint32)rand()) << 16) | (Uint32)rand();
#else
    return (((Uint32)rand16()) << 16) | (Uint32)rand16();
#endif
  };
  
  static Uint64 rand64()
  {
#if RAND_MAX >= 0xFFFFFFFFFFFFFFFF
    return (Uint64)rand();
#elif RAND_MAX > 0xFFFFFFFF
    return (((Uint64)rand()) << 32) | (Uint64)rand();
#else
    return (((Uint64)rand16()) << 32) | (Uint64)rand16();
#endif
  };
  
  static Uint8 rand7()
  {
    return (Uint8)rand();
  };
  
  static Uint16 rand15()
  {
#if RAND_MAX >= 0x8FFF
    return (Uint16)rand();
#elif RAND_MAX > 0xFF
    return (((Uint16)rand()) << 8) | (Uint16)rand();
#else
    return (((Uint16)rand8()) << 8) | (Uint16)rand8();
#endif
  };
  
  static Uint32 rand31()
  {
#if RAND_MAX >= 0x8FFFFFFF
    return (Uint32)rand();
#elif RAND_MAX > 0xFFFF
    return (((Uint32)rand()) << 16) | (Uint32)rand();
#else
    return (((Uint32)rand16()) << 16) | (Uint32)rand16();
#endif
  };
  
  static Uint64 rand63()
  {
#if RAND_MAX >= 0x8FFFFFFFFFFFFFFF
    return (Uint64)rand();
#elif RAND_MAX > 0xFFFFFFFF
    return (((Uint64)rand()) << 32) | (Uint64)rand();
#else
    return (((Uint64)rand16()) << 32) | (Uint64)rand16();
#endif
  };
  
  static double randdouble()
  {
    return (double)rand() / (double)RAND_MAX;
  };
  
  static float randfloat()
  {
    return (float)rand() / (float)RAND_MAX;
  };
  
  static double randdouble(const double scale)
  {
    return scale * randdouble();
  };
  
  static float randfloat(const float scale)
  {
    return scale * randfloat(); 
  };
  
  static coord_t randcoord(void)
  {
    if (sizeof(coord_t) == 4)	// Compiler should optimize this out.
      return (coord_t)randfloat();
    else
      return (coord_t)randdouble();
  };
  
  static coord_t randcoord(const coord_t scale)
  {
    return scale * randcoord();
  };
  
  static color_t randcolor(void)
  {
    if (sizeof(color_t) == 4)
      return (color_t)randfloat();
    else
      return (color_t)randdouble();
  };
  
  static color_t randcolor(const color_t scale)
  {
    return scale * randcolor();
  };
  
  static alpha_t randalpha(void)
  {
    if (sizeof(alpha_t) == 4)
      return (alpha_t)randfloat();
    else
      return (alpha_t)randdouble();
  };
  
  static alpha_t randalpha(const alpha_t scale)
  {
    return scale * randalpha();
  };
  
  static energy_t randenergy(void)
  {
    if (sizeof(energy_t) == 4)
      return (energy_t)randfloat();
    else
      return (energy_t)randdouble();
  };
  
  static energy_t randenergy(const energy_t scale)
  {
    return scale * randenergy();
  };
  
  static light_t randlight(void)
  {
    if (sizeof(light_t) == 4)
      return (light_t)randfloat();
    else
      return (light_t)randdouble();
  };
  
  static light_t randlight(const light_t scale)
  {
    return scale * randlight();
  };
  
  static percent_t randpercent(void)
  {
    if (sizeof(percent_t) == 4)
      return (percent_t)randfloat();
    else
      return (percent_t)randdouble();
  };
  
  static percent_t randpercent(const percent_t scale)
  {
    return scale * randpercent();
  };
  
  static angle_t randangle(void)
  {
    if (sizeof(angle_t) == 4)
      return (angle_t)randfloat();
    else
      return (angle_t)randdouble();
  };
  
  static angle_t randangle(const angle_t scale)
  {
    return scale * randangle();
  };
  
  static double square(const double d)
  {
    return d * d;
  };
  
  static float square(const float f)
  {
    return f * f;
  };
  
  static int square(const int i)
  {
    return i * i;
  };
  
  static double cube(const double d)
  {
    return d * d * d;
  };
  
  static float cube(const float f)
  {
    return f * f * f;
  };
  
  static int cube(const int i)
  {
    return i * i * i;
  };
  
  static float fastsqrt(float f)	// This could probably stand to be faster; use invsqrt wherever possible.
  {
    return f * invsqrt(f);
  };

protected:
  static int get_lower_index(const float power);
  static void get_lower_index_and_percent(const float power, int& index, float& percent);

  float powf_map[10001][65];

};

////////////////////////////////////////////////////////////////////////////////

};

#endif

#endif	// #ifdef EYE_CANDY
