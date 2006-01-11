#ifdef	NEW_FRUSTUM
#ifndef	BBOX_TREE_H
#define BBOX_TREE_H

#include <stdlib.h>
#ifdef MAP_EDITOR2
#include "../map_editor2/global.h"
#include "../map_editor2/misc.h"
#else
#include "global.h"
#include "misc.h"
#endif
#include "md5.h"

#define TYPE_2D_NO_ALPHA_OBJECT					0x00
#define TYPE_2D_ALPHA_OBJECT					0x01
#define TYPE_3D_BLEND_GROUND_ALPHA_SELF_LIT_OBJECT		0x02
#define TYPE_3D_BLEND_GROUND_ALPHA_NO_SELF_LIT_OBJECT		0x03
#define TYPE_3D_BLEND_GROUND_NO_ALPHA_SELF_LIT_OBJECT		0x04
#define TYPE_3D_BLEND_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT	0x05
#define TYPE_3D_BLEND_NO_GROUND_ALPHA_SELF_LIT_OBJECT		0x06
#define TYPE_3D_BLEND_NO_GROUND_ALPHA_NO_SELF_LIT_OBJECT	0x07
#define TYPE_3D_BLEND_NO_GROUND_NO_ALPHA_SELF_LIT_OBJECT	0x08
#define TYPE_3D_BLEND_NO_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT	0x09
#define TYPE_3D_NO_BLEND_GROUND_ALPHA_SELF_LIT_OBJECT		0x0A
#define TYPE_3D_NO_BLEND_GROUND_ALPHA_NO_SELF_LIT_OBJECT	0x0B
#define TYPE_3D_NO_BLEND_GROUND_NO_ALPHA_SELF_LIT_OBJECT	0x0C
#define TYPE_3D_NO_BLEND_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT	0x0D
#define TYPE_3D_NO_BLEND_NO_GROUND_ALPHA_SELF_LIT_OBJECT	0x0E
#define TYPE_3D_NO_BLEND_NO_GROUND_ALPHA_NO_SELF_LIT_OBJECT	0x0F
#define TYPE_3D_NO_BLEND_NO_GROUND_NO_ALPHA_SELF_LIT_OBJECT	0x10
#define TYPE_3D_NO_BLEND_NO_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT	0x11
#define TYPE_PARTICLE_SYSTEM					0x12
#define	TYPE_LIGHT						0x13
#define	TYPE_TERRAIN						0x14
#define	TYPE_NO_REFLECTIV_WATER					0x15
#define	TYPE_REFLECTIV_WATER					0x16
#define	TYPES_COUNT						0x17
#define TYPE_DELETED						0xFF

#define	INTERSECTION_TYPE_DEFAULT			0x00
#define	INTERSECTION_TYPE_SHADOW			0x01
#define	INTERSECTION_TYPE_REFLECTION			0x02
#define	MAX_INTERSECTION_TYPES				0x03

#define	OUTSIDE		0x0000
#define	INSIDE		0x0001
#define INTERSECT	0x0002

#define BOUND_HUGE	10e30

#define	EXTRA_FIRST_SUB_NODE	0x0001

typedef struct
{
	VECTOR3 bbmin;
	VECTOR3 bbmax;
} AABBOX;

typedef struct
{
	VECTOR3 center;
	VECTOR3 direction;
	float length;
} LINE;

typedef	struct
{
	VECTOR4 		plane;
	VECTOR3I 		mask;
} PLANE;

typedef PLANE FRUSTUM[16];

typedef struct
{
	float			scale;
	unsigned int		mask;
	unsigned int		zero;
} PLANE_DATA;

typedef PLANE_DATA FRUSTUM_DATA[16];

typedef VECTOR3I VECTOR3Ix2[2];

typedef struct
{
	unsigned char		type;
	unsigned char		extra;
	unsigned int		texture_id;
	MD5_DIGEST		md5;
	unsigned int		ID;
} BBOX_ITEM_DATA;

typedef struct
{
	AABBOX			bbox;
	BBOX_ITEM_DATA		data;
} BBOX_ITEM;

typedef struct
{
	unsigned int		size;
	unsigned int		index;
	BBOX_ITEM*		items;
} BBOX_ITEMS;	

