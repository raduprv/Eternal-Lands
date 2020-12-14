/*!
 \brief Eye Candy extension for Eternal Lands

 Introduction:

 The Eye Candy object files were designed with the intent of adding a
 new suite of realistic special effects to Eternal Lands.  Developed in
 C++, they are connected to the game (written in C) through a wrapper,
 eye_candy_wrapper (.cpp and .h).

 Eye Candy effects are largely based on textured point sprites, although
 certain effects use polygons as well.  The point sprites can have multiple
 frames, although none are currently setup as frame-by-frame animations (the
 infrastructure allows for this, however).  Instead, existing point sprites
 randomly pick a frame from a range of possible choices.  The sprites can be
 drawn using one of three methods, two of which are wrapped: OpenGL point
 sprites (not supported on all cards, but theoretically faster), fast
 billboarded quads, and accurate billboarded quads (not wrapped).  There is
 not that much difference between "fast" and "accurate" billboarded quads in
 terms of visual accuracy and speed, so only one of those is wrapped.

 The advantage to using point sprite-based effects is that you can cram a
 great deal of detail into a particle without using many polygons.  Even the
 most basic approximation of a sphere (a tetrahedron) takes four polygons,
 and textures will not wrap well around such a boxy shape.  Textured polys
 allow for wispy, translucent features, smooth curves, and all kinds of other
 effects.  Additionally, they lend themselves naturally to looking as though
 they are filled when tranlucency is used; translucent polygons look like
 shells.

 The primary disadvantage to point sprites is that they are, quite simply,
 sprites.  Thus, they tend to work better for objects that don't vary much
 depending on which angle you look at them from, and appear most realistic
 when there are many on the screen at once, all behaving according to some
 realistic movement rules.

 Actual polygons are used in a few locations in the Eye Candy package, in
 places where point sprites are not suitable.  These include teleporation
 effects (for a translucent column of light) and blowing leaves/flower
 petals.  Fireflies, however, are point sprites, as they are expected to only
 be used in the dark when you wouldn't expect to see the insect body in
 detail.

 Translucency is done in the package using two blend methods.  The default,
 and more common, is "glowing" particles.  These blend accumulatively.  A
 glowing particle will never make its background darker.  Infinite
 accumulation of glowing particles, assuming that there is at least some R,
 G, and B in them, will always result in white.  They work well for magic and
 light sources.  "Non-glowing" particles blend with an average (as with
 glowing, weighted proportional to transparency).  Infinite accumulation of
 non-glowing particles results purely in the color of the particle itself.
 They work well for things like dust, debris, and smoke.

 All effects (except fireflies and leaves/petals, which don't need it) have a
 level of detail flag.  This is the maximum level of detail to use.  The
 number of particles that the effect will use is roughly proportional to its
 level of detail, although different effects may use more or less particles
 than others.  Naturally, lower level of detail is faster but poorer quality.
 Eye Candy also has a built-in particle limit.  As you near this limit, it
 automatically tells effects to lower their level of detail.  Any new
 particles that they create will be done according to the new LOD.  Lastly,
 the framerate of eternallands will also automatically adjust the level of
 detail.  When the total particle count is too high, eye candy will start to
 kill off particles.  Effects that are far enough away that they become
 "inactive" will eventually have their particle counts pruned away to
 nothing.

 The main, controlling object is EyeCandy.  There is only ever one EyeCandy
 object (in our case, it is defined in eye_candy_wrapper.cpp).  It acts as
 the control mechanism for all of the effects and particles, and it is how
 the wrapper interfaces with them.  Effect objects are created manually, but
 destroy themselves -- either automatically when the effect finishes, or when
 told to by having their "recall" flag set (after making sure that their
 particles expire peacefully).  Particle objects are created by the effect,
 and handle all of the details of their drawing.  Particles frequently are
 positioned by Spawners, which pick coordinates in 3-space based on various
 rules, and are moved by Movers, which can simulate things like gravity and
 wind.  Each specific effect class has its own effect_*.cpp file, and is
 based on an object that inherits from Effect (and typically uses particles
 that inherit from Particle).  An additional object used by the system is
 math_cache.cpp, which speeds up certain mathematics functions.

 Note that the Eye Candy system uses a different coordinate system
 than Eternal Lands (Y is up/down in Eye Candy, while in Eternal Lands, Z is
 up/down).  The wrapper takes care of hiding this from the user -- and even
 from Eye Candy itself.

 Lastly, one guiding principle when editing this code: independence.  Eye
 Candy is a completely indepenent piece of code.  It's *only* interface with
 the rest of Eternal Lands is eye_candy_wrapper.  This is by design.  Please
 do not put includes to any other EL code (and thus use any EL globals,
 functions, etc) in Eye Candy.  Rather, work through the wrapper to exchange
 any information you feel is necessary.  This way, Eye Candy remains clean
 and project-independent.
 */

#ifndef EYE_CANDY_H
#define EYE_CANDY_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#if defined(_WIN32) || defined(_WIN64)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <float.h>
#else
#include <sys/time.h>
#include <time.h>
#endif

#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <limits>
#include <cassert>
#include <SDL.h>
#include <stdlib.h>
#include <cmath>

#include "types.h"
#include "math_cache.h"
#include "../platform.h"

#ifdef CLUSTER_INSIDES
#include "../cluster.h"
#endif
#include <memory>
#include "../engine/hardwarebuffer.hpp"

namespace el = eternal_lands;

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
#define randcoord_non_zero MathCache::randcoord_non_zero
#define pow_randfloat MathCache::pow_randfloat
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
#define isnan _isnan
#define isinf !_finite
#define isfinite _finite
	inline float fmax(const float a, const float b)
	{	return ((a < b) ? b : a);};
	inline float round(const float a)
	{	return (a - floor(a) < 0.5f ? floor(a) : ceil(a));};
	inline float remainderf(const float a, const float b)
	{	return (a - (float)round(a / b) * b);};
	inline void usleep(const unsigned long a)
	{	Sleep(a / 1000);};

#pragma warning (disable : 4100) // Unreferenced formal parameter (Justification: I may have a parameter passed for later use.  No harm done.)
#pragma warning (disable : 4127) // Conditional expression is constant (Justification: Needed for sizeof() checks that will be optimized out; allows for type polymorphism)
#pragma warning (disable : 4244) // Conversion from type1 to type2 (Justification: This occurs on cases like "float f=1.0;".  In such cases, I don't want to hear a peep out of the compiler; that code does just what I want it to do.  Specifying 1.0f would hurt type polymorphism.)
#pragma warning (disable : 4305) // Truncation from type1 to type2 (Justification: Like above.)
#pragma warning (disable : 4311) // Explicitly asting a pointer to a non-pointer type (Justification: Occasionally I use the pointer to an object as salt in a function -- salt that's consistant across that object but not between objects)
#endif

	Uint64 get_time();
	void hsv_to_rgb(const color_t h, const color_t s, const color_t v,
		color_t& r, color_t& g, color_t& b);

	// M E M B E R S //////////////////////////////////////////////////////////////

#ifdef DEBUG
#ifndef EC_DEBUG
	const int EC_DEBUG = 1;
