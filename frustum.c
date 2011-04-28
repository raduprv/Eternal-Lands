//***********************************************************************//
//																		 //
//		- "Talk to me like I'm a 3 year old!" Programming Lessons -		 //
//                                                                       //
//		$Author:		DigiBen		digiben@gametutorials.com			 //
//																		 //
//		$Program:		Frustum Culling									 //
//																		 //
//		$Description:	Demonstrates checking if shapes are in view		 //
//																		 //
//		$Date:			8/28/01											 //
//																		 //
//***********************************************************************//
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "bbox_tree.h"
#include "shadows.h"
#include "elconfig.h"
#include "gl_init.h"
#include "tiles.h"

// We create an enum of the sides so we don't have to call each side 0 or 1.
// This way it makes it more understandable and readable when dealing with frustum sides.
enum FrustumSide
{
	RIGHT	= 0,		// The RIGHT side of the frustum
	LEFT	= 1,		// The LEFT	 side of the frustum
	BOTTOM	= 2,		// The BOTTOM side of the frustum
	TOP		= 3,		// The TOP side of the frustum
	BACK	= 4,		// The BACK	side of the frustum
	FRONT	= 5			// The FRONT side of the frustum
};

float m_Frustum[8][4];	// only use 6, but mult by 8 is faster
FRUSTUM main_frustum;
FRUSTUM reflection_frustum;
FRUSTUM shadow_frustum;
FRUSTUM* current_frustum;
unsigned int current_frustum_size;
double reflection_clip_planes[5][4];
PLANE* reflection_portals;

///////////////////////////////// NORMALIZE PLANE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This normalizes a plane (A side) from a given frustum.
/////
///////////////////////////////// NORMALIZE PLANE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

/*
 * Normalizes a given Plane.
 */
static __inline__ void normalize_plane(VECTOR4 plane)
{
	// Here we calculate the magnitude of the normal to the plane (point A B C)
	// Remember that (A, B, C) is that same thing as the normal's (X, Y, Z).
	// To calculate magnitude you use the equation:  magnitude = sqrt( x^2 + y^2 + z^2)
	float magnitude;
	
	magnitude = sqrt(plane[A]*plane[A] + plane[B]*plane[B] + plane[C]*plane[C]);

	// Then we divide the plane's values by it's magnitude.
	// This makes it easier to work with.
	plane[A] /= magnitude;
	plane[B] /= magnitude;
	plane[C] /= magnitude;
	plane[D] /= magnitude;
}

/*
 * Calculates the mask of given Plane used for check_aabb_in_frustum.
 */
static __inline__ void calc_plane_mask(PLANE* plane)
{
	plane->mask[0] = plane->plane[A] < 0.0f ? 0x00000000 : 0xFFFFFFFF;
	plane->mask[1] = plane->plane[B] < 0.0f ? 0x00000000 : 0xFFFFFFFF;
	plane->mask[2] = plane->plane[C] < 0.0f ? 0x00000000 : 0xFFFFFFFF;
}

/*
 * Inverts the given matrix.
 * \param	r The Result.
 * \param	m The matrix to invert.
 */
