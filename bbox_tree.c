#include "bbox_tree.h"
#include "draw_scene.h"
#include "lights.h"
#ifdef EXTRA_DEBUG
#include "errors.h"
#endif

#ifdef OSX
#include <sys/malloc.h>
#else
#ifndef	BSD
#ifndef alloca         // newer versions of SDL have their own alloca!
#include <malloc.h>
#endif   //alloca
#endif   //BSD
#endif   //OSX

#ifdef CLUSTER_INSIDES
#include "cluster.h"
#endif // CLUSTER_INSIDES

BBOX_TREE* main_bbox_tree = NULL;
BBOX_ITEMS* main_bbox_tree_items = NULL;

#ifdef	EXTRA_DEBUG
#define BBOX_TREE_LOG_INFO(item)	log_error_detailed("%s is NULL", __FILE__, __FUNCTION__, __LINE__, item);
#else	//DEBUG
#define BBOX_TREE_LOG_INFO(item)	/*!< NOP */
#endif	//DEBUG

#define	NO_INDEX	0xFFFFFFFF

static __inline__ Uint32 is_extra_first_sub_node(Uint32 extra)
{
	return ((extra & EXTRA_FIRST_SUB_NODE) != 0);
}

void set_all_intersect_update_needed(BBOX_TREE* bbox_tree)
{
	Uint32 i;

	if (bbox_tree != NULL)
	{
		for (i = 0; i < MAX_INTERSECTION_TYPES; i++)
			bbox_tree->intersect[i].intersect_update_needed = 1;
	}
	else BBOX_TREE_LOG_INFO("bbox_tree");
}

static __inline__ void adapt_intersect_list_size(BBOX_TREE* bbox_tree, Uint32 count)
{
	Uint32 size, idx;

	if (count == 0) count = 1;
	idx = bbox_tree->cur_intersect_type;
	size = bbox_tree->intersect[idx].size;

	if ((bbox_tree->intersect[idx].count+count) >= size)
	{
		size += max2i(count, size/2);
		bbox_tree->intersect[idx].items = (BBOX_ITEM*)realloc(bbox_tree->intersect[idx].items, size*sizeof(BBOX_ITEM));
		bbox_tree->intersect[idx].size = size;
	}
}

static __inline__ void add_intersect_item_to_list(BBOX_TREE* bbox_tree, BBOX_ITEM* item, Uint32 idx)
{
#ifdef CLUSTER_INSIDES
	if (item->cluster && item->cluster != current_cluster) return;
#endif // CLUSTER_INSIDES
	adapt_intersect_list_size(bbox_tree, 1);
	memcpy(&bbox_tree->intersect[idx].items[bbox_tree->intersect[idx].count], item, sizeof(BBOX_ITEM));
	bbox_tree->intersect[idx].count++;
}

static __inline__ void add_intersect_item(BBOX_TREE* bbox_tree, Uint32 index, Uint32 idx)
{
	add_intersect_item_to_list(bbox_tree, &bbox_tree->items[index], idx);
}

static __inline__ void add_intersect_items(BBOX_TREE* bbox_tree, Uint32 index, Uint32 count)
{
	Uint32 i;

	adapt_intersect_list_size(bbox_tree, count);
	for (i = 0; i < count; i++) add_intersect_item(bbox_tree, index+i, bbox_tree->cur_intersect_type);
}

static __inline__ void add_dyn_intersect_item(BBOX_TREE* bbox_tree, Uint32 node, Uint32 index, Uint32 idx)
{
	add_intersect_item_to_list(bbox_tree, &bbox_tree->nodes[node].dynamic_objects.items[index], idx);
}

static __inline__ void add_dyn_intersect_items(BBOX_TREE* bbox_tree, Uint32 node, Uint32 count)
{
	Uint32 i;

	adapt_intersect_list_size(bbox_tree, count);
	for (i = 0; i < count; i++) add_dyn_intersect_item(bbox_tree, node, i, bbox_tree->cur_intersect_type);
}

static __inline__ int check_aabb_in_frustum(const AABBOX bbox, const FRUSTUM frustum, Uint32 in_mask, Uint32 *out_mask)
{
	VECTOR4 n, p;
	VECTOR3 _n, _p;
	float v;
	Uint32 i, k, result;

	result = INSIDE;
	*out_mask = 0;

	for (i = 0, k = 1; k <= in_mask; i++, k += k)
	{
		if (k & in_mask)
		{
			VInvertSelect(_n, bbox.bbmin, bbox.bbmax, frustum[i].mask);
			VAssign4(n, _n, 1.0f);
			v = VDot4(n, frustum[i].plane);
			if (v < 0.0f)
			{
				return OUTSIDE;
			}

			VSelect(_p, bbox.bbmin, bbox.bbmax, frustum[i].mask);
			VAssign4(p, _p, 1.0f);
			v = VDot4(p, frustum[i].plane);
			if (v < 0.0f)
			{
				*out_mask |= k;
				result = INTERSECT;
			}
		}
	}

	return result;
}

static __inline__ int check_aabb_in_frustum_no_out_mask(const AABBOX bbox, const FRUSTUM frustum, Uint32 in_mask)
{
	VECTOR4 n, p;
	VECTOR3 _n, _p;
	float v;
	Uint32 i, k;

	for (i = 0, k = 1; k <= in_mask; i++, k += k)
	{
		if (k & in_mask)
		{
			VInvertSelect(_n, bbox.bbmin, bbox.bbmax, frustum[i].mask);
			VAssign4(n, _n, 1.0f);
			v = VDot4(n, frustum[i].plane);
			if (v < 0.0f)
			{
				return OUTSIDE;
			}

			VSelect(_p, bbox.bbmin, bbox.bbmax, frustum[i].mask);
			VAssign4(p, _p, 1.0f);
			v = VDot4(p, frustum[i].plane);
			if (v < 0.0f)
			{
				return INTERSECT;
			}
		}
	}

	return INSIDE;
}

static __inline__ int check_aabb_outside_frustum(const AABBOX bbox, const FRUSTUM frustum, Uint32 in_mask)
{
	VECTOR4 n;
	VECTOR3 _n;
	float v;
	Uint32 i, k;

	for (i = 0, k = 1; k <= in_mask; i++, k += k)
	{
		if (k & in_mask)
		{
			VInvertSelect(_n, bbox.bbmin, bbox.bbmax, frustum[i].mask);
			VAssign4(n, _n, 1.0f);
			v = VDot4(n, frustum[i].plane);
			if (v < 0.0f)
			{
				return OUTSIDE;
			}
		}
	}

	return INTERSECT;
}

static __inline__ int check_aabb_inside_portals(const AABBOX bbox, const PLANE* portals, Uint32 count)
{
	VECTOR4 n;
	VECTOR3 _n;
	float v;
	Uint32 i, j, ret;

	for (i = 0; i < count; i++)
	{
		ret = 1;
		for (j = 0; j < 4; j++)
		{
			VInvertSelect(_n, bbox.bbmin, bbox.bbmax, portals[i * 4 + j].mask);
			VAssign4(n, _n, 1.0f);
			v = VDot4(n, portals[i * 4 + j].plane);
			if (v < 0.0f)
			{
				ret = 0;
				break;
			}
		}
		if (ret == 1)
		{
			return 1;
		}
	}

	return 0;
}

static __inline__ void add_items(BBOX_TREE* bbox_tree, Uint32 sub_node, Uint32 in_mask)
{
	Uint32 idx1, idx2, size, i;

	idx1 = bbox_tree->cur_intersect_type;
	idx2 = bbox_tree->nodes[sub_node].items_index;
	size = bbox_tree->nodes[sub_node].items_count;

	for (i = 0; i < size; i++)
	{
		if (check_aabb_outside_frustum(bbox_tree->items[idx2+i].bbox, bbox_tree->intersect[idx1].frustum, in_mask) != OUTSIDE) 
			add_intersect_item(bbox_tree, idx2+i, idx1);
	}
}

static __inline__ void add_dyn_items(BBOX_TREE* bbox_tree, Uint32 sub_node, Uint32 in_mask)
{
	Uint32 idx, size, i;

	idx = bbox_tree->cur_intersect_type;
	size = bbox_tree->nodes[sub_node].dynamic_objects.index;

	for (i = 0; i < size; i++)
	{
		if (check_aabb_outside_frustum(bbox_tree->nodes[sub_node].dynamic_objects.items[i].bbox, bbox_tree->intersect[idx].frustum, in_mask) != OUTSIDE) 
			add_dyn_intersect_item(bbox_tree, sub_node, i, idx);
	}
}

