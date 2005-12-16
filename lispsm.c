#ifdef	USE_LISPSM
#include <float.h>
#include <malloc.h>
#include "global.h"

typedef VECTOR3D VECTOR3x8D[8];

//Plane defined through normal vector n and distance to origin d
typedef struct
{
	VECTOR3D n;
	float d;
} Plane;

//a dynamic array of planes
typedef struct
{
	Plane* plane;
	int size;
} VecPlane;

//a dynamic array 3d points
typedef struct
{
	VECTOR3D* points;
	int size;
} VecPoint;

//a dynamic array of point list each point list is a polygon
typedef struct
{
	VecPoint* poly;
	int size;
} Object;

//Axis-Aligned Bounding Box defined through the two extreme points
typedef struct
{
	VECTOR3D bbmin;
	VECTOR3D bbmax;
} AABBOXD;

/****/
static const Object OBJECT_NULL = {NULL, 0};
static const VecPoint VECPOINT_NULL = {NULL, 0};
static const VecPlane VECPLANE_NULL = {NULL, 0};

const double PI = M_PI;
const double PI_2 = M_PI/2;
const double PI_180 = M_PI/180;
static const VECTOR3D ZERO = {0.0, 0.0, 0.0};
static const VECTOR3D UNIT_X = {1.0, 0.0, 0.0};
static const VECTOR3D UNIT_Y = {0.0, 1.0, 0.0};
static const VECTOR3D UNIT_Z = {0.0, 0.0, 1.0};

static const MATRIX4x4D IDENTITY = {1.0, 0.0, 0.0, 0.0, 
				0.0, 1.0, 0.0, 0.0,
				0.0, 0.0, 1.0, 0.0,
				0.0, 0.0, 0.0, 1.0};

static __inline__ double max2d(const double a, const double b) 
{
	return a > b ? a : b;
}

static __inline__ double min2d(const double a, const double b) 
{
	return a < b ? a : b;
}

static __inline__ void clamp(double* value, const double min, const double max)
{
	*value = min2d(max2d(*value, min), max);
}

static __inline__ double coTan(const double vIn)
{
	return -tan(vIn+PI_2);
}

static __inline__ double relativeEpsilon(const double a, const double epsilon)
{
	return max2d(abs(a*epsilon), epsilon);
}

static __inline__ int alike(const double a, const double b, const double epsilon)
{
	double relEps;
	
	if (a == b) return 1;
	else
	{
		relEps = relativeEpsilon(a,epsilon);
		return (a-relEps <= b) && (b <= a+relEps);
	}
}

static __inline__ int alikeVECTOR3(const VECTOR3D a, const VECTOR3D b,
	const double epsilon)
{
	return alike(a[0], b[0], epsilon) && alike(a[1], b[1], epsilon) &&
		alike(a[2], b[2], epsilon);
}

static __inline__ void linCombVECTOR3(VECTOR3D result, const VECTOR3D pos,
	const VECTOR3D dir, const double t)
{
	result[0] = pos[0] + t*dir[0];
	result[1] = pos[1] + t*dir[1];
	result[2] = pos[2] + t*dir[2];
}

static __inline__ double squaredLength(const VECTOR3D vec)
{
	return vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2];
}

static __inline__ void normalize(VECTOR3D vec) 
{
	double len;
	
	len = 1.0/sqrt(squaredLength(vec));
	vec[0] *= len;
	vec[1] *= len;
	vec[2] *= len;
}

static __inline__ void copyVECTOR3(VECTOR3D result, const VECTOR3D input)
{
	result[0] = input[0]; 
	result[1] = input[1];
	result[2] = input[2];
}

static __inline__ void addVECTOR3(VECTOR3D result, const VECTOR3D a,
	const VECTOR3D b)
{
	result[0] = a[0] + b[0];
	result[1] = a[1] + b[1];
	result[2] = a[2] + b[2];
}

static __inline__ void diffVECTOR3(VECTOR3D result, const VECTOR3D a,
	const VECTOR3D b)
{
	result[0] = a[0] - b[0];
	result[1] = a[1] - b[1];
	result[2] = a[2] - b[2];
}

static __inline__ void copyVECTOR3Values(VECTOR3D result, const double x,
	const double y, const double z)
{
	result[0] = x; 
	result[1] = y;
	result[2] = z;
}

static __inline__ void copyMatrix(MATRIX4x4D result, const MATRIX4x4D input)
{
	if (input != result) memcpy(result, input, sizeof(MATRIX4x4D));
}

static __inline__ void planesSetSize(VecPlane* p, const int size)
{
	if (p != NULL)
	{
		if (size != p->size)
		{
			p->plane = (Plane*)realloc(p->plane, size*sizeof(Plane));
			p->size = size;
		}
	}
}

static __inline__ void emptyVecPlane(VecPlane* p)
{
	planesSetSize(p, 0);
}

static __inline__ void vecPointSetSize(VecPoint* poly, const int size)
{
	if (poly != NULL)
	{
		if (size != poly->size)
		{
			poly->points = (VECTOR3D*)realloc(poly->points, size*sizeof(VECTOR3D));
			poly->size = size;
		}
	}
}

static __inline__ void emptyVecPoint(VecPoint* poly)
{
	vecPointSetSize(poly, 0);
}

