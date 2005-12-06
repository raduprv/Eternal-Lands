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
//#include "SDL_opengl.h"
#ifdef MAP_EDITOR2
 #include "../map_editor2/global.h"
#else
 #include "global.h"
#endif
#ifdef	NEW_FRUSTUM
#include <string.h>
#include "bbox_tree.h"
#endif

struct Sphere
{
	float xPos, yPos, zPos, radius;						// We want to hold a XYZ position and radius
	unsigned char  r, g, b;								// These will store the color of the sphere
};


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

#ifndef	NEW_FRUSTUM
// Like above, instead of saying a number for the ABC and D of the plane, we
// want to be more descriptive.
enum PlaneData
{
	A = 0,				// The X value of the plane's normal
	B = 1,				// The Y value of the plane's normal
	C = 2,				// The Z value of the plane's normal
	D = 3				// The distance the plane is from the origin
};
#endif

float m_Frustum[8][4];	// only use 6, but mult by 8 is faster
#ifdef	NEW_FRUSTUM
FRUSTUM main_frustum;
FRUSTUM reflection_frustum;
FRUSTUM shadow_frustum;
FRUSTUM selection_frustum;
FRUSTUM* current_frustum;
unsigned int current_frustum_size;
double reflection_clip_planes[5][4];
#endif
///////////////////////////////// NORMALIZE PLANE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This normalizes a plane (A side) from a given frustum.
/////
///////////////////////////////// NORMALIZE PLANE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

#ifndef	NEW_FRUSTUM
void NormalizePlane(float frustum[6][4], int side)
{
	// Here we calculate the magnitude of the normal to the plane (point A B C)
	// Remember that (A, B, C) is that same thing as the normal's (X, Y, Z).
	// To calculate magnitude you use the equation:  magnitude = sqrt( x^2 + y^2 + z^2)
	float magnitude = (float)sqrt( frustum[side][A] * frustum[side][A] +
								   frustum[side][B] * frustum[side][B] +
								   frustum[side][C] * frustum[side][C] );

	// Then we divide the plane's values by it's magnitude.
	// This makes it easier to work with.
	frustum[side][A] /= magnitude;
	frustum[side][B] /= magnitude;
	frustum[side][C] /= magnitude;
	frustum[side][D] /= magnitude;
}
#endif

#ifdef	NEW_FRUSTUM
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
	plane->mask[0] = plane->plane[A] < 0.0f ? 0 : 1;
	plane->mask[1] = plane->plane[B] < 0.0f ? 0 : 1;
	plane->mask[2] = plane->plane[C] < 0.0f ? 0 : 1;
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
	VSub(t3, p2, p3);
	VCross(t1, t2, t3);
	Normalize(t0, t1);
	
	plane[A] = t0[X];
	plane[B] = t0[Y];
	plane[C] = t0[Z];
	plane[D] = -VDot(t0, p1);
}

void enable_reflection_clip_planes()
{
	glEnable(GL_CLIP_PLANE0);
#ifdef	USE_EXTRA_CLIP_PLANES
	glEnable(GL_CLIP_PLANE1);
	glEnable(GL_CLIP_PLANE2);
	glEnable(GL_CLIP_PLANE3);
	glEnable(GL_CLIP_PLANE4);
#endif
	glClipPlane(GL_CLIP_PLANE0, reflection_clip_planes[0]);
#ifdef	USE_EXTRA_CLIP_PLANES
	glClipPlane(GL_CLIP_PLANE1, reflection_clip_planes[1]);
	glClipPlane(GL_CLIP_PLANE2, reflection_clip_planes[2]);
	glClipPlane(GL_CLIP_PLANE3, reflection_clip_planes[3]);
	glClipPlane(GL_CLIP_PLANE4, reflection_clip_planes[4]);
#endif
}

void disable_reflection_clip_planes()
{
	glDisable(GL_CLIP_PLANE0);
#ifdef	USE_EXTRA_CLIP_PLANES
	glDisable(GL_CLIP_PLANE1);
	glDisable(GL_CLIP_PLANE2);
	glDisable(GL_CLIP_PLANE3);
	glDisable(GL_CLIP_PLANE4);
#endif
}

void set_current_frustum(unsigned int intersect_type)
{
	switch (intersect_type)
	{
		case ITERSECTION_TYPE_DEFAULT:
			current_frustum_size = 6;
			current_frustum = &main_frustum;
			break;
		case ITERSECTION_TYPE_SHADOW:
			current_frustum_size = 6;
			current_frustum = &shadow_frustum;
			break;
		case ITERSECTION_TYPE_REFLECTION:
			current_frustum_size = 9;
			current_frustum = &reflection_frustum;
			break;
		case ITERSECTION_TYPE_SELECTION:
			current_frustum_size = 6;
			current_frustum = &selection_frustum;
			break;			
		default:
			current_frustum_size = 0;
			break;
	}
}

