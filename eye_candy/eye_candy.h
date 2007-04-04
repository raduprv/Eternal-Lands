#ifdef EYE_CANDY

#ifndef EYE_CANDY_H
#define EYE_CANDY_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#ifdef WINDOWS
 #include <windows.h>     
#else
 #include <sys/time.h>
 #include <time.h>
#endif

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <cassert>
#include <SDL.h>
#if defined (OSX) || (OSX86)
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#include <OpenGL/glext.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glext.h>
#endif
#include <stdlib.h>
#include <math.h>

#include "types.h"
#include "math_cache.h"

namespace ec
{

// P R O T O T Y P E S ////////////////////////////////////////////////////////

#define randdouble MathCache::randdouble	// Aliases to static functions to make things easier to type.
#define randfloat MathCache::randfloat
#define randdouble MathCache::randdouble
#define randfloat MathCache::randfloat
#define randint MathCache::randint
#define rand8 Uint8 rand8();	//Functions to ensure a minimum entropy range for the rand function.
#define rand16 MathCache::rand16
#define rand32 MathCache::rand32
#define rand64 MathCache::rand64
#define rand7 MathCache::rand7
#define rand15 MathCache::rand15
#define rand31 MathCache::rand31
#define rand63 MathCache::rand63
#define randcoord MathCache::randcoord
#define randcoord MathCache::randcoord
#define randcolor MathCache::randcolor
#define randcolor MathCache::randcolor
#define randalpha MathCache::randalpha
#define randalpha MathCache::randalpha
#define randenergy MathCache::randenergy
#define randenergy MathCache::randenergy
#define randlight MathCache::randlight
#define randlight MathCache::randlight
#define randpercent MathCache::randpercent
#define randpercent MathCache::randpercent
#define randangle MathCache::randangle
#define randangle MathCache::randangle
#define square MathCache::square
#define square MathCache::square
#define square MathCache::square
#define cube MathCache::cube
#define cube MathCache::cube
#define cube MathCache::cube
#define invsqrt MathCache::invsqrt
#define fastsqrt MathCache::fastsqrt

#ifdef _MSC_VER
 #define copysign _copysign
 inline float fmax(const float a, const float b) { return ((a < b) ? b : a); };
 inline float round(const float a) { return (a - floor(a) < 0.5f ? floor(a) : ceil(a)); };
 inline float remainderf(const float a, const float b) { return (a - (float)round(a / b) * b); };
 #define random rand
 inline void usleep(const unsigned long a) { Sleep(a / 1000); } ;
 
 #pragma warning (disable : 4100) // Unreferenced formal parameter (Justification: I may have a parameter passed for later use.  No harm done.)
 #pragma warning (disable : 4127) // Conditional expression is constant (Justification: Needed for sizeof() checks that will be optimized out; allows for type polymorphism)
 #pragma warning (disable : 4244) // Conversion from type1 to type2 (Justification: This occurs on cases like "float f=1.0;".  In such cases, I don't want to hear a peep out of the compiler; that code does just what I want it to do.  Specifying 1.0f would hurt type polymorphism.)
 #pragma warning (disable : 4305) // Truncation from type1 to type2 (Justification: Like above.)
 #pragma warning (disable : 4311) // Explicitly asting a pointer to a non-pointer type (Justification: Occasionally I use the pointer to an object as salt in a function -- salt that's consistant across that object but not between objects)
#endif

Uint64 get_time();

// M E M B E R S //////////////////////////////////////////////////////////////

const int EC_DEBUG = 1;
const float PI = 3.141592654;
const energy_t G = 6.673e-11;
const int MaxMotionBlurPoints = 5;
const coord_t MAX_DRAW_DISTANCE_SQUARED = 700;

// E X T E R N S //////////////////////////////////////////////////////////////

extern MathCache math_cache;

class Obstruction;
extern std::vector<Obstruction*> null_obstructions;	// Used where we don't want to have to pass a list.

// E N U M S //////////////////////////////////////////////////////////////////

enum EffectEnum
{
  EC_LAMP = 0,
  EC_CAMPFIRE = 1,
  EC_FOUNTAIN = 2,
  EC_TELEPORTER = 3,
  EC_FIREFLY = 4,
  EC_SWORD = 5,
  EC_SUMMON = 6,
  EC_SELFMAGIC = 7,
  EC_TARGETMAGIC = 8,
  EC_ONGOING = 9,
  EC_IMPACT = 10,
  EC_SMOKE = 11,
  EC_BAG = 12,
  EC_CLOUD = 13,
  EC_HARVESTING = 14,
  EC_WIND = 15,
  EC_BREATH = 16
};

// C L A S S E S //////////////////////////////////////////////////////////////

class Vec3
{
public:
  Vec3() {};
  Vec3(coord_t _x, coord_t _y, coord_t _z) { x = _x; y = _y; z = _z; };
  ~Vec3() {};
  