static __inline__ void append2VecPoint(VecPoint* poly, const VECTOR3D p)
{
	int size;
	
	if (poly != NULL)
	{
		size = poly->size;
		vecPointSetSize(poly, size+1);
		copyVECTOR3(poly->points[size], p);
	}
}

static __inline__ void copyVecPoint(VecPoint* poly, const VecPoint poly2)
{
	int i;
	
	if (poly != NULL)
	{
		vecPointSetSize(poly, poly2.size);
		for (i = 0; i < poly2.size; i++)
			copyVECTOR3(poly->points[i], poly2.points[i]);
	}
}

static __inline__ void swapVecPoint(VecPoint* poly1, VecPoint* poly2)
{
	VECTOR3D* points;
	int size;
	
	if ((poly1 != NULL) && (poly2 != NULL) && (poly1 != poly2))
	{
		points = poly1->points;
		poly1->points = poly2->points;
		poly2->points = points;
		size = poly1->size;
		poly1->size = poly2->size;
		poly2->size = size;
	}
}

static __inline__ void objectSetSize(Object* obj, const int size)
{
	int i;
	
	if (obj != NULL)
	{
		if (size != obj->size)
		{
			//dispose if shrinking
			for (i = size; i < obj->size; i++)
				emptyVecPoint(&(obj->poly[i]));

			//allocate new place
			obj->poly = (VecPoint*)realloc(obj->poly, size*sizeof(VecPoint));
			
			//initialize new place
			for(i = obj->size; i < size; i++)
				obj->poly[i] = VECPOINT_NULL;
			obj->size = size;
		}
	}
}

static __inline__ void emptyObject(Object* obj)
{
	objectSetSize(obj, 0);
}

static __inline__ void copyObject(Object* obj, const Object objIn)
{
	int i;
	
	if (obj != NULL)
	{
		objectSetSize(obj, objIn.size);
		for (i = 0; i < objIn.size; i++)
			copyVecPoint(&(obj->poly[i]), objIn.poly[i]);
	}
}

static __inline__ void append2Object(Object* obj, const VecPoint poly)
{
	int size;
	
	if (obj != NULL)
	{
		size = obj->size;
		objectSetSize(obj, size+1);
		copyVecPoint(&(obj->poly[size]), poly);
	}
}

static __inline__ void convObject2VecPoint(VecPoint* points, const Object obj)
{
	VecPoint* p;
	int i, j;
	
	if (points != NULL)
	{
		emptyVecPoint(points);
		
		for (i = 0; i < obj.size; i++)
		{
			p = &(obj.poly[i]);
			
			for (j = 0; j < p->size; j++)
				append2VecPoint(points, p->points[j]);
		}
	}
}

static __inline__ void calcAABoxPoints(VECTOR3x8D points, const AABBOXD b)
{
    //generate 8 corners of the box
	copyVECTOR3Values(points[0], b.bbmin[0], b.bbmin[1], b.bbmin[2]);//     7+------+6
	copyVECTOR3Values(points[1], b.bbmax[0], b.bbmin[1], b.bbmin[2]);//     /|     /|
	copyVECTOR3Values(points[2], b.bbmax[0], b.bbmax[1], b.bbmin[2]);//    / |    / |
	copyVECTOR3Values(points[3], b.bbmin[0], b.bbmax[1], b.bbmin[2]);//   / 4+---/--+5
	copyVECTOR3Values(points[4], b.bbmin[0], b.bbmin[1], b.bbmax[2]);// 3+------+2 /    y   z
	copyVECTOR3Values(points[5], b.bbmax[0], b.bbmin[1], b.bbmax[2]);//  | /    | /     |  /
	copyVECTOR3Values(points[6], b.bbmax[0], b.bbmax[1], b.bbmax[2]);//  |/     |/      |/
	copyVECTOR3Values(points[7], b.bbmin[0], b.bbmax[1], b.bbmax[2]);// 0+------+1      *---x
}

static __inline__ void calcAABoxPlanes(VecPlane* planes, const AABBOXD b)
{
	Plane *p;
	
	if (planes != NULL)
	{
		planesSetSize(planes,6);
		
		//bottom plane
		p = &(planes->plane[0]);
		copyVECTOR3Values(p->n, 0.0, -1.0, 0);
		p->d = abs(b.bbmin[1]);

		//top plane
		p = &(planes->plane[1]);
		copyVECTOR3Values(p->n, 0.0, 1.0, 0.0);
		p->d = abs(b.bbmax[1]);

		//left plane
		p = &(planes->plane[2]);
		copyVECTOR3Values(p->n, -1.0, 0.0, 0.0);
		p->d = abs(b.bbmin[0]);

		//right plane
		p = &(planes->plane[3]);
		copyVECTOR3Values(p->n, 1.0, 0.0, 0.0);
		p->d = abs(b.bbmax[0]);

		//back plane
		p = &(planes->plane[4]);
		copyVECTOR3Values(p->n, 0.0, 0.0, -1.0);
		p->d = abs(b.bbmin[2]);

		//front plane
		p = &(planes->plane[5]);
		copyVECTOR3Values(p->n, 0.0, 0.0, 1.0);
		p->d = abs(b.bbmax[2]);
	}
}

static __inline__ void copyNormalize(VECTOR3D vec, const VECTOR3D input)
{
	copyVECTOR3(vec, input);
	normalize(vec);
}

/*	| a1 a2 |
	| b1 b2 | calculate the determinent of a 2x2 matrix*/