static __inline__ void merge_items(BBOX_TREE* bbox_tree, Uint32 sub_node, Uint32 in_mask, AABBOX* bbox)
{
	Uint32 idx1, idx2, size, i;

	idx1 = bbox_tree->cur_intersect_type;
	idx2 = bbox_tree->nodes[sub_node].items_index;
	size = bbox_tree->nodes[sub_node].items_count;

	for (i = 0; i < size; i++)
	{
		if (check_aabb_outside_frustum(bbox_tree->items[idx2+i].bbox, bbox_tree->intersect[idx1].frustum, in_mask) != OUTSIDE)
		{
			VMin(bbox->bbmin, bbox->bbmin, bbox_tree->items[idx2+i].bbox.bbmin);
			VMax(bbox->bbmax, bbox->bbmax, bbox_tree->items[idx2+i].bbox.bbmax);
		}
	}
}

static __inline__ void merge_dyn_items(BBOX_TREE* bbox_tree, Uint32 sub_node, Uint32 in_mask, AABBOX* bbox)
{
	Uint32 idx, size, i;

	idx = bbox_tree->cur_intersect_type;
	size = bbox_tree->nodes[sub_node].dynamic_objects.index;

	for (i = 0; i < size; i++)
	{
		if (check_aabb_outside_frustum(bbox_tree->nodes[sub_node].dynamic_objects.items[i].bbox,
			bbox_tree->intersect[idx].frustum, in_mask) != OUTSIDE)
		{
			VMin(bbox->bbmin, bbox->bbmin, bbox_tree->nodes[sub_node].dynamic_objects.items[i].bbox.bbmin);
			VMax(bbox->bbmax, bbox->bbmax, bbox_tree->nodes[sub_node].dynamic_objects.items[i].bbox.bbmax);			
		}
	}
}

static __inline__ void check_sub_nodes(BBOX_TREE* bbox_tree, Uint32 sub_node, Uint32 in_mask)
{
	Uint32 out_mask, result, idx;

	if (sub_node != NO_INDEX)
	{
		idx = bbox_tree->cur_intersect_type;
		result = check_aabb_in_frustum(bbox_tree->nodes[sub_node].bbox, bbox_tree->intersect[idx].frustum, in_mask, &out_mask);
		if (result == INSIDE)
		{
			add_intersect_items(bbox_tree, bbox_tree->nodes[sub_node].items_index, bbox_tree->nodes[sub_node].items_count);
			add_dyn_intersect_items(bbox_tree, sub_node, bbox_tree->nodes[sub_node].dynamic_objects.index);
		}
		else
		{
			if (result == INTERSECT)
			{
				add_dyn_items(bbox_tree, sub_node, out_mask);
				if (	(bbox_tree->nodes[sub_node].nodes[0] == NO_INDEX) &&
					(bbox_tree->nodes[sub_node].nodes[1] == NO_INDEX)) add_items(bbox_tree, sub_node, out_mask);
				else
				{
					check_sub_nodes(bbox_tree, bbox_tree->nodes[sub_node].nodes[0], out_mask);
					check_sub_nodes(bbox_tree, bbox_tree->nodes[sub_node].nodes[1], out_mask);
				}
			}
		}
	}
}

static __inline__ void calc_bbox_sub_nodes(BBOX_TREE* bbox_tree, Uint32 sub_node, Uint32 in_mask, AABBOX* bbox)
{
	Uint32 out_mask, result, idx;

	if (sub_node != NO_INDEX)
	{
		idx = bbox_tree->cur_intersect_type;
		result = check_aabb_in_frustum(bbox_tree->nodes[sub_node].bbox, bbox_tree->intersect[idx].frustum, in_mask, &out_mask);
		if (result == INSIDE)
		{
			VMin(bbox->bbmin, bbox->bbmin, bbox_tree->nodes[sub_node].bbox.bbmin);
			VMax(bbox->bbmax, bbox->bbmax, bbox_tree->nodes[sub_node].bbox.bbmax);
		}
		else
		{
			if (result == INTERSECT)
			{
				merge_dyn_items(bbox_tree, sub_node, out_mask, bbox);
				if (	(bbox_tree->nodes[sub_node].nodes[0] == NO_INDEX) &&
					(bbox_tree->nodes[sub_node].nodes[1] == NO_INDEX)) merge_items(bbox_tree, sub_node, out_mask, bbox);
				else
				{
					calc_bbox_sub_nodes(bbox_tree, bbox_tree->nodes[sub_node].nodes[0], out_mask, bbox);
					calc_bbox_sub_nodes(bbox_tree, bbox_tree->nodes[sub_node].nodes[1], out_mask, bbox);
				}
			}
		}
	}
}

static __inline__ int light_depht_comp(BBOX_ITEM* a, BBOX_ITEM* b)
{
	Uint32 ai, bi;
	float ax, ay, az, bx, by, bz, ad, bd;

	ai = a->ID;
	bi = b->ID;

	if ((lights_list[ai] == NULL) || (lights_list[bi] == NULL)) return 0;

	ax = lights_list[ai]->pos_x + camera_x;
	ay = lights_list[ai]->pos_y + camera_y;
	az = lights_list[ai]->pos_z;
	bx = lights_list[bi]->pos_x + camera_x;
	by = lights_list[bi]->pos_y + camera_y;
	bz = lights_list[bi]->pos_z;
	ad = ax*ax+ay*ay+az*az;
	bd = bx*bx+by*by+bz*bz;

	if (ad < bd) return -1;
	else
	{
		if (ad == bd) return 0;
		else return 1;
	}
}

static int comp_items(const void *in_a, const void *in_b)
{
	BBOX_ITEM *a, *b;
	Uint32 am, bm;

	a = (BBOX_ITEM *)in_a;
	b = (BBOX_ITEM *)in_b;

	am = a->type;
	bm = b->type;

	if (am < bm) return -1;
	else
	{
		if (am == bm)
		{
			if (am == TYPE_LIGHT) return light_depht_comp(a, b);
			else
			{
				am = a->texture_id;
				bm = b->texture_id;
				if (am < bm) return -1;
				else
				{
					if (am == bm)
					{
						return 0;
					}
					else
					{
						return 1;
					}
				}
			}
		}
		else return 1;
	}
}

static __inline__ void build_start_stop(BBOX_TREE* bbox_tree)
{
	Uint32 idx, i, cur_type, type;

	idx = bbox_tree->cur_intersect_type;
	memset(bbox_tree->intersect[idx].start, 0, TYPES_COUNT*sizeof(Uint32));
	memset(bbox_tree->intersect[idx].stop, 0, TYPES_COUNT*sizeof(Uint32));

	if (bbox_tree->intersect[idx].count > 0)
	{
		i = 0;
		cur_type = bbox_tree->intersect[idx].items[i].type;
		if (cur_type == TYPE_DELETED) return;

		set_bbox_intersect_flag(bbox_tree, cur_type, ide_changed);
		bbox_tree->intersect[idx].start[cur_type] = i;
		for (i = 1; i < bbox_tree->intersect[idx].count; i++)
		{
			type = bbox_tree->intersect[idx].items[i].type;
			if (type != cur_type)
			{
				if (type == TYPE_DELETED) break;
				bbox_tree->intersect[idx].start[type] = i;
				bbox_tree->intersect[idx].stop[cur_type] = i;
				cur_type = type;
				set_bbox_intersect_flag(bbox_tree, cur_type, ide_changed);
			}
		}
		bbox_tree->intersect[idx].stop[cur_type] = i;
	}
}

static __inline__ void delete_item_from_intersect_list(BBOX_TREE* bbox_tree, Uint32 ID, Uint32 type_mask)
{
	Uint32 i, j, k, size;
	int start, stop;
	Uint32 id;

	for (i = 0; i < MAX_INTERSECTION_TYPES; i++)
	{
		for (j = 0; j < TYPES_COUNT; j++)
		{
			if (type_mask != get_type_mask_from_type(j)) continue;
			start = bbox_tree->intersect[i].start[j];
			stop = bbox_tree->intersect[i].stop[j];
			for (k = start; k < stop; k++)
			{
				if ((type_mask == TYPE_MASK_3D_BLEND_SELF_LIT_OBJECT) ||
				    (type_mask == TYPE_MASK_3D_BLEND_NO_SELF_LIT_OBJECT) ||
				    (type_mask == TYPE_MASK_3D_NO_BLEND_SELF_LIT_OBJECT) ||
				    (type_mask == TYPE_MASK_3D_NO_BLEND_NO_SELF_LIT_OBJECT))
				    id = get_3dobject_index(bbox_tree->intersect[i].items[k].ID);
				else id = bbox_tree->intersect[i].items[k].ID;

                		if (id == ID)
				{
					size = stop - k -1;
					if (size > 0) 
						memmove(&bbox_tree->intersect[i].items[k], &bbox_tree->intersect[i].items[k+1], size*sizeof(BBOX_ITEM));
					bbox_tree->intersect[i].stop[j]--;
                			stop--;
                			k--;
				}
			}
		}
	}
}