typedef struct
{
	unsigned int		size;
	unsigned int		index;
	unsigned int		sub_size;
	unsigned int		sub_index;
	BBOX_ITEM*		items;
	BBOX_ITEM_DATA*		sub_items;
} BBOX_TREE_DYN_NODE;

typedef struct BBox_Tree_Node_Struct BBOX_TREE_NODE;

struct BBox_Tree_Node_Struct
{
	AABBOX			bbox;
	AABBOX			orig_bbox;
	unsigned int		nodes[2];
	BBOX_TREE_DYN_NODE	dynamic_objects;
	unsigned int		items_index;
	unsigned int		items_count;
};

typedef struct
{
	unsigned int		intersect_update_needed;
	unsigned int		size;
	unsigned int		count;
	unsigned int		start[TYPES_COUNT];
	unsigned int		stop[TYPES_COUNT];
	BBOX_ITEM_DATA*		items;
} BBOX_INTERSECTION_DATA;

typedef	struct
{
	unsigned int		items_count;
	BBOX_ITEM*		items;
	unsigned int		nodes_count;
	BBOX_TREE_NODE*		nodes;
	unsigned int		cur_intersect_type;
	BBOX_INTERSECTION_DATA	intersect[MAX_INTERSECTION_TYPES];
} BBOX_TREE;

enum
{
	A = 0,
	B = 1,
	C = 2,
	D = 3
};

static __inline__ void calc_light_aabb(AABBOX* bbox, float pos_x, float pos_y, float pos_z, float diff_r, float diff_g, float diff_b, 
		float att, float exp, float clamp)
{
	float h, r;

	h = max2f(abs(diff_r), max2f(abs(diff_g), abs(diff_b)));

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
	bbox->bbmin[X] = min2f(min2f(matrix_1[0], matrix_1[4]), min2f(matrix_1[8], matrix_1[12]));
	bbox->bbmin[X] = min2f(bbox->bbmin[X], min2f(min2f(matrix_2[0], matrix_2[4]), min2f(matrix_2[8], matrix_2[12])));
	bbox->bbmin[Y] = min2f(min2f(matrix_1[1], matrix_1[5]), min2f(matrix_1[9], matrix_1[13]));
	bbox->bbmin[Y] = min2f(bbox->bbmin[Y], min2f(min2f(matrix_2[1], matrix_2[5]), min2f(matrix_2[9], matrix_2[13])));
	bbox->bbmin[Z] = min2f(min2f(matrix_1[2], matrix_1[6]), min2f(matrix_1[10], matrix_1[14]));
	bbox->bbmin[Z] = min2f(bbox->bbmin[Z], min2f(min2f(matrix_2[2], matrix_2[6]), min2f(matrix_2[10], matrix_2[14])));

	bbox->bbmax[X] = max2f(max2f(matrix_1[0], matrix_1[4]), max2f(matrix_1[8], matrix_1[12]));
	bbox->bbmax[X] = max2f(bbox->bbmax[X], max2f(max2f(matrix_2[0], matrix_2[4]), max2f(matrix_2[8], matrix_2[12])));
	bbox->bbmax[Y] = max2f(max2f(matrix_1[1], matrix_1[5]), max2f(matrix_1[9], matrix_1[13]));
	bbox->bbmax[Y] = max2f(bbox->bbmax[Y], max2f(max2f(matrix_2[1], matrix_2[5]), max2f(matrix_2[9], matrix_2[13])));
	bbox->bbmax[Z] = max2f(max2f(matrix_1[2], matrix_1[6]), max2f(matrix_1[10], matrix_1[14]));
	bbox->bbmax[Z] = max2f(bbox->bbmax[Z], max2f(max2f(matrix_2[2], matrix_2[6]), max2f(matrix_2[10], matrix_2[14])));
}

