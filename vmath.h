/*!
 * \file
 * \ingroup 	misc_utils
 * \brief 	Handles some math function for vectors.
 */
#ifndef	VMATH_H
#define	VMATH_H
#include <math.h>
#include <string.h>
#ifdef MAP_EDITOR
#include "map_editor/misc.h"
#else
#include "misc.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Compute a rotation matrix for a rotation around the X axis
#define MAT3_ROT_X(mat,angle) \
(mat[0]=1.0,mat[3]=0.0        ,mat[6]=0.0         ,\
 mat[1]=0.0,mat[4]=cosf(angle),mat[7]=-sinf(angle),\
 mat[2]=0.0,mat[5]=-mat[7]    ,mat[8]=mat[4]      )

// Compute a rotation matrix for a rotation around the Y axis
#define MAT3_ROT_Y(mat,angle) \
(mat[0]=cosf(angle),mat[3]=0.0,mat[6]=sinf(angle),\
 mat[1]=0.0        ,mat[4]=1.0,mat[7]=0.0        ,\
 mat[2]=-mat[6]    ,mat[5]=0.0,mat[8]=mat[0]     )

// Compute a rotation matrix for a rotation around the Z axis
#define MAT3_ROT_Z(mat,angle) \
(mat[0]=cosf(angle),mat[3]=-sinf(angle),mat[6]=0.0,\
 mat[1]=-mat[3]    ,mat[4]=mat[0]      ,mat[7]=0.0,\
 mat[2]=0.0        ,mat[5]=0.0         ,mat[8]=1.0)

// Multiply a vector by a rotation matrix
#define MAT3_VECT3_MULT(res,mat,vect) \
((res)[0]=mat[0]*(vect)[0]+mat[3]*(vect)[1]+mat[6]*(vect)[2],\
 (res)[1]=mat[1]*(vect)[0]+mat[4]*(vect)[1]+mat[7]*(vect)[2],\
 (res)[2]=mat[2]*(vect)[0]+mat[5]*(vect)[1]+mat[8]*(vect)[2])

// Multiply two rotation matrices
#define MAT3_MULT(res,mat1,mat2) \
(MAT3_VECT3_MULT(&res[0],mat1,&mat2[0]),\
 MAT3_VECT3_MULT(&res[3],mat1,&mat2[3]),\
 MAT3_VECT3_MULT(&res[6],mat1,&mat2[6]))

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
 * VECTOR4I is used as a selection mask.
 */
typedef int VECTOR4I[4];
/*! 
 * VECTOR3I is used as a selection mask.
 */
typedef int VECTOR3I[3];
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
 * \brief 	Vector min.
 * 
 * Calculating vector min of two vectors of four floats.
 * \paran 	v1 The return value. 
 * \param 	v2 Value one.
 * \param 	v3 Value two.
 * 
 * \callgraph
 */
static __inline__ void VMin4(VECTOR4 v1, const VECTOR4 v2, const VECTOR4 v3)
{
	v1[X] = min2f(v2[X], v3[X]);
	v1[Y] = min2f(v2[Y], v3[Y]);
	v1[Z] = min2f(v2[Z], v3[Z]);
	v1[W] = min2f(v2[W], v3[W]);
}

/*!
 * \ingroup 	misc_utils
 * \brief 	Vector max.
 * 
 * Calculating vector max of two vectors of four floats.
 * \paran 	v1 The return value. 
 * \param 	v2 Value one.
 * \param 	v3 Value two.
 * 
 * \callgraph
 */