#endif // EC_DEBUG
#else // DEBUG
#ifndef EC_DEBUG
	const int EC_DEBUG = 0;
#endif // EC_DEBUG
#endif // DEBUG
	const float PI = 3.141592654;
	const energy_t G = 6.673e-11;
	const int MaxMotionBlurPoints = 5;
	const coord_t MAX_DRAW_DISTANCE = 24.0;
	const coord_t MAX_DRAW_DISTANCE_SQUARED = MAX_DRAW_DISTANCE
		* MAX_DRAW_DISTANCE;

	// E X T E R N S //////////////////////////////////////////////////////////////

	extern MathCache math_cache;

	class Obstruction;
	extern std::vector<Obstruction*> null_obstructions; // Used where we don't want to have to pass a list.

	// E N U M S //////////////////////////////////////////////////////////////////

	// Keep in sync with eye_candy_wrapper.h!
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
		EC_BREATH = 16,
		EC_CANDLE = 17,
		EC_MINES = 18,
		EC_GLOW = 19,
		EC_MISSILE = 20,
		EC_STAFF
	};

	enum TextureEnum
	{
		EC_CRYSTAL,
		EC_FLARE,
		EC_INVERSE,
		EC_SHIMMER,
		EC_SIMPLE,
		EC_TWINFLARE,
		EC_VOID,
		EC_WATER,
		EC_LEAF_ASH,
		EC_LEAF_MAPLE,
		EC_LEAF_OAK,
		EC_PETAL,
		EC_SNOWFLAKE
	};

	// C L A S S E S //////////////////////////////////////////////////////////////

	/*!
	 \brief Vec3: A three-coordinate vector

	 Vec3 contains an x, y, and z coordinate and nothing else.  Unlike
	 std::vectors, which are for data storage, these are a fixed-size, fixed-type
	 structure used for mathematics vector operations -- namely, for particle
	 coordinates and velocities.

	 Possible speed improvement: use SSE like in the math cache's invsqrt to
	 group the variables together into a single 128-bit structure for collective
	 math ops.
	 */
	class Vec3
	{
		public:
			Vec3()
			{
				x = 0.0f;
				y = 0.0f;
				z = 0.0f;
			}
			Vec3(coord_t _x, coord_t _y, coord_t _z)
			{
				x = _x;
				y = _y;
				z = _z;
			}
			Vec3(const Vec3& v) = default;
			~Vec3()
			{
			}
			;

			Vec3 operator+=(const Vec3& rhs)
			{
				x += rhs.x;
				y += rhs.y;
				z += rhs.z;

				if (!is_valid())
				{
					x = 0.0;
					y = 0.0;
					z = 0.0;
				}

				return *this;
			}
			;

			Vec3 operator-=(const Vec3& rhs)
			{
				x -= rhs.x;
				y -= rhs.y;
				z -= rhs.z;

				if (!is_valid())
				{
					x = 0.0;
					y = 0.0;
					z = 0.0;
				}

				return *this;
			}
			;

			Vec3 operator+(const Vec3& rhs) const
			{
				Vec3 lhs(x, y, z);
				lhs += rhs;

				return lhs;
			}
			;

			Vec3 operator-(const Vec3& rhs) const
			{
				Vec3 lhs(x, y, z);
				lhs -= rhs;

				return lhs;
			}
			;

			Vec3 operator*=(const coord_t d)
			{
				x *= d;
				y *= d;
				z *= d;

				return *this;
			}
			;

			Vec3 operator/=(const coord_t d)
			{
				x /= d;
				y /= d;
				z /= d;

				return *this;
			}
			;

			Vec3 operator*(const coord_t d) const
			{
				Vec3 lhs(x, y, z);
				lhs *= d;

				return lhs;
			}
			;

			Vec3 operator/(const coord_t d) const
			{
				Vec3 lhs(x, y, z);
				lhs /= d;

				return lhs;
			}
			;

			bool operator==(const Vec3& rhs) const
			{
				if ((x == rhs.x) && (y == rhs.y) && (z == rhs.z))
					return true;
				else
					return false;
			}
			;

			bool operator!=(const Vec3& rhs) const
			{
				return !(*this == rhs);
			}
			;

			Vec3& operator=(const Vec3& rhs) = default;

			Vec3 operator-()
			{
				return Vec3(-x, -y, -z);
			}
			;

			coord_t magnitude() const
			{
				return std::sqrt(square(x) + square(y) + square(z));
			}
			;

			coord_t magnitude_squared() const
			{
				return square(x) + square(y) + square(z);
			}
			;

			coord_t planar_magnitude() const
			{
				return std::sqrt(square(x) + square(z));
			}
			;

			coord_t planar_magnitude_squared() const
			{
				return square(x) + square(z);
			}
			;

			Vec3 normalize(const coord_t scale = 1.0f)
			{
				coord_t tmp;

				tmp = std::max(magnitude_squared(), 0.0001f);

				(*this) *= (scale / std::sqrt(tmp));

				return *this;
			}
			;

			void randomize(const coord_t scale = 1.0)
			{
				angle_t a, b;

				a = randfloat() * 2.0f * M_PI;
				b = randfloat() * 2.0f * M_PI;

				x = scale * std::sin(a) * std::cos(b);
				y = scale * std::sin(a) * std::sin(b);
				z = scale * std::cos(a);
			}
			;

			angle_t dot(const Vec3 rhs) const
			{
				return x * rhs.x + y * rhs.y + z * rhs.z;
			}
			;

			angle_t angle_to(const Vec3 rhs) const
			{
				Vec3 lhs_normal = *this;
				lhs_normal.normalize();
				Vec3 rhs_normal = rhs;
				rhs_normal.normalize();

				return std::acos(lhs_normal.x * rhs_normal.x + lhs_normal.y
					* rhs_normal.y + lhs_normal.z * rhs_normal.z);
			}
			;

			angle_t angle_to_prenormalized(const Vec3 rhs) const
			{
				return std::acos(x * rhs.x + y * rhs.y + z * rhs.z);
			}
			;

			Vec3 cross(const Vec3 rhs) const
			{
				return Vec3(y * rhs.z - z * rhs.y, z * rhs.x -
					x * rhs.z, x * rhs.y - y * rhs.x);
			}

			static bool is_valid(const float value)
			{
				return (std::abs(value) <
					std::numeric_limits<float>::infinity())
					// test for NaN
					&& (value == value);
			}

			bool is_valid() const
			{
				return is_valid(x) && is_valid(y) &&
					is_valid(z);
			}

			Vec3 as_el()
			{
				return Vec3(x, y, z);
			}

			Vec3 as_ec()
			{
				return Vec3(x, z, -y);
			}

			coord_t x, y, z;
	};

	inline std::ostream& operator<<(std::ostream& lhs, const Vec3 rhs)
	{
		return lhs << "<" << rhs.x << ", " << rhs.y << ", " << rhs.z << ">";
	}
	;

	/*!
	 This class is a standard quaternion.  Think of a quaternion ("quat") as a
	 vector to rotate around and an angle.  That's not exactly correct, but close
	 enough.  The standard method of storing an x rotation, y rotation, and z
	 rotation us subject to a phenominon called "Gimbal locking" and doesn't
	 accumulate well.

	 Like with Vec3s, this class could potentially be sped up by grouping its
	 x, y, and z into a single 128-bit element for aggregate SSE ops.
	 */
	class Quaternion
	{
		public:
			Quaternion()
			{
				vec.x = 0.0;
				vec.y = 0.0;
				vec.z = 0.0;
				scalar = 1.0;
			}
			;
			Quaternion(angle_t _x, angle_t _y, angle_t _z, angle_t _scalar)
			{
				vec.x = _x;
				vec.y = _y;
				vec.z = _z;
				scalar = _scalar;
			}
			;
			Quaternion(angle_t _scalar, const Vec3 _vec)
			{
				scalar = _scalar;
				vec = _vec;
			}
			;
			~Quaternion()
			{
			}
			;

			Quaternion conjugate() const
			{
				return Quaternion(-vec.x, -vec.y, -vec.z, scalar);
			}
			;

			Quaternion inverse(const Quaternion rhs) const
			{
				return rhs.conjugate();
			}
			;

			angle_t magnitude() const
			{
				return std::sqrt(square(vec.x) + square(vec.y) + square(vec.z) + square(scalar));
			}
			;

			Quaternion normalize()
			{
				const angle_t inv_sqrt= 1.0f / std::sqrt(vec.magnitude_squared() + square(scalar));
				vec *= inv_sqrt;
				scalar *= inv_sqrt;
				return *this;
			}
			;

			Quaternion operator*(const Quaternion rhs) const
			{
				Quaternion ret;
				ret.scalar = vec.dot(rhs.vec);
				ret.vec = vec.cross(rhs.vec) + vec * rhs.scalar + rhs.vec
					* scalar;
				ret.normalize();
				return ret;
			}
			;

			Quaternion operator*=(const Quaternion rhs)
			{
				(*this) = (*this) * rhs;
				return *this;
			}
			;

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
				ret[1] = 2 * (xy - zw);
				ret[2] = 2 * (xz + yw);
				ret[3] = 0;

				ret[4] = 2 * (xy + zw);
				ret[5] = 1 - 2 * (xx - zz);
				ret[6] = 2 * (yz + xw);
				ret[7] = 0;

				ret[8] = 2 * (xz + yw);
				ret[9] = 2 * (yz - xw);
				ret[10] = 1 - 2 * (xx + yy);
				ret[11] = 0;

				ret[12] = 0;
				ret[13] = 0;
				ret[14] = 0;
				ret[15] = 1;

				return ret;
			}
			;

			void from_matrix(const GLfloat* matrix)
			{
#if 0		// Assumedly slower but equivalent version
				const angle_t trace = matrix[0] + matrix[5] + matrix[10] + 1;
				if (trace> 0)
				{
					const angle_t s = 0.5 / std::sqrt(trace);
					scalar = 0.25 / s;
					vec = Vec3(matrix[9] - matrix[6], matrix[2] - matrix[8], matrix[4] - matrix[1]) * s;
				}
				else
				{
					if ((matrix[0]> matrix[5]) && (matrix[0]> matrix[10]))
					{
						const angle_t s = 2.0 * std::sqrt(1.0 + matrix[0] - matrix[5] - matrix[10]);
						vec.x = 0.5 / s;
						vec.y = (matrix[1] + matrix[4]) / s;
						vec.z = (matrix[2] + matrix[8]) / s;
						scalar = (matrix[6] + matrix[9]) / s;
					}
					else if (matrix[5]> matrix[10])
					{
						const angle_t s = 2.0 * std::sqrt(1.0 + matrix[5] - matrix[0] - matrix[10]);
						vec.x = (matrix[1] + matrix[4]) / s;
						vec.y = 0.5 / s;
						vec.z = (matrix[6] + matrix[9]) / s;
						scalar = (matrix[2] + matrix[8]) / s;
					}
					else
					{
						const angle_t s = 2.0 * std::sqrt(1.0 + matrix[10] - matrix[0] - matrix[5]);
						vec.x = (matrix[2] + matrix[8]) / s;
						vec.y = (matrix[6] + matrix[9]) / s;
						vec.z = 0.5 / s;
						scalar = (matrix[1] + matrix[4]) / s;
					}
				}
#else
				scalar = std::sqrt(fmax( 0, 1 + matrix[0] + matrix[5] + matrix[10])) / 2;
				vec.x = std::sqrt(fmax( 0, 1 + matrix[0] - matrix[5] - matrix[10])) / 2;
				vec.y = std::sqrt(fmax( 0, 1 - matrix[0] + matrix[5] - matrix[10])) / 2;
				vec.z = std::sqrt(fmax( 0, 1 - matrix[0] - matrix[5] + matrix[10])) / 2;
				vec.x = copysign(vec.x, matrix[9] - matrix[6]);
				vec.y = copysign(vec.y, matrix[2] - matrix[8]);
				vec.z = copysign(vec.z, matrix[4] - matrix[1]);
#endif
			}
			;

			void from_axis_and_angle(const Vec3 axis, const angle_t angle)
			{
				vec = axis * sin(angle * 0.5);
				scalar = cos(angle * 0.5);
				normalize();
			}
			;

			void get_axis_and_angle(Vec3& axis, angle_t& angle) const
			{
				angle = acos(scalar);
				angle_t sin_angle= std::sqrt(1.0 - square(scalar));

				if (fabs(sin_angle) < 0.0001)
					sin_angle = 1;

				axis = vec / sin_angle;
			}
			;

			Vec3 vec;
			angle_t scalar;
	};

	inline std::ostream& operator<<(std::ostream& lhs, const Quaternion rhs)
	{
		return lhs << "[" << rhs.vec << ", " << rhs.scalar << "]";
	}
	;

	/*!
	 \brief The base class for drawing untextured geometric primitives

	 A variety of geometric primitives inherit from Shape.  They all make use
	 of its draw routine, but set their vertex data on their own.  Shapes are
	 used for things like columns of light.
	 */
	class EyeCandy;

	class Shape
	{
		public:
			Shape(EyeCandy* _base)
			{
				base = _base;
			}
			;
			~Shape();
			void draw();

			Vec3 pos;
			Vec3 color;
			alpha_t alpha;
			EyeCandy* base;

		protected:
			int vertex_count;
			int facet_count;
			el::HardwareBuffer vertex_buffer;
			el::HardwareBuffer index_buffer;

			class Facet
			{
				public:
					Facet(int f1, int f2, int f3)
					{
						f[0] = f1;
						f[1] = f2;
						f[2] = f3;
					}
					;
					~Facet()
					{
					}
					;

					int f[3];
			};
	};

	class CaplessCylinder : public Shape
	{
		public:
			CaplessCylinder(EyeCandy* _base, const Vec3 _start,
				const Vec3 _end, const Vec3 _color, const alpha_t _alpha,
				const coord_t _radius, const int polys);
			~CaplessCylinder()
			{
			}
			;

			coord_t radius;
			Vec3 start;
			Vec3 end;
			Vec3 orig_start;
			Vec3 orig_end;
	};

	class Cylinder : public Shape
	{
		public:
			Cylinder(EyeCandy* _base, const Vec3 _start, const Vec3 _end,
				const Vec3 _color, const alpha_t _alpha, const coord_t _radius,
				const int polys);
			~Cylinder()
			{
			}
			;

			coord_t radius;
			Vec3 start;
			Vec3 end;
			Vec3 orig_start;
			Vec3 orig_end;
	};

	class Sphere : public Shape
	{
		public:
			Sphere(EyeCandy* _base, const Vec3 pos, const Vec3 _color,
				const alpha_t _alpha, const coord_t _radius, const int polys);
			~Sphere()
			{
			}
			;

			void average_points(const coord_t p1_first, const coord_t p2_first,
				const coord_t p1_second, const coord_t p2_second, coord_t& p,
				coord_t& q);

			coord_t radius;
	};


	class CaplessCylinders
	{
		public:
			class CaplessCylinderItem
			{
				public:
					CaplessCylinderItem(const Vec3 _start,
						const Vec3 _end,
						const Vec3 _color,
						const alpha_t _alpha,
						const coord_t _radius,
						const int _polys)
					{
						start = _start;
						end = _end;
						color = _color;
						alpha = _alpha;
						radius = _radius;
						polys = _polys;
					}

					Vec3 start;
					Vec3 end;
					Vec3 color;
					alpha_t alpha;
					coord_t radius;
					int polys;
			};

			CaplessCylinders(EyeCandy* _base,
				const std::vector<CaplessCylinderItem> &items);

			void draw(const float alpha_scale);

		protected:
			struct CaplessCylindersVertex
			{
				GLfloat x, y, z;
				GLfloat nx, ny, nz;
				GLubyte r, g, b, a;
			};

			EyeCandy* base;
			int vertex_count;
			int facet_count;
			el::HardwareBuffer vertex_buffer;
			el::HardwareBuffer index_buffer;

	};

	/*!
	 \brief The basic element of a geometric boundary comprised of sinous polar
	 coordinates elements.
	 */
	class PolarCoordElement
	{
		public:
			PolarCoordElement(const coord_t _frequency, const coord_t _offset,
				const coord_t _scalar, const coord_t _power);
			~PolarCoordElement()
			{
			}
			;

			coord_t get_radius(const angle_t angle) const;

			coord_t frequency;
			coord_t offset;
			coord_t scalar;
			coord_t power;
	};

	/*!
	 \brief The basic element of a geometric boundary comprised of sinous polar
	 coordinates elements.
	 */
	class SmoothPolygonElement
	{
		public:
			SmoothPolygonElement(const angle_t _angle, const coord_t _radius)
			{
				angle = _angle;
				radius = _radius;
			}
			;
			~SmoothPolygonElement()
			{
			}
			;

			coord_t get_radius(const angle_t angle) const;

			angle_t angle;
			coord_t radius;
	};

	class ParticleMover;
	class Effect;

	/*!
	 \brief An ultra-simplified particle element

	 Used for a cheap kind of motion blur, if desired.  Contains only the most
	 elementary drawing information.
	 */
	class ParticleHistory
	{
		public:
			ParticleHistory()
			{
				size = 0.00001;
				texture = 0;
				color[0] = 0.0;
				color[1] = 0.0;
				color[2] = 0.0;
				alpha = 0.0;
				pos = Vec3(0.0, 0.0, 0.0);
			}
			;
			ParticleHistory(const coord_t _size, const GLuint _texture,
				const color_t _red, const color_t _green, const color_t _blue,
				const alpha_t _alpha, const Vec3 _pos)
			{
				size = _size;
				texture = _texture;
				color[0] = _red;
				color[1] = _green;
				color[2] = _blue;
				alpha = _alpha;
				pos = _pos;
			}
			;
			~ParticleHistory()
			{
			}
			;

			coord_t size;
			GLuint texture;
			color_t color[3];
			alpha_t alpha;
			Vec3 pos;
	};

	/*!
	 \brief That which adds the term "particle" to the term "particle effect"

	 Apart from the occasional Shape, Particles are what you see when an effect
	 is going off.  Particles are created in Effects, and are referenced both in
	 the effect and the base EyeCandy object.  A particle can order itself to be
	 deleted by returning false in its idle function, but the EyeCandy object
	 retains the right to terminate it at any time without warning.  Particles
	 can be set to "flare" randomly as they move through space, and they can
	 optionally use a basic form of motion blur (not usually worth it).
	 */
	class Particle
	{
		public:
			Particle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos,
				const Vec3 _velocity, const coord_t _size = 1.0f);
			virtual ~Particle();

			virtual bool idle(const Uint64 delta_t) = 0;
			virtual Uint32 get_texture() = 0;
			virtual light_t estimate_light_level() const = 0;
			virtual light_t get_light_level()
			{
				return alpha * size / 1500;
			}
			;
			virtual bool deletable()
			{
				return true;
			}
			;

			virtual float get_burn() const
			{
				return 1.0f;
			}

			void draw(const Uint64 usec);
			virtual coord_t flare() const;

			ParticleMover* mover;
			Vec3 velocity;
			Vec3 pos;
			color_t color[3];
			alpha_t alpha;
			coord_t size;
			Uint64 born;
			energy_t energy;
			coord_t flare_max; // Bigger values mean bigger flares.  1.0 to max particle size.
			coord_t flare_exp; // Lower values mean rarer flares.  0.0 to 1.0.
			coord_t flare_frequency; // Geographic scalar between flares.
			Uint16 state;
			Effect* effect;
			EyeCandy* base;

			ParticleHistory* motion_blur;
			int cur_motion_blur_point;

	};

	/*!
	 \brief A base class for classes that can move particles around

	 Movers take in a particle and a length of time, and put the particle in its
	 new position.  The most basic particle mover simply follows your typical
	 "position += velocity * time" algorithm.
	 */
	class ParticleMover
	{
		public:
			ParticleMover(Effect* _effect);
			virtual ~ParticleMover()
			{
			}
			;

			virtual void move(Particle& p, Uint64 usec)
			{
				p.pos += p.velocity * (usec / 1000000.0);
			}
			;
			virtual energy_t calculate_energy(const Particle& p) const
			{
				return 0;
			}
			;

			Vec3 vec_shift(const Vec3 src, const Vec3 dest,
				const percent_t percent) const;
			Vec3 vec_shift_amount(const Vec3 src, const Vec3 dest,
				const coord_t amount) const;
			Vec3 nonpreserving_vec_shift(const Vec3 src, const Vec3 dest,
				const percent_t percent) const;
			Vec3 nonpreserving_vec_shift_amount(const Vec3 src,
				const Vec3 dest, const percent_t amount) const;

			Effect* effect;
			EyeCandy* base;

			virtual void attachParticle(Particle *)
			{
			}
			;
			virtual void detachParticle(Particle *)
			{
			}
			;
	};

	/*!
	 \brief A Mover which applies an acceleration gradient.
	 */
	class GradientMover : public ParticleMover
	{
		public:
			GradientMover(Effect* _effect) :
				ParticleMover(_effect)
			{
			}
			;
			virtual ~GradientMover()
			{
			}
			;

			virtual void move(Particle& p, Uint64 usec);

			virtual Vec3 get_force_gradient(Particle& p) const;
			virtual Vec3 get_obstruction_gradient(Particle& p) const;
	};

	/*!
	 \brief A simple example of a gradient mover, used for simple smoke effects.
	 */
	class SmokeMover : public GradientMover
	{
		public:
			SmokeMover(Effect* _effect) :
				GradientMover(_effect)
			{
				strength = 1.0;
			}
			;
			SmokeMover(Effect* _effect, const coord_t _strength) :
				GradientMover(_effect)
			{
				strength = _strength;
			}
			;
			virtual ~SmokeMover()
			{
			}
			;

			//  virtual void move(Particle& p, Uint64 usec);
			virtual Vec3 get_force_gradient(Particle& p) const;

			coord_t strength;
	};

	/*!
	 \brief A gradient mover which whirls/pinches particles.

	 If spiral speed is equal to -pinch_rate, particles will follow a rough
	 circle.  A greater magnitude pinch and they'll go inwards.  A lesser
	 magnitude pinch and they'll spiral outwards.
	 */
	class SpiralMover : public GradientMover
	{
		public:
			SpiralMover(Effect* _effect, Vec3* _center,
				const coord_t _spiral_speed, const coord_t _pinch_rate) :
				GradientMover(_effect)
			{
				center = _center;
				spiral_speed = _spiral_speed;
				pinch_rate = _pinch_rate;
			}
			;
			virtual ~SpiralMover()
			{
			}
			;

			virtual Vec3 get_force_gradient(Particle& p) const;

			Vec3* center;
			coord_t spiral_speed;
			coord_t pinch_rate;
	};

	/*!
	 \brief Defines a range to be used by bounding movers and spawners.
	 */
	class BoundingRange
	{
		public:
			BoundingRange()
			{
			}
			;
			virtual ~BoundingRange()
			{
			}
			;

			virtual coord_t get_radius(const angle_t angle) const = 0;
	};

	/*!
	 \brief A bounding range composed of a sum of sinusoidal elements in polar
	 coordinates space.
	 */

	class PolarCoordsBoundingRange : public BoundingRange
	{
		public:
			PolarCoordsBoundingRange()
			{
			}
			;
			~PolarCoordsBoundingRange()
			{
			}
			;

			virtual coord_t get_radius(const angle_t angle) const;

			std::vector<PolarCoordElement> elements;
	};

	/*!
	 \brief A bounding range composed of a series of angles and radii forming a
	 polygon that is smoothly interpolated.
	 */

	class SmoothPolygonBoundingRange : public BoundingRange
	{
		public:
			SmoothPolygonBoundingRange()
			{
			}
			;
			~SmoothPolygonBoundingRange()
			{
			}
			;

			virtual coord_t get_radius(const angle_t angle) const;

			std::vector<SmoothPolygonElement> elements;
	};

	/*!
	 \brief A gradient mover that confines particles to a bounding range

	 This is a base class for specific bounding movers.
	 */
	class BoundingMover : public GradientMover
	{
		public:
			BoundingMover(Effect* _effect, const Vec3 _center_pos,
				BoundingRange* _bounding_range, const coord_t _force);
			~BoundingMover()
			{
			}
			;

			virtual Vec3 get_force_gradient(Particle& p) const;

			coord_t force;
			Vec3 center_pos;
			BoundingRange* bounding_range;
	};

	/*!
	 \brief A simple gradient mover that implements downward-only gravitational
	 acceleration.
	 */
	class SimpleGravityMover : public GradientMover // Your basic downward acceleration.
	{
		public:
			SimpleGravityMover(Effect* _effect) :
				GradientMover(_effect)
			{
			}
			;
			virtual ~SimpleGravityMover()
			{
			}
			;

			virtual Vec3 get_force_gradient(Particle& p) const;
	};

	/*!
	 \brief A true gravity mover

	 This mover implements a near physics-sim-quality gravitational attraction
	 mechanism (single source gravity only, though).  This includes things like
	 tracking how much potential/kinetic energy a particle has in order to
	 compensate for the uneven acceleration that occurs between frames -- a
	 particle passing right past the center may be 5 units away on one frame, and
	 0.1 units on the next frame.  The latter frame will experience a much
	 greater gravitational attraction, even though it really should be taking a
	 smooth curve.  The only better way to keep particles moving properly than
	 our energy-tracking mechanism would be to actually integrate across the path,
	 and there's no way we'd have the CPU time for that.
	 */
	class GravityMover : public GradientMover // A full-featured gravity simulator.
	{
		public:
			GravityMover(Effect* _effect, Vec3* _center);
			GravityMover(Effect* _effect, Vec3* _center, const energy_t _mass);
			virtual ~GravityMover()
			{
			}
			;

			void set_gravity_center(Vec3* _gravity_center);
			virtual void move(Particle& p, Uint64 usec);
			energy_t calculate_velocity_energy(const Particle& p) const;
			energy_t calculate_position_energy(const Particle& p) const;
			coord_t gravity_dist(const Particle& p, const Vec3& center) const;
			virtual energy_t calculate_energy(const Particle& p) const;

			Vec3 old_gravity_center;
			Vec3* gravity_center_ptr;
			Vec3 gravity_center;
			energy_t mass;
			energy_t max_gravity;
	};

	/*!
	 \brief The base class for particle spawners

	 Particle spawners are effort-saving objects that can be called to determine
	 coordinates for placement of new particles.
	 */
	class ParticleSpawner
	{
		public:
			ParticleSpawner()
			{
			}
			;
			virtual ~ParticleSpawner()
			{
			}
			;

			virtual Vec3 get_new_coords() = 0;
	};

	/*!
	 \brief Base class for objects used in setting up an IFS (Iterative Function
	 System) particle spawner's shape.
	 */
	class IFSParticleElement
	{
		public:
			IFSParticleElement(const coord_t _scale)
			{
				scale = _scale;
				inv_scale = 1.0 - _scale;
			}
			;
			virtual ~IFSParticleElement()
			{
			}
			;

			virtual Vec3 get_new_coords(const Vec3& center) = 0;

			coord_t scale;
			coord_t inv_scale;
	};

	/*!
	 \brief Your standard IFS linear-interpolation between points
	 */
	class IFSLinearElement : public IFSParticleElement
	{
		public:
			IFSLinearElement(const Vec3 _center, const coord_t _scale) :
				IFSParticleElement(_scale)
			{
				center = _center;
			}
			;
			virtual ~IFSLinearElement()
			{
			}
			;

			virtual Vec3 get_new_coords(const Vec3& pos);
			Vec3 center;
	};

	/*!
	 \brief A "flame" extension to IFS.  Only works so-so in this case.
	 */
	class IFSSinusoidalElement : public IFSParticleElement
	{
		public:
			IFSSinusoidalElement(const coord_t _scale, const Vec3 _offset,
				const Vec3 _scalar, const Vec3 _scalar2) :
				IFSParticleElement(_scale)
			{
				offset = _offset;
				scalar = _scalar;
				scalar2 = _scalar2;
			}
			;
			virtual ~IFSSinusoidalElement()
			{
			}
			;

			virtual Vec3 get_new_coords(const Vec3& pos);

			Vec3 offset;
			Vec3 scalar;
			Vec3 scalar2;
	};

	/*!
	 \brief A "flame" extension to IFS.  Only works so-so in this case.
	 */
	class IFSSphericalElement : public IFSParticleElement
	{
		public:
			IFSSphericalElement(const coord_t _scale,
				const Vec3 _numerator_adjust, const Vec3 _denominator_adjust) :
				IFSParticleElement(_scale)
			{
				numerator_adjust = _numerator_adjust;
				denominator_adjust = _denominator_adjust;
			}
			;
			virtual ~IFSSphericalElement()
			{
			}
			;

			virtual Vec3 get_new_coords(const Vec3& pos);
			Vec3 numerator_adjust;
			Vec3 denominator_adjust;
	};

	/*!
	 \brief A "flame" extension to IFS.  Only works so-so in this case.
	 */
	class IFSRingElement : public IFSParticleElement
	{
		public:
			IFSRingElement(const coord_t _scale, const Vec3 _numerator_adjust,
				const Vec3 _denominator_adjust) :
				IFSParticleElement(_scale)
			{
				numerator_adjust = _numerator_adjust;
				denominator_adjust = _denominator_adjust;
			}
			;
			virtual ~IFSRingElement()
			{
			}
			;

			virtual Vec3 get_new_coords(const Vec3& pos);
			Vec3 numerator_adjust;
			Vec3 denominator_adjust;
	};

	/*!
	 \brief A "flame" extension to IFS.  Only works so-so in this case.
	 */
	class IFSSwirlElement : public IFSParticleElement
	{
		public:
			IFSSwirlElement(const coord_t _scale) :
				IFSParticleElement(_scale)
			{
			}
			;
			virtual ~IFSSwirlElement()
			{
			}
			;

			virtual Vec3 get_new_coords(const Vec3& pos);
	};

	/*!
	 \brief A "flame" extension to IFS.  Only works so-so in this case.
	 */
	class IFS2DSwirlElement : public IFSParticleElement
	{
		public:
			IFS2DSwirlElement(const coord_t _scale) :
				IFSParticleElement(_scale)
			{
			}
			;
			virtual ~IFS2DSwirlElement()
			{
			}
			;

			virtual Vec3 get_new_coords(const Vec3& pos);
	};

	/*!
	 \brief A "flame" extension to IFS.  Only works so-so in this case.
	 */
	class IFSHorseshoeElement : public IFSParticleElement
	{
		public:
			IFSHorseshoeElement(const coord_t _scale) :
				IFSParticleElement(_scale)
			{
			}
			;
			virtual ~IFSHorseshoeElement()
			{
			}
			;

			virtual Vec3 get_new_coords(const Vec3& pos);
	};

	/*!
	 \brief A "flame" extension to IFS.  Only works so-so in this case.
	 */
	class IFS2DHorseshoeElement : public IFSParticleElement
	{
		public:
			IFS2DHorseshoeElement(const coord_t _scale) :
				IFSParticleElement(_scale)
			{
			}
			;
			virtual ~IFS2DHorseshoeElement()
			{
			}
			;

			virtual Vec3 get_new_coords(const Vec3& pos);
	};

	/*!
	 \brief An IFS (Iterative Function System) particle spawner, for fractaline
	 spawning shapes.
	 */
	class IFSParticleSpawner : public ParticleSpawner
	{
		public:
			IFSParticleSpawner()
			{
				pos = Vec3(0.0, 0.0, 0.0);
			}
			;
			IFSParticleSpawner(const int count, const coord_t size);
			IFSParticleSpawner(const int count, const Vec3 scale);
			virtual ~IFSParticleSpawner();

			virtual void generate(const int count, const Vec3 scale);
			virtual Vec3 get_new_coords();

			std::vector<IFSParticleElement*> ifs_elements;
			Vec3 pos;
	};

	/*!
	 \brief A sample IFS spawner: spawns particles in a Sierpinski tetrahedron.
	 */
	class SierpinskiIFSParticleSpawner : public IFSParticleSpawner // Just a sample.
	{
		public:
			SierpinskiIFSParticleSpawner()
			{
				ifs_elements.push_back(new IFSLinearElement(Vec3(0.0, 1, 0.0), 0.5));
				ifs_elements.push_back(new IFSLinearElement(Vec3(1, -1, -1), 0.5));
				ifs_elements.push_back(new IFSLinearElement(Vec3(-1.155, -1, -1.155), 0.5));
				ifs_elements.push_back(new IFSLinearElement(Vec3(0.0, -1, 1), 0.5));
			}
			;
			SierpinskiIFSParticleSpawner(float scale)
			{
				ifs_elements.push_back(new IFSLinearElement(Vec3(0.0 * scale, 1 * scale, 0.0 * scale), 0.5 * scale));
				ifs_elements.push_back(new IFSLinearElement(Vec3(1 * scale, -1 * scale, -1 * scale), 0.5 * scale));
				ifs_elements.push_back(new IFSLinearElement(Vec3(-1.155 * scale, -1 * scale, -1.155 * scale), 0.5 * scale));
				ifs_elements.push_back(new IFSLinearElement(Vec3(0.0 * scale, -1 * scale, 1 * scale), 0.5 * scale));
			}
			;
	};

	/*!
	 \brief Spawns particles throughout a sphere.
	 */
	class FilledSphereSpawner : public ParticleSpawner
	{
		public:
			FilledSphereSpawner(const coord_t _radius)
			{
				radius = _radius;
			}
			;
			virtual ~FilledSphereSpawner()
			{
			}
			;

			virtual Vec3 get_new_coords();

			coord_t radius;
	};

	/*!
	 \brief Spawns particles throughout an ellipsoid.
	 */
	class FilledEllipsoidSpawner : public ParticleSpawner
	{
		public:
			FilledEllipsoidSpawner(const Vec3 _radius)
			{
				radius = _radius;
			}
			;
			virtual ~FilledEllipsoidSpawner()
			{
			}
			;

			virtual Vec3 get_new_coords();

			Vec3 radius;
	};

	/*!
	 \brief Spawns particles on the rim of a sphere.
	 */
	class HollowSphereSpawner : public ParticleSpawner
	{
		public:
			HollowSphereSpawner(const coord_t _radius)
			{
				radius = _radius;
			}
			;
			virtual ~HollowSphereSpawner()
			{
			}
			;

			virtual Vec3 get_new_coords();

			coord_t radius;
	};

	/*!
	 \brief Spawns particles on the rim of an ellipsoid.
	 */
	class HollowEllipsoidSpawner : public ParticleSpawner
	{
		public:
			HollowEllipsoidSpawner(const Vec3 _radius)
			{
				radius = _radius;
			}
			;
			virtual ~HollowEllipsoidSpawner()
			{
			}
			;

			virtual Vec3 get_new_coords();

			Vec3 radius;
	};

	/*!
	 \brief Spawns particles throughout a two-dimensional disc.
	 */
	class FilledDiscSpawner : public ParticleSpawner
	{
		public:
			FilledDiscSpawner(const coord_t _radius)
			{
				radius = _radius;
			}
			;
			virtual ~FilledDiscSpawner()
			{
			}
			;

			virtual Vec3 get_new_coords();

			coord_t radius;
	};

	/*!
	 \brief Spawns particles on the rim of a two-dimensional disc.
	 */
	class HollowDiscSpawner : public ParticleSpawner
	{
		public:
			HollowDiscSpawner(const coord_t _radius)
			{
				radius = _radius;
			}
			;
			virtual ~HollowDiscSpawner()
			{
			}
			;

			virtual Vec3 get_new_coords();

			coord_t radius;
	};

	/*!
	 \brief Spawns particles within a range
	 */
	class FilledBoundingSpawner : public ParticleSpawner
	{
		public:
			FilledBoundingSpawner(BoundingRange* _bounding_range,
				Vec3* _center, Vec3* _base_center, float _range_scalar = 1.0)
			{
				bounding_range = _bounding_range;
				center = _center;
				base_center = _base_center;
				range_scalar = _range_scalar;
			}
			;
			virtual ~FilledBoundingSpawner()
			{
			}
			;

			virtual Vec3 get_new_coords();
			coord_t get_area() const;

			BoundingRange* bounding_range;
			Vec3* center;
			Vec3* base_center;
			float range_scalar;
	};

	/*!
	 \brief Spawns particles within a range
	 */
	class NoncheckingFilledBoundingSpawner : public ParticleSpawner
	{
		public:
			NoncheckingFilledBoundingSpawner(BoundingRange* _bounding_range)
			{
				bounding_range = _bounding_range;
			}
			;
			virtual ~NoncheckingFilledBoundingSpawner()
			{
			}
			;

			virtual Vec3 get_new_coords();
			coord_t get_area() const;

			BoundingRange* bounding_range;
	};

	/*!
	 \brief Spawns particles on the edge of a range
	 */
	class HollowBoundingSpawner : public ParticleSpawner
	{
		public:
			HollowBoundingSpawner(BoundingRange* _bounding_range)
			{
				bounding_range = _bounding_range;
			}
			;
			virtual ~HollowBoundingSpawner()
			{
			}
			;

			virtual Vec3 get_new_coords();
			coord_t get_area() const;

			BoundingRange* bounding_range;
	};

	/*!
	 \brief Base class for obstructions -- objects that deflect particles.
	 */
	class Obstruction
	{
		public:
			Obstruction(const coord_t _max_distance, const coord_t _force);
			virtual ~Obstruction()
			{
			}
			;

			virtual Vec3 get_force_gradient(Particle& p) const = 0;

			coord_t max_distance;
			coord_t max_distance_squared;
			coord_t force;
	};

	/*!
	 \brief An obstruction shaped like a vertical cylinder of infinite length (fast)
	 */
	class SimpleCylinderObstruction : public Obstruction // Vertical and infinite.  Speeds up the math if you don't need the extra detail.
	{
		public:
			SimpleCylinderObstruction(Vec3* _pos, const coord_t _max_distance,
				const coord_t _force) :
				Obstruction(_max_distance, _force)
			{
				pos = _pos;
			}
			;
			virtual ~SimpleCylinderObstruction()
			{
			}
			;

			virtual Vec3 get_force_gradient(Particle& p) const;

			Vec3* pos;
	};

	/*!
	 \brief Like above, but with top and bottom caps
	 */
	class CappedSimpleCylinderObstruction : public Obstruction
	{
		public:
			CappedSimpleCylinderObstruction(Vec3* _pos,
				const coord_t _max_distance, const coord_t _force,
				const coord_t _bottom, const coord_t _top) :
				Obstruction(_max_distance, _force)
			{
				pos = _pos;
				bottom = _bottom;
				top = _top;
			}
			;
			virtual ~CappedSimpleCylinderObstruction()
			{
			}
			;

			virtual Vec3 get_force_gradient(Particle& p) const;

			Vec3* pos;
			coord_t bottom;
			coord_t top;
	};

	/*!
	 \brief An obstruction shaped like a cylinder of finite length at any angle (slow)
	 */
	class CylinderObstruction : public Obstruction // Note: assumes that (*end - *start) doesn't change.
	{
		public:
			CylinderObstruction(Vec3* _start, Vec3* _end,
				const coord_t _max_distance, const coord_t _force);
			virtual ~CylinderObstruction()
			{
			}
			;

			virtual Vec3 get_force_gradient(Particle& p) const;

			Vec3* start;
			Vec3* end;
			Vec3 length_vec;
			coord_t length_vec_mag;
	};

	/*!
	 \brief An obstruction shaped like a sphere (fast)
	 */
	class SphereObstruction : public Obstruction
	{
		public:
			SphereObstruction(Vec3* _pos, const coord_t _max_distance,
				const coord_t _force) :
				Obstruction(_max_distance, _force)
			{
				pos = _pos;
			}
			;
			virtual ~SphereObstruction()
			{
			}
			;

			virtual Vec3 get_force_gradient(Particle& p) const;

			Vec3* pos;
	};

	/*!
	 \brief An obstruction shaped like a box at any angle (slow)
	 */
	class BoxObstruction : public Obstruction
	{
		public:
			BoxObstruction(const Vec3 _start, const Vec3 _end, Vec3* _center,
				float* _sin_rot_x, float*_cos_rot_x, float* _sin_rot_y,
				float* _cos_rot_y, float* _sin_rot_z, float* _cos_rot_z,
				float* _sin_rot_x2, float*_cos_rot_x2, float* _sin_rot_y2,
				float* _cos_rot_y2, float* _sin_rot_z2, float* _cos_rot_z2,
				const coord_t _force) :
				Obstruction(1.0, _force)
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
			}
			;
			virtual ~BoxObstruction()
			{
			}
			;

			virtual Vec3 get_force_gradient(Particle& p) const;

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

	/*!
	 \brief The base element for every kind of eye candy effect

	 An eye candy effect is a single visual phenominon composed of many particles.
	 Example effects would include things like a "fountain" or a "fire".  Effects
	 can run their course and then expire (simply by returning false in their
	 idle function) or be persistent.  Effects are manually created by the caller
	 (using the c++ "new" operator), but delete themselves.  For the caller to
	 stop an effect, they need to flag it's recall flag; an effect must respect
	 the recall flag when it sees it and clean up its state, then return false
	 in its idle function.  Effects can also draw non-particle elements if they
	 wish in their draw function.
	 */
	class Effect
	{
		public:
			Effect()
			{
				state = 0;
				motion_blur_points = 0;
				motion_blur_fade_rate = 0.001;
				born = get_time();
				recall = false; //NOTE: All effects *must* respect recall!  If this is flagged, all of its particles should disappear ASAP, and the effect should then return false.
				desired_LOD = 10;
				LOD = desired_LOD;
				active = true;
				obstructions = &null_obstructions;
				bounds = NULL;
				particle_max_count = 0;
				particle_count = 0;
				buffer = 0;
			}
			;
			virtual ~Effect()
			{
				*dead = true;
			}
			;

			void draw_particle(const coord_t size,
				const Uint32 texture, const color_t r,
				const color_t g, const color_t b,
				const alpha_t alpha, const Vec3 pos,
				const alpha_t burn);
			void build_particle_buffer(const Uint64 time_diff);
			void draw_particle_buffer();

			void register_particle(Particle* p)
			{
				particles[p] = true;
			}
			;
			void unregister_particle(Particle* p)
			{
				particles.erase(particles.find(p));
			}
			;

			virtual EffectEnum get_type() = 0;
			virtual bool idle(const Uint64 usec) = 0;
			virtual void draw(const Uint64 usec)
			{
				for (auto& iter2: particles)
				{
					for (auto obstruction: *obstructions)
					{
						obstruction->get_force_gradient(*iter2.first);
					}
				}
			}
			;
			virtual void request_LOD(const float _LOD)
			{
				if (fabs(_LOD - (float)LOD) < 1.0)
					return;
				const Uint16 rounded_LOD = (Uint16)round(_LOD);
				if (rounded_LOD <= desired_LOD)
					LOD = rounded_LOD;
				else
					LOD = desired_LOD;
			}
			;
			static Uint64 get_max_end_time()
			{
				return 0x8000000000000000ull;};
			virtual Uint64 get_expire_time()
			{	return 0x8000000000000000ull;};

#ifdef CLUSTER_INSIDES
			short getCluster () const
			{
				if (!pos)
				return 0;
				return get_cluster (int (pos->x / 0.5f), int (-pos->z / 0.5f));
			}

			bool belongsToCluster (short cluster) const
			{
				short my_cluster = getCluster ();
				return my_cluster == cluster || my_cluster == 0;
			}
#endif

			EyeCandy* base;
			int motion_blur_points;
			percent_t motion_blur_fade_rate; //0 to 1; higher means less fade.
			Uint16 state;
			Uint64 born;
			bool* dead; //Provided by the effect caller; set when this effect is going away.
			Vec3* pos;
			std::vector<Obstruction*>* obstructions;
			std::map<Particle*, bool> particles;
			BoundingRange* bounds;
			bool active;
			bool recall;
			Uint16 desired_LOD;
			Uint16 LOD;
			protected:
			el::HardwareBuffer particle_vertex_buffer;
			Uint32 particle_max_count;
			Uint32 particle_count;
			float* buffer;
		};

		/*!
		 \brief The core object of all eye candy

		 The EyeCandy object (there should only ever be one) encapsulates all of the
		 effects and particles that will occur in a program.  There are numerous
		 options, flags, and settings that can be set for the eye candy object.
		 A few critical notes:

		 1) When initializing the object, be sure to load_textures().
		 2) Optimially, give it a few lights (the add_light() function).
		 3) Between idle calls, set how much time has passed (time_diff) and the
		 camera's location (set_camera()).
		 4) The key functions to call every cycle are ec_idle() and ec_draw().
		 5) When not drawing, don't call ec_draw().  If you wish to save CPU time,
		 you can skip calling ec_idle() most of the time, but if you ever delete
		 effects, you'll want to let it run once to help clear out the system.

		 */
		class EyeCandy
		{
			public:

			EyeCandy();
			EyeCandy(int _max_particles);
			~EyeCandy();

			void set_thresholds(const int _max_particles, const float min_framerate, const float max_framerate);
			void load_textures();
			void push_back_effect(Effect* e);
			bool push_back_particle(Particle* p);
			void set_camera(const Vec3& _camera)
			{	camera = _camera;};
			void set_center(const Vec3& _center)
			{	center = _center;};
			void set_dimensions(const coord_t _width, const coord_t _height, const angle_t _zoom)
			{	width = _width; height = _height; zoom = _zoom; temp_sprite_scalar = sprite_scalar * _height / _zoom;};
			void set_sprite_scalar(const coord_t _scalar)
			{	sprite_scalar = _scalar; temp_sprite_scalar = _scalar * height;};
			void draw();
			void idle();
			void add_light(GLenum light_id);
			void start_draw();
			void end_draw();
			Uint32 get_texture(const TextureEnum type) const;
			void set_particle_texture_combiner();
			void set_shape_texture_combiner(const float alpha_scale);

#if __cplusplus >= 201103L
			std::unique_ptr<el::HardwareBuffer> index_buffer;
#else
			std::auto_ptr<el::HardwareBuffer> index_buffer;
#endif
			Uint32 texture_atlas;
			Uint32 texture_burn;
			int max_particles;
			Uint64 max_usec_per_particle_move;
			coord_t max_point_size;
			coord_t max_allowable_point_size;
			Vec3 camera;
			Vec3 center;
			coord_t width;
			coord_t height;
			angle_t zoom;
			Uint64 time_diff;
			float framerate;
			float max_fps;
			light_t lighting_scalar;
			light_t light_estimate;
			bool use_lights;
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
			float LOD_10_time_threshold;
			int allowable_particles_to_add;
			bool draw_shapes;
			Uint16 last_forced_LOD;
			coord_t billboard_scalar;
			coord_t sprite_scalar;
			coord_t temp_sprite_scalar;
			Vec3 corner_offset1;
			Vec3 corner_offset2;
			std::vector<Effect*> effects;
			std::vector<Particle*> particles;
			std::vector<GLenum> lights;
		};

		extern bool ec_error_status;
		inline bool get_error_status()
		{	return ec_error_status;};

		/*!
		 \brief A class to simplify logging of output.
		 */
		class LoggerBuf : public std::streambuf
		{
			public:
			LoggerBuf()
			{	temp_log = "";};
			virtual ~LoggerBuf()
			{};

			std::streamsize xsputn(const char_type* str, std::streamsize n)
			{
				std::string new_str(str, n);
				temp_log += new_str;

				std::cout << new_str;

				const std::streamsize ret = static_cast<std::streamsize>(new_str.length());
				return ret;
			};

			int_type overflow(int_type ch)
			{
				if ((char)ch == '\n')
				{
					logs.push_back(temp_log + "\n");
					temp_log = "";
					std::cout << std::endl;
				}
				return ch;
			}

			public:
			std::string temp_log;
			std::vector<std::string> logs;
		};

		class Logger : public std::ostream
		{
			public:
			Logger() : std::ios(0), std::ostream(new LoggerBuf())
			{};
			~Logger()
			{	delete rdbuf();};

			void log_text(const std::string& message)
			{	((LoggerBuf*)rdbuf())->logs.push_back(message);};
			void log_warning(const std::string& message)
			{	log_text("WARNING: " + message + "\n");};
			void log_error(const std::string& message)
			{	log_text("ERROR: " + message + "\n"); ec_error_status = true;};
			std::vector<std::string> fetch()
			{	const std::vector<std::string> ret(((LoggerBuf*)rdbuf())->logs); ((LoggerBuf*)rdbuf())->logs.clear(); return ret;};
		};

		extern Logger logger;

		///////////////////////////////////////////////////////////////////////////////

	} // End namespace ec

#endif	// defined EYE_CANDY_H