int aabb_in_frustum(AABBOX *bbox)
{
	unsigned int i;
	float m, nx, ny, nz;
	
	for(i = 0; i < current_frustum_size; i++)
	{
		nx = !current_frustum[0][i].mask[0] ? bbox->bbmin[X] :  bbox->bbmax[X];
		ny = !current_frustum[0][i].mask[1] ? bbox->bbmin[Y] :  bbox->bbmax[Y];
		nz = !current_frustum[0][i].mask[2] ? bbox->bbmin[Z] :  bbox->bbmax[Z];
		m = (	current_frustum[0][i].plane[A] * nx + 
			current_frustum[0][i].plane[B] * ny + 
			current_frustum[0][i].plane[C] * nz);
		if (m < -current_frustum[0][i].plane[D]) return 0;
	}

	return 1;
}

void set_selection_matrix()
{
	MATRIX4x4 proj;								// This will hold our projection matrix
	MATRIX4x4 modl;								// This will hold our modelview matrix
	MATRIX4x4 clip;								// This will hold the clipping planes
	GLint viewport[4];
	unsigned int cur_intersect_type;

	// glGetFloatv() is used to extract information about our OpenGL world.
	// Below, we pass in GL_PROJECTION_MATRIX to abstract our projection matrix.
	// It then stores the matrix into an array of [16].
	glGetFloatv(GL_PROJECTION_MATRIX, proj);
	glGetIntegerv(GL_VIEWPORT, viewport);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPickMatrix(mouse_x, window_height-mouse_y, 1.0, 1.0, viewport);
	glMultMatrixf(proj);
	glMatrixMode(GL_MODELVIEW);
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

	cur_intersect_type = get_cur_intersect_type(main_bbox_tree);
	set_cur_intersect_type(main_bbox_tree, ITERSECTION_TYPE_SELECTION);
	main_bbox_tree->intersect[ITERSECTION_TYPE_SELECTION].intersect_update_needed = 1;

	calculate_frustum_from_clip_matrix(selection_frustum, clip);
	check_bbox_tree(main_bbox_tree, &selection_frustum, 63);
	
	set_cur_intersect_type(main_bbox_tree, cur_intersect_type);
}