static __inline__ void VMInvert(MATRIX4x4 r, MATRIX4x4 m)
{
	float d00, d01, d02, d03;
	float d10, d11, d12, d13;
	float d20, d21, d22, d23;
	float d30, d31, d32, d33;
	float m00, m01, m02, m03;
	float m10, m11, m12, m13;
	float m20, m21, m22, m23;
	float m30, m31, m32, m33;
	float D;

	m00 = m[0];
	m01 = m[4];
	m02 = m[8];
	m03 = m[12];
	m10 = m[1];
	m11 = m[5];
	m12 = m[9];
	m13 = m[13];
	m20 = m[2];
	m21 = m[6];
	m22 = m[10];
	m23 = m[14];
	m30 = m[3];
	m31 = m[7];
	m32 = m[11];
	m33 = m[15];

	d00 = m11*m22*m33 + m12*m23*m31 + m13*m21*m32 - m31*m22*m13 - m32*m23*m11 - m33*m21*m12;
	d01 = m10*m22*m33 + m12*m23*m30 + m13*m20*m32 - m30*m22*m13 - m32*m23*m10 - m33*m20*m12;
	d02 = m10*m21*m33 + m11*m23*m30 + m13*m20*m31 - m30*m21*m13 - m31*m23*m10 - m33*m20*m11;
	d03 = m10*m21*m32 + m11*m22*m30 + m12*m20*m31 - m30*m21*m12 - m31*m22*m10 - m32*m20*m11;

	d10 = m01*m22*m33 + m02*m23*m31 + m03*m21*m32 - m31*m22*m03 - m32*m23*m01 - m33*m21*m02;
	d11 = m00*m22*m33 + m02*m23*m30 + m03*m20*m32 - m30*m22*m03 - m32*m23*m00 - m33*m20*m02;
	d12 = m00*m21*m33 + m01*m23*m30 + m03*m20*m31 - m30*m21*m03 - m31*m23*m00 - m33*m20*m01;
	d13 = m00*m21*m32 + m01*m22*m30 + m02*m20*m31 - m30*m21*m02 - m31*m22*m00 - m32*m20*m01;

	d20 = m01*m12*m33 + m02*m13*m31 + m03*m11*m32 - m31*m12*m03 - m32*m13*m01 - m33*m11*m02;
	d21 = m00*m12*m33 + m02*m13*m30 + m03*m10*m32 - m30*m12*m03 - m32*m13*m00 - m33*m10*m02;
	d22 = m00*m11*m33 + m01*m13*m30 + m03*m10*m31 - m30*m11*m03 - m31*m13*m00 - m33*m10*m01;
	d23 = m00*m11*m32 + m01*m12*m30 + m02*m10*m31 - m30*m11*m02 - m31*m12*m00 - m32*m10*m01;

	d30 = m01*m12*m23 + m02*m13*m21 + m03*m11*m22 - m21*m12*m03 - m22*m13*m01 - m23*m11*m02;
	d31 = m00*m12*m23 + m02*m13*m20 + m03*m10*m22 - m20*m12*m03 - m22*m13*m00 - m23*m10*m02;
	d32 = m00*m11*m23 + m01*m13*m20 + m03*m10*m21 - m20*m11*m03 - m21*m13*m00 - m23*m10*m01;
	d33 = m00*m11*m22 + m01*m12*m20 + m02*m10*m21 - m20*m11*m02 - m21*m12*m00 - m22*m10*m01;

	D = m00*d00 - m01*d01 + m02*d02 - m03*d03;

	r[0] =  d00/D;
	r[1] = -d10/D;
	r[2] =  d20/D;
	r[3] = -d30/D;
	r[4] = -d01/D;
	r[5] =  d11/D;
	r[6] = -d21/D;
	r[7] =  d31/D;
	r[8] =  d02/D;
	r[9] = -d12/D;
	r[10] =  d22/D;
	r[11] = -d32/D;
	r[12] = -d03/D;
	r[13] =  d13/D;
	r[14] = -d23/D;
	r[15] =  d33/D;
}

static __inline__ void calculate_frustum_from_clip_matrix(FRUSTUM frustum, MATRIX4x4 clip)
{
	frustum[RIGHT].plane[A] = clip[ 3] - clip[ 0];
	frustum[RIGHT].plane[B] = clip[ 7] - clip[ 4];
	frustum[RIGHT].plane[C] = clip[11] - clip[ 8];
	frustum[RIGHT].plane[D] = clip[15] - clip[12];

	// This will extract the LEFT side of the frustum
	frustum[LEFT].plane[A] = clip[ 3] + clip[ 0];
	frustum[LEFT].plane[B] = clip[ 7] + clip[ 4];
	frustum[LEFT].plane[C] = clip[11] + clip[ 8];
	frustum[LEFT].plane[D] = clip[15] + clip[12];

	// This will extract the BOTTOM side of the frustum
	frustum[BOTTOM].plane[A] = clip[ 3] + clip[ 1];
	frustum[BOTTOM].plane[B] = clip[ 7] + clip[ 5];
	frustum[BOTTOM].plane[C] = clip[11] + clip[ 9];
	frustum[BOTTOM].plane[D] = clip[15] + clip[13];

	// This will extract the TOP side of the frustum
	frustum[TOP].plane[A] = clip[ 3] - clip[ 1];
	frustum[TOP].plane[B] = clip[ 7] - clip[ 5];
	frustum[TOP].plane[C] = clip[11] - clip[ 9];
	frustum[TOP].plane[D] = clip[15] - clip[13];

	// This will extract the BACK side of the frustum
	frustum[BACK].plane[A] = clip[ 3] - clip[ 2];
	frustum[BACK].plane[B] = clip[ 7] - clip[ 6];
	frustum[BACK].plane[C] = clip[11] - clip[10];
	frustum[BACK].plane[D] = clip[15] - clip[14];

	// This will extract the FRONT side of the frustum
	frustum[FRONT].plane[A] = clip[ 3] + clip[ 2];
	frustum[FRONT].plane[B] = clip[ 7] + clip[ 6];
	frustum[FRONT].plane[C] = clip[11] + clip[10];
	frustum[FRONT].plane[D] = clip[15] + clip[14];

	// Now that we have a normal (A,B,C) and a distance (D) to the plane,
	// we want to normalize that normal and distance.

	// Normalize the RIGHT side
	normalize_plane(frustum[RIGHT].plane);

	// Normalize the LEFT side
	normalize_plane(frustum[LEFT].plane);
 
 	// Normalize the BOTTOM side
	normalize_plane(frustum[BOTTOM].plane);
 
 	// Normalize the TOP side
	normalize_plane(frustum[TOP].plane);

	// Normalize the BACK side
	normalize_plane(frustum[BACK].plane);

 	// Normalize the FRONT side
	normalize_plane(frustum[FRONT].plane);
	
	calc_plane_mask(&frustum[RIGHT]);
	calc_plane_mask(&frustum[LEFT]);
	calc_plane_mask(&frustum[BOTTOM]);
	calc_plane_mask(&frustum[TOP]);
	calc_plane_mask(&frustum[BACK]);
	calc_plane_mask(&frustum[FRONT]);
}

