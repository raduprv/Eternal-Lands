/*!
 * \file
 * \ingroup 	misc_utils
 * \brief 	Handles some math function for vectors.
 */
#ifndef	VMATH_H
#define	VMATH_H
#include <string.h>
#ifdef MAP_EDITOR2
#include "../map_editor2/misc.h"
#else
#include "misc.h"
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
 * VECTOR4 is used for normal, bounding box etc. calculating using SSE, SSE2 and SSE3.
 */
typedef float VECTOR4[4];
/*! 
 * VECTOR3 is used for normal, bounding box etc. calculating without SIMD.
 */
typedef float VECTOR3[3];
/*! 
 * VECTOR3D is the same as VECTOR3 but with double precision.
 */
typedef double VECTOR3D[3];
/*! 
 * SHORT_VEC3 is used for normal calculating using little memory.
 */
typedef short SHORT_VEC3[3];
/*! 
 * TEXTCOORD2 is used for texture coordinates.
 */
typedef float TEXTCOORD2[2];
/*!
 * MATRIX4x4 is used for translation and rotation.
 */
typedef float MATRIX4x4[16];
/*! 
 * MATRIX4x4D is the same as MATRIX4x4 but with double precision.
 */
typedef double MATRIX4x4D[16];
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
 * \brief 	Vector min.
 * 
 * Calculating vector min of two vectors of three floats.
 * \paran 	v1 The return value. 
 * \param 	v2 Value one.
 * \param 	v3 Value two.
 * 
 * \callgraph
 */
static __inline__ void VMin(VECTOR3 v1, const VECTOR3 v2, const VECTOR3 v3)
{
	v1[X] = min2f(v2[X], v3[X]);
	v1[Y] = min2f(v2[Y], v3[Y]);
	v1[Z] = min2f(v2[Z], v3[Z]);
}

/*!
 * \ingroup 	misc_utils
 * \brief 	Vector max.
 * 
 * Calculating vector max of two vectors of three floats.
 * \paran 	v1 The return value. 
 * \param 	v2 Value one.
 * \param 	v3 Value two.
 * 
 * \callgraph
 */
static __inline__ void VMax(VECTOR3 v1, const VECTOR3 v2, const VECTOR3 v3)
{
	v1[X] = max2f(v2[X], v3[X]);
	v1[Y] = max2f(v2[Y], v3[Y]);
	v1[Z] = max2f(v2[Z], v3[Z]);
}

/*!
 * \ingroup 	misc_utils
 * \brief 	Vector sub.
 * 
 * Calculating vector sub of two vectors of three floats.
 * \paran 	v1 The return value. 
 * \param 	v2 Value one.
 * \param 	v3 Value two.
 * 
 * \callgraph
 */
static __inline__ void VSub(VECTOR3 v1, const VECTOR3 v2, const VECTOR3 v3)
{
	v1[X] = v2[X] - v3[X];
	v1[Y] = v2[Y] - v3[Y];
	v1[Z] = v2[Z] - v3[Z];
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
 * \brief 	Vector fill.
 *
 * Makes a vector of thre floats.
 * \paran 	v1 The return value. 
 * \param 	f X, Y and Z value of the vector.
 * 
 * \callgraph
 */
static __inline__ void VFill(VECTOR3 v1, float f)
{
	v1[X] = f;
	v1[Y] = f;
	v1[Z] = f;
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
	n = sqrt(n);
	v1[X] = v2[X] / n;
	v1[Y] = v2[Y] / n;
	v1[Z] = v2[Z] / n;
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

/*!
 * \ingroup 	misc_utils
 * \brief 	Vector cross product.
 * 
 * Calculating vector cross product of two vectors of three floats.
 * \paran 	v1 The return value. 
 * \param 	v2 Value one.
 * \param 	v3 Value two.
 * 
 * \callgraph
 */
static __inline__ void VCross(VECTOR3 v1, const VECTOR3 v2, const VECTOR3 v3)
{
	VECTOR3 tmp;

	tmp[X] = v2[Y] * v3[Z] - v2[Z] * v3[Y];
	tmp[Y] = v2[Z] * v3[X] - v2[X] * v3[Z];
	tmp[Z] = v2[X] * v3[Y] - v2[Y] * v3[X];

	VAssign(v1, tmp);
}

/*!
 * \ingroup 	misc_utils
 * \brief 	Vector dot product.
 * 
 * Calculating vector dot product of two vectors of three floats.
 * \param 	v2 Value one.
 * \param 	v3 Value two.
 * \retval 	float The vector dot product. 
 * 
 * \callgraph
 */
static __inline__ float VDot(const VECTOR3 v2, const VECTOR3 v3)
{
	return v2[X]*v3[X] + v2[Y]*v3[Y] + v2[Z]*v3[Z];	
}

static __inline__ void calc_rotation_and_translation_matrix(MATRIX4x4 matrix, float trans_x, float trans_y, float trans_z, float rot_x, float rot_y, float rot_z)
{
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(trans_x, trans_y, trans_z);
	glRotatef(rot_z, 0.0f, 0.0f, 1.0f);
	glRotatef(rot_x, 1.0f, 0.0f, 0.0f);
	glRotatef(rot_y, 0.0f, 1.0f, 0.0f);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	glPopMatrix();
}

void calculate_Light_Matrix(int useBodyVec, double nearDist,
	const VECTOR3D lightDir, const MATRIX4x4D ModelViewMatrix,
	const MATRIX4x4D ProjectionMatrix, MATRIX4x4D lightView,
	MATRIX4x4D lightProjection);
#endif