static __inline__ void VMax4(VECTOR4 v1, const VECTOR4 v2, const VECTOR4 v3)
{
	v1[X] = max2f(v2[X], v3[X]);
	v1[Y] = max2f(v2[Y], v3[Y]);
	v1[Z] = max2f(v2[Z], v3[Z]);
	v1[W] = max2f(v2[W], v3[W]);
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
 * \brief 	Vector mul.
 * 
 * Calculating vector sub of two vectors of four floats.
 * \paran 	v1 The return value. 
 * \param 	v2 Value one.
 * \param 	v3 Value two.
 * 
 * \callgraph
 */
static __inline__ void VMul4(VECTOR4 v1, const VECTOR4 v2, const VECTOR4 v3)
{
	v1[X] = v2[X] * v3[X];
	v1[Y] = v2[Y] * v3[Y];
	v1[Z] = v2[Z] * v3[Z];
	v1[W] = v2[W] * v3[W];
}

/*!
 * \ingroup 	misc_utils
 * \brief 	Vector sum of four vectors.
 * 
 * Calculating vector sum of four vectors of four floats.
 * \paran 	v1 The return value. 
 * \param 	v2 Value one.
 * \param 	v3 Value two.
 * \param 	v4 Value three.
 * \param 	v5 Value four.
 * 
 * \callgraph
 */
static __inline__ void VSum4x4(VECTOR4 v1, const VECTOR4 v2, const VECTOR4 v3, const VECTOR4 v4, const VECTOR4 v5)
{
	v1[X] = v2[X] + v2[Y] + v2[Z] + v2[W];
	v1[Y] = v3[X] + v3[Y] + v3[Z] + v3[W];
	v1[Z] = v4[X] + v4[Y] + v4[Z] + v4[W];
	v1[W] = v5[X] + v5[Y] + v5[Z] + v5[W];
}

/*!
 * \ingroup 	misc_utils
 * \brief 	Vector select.
 * 
 * Calculating vector select of two vector of three floats from a mask of three ints.
 * \paran 	v1 The return value. 
 * \param 	v2 Value one.
 * \param 	v3 Value two.
 * \param 	mask The selection mask.
 * 
 * \callgraph
 */
static __inline__ void VSelect(VECTOR3 v1, const VECTOR3 v2, const VECTOR3 v3, const VECTOR3I mask)
{
	v1[X] = mask[X] ? v2[X] : v3[X];
	v1[Y] = mask[Y] ? v2[Y] : v3[Y];
	v1[Z] = mask[Z] ? v2[Z] : v3[Z];
}

/*!
 * \ingroup 	misc_utils
 * \brief 	Vector invert select.
 * 
 * Calculating vector select of two vector of three floats from an inverted mask of three ints.
 * \paran 	v1 The return value. 
 * \param 	v2 Value one.
 * \param 	v3 Value two.
 * \param 	mask The selection mask.
 * 
 * \callgraph
 */
static __inline__ void VInvertSelect(VECTOR3 v1, const VECTOR3 v2, const VECTOR3 v3, const VECTOR3I mask)
{
	v1[X] = !mask[X] ? v2[X] : v3[X];
	v1[Y] = !mask[Y] ? v2[Y] : v3[Y];
	v1[Z] = !mask[Z] ? v2[Z] : v3[Z];
}

/*!
 * \ingroup 	misc_utils
 * \brief 	Vector select.
 * 
 * Calculating vector select of two vector of four floats from a mask of four ints.
 * \paran 	v1 The return value. 
 * \param 	v2 Value one.
 * \param 	v3 Value two.
 * \param 	mask The selection mask.
 * 
 * \callgraph
 */
static __inline__ void VSelect4(VECTOR4 v1, const VECTOR4 v2, const VECTOR4 v3, const VECTOR4I mask)
{
	v1[X] = mask[X] == 0 ? v2[X] : v3[X];
	v1[Y] = mask[Y] == 0 ? v2[Y] : v3[Y];
	v1[Z] = mask[Z] == 0 ? v2[Z] : v3[Z];
	v1[W] = mask[W] == 0 ? v2[W] : v3[W];
}

/*!
 * \ingroup 	misc_utils
 * \brief 	Vector invert select.
 * 
 * Calculating vector select of two vector of four floats from an inverted mask of four ints.
 * \paran 	v1 The return value. 
 * \param 	v2 Value one.
 * \param 	v3 Value two.
 * \param 	mask The selection mask.
 * 
 * \callgraph
 */
static __inline__ void VInvertSelect4(VECTOR4 v1, const VECTOR4 v2, const VECTOR4 v3, const VECTOR4I mask)
{
	v1[X] = mask[X] != 0 ? v2[X] : v3[X];
	v1[Y] = mask[Y] != 0 ? v2[Y] : v3[Y];
	v1[Z] = mask[Z] != 0 ? v2[Z] : v3[Z];
	v1[W] = mask[W] != 0 ? v2[W] : v3[W];
}

/*!
 * \ingroup 	misc_utils
 * \brief 	Vector abs.
 * 
 * Calculating vector abs of a vector of three floats.
 * \paran 	v1 The return value. 
 * \param 	v2 Input value.
 * 
 * \callgraph
 */
static __inline__ void VAbs(VECTOR3 v1, const VECTOR3 v2)
{
	v1[X] = fabs(v2[X]);
	v1[Y] = fabs(v2[Y]);
	v1[Z] = fabs(v2[Z]);
}

/*!
 * \ingroup 	misc_utils
 * \brief 	Vector scale.
 * 
 * Calculating vector scale of a vector of three floats and a float value.
 * \paran 	v1 The return value. 
 * \param 	v2 Input value.
 * \param 	v3 Scale value.
 * 
 * \callgraph
 */
static __inline__ void VScale(VECTOR3 v1, const VECTOR3 v2, float v3)
{
	v1[X] = v2[X] * v3;
	v1[Y] = v2[Y] * v3;
	v1[Z] = v2[Z] * v3;
}

/*!
 * \ingroup 	misc_utils
 * \brief 	Vector cmp le.
 * 
 * Calculating vector cmp le of two vectors of three floats.
 * \paran 	v1 The return value. 
 * \param 	v2 Input value.
 * 
 * \callgraph
 */
static __inline__ int VCmpLE(const VECTOR3 v1, const VECTOR3 v2)
{
	int ret;
	ret = 0;
	ret += v1[X] < v2[X] ? 1 : 0;
	ret += v1[Y] < v2[Y] ? 2 : 0;
	ret += v1[Z] < v2[Z] ? 4 : 0;
	return ret;
}

/*!
 * \ingroup 	misc_utils
 * \brief 	Vector cmp le.
 * 
 * Calculating vector cmp le of two vectors of four floats.
 * \paran 	v1 The return value. 
 * \param 	v2 Input value.
 * 
 * \callgraph
 */
static __inline__ int VCmpLE4(const VECTOR4 v1, const VECTOR4 v2)
{
	int ret;
	ret = 0;
	ret += v1[X] < v2[X] ? 1 : 0;
	ret += v1[Y] < v2[Y] ? 2 : 0;
	ret += v1[Z] < v2[Z] ? 4 : 0;
	ret += v1[W] < v2[W] ? 8 : 0;
	return ret;
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
 * \brief 	Vector construction.
 *
 * Makes a vector of three ints.
 * \paran 	v1 The return value. 
 * \param 	v_x X value of the vector.
 * \param 	v_y Y value of the vector.
 * \param 	v_z Z value of the vector.
 * 
 * \callgraph
 */
static __inline__ void VMakeI(VECTOR3I v1, const int v_x, const int v_y, const int v_z)
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
 * \brief 	Vector fill.
 *
 * Makes a vector of four floats.
 * \paran 	v1 The return value. 
 * \param 	f X, Y, Z and W value of the vector.
 * 
 * \callgraph
 */
static __inline__ void VFill4(VECTOR4 v1, float f)
{
	v1[X] = f;
	v1[Y] = f;
	v1[Z] = f;
	v1[W] = f;
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
	v1[X] = (short) (v2[X]*32767.0f);
	v1[Y] = (short) (v2[Y]*32767.0f);
	v1[Z] = (short) (v2[Z]*32767.0f);
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

/*!
 * \ingroup 	misc_utils
 * \brief 	Vector dot product.
 * 
 * Calculating vector dot product of two vectors of four floats.
 * \param 	v2 Value one.
 * \param 	v3 Value two.
 * \retval 	float The vector dot product. 
 * 
 * \callgraph
 */
static __inline__ float VDot4(const VECTOR4 v2, const VECTOR4 v3)
{
	return v2[X]*v3[X] + v2[Y]*v3[Y] + v2[Z]*v3[Z] + v2[W]*v3[W];	
}

static __inline__ float VExtract(const VECTOR3 v1, int number)
{
	return v1[number];
}

static __inline__ float VExtract4(const VECTOR4 v1, int number)
{
	return v1[number];
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

#ifdef __cplusplus
} // extern "C"
#endif

#endif