void calculate_reflection_frustum(unsigned int num, float water_height)
{
	MATRIX4x4 proj;
	MATRIX4x4 modl;
	MATRIX4x4 clip;
	MATRIX4x4 inv;
	VECTOR3 pos, p1, p2, p3, p4;
	float x_min, x_max, y_min, y_max, x_scaled, y_scaled;
	unsigned int cur_intersect_type, i, l, start, stop, x, y;

	if (main_bbox_tree->intersect[ITERSECTION_TYPE_REFLECTION].intersect_update_needed == 0) return;
		
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, water_height);
	glScalef(1.0f, 1.0f, -1.0f);
	glTranslatef(0.0f, 0.0f, -water_height);
	glGetFloatv(GL_MODELVIEW_MATRIX, modl);
	glPopMatrix();
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
	set_cur_intersect_type(main_bbox_tree, ITERSECTION_TYPE_DEFAULT);

	VMInvert(inv, clip);
	
	x_min = BOUND_HUGE;
	x_max = -BOUND_HUGE;
	y_min = BOUND_HUGE;
	y_max = -BOUND_HUGE;
	get_intersect_start_stop(main_bbox_tree, TYPE_REFLECTIV_WATER, &start, &stop);
	for (i = start; i < stop; i++)
	{
		l = get_intersect_item_ID(main_bbox_tree, i);
		x = get_terrain_x(l);
		y = get_terrain_y(l);
		y_scaled = y*3.0f;
		x_scaled = x*3.0f;
		x_min = min2f(x_min, x_scaled);
		x_max = max2f(x_max, x_scaled+3.0f);
		y_min = min2f(y_min, y_scaled);
		y_max = max2f(y_max, y_scaled+3.0f);
	}
	
	set_cur_intersect_type(main_bbox_tree, ITERSECTION_TYPE_REFLECTION);
	calculate_frustum_from_clip_matrix(reflection_frustum, clip);

	pos[0] = inv[3]/inv[15];
	pos[1] = inv[7]/inv[15];
	pos[2] = inv[11]/inv[15];
	p1[X] = x_min;
	p1[Y] = y_min;
	p1[Z] = water_height;
	p2[X] = x_min;
	p2[Y] = y_max;
	p2[Z] = water_height;
	p3[X] = x_max;
	p3[Y] = y_min;
	p3[Z] = water_height;
	p4[X] = x_max;
	p4[Y] = y_max;
	p4[Z] = water_height;
	calc_plane(reflection_frustum[4].plane, p2, p1, p3);
	calc_plane(reflection_frustum[5].plane, pos, p2, p1);
	calc_plane(reflection_frustum[6].plane, pos, p3, p4);
	calc_plane(reflection_frustum[7].plane, pos, p4, p2);
	calc_plane(reflection_frustum[8].plane, pos, p1, p3);
	calc_plane_mask(&reflection_frustum[4]);
	calc_plane_mask(&reflection_frustum[5]);
	calc_plane_mask(&reflection_frustum[6]);
	calc_plane_mask(&reflection_frustum[7]);
	calc_plane_mask(&reflection_frustum[8]);

	reflection_clip_planes[0][A] = reflection_frustum[4].plane[A];
	reflection_clip_planes[0][B] = reflection_frustum[4].plane[B];
	reflection_clip_planes[0][C] = reflection_frustum[4].plane[C];
	reflection_clip_planes[0][D] = reflection_frustum[4].plane[D];
	reflection_clip_planes[1][A] = reflection_frustum[5].plane[A];
	reflection_clip_planes[1][B] = reflection_frustum[5].plane[B];
	reflection_clip_planes[1][C] = reflection_frustum[5].plane[C];
	reflection_clip_planes[1][D] = reflection_frustum[5].plane[D];
	reflection_clip_planes[2][A] = reflection_frustum[6].plane[A];
	reflection_clip_planes[2][B] = reflection_frustum[6].plane[B];
	reflection_clip_planes[2][C] = reflection_frustum[6].plane[C];
	reflection_clip_planes[2][D] = reflection_frustum[6].plane[D];
	reflection_clip_planes[3][A] = reflection_frustum[7].plane[A];
	reflection_clip_planes[3][B] = reflection_frustum[7].plane[B];
	reflection_clip_planes[3][C] = reflection_frustum[7].plane[C];
	reflection_clip_planes[3][D] = reflection_frustum[7].plane[D];
	reflection_clip_planes[4][A] = reflection_frustum[8].plane[A];
	reflection_clip_planes[4][B] = reflection_frustum[8].plane[B];
	reflection_clip_planes[4][C] = reflection_frustum[8].plane[C];
	reflection_clip_planes[4][D] = reflection_frustum[8].plane[D];
	check_bbox_tree(main_bbox_tree, &reflection_frustum, 511);
	set_cur_intersect_type(main_bbox_tree, cur_intersect_type);	
}

void calculate_shadow_frustum()
{
	MATRIX4x4 proj;								// This will hold our projection matrix
	MATRIX4x4 modl;								// This will hold our modelview matrix
	MATRIX4x4 clip;								// This will hold the clipping planes
	unsigned int cur_intersect_type;

	main_bbox_tree->intersect[ITERSECTION_TYPE_SHADOW].intersect_update_needed = 1;

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
	set_cur_intersect_type(main_bbox_tree, ITERSECTION_TYPE_SHADOW);	
	check_bbox_tree(main_bbox_tree, &shadow_frustum, 63);
	set_cur_intersect_type(main_bbox_tree, cur_intersect_type);
}