void check_bbox_tree(BBOX_TREE* bbox_tree)
{
	Uint32 idx;

	if (bbox_tree != NULL)
	{
		idx = bbox_tree->cur_intersect_type;
		if (bbox_tree->intersect[idx].intersect_update_needed > 0)
		{
			bbox_tree->intersect[idx].count = 0;
			check_sub_nodes(bbox_tree, 0, bbox_tree->intersect[idx].frustum_mask);
			qsort((void *)(bbox_tree->intersect[idx].items), bbox_tree->intersect[idx].count, sizeof(BBOX_ITEM), comp_items);
			build_start_stop(bbox_tree);
			bbox_tree->intersect[idx].intersect_update_needed = 0;
		}
	}
	else BBOX_TREE_LOG_INFO("bbox_tree");
}

void calc_scene_bbox(BBOX_TREE* bbox_tree, AABBOX* bbox)
{
	Uint32 idx;

	if (bbox_tree != NULL)
	{
		idx = bbox_tree->cur_intersect_type;
		VFill(bbox->bbmax, -BOUND_HUGE);
		VFill(bbox->bbmin, BOUND_HUGE);
		calc_bbox_sub_nodes(bbox_tree, 0, bbox_tree->intersect[idx].frustum_mask, bbox);
	}
	else BBOX_TREE_LOG_INFO("bbox_tree");
}

static __inline__ void free_bbox_tree_data(BBOX_TREE* bbox_tree)
{
	Uint32 i;

	if (bbox_tree->items != NULL)
	{
		free(bbox_tree->items);
		bbox_tree->items = NULL;
	}
	else BBOX_TREE_LOG_INFO("bbox_tree->items");

	bbox_tree->items_count = 0;

	for (i = 0; i < bbox_tree->nodes_count; i++)
	{
		if (bbox_tree->nodes[i].dynamic_objects.items != NULL)
		{
			free(bbox_tree->nodes[i].dynamic_objects.items);
			bbox_tree->nodes[i].dynamic_objects.items = NULL;
		}
	}
	if (bbox_tree->nodes != NULL)
	{
		free(bbox_tree->nodes);
		bbox_tree->nodes = NULL;
	}
	else BBOX_TREE_LOG_INFO("bbox_tree->nodes");

	bbox_tree->nodes_count = 0;
}

void clear_bbox_tree(BBOX_TREE* bbox_tree)
{
	Uint32 i;

	if (bbox_tree != NULL)
	{
		for (i = 0; i < MAX_INTERSECTION_TYPES; i++)
		{
			memset(bbox_tree->intersect[i].start, 0, TYPES_COUNT*sizeof(Uint32));
			memset(bbox_tree->intersect[i].stop, 0, TYPES_COUNT*sizeof(Uint32));
			bbox_tree->intersect[i].size = 0;
			bbox_tree->intersect[i].count = 0;
			bbox_tree->intersect[i].intersect_update_needed = 0;
			if (bbox_tree->intersect[i].items != NULL)
			{
				free(bbox_tree->intersect[i].items);
				bbox_tree->intersect[i].items = NULL;
			}
		}
		free_bbox_tree_data(bbox_tree);
	}
	else BBOX_TREE_LOG_INFO("bbox_tree");
}

void free_bbox_tree(BBOX_TREE* bbox_tree)
{
	if (bbox_tree != NULL)
	{
		clear_bbox_tree(bbox_tree);
		free(bbox_tree);
		bbox_tree = NULL;
	}
	else BBOX_TREE_LOG_INFO("bbox_tree");
}

static int Axis = 0;

static int compboxes(const void *in_a, const void *in_b)
{
	const BBOX_ITEM *a = in_a, *b = in_b;
	float am, bm;

	am = a->bbox.bbmin[Axis];
	bm = b->bbox.bbmin[Axis];

	if (am < bm)
		return -1;
	else if (bm < am)
		return 1;
	else
		return 0;
}

#ifdef FASTER_MAP_LOAD
static __inline__ void build_area_table(const BBOX_TREE *bbox_tree,
	Uint32 a, Uint32  b, float *areas)
{
	VECTOR3 bmin, bmax, len;
	int i, j;

	VFill(bmin, BOUND_HUGE);
	VFill(bmax, -BOUND_HUGE);

	if (a < b)
	{
		for (i = a, j = 0; i <= b; i++, j++)
		{
			VMin(bmin, bmin, bbox_tree->items[i].bbox.bbmin);
			VMax(bmax, bmax, bbox_tree->items[i].bbox.bbmax);
			VSub(len, bmax, bmin);

			areas[j] = len[X] * len[Y] * len[Z];
		}
	}
	else
	{
		for (i = a, j = a-b; i >= (int)b; i--, j--)
		{
			VMin(bmin, bmin, bbox_tree->items[i].bbox.bbmin);
			VMax(bmax, bmax, bbox_tree->items[i].bbox.bbmax);
			VSub(len, bmax, bmin);

			areas[j] = len[X] * len[Y] * len[Z];
		}
	}
}

static __inline__ void find_axis_and_bbox(const BBOX_TREE *bbox_tree,
	Uint32 first, Uint32 last, Uint32 *ret, AABBOX *bbox)
{
	Uint32 i, a1, a2, a3;
	VECTOR3 bmin, bmax;
	float d, e;

	VFill(bmin, BOUND_HUGE);
	VFill(bmax, -BOUND_HUGE);

	for (i = first; i < last; i++)
	{
		VMin(bmin, bmin, bbox_tree->items[i].bbox.bbmin);
		VMax(bmax, bmax, bbox_tree->items[i].bbox.bbmax);
	}

	VAssign(bbox->bbmin, bmin);
	VAssign(bbox->bbmax, bmax);

	a1 = 0;
	a2 = 1;
	a3 = 2;

	d = bmax[a1] - bmin[a1];
	e = bmax[a2] - bmin[a2];
	if (d < e)
	{
		i = a2;
		a2 = a1;
		a1 = i;
	}

	d = bmax[a1] - bmin[a1];
	e = bmax[a3] - bmin[a3];
	if (d < e)
	{
//		i = a2;
//		a2 = a1;
		i = a3;
		a3 = a1;
		a1 = i;
	}

	d = bmax[a2] - bmin[a2];
	e = bmax[a3] - bmin[a3];
	if (d < e)
	{
		i = a3;
		a3 = a2;
		a2 = i;
	}

	ret[0] = a1;
	ret[1] = a2;
	ret[2] = a3;
}
#else  // FASTER_MAP_LOAD
static __inline__ void build_area_table(BBOX_TREE *bbox_tree, Uint32 a, Uint32  b, float *areas)
{
	int i, imin, dir;
	VECTOR3 bmin, bmax, len;

	if (a < b)
	{
		imin = a;
		dir =  1;
	}
	else
	{
		imin = b;
		dir = -1;
	}

	VFill(bmin, BOUND_HUGE);
	VFill(bmax, -BOUND_HUGE);

	for (i = a; i != (b + dir); i += dir)
	{
		VMin(bmin, bmin, bbox_tree->items[i].bbox.bbmin);
		VMax(bmax, bmax, bbox_tree->items[i].bbox.bbmax);
		VSub(len, bmax, bmin);

		areas[i - imin] = len[X] * len[Y] * len[Z];
	}
}

static __inline__ void find_axis(BBOX_TREE *bbox_tree, Uint32 first, Uint32 last, Uint32 *ret)
{
	Uint32 i, a1, a2, a3;
	VECTOR3 bmin, bmax;
	float d, e;

	VFill(bmin, BOUND_HUGE);
	VFill(bmax, -BOUND_HUGE);

	for (i = first; i < last; i++)
	{
		VMin(bmin, bmin, bbox_tree->items[i].bbox.bbmin);
		VMax(bmax, bmax, bbox_tree->items[i].bbox.bbmax);
	}

	a1 = 0;
	a2 = 1;
	a3 = 2;
	
	d = bmax[a1] - bmin[a1];
	e = bmax[a2] - bmin[a2];
	
	if (d < e)
	{
		i = a2;
		a2 = a1;
		a1 = i;
	}

	d = bmax[a1] - bmin[a1];
	e = bmax[a3] - bmin[a3];
	
	if (d < e)
	{
		i = a2;
		a2 = a1;
		a1 = i;
	}
	
	d = bmax[a2] - bmin[a2];
	e = bmax[a3] - bmin[a3];
	
	if (d < e)
	{
		i = a3;
		a3 = a2;
		a2 = i;
	}

	ret[0] = a1;
	ret[1] = a2;
	ret[2] = a3;
}

