#ifdef EYE_CANDY

#ifndef MATH_CACHE_H
#define MATH_CACHE_H

// I N C L U D E S ////////////////////////////////////////////////////////////

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

class MathCache_Hirange
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

class MathCache_Lorange		//Currently unused.
{
public:
  MathCache_Lorange();
  ~MathCache_Lorange() {};
  
  float powf_05_rough(const float power) const;
  float powf_05_close(const float power) const;
  float powf_0_1_rough_rough(const float base, const float power) const;
  float powf_0_1_rough_close(const float base, const float power) const;
  float powf_0_1_close_rough(const float base, const float power) const;
  float powf_0_1_close_close(const float base, const float power) const;

  
protected:
  static int get_lower_index(const float power);
  static void get_lower_index_and_percent(const float power, int& index, float& percent);

  float powf_map[10001][65];
};

////////////////////////////////////////////////////////////////////////////////

};

#endif

#endif	// #ifdef EYE_CANDY
