#ifdef	NEW_FRUSTUM
#ifndef	BBOX_TREE_H
#define BBOX_TREE_H

#include <math.h>

#define TYPE_2D_NO_ALPHA_OBJECT				0x00
#define TYPE_2D_ALPHA_OBJECT				0x01
#define TYPE_3D_NO_BLEND_NO_GOUND_OBJECT		0x02
#define TYPE_3D_NO_BLEND_GROUND_OBJECT			0x03
#define TYPE_3D_BLEND_NO_GROUND_OBJECT			0x04
#define TYPE_3D_BLEND_GROUND_OBJECT			0x05
#define	TYPE_REFLECTION_FRUSTUM				0x06
#define TYPE_PARTICLE					0x07
#define	TYPE_LIGHT					0x08
#define	TYPE_WATER					0x09
#define	TYPE_TERRAIN					0x0A
#define	TYPES_COUNT					0x0B

#define	OUTSIDE		0x0000
#define	INSIDE		0x0001
#define INTERSECT	0x0002

#define BOUND_HUGE	10e30

typedef float VECTOR3[3];
typedef float VECTOR4[4];
typedef unsigned short IDX_TYPE;

typedef struct
{
	VECTOR3 bbmin;
	VECTOR3 bbmax;
} AABBOX;

typedef	struct
{
	VECTOR4 		plane;
	unsigned char 		mask[3];
} PLANE;

typedef PLANE FRUSTUM[6];

typedef	PLANE PORTAL_FRUSTUM[8];

typedef struct
{
	unsigned short		count;
	PORTAL_FRUSTUM*		portal_frustums;
} PORTAL_FRUSTUMS;

typedef struct
{
	unsigned char		type;
	unsigned char		sort_data;
	unsigned short		ID;
} BBOX_ITEM_DATA;

typedef struct
{
	AABBOX			bbox;
	BBOX_ITEM_DATA		data;
} BBOX_ITEM;

typedef struct
{
	IDX_TYPE		size;
	IDX_TYPE		index;
	BBOX_ITEM*		items;
} BBOX_ITEMS;	

typedef struct
{
	IDX_TYPE		size;
	IDX_TYPE		index;
	IDX_TYPE		sub_size;
	IDX_TYPE		sub_index;
	BBOX_ITEM*		items;
	BBOX_ITEM_DATA*		sub_items;
} BBOX_TREE_DYN_NODE;

typedef struct BBox_Tree_Node_Struct BBOX_TREE_NODE;

struct BBox_Tree_Node_Struct
{
	AABBOX			bbox;
	BBOX_TREE_NODE*		nodes[2];
	BBOX_TREE_DYN_NODE	dynamic_objects;
	IDX_TYPE		items_index;
	IDX_TYPE		items_count;
};

typedef	struct
{
	BBOX_TREE_NODE*		root_node;
	IDX_TYPE		items_count;
	BBOX_ITEM*		items;
	IDX_TYPE		nodes_count;
	BBOX_TREE_NODE*		nodes;	
	IDX_TYPE		intersect_count;
	IDX_TYPE		type_start[TYPES_COUNT];
	IDX_TYPE		type_stop[TYPES_COUNT];
	BBOX_ITEM_DATA*		intersect_items;
} BBOX_TREE;

enum
{
	A = 0,
	B = 1,
	C = 2,
	D = 3
};

enum
{
	X = 0,
	Y = 1,
	Z = 2,
	W = 3
};

static __inline__ float min(float f1, float f2)
{
	if (f1 < f2) return f1;
	else return f2;
}

static __inline__ float max(float f1, float f2)
{
	if (f1 > f2) return f1;
	else return f2;
}

static __inline__ void VMin(VECTOR3 v1, const VECTOR3 v2, const VECTOR3 v3)
{
	v1[X] = min(v2[X], v3[X]);
	v1[Y] = min(v2[Y], v3[Y]);
	v1[Z] = min(v2[Z], v3[Z]);
}

static __inline__ void VMax(VECTOR3 v1, const VECTOR3 v2, const VECTOR3 v3)
{
	v1[X] = max(v2[X], v3[X]);
	v1[Y] = max(v2[Y], v3[Y]);
	v1[Z] = max(v2[Z], v3[Z]);
}

static __inline__ void VSub(VECTOR3 v1, const VECTOR3 v2, const VECTOR3 v3)
{
	v1[X] = v2[X] - v3[X];
	v1[Y] = v2[Y] - v3[Y];
	v1[Z] = v2[Z] - v3[Z];
}

static __inline__ void Make_Vector3(VECTOR3 v1, float f)
{
	v1[X] = f;
	v1[Y] = f;
	v1[Z] = f;
}

static __inline__ void calc_light_aabb(AABBOX *bbox, float pos_x, float pos_y, float pos_z, float diff_r, float diff_g, float diff_b, 
		float att, float exp, float clamp)
{
	float h, r;

	h = max(abs(diff_r), max(abs(diff_g), abs(diff_b)));

	r = (h/clamp-1)/att;

	if (exp <= 0.0f) return;
	else
	{
		if (exp == 2.0f) r = sqrt(r);
		else
		{
			if (exp != 1.0f) r = pow(r, exp);
		}
	}
		
	bbox->bbmin[X] = pos_x - r;
	bbox->bbmin[Y] = pos_y - r;
	bbox->bbmin[Z] = pos_z - r;
	bbox->bbmax[X] = pos_x + r;
	bbox->bbmax[Y] = pos_y + r;
	bbox->bbmax[Z] = pos_z + r;
}