static __inline__ void calc_bbox(AABBOX *bbox, BBOX_TREE *bbox_tree, Uint32 first, Uint32 last)
{
	int i;
	VECTOR3 bmin, bmax;

	VFill(bmin, BOUND_HUGE);
	VFill(bmax, -BOUND_HUGE);

	for (i = first; i < last; i++)
	{
		VMin(bmin, bmin, bbox_tree->items[i].bbox.bbmin);
		VMax(bmax, bmax, bbox_tree->items[i].bbox.bbmax);
	}

	VAssign(bbox->bbmin, bmin);
	VAssign(bbox->bbmax, bmax);
}
#endif // FASTER_MAP_LOAD

static __inline__ Uint32 sort_and_split(BBOX_TREE* bbox_tree, Uint32 node, Uint32* index, Uint32 first, Uint32 last)
{
	Uint32 size, i, j, axis[3];
	int best_loc;
	float *area_left, *area_right;
	float best_index, new_index;
#ifdef FASTER_MAP_LOAD
	AABBOX bbox;
#endif

	size = last - first;

	if (size < 1) return -1;

#ifdef FASTER_MAP_LOAD
	find_axis_and_bbox(bbox_tree, first, last, axis, &bbox);
#else
	find_axis(bbox_tree, first, last, axis);
#endif

	best_loc = -1;

	if (size > 8)
	{
		area_left = malloc(size * sizeof(float));
		area_right = malloc(size * sizeof(float));

		for (j = 0; j < 3; j++)
		{
			Axis = axis[j];

			qsort(bbox_tree->items + first, size, sizeof(BBOX_ITEM), compboxes);
			build_area_table(bbox_tree, first, last - 1, area_left);
			build_area_table(bbox_tree, last - 1, first, area_right);

			best_index = area_right[0] * (size - 3.0);

			/*
			 * Find the most effective point to split. The best location will be
			 * the one that minimizes the function N1*A1 + N2*A2 where N1 and N2
			 * are the number of objects in the two groups and A1 and A2 are the
			 * surface areas of the bounding boxes of the two groups.
			 */
			for (i = 0; i < size - 1; i++)
			{
				new_index = (i + 1) * area_left[i] + (size - 1 - i) * area_right[i + 1];

				if (new_index < best_index)
				{
					best_index = new_index;
					best_loc = i + first;
				}
			}
			if (best_loc >= 0) break;
		}

		free(area_left);
		free(area_right);
	}

#ifdef FASTER_MAP_LOAD
	VAssign(bbox_tree->nodes[node].bbox.bbmin, bbox.bbmin);
	VAssign(bbox_tree->nodes[node].bbox.bbmax, bbox.bbmax);
	VAssign(bbox_tree->nodes[node].orig_bbox.bbmin, bbox.bbmin);
	VAssign(bbox_tree->nodes[node].orig_bbox.bbmax, bbox.bbmax);
#else
	calc_bbox(&bbox_tree->nodes[node].bbox, bbox_tree, first, last);
	VAssign(bbox_tree->nodes[node].orig_bbox.bbmin, bbox_tree->nodes[node].bbox.bbmin);
	VAssign(bbox_tree->nodes[node].orig_bbox.bbmax, bbox_tree->nodes[node].bbox.bbmax);
#endif
	bbox_tree->nodes[node].items_index = first;
	bbox_tree->nodes[node].items_count = size;

	bbox_tree->nodes[node].dynamic_objects.size = 0;
	bbox_tree->nodes[node].dynamic_objects.index = 0;
	bbox_tree->nodes[node].dynamic_objects.items = NULL;

	if (best_loc < 0)
	{
		bbox_tree->nodes[node].nodes[0] = NO_INDEX;
		bbox_tree->nodes[node].nodes[1] = NO_INDEX;
		return 1;
	}
	else
	{
		if (*index+2 >= bbox_tree->nodes_count)
		{
			bbox_tree->nodes_count *= 2;
			bbox_tree->nodes = (BBOX_TREE_NODE*)realloc(bbox_tree->nodes, bbox_tree->nodes_count*sizeof(BBOX_TREE_NODE));
		}
		bbox_tree->nodes[node].nodes[0] = (*index)+0;
		bbox_tree->nodes[node].nodes[1] = (*index)+1;
		*index += 2;
		sort_and_split(bbox_tree, bbox_tree->nodes[node].nodes[0], index, first, best_loc + 1);
		sort_and_split(bbox_tree, bbox_tree->nodes[node].nodes[1], index, best_loc + 1, last);
		return 0;
	}
}

void init_bbox_tree(BBOX_TREE* bbox_tree, const BBOX_ITEMS *bbox_items)
{
	Uint32 size, index;

	if (bbox_items != NULL)
	{
		if (bbox_items->index > 0)
		{
			size = bbox_items->index;	
			index = 1;
			bbox_tree->nodes_count = 2*size;
			bbox_tree->nodes = (BBOX_TREE_NODE*)malloc(size*2*sizeof(BBOX_TREE_NODE));
			bbox_tree->items_count = size;
			bbox_tree->items = (BBOX_ITEM*)malloc(size*sizeof(BBOX_ITEM));
			memcpy(bbox_tree->items, bbox_items->items, size*sizeof(BBOX_ITEM));
			sort_and_split(bbox_tree, 0, &index, 0, size);
			bbox_tree->nodes_count = index;
			bbox_tree->nodes = (BBOX_TREE_NODE*)realloc(bbox_tree->nodes, index*sizeof(BBOX_TREE_NODE));
			set_all_intersect_update_needed(bbox_tree);
		}
	}
	else BBOX_TREE_LOG_INFO("bbox_items");
}

static __inline__ void add_aabb_to_list(BBOX_ITEMS *bbox_items, const AABBOX bbox, Uint32 ID, Uint32 type, Uint32 texture_id)
{
	Uint32 index, size;

	index = bbox_items->index;
	size = bbox_items->size;

	if (size <= index)
	{
		size *= 2;
		bbox_items->items = (BBOX_ITEM*)realloc(bbox_items->items, size*sizeof(BBOX_ITEM));
		bbox_items->size = size;
	}
	VAssign(bbox_items->items[index].bbox.bbmin, bbox.bbmin);
	VAssign(bbox_items->items[index].bbox.bbmax, bbox.bbmax);
	bbox_items->items[index].type = type;
	bbox_items->items[index].extra = 0;
	bbox_items->items[index].texture_id = texture_id;
	bbox_items->items[index].ID = ID;
#ifdef CLUSTER_INSIDES
	bbox_items->items[index].cluster = current_cluster;
#endif // CLUSTER_INSIDES
	bbox_items->index = index + 1;
}

void add_light_to_list(BBOX_ITEMS *bbox_items, Uint32 ID, const AABBOX bbox)
{
	add_aabb_to_list(bbox_items, bbox, ID, TYPE_LIGHT, 0);
}

static __inline__ Uint32 get_3D_type(Uint32 blend, Uint32 ground, Uint32 alpha, Uint32 self_lit)
{
	Uint32 type;

	type = 0;
	if (!blend) type += 8;
	if (!ground) type += 4;
	if (!alpha) type += 2;
	if (!self_lit) type += 1;

	switch (type)
	{
		case 0: return TYPE_3D_BLEND_GROUND_ALPHA_SELF_LIT_OBJECT;
		case 1: return TYPE_3D_BLEND_GROUND_ALPHA_NO_SELF_LIT_OBJECT;
		case 2: return TYPE_3D_BLEND_GROUND_NO_ALPHA_SELF_LIT_OBJECT;
		case 3: return TYPE_3D_BLEND_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT;
		case 4: return TYPE_3D_BLEND_NO_GROUND_ALPHA_SELF_LIT_OBJECT;
		case 5: return TYPE_3D_BLEND_NO_GROUND_ALPHA_NO_SELF_LIT_OBJECT;
		case 6: return TYPE_3D_BLEND_NO_GROUND_NO_ALPHA_SELF_LIT_OBJECT;
		case 7: return TYPE_3D_BLEND_NO_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT;
		case 8: return TYPE_3D_NO_BLEND_GROUND_ALPHA_SELF_LIT_OBJECT;
		case 9: return TYPE_3D_NO_BLEND_GROUND_ALPHA_NO_SELF_LIT_OBJECT;
		case 10: return TYPE_3D_NO_BLEND_GROUND_NO_ALPHA_SELF_LIT_OBJECT;
		case 11: return TYPE_3D_NO_BLEND_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT;
		case 12: return TYPE_3D_NO_BLEND_NO_GROUND_ALPHA_SELF_LIT_OBJECT;
		case 13: return TYPE_3D_NO_BLEND_NO_GROUND_ALPHA_NO_SELF_LIT_OBJECT;
		case 14: return TYPE_3D_NO_BLEND_NO_GROUND_NO_ALPHA_SELF_LIT_OBJECT;
		case 15: return TYPE_3D_NO_BLEND_NO_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT;
		default: return 0xFF;
	}
}