static __inline__ void calc_plane(VECTOR4 plane, const VECTOR3 p1, const VECTOR3 p2, const VECTOR3 p3)
{
	VECTOR3 t0, t1, t2, t3;

	VSub(t2, p1, p2);
	VSub(t3, p1, p3);
	VCross(t1, t2, t3);
	Normalize(t0, t1);

	VAssign4(plane, t0, -VDot(t0, p1));
}

void enable_reflection_clip_planes()
{
	MATRIX4x4 proj;
	MATRIX4x4 modl;
	MATRIX4x4 clip;
	MATRIX4x4 inv;
	MATRIX4x4 suffix;
	MATRIX4x4 new_proj;
	VECTOR4 oplane;
	VECTOR4 cplane;
	float tmp;
	int i;

	glGetFloatv(GL_MODELVIEW_MATRIX, modl);
	glGetFloatv(GL_PROJECTION_MATRIX, proj);
	
	clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
	clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
	clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
	clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];

	clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
	clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
	clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
	clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];

	clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
	clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
	clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
	clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];

	clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
	clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
	clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
	clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];

	VMInvert(inv, clip);

	for (i = 0; i < 4; i++)
	{
		oplane[i] = reflection_clip_planes[0][i];
	}

	cplane[ 0] = oplane[ 0] * inv[ 0] + oplane[ 1] * inv[ 4] + oplane[ 2] * inv[ 8] + oplane[ 3] * inv[12];
	cplane[ 1] = oplane[ 0] * inv[ 1] + oplane[ 1] * inv[ 5] + oplane[ 2] * inv[ 9] + oplane[ 3] * inv[13];
	cplane[ 2] = oplane[ 0] * inv[ 2] + oplane[ 1] * inv[ 6] + oplane[ 2] * inv[10] + oplane[ 3] * inv[14];
	cplane[ 3] = oplane[ 0] * inv[ 3] + oplane[ 1] * inv[ 7] + oplane[ 2] * inv[11] + oplane[ 3] * inv[15];

	tmp = fabsf(cplane[2]); // normalize such that depth is not scaled

	cplane[0] /= tmp;
	cplane[1] /= tmp;
	cplane[2] /= tmp;
	cplane[3] /= tmp;

	cplane[3] -= 1.0f;

	if (cplane[2] < 0.0f)
	{
		cplane[0] *= -1.0f;
		cplane[1] *= -1.0f;
		cplane[2] *= -1.0f;
		cplane[3] *= -1.0f;
	}

	suffix[ 0] = 1.0f;
	suffix[ 1] = 0.0f;
	suffix[ 2] = 0.0f;
	suffix[ 3] = 0.0f;

	suffix[ 4] = 0.0f;
	suffix[ 5] = 1.0f;
	suffix[ 6] = 0.0f;
	suffix[ 7] = 0.0f;

	suffix[ 8] = 0.0f;
	suffix[ 9] = 0.0f;
	suffix[10] = 1.0f;
	suffix[11] = 0.0f;

	suffix[12] = 0.0f;
	suffix[13] = 0.0f;
	suffix[14] = 0.0f;
	suffix[15] = 1.0f;

	suffix[ 2] = cplane[0];
	suffix[ 6] = cplane[1];
	suffix[10] = cplane[2];
	suffix[14] = cplane[3];

	new_proj[ 0] = proj[ 0] * suffix[ 0] + proj[ 1] * suffix[ 4] + proj[ 2] * suffix[ 8] + proj[ 3] * suffix[12];
	new_proj[ 1] = proj[ 0] * suffix[ 1] + proj[ 1] * suffix[ 5] + proj[ 2] * suffix[ 9] + proj[ 3] * suffix[13];
	new_proj[ 2] = proj[ 0] * suffix[ 2] + proj[ 1] * suffix[ 6] + proj[ 2] * suffix[10] + proj[ 3] * suffix[14];
	new_proj[ 3] = proj[ 0] * suffix[ 3] + proj[ 1] * suffix[ 7] + proj[ 2] * suffix[11] + proj[ 3] * suffix[15];

	new_proj[ 4] = proj[ 4] * suffix[ 0] + proj[ 5] * suffix[ 4] + proj[ 6] * suffix[ 8] + proj[ 7] * suffix[12];
	new_proj[ 5] = proj[ 4] * suffix[ 1] + proj[ 5] * suffix[ 5] + proj[ 6] * suffix[ 9] + proj[ 7] * suffix[13];
	new_proj[ 6] = proj[ 4] * suffix[ 2] + proj[ 5] * suffix[ 6] + proj[ 6] * suffix[10] + proj[ 7] * suffix[14];
	new_proj[ 7] = proj[ 4] * suffix[ 3] + proj[ 5] * suffix[ 7] + proj[ 6] * suffix[11] + proj[ 7] * suffix[15];

	new_proj[ 8] = proj[ 8] * suffix[ 0] + proj[ 9] * suffix[ 4] + proj[10] * suffix[ 8] + proj[11] * suffix[12];
	new_proj[ 9] = proj[ 8] * suffix[ 1] + proj[ 9] * suffix[ 5] + proj[10] * suffix[ 9] + proj[11] * suffix[13];
	new_proj[10] = proj[ 8] * suffix[ 2] + proj[ 9] * suffix[ 6] + proj[10] * suffix[10] + proj[11] * suffix[14];
	new_proj[11] = proj[ 8] * suffix[ 3] + proj[ 9] * suffix[ 7] + proj[10] * suffix[11] + proj[11] * suffix[15];

	new_proj[12] = proj[12] * suffix[ 0] + proj[13] * suffix[ 4] + proj[14] * suffix[ 8] + proj[15] * suffix[12];
	new_proj[13] = proj[12] * suffix[ 1] + proj[13] * suffix[ 5] + proj[14] * suffix[ 9] + proj[15] * suffix[13];
	new_proj[14] = proj[12] * suffix[ 2] + proj[13] * suffix[ 6] + proj[14] * suffix[10] + proj[15] * suffix[14];
	new_proj[15] = proj[12] * suffix[ 3] + proj[13] * suffix[ 7] + proj[14] * suffix[11] + proj[15] * suffix[15];

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(new_proj);
	glMatrixMode(GL_MODELVIEW);

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void disable_reflection_clip_planes()
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void set_current_frustum(unsigned int intersect_type)
{
	switch (intersect_type)
	{
		case INTERSECTION_TYPE_DEFAULT:
			current_frustum_size = 6;
			current_frustum = &main_frustum;
			break;
		case INTERSECTION_TYPE_SHADOW:
			current_frustum_size = 6;
			current_frustum = &shadow_frustum;
			break;
		case INTERSECTION_TYPE_REFLECTION:
			current_frustum_size = 7;
			current_frustum = &reflection_frustum;
			break;
		default:
			current_frustum_size = 0;
			break;
	}
}

