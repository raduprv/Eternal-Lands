/****************************************************************************
 *            normal.c
 *
 * Author: 2008  Daniel Jungmann <dsj@gmx.net>
 * Copyright: See COPYING file that comes with this distribution
 ****************************************************************************/

#include "normal.h"
#include <math.h>

// upper 3 bits
#define SIGN_MASK  0xe000
#define XSIGN_MASK 0x8000
#define YSIGN_MASK 0x4000
#define ZSIGN_MASK 0x2000

// middle 6 bits - xbits
#define TOP_MASK  0x1f80

// lower 7 bits - ybits
#define BOTTOM_MASK  0x007f

Uint16 compress_normal(const float *normal)
{
	float tmp[3];
	float w;
	Uint32 xbits, ybits;
	Uint16 result;

	result = 0;

	if (tmp[0] < 0.0f)
	{
		result |= XSIGN_MASK;
		tmp[0] = -tmp[0];
	}

	if (tmp[1] < 0.0f)
	{
		result |= YSIGN_MASK;
		tmp[1] = -tmp[1];
	}

	if (tmp[2] < 0.0f)
	{
		result |= ZSIGN_MASK;
		tmp[2] = -tmp[2];
	}

	// project the normal onto the plane that goes through
	// X0=(1,0,0),Y0=(0,1,0),Z0=(0,0,1).

	// on that plane we choose an (projective!) coordinate system
	// such that X0->(0,0), Y0->(126,0), Z0->(0,126),(0,0,0)->Infinity

	// a little slower... old pack was 4 multiplies and 2 adds. 
	// This is 2 multiplies, 2 adds, and a divide....
	w = 126.0f / (tmp[0] + tmp[1] + tmp[2]);
	xbits = (Uint32)(tmp[0] * w);
	ybits = (Uint32)(tmp[1] * w);

	// Now we can be sure that 0<=xp<=126, 0<=yp<=126, 0<=xp+yp<=126

	// however for the sampling we want to transform this triangle 
	// into a rectangle.
	if (xbits >= 64)
	{ 
		xbits = 127 - xbits; 
		ybits = 127 - ybits; 
	}

	// now we that have xp in the range (0,127) and yp in the range (0,63), 
	// we can pack all the bits together
	result |= xbits << 7;
	result |= ybits;

	return result;
}

void uncompress_normal(const Uint16 value, float *normal)
{
	Uint32 x, y;
	float len;

	/**
	 * if we do a straightforward backward transform
	 * we will get points on the plane X0,Y0,Z0
	 * however we need points on a sphere that goes through
	 * these points. Therefore we need to adjust x,y,z so
	 * that x^2+y^2+z^2=1 by normalizing the vector. We have
	 * already precalculated the amount by which we need to
	 * scale, so all we do is a table lookup and a
	 * multiplication
	 * get the x and y bits
	 */

	x = (value & TOP_MASK) >> 7;
	y = value & BOTTOM_MASK;

	// map the numbers back to the triangle (0,0)-(0,126)-(126,0)
	if ((x + y) >= 127)
	{
		x = 127 - x;
		y = 127 - y;
	}

	/**
	 * do the inverse transform and normalization
	 * costs 3 extra multiplies and 2 subtracts. No big deal.
	 */
	normal[0] = x;
	normal[1] = y;
	normal[2] = 126 - x - y;

	// set all the sign bits
	if (value & XSIGN_MASK)
	{
		normal[0] = -normal[0];
	}

	if (value & YSIGN_MASK)
	{
		normal[1] = -normal[1];
	}

	if (value & ZSIGN_MASK)
	{
		normal[2] = -normal[2];
	}

	len = sqrtf(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);

	normal[0] /= len;
	normal[1] /= len;
	normal[2] /= len;
}