  Vec3 operator+=(const Vec3& rhs)
  {
    x += rhs.x;
    y += rhs.y; 
    z += rhs.z;
    return *this;
  };
  
  Vec3 operator-=(const Vec3& rhs)
  {
    x -= rhs.x;
    y -= rhs.y; 
    z -= rhs.z;
    return *this;
  };
  
  Vec3 operator+(const Vec3& rhs) const
  {
    Vec3 lhs(x, y, z);
    lhs += rhs;
    return lhs;
  };
  
  Vec3 operator-(const Vec3& rhs) const
  {
    Vec3 lhs(x, y, z);
    lhs -= rhs;
    return lhs;
  };
  
  Vec3 operator*=(const coord_t d)
  {
    x *= d;
    y *= d;
    z *= d;
    return *this;
  };
  
  Vec3 operator/=(const coord_t d)
  {
    x /= d;
    y /= d;
    z /= d;
    return *this;
  };
  
  Vec3 operator*(const coord_t d) const
  {
    Vec3 lhs(x, y, z);
    lhs *= d;
    return lhs;
  };
  
  Vec3 operator/(const coord_t d) const
  {
    Vec3 lhs(x, y, z);
    lhs /= d;
    return lhs;
  };
  
  bool operator==(const Vec3& rhs) const
  {
    if ((x == rhs.x) && (y == rhs.y) && (z == rhs.z))
      return true;
    else
      return false;
  };
  
  bool operator!=(const Vec3& rhs) const
  {
    return !(*this == rhs);
  };
  
  Vec3 operator=(const Vec3 rhs)
  {
    x = rhs.x;
    y = rhs.y;
    z = rhs.z;
    return *this;
  };
  
  Vec3 operator-()
  {
    return Vec3(-x, -y, -z);
  };
  
  coord_t magnitude() const
  {
    return fastsqrt(square(x) + square(y) + square(z));
  };

  coord_t magnitude_squared() const
  {
    return square(x) + square(y) + square(z);
  };

  Vec3 normalize()
  {
    (*this) *= invsqrt(magnitude_squared());
    return *this;
  };
  
  Vec3 normalize(const coord_t scale)
  {
    (*this) *= (scale * invsqrt(magnitude_squared()));
    return *this;
  };
  
  void randomize(const coord_t scale = 1.0)
  {
    x = scale * (randcoord() * 2.0 - 1.0);
    y = scale * (randcoord() * 2.0 - 1.0);
    z = scale * (randcoord() * 2.0 - 1.0);
  };
  
  angle_t dot(const Vec3 rhs) const
  {
    return x * rhs.x + y * rhs.y + z * rhs.z;
  };
  
  angle_t angle_to(const Vec3 rhs) const
  {
    Vec3 lhs_normal = *this;
    lhs_normal.normalize();
    Vec3 rhs_normal = rhs;
    rhs_normal.normalize();
    return acos(lhs_normal.x * rhs_normal.x + lhs_normal.y * rhs_normal.y + lhs_normal.z * rhs_normal.z);
  };
  
  angle_t angle_to_prenormalized(const Vec3 rhs) const
  {
    return acos(x * rhs.x + y * rhs.y + z * rhs.z);
  };
  
  Vec3 cross(const Vec3 rhs) const
  {
    return Vec3(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x);
  };
  
  coord_t x, y, z;
};

inline std::ostream& operator<<(std::ostream& lhs, const Vec3 rhs)
{
  return lhs << "<" << rhs.x << ", " << rhs.y << ", " << rhs.z << ">";
};


class Quaternion
{
public:
  Quaternion() { vec.x = 0.0; vec.y = 0.0; vec.z = 0.0; scalar = 1.0; };
  Quaternion(angle_t _x, angle_t _y, angle_t _z, angle_t _scalar) { vec.x = _x; vec.y = _y; vec.z = _z; scalar = _scalar; };
  Quaternion(angle_t _scalar, const Vec3 _vec) { scalar = _scalar; vec = _vec; };
  ~Quaternion() {};
  
  Quaternion conjugate() const
  {
    return Quaternion(-vec.x, -vec.y, -vec.z, scalar);
  };

  Quaternion inverse(const Quaternion rhs) const
  {
    return rhs.conjugate();
  };

  angle_t magnitude() const
  {
    return fastsqrt(square(vec.x) + square(vec.y) + square(vec.z) + square(scalar));
  };

  Quaternion normalize()
  {
    const angle_t inv_sqrt = invsqrt(vec.magnitude_squared() + square(scalar));
    vec *= inv_sqrt;
    scalar *= inv_sqrt;
    return *this;
  };

  Quaternion operator*(const Quaternion rhs) const
  {
    Quaternion ret;
    ret.scalar = vec.dot(rhs.vec);
    ret.vec = vec.cross(rhs.vec) + vec * rhs.scalar + rhs.vec * scalar;
    ret.normalize();
    return ret;
  };