static __inline__ void rotate_aabb(AABBOX* bbox, float r_x, float r_y, float r_z)
{
	float matrix_1[16], matrix_2[16];
	matrix_1[0] = bbox->bbmax[X];
	matrix_1[1] = bbox->bbmax[Y];
	matrix_1[2] = bbox->bbmax[Z];
	matrix_1[3] = 1.0f;
	matrix_1[4] = bbox->bbmax[X];
	matrix_1[5] = bbox->bbmax[Y];
	matrix_1[6] = bbox->bbmin[Z];
	matrix_1[7] = 1.0f;
	matrix_1[8] = bbox->bbmax[X];
	matrix_1[9] = bbox->bbmin[Y];
	matrix_1[10] = bbox->bbmax[Z];
	matrix_1[11] = 1.0f;
	matrix_1[12] = bbox->bbmax[X];
	matrix_1[13] = bbox->bbmin[Y];
	matrix_1[14] = bbox->bbmin[Z];
	matrix_1[15] = 1.0f;
	matrix_2[0] = bbox->bbmin[X];
	matrix_2[1] = bbox->bbmax[Y];
	matrix_2[2] = bbox->bbmax[Z];
	matrix_2[3] = 1.0f;
	matrix_2[4] = bbox->bbmin[X];
	matrix_2[5] = bbox->bbmax[Y];
	matrix_2[6] = bbox->bbmin[Z];
	matrix_2[7] = 1.0f;
	matrix_2[8] = bbox->bbmin[X];
	matrix_2[9] = bbox->bbmin[Y];
	matrix_2[10] = bbox->bbmax[Z];
	matrix_2[11] = 1.0f;
	matrix_2[12] = bbox->bbmin[X];
	matrix_2[13] = bbox->bbmin[Y];
	matrix_2[14] = bbox->bbmin[Z];
	matrix_2[15] = 1.0f;
	glPushMatrix();
	glLoadIdentity();
	glRotatef(r_z, 0.0f, 0.0f, 1.0f);
	glRotatef(r_x, 1.0f, 0.0f, 0.0f);
	glRotatef(r_y, 0.0f, 1.0f, 0.0f);
	glPushMatrix();
	glMultMatrixf(matrix_1);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix_1);
	glPopMatrix();
	glMultMatrixf(matrix_2);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix_2);
	glPopMatrix();
	bbox->bbmin[X] = min(min(matrix_1[0], matrix_1[4]), min(matrix_1[8], matrix_1[12]));
	bbox->bbmin[X] = min(bbox->bbmin[X], min(min(matrix_2[0], matrix_2[4]), min(matrix_2[8], matrix_2[12])));
	bbox->bbmin[Y] = min(min(matrix_1[1], matrix_1[5]), min(matrix_1[9], matrix_1[13]));
	bbox->bbmin[Y] = min(bbox->bbmin[Y], min(min(matrix_2[1], matrix_2[5]), min(matrix_2[9], matrix_2[13])));
	bbox->bbmin[Z] = min(min(matrix_1[2], matrix_1[6]), min(matrix_1[10], matrix_1[14]));
	bbox->bbmin[Z] = min(bbox->bbmin[Z], min(min(matrix_2[2], matrix_2[6]), min(matrix_2[10], matrix_2[14])));

	bbox->bbmax[X] = max(max(matrix_1[0], matrix_1[4]), max(matrix_1[8], matrix_1[12]));
	bbox->bbmax[X] = max(bbox->bbmax[X], max(max(matrix_2[0], matrix_2[4]), max(matrix_2[8], matrix_2[12])));
	bbox->bbmax[Y] = max(max(matrix_1[1], matrix_1[5]), max(matrix_1[9], matrix_1[13]));
	bbox->bbmax[Y] = max(bbox->bbmax[Y], max(max(matrix_2[1], matrix_2[5]), max(matrix_2[9], matrix_2[13])));
	bbox->bbmax[Z] = max(max(matrix_1[2], matrix_1[6]), max(matrix_1[10], matrix_1[14]));
	bbox->bbmax[Z] = max(bbox->bbmax[Z], max(max(matrix_2[2], matrix_2[6]), max(matrix_2[10], matrix_2[14])));
}

void check_bbox_tree(BBOX_TREE* bbox_tree, FRUSTUM *frustum);
void free_bbox_tree(BBOX_TREE* bbox_tree);
BBOX_TREE* build_bbox_tree(BBOX_ITEMS *bbox_items);
void add_light_to_abt(BBOX_ITEMS *bbox_items, unsigned int ID, AABBOX *bbox);
void add_3dobject_to_abt(BBOX_ITEMS *bbox_items, unsigned int ID, AABBOX *bbox, unsigned int blend, unsigned int ground);
void add_2dobject_to_abt(BBOX_ITEMS *bbox_items, unsigned int ID, AABBOX *bbox, unsigned int alpha);
void add_particle_to_abt(BBOX_ITEMS *bbox_items, unsigned int ID, AABBOX *bbox, unsigned int sblend, unsigned int dblend);
void add_dynamic_3dobject_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, AABBOX *bbox, unsigned int blend, unsigned int ground);
void add_dynamic_particle_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, AABBOX *bbox, unsigned int sblend, unsigned int dblend);
void add_dynamic_light_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, AABBOX *bbox);
void delete_dynamic_3dobject_from_abt(BBOX_TREE *bbox_tree, unsigned int ID, unsigned int blend, unsigned int ground);
void delete_dynamic_particle_from_abt(BBOX_TREE *bbox_tree, unsigned int ID);
void delete_dynamic_light_from_abt(BBOX_TREE *bbox_tree, unsigned int ID);
BBOX_ITEMS* create_bbox_items(unsigned int size);
void free_bbox_items(BBOX_ITEMS* bbox_items);

extern FRUSTUM frustum;
extern BBOX_TREE* bbox_tree;

#endif
#endif