static __inline__ double det2x2(const double a1, const double a2,
	const double b1, const double b2)
{
	return a1*b2 - b1*a2;
}

/*	| a1 a2 a3 |
	| b1 b2 b3 |
	| c1 c2 c3 | calculate the determinent of a 3x3 matrix*/
static __inline__ double det3x3(const double a1, const double a2,
	const double a3, const double b1, const double b2, const double b3,
	 const double c1, const double c2, const double c3)
{
	return a1*det2x2(b2,b3,c2,c3) - b1*det2x2(a2,a3,c2,c3) +
		c1*det2x2(a2,a3,b2,b3);
}

static __inline__ void cross(VECTOR3D result, const VECTOR3D a, const VECTOR3D b)
{
	result[0] =  det2x2(a[1], b[1], a[2], b[2]);
	result[1] = -det2x2(a[0], b[0], a[2], b[2]);
	result[2] =  det2x2(a[0], b[0], a[1], b[1]);
}

static __inline__ double dot(const VECTOR3D a, const VECTOR3D b)
{
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

static __inline__ void look(MATRIX4x4D output, const VECTOR3D pos,
	const VECTOR3D dir, const VECTOR3D up)
{
	VECTOR3D dirN;
	VECTOR3D upN;
	VECTOR3D lftN;

	cross(lftN, dir, up);
	normalize(lftN);

	cross(upN, lftN, dir);
	normalize(upN);
	copyNormalize(dirN, dir);

	output[ 0] = lftN[0];
	output[ 1] = upN[0];
	output[ 2] = -dirN[0];
	output[ 3] = 0.0;

	output[ 4] = lftN[1];
	output[ 5] = upN[1];
	output[ 6] = -dirN[1];
	output[ 7] = 0.0;

	output[ 8] = lftN[2];
	output[ 9] = upN[2];
	output[10] = -dirN[2];
	output[11] = 0.0;

	output[12] = -dot(lftN, pos);
	output[13] = -dot(upN, pos);
	output[14] = dot(dirN, pos);
	output[15] = 1.0;
}

static __inline__ void makeScaleMtx(MATRIX4x4D output, const double x,
	const double y, const double z)
{
	output[ 0] = x;
	output[ 1] = 0.0;
	output[ 2] = 0.0;
	output[ 3] = 0.0;

	output[ 4] = 0.0;
	output[ 5] = y;
	output[ 6] = 0.0;
	output[ 7] = 0.0;

	output[ 8] = 0.0;
	output[ 9] = 0.0;
	output[10] = z;
	output[11] = 0.0;

	output[12] = 0.0;
	output[13] = 0.0;
	output[14] = 0.0;
	output[15] = 1.0;
}

static __inline__ void scaleTranslateToFit(MATRIX4x4D output,
	const VECTOR3D vMin, const VECTOR3D vMax)
{
	output[ 0] = 2/(vMax[0]-vMin[0]);
	output[ 4] = 0;
	output[ 8] = 0;
	output[12] = -(vMax[0]+vMin[0])/(vMax[0]-vMin[0]);

	output[ 1] = 0;
	output[ 5] = 2/(vMax[1]-vMin[1]);
	output[ 9] = 0;
	output[13] = -(vMax[1]+vMin[1])/(vMax[1]-vMin[1]);

	output[ 2] = 0;
	output[ 6] = 0;
	output[10] = 2/(vMax[2]-vMin[2]);
	output[14] = -(vMax[2]+vMin[2])/(vMax[2]-vMin[2]);

	output[ 3] = 0;
	output[ 7] = 0;
	output[11] = 0;
	output[15] = 1;
}

static __inline__ void perspectiveRad(MATRIX4x4D output, const double vFovy,
	const double vAspect, const double vNearDis, const double vFarDis)
{
	double f, dif;
	
	f = coTan(vFovy/2.0);
	dif = 1.0/(vNearDis-vFarDis);

	output[ 0] = f/vAspect;
	output[ 4] = 0.0;
	output[ 8] = 0.0;
	output[12] = 0.0;

	output[ 1] = 0.0;
	output[ 5] = f;
	output[ 9] = 0.0;
	output[13] = 0.0;

	output[ 2] = 0.0;
	output[ 6] = 0.0;
	output[10] = (vFarDis+vNearDis)*dif;
	output[14] = 2.0*vFarDis*vNearDis*dif;

	output[ 3] = 0.0;
	output[ 7] = 0.0;
	output[11] = -1.0;
	output[15] = 0.0;
}

static __inline__ void perspectiveDeg(MATRIX4x4D output, const double vFovy,
	const double vAspect, const double vNearDis, const double vFarDis)
{
	perspectiveRad(output, vFovy*PI_180, vAspect, vNearDis, vFarDis);
}

static __inline__ void invert(MATRIX4x4D output, const MATRIX4x4D i)
{
	double a11, a21, a31, a41, a12, a22, a32, a42;
	double a13, a23, a33, a43, a14, a24, a34, a44;
	double det, oodet;

	a11 =  det3x3(i[5],i[6],i[7],i[9],i[10],i[11],i[13],i[14],i[15]);
	a21 = -det3x3(i[1],i[2],i[3],i[9],i[10],i[11],i[13],i[14],i[15]);
	a31 =  det3x3(i[1],i[2],i[3],i[5],i[6],i[7],i[13],i[14],i[15]);
	a41 = -det3x3(i[1],i[2],i[3],i[5],i[6],i[7],i[9],i[10],i[11]);

	a12 = -det3x3(i[4],i[6],i[7],i[8],i[10],i[11],i[12],i[14],i[15]);
	a22 =  det3x3(i[0],i[2],i[3],i[8],i[10],i[11],i[12],i[14],i[15]);
	a32 = -det3x3(i[0],i[2],i[3],i[4],i[6],i[7],i[12],i[14],i[15]);
	a42 =  det3x3(i[0],i[2],i[3],i[4],i[6],i[7],i[8],i[10],i[11]);

	a13 =  det3x3(i[4],i[5],i[7],i[8],i[9],i[11],i[12],i[13],i[15]);
	a23 = -det3x3(i[0],i[1],i[3],i[8],i[9],i[11],i[12],i[13],i[15]);
	a33 =  det3x3(i[0],i[1],i[3],i[4],i[5],i[7],i[12],i[13],i[15]);
	a43 = -det3x3(i[0],i[1],i[3],i[4],i[5],i[7],i[8],i[9],i[11]);

	a14 = -det3x3(i[4],i[5],i[6],i[8],i[9],i[10],i[12],i[13],i[14]);
	a24 =  det3x3(i[0],i[1],i[2],i[8],i[9],i[10],i[12],i[13],i[14]);
	a34 = -det3x3(i[0],i[1],i[2],i[4],i[5],i[6],i[12],i[13],i[14]);
	a44 =  det3x3(i[0],i[1],i[2],i[4],i[5],i[6],i[8],i[9],i[10]);

	det = (i[0]*a11) + (i[4]*a21) + (i[8]*a31) + (i[12]*a41);
	oodet = 1/det;

	output[ 0] = a11*oodet;
	output[ 1] = a21*oodet;
	output[ 2] = a31*oodet;
	output[ 3] = a41*oodet;

	output[ 4] = a12*oodet;
	output[ 5] = a22*oodet;
	output[ 6] = a32*oodet;
	output[ 7] = a42*oodet;

	output[ 8] = a13*oodet;
	output[ 9] = a23*oodet;
	output[10] = a33*oodet;
	output[11] = a43*oodet;

	output[12] = a14*oodet;
	output[13] = a24*oodet;
	output[14] = a34*oodet;
	output[15] = a44*oodet;
}

static __inline__ void multUnSave(MATRIX4x4D output, const MATRIX4x4D a,
	const MATRIX4x4D b)
{
	int iCol, cID, iRow, id, k;
	
	for (iCol = 0; iCol < 4; iCol++)
	{
		cID = iCol*4;
		
		for (iRow = 0; iRow < 4; iRow++)
		{
			id = iRow + cID;
			output[id] = a[iRow]*b[cID];
			
			for (k = 1; k < 4; k++)
				output[id] += a[iRow + k*4]*b[k + cID];
		}
	}
}

static __inline__ void mult(MATRIX4x4D output, const MATRIX4x4D a,
	const MATRIX4x4D b)
{
	MATRIX4x4D tmp;
	
	if (a == output)
	{
		copyMatrix(tmp, a);
		if (b == output) multUnSave(output, tmp, tmp);
		else multUnSave(output, tmp, b);
	}
	else
	{
		if (b == output)
		{
			copyMatrix(tmp, b);
			multUnSave(output, a, tmp);
		}
		else multUnSave(output, a, b);
	}
}

static __inline__ void mulHomogenPoint(VECTOR3D output, const MATRIX4x4D m,
	const VECTOR3D v)
{
	double x, y, z, w;
	
	//if v == output -> overwriting problems -> so store in temp
	x = m[0]*v[0] + m[4]*v[1] + m[ 8]*v[2] + m[12];
	y = m[1]*v[0] + m[5]*v[1] + m[ 9]*v[2] + m[13];
	z = m[2]*v[0] + m[6]*v[1] + m[10]*v[2] + m[14];
	w = m[3]*v[0] + m[7]*v[1] + m[11]*v[2] + m[15];

	output[0] = x/w;
	output[1] = y/w;
	output[2] = z/w;
}

static __inline__ void appendToCubicHull(VECTOR3D min, VECTOR3D max,
	const VECTOR3D v)
{
	min[0] = min2d(min[0], v[0]);
	min[1] = min2d(min[1], v[1]);
	min[2] = min2d(min[2], v[2]);
	max[0] = max2d(max[0], v[0]);
	max[1] = max2d(max[1], v[1]);
	max[2] = max2d(max[2], v[2]);
}

static __inline__ void calcCubicHull(VECTOR3D min, VECTOR3D max,
	const VECTOR3D* ps, const int size)
{
	int i;
	
	if (size > 0)
	{
		copyVECTOR3(min, ps[0]);
		copyVECTOR3(max, ps[0]);
		
		for (i = 1; i < size; i++)
			appendToCubicHull(min, max, ps[i]);
	}
}

static __inline__ void calcViewFrustumWorldCoord(VECTOR3x8D points,
	const MATRIX4x4D invEyeProjView)
{
	int i;
	const AABBOXD box = {{-1.0, -1.0, -1.0}, {1.0, 1.0, 1.0}};
	
	calcAABoxPoints(points,box); //calc unit cube corner points
	for (i = 0; i < 8; i++)
		mulHomogenPoint(points[i], invEyeProjView, points[i]); //camera to world frame
	//viewFrustumWorldCoord[0] == near-bottom-left
	//viewFrustumWorldCoord[1] == near-bottom-right
	//viewFrustumWorldCoord[2] == near-top-right
	//viewFrustumWorldCoord[3] == near-top-left
	//viewFrustumWorldCoord[4] == far-bottom-left
	//viewFrustumWorldCoord[5] == far-bottom-right
	//viewFrustumWorldCoord[6] == far-top-right
	//viewFrustumWorldCoord[7] == far-top-left
}

static __inline__ void calcViewFrustObject(Object* obj, const VECTOR3x8D p)
{
	int i;
	VECTOR3D* ps;
	
	objectSetSize(obj,6);
	for (i = 0; i < 6; i++)
		vecPointSetSize(&obj->poly[i], 4);

	//near poly ccw
	ps = obj->poly[0].points;
	for (i = 0; i < 4; i++)
		copyVECTOR3(ps[i], p[i]);

	//far poly ccw
	ps = obj->poly[1].points;
	for (i = 4; i < 8; i++)
		copyVECTOR3(ps[i-4], p[11-i]);

	//left poly ccw
	ps = obj->poly[2].points;
	copyVECTOR3(ps[0], p[0]);
	copyVECTOR3(ps[1], p[3]);
	copyVECTOR3(ps[2], p[7]);
	copyVECTOR3(ps[3], p[4]);

	//right poly ccw
	ps = obj->poly[3].points;
	copyVECTOR3(ps[0], p[1]);
	copyVECTOR3(ps[1], p[5]);
	copyVECTOR3(ps[2], p[6]);
	copyVECTOR3(ps[3], p[2]);

	//bottom poly ccw
	ps = obj->poly[4].points;
	copyVECTOR3(ps[0], p[4]);
	copyVECTOR3(ps[1], p[5]);
	copyVECTOR3(ps[2], p[1]);
	copyVECTOR3(ps[3], p[0]);

	//top poly ccw
	ps = obj->poly[5].points;
	copyVECTOR3(ps[0], p[6]);
	copyVECTOR3(ps[1], p[7]);
	copyVECTOR3(ps[2], p[3]);
	copyVECTOR3(ps[3], p[2]);
}

static __inline__ void transformVecPoint(VecPoint* poly, const MATRIX4x4D xForm)
{
	int i;
	
	if (poly != NULL)
	{
		for (i = 0; i < poly->size; i++)
			mulHomogenPoint(poly->points[i], xForm,poly->points[i]);
	}
}

static __inline__ void transformObject(Object* obj, const MATRIX4x4D xForm)
{
	int i;
	
	if (obj != NULL)
	{
		for (i = 0; i < obj->size; i++)
			transformVecPoint(&(obj->poly[i]), xForm);
	}
}

static __inline__ void calcObjectCubicHull(VECTOR3D min, VECTOR3D max,
	const Object obj)
{
	VecPoint* p;
	int i, j;
	
	if (obj.size > 0)
	{
		calcCubicHull(min, max, obj.poly[0].points, obj.poly[0].size);
		
		for (i = 1; i < obj.size; i++)
		{
			p = &(obj.poly[i]);
			
			for (j = 0; j < p->size; j++)
				appendToCubicHull(min, max, p->points[j]);
		}
	}
}

static __inline__ int findSamePointInVecPoint(const VecPoint poly,
	const VECTOR3D p)
{
	int i;
	
	for (i = 0; i < poly.size; i++)
	{
		if (alikeVECTOR3(poly.points[i], p, 0.001)) return i;
	}
	
	return -1;
}

static __inline__ int findSamePointInObjectAndSwapWithLast(Object *inter,
	const VECTOR3D p)
{
	VecPoint* poly;
	int i, nr;
	
	if (inter == 0) return -1;

	if (inter->size < 1) return -1;

	for (i = inter->size; i > 0; i--)
	{
		poly = &(inter->poly[i-1]);
		
		if (poly->size == 2)
		{
			nr = findSamePointInVecPoint(*poly, p);
			
			if (nr >= 0)
			{
				//swap with last
				swapVecPoint(poly, &(inter->poly[inter->size-1]));
				return nr;
			}
		}
	}
	
	return -1;
}

static __inline__ void appendIntersectionVecPoint(Object* obj, Object* inter)
{
	VECTOR3D *lastPt;
	VecPoint *polyOut;
	VecPoint *polyIn;
	int size, i, nr;
	
	size = obj->size;
	
	//you need at least 3 sides for a polygon
	if (3 > inter->size) return;

	//compact inter: remove poly.size != 2 from end on forward
	for (i = inter->size; 0 < i; i--)
	{
		if (inter->poly[i-1].size == 2)	break;
	}
	
	objectSetSize(inter, i);
	
	//you need at least 3 sides for a polygon
	if (inter->size < 3) return;

	//make place for one additional polygon in obj
	objectSetSize(obj, size+1);
	polyOut = &(obj->poly[size]);

	//we have line segments in each poly of inter 
	//take last linesegment as first and second point
	polyIn = &(inter->poly[inter->size-1]);
	append2VecPoint(polyOut, polyIn->points[0]);
	append2VecPoint(polyOut, polyIn->points[1]);

	//remove last poly from inter, because it is already saved
	objectSetSize(inter, inter->size-1);

	//iterate over inter until their is no line segment left
	while (inter->size > 0)
	{
		//pointer on last point to compare
		lastPt = &(polyOut->points[polyOut->size-1]);
		
		//find same point in inter to continue polygon
		nr = findSamePointInObjectAndSwapWithLast(inter, *lastPt);
		
		if (nr >= 0)
		{
			//last line segment
			polyIn = &(inter->poly[inter->size-1]);
			
			//get the other point in this polygon and save into polyOut
			append2VecPoint(polyOut, polyIn->points[(nr+1)%2]);
		}
		
		//remove last poly from inter, because it is already saved or degenerated
		objectSetSize(inter, inter->size-1);
	}
	
	//last point can be deleted, because he is the same as the first (closes polygon)
	vecPointSetSize(polyOut, polyOut->size-1);
}

static __inline__ double pointPlaneDistance(const Plane A, const VECTOR3D p)
{
	return dot(A.n,p) - A.d;
}

static __inline__ int pointBeforePlane(const Plane A, const VECTOR3D p)
{
	return pointPlaneDistance(A,p) > 0.0;
}

static __inline__ int intersectPlaneEdge(VECTOR3D output, const Plane A,
	const VECTOR3D a, const VECTOR3D b)
{
	VECTOR3D diff;
	double t;
	
	diffVECTOR3(diff, b, a);
	t = dot(A.n, diff);
	if (0.0 == t) return 0;
		
	t = (A.d - dot(A.n,a))/t;
	if ((t < 0.0) || (t > 1.0)) return 0;
		
	linCombVECTOR3(output, a, diff, t);
	return 1;
}

static __inline__ void clipVecPointByPlane(const VecPoint poly, const Plane A,
	VecPoint* polyOut, VecPoint* interPts)
{
	VECTOR3D inter;
	int i, idNext;
	int *outside;
	
	outside = NULL;
	
	if ((poly.size < 3) || (polyOut == NULL)) return;
	outside = (int*)realloc(outside, poly.size*sizeof(int));
	
	//for each point
	for (i = 0; i < poly.size; i++)
		outside[i] = pointBeforePlane(A, poly.points[i]);

	for (i = 0; i < poly.size; i++)
	{
		idNext = (i+1) % poly.size;
		
		//both outside -> save none
		if (outside[i] && outside[idNext]) continue;

		//outside to inside -> calc intersection save intersection and save i+1
		if (outside[i])
		{
			if (intersectPlaneEdge(inter, A, poly.points[i], poly.points[idNext]))
			{
				append2VecPoint(polyOut, inter);
				if (interPts != NULL) append2VecPoint(interPts, inter);
			}
			append2VecPoint(polyOut, poly.points[idNext]);
			continue;
		}
		
		//inside to outside -> calc intersection save intersection
		if (outside[idNext])
		{
			if (intersectPlaneEdge(inter, A, poly.points[i], poly.points[idNext]))
			{
				append2VecPoint(polyOut, inter);
				if (interPts != NULL) append2VecPoint(interPts, inter);
			}
			continue;
		}
		
		//both inside -> save point i+1
		append2VecPoint(polyOut, poly.points[idNext]);
	}
	
	outside = (int*)realloc(outside, 0);
}

static __inline__ void clipObjectByPlane(const Object obj, const Plane A,
	Object* objOut)
{
	int i, size;
	Object inter, objIn;
	
	inter = OBJECT_NULL;
	objIn = OBJECT_NULL;
	
	if ((obj.size == 0) || (objOut == NULL)) return;
		
	if (obj.poly == objOut->poly)
	{
		//need to copy object if input and output are the same
		copyObject(&objIn, obj);
	}
	else objIn = obj;

	emptyObject(objOut);

	for (i = 0; i < objIn.size; i++)
	{
		size = objOut->size;
		objectSetSize(objOut, size+1);
		objectSetSize(&inter, size+1);
		clipVecPointByPlane(objIn.poly[i], A, &(objOut->poly[size]), &(inter.poly[size]));
		
		//if whole poly was clipped away -> delete empty poly
		if (objOut->poly[size].size == 0)
		{
			objectSetSize(objOut, size);
			objectSetSize(&inter, size);
		}
	}
	
	//add a polygon of all intersection points with plane to close the object
	appendIntersectionVecPoint(objOut, &inter);
	emptyObject(&inter);
	emptyObject(&objIn);
}


static __inline__ void clipObjectByAABox(Object* obj, const AABBOXD box)
{
	VecPlane planes = VECPLANE_NULL;
	int i;
	
	if (obj == NULL) return;
	
	calcAABoxPlanes(&planes, box);
	
	for (i = 0; i < planes.size; i++)
		clipObjectByPlane(*obj, planes.plane[i], obj);

	emptyVecPlane(&planes);
}

static __inline__ int clipTest(const double p, const double q, double* u1,
	double* u2)
{
	double r;
	
	// Return value is 'true' if line segment intersects the current test
	// plane.  Otherwise 'false' is returned in which case the line segment
	// is entirely clipped.
	if ((u1 == NULL) || (u2 == NULL)) return 0;

	if (p < 0.0)
	{
		r = q/p;
		if (r > (*u2)) return 0;
		else
		{
			if (r > (*u1)) (*u1) = r;
			return 1;
		}
	}
	else
	{
		if (p > 0.0)
		{
			r = q/p;
			if (r < (*u1)) return 0;
			else
			{
				if (r < (*u2)) (*u2) = r;
				return 1;
			}
		}
		else return q >= 0.0;
	}
}

static __inline__ int intersectionLineAABox(VECTOR3D v, const VECTOR3D p,
	const VECTOR3D dir, const AABBOXD b)
{
	double t1, t2;
	int intersect;
	
	t1 = 0.0;
	t2 = DBL_MAX;
	
	intersect = 	clipTest(-dir[2], p[2]-b.bbmin[2], &t1, &t2) &&
			clipTest(dir[2], b.bbmax[2]-p[2], &t1, &t2) &&
			clipTest(-dir[1], p[1]-b.bbmin[1], &t1, &t2) &&
			clipTest(dir[1], b.bbmax[1]-p[1], &t1, &t2) &&
			clipTest(-dir[0], p[0]-b.bbmin[0], &t1, &t2) &&
			clipTest(dir[0], b.bbmax[0]-p[0], &t1, &t2);
	
	if (!intersect) return 0;
		
	intersect = 0;
	if (t1 >= 0)
	{
		linCombVECTOR3(v, p, dir, t1);
		intersect = 1;
	}
	
	if (t2 >= 0)
	{
		linCombVECTOR3(v, p, dir, t2);
		intersect = 1;
	}
	
	return intersect;
}


static __inline__ void includeObjectLightVolume(VecPoint* points,
	const Object obj, const VECTOR3D lightDir, const AABBOXD sceneAABox)
{
	VECTOR3D ld, pt;
	int i, size;
	
	if (points != NULL)
	{
		copyVECTOR3Values(ld, -lightDir[0], -lightDir[1], -lightDir[2]);
		convObject2VecPoint(points, obj);
		size = points->size;
		
		//for each point add the point on the ray in -lightDir
		//intersected with the sceneAABox
		for (i = 0; i < size; i++)
		{
			if (intersectionLineAABox(pt,points->points[i], ld, sceneAABox))
				append2VecPoint(points, pt);
		}
	}
}

static __inline__ void calcFocusedLightVolumePoints(VecPoint* points,
	const MATRIX4x4D invEyeProjView, const VECTOR3D lightDir,
	const AABBOXD sceneAABox)
{
	VECTOR3x8D pts;
	Object obj;
	
	obj = OBJECT_NULL;

	calcViewFrustumWorldCoord(pts, invEyeProjView);
	calcViewFrustObject(&obj, pts);
	clipObjectByAABox(&obj, sceneAABox);
	includeObjectLightVolume(points, obj, lightDir, sceneAABox);
	emptyObject(&obj);
}

//CHANGED
static __inline__ void calcNewDir(VECTOR3D dir, const VecPoint* B,
	const VECTOR3D eyePos)
{
	VECTOR3D p;
	int i;
	
	copyVECTOR3(dir, ZERO);
	
	for (i = 0; i < B->size; i++)
	{
		diffVECTOR3(p, B->points[i], eyePos);
		addVECTOR3(dir, dir, p);
	}
	
	normalize(dir);
}

//calculates the up vector for the light coordinate frame
static __inline__ void calcUpVec(VECTOR3D up, const VECTOR3D viewDir,
	const VECTOR3D lightDir)
{
	//we do what gluLookAt does...
	VECTOR3D left;
	
	//left is the normalized vector perpendicular to lightDir and viewDir
	//this means left is the normalvector of the yz-plane from the paper
	cross(left, lightDir, viewDir);
	
	//we now can calculate the rotated(in the yz-plane) viewDir vector
	//and use it as up vector in further transformations
	cross(up, left, lightDir);
	normalize(up);
}

//this is the algorithm discussed in the paper
static __inline__ void calcLispSMMtx(VecPoint* B, int useBodyVec,
	double nearDist, const VECTOR3D viewDir, const VECTOR3D lightDir, 
	const VECTOR3D eyePos, MATRIX4x4D lightView, MATRIX4x4D lightProjection)
{
	VECTOR3D min, max, pos;
	VECTOR3D up, newDir;
	MATRIX4x4D lispMtx;
	VecPoint Bcopy;
	double dotProd, sinGamma, factor, z_n, d, z_f, n, f;

	Bcopy = VECPOINT_NULL;
	dotProd = dot(viewDir, lightDir);

	sinGamma = sqrt(1.0-dotProd*dotProd);

	copyMatrix(lispMtx, IDENTITY);

	copyVecPoint(&Bcopy, *B);

	//CHANGED
	if (useBodyVec)
	{
		calcNewDir(newDir, B, eyePos);
		calcUpVec(up, newDir, lightDir);
	}
	else calcUpVec(up, viewDir, lightDir);

	//temporal light View
	//look from position(eyePos)
	//into direction(lightDir) 
	//with up vector(up)
	look(lightView, eyePos, lightDir, up);

	//transform the light volume points from world into light space
	transformVecPoint(B, lightView);

	//calculate the cubic hull (an AABB) 
	//of the light space extents of the intersection body B
	//and save the two extreme points min and max
	calcCubicHull(min, max, B->points, B->size);

	//use the formulas of the paper to get n (and f)
	factor = 1.0/sinGamma;
	z_n = factor*nearDist; //often 1 
	d = abs(max[1] - min[1]); //perspective transform depth //light space y extents
	z_f = z_n + d*sinGamma;
	n = (z_n + sqrt(z_f*z_n))/sinGamma;
	f = n + d;

	//new observer point n-1 behind eye position
	//pos = eyePos-up*(n-nearDist)
	linCombVECTOR3(pos, eyePos, up, -(n-nearDist));

	look(lightView, pos, lightDir, up);

	//one possibility for a simple perspective transformation matrix
	//with the two parameters n(near) and f(far) in y direction
	copyMatrix(lispMtx, IDENTITY);	// a = (f+n)/(f-n); b = -2*f*n/(f-n);
	lispMtx[ 5] = (f + n)/(f - n);	// [ 1 0 0 0] 
	lispMtx[13] = -2.0*f*n/(f - n);	// [ 0 a 0 b]
	lispMtx[ 7] = 1.0;		// [ 0 0 1 0]
	lispMtx[15] = 0.0;		// [ 0 1 0 0]

	//temporal arrangement for the transformation of the points to post-perspective space
	mult(lightProjection, lispMtx, lightView); // ligthProjection = lispMtx*lightView
		
	//transform the light volume points from world into the distorted light space
	transformVecPoint(&Bcopy, lightProjection);

	//calculate the cubic hull (an AABB) 
	//of the light space extents of the intersection body B
	//and save the two extreme points min and max
	calcCubicHull(min, max, Bcopy.points, Bcopy.size);

	//refit to unit cube
	//this operation calculates a scale translate matrix that
	//maps the two extreme points min and max into (-1,-1,-1) and (1,1,1)
	scaleTranslateToFit(lightProjection, min, max);

	//together
	mult(lightProjection, lightProjection, lispMtx); // ligthProjection = scaleTranslate*lispMtx
}

void calculate_Light_Matrix(int useBodyVec, double nearDist,
	const VECTOR3D lightDir, const MATRIX4x4D ModelViewMatrix,
	const MATRIX4x4D ProjectionMatrix, MATRIX4x4D lightView,
	MATRIX4x4D lightProjection)
{
	MATRIX4x4D tmp;
	MATRIX4x4D invEyeProjView; //= eyeProjView^(-1)
	AABBOXD sceneAABox;
	VECTOR3D eyePos, viewDir;
	FRUSTUM frustum;
	AABBOX bbox;
	double w;
	VecPoint B;
	int i;

	if (main_bbox_tree != NULL)
	{
		if (main_bbox_tree->root_node != NULL)
		{
			invert(tmp, ModelViewMatrix);
	
			eyePos[X] = tmp[12]/tmp[15];
			eyePos[Y] = tmp[13]/tmp[15];
			eyePos[Z] = tmp[14]/tmp[15];
	
			viewDir[X] = tmp[ 8] + tmp[12];
			viewDir[Y] = tmp[ 9] + tmp[13];	
			viewDir[Z] = tmp[10] + tmp[14];
			w = tmp[11] + tmp[15];
			viewDir[X] /= w;
			viewDir[Y] /= w;
			viewDir[Z] /= w;

			diffVECTOR3(viewDir, viewDir, eyePos);
			viewDir[X] *= -1.0f;
			viewDir[Y] *= -1.0f;
			viewDir[Z] *= -1.0f;
			normalize(viewDir);

			mult(tmp, ProjectionMatrix, ModelViewMatrix);
			invert(invEyeProjView, tmp);

			sceneAABox.bbmin[0] = main_bbox_tree->root_node->bbox.bbmin[0];
			sceneAABox.bbmin[1] = main_bbox_tree->root_node->bbox.bbmin[1];
			sceneAABox.bbmin[2] = main_bbox_tree->root_node->bbox.bbmin[2];
			sceneAABox.bbmax[0] = main_bbox_tree->root_node->bbox.bbmax[0];
			sceneAABox.bbmax[1] = main_bbox_tree->root_node->bbox.bbmax[1];
			sceneAABox.bbmax[2] = main_bbox_tree->root_node->bbox.bbmax[2];

			for (i = 0; i < 5; i++)
			{
				B = VECPOINT_NULL;

				//calculates the ViewFrustum Object; clippes this Object By the sceneAABox and
				//extrudes the object into -lightDir and clippes by the sceneAABox
				//the defining points are returned
			
				calcFocusedLightVolumePoints(&B, invEyeProjView, lightDir, sceneAABox);
				calcLispSMMtx(&B, useBodyVec, nearDist, viewDir, lightDir, eyePos, lightView, lightProjection);

				emptyVecPoint(&B);

				//transform from right handed into left handed coordinate system
				makeScaleMtx(tmp, 1.0f, 1.0f, -1.0f);
				mult(lightProjection, tmp, lightProjection);
				
				calculate_light_frustum(frustum, lightView, lightProjection);
				calc_scene_bbox(main_bbox_tree, &frustum, &bbox);
				sceneAABox.bbmin[0] = bbox.bbmin[0];
				sceneAABox.bbmin[1] = bbox.bbmin[1];
				sceneAABox.bbmin[2] = bbox.bbmin[2];
				sceneAABox.bbmax[0] = bbox.bbmax[0];
				sceneAABox.bbmax[1] = bbox.bbmax[1];
				sceneAABox.bbmax[2] = bbox.bbmax[2];
			}
		}
	}
}
#endif
