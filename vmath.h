#ifdef	TERRAIN
/*!
 * \file
 * \ingroup 	misc_utils
 * \brief 	Handles some math function for vectors.
 */
#ifndef	VMATH_H
#define	VMATH_H
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef	MIN
/*!
 * \ingroup 	misc_utils
 * \brief 	minimum
 * 
 * Calculating minimun of two signed integers.
 * \paran 	a Value one.
 * \param 	b Value two.
 * \retval 	int The minimum of a and b.
 * 
 * \callgraph
 */
static __inline__ int min(int a, int b)
{
	return a < b ? a : b;
}
#endif

#ifndef	MAX
/*!
 * \ingroup 	misc_utils
 * \brief	maximun
 * 
 * Calculating maximum of two signed integers.
 * \paran 	a Value one.
 * \param 	b Value two.
 * \retval 	int The maximum of a and b.
 * 
 * \callgraph
 */
static __inline__ int max(int a, int b)
{
	return a > b ? a : b;
}
#endif

/*!
 * \name Vector item names.
 */
/*! @{ */
typedef enum {
	X = 0,
	Y = 1,
	Z = 2,
	W = 3
} vector_item;
/*! @} */

/*! 
 * VECTOR4 is used for normal calculating using SSE, SSE2 and SSE3.
 */
typedef float VECTOR4[4];

/*! 
 * VECTOR3 is used for normal calculating without SIMD.
 */
typedef float VECTOR3[3];

/*! 
 * SHORT_VEC3 is used for normal calculating using little memory.
 */
typedef short SHORT_VEC3[3];

/*!
 * \ingroup 	misc_utils
 * \brief 	Vector add.
 * 
 * Calculating vector add of two vectors of three floats.
 * \paran 	v1 The return value. 
 * \param 	v2 Value one.
 * \param 	v3 Value two.
 * 
 * \callgraph
 */
static __inline__ void VAdd(VECTOR3 v1, const VECTOR3 v2, const VECTOR3 v3)
{
	v1[X] = v2[X] + v3[X];
	v1[Y] = v2[Y] + v3[Y];
	v1[Z] = v2[Z] + v3[Z];
}

/*!
 * \ingroup 	misc_utils
 * \brief 	Vector add.
 * 
 * Calculating vector add of two vectors of three floats.
 * \paran 	v1 Value one and The return value. 
 * \param 	v2 Value two.
 * 
 * \callgraph
 */
static __inline__ void VAddEq(VECTOR3 v1, const VECTOR3 v2)
{
	v1[X] += v2[X];
	v1[Y] += v2[Y];
	v1[Z] += v2[Z];
}

/*!
 * \ingroup 	misc_utils
 * \brief 	Vector assign.
 * 
 * Assign v2 to v1. Both vectors are three floats.
 * \paran 	v1 The return value. 
 * \param 	v2 Value one.
 * 
 * \callgraph
 */
static __inline__ void VAssign(VECTOR3 v1, const VECTOR3 v2)
{
	memcpy(v1, v2, sizeof(VECTOR3));
}

/*!
 * \ingroup 	misc_utils
 * \brief 	Vector assign.
 * 
 * Assign v2[X..Z] to v1[X..Z] and vw to v1[W]. Vector v2 are three floats,
 * vector v1 are four floats.
 * \paran 	v1 The return value. 
 * \param 	v2 Value one.
 * \param 	vw W-value.
 * 
 * \callgraph
 */
static __inline__ void VAssign4(VECTOR4 v1, const VECTOR3 v2, const float vw)
{
	memcpy(v1, v2, sizeof(VECTOR3));
	v1[W] = vw;
}

/*!
 * \ingroup 	misc_utils
 * \brief 	Vector construction.
 *
 * Makes a vector of thre floats.
 * \paran 	v1 The return value. 
 * \param 	v_x X value of the vector.
 * \param 	v_y Y value of the vector.
 * \param 	v_z Z value of the vector.
 * 
 * \callgraph
 */
static __inline__ void VMake(VECTOR3 v1, const float v_x, const float v_y, const float v_z)
{
	v1[X] = v_x;
	v1[Y] = v_y;
	v1[Z] = v_z;
}

/*!
 * \ingroup 	misc_utils
 * \brief 	Vector normalize.
 * 
 * Normalize vector of three floats.
 * \paran 	v1 The return value. 
 * \param 	v2 Input value.
 * 
 * \callgraph
 */
static __inline__ void Normalize(VECTOR3 v1, const VECTOR3 v2)
{
	float n;
	n = v2[X] * v2[X] + v2[Y] * v2[Y] + v2[Z] * v2[Z];
	n = 1.0f/sqrt(n);
	v1[X] = v2[X] * n;
	v1[Y] = v2[Y] * n;
	v1[Z] = v2[Z] * n;
}

/*!
 * \ingroup 	misc_utils
 * \brief 	Vector float to signed short conversion.
 * 
 * Converts vector of three floats to vector of three signed shorts 
 * by multiplying the floats with 32767.0f and then truncation them.
 * \paran 	v1 The return value. 
 * \param 	v2 Input value.
 * 
 * \callgraph
 */
static __inline__ void VAssignS3(SHORT_VEC3 v1, const VECTOR3 v2)
{
	v1[X] = v2[X]*32767.0f;
	v1[Y] = v2[Y]*32767.0f;
	v1[Z] = v2[Z]*32767.0f;
}

#endif
#endif