#endif
///////////////////////////////// CALCULATE FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This extracts our frustum from the projection and modelview matrix.
/////
///////////////////////////////// CALCULATE FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void CalculateFrustum()
{
#ifdef	NEW_FRUSTUM
	MATRIX4x4 proj;								// This will hold our projection matrix
	MATRIX4x4 modl;								// This will hold our modelview matrix
	MATRIX4x4 clip;								// This will hold the clipping planes
	
	if (main_bbox_tree->intersect[ITERSECTION_TYPE_DEFAULT].intersect_update_needed == 0) return;
#else
	float   proj[16];								// This will hold our projection matrix
	float   modl[16];								// This will hold our modelview matrix
	float   clip[16];								// This will hold the clipping planes

#endif

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

#ifndef	NEW_FRUSTUM
	// Now we actually want to get the sides of the frustum.  To do this we take
	// the clipping planes we received above and extract the sides from them.

	// This will extract the RIGHT side of the frustum
	m_Frustum[RIGHT][A] = clip[ 3] - clip[ 0];
	m_Frustum[RIGHT][B] = clip[ 7] - clip[ 4];
	m_Frustum[RIGHT][C] = clip[11] - clip[ 8];
	m_Frustum[RIGHT][D] = clip[15] - clip[12];

	// Now that we have a normal (A,B,C) and a distance (D) to the plane,
	// we want to normalize that normal and distance.

	// Normalize the RIGHT side
	NormalizePlane(m_Frustum, RIGHT);

	// This will extract the LEFT side of the frustum
	m_Frustum[LEFT][A] = clip[ 3] + clip[ 0];
	m_Frustum[LEFT][B] = clip[ 7] + clip[ 4];
	m_Frustum[LEFT][C] = clip[11] + clip[ 8];
	m_Frustum[LEFT][D] = clip[15] + clip[12];

	// Normalize the LEFT side
	NormalizePlane(m_Frustum, LEFT);

	// This will extract the BOTTOM side of the frustum
	m_Frustum[BOTTOM][A] = clip[ 3] + clip[ 1];
	m_Frustum[BOTTOM][B] = clip[ 7] + clip[ 5];
	m_Frustum[BOTTOM][C] = clip[11] + clip[ 9];
	m_Frustum[BOTTOM][D] = clip[15] + clip[13];

	// Normalize the BOTTOM side
	NormalizePlane(m_Frustum, BOTTOM);

	// This will extract the TOP side of the frustum
	m_Frustum[TOP][A] = clip[ 3] - clip[ 1];
	m_Frustum[TOP][B] = clip[ 7] - clip[ 5];
	m_Frustum[TOP][C] = clip[11] - clip[ 9];
	m_Frustum[TOP][D] = clip[15] - clip[13];

	// Normalize the TOP side
	NormalizePlane(m_Frustum, TOP);

	// This will extract the BACK side of the frustum
	m_Frustum[BACK][A] = clip[ 3] - clip[ 2];
	m_Frustum[BACK][B] = clip[ 7] - clip[ 6];
	m_Frustum[BACK][C] = clip[11] - clip[10];
	m_Frustum[BACK][D] = clip[15] - clip[14];

	// Normalize the BACK side
	NormalizePlane(m_Frustum, BACK);

	// This will extract the FRONT side of the frustum
	m_Frustum[FRONT][A] = clip[ 3] + clip[ 2];
	m_Frustum[FRONT][B] = clip[ 7] + clip[ 6];
	m_Frustum[FRONT][C] = clip[11] + clip[10];
	m_Frustum[FRONT][D] = clip[15] + clip[14];

	// Normalize the FRONT side
	NormalizePlane(m_Frustum, FRONT);
#else
	calculate_frustum_from_clip_matrix(main_frustum, clip);
	check_bbox_tree(main_bbox_tree, &main_frustum, 63);
#endif
}

// The code below will allow us to make checks within the frustum.  For example,
// if we want to see if a point, a sphere, or a cube lies inside of the frustum.
// Because all of our planes point INWARDS (The normals are all pointing inside the frustum)
// we then can assume that if a point is in FRONT of all of the planes, it's inside.

///////////////////////////////// POINT IN FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This determines if a point is inside of the frustum
/////
///////////////////////////////// POINT IN FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

#ifndef	NEW_FRUSTUM
int PointInFrustum( float x, float y, float z )
{
	// If you remember the plane equation (A*x + B*y + C*z + D = 0), then the rest
	// of this code should be quite obvious and easy to figure out yourself.
	// In case don't know the plane equation, it might be a good idea to look
	// at our Plane Collision tutorial at www.GameTutorials.com in OpenGL Tutorials.
	// I will briefly go over it here.  (A,B,C) is the (X,Y,Z) of the normal to the plane.
	// They are the same thing... but just called ABC because you don't want to say:
	// (x*x + y*y + z*z + d = 0).  That would be wrong, so they substitute them.
	// the (x, y, z) in the equation is the point that you are testing.  The D is
	// The distance the plane is from the origin.  The equation ends with "= 0" because
	// that is 1 when the point (x, y, z) is ON the plane.  When the point is NOT on
	// the plane, it is either a negative number (the point is behind the plane) or a
	// positive number (the point is in front of the plane).  We want to check if the point
	// is in front of the plane, so all we have to do is go through each point and make
	// sure the plane equation goes out to a positive number on each side of the frustum.
	// The result (be it positive or negative) is the distance the point is front the plane.

	// Go through all the sides of the frustum
	int i = 0;
	for(; i < 6; i++ )
	{
		// Calculate the plane equation and check if the point is behind a side of the frustum
		if(m_Frustum[i][A] * x + m_Frustum[i][B] * y + m_Frustum[i][C] * z + m_Frustum[i][D] <= 0.0)
		{
			// The point was behind a side, so it ISN'T in the frustum
			return 0;
		}
	}

	// The point was inside of the frustum (In front of ALL the sides of the frustum)
	return 1;
}