  Quaternion operator*=(const Quaternion rhs)
  {
    (*this) = (*this) * rhs;
    return *this;
  };

  GLfloat* get_matrix(GLfloat* ret) const
  {
    const angle_t xx = vec.x * vec.x;
    const angle_t xy = vec.x * vec.y;
    const angle_t xz = vec.x * vec.z;
    const angle_t xw = vec.x * scalar;
    const angle_t yy = vec.y * vec.y;
    const angle_t yz = vec.y * vec.z; 
    const angle_t yw = vec.y * scalar;
    const angle_t zz = vec.z * vec.z;
    const angle_t zw = vec.z * scalar;

    ret[0] = 1 - 2 * (yy + zz);
    ret[1] =     2 * (xy - zw);
    ret[2] =     2 * (xz + yw);
    ret[3] = 0;
    
    ret[4] =     2 * (xy + zw);
    ret[5] = 1 - 2 * (xx - zz);
    ret[6] =     2 * (yz + xw);
    ret[7] = 0;
    
    ret[8] =      2 * (xz + yw);
    ret[9] =      2 * (yz - xw);
    ret[10] = 1 - 2 * (xx + yy);
    ret[11] = 0;
    
    ret[12] = 0;
    ret[13] = 0;
    ret[14] = 0;
    ret[15] = 1;
    
    return ret;
  };

  void from_matrix(const GLfloat* matrix)
  {
#if 0		// Assumedly slower but equivalent version
    const angle_t trace = matrix[0] + matrix[5] + matrix[10] + 1;
    if (trace > 0)
    {
      const angle_t s = 0.5 * invsqrt(trace);
      scalar = 0.25 / s;
      vec = Vec3(matrix[9] - matrix[6], matrix[2] - matrix[8], matrix[4] - matrix[1]) * s;
    }
    else
    {
      if ((matrix[0] > matrix[5]) && (matrix[0] > matrix[10]))
      {
        const angle_t s = 2.0 * fastsqrt(1.0 + matrix[0] - matrix[5] - matrix[10]);
        vec.x = 0.5 / s;
        vec.y = (matrix[1] + matrix[4]) / s;
        vec.z = (matrix[2] + matrix[8]) / s;
        scalar = (matrix[6] + matrix[9]) / s;
      }
      else if (matrix[5] > matrix[10])
      {
        const angle_t s = 2.0 * fastsqrt(1.0 + matrix[5] - matrix[0] - matrix[10]);
        vec.x = (matrix[1] + matrix[4]) / s;
        vec.y = 0.5 / s;
        vec.z = (matrix[6] + matrix[9]) / s;
        scalar = (matrix[2] + matrix[8]) / s;
      }
      else
      {
        const angle_t s = 2.0 * fastsqrt(1.0 + matrix[10] - matrix[0] - matrix[5]);
        vec.x = (matrix[2] + matrix[8]) / s;
        vec.y = (matrix[6] + matrix[9]) / s;
        vec.z = 0.5 / s;
        scalar = (matrix[1] + matrix[4]) / s;
      }
    }
#else
    scalar = fastsqrt(fmax( 0, 1 + matrix[0] + matrix[5] + matrix[10])) / 2;
    vec.x = fastsqrt(fmax( 0, 1 + matrix[0] - matrix[5] - matrix[10])) / 2;
    vec.y = fastsqrt(fmax( 0, 1 - matrix[0] + matrix[5] - matrix[10])) / 2;
    vec.z = fastsqrt(fmax( 0, 1 - matrix[0] - matrix[5] + matrix[10])) / 2;
    vec.x = copysign(vec.x, matrix[9] - matrix[6] ); 
    vec.y = copysign(vec.y, matrix[2] - matrix[8] );
    vec.z = copysign(vec.z, matrix[4] - matrix[1] );
#endif
  };
  
  void from_axis_and_angle(const Vec3 axis, const angle_t angle)
  {
    vec = axis * sin(angle * 0.5);
    scalar = cos(angle * 0.5);
    normalize();
  };
  
  void get_axis_and_angle(Vec3& axis, angle_t& angle) const
  {
    angle = acos(scalar);
    angle_t sin_angle = fastsqrt(1.0 - square(scalar));
    
    if (fabs(sin_angle) < 0.0001)
      sin_angle = 1;
    
    axis = vec / sin_angle;
  };
  
  Vec3 vec;
  angle_t scalar;
};

inline std::ostream& operator<<(std::ostream& lhs, const Quaternion rhs)
{
  return lhs << "[" << rhs.vec << ", " << rhs.scalar << "]";
};

class Texture
{
public:
  Texture();
  ~Texture();
  
  void push_texture(const std::string filename);
  GLuint get_texture(const Uint16 res_index) const;
  GLuint get_texture(const Uint16 res_index, const int frame) const;
  GLuint get_texture(const Uint16 res_index, const Uint64 born, const Uint64 changerate) const;
  