static __inline__ void matrix_mul_aabb(AABBOX* bbox, MATRIX4x4 matrix)
{
	MATRIX4x4 matrix_1, matrix_2;
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
	glLoadMatrixf(matrix);
	glPushMatrix();
	glMultMatrixf(matrix_1);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix_1);
	glPopMatrix();
	glMultMatrixf(matrix_2);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix_2);
	glPopMatrix();
	bbox->bbmin[X] = min2f(min2f(matrix_1[0], matrix_1[4]), min2f(matrix_1[8], matrix_1[12]));
	bbox->bbmin[X] = min2f(bbox->bbmin[X], min2f(min2f(matrix_2[0], matrix_2[4]), min2f(matrix_2[8], matrix_2[12])));
	bbox->bbmin[Y] = min2f(min2f(matrix_1[1], matrix_1[5]), min2f(matrix_1[9], matrix_1[13]));
	bbox->bbmin[Y] = min2f(bbox->bbmin[Y], min2f(min2f(matrix_2[1], matrix_2[5]), min2f(matrix_2[9], matrix_2[13])));
	bbox->bbmin[Z] = min2f(min2f(matrix_1[2], matrix_1[6]), min2f(matrix_1[10], matrix_1[14]));
	bbox->bbmin[Z] = min2f(bbox->bbmin[Z], min2f(min2f(matrix_2[2], matrix_2[6]), min2f(matrix_2[10], matrix_2[14])));

	bbox->bbmax[X] = max2f(max2f(matrix_1[0], matrix_1[4]), max2f(matrix_1[8], matrix_1[12]));
	bbox->bbmax[X] = max2f(bbox->bbmax[X], max2f(max2f(matrix_2[0], matrix_2[4]), max2f(matrix_2[8], matrix_2[12])));
	bbox->bbmax[Y] = max2f(max2f(matrix_1[1], matrix_1[5]), max2f(matrix_1[9], matrix_1[13]));
	bbox->bbmax[Y] = max2f(bbox->bbmax[Y], max2f(max2f(matrix_2[1], matrix_2[5]), max2f(matrix_2[9], matrix_2[13])));
	bbox->bbmax[Z] = max2f(max2f(matrix_1[2], matrix_1[6]), max2f(matrix_1[10], matrix_1[14]));
	bbox->bbmax[Z] = max2f(bbox->bbmax[Z], max2f(max2f(matrix_2[2], matrix_2[6]), max2f(matrix_2[10], matrix_2[14])));
}

static __inline__ void get_intersect_start_stop(BBOX_TREE* bbox_tree, unsigned int type, unsigned int* start, unsigned int* stop)
{
	unsigned int idx;

	idx = bbox_tree->cur_intersect_type;
	*start = bbox_tree->intersect[idx].start[type];
	*stop = bbox_tree->intersect[idx].stop[type];
}

static __inline__ unsigned int get_intersect_item_ID(BBOX_TREE* bbox_tree, unsigned int index)
{
	unsigned int idx;

	idx = bbox_tree->cur_intersect_type;
	return bbox_tree->intersect[idx].items[index].ID;
}

static __inline__ unsigned int get_cur_intersect_type(BBOX_TREE* bbox_tree)
{
	return bbox_tree->cur_intersect_type;
}

static __inline__ void set_cur_intersect_type(BBOX_TREE* bbox_tree, unsigned int intersec_type)
{
	bbox_tree->cur_intersect_type = intersec_type;
}

static __inline__ unsigned int get_terrain_id(unsigned int x, unsigned int y)
{
	return (y << 8) + x;
}

static __inline__ unsigned int get_terrain_x(unsigned int ID)
{
	return ID & 0xFF;
}

static __inline__ unsigned int get_terrain_y(unsigned int ID)
{
	return ID >> 8;
}

static __inline__ unsigned int is_blend_3d_object(unsigned int type)
{
	switch (type)
	{
		case TYPE_3D_BLEND_GROUND_ALPHA_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_BLEND_GROUND_ALPHA_NO_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_BLEND_GROUND_NO_ALPHA_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_BLEND_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_BLEND_NO_GROUND_ALPHA_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_BLEND_NO_GROUND_ALPHA_NO_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_BLEND_NO_GROUND_NO_ALPHA_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_BLEND_NO_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_NO_BLEND_GROUND_ALPHA_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_NO_BLEND_GROUND_ALPHA_NO_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_NO_BLEND_GROUND_NO_ALPHA_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_NO_BLEND_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_NO_BLEND_NO_GROUND_ALPHA_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_NO_BLEND_NO_GROUND_ALPHA_NO_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_NO_BLEND_NO_GROUND_NO_ALPHA_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_NO_BLEND_NO_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT: return 0;
		default: return 0;
	}
}

