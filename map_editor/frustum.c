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
#include <math.h>
#include "frustum.h"
#include "../bbox_tree.h"
#include "../platform.h"

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

float m_Frustum[8][4];	// only use 6, but mult by 8 is faster

///////////////////////////////// NORMALIZE PLANE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This normalizes a plane (A side) from a given frustum.
/////
///////////////////////////////// NORMALIZE PLANE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void NormalizePlane(float frustum[8][4], int side)
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


///////////////////////////////// CALCULATE FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This extracts our frustum from the projection and modelview matrix.
/////
///////////////////////////////// CALCULATE FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void CalculateFrustum()
{
	float   proj[16];								// This will hold our projection matrix
	float   modl[16];								// This will hold our modelview matrix
	float   clip[16];								// This will hold the clipping planes

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