  std::vector<GLuint> texture_ids[4];
};

class Shape
{
public:
  Shape() { };
  virtual ~Shape();
  
  virtual void draw();
  
  Vec3 pos;
  Vec3 color;
  alpha_t alpha;

protected:
  int vertex_count;
  coord_t* vertices;
  coord_t* normals;
  int facet_count;
  GLuint* facets;

  class Facet
  {
  public:
    Facet(int f1, int f2, int f3) { f[0] = f1; f[1] = f2; f[2] = f3; };
    ~Facet() {};
    
    int f[3];
  };
};

class CaplessCylinder : public Shape
{
public:
  CaplessCylinder(const Vec3 _start, const Vec3 _end, const Vec3 _color, const alpha_t _alpha, const coord_t _radius, const int polys);
  ~CaplessCylinder() {};
  
  coord_t radius;
  Vec3 start;
  Vec3 end;
  Vec3 orig_start;
  Vec3 orig_end;
};

class Cylinder : public Shape
{
public:
  Cylinder(const Vec3 _start, const Vec3 _end, const Vec3 _color, const alpha_t _alpha, const coord_t _radius, const int polys);
  ~Cylinder() {};

  coord_t radius;
  Vec3 start;
  Vec3 end;
  Vec3 orig_start;
  Vec3 orig_end;
};

class Sphere : public Shape
{
public:
  Sphere(const Vec3 pos, const Vec3 _color, const alpha_t _alpha, const coord_t _radius, const int polys);
  ~Sphere() {};

  void average_points(const coord_t p1_first, const coord_t p2_first, const coord_t p1_second, const coord_t p2_second, coord_t& p, coord_t& q);
  
  coord_t radius;
};

class PolarCoordElement
{
public:
  PolarCoordElement(const coord_t _frequency, const coord_t _offset, const coord_t _scalar, const coord_t _power);
  ~PolarCoordElement() {};
  
  coord_t get_radius(const angle_t angle) const;

  coord_t frequency;
  coord_t offset;
  coord_t scalar;
  coord_t power;
};

class ParticleMover;
class EyeCandy;
class Effect;

class ParticleHistory
{
public:
  ParticleHistory() {};
  ParticleHistory(const coord_t _size, const GLuint _texture, const color_t _red, const color_t _green, const color_t _blue, const alpha_t _alpha, const Vec3 _pos)
  {
    size = _size;
    texture = _texture;
    color[0] = _red;
    color[1] = _green;
    color[2] = _blue;
    alpha = _alpha;
    pos = _pos;
  };
  ~ParticleHistory() {};

  coord_t size;
  GLuint texture;
  color_t color[3];
  alpha_t alpha;
  Vec3 pos;
};

class Particle
{
public:
  Particle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity);
  virtual ~Particle();
  
  virtual bool idle(const Uint64 delta_t) = 0;
  virtual GLuint get_texture(const Uint16 res_index) = 0;
  virtual light_t estimate_light_level() const = 0;
  virtual light_t get_light_level() { return alpha * size / 1500; };
  virtual bool deletable() { return true; };
  
  virtual void draw(const Uint64 usec);
  virtual coord_t flare() const;
 
  ParticleMover* mover;
  Vec3 velocity;
  Vec3 pos;
  color_t color[3];
  alpha_t alpha;
  coord_t size;
  Uint64 born;
  energy_t energy;
  coord_t flare_max;	// Bigger values mean bigger flares.  1.0 to max particle size.
  coord_t flare_exp;	// Lower values mean rarer flares.  0.0 to 1.0.
  coord_t flare_frequency; // Geographic scalar between flares.
  Uint16 state;
  Effect* effect;
  EyeCandy* base;
  
  ParticleHistory* motion_blur;
  int cur_motion_blur_point;
};

class ParticleMover
{
public:
  ParticleMover(Effect* _effect);
  virtual ~ParticleMover() {};
  
  virtual void move(Particle& p, Uint64 usec) { p.pos += p.velocity * (usec / 1000000.0); };
  virtual energy_t calculate_energy(const Particle& p) { return 0; };

  Vec3 vec_shift(const Vec3 src, const Vec3 dest, const percent_t percent) const;
  Vec3 vec_shift_amount(const Vec3 src, const Vec3 dest, const coord_t amount) const;
  Vec3 nonpreserving_vec_shift(const Vec3 src, const Vec3 dest, const percent_t percent) const;
  Vec3 nonpreserving_vec_shift_amount(const Vec3 src, const Vec3 dest, const percent_t amount) const;

  Effect* effect;
  EyeCandy* base;
};

class GradientMover : public ParticleMover
{
public:
  GradientMover(Effect* _effect) : ParticleMover(_effect) {};
  virtual ~GradientMover() {};
  