static __inline__ Uint32 get_3D_type_mask(Uint32 blend, Uint32 self_lit)
{
	Uint32 type;

	type = 0;
	if (!blend) type += 2;
	if (!self_lit) type += 1;

	switch (type)
	{
		case 0: return TYPE_MASK_3D_BLEND_SELF_LIT_OBJECT;
		case 1: return TYPE_MASK_3D_BLEND_NO_SELF_LIT_OBJECT;
		case 2: return TYPE_MASK_3D_NO_BLEND_SELF_LIT_OBJECT;
		case 3: return TYPE_MASK_3D_NO_BLEND_NO_SELF_LIT_OBJECT;
		default: return 0xFF;
	}
}

void add_3dobject_to_list(BBOX_ITEMS *bbox_items, Uint32 ID, const AABBOX bbox, Uint32 blend, Uint32 ground, Uint32 alpha, Uint32 self_lit, Uint32 texture_id)
{
	add_aabb_to_list(bbox_items, bbox, ID, get_3D_type(blend, ground, alpha, self_lit), texture_id);
}

static __inline__ Uint32 get_2D_type(Uint32 alpha)
{
	if (alpha == 0) return TYPE_2D_NO_ALPHA_OBJECT;
	else return TYPE_2D_ALPHA_OBJECT;
}

static __inline__ Uint32 get_2D_type_mask(Uint32 alpha)
{
	if (alpha == 0) return TYPE_MASK_2D_ALPHA_OBJECT;
	else return TYPE_MASK_2D_NO_ALPHA_OBJECT;
}

void add_2dobject_to_list(BBOX_ITEMS *bbox_items, Uint32 ID, const AABBOX bbox, Uint32 alpha, Uint32 texture_id)
{
	add_aabb_to_list(bbox_items, bbox, ID, get_2D_type(alpha), texture_id);
}

static __inline__ Uint32 get_blend_type(Uint32 blend)
{
	switch (blend)
	{
		case GL_ZERO: return 0;
		case GL_ONE: return 1;
		case GL_SRC_COLOR: return 2;
		case GL_ONE_MINUS_SRC_COLOR: return 3;
		case GL_DST_COLOR: return 4;
		case GL_ONE_MINUS_DST_COLOR: return 5;
		case GL_SRC_ALPHA: return 6;
		case GL_ONE_MINUS_SRC_ALPHA: return 7;
		case GL_DST_ALPHA: return 8;
		case GL_ONE_MINUS_DST_ALPHA: return 9;
		case GL_SRC_ALPHA_SATURATE: return 10;
		default: return 0xFF;
	}
}

static __inline__ Uint32 get_particle_type(Uint32 sblend, Uint32 dblend)
{
	return ((get_blend_type(sblend) << 4) + get_blend_type(dblend));
}

void add_particle_sys_to_list(BBOX_ITEMS *bbox_items, Uint32 ID, const AABBOX bbox, Uint32 sblend, Uint32 dblend)
{
	add_aabb_to_list(bbox_items, bbox, ID, TYPE_PARTICLE_SYSTEM, get_particle_type(sblend, dblend));
}

void add_terrain_to_list(BBOX_ITEMS *bbox_items, Uint32 ID, const AABBOX bbox, Uint32 texture_id)
{
	add_aabb_to_list(bbox_items, bbox, ID, TYPE_TERRAIN, texture_id);
}

static __inline__ Uint32 get_water_type(Uint32 reflectiv)
{
	if (reflectiv) return TYPE_REFLECTIV_WATER;
	else return TYPE_NO_REFLECTIV_WATER;
}

static __inline__ Uint32 get_water_type_mask(Uint32 reflectiv)
{
	if (reflectiv) return TYPE_MASK_REFLECTIV_WATER;
	else return TYPE_MASK_NO_REFLECTIV_WATER;
}

void add_water_to_list(BBOX_ITEMS *bbox_items, Uint32 ID, const AABBOX bbox, Uint32 texture_id, Uint32 reflectiv)
{
	add_aabb_to_list(bbox_items, bbox, ID, get_water_type(reflectiv), texture_id);
}

static __inline__ Uint32 check_aabb_aabb(const AABBOX bbox, const AABBOX dyn_bbox, float grow)
{
	AABBOX new_bbox;
	VECTOR3 len;
	float old_v, new_v;

	VMin(new_bbox.bbmin, bbox.bbmin, dyn_bbox.bbmin);
	VMax(new_bbox.bbmax, bbox.bbmax, dyn_bbox.bbmax);

	VSub(len, bbox.bbmax, bbox.bbmin);
	old_v = len[X] * len[Y] * len[Z];
	VSub(len, new_bbox.bbmax, new_bbox.bbmin);
	new_v = len[X] * len[Y] * len[Z];

	if ((new_v / old_v) > grow) return 0;
	else return 1;
}

static __inline__ void add_dynamic_item_to_node(BBOX_TREE *bbox_tree, Uint32 node, const AABBOX bbox, Uint32 ID, Uint32 type, Uint32 texture_id, Uint32 extra)
{	
	Uint32 index, size;

	if (node != NO_INDEX)
	{
		index = bbox_tree->nodes[node].dynamic_objects.index;
		size = bbox_tree->nodes[node].dynamic_objects.size;

		if (size <= index)
		{
			if (size < 4) size = 4;
			size *= 2;
			bbox_tree->nodes[node].dynamic_objects.items = (BBOX_ITEM*)realloc(bbox_tree->nodes[node].dynamic_objects.items, size*sizeof(BBOX_ITEM));
			bbox_tree->nodes[node].dynamic_objects.size = size;
		}

		bbox_tree->nodes[node].dynamic_objects.items[index].ID = ID;
		bbox_tree->nodes[node].dynamic_objects.items[index].extra = extra;
		bbox_tree->nodes[node].dynamic_objects.items[index].texture_id = texture_id;
		bbox_tree->nodes[node].dynamic_objects.items[index].type = type;
#ifdef CLUSTER_INSIDES
		bbox_tree->nodes[node].dynamic_objects.items[index].cluster = current_cluster;
#endif // CLUSTER_INSIDES
		VAssign(bbox_tree->nodes[node].dynamic_objects.items[index].bbox.bbmin, bbox.bbmin);
		VAssign(bbox_tree->nodes[node].dynamic_objects.items[index].bbox.bbmax, bbox.bbmax);
		bbox_tree->nodes[node].dynamic_objects.index = index + 1;
		VMin(bbox_tree->nodes[node].bbox.bbmin, bbox_tree->nodes[node].bbox.bbmin, bbox.bbmin);
		VMax(bbox_tree->nodes[node].bbox.bbmax, bbox_tree->nodes[node].bbox.bbmax, bbox.bbmax);
	}
}

static __inline__ int add_dynamic_aabb_to_abt_node(BBOX_TREE *bbox_tree, Uint32 node, const AABBOX bbox, Uint32 ID, Uint32 type, Uint32 texture_id)
{
	Uint32 result, extra;

	if (node != NO_INDEX)
	{
		if (check_aabb_aabb(bbox_tree->nodes[node].orig_bbox, bbox, 1.1f))
		{
			extra = EXTRA_FIRST_SUB_NODE;
			result = add_dynamic_aabb_to_abt_node(bbox_tree, bbox_tree->nodes[node].nodes[0], bbox, ID, type, texture_id);
			if (result == 0)
			{
				result = add_dynamic_aabb_to_abt_node(bbox_tree, bbox_tree->nodes[node].nodes[1], bbox, ID, type, texture_id);
				extra = 0;
			}
			add_dynamic_item_to_node(bbox_tree, node, bbox, ID, type, texture_id, extra);
			return 1;
		}
		else return 0;
	}
	else return 0;
}

static __inline__ void add_aabb_to_abt(BBOX_TREE *bbox_tree, const AABBOX bbox, Uint32 ID, Uint32 type, Uint32 texture_id, Uint32 dynamic)
{
	Uint32 result;
	
	if (bbox_tree != NULL)
	{
		result = add_dynamic_aabb_to_abt_node(bbox_tree, 0, bbox, ID, type, texture_id);
		if (result == 0) add_dynamic_item_to_node(bbox_tree, 0, bbox, ID, type, texture_id, 0);
		set_all_intersect_update_needed(bbox_tree);
	}
	else BBOX_TREE_LOG_INFO("bbox_tree");
}