static __inline__ unsigned int is_ground_3d_object(unsigned int type)
{
	switch (type)
	{
		case TYPE_3D_BLEND_GROUND_ALPHA_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_BLEND_GROUND_ALPHA_NO_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_BLEND_GROUND_NO_ALPHA_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_BLEND_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_BLEND_NO_GROUND_ALPHA_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_BLEND_NO_GROUND_ALPHA_NO_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_BLEND_NO_GROUND_NO_ALPHA_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_BLEND_NO_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_NO_BLEND_GROUND_ALPHA_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_NO_BLEND_GROUND_ALPHA_NO_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_NO_BLEND_GROUND_NO_ALPHA_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_NO_BLEND_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_NO_BLEND_NO_GROUND_ALPHA_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_NO_BLEND_NO_GROUND_ALPHA_NO_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_NO_BLEND_NO_GROUND_NO_ALPHA_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_NO_BLEND_NO_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT: return 0;
		default: return 0;
	}
}

static __inline__ unsigned int is_alpha_3d_object(unsigned int type)
{
	switch (type)
	{
		case TYPE_3D_BLEND_GROUND_ALPHA_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_BLEND_GROUND_ALPHA_NO_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_BLEND_GROUND_NO_ALPHA_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_BLEND_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_BLEND_NO_GROUND_ALPHA_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_BLEND_NO_GROUND_ALPHA_NO_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_BLEND_NO_GROUND_NO_ALPHA_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_BLEND_NO_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_NO_BLEND_GROUND_ALPHA_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_NO_BLEND_GROUND_ALPHA_NO_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_NO_BLEND_GROUND_NO_ALPHA_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_NO_BLEND_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_NO_BLEND_NO_GROUND_ALPHA_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_NO_BLEND_NO_GROUND_ALPHA_NO_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_NO_BLEND_NO_GROUND_NO_ALPHA_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_NO_BLEND_NO_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT: return 0;
		default: return 0;
	}
}

static __inline__ unsigned int is_self_lit_3d_object(unsigned int type)
{
	switch (type)
	{
		case TYPE_3D_BLEND_GROUND_ALPHA_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_BLEND_GROUND_ALPHA_NO_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_BLEND_GROUND_NO_ALPHA_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_BLEND_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_BLEND_NO_GROUND_ALPHA_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_BLEND_NO_GROUND_ALPHA_NO_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_BLEND_NO_GROUND_NO_ALPHA_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_BLEND_NO_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_NO_BLEND_GROUND_ALPHA_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_NO_BLEND_GROUND_ALPHA_NO_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_NO_BLEND_GROUND_NO_ALPHA_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_NO_BLEND_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_NO_BLEND_NO_GROUND_ALPHA_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_NO_BLEND_NO_GROUND_ALPHA_NO_SELF_LIT_OBJECT: return 0;
		case TYPE_3D_NO_BLEND_NO_GROUND_NO_ALPHA_SELF_LIT_OBJECT: return 1;
		case TYPE_3D_NO_BLEND_NO_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT: return 0;
		default: return 0;
	}
}

/*!
 * \ingroup misc
 * \brief Checks which objects of the bounding-box-tree are in the frustum.
 *
 * Checks which objects of the axis-aligned-bounding-box-tree are in the frustum.
 *
 * \param bbox_tree	The bounding-box-tree holding the objects.
 * \param frustum	The frustum, mostly the view-frustum.
 * \param mask		The mask used with the frustum.
 *
 * \callgraph
 */
void check_bbox_tree(BBOX_TREE* bbox_tree, const FRUSTUM frustum, unsigned int mask);

/*!
 * \ingroup misc
 * \brief Frees the given bounding-box-tree.
 *
 * Frees the given bounding-box-tree.
 *
 * \param bbox_tree	The bounding-box-tree.
 *
 * \callgraph
 */
void free_bbox_tree(BBOX_TREE* bbox_tree);

/*!
 * \ingroup misc
 * \brief Clears the given bounding-box-tree.
 *
 * Clears the data of the given bounding-box-tree without 
 * freeing the bounding boxtree.
 *
 * \param bbox_tree	The bounding-box-tree.
 *
 * \callgraph
 */