  virtual void move(Particle& p, Uint64 usec);

  virtual Vec3 get_force_gradient(Particle& p) const;
  virtual Vec3 get_obstruction_gradient(Particle& p) const;
};

class SmokeMover : public GradientMover
{
public:
  SmokeMover(Effect* _effect) : GradientMover(_effect) { strength = 1.0; };
  SmokeMover(Effect* _effect, const coord_t _strength) : GradientMover(_effect) { strength = _strength; };
  virtual ~SmokeMover() {};
  
//  virtual void move(Particle& p, Uint64 usec);
  virtual Vec3 get_force_gradient(Particle& p) const;
  
  coord_t strength;
};

class SpiralMover : public GradientMover
{
public:
  SpiralMover(Effect* _effect, Vec3* _center, const coord_t _spiral_speed, const coord_t _pinch_rate) : GradientMover(_effect) { center = _center; spiral_speed = _spiral_speed; pinch_rate = _pinch_rate; };
  virtual ~SpiralMover() {};
  
  virtual Vec3 get_force_gradient(Particle& p) const;
  
  Vec3* center;
  coord_t spiral_speed;
  coord_t pinch_rate;
};

class PolarCoordsBoundingMover : public GradientMover
{
public:
  PolarCoordsBoundingMover(Effect* _effect, const Vec3 _center_pos, const std::vector<PolarCoordElement> _bounding_range, const coord_t _force);
  virtual ~PolarCoordsBoundingMover() {};
  
  virtual Vec3 get_force_gradient(Particle& p) const;

  std::vector<PolarCoordElement> bounding_range;
  coord_t force;
  Vec3 center_pos;
};

class SimpleGravityMover : public GradientMover		// Your basic downward acceleration.
{
public:
  SimpleGravityMover(Effect* _effect) : GradientMover(_effect) {};
  virtual ~SimpleGravityMover() {};
  
  virtual Vec3 get_force_gradient(Particle& p) const;
};

class GravityMover : public GradientMover	// A full-featured gravity simulator.
{
public:
  GravityMover(Effect* _effect, Vec3* _center);
  GravityMover(Effect* _effect, Vec3* _center, const energy_t _mass);
  virtual ~GravityMover() {};
  
  void set_gravity_center(Vec3* _gravity_center);
  virtual void move(Particle& p, Uint64 usec);
  energy_t calculate_velocity_energy(const Particle& p) const;
  energy_t calculate_position_energy(const Particle& p) const;
  coord_t gravity_dist(const Particle& p, const Vec3& center) const;
  energy_t calculate_energy(const Particle& p) const;

  Vec3 old_gravity_center;
  Vec3* gravity_center_ptr;
  Vec3 gravity_center;
  energy_t mass;
  energy_t max_gravity;
};

class ParticleSpawner
{
public:
  ParticleSpawner() {};
  virtual ~ParticleSpawner() {};
  
  virtual Vec3 get_new_coords() = 0;
};

class IFSParticleElement
{
public:
  IFSParticleElement(const coord_t _scale)  { scale = _scale; inv_scale = 1.0 - _scale; };
  virtual ~IFSParticleElement() {};
  
  virtual Vec3 get_new_coords(const Vec3& center) = 0;

  coord_t scale;
  coord_t inv_scale;
};

class IFSLinearElement : public IFSParticleElement
{
public:
  IFSLinearElement(const Vec3 _center, const coord_t _scale) : IFSParticleElement(_scale) { center = _center; };
  virtual ~IFSLinearElement() {};
  
  virtual Vec3 get_new_coords(const Vec3& pos);
  Vec3 center;
};

class IFSSinusoidalElement : public IFSParticleElement
{
public:
  IFSSinusoidalElement(const coord_t _scale, const Vec3 _offset, const Vec3 _scalar, const Vec3 _scalar2) : IFSParticleElement(_scale) { offset = _offset; scalar = _scalar; scalar2 = _scalar2; };
  virtual ~IFSSinusoidalElement() {};
  
  virtual Vec3 get_new_coords(const Vec3& pos);
  
  Vec3 offset;
  Vec3 scalar;
  Vec3 scalar2;
};

class IFSSphericalElement : public IFSParticleElement
{
public:
  IFSSphericalElement(const coord_t _scale, const Vec3 _numerator_adjust, const Vec3 _denominator_adjust) : IFSParticleElement(_scale) { numerator_adjust = _numerator_adjust; denominator_adjust = _denominator_adjust; };
  virtual ~IFSSphericalElement() {};
  
  virtual Vec3 get_new_coords(const Vec3& pos);
  Vec3 numerator_adjust;
  Vec3 denominator_adjust;
};

class IFSRingElement : public IFSParticleElement
{
public:
  IFSRingElement(const coord_t _scale, const Vec3 _numerator_adjust, const Vec3 _denominator_adjust) : IFSParticleElement(_scale) { numerator_adjust = _numerator_adjust; denominator_adjust = _denominator_adjust; };
  virtual ~IFSRingElement() {};
  