void add_light_to_abt(BBOX_TREE *bbox_tree, Uint32 ID, const AABBOX bbox, Uint32 dynamic)
{
	add_aabb_to_abt(bbox_tree, bbox, ID, TYPE_LIGHT, 0, dynamic);
}

void add_3dobject_to_abt(BBOX_TREE *bbox_tree, Uint32 ID, const AABBOX bbox, Uint32 blend, Uint32 ground, Uint32 alpha, Uint32 self_lit, Uint32 texture_id, Uint32 dynamic)
{
	add_aabb_to_abt(bbox_tree, bbox, ID, get_3D_type(blend, ground, alpha, self_lit), texture_id, dynamic);
}

void add_2dobject_to_abt(BBOX_TREE *bbox_tree, Uint32 ID, const AABBOX bbox, Uint32 alpha, Uint32 texture_id, Uint32 dynamic)
{
	add_aabb_to_abt(bbox_tree, bbox, ID, get_2D_type(alpha), texture_id, dynamic);
}

void add_particle_to_abt(BBOX_TREE *bbox_tree, Uint32 ID, const AABBOX bbox, Uint32 sblend, Uint32 dblend, Uint32 dynamic)
{
	add_aabb_to_abt(bbox_tree, bbox, ID, TYPE_PARTICLE_SYSTEM, get_particle_type(sblend, dblend), dynamic);
}

void add_terrain_to_abt(BBOX_TREE *bbox_tree, Uint32 ID, const AABBOX bbox, Uint32 texture_id, Uint32 dynamic)
{
	add_aabb_to_abt(bbox_tree, bbox, ID, TYPE_TERRAIN, texture_id, dynamic);
}

void add_water_to_abt(BBOX_TREE *bbox_tree, Uint32 ID, const AABBOX bbox, Uint32 texture_id, Uint32 reflectiv, Uint32 dynamic)
{
	add_aabb_to_abt(bbox_tree, bbox, ID, get_water_type(reflectiv), texture_id, dynamic);
}

static __inline__ Uint32 delete_dynamic_aabb_from_node(BBOX_TREE *bbox_tree, Uint32 node, Uint32 ID, Uint32 type_mask);

static __inline__ void delete_dynamic_item_from_node(BBOX_TREE *bbox_tree, Uint32 node, Uint32 idx, Uint32 ID, Uint32 type_mask)
{
	int index, size, count;
	
	index = bbox_tree->nodes[node].dynamic_objects.index;
	size = bbox_tree->nodes[node].dynamic_objects.size;
	
	if (is_extra_first_sub_node(bbox_tree->nodes[node].dynamic_objects.items[idx].extra))
		delete_dynamic_aabb_from_node(bbox_tree, bbox_tree->nodes[node].nodes[0], ID, type_mask);
	else delete_dynamic_aabb_from_node(bbox_tree, bbox_tree->nodes[node].nodes[1], ID, type_mask);
	
	if (index <= 1)
	{
		size = 0;
		index = 0;
		if (bbox_tree->nodes[node].dynamic_objects.items != NULL) 
		{
			free(bbox_tree->nodes[node].dynamic_objects.items);
			bbox_tree->nodes[node].dynamic_objects.items = NULL;
		}
	}
	else
	{
		if (index < (size/2))
		{
			size /= 2;
			bbox_tree->nodes[node].dynamic_objects.items = (BBOX_ITEM*)realloc(bbox_tree->nodes[node].dynamic_objects.items, size*sizeof(BBOX_ITEM));
		}
		
		count = index-idx-1;
		if (count > 0) memmove(&bbox_tree->nodes[node].dynamic_objects.items[idx], &bbox_tree->nodes[node].dynamic_objects.items[idx+1], count*sizeof(BBOX_ITEM));
		index--;
	}
	bbox_tree->nodes[node].dynamic_objects.index = index;
	bbox_tree->nodes[node].dynamic_objects.size = size;
}

static __inline__ Uint32 dynamic_aabb_is_in_node(BBOX_TREE *bbox_tree, Uint32 node, Uint32 ID, Uint32 type_mask)
{
	Uint32 i, result;
	Uint32 id;

	result = 0;
	
	for (i = 0; i < bbox_tree->nodes[node].dynamic_objects.index; i++)
	{
	    if ((type_mask == TYPE_MASK_3D_BLEND_SELF_LIT_OBJECT) ||
		(type_mask == TYPE_MASK_3D_BLEND_NO_SELF_LIT_OBJECT) ||
		(type_mask == TYPE_MASK_3D_NO_BLEND_SELF_LIT_OBJECT) ||
		(type_mask == TYPE_MASK_3D_NO_BLEND_NO_SELF_LIT_OBJECT))
		id = get_3dobject_index(bbox_tree->nodes[node].dynamic_objects.items[i].ID);
	    else id = bbox_tree->nodes[node].dynamic_objects.items[i].ID;

        if ((id == ID) &&
            (get_type_mask_from_type(bbox_tree->nodes[node].dynamic_objects.items[i].type) == type_mask))
		{
			delete_dynamic_item_from_node(bbox_tree, node, i, ID, type_mask);
			result = 1;
		}
	}

	return result;
}

static __inline__ Uint32 delete_dynamic_aabb_from_node(BBOX_TREE *bbox_tree, Uint32 node, Uint32 ID, Uint32 type_mask)
{
	Uint32 i, result, idx1, idx2;
	AABBOX new_bbox;

	if (node != NO_INDEX && bbox_tree->nodes_count > 0)
	{		
		result = dynamic_aabb_is_in_node(bbox_tree, node, ID, type_mask);
		if (result != 0)
		{
			if ((bbox_tree->nodes[node].nodes[0] != NO_INDEX) && (bbox_tree->nodes[node].nodes[1] != NO_INDEX))
			{
				idx1 = bbox_tree->nodes[node].nodes[0];
				idx2 = bbox_tree->nodes[node].nodes[1];
				VMin(new_bbox.bbmin, bbox_tree->nodes[idx1].bbox.bbmin, bbox_tree->nodes[idx2].bbox.bbmin);
				VMax(new_bbox.bbmax, bbox_tree->nodes[idx1].bbox.bbmax, bbox_tree->nodes[idx2].bbox.bbmax);
			}
			else
			{
				VAssign(new_bbox.bbmin, bbox_tree->nodes[node].orig_bbox.bbmin);
				VAssign(new_bbox.bbmax, bbox_tree->nodes[node].orig_bbox.bbmax);
			}

			for (i = 0; i < bbox_tree->nodes[node].dynamic_objects.index; i++)
			{
				VMin(new_bbox.bbmin, new_bbox.bbmin, bbox_tree->nodes[node].dynamic_objects.items[i].bbox.bbmin);
				VMax(new_bbox.bbmax, new_bbox.bbmax, bbox_tree->nodes[node].dynamic_objects.items[i].bbox.bbmax);
			}

			VAssign(bbox_tree->nodes[node].bbox.bbmin, new_bbox.bbmin);
			VAssign(bbox_tree->nodes[node].bbox.bbmax, new_bbox.bbmax);

			return 1;
		}
	}
	return 0;
}

static __inline__ void delete_aabb_from_abt(BBOX_TREE *bbox_tree, Uint32 ID, Uint32 type_mask)
{
	if (bbox_tree != NULL)
	{
		delete_item_from_intersect_list(bbox_tree, ID, type_mask);
		delete_dynamic_aabb_from_node(bbox_tree, 0, ID, type_mask);
	}
	else BBOX_TREE_LOG_INFO("bbox_tree");
}

void delete_3dobject_from_abt(BBOX_TREE *bbox_tree, Uint32 ID, Uint32 blend, Uint32 self_lit)
{
	delete_aabb_from_abt(bbox_tree, ID, get_3D_type_mask(blend, self_lit));
}

void delete_2dobject_from_abt(BBOX_TREE *bbox_tree, Uint32 ID, Uint32 alpha)
{
	delete_aabb_from_abt(bbox_tree, ID, get_2D_type_mask(alpha));
}

void delete_particle_from_abt(BBOX_TREE *bbox_tree, Uint32 ID)
{
	delete_aabb_from_abt(bbox_tree, ID, TYPE_MASK_PARTICLE_SYSTEM);
}

void delete_light_from_abt(BBOX_TREE *bbox_tree, Uint32 ID)
{
	delete_aabb_from_abt(bbox_tree, ID, TYPE_MASK_LIGHT);
}

void delete_terrain_from_abt(BBOX_TREE *bbox_tree, Uint32 ID)
{
	delete_aabb_from_abt(bbox_tree, ID, TYPE_MASK_TERRAIN);
}