int aabb_in_frustum(const AABBOX bbox)
{
	unsigned int i;
	float m, nx, ny, nz;
	
	for(i = 0; i < current_frustum_size; i++)
	{
		nx = !current_frustum[0][i].mask[0] ? bbox.bbmin[X] :  bbox.bbmax[X];
		ny = !current_frustum[0][i].mask[1] ? bbox.bbmin[Y] :  bbox.bbmax[Y];
		nz = !current_frustum[0][i].mask[2] ? bbox.bbmin[Z] :  bbox.bbmax[Z];
		m = (	current_frustum[0][i].plane[A] * nx + 
			current_frustum[0][i].plane[B] * ny + 
			current_frustum[0][i].plane[C] * nz);
		if (m < -current_frustum[0][i].plane[D]) return 0;
	}

	return 1;
}

void init_reflection_portals(int size)
{
	reflection_portals = realloc(reflection_portals, size * 4 * sizeof(PLANE));
}

void calculate_reflection_frustum(float water_height)
{
	MATRIX4x4 proj;
	MATRIX4x4 modl;
	MATRIX4x4 clip;
	MATRIX4x4 inv;
	float x_min, x_max, y_min, y_max, x_scaled, y_scaled;
	unsigned int i, j, l, start, stop;
	unsigned int cur_intersect_type;
	VECTOR3 p1, p2, p3, p4, pos;

	if (main_bbox_tree->intersect[INTERSECTION_TYPE_REFLECTION].intersect_update_needed == 0) return;

	glGetFloatv(GL_MODELVIEW_MATRIX, modl);
	glGetFloatv(GL_PROJECTION_MATRIX, proj);
	
	clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
	clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
	clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
	clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];

	clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
	clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
	clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
	clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];

	clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
	clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
	clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
	clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];

	clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
	clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
	clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
	clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];

	cur_intersect_type = get_cur_intersect_type(main_bbox_tree);

	set_cur_intersect_type(main_bbox_tree, INTERSECTION_TYPE_REFLECTION);
	calculate_frustum_from_clip_matrix(reflection_frustum, clip);
	reflection_frustum[6].plane[A] = 0.0;
	reflection_frustum[6].plane[B] = 0.0;
	reflection_frustum[6].plane[C] = 1.0;
	reflection_frustum[6].plane[D] = -water_height;
	reflection_frustum[BACK].plane[D] -= (reflection_frustum[BACK].plane[A]*reflection_frustum[BACK].plane[A]*(far_plane-far_reflection_plane) +
										  reflection_frustum[BACK].plane[B]*reflection_frustum[BACK].plane[B]*(far_plane-far_reflection_plane) +
										  reflection_frustum[BACK].plane[C]*reflection_frustum[BACK].plane[C]*(far_plane-far_reflection_plane));
	calc_plane_mask(&reflection_frustum[6]);
	reflection_clip_planes[0][A] = reflection_frustum[6].plane[A];
	reflection_clip_planes[0][B] = reflection_frustum[6].plane[B];
	reflection_clip_planes[0][C] = reflection_frustum[6].plane[C];
	reflection_clip_planes[0][D] = reflection_frustum[6].plane[D];

	set_frustum(main_bbox_tree, reflection_frustum, 127);
	check_bbox_tree(main_bbox_tree);

	VMInvert(inv, modl);
	VMake(pos, inv[3] / inv[15], inv[7] / inv[15], inv[11] / inv[15]);

	set_cur_intersect_type(main_bbox_tree, INTERSECTION_TYPE_DEFAULT);
	j = 0;	
	get_intersect_start_stop(main_bbox_tree, TYPE_REFLECTIV_WATER, &start, &stop);
	for (i = start; i < stop; i++)
	{
		unsigned int tx, ty;
		l = get_intersect_item_ID(main_bbox_tree, i);
		tx = get_terrain_x(l);
		ty = get_terrain_y(l);
		x_scaled = tx * 3.0f;
		y_scaled = ty * 3.0f;

		x_min = x_scaled;
		x_max = x_scaled + 3.0f;
		y_min = y_scaled;
		y_max = y_scaled + 3.0f;

		if (tx == 0) x_min -= water_tiles_extension;
		else if (tx == tile_map_size_x-1) x_max += water_tiles_extension;
		if (ty == 0) y_min -= water_tiles_extension;
		else if (ty == tile_map_size_y-1) y_max += water_tiles_extension;

		VMake(p1, x_min, y_max, water_height);
		VMake(p2, x_max, y_max, water_height);
		VMake(p3, x_max, y_min, water_height);
		VMake(p4, x_min, y_min, water_height);

		calc_plane(reflection_portals[j * 4 + 0].plane, pos, p2, p1);
		calc_plane(reflection_portals[j * 4 + 1].plane, pos, p3, p2);
		calc_plane(reflection_portals[j * 4 + 2].plane, pos, p4, p3);
		calc_plane(reflection_portals[j * 4 + 3].plane, pos, p1, p4);
		calc_plane_mask(&reflection_portals[j * 4 + 0]);
		calc_plane_mask(&reflection_portals[j * 4 + 1]);
		calc_plane_mask(&reflection_portals[j * 4 + 2]);
		calc_plane_mask(&reflection_portals[j * 4 + 3]);
		j++;
	}
	set_cur_intersect_type(main_bbox_tree, INTERSECTION_TYPE_REFLECTION);

	reflection_portal_check(main_bbox_tree, reflection_portals, j);

	set_cur_intersect_type(main_bbox_tree, cur_intersect_type);
}