  virtual Vec3 get_new_coords(const Vec3& pos);
  Vec3 numerator_adjust;
  Vec3 denominator_adjust;
};

class IFSSwirlElement : public IFSParticleElement
{
public:
  IFSSwirlElement(const coord_t _scale) : IFSParticleElement(_scale) {};
  virtual ~IFSSwirlElement() {};
  
  virtual Vec3 get_new_coords(const Vec3& pos);
};

class IFS2DSwirlElement : public IFSParticleElement
{
public:
  IFS2DSwirlElement(const coord_t _scale) : IFSParticleElement(_scale) {};
  virtual ~IFS2DSwirlElement() {};
  
  virtual Vec3 get_new_coords(const Vec3& pos);
};

class IFSHorseshoeElement : public IFSParticleElement
{
public:
  IFSHorseshoeElement(const coord_t _scale) : IFSParticleElement(_scale) {};
  virtual ~IFSHorseshoeElement() {};
  
  virtual Vec3 get_new_coords(const Vec3& pos);
};

class IFS2DHorseshoeElement : public IFSParticleElement
{
public:
  IFS2DHorseshoeElement(const coord_t _scale) : IFSParticleElement(_scale) {};
  virtual ~IFS2DHorseshoeElement() {};
  
  virtual Vec3 get_new_coords(const Vec3& pos);
};

class IFSParticleSpawner : public ParticleSpawner
{
public:
  IFSParticleSpawner() { pos = Vec3(0.0, 0.0, 0.0); };
  IFSParticleSpawner(const int count, const coord_t size);
  IFSParticleSpawner(const int count, const Vec3 scale);
  virtual ~IFSParticleSpawner();
  
  virtual void generate(const int count, const Vec3 scale);
  virtual Vec3 get_new_coords();
  
  std::vector<IFSParticleElement*> ifs_elements;
  Vec3 pos;
};

class SierpinskiIFSParticleSpawner : public IFSParticleSpawner	// Just a sample.
{
public:
  SierpinskiIFSParticleSpawner()
  {
    ifs_elements.push_back(new IFSLinearElement(Vec3(0.0, 1, 0.0), 0.5));
    ifs_elements.push_back(new IFSLinearElement(Vec3(1, -1, -1), 0.5));
    ifs_elements.push_back(new IFSLinearElement(Vec3(-1.155, -1, -1.155), 0.5));
    ifs_elements.push_back(new IFSLinearElement(Vec3(0.0, -1, 1), 0.5));
  };
};

class FilledSphereSpawner : public ParticleSpawner
{
public:
  FilledSphereSpawner(const coord_t _radius) { radius = _radius; };
  virtual ~FilledSphereSpawner() {};
  
  virtual Vec3 get_new_coords();
  
  coord_t radius;
};

class FilledEllipsoidSpawner : public ParticleSpawner
{
public:
  FilledEllipsoidSpawner(const Vec3 _radius) { radius = _radius; };
  virtual ~FilledEllipsoidSpawner() {};
  
  virtual Vec3 get_new_coords();
  
  Vec3 radius;
};

class HollowSphereSpawner : public ParticleSpawner
{
public:
  HollowSphereSpawner(const coord_t _radius) { radius = _radius; };
  virtual ~HollowSphereSpawner() {};
  
  virtual Vec3 get_new_coords();
  
  coord_t radius;
};

class HollowEllipsoidSpawner : public ParticleSpawner
{
public:
  HollowEllipsoidSpawner(const Vec3 _radius) { radius = _radius; };
  virtual ~HollowEllipsoidSpawner() {};
  
  virtual Vec3 get_new_coords();
  
  Vec3 radius;
};

class FilledDiscSpawner : public ParticleSpawner
{
public:
  FilledDiscSpawner(const coord_t _radius) { radius = _radius; };
  virtual ~FilledDiscSpawner() {};
  
  virtual Vec3 get_new_coords();
  
  coord_t radius;
};

class HollowDiscSpawner : public ParticleSpawner
{
public:
  HollowDiscSpawner(const coord_t _radius) { radius = _radius; };
  virtual ~HollowDiscSpawner() {};
  
  virtual Vec3 get_new_coords();
  
  coord_t radius;
};

class FilledPolarCoordsSpawner : public ParticleSpawner
{
public:
  FilledPolarCoordsSpawner(const std::vector<PolarCoordElement> _bounding_range) { bounding_range = _bounding_range; };
  virtual ~FilledPolarCoordsSpawner() {};
  
  virtual Vec3 get_new_coords();
  coord_t get_area() const;
  
  std::vector<PolarCoordElement> bounding_range;
};