void clear_bbox_tree(BBOX_TREE* bbox_tree);

/*!
 * \ingroup misc
 * \brief Creates a bounding-box-tree.
 *
 * Creates an empty bounding-box-tree.
 *
 * \retval BBOX_TREE	The bounding-box-tree.
 * \callgraph
 */
BBOX_TREE* build_bbox_tree();

/*!
 * \ingroup misc
 * \brief Inits a bounding-box-tree.
 *
 * Inits a bounding-box-tree from a list of static objects.
 *
 * \param bbox_items	The list of the static objects.
 * \param bbox_tree	The bounding-box-tree.
 * \callgraph
 */
void init_bbox_tree(BBOX_TREE* bbox_tree, BBOX_ITEMS *bbox_items);

/*!
 * \ingroup misc
 * \brief Adds a static light to a list of static objects.
 *
 * Adds a static light to a list of static objects.
 *
 * \param bbox_items	The list of the static objects.
 * \param ID		The ID of the static light.
 * \param bbox		The bounding box of the static light.
 * \callgraph
 */
void add_light_to_list(BBOX_ITEMS *bbox_items, unsigned int ID, const AABBOX bbox);

/*!
 * \ingroup misc
 * \brief Adds a static 3d object to a list of static objects.
 *
 * Adds a static 3d object to a list of static objects.
 *
 * \param bbox_items	The list of the static objects.
 * \param ID		The ID of the static 3d object.
 * \param bbox		The bounding box of the static 3d object.
 * \param blend		Is this a blend object?
 * \param ground	Is this a ground object?
 * \callgraph
 */
void add_3dobject_to_list(BBOX_ITEMS *bbox_items, unsigned int ID, const AABBOX bbox, unsigned int blend, unsigned int ground, unsigned int alpha, unsigned int self_lit, unsigned int texture_id, const MD5_DIGEST md5);

/*!
 * \ingroup misc
 * \brief Adds a static 2d object to a list of static objects.
 *
 * Adds a static 2d object to a list of static objects.
 *
 * \param bbox_items	The list of the static objects.
 * \param ID		The ID of the static 2d object.
 * \param bbox		The bounding box of the static 2d object.
 * \param alpha		Is this an alpha object?
 * \callgraph
 */
void add_2dobject_to_list(BBOX_ITEMS *bbox_items, unsigned int ID, const AABBOX bbox, unsigned int alpha, unsigned int texture_id);

/*!
 * \ingroup misc
 * \brief Adds a static particle system to a list of static objects.
 *
 * Adds a static particle system to a list of static objects.
 *
 * \param bbox_items	The list of the static objects.
 * \param ID		The ID of the static particle system.
 * \param bbox		The bounding box of the static particle system.
 * \param sblend	The sblend value of the static particle system.
 * \param dblend	The dblend value of the static particle system.
 * \callgraph
 */
void add_particle_sys_to_list(BBOX_ITEMS *bbox_items, unsigned int ID, const AABBOX bbox, unsigned int sblend, unsigned int dblend);

/*!
 * \ingroup misc
 * \brief Adds a static terrain tile to a list of static objects.
 *
 * Adds a static terrain tile to a list of static objects.
 *
 * \param bbox_items	The list of the static objects.
 * \param ID		The ID of the static terrain tile.
 * \param bbox		The bounding box of the static terrain tile.
 * \param texture_id	The ID of the texture_id.
 * \callgraph
 */
void add_terrain_to_list(BBOX_ITEMS *bbox_items, unsigned int ID, const AABBOX bbox, unsigned int texture_id);

/*!
 * \ingroup misc
 * \brief Adds a static water tile to a list of static objects.
 *
 * Adds a static water tile to a list of static objects.
 *
 * \param bbox_items	The list of the static objects.
 * \param ID		The ID of the static water tile.
 * \param bbox		The bounding box of the static water tile.
 * \param reflectiv	Is the tile reflectiv.
 * \param texture_id	The ID of the texture_id.
 * \callgraph
 */
void add_water_to_list(BBOX_ITEMS *bbox_items, unsigned int ID, const AABBOX bbox, unsigned int reflectiv, unsigned int texture_id);

