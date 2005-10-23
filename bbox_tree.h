#ifdef	NEW_FRUSTUM
#ifndef	BBOX_TREE_H
#define BBOX_TREE_H

#include <math.h>
#include "global.h"
#include "misc.h"

#define TYPE_2D_NO_ALPHA_OBJECT				0x00
#define TYPE_2D_ALPHA_OBJECT				0x01
#define TYPE_3D_NO_BLEND_NO_GOUND_OBJECT		0x02
#define TYPE_3D_NO_BLEND_GROUND_OBJECT			0x03
#define TYPE_3D_BLEND_NO_GROUND_OBJECT			0x04
#define TYPE_3D_BLEND_GROUND_OBJECT			0x05
#define TYPE_PARTICLE_SYSTEM				0x06
#define	TYPE_LIGHT					0x07
#define	TYPE_WATER					0x08
#define	TYPE_TERRAIN					0x09
#define	TYPES_COUNT					0x0A
#define TYPE_DELETED					0xFF

#define	ITERSECTION_TYPES_DEFAULT			0x00
#define	ITERSECTION_TYPES_SHADOW			0x01
#define	ITERSECTION_TYPES_REFLECTION			0x02
#define	MAX_ITERSECTION_TYPES				0x03

#define	OUTSIDE		0x0000
#define	INSIDE		0x0001
#define INTERSECT	0x0002

#define BOUND_HUGE	10e30

typedef float VECTOR3[3];
typedef float VECTOR4[4];
typedef unsigned int IDX_TYPE;

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

typedef struct
{
	IDX_TYPE		size;
	IDX_TYPE		count;
	IDX_TYPE		start[TYPES_COUNT];
	IDX_TYPE		stop[TYPES_COUNT];
	BBOX_ITEM_DATA*		items;
} BBOX_INTERSECTION_DATA;

#ifdef	FRUSTUM_THREADS
typedef	struct
{
	BBOX_ITEM		item;
	unsigned char		dynamic;
} BBOX_UPDATE_ITEM_DATA;

typedef struct
{
	unsigned int		bbox_tree_degeneration;
	IDX_TYPE		size;
	IDX_TYPE		index;
	BBOX_UPDATE_ITEM_DATA*	list;
} BBOX_TREE_UPDATE_DATA;
#endif

typedef	struct
{
	BBOX_TREE_NODE*		root_node;
	IDX_TYPE		items_count;
	BBOX_ITEM*		items;
	IDX_TYPE		nodes_count;
	BBOX_TREE_NODE*		nodes;
	unsigned short		cur_intersect_type;
	BBOX_INTERSECTION_DATA	intersect[MAX_ITERSECTION_TYPES];
	SDL_mutex*		bbox_tree_mutex;
#ifdef	FRUSTUM_THREADS
	SDL_cond*		update_condition;
	BBOX_TREE_UPDATE_DATA	update_data;
	unsigned char		done;
	SDL_Thread*		thread_id;
#endif
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

static __inline__ void VMin(VECTOR3 v1, const VECTOR3 v2, const VECTOR3 v3)
{
	v1[X] = min2f(v2[X], v3[X]);
	v1[Y] = min2f(v2[Y], v3[Y]);
	v1[Z] = min2f(v2[Z], v3[Z]);
}

static __inline__ void VMax(VECTOR3 v1, const VECTOR3 v2, const VECTOR3 v3)
{
	v1[X] = max2f(v2[X], v3[X]);
	v1[Y] = max2f(v2[Y], v3[Y]);
	v1[Z] = max2f(v2[Z], v3[Z]);
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

static __inline__ int lock_bbox_tree(BBOX_TREE* bbox_tree)
{
	return SDL_LockMutex(bbox_tree->bbox_tree_mutex);
}

static __inline__ int unlock_bbox_tree(BBOX_TREE* bbox_tree)
{
	return SDL_UnlockMutex(bbox_tree->bbox_tree_mutex);
}

/*!
 * \ingroup misc
 * \brief Checks which objects of the bounding-box-tree are in the frustum.
 *
 * Checks which objects of the axis-aligned-bounding-box-tree are in the frustum.
 *
 * \param bbox_tree	The bounding-box-tree holding the objects.
 * \param frustum	The frustum, mostly the view-frustum.
 *
 * \callgraph
 */
void check_bbox_tree(BBOX_TREE* bbox_tree, FRUSTUM *frustum);

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
 * \param bbox	The bounding box of the static light.
 * \callgraph
 */
void add_light_to_list(BBOX_ITEMS *bbox_items, unsigned int ID, AABBOX *bbox);

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
void add_3dobject_to_list(BBOX_ITEMS *bbox_items, unsigned int ID, AABBOX *bbox, unsigned int blend, unsigned int ground);

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
void add_2dobject_to_list(BBOX_ITEMS *bbox_items, unsigned int ID, AABBOX *bbox, unsigned int alpha);

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
void add_particle_sys_to_list(BBOX_ITEMS *bbox_items, unsigned int ID, AABBOX *bbox, unsigned int sblend, unsigned int dblend);

/*!
 * \ingroup misc
 * \brief Adds a dynamic 3d object to the bounding-box-tree.
 *
 * Adds a dynamic 3d object to the bounding-box-tree.
 *
 * \param bbox_tree	The bounding-box-tree.
 * \param ID		The ID of the dynamic 3d object.
 * \param bbox		The bounding box of the dynamic 3d object.
 * \param blend		Is this a blend object?
 * \param ground	Is this a ground object?
 * \param dynamic	Is this a dynamic object?
 * \callgraph
 */
void add_3dobject_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, AABBOX *bbox, unsigned int blend, unsigned int ground, unsigned int dynamic);

/*!
 * \ingroup misc
 * \brief Adds a dynamic 2d object to the bounding-box-tree.
 *
 * Adds a dynamic 2d object to the bounding-box-tree.
 *
 * \param bbox_tree	The bounding-box-tree.
 * \param ID		The ID of the dynamic 3d object.
 * \param bbox		The bounding box of the dynamic 2d object.
 * \param alpha		Is this an alpha object?
 * \param dynamic	Is this a dynamic object?
 * \callgraph
 */
void add_2dobject_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, AABBOX *bbox, unsigned int alpha, unsigned int dynamic);