class HollowPolarCoordsSpawner : public ParticleSpawner
{
public:
  HollowPolarCoordsSpawner(const std::vector<PolarCoordElement> _bounding_range) { bounding_range = _bounding_range; };
  virtual ~HollowPolarCoordsSpawner() {};
  
  virtual Vec3 get_new_coords();
  coord_t get_area() const;
  
  std::vector<PolarCoordElement> bounding_range;
};

class Obstruction
{
public:
  Obstruction(const coord_t _max_distance, const coord_t _force);
  virtual ~Obstruction() {};
  
  virtual Vec3 get_force_gradient(Particle& p) = 0;
  
  coord_t max_distance;
  coord_t max_distance_squared;
  coord_t force;
};

class SimpleCylinderObstruction : public Obstruction	// Vertical and infinite.  Speeds up the math if you don't need the extra detail.
{
public:
  SimpleCylinderObstruction(Vec3* _pos, const coord_t _max_distance, const coord_t _force) : Obstruction(_max_distance, _force) { pos = _pos; };
  virtual ~SimpleCylinderObstruction() {};
  
  virtual Vec3 get_force_gradient(Particle& p);

  Vec3* pos;
};

class CylinderObstruction : public Obstruction	// Note: assumes that (*end - *start) doesn't change.
{
public:
  CylinderObstruction(Vec3* _start, Vec3* _end, const coord_t _max_distance, const coord_t _force);
  virtual ~CylinderObstruction() {};
  
  virtual Vec3 get_force_gradient(Particle& p);

  Vec3* start;
  Vec3* end;
  Vec3 length_vec;
  coord_t length_vec_mag;
};

class SphereObstruction : public Obstruction
{
public:
  SphereObstruction(Vec3* _pos, const coord_t _max_distance, const coord_t _force) : Obstruction(_max_distance, _force) { pos = _pos; };
  virtual ~SphereObstruction() {};
  
  virtual Vec3 get_force_gradient(Particle& p);
  
  Vec3* pos;
};

class BoxObstruction : public Obstruction
{
public:
  BoxObstruction(const Vec3 _start, const Vec3 _end, Vec3* _center, float* _sin_rot_x, float*_cos_rot_x, float* _sin_rot_y, float* _cos_rot_y, float* _sin_rot_z, float* _cos_rot_z, float* _sin_rot_x2, float*_cos_rot_x2, float* _sin_rot_y2, float* _cos_rot_y2, float* _sin_rot_z2, float* _cos_rot_z2, const coord_t _force) : Obstruction(1.0, _force)
  {
    start = _start;
    end = _end;
    midpoint = (start + end) / 2;
    size = end - start;
    max_distance_squared = size.magnitude_squared() / 4;
    center = _center;
    sin_rot_x = _sin_rot_x;
    cos_rot_x = _cos_rot_x;
    sin_rot_y = _sin_rot_y;
    cos_rot_y = _cos_rot_y;
    sin_rot_z = _sin_rot_z;
    cos_rot_z = _cos_rot_z;
    sin_rot_x2 = _sin_rot_x2;
    cos_rot_x2 = _cos_rot_x2;
    sin_rot_y2 = _sin_rot_y2;
    cos_rot_y2 = _cos_rot_y2;
    sin_rot_z2 = _sin_rot_z2;
    cos_rot_z2 = _cos_rot_z2;
  };
  virtual ~BoxObstruction() {};
  
  virtual Vec3 get_force_gradient(Particle& p);

  Vec3 start;
  Vec3 end;
  Vec3 midpoint; // Local coordinates
  Vec3 size;
  Vec3* center; // World coordinates
  float max_distance_squared; // The squared radius of a verticle cylinder that describes the maximum area of effect of this bounded object.
  float* sin_rot_x;
  float* cos_rot_x;
  float* sin_rot_y;
  float* cos_rot_y;
  float* sin_rot_z;
  float* cos_rot_z;
  float* sin_rot_x2;
  float* cos_rot_x2;
  float* sin_rot_y2;
  float* cos_rot_y2;
  float* sin_rot_z2;
  float* cos_rot_z2;
};

class Effect
{
public:
  Effect()
  {
    state = 0;
    motion_blur_points = 0;
    motion_blur_fade_rate = 0.001;
    born = get_time();
    recall = false;	//NOTE: All effects *must* respect recall!  If this is flagged, all of its particles should disappear ASAP, and the effect should then return false.
    desired_LOD = 10;
    LOD = desired_LOD;
    active = true;
    obstructions = &null_obstructions;
  };
  virtual ~Effect() { *dead = true; };
  
  void register_particle(Particle* p) { particles[p] = true; };
  void unregister_particle(Particle* p) { particles.erase(particles.find(p)); };
  