///////////////////////////////// SPHERE IN FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This determines if a sphere is inside of our frustum by it's center and radius.
/////
///////////////////////////////// SPHERE IN FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

int SphereInFrustum( float x, float y, float z, float radius )
{
	// Now this function is almost identical to the PointInFrustum(), except we
	// now have to deal with a radius around the point.  The point is the center of
	// the radius.  So, the point might be outside of the frustum, but it doesn't
	// mean that the rest of the sphere is.  It could be half and half.  So instead of
	// checking if it's less than 0, we need to add on the radius to that.  Say the
	// equation produced -2, which means the center of the sphere is the distance of
	// 2 behind the plane.  Well, what if the radius was 5?  The sphere is still inside,
	// so we would say, if(-2 < -5) then we are outside.  In that case it's 0,
	// so we are inside of the frustum, but a distance of 3.  This is reflected below.

	// Go through all the sides of the frustum
	float	tradius=-radius-1.0;
	int i = 0;
	for(; i < 6; i++ )
	{
		// If the center of the sphere is farther away from the plane than the radius
		if( m_Frustum[i][A] * x + m_Frustum[i][B] * y + m_Frustum[i][C] * z + m_Frustum[i][D] <= tradius )
		{
			// The distance was greater than the radius so the sphere is outside of the frustum
			return 0;
		}
	}

	// The sphere was inside of the frustum!
	return 1;
}


///////////////////////////////// CUBE IN FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This determines if a cube is in or around our frustum by it's center and 1/2 it's length
/////
///////////////////////////////// CUBE IN FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

int CubeInFrustum(float x, float y, float z, float x_size, float y_size, float z_size)
{
	// This test is a bit more work, but not too much more complicated.
	// Basically, what is going on is, that we are given the center of the cube,
	// and half the length.  Think of it like a radius.  Then we checking each point
	// in the cube and seeing if it is inside the frustum.  If a point is found in front
	// of a side, then we skip to the next side.  If we get to a plane that does NOT have
	// a point in front of it, then it will return 0.

	// *Note* - This will sometimes say that a cube is inside the frustum when it isn't.
	// This happens when all the corners of the bounding box are not behind any one plane.
	// This is rare and shouldn't effect the overall rendering speed.
	int i = 0;
	for(; i < 6; i++ )
	{
		if(m_Frustum[i][A] * (x-x_size) + m_Frustum[i][B] * (y-y_size) + m_Frustum[i][C] * (z-z_size) + m_Frustum[i][D] > -1.0)
		   continue;
		if(m_Frustum[i][A] * (x+x_size) + m_Frustum[i][B] * (y-y_size) + m_Frustum[i][C] * (z-z_size) + m_Frustum[i][D] > -1.0)
		   continue;
		if(m_Frustum[i][A] * (x-x_size) + m_Frustum[i][B] * (y+y_size) + m_Frustum[i][C] * (z-z_size) + m_Frustum[i][D] > -1.0)
		   continue;
		if(m_Frustum[i][A] * (x+x_size) + m_Frustum[i][B] * (y+y_size) + m_Frustum[i][C] * (z-z_size) + m_Frustum[i][D] > -1.0)
		   continue;
		if(m_Frustum[i][A] * (x-x_size) + m_Frustum[i][B] * (y-y_size) + m_Frustum[i][C] * (z+z_size) + m_Frustum[i][D] > -1.0)
		   continue;
		if(m_Frustum[i][A] * (x+x_size) + m_Frustum[i][B] * (y-y_size) + m_Frustum[i][C] * (z+z_size) + m_Frustum[i][D] > -1.0)
		   continue;
		if(m_Frustum[i][A] * (x-x_size) + m_Frustum[i][B] * (y+y_size) + m_Frustum[i][C] * (z+z_size) + m_Frustum[i][D] > -1.0)
		   continue;
		if(m_Frustum[i][A] * (x+x_size) + m_Frustum[i][B] * (y+y_size) + m_Frustum[i][C] * (z+z_size) + m_Frustum[i][D] > -1.0)
		   continue;

		// If we get here, it isn't in the frustum
		return 0;
	}

	return 1;
}


int check_tile_in_frustrum(float x,float y)
{
	return(SphereInFrustum(x+1.5f, y+1.5f, 0, 2.449f));
	//if(SphereInFrustum(x+1.5f, y+1.5f, 0, 2.449f))return 1;
	//else return 0;
}
#endif