/*!
 * \ingroup misc
 * \brief Adds a dynamic particle system to the bounding-box-tree.
 *
 * Adds a dynamic particle system to the bounding-box-tree.
 *
 * \param bbox_tree	The bounding-box-tree.
 * \param ID		The ID of the dynamic particle system.
 * \param bbox		The bounding box of the dynamic particle system.
 * \param sblend	The sblend value of the dynamic particle system.
 * \param dblend	The dblend value of the dynamic particle system.
 * \param dynamic	Is this a dynamic object?
 * \callgraph
 */
void add_particle_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, AABBOX *bbox, unsigned int sblend, unsigned int dblend, unsigned int dynamic);

/*!
 * \ingroup misc
 * \brief Adds a dynamic light to the bounding-box-tree.
 *
 * Adds a dynamic light to the bounding-box-tree.
 *
 * \param bbox_tree	The bounding-box-tree.
 * \param ID		The ID of the dynamic light.
 * \param bbox		The bounding box of the dynamic light.
 * \param dynamic	Is this a dynamic object?
 * \callgraph
 */
void add_light_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, AABBOX *bbox, unsigned int dynamic);

/*!
 * \ingroup misc
 * \brief Deletes a dynamic 3d object to the bounding-box-tree.
 *
 * Deletes a dynamic 3d object to the bounding-box-tree.
 *
 * \param bbox_tree	The bounding-box-tree.
 * \param ID		The ID of the dynamic 3d object.
 * \param blend		Is this a blend object?
 * \param ground	Is this a ground object?
 * \param dynamic	Is this a dynamic object?
 * \callgraph
 */
void delete_3dobject_from_abt(BBOX_TREE *bbox_tree, unsigned int ID, unsigned int blend, unsigned int ground, unsigned int dynamic);

/*!
 * \ingroup misc
 * \brief Deletes a dynamic 2d object to the bounding-box-tree.
 *
 * Deletes a dynamic 2d object to the bounding-box-tree.
 *
 * \param bbox_tree	The bounding-box-tree.
 * \param ID		The ID of the dynamic 3d object.
 * \param alpha		Is this an alpha object?
 * \param dynamic	Is this a dynamic object?
 * \callgraph
 */
void delete_2dobject_from_abt(BBOX_TREE *bbox_tree, unsigned int ID, unsigned int alpha, unsigned int dynamic);

/*!
 * \ingroup misc
 * \brief Deletes a dynamic particle system to the bounding-box-tree.
 *
 * Deletes a dynamic particle system to the bounding-box-tree.
 *
 * \param bbox_tree	The bounding-box-tree.
 * \param ID		The ID of the dynamic particle system.
 * \param dynamic	Is this a dynamic object?
 * \callgraph
 */
void delete_particle_from_abt(BBOX_TREE *bbox_tree, unsigned int ID, unsigned int dynamic);

/*!
 * \ingroup misc
 * \brief Deletes a dynamic light to the bounding-box-tree.
 *
 * Deletes a dynamic light to the bounding-box-tree.
 *
 * \param bbox_tree	The bounding-box-tree.
 * \param ID		The ID of the dynamic light.
 * \param dynamic	Is this a dynamic object?
 * \callgraph
 */
void delete_light_from_abt(BBOX_TREE *bbox_tree, unsigned int ID, unsigned int dynamic);

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

extern FRUSTUM main_frustum;
extern BBOX_TREE* main_bbox_tree;
extern BBOX_ITEMS* main_bbox_tree_items;

#endif
#endif