  virtual EffectEnum get_type() = 0;
  virtual bool idle(const Uint64 usec) = 0;
  virtual void draw(const Uint64 usec) { };
  virtual void request_LOD(const Uint16 _LOD)
  {
    if (_LOD <= desired_LOD)
      LOD = _LOD;
    else
      LOD = desired_LOD;
  };
  static Uint64 get_max_end_time() { return 0x8000000000000000ull; };
  virtual Uint64 get_expire_time() { return 0x8000000000000000ull; };
  
  EyeCandy* base;
  int motion_blur_points;
  percent_t motion_blur_fade_rate; 	//0 to 1; higher means less fade.
  Uint16 state;
  Uint64 born;
  bool* dead;				//Provided by the effect caller; set when this effect is going away.
  Vec3* pos;
  std::vector<Obstruction*>* obstructions;
  std::map<Particle*, bool> particles;
  bool active;
  bool recall;
  Uint16 desired_LOD;
  Uint16 LOD;
};

class EyeCandy
{
public:
  enum DrawType
  {
    POINT_SPRITES,
    FAST_BILLBOARDS,
    ACCURATE_BILLBOARDS
  };

  EyeCandy();
  EyeCandy(int _max_particles);
  ~EyeCandy();
  
  void set_thresholds(int _max_particles, int min_framerate);
  void load_textures(const std::string basepath);
  void push_back_effect(Effect* e);
  bool push_back_particle(Particle* p);
  void set_camera(const Vec3& _camera) { camera = _camera; };
  void set_dimensions(const coord_t _width, const coord_t _height, const angle_t _zoom) { width = _width; height = _height; zoom = _zoom; temp_sprite_scalar = sprite_scalar * _height / _zoom; };
  void set_sprite_scalar(const coord_t _scalar) { sprite_scalar = _scalar; temp_sprite_scalar = _scalar * height; };
  void draw();
  void idle();
  void add_light(GLenum light_id);
  void start_draw();
  void end_draw();
  void draw_point_sprite_particle(const coord_t size, const GLuint texture, const color_t r, const color_t g, const color_t b, const alpha_t alpha, const Vec3 pos);
  void draw_fast_billboard_particle(const coord_t size, const GLuint texture, const color_t r, const color_t g, const color_t b, const alpha_t alpha, const Vec3 pos);
  void draw_accurate_billboard_particle(const coord_t size, const GLuint texture, const color_t r, const color_t g, const color_t b, const alpha_t alpha, const Vec3 pos);
  Texture TexSimple;
  Texture TexFlare;
  Texture TexVoid;
  Texture TexTwinflare;
  Texture TexInverse;
  Texture TexCrystal;
  Texture TexShimmer;
  Texture TexWater;
  Texture Tex2Lava;
  Texture TexLeafMaple;
  Texture TexLeafOak;
  Texture TexLeafAsh;
  Texture TexPetal;
  Texture TexSnowflake;
  int max_particles;
  Uint64 max_usec_per_particle_move;
  coord_t max_point_size;
  coord_t max_allowable_point_size;
  Vec3 camera;
  coord_t width;
  coord_t height;
  angle_t zoom;
  Uint64 time_diff;
  float framerate;
  light_t lighting_scalar;
  light_t light_estimate;
  std::vector< std::pair<Particle*, light_t> > light_particles;
  unsigned int LOD_1_threshold;
  unsigned int LOD_2_threshold;
  unsigned int LOD_3_threshold;
  unsigned int LOD_4_threshold;
  unsigned int LOD_5_threshold;
  unsigned int LOD_6_threshold;
  unsigned int LOD_7_threshold;
  unsigned int LOD_8_threshold;
  unsigned int LOD_9_threshold;
  float LOD_1_time_threshold;
  float LOD_2_time_threshold;
  float LOD_3_time_threshold;
  float LOD_4_time_threshold;
  float LOD_5_time_threshold;
  float LOD_6_time_threshold;
  float LOD_7_time_threshold;
  float LOD_8_time_threshold;
  float LOD_9_time_threshold;
  int allowable_particles_to_add;
  Uint16 last_forced_LOD;
  DrawType draw_method;
  coord_t billboard_scalar;
  coord_t sprite_scalar;
  coord_t temp_sprite_scalar;
  Vec3 corner_offset1;
  Vec3 corner_offset2;
  std::vector<Effect*> effects;
  std::vector<Particle*> particles;
  std::vector<GLenum> lights;
};

extern std::vector<std::string> ec_logs;
extern bool ec_error_status;

inline void log_warning(std::string message) { ec_logs.push_back("WARNING: " + message + "\n"); };
inline void log_error(std::string message) { ec_logs.push_back("ERROR: " + message + "\n"); ec_error_status = true; };
inline std::vector<std::string> fetch_logs() { const std::vector<std::string> ret(ec_logs); ec_logs.clear(); return ret; };
inline bool get_error_status() { return ec_error_status; };
 
///////////////////////////////////////////////////////////////////////////////

}	// End namespace ec

#endif	// defined EYE_CANDY_H

#endif	// #ifdef EYE_CANDY