/*!
 * \ingroup misc
 * \brief Adds a 3d object to the bounding-box-tree.
 *
 * Adds a 3d object to the bounding-box-tree.
 *
 * \param bbox_tree	The bounding-box-tree.
 * \param ID		The ID of the 3d object.
 * \param bbox		The bounding box of the dynamic 3d object.
 * \param blend		Is this a blend object?
 * \param ground	Is this a ground object?
 * \param dynamic	Is this a dynamic object?
 * \callgraph
 */
void add_3dobject_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, const AABBOX bbox, unsigned int blend, unsigned int ground, unsigned int alpha, unsigned int self_lit, unsigned int texture_id, const MD5_DIGEST md5, unsigned int dynamic);

/*!
 * \ingroup misc
 * \brief Adds a 2d object to the bounding-box-tree.
 *
 * Adds a 2d object to the bounding-box-tree.
 *
 * \param bbox_tree	The bounding-box-tree.
 * \param ID		The ID of the 3d object.
 * \param bbox		The bounding box of the dynamic 2d object.
 * \param alpha		Is this an alpha object?
 * \param dynamic	Is this a dynamic object?
 * \callgraph
 */
void add_2dobject_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, const AABBOX bbox, unsigned int alpha, unsigned int texture_id, unsigned int dynamic);

/*!
 * \ingroup misc
 * \brief Adds a particle system to the bounding-box-tree.
 *
 * Adds a particle system to the bounding-box-tree.
 *
 * \param bbox_tree	The bounding-box-tree.
 * \param ID		The ID of the particle system.
 * \param bbox		The bounding box of the dynamic particle system.
 * \param sblend	The sblend value of the dynamic particle system.
 * \param dblend	The dblend value of the dynamic particle system.
 * \param dynamic	Is this a dynamic object?
 * \callgraph
 */
void add_particle_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, const AABBOX bbox, unsigned int sblend, unsigned int dblend, unsigned int dynamic);

/*!
 * \ingroup misc
 * \brief Adds a light to the bounding-box-tree.
 *
 * Adds a light to the bounding-box-tree.
 *
 * \param bbox_tree	The bounding-box-tree.
 * \param ID		The ID of the light.
 * \param bbox		The bounding box of the light.
 * \param dynamic	Is this a dynamic object?
 * \callgraph
 */
void add_light_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, const AABBOX bbox, unsigned int dynamic);

/*!
 * \ingroup misc
 * \brief Adds a terrain tile to the bounding-box-tree.
 *
 * Adds a terrain tile to the bounding-box-tree.
 *
 * \param bbox_tree	The bounding-box-tree.
 * \param ID		The ID of the terrain tile.
 * \param bbox		The bounding box of the terrain tile.
 * \param texture_id	The ID of the texture_id.
 * \callgraph
 */
void add_terrain_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, const AABBOX bbox, unsigned int texture_id, unsigned int dynamic);

/*!
 * \ingroup misc
 * \brief Adds a water tile to the bounding-box-tree.
 *
 * Adds a water tile to the bounding-box-tree.
 *
 * \param bbox_tree	The bounding-box-tree.
 * \param ID		The ID of the water tile.
 * \param bbox		The bounding box of the water tile.
 * \param reflectiv	Is the tile reflectiv.
 * \param texture_id	The ID of the texture_id.
 * \callgraph
 */
void add_water_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, const AABBOX bbox, unsigned int reflectiv, unsigned int texture_id, unsigned int dynamic);

/*!
 * \ingroup misc
 * \brief Deletes a 3d object from the bounding-box-tree.
 *
 * Deletes a 3d object from the bounding-box-tree.
 *
 * \param bbox_tree	The bounding-box-tree.
 * \param ID		The ID of the 3d object.
 * \param blend		Is this a blend object?
 * \param ground	Is this a ground object?
 * \param dynamic	Is this a dynamic object?
 * \callgraph
 */
void delete_3dobject_from_abt(BBOX_TREE *bbox_tree, unsigned int ID, unsigned int blend, unsigned int ground, unsigned int alpha, unsigned int self_lit);

/*!
 * \ingroup misc
 * \brief Deletes a 2d object from the bounding-box-tree.
 *
 * Deletes a 2d object from the bounding-box-tree.
 *
 * \param bbox_tree	The bounding-box-tree.
 * \param ID		The ID of the 2d object.
 * \param alpha		Is this an alpha object?
 * \param dynamic	Is this a dynamic object?
 * \callgraph
 */