void delete_water_from_abt(BBOX_TREE *bbox_tree, Uint32 ID, Uint32 reflectiv)
{
	delete_aabb_from_abt(bbox_tree, ID, get_water_type_mask(reflectiv));
}

BBOX_ITEMS* create_bbox_items(Uint32 size)
{
	BBOX_ITEMS* bbox_items;
	
	bbox_items = (BBOX_ITEMS*)malloc(sizeof(BBOX_ITEMS));
	size = max2u(8, size);
	bbox_items->size = size;
	bbox_items->index = 0;
	bbox_items->items = (BBOX_ITEM*)malloc(size*sizeof(BBOX_ITEM));

	return bbox_items;
}

void free_bbox_items(BBOX_ITEMS* bbox_items)
{
	if (bbox_items != NULL)
	{
		if (bbox_items->items != NULL) 
		{
			free(bbox_items->items);
			bbox_items->items = NULL;
		}
		else BBOX_TREE_LOG_INFO("bbox_items->items");
		free(bbox_items);
		bbox_items = NULL;
	}
	else BBOX_TREE_LOG_INFO("bbox_items");
}

BBOX_TREE* build_bbox_tree()
{
	BBOX_TREE* bbox_tree;
	Uint32 i;

	bbox_tree = (BBOX_TREE*)malloc(sizeof(BBOX_TREE));
	
	bbox_tree->cur_intersect_type = INTERSECTION_TYPE_DEFAULT;
	for (i = 0; i < MAX_INTERSECTION_TYPES; i++)
	{
		bbox_tree->intersect[i].size = 8;
		bbox_tree->intersect[i].count = 0;
		bbox_tree->intersect[i].items = (BBOX_ITEM*)malloc(8*sizeof(BBOX_ITEM));
		memset(&bbox_tree->intersect[i].flags, 0, sizeof(bbox_tree->intersect[i].flags));
	}
	bbox_tree->nodes_count = 0;
	bbox_tree->nodes = NULL;
	bbox_tree->items_count = 0;
	bbox_tree->items = NULL;
	return bbox_tree;
}

