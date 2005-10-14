#ifdef	NEW_FRUSTUM
#ifndef	BBOX_TREE_H
#define BBOX_TREE_H

typedef float VECTOR3[3];
typedef float VECTOR4[4];

typedef struct
{
	VECTOR4 bbmin;
	VECTOR4 bbmax;
} AABBOX;

typedef	struct
{
	VECTOR4 	plane;
	unsigned int 	mask[8];
} PLANE;

typedef PLANE FRUSTUM[6];

typedef struct
{
	AABBOX		bbox;
	unsigned int	type;
	unsigned int	ID;
} BBOX_ITEM;

typedef struct
{
	unsigned int	size;
	unsigned int	index;
	BBOX_ITEM*	items;
} BBOX_ITEMS;	

typedef struct BBox_Tree_Struct BBOX_TREE_NODE;

struct BBox_Tree_Struct
{
	AABBOX		bbox;
	BBOX_TREE_NODE*	nodes[2];
	unsigned int	items_index;
	unsigned int	items_count;
};

typedef	struct
{
	BBOX_TREE_NODE*	root_node;
	unsigned int	items_count;
	BBOX_ITEM*	items;
	unsigned int	nodes_count;
	BBOX_TREE_NODE*	nodes;	
	unsigned int	intersect_index;
	BBOX_ITEM*	intersect_items;
} BBOX_TREE;

#define	TYPE_LIGHT	0x0001
#define TYPE_3D_OBJECT	0x0002
#define TYPE_2D_OBJECT	0x0003

#define	OUTSIDE		0x0000
#define	INSIDE		0x0001
#define INTERSECT	0x0002

#define BOUND_HUGE	10e30

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

static __inline__ float min(const float f1, const float f2)
{
	if (f1 < f2) return f1;
	else return f2;
}

static __inline__ float max(const float f1, const float f2)
{
	if (f1 > f2) return f1;
	else return f2;
}

static __inline__ void VMin4(VECTOR4 v1, const VECTOR4 v2, const VECTOR4 v3)
{
	v1[X] = min(v2[X], v3[X]);
	v1[Y] = min(v2[Y], v3[Y]);
	v1[Z] = min(v2[Z], v3[Z]);
}

static __inline__ void VMax4(VECTOR4 v1, const VECTOR4 v2, const VECTOR4 v3)
{
	v1[X] = max(v2[X], v3[X]);
	v1[Y] = max(v2[Y], v3[Y]);
	v1[Z] = max(v2[Z], v3[Z]);
}

static __inline__ void VSub4(VECTOR4 v1, const VECTOR4 v2, const VECTOR4 v3)
{
	v1[X] = v2[X] - v3[X];
	v1[Y] = v2[Y] - v3[Y];
	v1[Z] = v2[Z] - v3[Z];
}

static __inline__ void Make_Vector4(VECTOR4 v1, const float f1, const float f2, const float f3, const float f4)
{
	v1[X] = f1;
	v1[Y] = f2;
	v1[Z] = f3;
	v1[W] = f4;
}

static __inline__ void VAssign4(VECTOR4 v1, const VECTOR3 v2, const float vw)
{
	memcpy(v1, v2, sizeof(VECTOR3));
	v1[W] = vw;
}

void check_bbox_tree(BBOX_TREE* bbox_tree, FRUSTUM *frustum);
void free_bbox_tree(BBOX_TREE* bbox_tree);
BBOX_TREE* build_bbox_tree(BBOX_ITEMS *bbox_items);
void add_light_to_abt(BBOX_ITEMS *bbox_items, unsigned int ID, float pos_x, float pos_y, float pos_z, float diff_r, float diff_g, float diff_b, float att, int exp, float clamp);
void add_3dobject_to_abt(BBOX_ITEMS *bbox_items, unsigned int ID, AABBOX *bbox);
void add_2dobject_to_abt(BBOX_ITEMS *bbox_items, unsigned int ID, AABBOX *bbox);
BBOX_ITEMS* create_bbox_items(unsigned int size);
void free_bbox_items(BBOX_ITEMS* bbox_items);

extern FRUSTUM frustum;
extern BBOX_TREE* bbox_tree;

#endif
#endif