void delete_2dobject_from_abt(BBOX_TREE *bbox_tree, unsigned int ID, unsigned int alpha);

/*!
 * \ingroup misc
 * \brief Deletes a particle system from the bounding-box-tree.
 *
 * Deletes a particle system from the bounding-box-tree.
 *
 * \param bbox_tree	The bounding-box-tree.
 * \param ID		The ID of the particle system.
 * \param dynamic	Is this a dynamic object?
 * \callgraph
 */
void delete_particle_from_abt(BBOX_TREE *bbox_tree, unsigned int ID);

/*!
 * \ingroup misc
 * \brief Deletes a light from the bounding-box-tree.
 *
 * Deletes a light from the bounding-box-tree.
 *
 * \param bbox_tree	The bounding-box-tree.
 * \param ID		The ID of the light.
 * \param dynamic	Is this a dynamic object?
 * \callgraph
 */
void delete_light_from_abt(BBOX_TREE *bbox_tree, unsigned int ID);

/*!
 * \ingroup misc
 * \brief Deletes a terrain tile from the bounding-box-tree.
 *
 * Deletes a terrain tile from the bounding-box-tree.
 *
 * \param bbox_tree	The bounding-box-tree.
 * \param ID		The ID of the terrain tile.
 * \param dynamic	Is this a dynamic object?
 * \callgraph
 */
void delete_terrain_from_abt(BBOX_TREE *bbox_tree, unsigned int ID);

/*!
 * \ingroup misc
 * \brief Deletes a water tile from the bounding-box-tree.
 *
 * Deletes a water tile from the bounding-box-tree.
 *
 * \param bbox_tree	The bounding-box-tree.
 * \param ID		The ID of the water tile.
 * \param reflectiv	Is the tile reflectiv.
 * \param dynamic	Is this a dynamic object?
 * \callgraph
 */
void delete_water_from_abt(BBOX_TREE *bbox_tree, unsigned int ID, unsigned int reflectiv);

/*!
 * \ingroup misc
 * \brief Creates a list for static objects.
 *
 * Creates a list for static objects.
 *
 * \param size		The minimum size of the list for the static objects.
 * \retval BBOX_ITEMS	The list for the static objects.
 * \callgraph
 */
BBOX_ITEMS* create_bbox_items(unsigned int size);

/*!
 * \ingroup misc
 * \brief Frees the list for static objects.
 *
 * Frees the list for static objects.
 *
 * \param bbox_items	The list of the static objects.
 * \callgraph
 */
void free_bbox_items(BBOX_ITEMS* bbox_items);

/*!
 * \ingroup misc
 * \brief Sets all intersection lists to update needed.
 *
 * Sets all intersection lists to update needed.
 *
 * \param bbox_tree	The bounding box tree of the intersection list.
 * \callgraph
 */
void set_all_intersect_update_needed(BBOX_TREE* bbox_tree);

/*!
 * \ingroup misc
 * \brief .
 *
 * .
 *
 * \param bbox_tree	The bounding-box-tree holding the objects.
 * \param frustum	The frustum.
 * \param bbox		The bbox of the objects in the frustum.
 *
 * \callgraph
 */
void calc_scene_bbox(BBOX_TREE* bbox_tree, const FRUSTUM frustum, AABBOX* bbox);

extern BBOX_TREE* main_bbox_tree;
extern BBOX_ITEMS* main_bbox_tree_items;

int aabb_in_frustum(const AABBOX bbox);
void calculate_light_frustum(FRUSTUM frustum, double* modl, double* proj);

/*!
 * \ingroup misc
 * \brief   Checks if the box intersect with the click line.
 *
 *      Checks if the bounding box intersect with the click line.
 *
 * \param bbox      bounding box
 * \retval int      1 (true), if intersect, else 0 (false).
 * \callgraph
 */
int click_line_bbox_intersection(const AABBOX bbox);

/*!
 * \ingroup misc
 * \brief   Set the click line.
 *
 *      Set click line.
 *
 * \callgraph
 */
void set_click_line();

extern LINE click_line;
#endif
#endif