static __inline__ void get_point_from_aabbox(VECTOR3 point, const AABBOX bbox, Uint32 number)
{
	VECTOR3I mask;
	
	switch (number)
	{
		case 0:
			VMakeI(mask, 0x00000000, 0x00000000, 0x00000000);
			break;
		case 1:
			VMakeI(mask, 0x00000000, 0x00000000, 0xFFFFFFFF);
			break;
		case 2:
			VMakeI(mask, 0x00000000, 0xFFFFFFFF, 0x00000000);
			break;
		case 3:
			VMakeI(mask, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF);
			break;
		case 4:
			VMakeI(mask, 0xFFFFFFFF, 0x00000000, 0x00000000);
			break;
		case 5:
			VMakeI(mask, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF);
			break;
		case 6:
			VMakeI(mask, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000);
			break;
		case 7:
			VMakeI(mask, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
			break;
		default:
			VMakeI(mask, 0x00000000, 0x00000000, 0x00000000);
#ifdef	DEBUG
			LOG_ERROR("Wrong number (%d) for get_point_from_aabbox!", number);
#endif
			break;
	}
	VSelect(point, bbox.bbmin, bbox.bbmax, mask);
}

static __inline__ void calculate_frustum_data(FRUSTUM_DATA data, const FRUSTUM frustum, const VECTOR3 light_dir, Uint32 mask)
{
	VECTOR3 n;
	VECTOR4 t;
	float v;
	int i, k;
	
	VAssign(n, light_dir);
	Normalize(n, n);
	VAssign4(t, n, 0.0f);

	for (i = 0, k = 1; k <= mask; i++, k += k)
	{
		if (k & mask)
		{
			v = VDot4(t, frustum[i].plane);
			if (max2f(v, -v) < 0.0001f)
			{
				data[i].zero = 1;
				data[i].scale = 1.0f;
				data[i].mask = 0;
			}
			else
			{
				data[i].zero = 0;
				data[i].scale = v;
				if (v < 0.0f) data[i].mask = 1;
				else data[i].mask = 0;
			}
		}
	}
}

static __inline__ int check_shadow_line_walk(const AABBOX bbox, const FRUSTUM frustum, const VECTOR3 light_dir, Uint32 mask, const float* hit)
{
	AABBOX tmp_bbox;
	VECTOR3 walk;
	int i, k, result, intersect;

	intersect = 0;
	
	result = check_aabb_in_frustum_no_out_mask(bbox, frustum, mask);
	
	if (result == INSIDE) return result;
	else if (result == INTERSECT) intersect = 1;
	
	for (i = 0, k = 1; k <= mask; i++, k += k)
	{
		if ((k & mask) && (hit[i] > 0.0f))
		{
			VScale(walk, light_dir, hit[i]);
			VAdd(tmp_bbox.bbmin, bbox.bbmin, walk);
			VAdd(tmp_bbox.bbmax, bbox.bbmax, walk);
			result = check_aabb_in_frustum_no_out_mask(tmp_bbox, frustum, mask);
			if (result == INSIDE) return result;
			else if (result == INTERSECT) intersect = 1;
		}
	}
	if (intersect != 0) return INTERSECT;
	else return OUTSIDE;
}

static __inline__ int check_shadow_lines(const AABBOX bbox, const FRUSTUM frustum, const FRUSTUM_DATA data, const VECTOR3 light_dir, Uint32 mask, Uint32 point_mask)
{
	VECTOR3 _p;
	VECTOR4 p;
	float tmp, vmin, vmax, step, hit[16];
	int i, j, k, l, intersect, intersects, points;
	
	intersects = 0;
	points = 0;
	
	memset(hit, 0, sizeof(hit));
	
	for (i = 0, k = 1; k <= point_mask; i++, k += k)
	{
		if (k & point_mask)
		{
			get_point_from_aabbox(_p, bbox, i);
			VAssign4(p, _p, 1.0f);
			vmin = 0.0f;
			vmax = BOUND_HUGE;
			points++;
			intersect = 1;

			for (j = 0, l = 1; l <= mask; j++, l += l)
			{
				if (l & mask)
				{
					tmp = VDot4(p, frustum[j].plane);
					
					if (data[j].zero == 0)
					{
						step = tmp/data[j].scale;
						if (step < 0.0f)
						{
							if (tmp < 0.0f)
							{
								intersect = 0;
							}
						}
						else
						{
							hit[j] = max2f(hit[j], step);
							if (intersect != 0)
							{
								if (data[j].mask)
								{
									vmin = max2f(vmin, step);
								}
								else
								{
									vmax = min2f(vmax, step);
								}
								if (vmin > vmax)
								{
									intersect = 0;
								}
							}
						}
					}
					else
					{
						if (tmp < 0.0f)
						{
							intersect = 0;
						}
					}
				}
			}
			intersects += intersect;
		}
	}
	if (intersects == 0)
	{
		return check_shadow_line_walk(bbox, frustum, light_dir, mask, hit);
	}
	else if (intersects == points) return INSIDE;
	else return INTERSECT;
}

static __inline__ int check_shadow_line_walk_outside(const AABBOX bbox, const FRUSTUM frustum, const VECTOR3 light_dir, Uint32 mask, const float* hit)
{
	AABBOX tmp_bbox;
	VECTOR3 walk;
	int i, k;

	if (check_aabb_outside_frustum(bbox, frustum, mask) != OUTSIDE) return INTERSECT;
	
	for (i = 0, k = 1; k <= mask; i++, k += k)
	{
		if ((k & mask) && (hit[i] > 0.0f))
		{
			VScale(walk, light_dir, hit[i]);
			VAdd(tmp_bbox.bbmin, bbox.bbmin, walk);
			VAdd(tmp_bbox.bbmax, bbox.bbmax, walk);
			if (check_aabb_outside_frustum(tmp_bbox, frustum, mask) != OUTSIDE) return INTERSECT;
		}
	}
	return OUTSIDE;
}

static __inline__ int check_shadow_lines_outside(const AABBOX bbox, const FRUSTUM frustum, const FRUSTUM_DATA data, const VECTOR3 light_dir, Uint32 mask, Uint32 point_mask)
{
	VECTOR3 _p;
	VECTOR4 p;
	float tmp, vmin, vmax, step, hit[16];
	int i, j, k, l, intersect;
	
	memset(hit, 0, sizeof(hit));
	
	for (i = 0, k = 1; k <= point_mask; i++, k += k)
	{
		if (k & point_mask)
		{
			get_point_from_aabbox(_p, bbox, i);
			VAssign4(p, _p, 1.0f);
			vmin = 0.0f;
			vmax = BOUND_HUGE;
			intersect = 1;

			for (j = 0, l = 1; l <= mask; j++, l += l)
			{
				if (l & mask)
				{
					tmp = VDot4(p, frustum[j].plane);
					
					if (data[j].zero == 0)
					{
						step = tmp/data[j].scale;
						if (step < 0.0f)
						{
							if (tmp < 0.0f)
							{
								intersect = 0;
							}
						}
						else
						{
							hit[j] = max2f(hit[j], step);
							if (intersect != 0)
							{
								if (data[j].mask)
								{
									vmin = max2f(vmin, step);
								}
								else
								{
									vmax = min2f(vmax, step);
								}
								if (vmin > vmax)
								{
									intersect = 0;
								}
							}
						}
					}
					else
					{
						if (tmp < 0.0f)
						{
							intersect = 0;
						}
					}
				}
			}
			if (intersect != 0) return INTERSECT;
		}
	}
	return check_shadow_line_walk_outside(bbox, frustum, light_dir, mask, hit);
}

static __inline__ void add_dyn_items_shadow(BBOX_TREE* bbox_tree, Uint32 sub_node, const FRUSTUM frustum, Uint32 in_mask,
	const FRUSTUM view_frustum, const FRUSTUM_DATA data, const VECTOR3 light_dir, Uint32 mask, Uint32 point_mask)
{
	Uint32 idx, size, i;

	idx = bbox_tree->cur_intersect_type;
	size = bbox_tree->nodes[sub_node].dynamic_objects.index;
		
	for (i = 0; i < size; i++)
	{
		if (check_aabb_outside_frustum(bbox_tree->nodes[sub_node].dynamic_objects.items[i].bbox, frustum, in_mask) != OUTSIDE)
		{
			if (check_shadow_lines_outside(bbox_tree->nodes[sub_node].dynamic_objects.items[i].bbox, view_frustum, data, light_dir,
				mask, point_mask) != OUTSIDE)
				add_dyn_intersect_item(bbox_tree, sub_node, i, idx);
		}
	}
}

static __inline__ void add_items_shadow(BBOX_TREE* bbox_tree, Uint32 sub_node, const FRUSTUM frustum, Uint32 in_mask,
	const FRUSTUM view_frustum, const FRUSTUM_DATA data, const VECTOR3 light_dir, Uint32 mask, Uint32 point_mask)
{
	Uint32 idx1, idx2, size, i;

	idx1 = bbox_tree->cur_intersect_type;
	idx2 = bbox_tree->nodes[sub_node].items_index;
	size = bbox_tree->nodes[sub_node].items_count;
		
	for (i = 0; i < size; i++)
	{
		if (check_aabb_outside_frustum(bbox_tree->items[idx2+i].bbox, frustum, in_mask) != OUTSIDE)
		{
			if (check_shadow_lines_outside(bbox_tree->items[idx2+i].bbox, view_frustum, data, light_dir, mask, point_mask) != OUTSIDE)
				add_intersect_item(bbox_tree, idx2+i, idx1);
		}
	}
}

static __inline__ void check_sub_nodes_shadow(BBOX_TREE* bbox_tree, Uint32 sub_node, const FRUSTUM frustum, Uint32 in_mask,
	const FRUSTUM view_frustum, const FRUSTUM_DATA data, const VECTOR3 light_dir, Uint32 mask, Uint32 point_mask)
{
	Uint32 out_mask, result;
	
	if (sub_node != NO_INDEX)
	{
		result = check_aabb_in_frustum(bbox_tree->nodes[sub_node].bbox, frustum, in_mask, &out_mask);
		
		if (result == OUTSIDE) return;
		result = check_shadow_lines(bbox_tree->nodes[sub_node].bbox, view_frustum, data, light_dir, mask, point_mask);
		
		if (result == INSIDE)
		{
			add_intersect_items(bbox_tree, bbox_tree->nodes[sub_node].items_index, bbox_tree->nodes[sub_node].items_count);
			add_dyn_intersect_items(bbox_tree, sub_node, bbox_tree->nodes[sub_node].dynamic_objects.index);
		}
		else
		{
			if (result == INTERSECT)
			{
				add_dyn_items_shadow(bbox_tree, sub_node, frustum, out_mask, view_frustum, data, light_dir, mask, point_mask);
				if (	(bbox_tree->nodes[sub_node].nodes[0] == NO_INDEX) &&
					(bbox_tree->nodes[sub_node].nodes[1] == NO_INDEX)) 
					add_items_shadow(bbox_tree, sub_node, frustum, out_mask, view_frustum, data, light_dir, mask, point_mask);
				else
				{
					check_sub_nodes_shadow(bbox_tree, bbox_tree->nodes[sub_node].nodes[0], frustum, out_mask,
						view_frustum, data, light_dir, mask, point_mask);
					check_sub_nodes_shadow(bbox_tree, bbox_tree->nodes[sub_node].nodes[1], frustum, out_mask,
						view_frustum, data, light_dir, mask, point_mask);
				}
			}
		}
	}
}

static __inline__ int calculate_point_mask(const VECTOR3 light_dir)
{
	int mask, idx, i, k;

	idx = 0;
	
	if (VExtract(light_dir, X) >= 0.0f) idx += 1;
	if (VExtract(light_dir, Y) >= 0.0f) idx += 2;
	if (VExtract(light_dir, Z) >= 0.0f) idx += 4;

	mask = 0;

	for (i = 0, k = 1; i < 8; i++, k += k)
	{
		if ((i != idx) && (i != (7-idx))) mask |= k;
	}

	return mask;
}

void check_bbox_tree_shadow(BBOX_TREE* bbox_tree, const FRUSTUM frustum, Uint32 mask, const FRUSTUM view_frustum,
	Uint32 view_mask, const VECTOR3 light_dir)
{
	Uint32 idx, point_mask;
	FRUSTUM_DATA data;

	if (bbox_tree != NULL)
	{
		idx = bbox_tree->cur_intersect_type;
		if (idx != INTERSECTION_TYPE_SHADOW) return;
		if (bbox_tree->intersect[idx].intersect_update_needed > 0)
		{
			point_mask = calculate_point_mask(light_dir);
			calculate_frustum_data(data, view_frustum, light_dir, view_mask);
			bbox_tree->intersect[idx].count = 0;
			check_sub_nodes_shadow(bbox_tree, 0, frustum, mask, view_frustum, data, light_dir, view_mask, point_mask);
			qsort((void *)(bbox_tree->intersect[idx].items), bbox_tree->intersect[idx].count, sizeof(BBOX_ITEM), comp_items);
			build_start_stop(bbox_tree);
			bbox_tree->intersect[idx].intersect_update_needed = 0;
		}
	}
	else BBOX_TREE_LOG_INFO("bbox_tree");
}

void set_frustum(BBOX_TREE* bbox_tree, const FRUSTUM frustum, Uint32 mask)
{
	Uint32 idx;

	if (bbox_tree != NULL)
	{
		idx = bbox_tree->cur_intersect_type;
		memcpy(bbox_tree->intersect[idx].frustum, frustum, sizeof(FRUSTUM));
		bbox_tree->intersect[idx].frustum_mask = mask;
	}	
	else BBOX_TREE_LOG_INFO("bbox_tree");
}

static __inline__ void reflection_portal_checks(BBOX_TREE* bbox_tree, const PLANE* portals, Uint32 count)
{
	Uint32 i, j, idx;
	int start, stop;
	
	idx = bbox_tree->cur_intersect_type;
	
	for (i = 0; i < TYPES_COUNT; i++)
	{
		start = bbox_tree->intersect[idx].start[i];
		stop = bbox_tree->intersect[idx].stop[i];
		for (j = start; j < stop; j++)
		{
			if (check_aabb_inside_portals(bbox_tree->intersect[idx].items[j].bbox, portals, count) == 0)
			{
				bbox_tree->intersect[idx].items[j].type = TYPE_DELETED;
			}
		}
	}
}

void reflection_portal_check(BBOX_TREE* bbox_tree, const PLANE* portals, Uint32 count)
{
	Uint32 idx;

	if (bbox_tree != NULL)
	{
		idx = bbox_tree->cur_intersect_type;
		if (bbox_tree->intersect[idx].intersect_update_needed == 0)
		{
			reflection_portal_checks(bbox_tree, portals, count);
			qsort((void *)(bbox_tree->intersect[idx].items), bbox_tree->intersect[idx].count, sizeof(BBOX_ITEM), comp_items);
			build_start_stop(bbox_tree);
			bbox_tree->intersect[idx].intersect_update_needed = 0;
		}
	}
	else BBOX_TREE_LOG_INFO("bbox_tree");
}