void calculate_shadow_frustum()
{
	MATRIX4x4 proj;								// This will hold our projection matrix
	MATRIX4x4 modl;								// This will hold our modelview matrix
	MATRIX4x4 clip;								// This will hold the clipping planes
	VECTOR3	ld;
	unsigned int cur_intersect_type;

	if (main_bbox_tree->intersect[INTERSECTION_TYPE_SHADOW].intersect_update_needed == 0) return;

	// glGetFloatv() is used to extract information about our OpenGL world.
	// Below, we pass in GL_PROJECTION_MATRIX to abstract our projection matrix.
	// It then stores the matrix into an array of [16].
	glGetFloatv(GL_PROJECTION_MATRIX, proj);

	// By passing in GL_MODELVIEW_MATRIX, we can abstract our model view matrix.
	// This also stores it in an array of [16].
	glGetFloatv(GL_MODELVIEW_MATRIX, modl);

	// Now that we have our modelview and projection matrix, if we combine these 2 matrices,
	// it will give us our clipping planes.  To combine 2 matrices, we multiply them.

	clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
	clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
	clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
	clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];

	clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
	clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
	clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
	clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];

	clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
	clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
	clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
	clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];

	clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
	clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
	clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
	clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];

	calculate_frustum_from_clip_matrix(shadow_frustum, clip);
	cur_intersect_type = get_cur_intersect_type(main_bbox_tree);
	set_cur_intersect_type(main_bbox_tree, INTERSECTION_TYPE_SHADOW);
	VMake(ld, sun_position[X], sun_position[Y], sun_position[Z]);
	set_frustum(main_bbox_tree, shadow_frustum, 63);
	check_bbox_tree_shadow(main_bbox_tree, shadow_frustum, 63, main_frustum, 63, ld);
	set_cur_intersect_type(main_bbox_tree, cur_intersect_type);
}

void calculate_light_frustum(double* modl, double* proj)
{
	MATRIX4x4 clip;
	FRUSTUM frustum;
	
	clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
	clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
	clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
	clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];

	clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
	clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
	clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
	clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];

	clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
	clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
	clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
	clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];

	clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
	clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
	clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
	clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];
	
	calculate_frustum_from_clip_matrix(frustum, clip);
	set_frustum(main_bbox_tree, frustum, 63);
}
///////////////////////////////// CALCULATE FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This extracts our frustum from the projection and modelview matrix.
/////
///////////////////////////////// CALCULATE FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void CalculateFrustum()
{
	MATRIX4x4 proj;								// This will hold our projection matrix
	MATRIX4x4 modl;								// This will hold our modelview matrix
	MATRIX4x4 clip;								// This will hold the clipping planes
	unsigned int cur_intersect_type;
	
	if (main_bbox_tree->intersect[INTERSECTION_TYPE_DEFAULT].intersect_update_needed == 0) return;

	// glGetFloatv() is used to extract information about our OpenGL world.
	// Below, we pass in GL_PROJECTION_MATRIX to abstract our projection matrix.
	// It then stores the matrix into an array of [16].
	glGetFloatv( GL_PROJECTION_MATRIX, proj );

	// By passing in GL_MODELVIEW_MATRIX, we can abstract our model view matrix.
	// This also stores it in an array of [16].
	glGetFloatv( GL_MODELVIEW_MATRIX, modl );

	// Now that we have our modelview and projection matrix, if we combine these 2 matrices,
	// it will give us our clipping planes.  To combine 2 matrices, we multiply them.

	clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
	clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
	clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
	clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];

	clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
	clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
	clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
	clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];

	clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
	clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
	clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
	clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];

	clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
	clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
	clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
	clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];

	calculate_frustum_from_clip_matrix(main_frustum, clip);
	cur_intersect_type = get_cur_intersect_type(main_bbox_tree);
	set_cur_intersect_type(main_bbox_tree, INTERSECTION_TYPE_DEFAULT);
	set_frustum(main_bbox_tree, main_frustum, 63);
	check_bbox_tree(main_bbox_tree);
	set_cur_intersect_type(main_bbox_tree, cur_intersect_type);
}
