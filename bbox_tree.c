#ifdef	NEW_FRUSTUM
#ifdef MAP_EDITOR2
#include "../map_editor2/global.h"
#else
#include "global.h"
#endif
#ifndef	BSD
#include <malloc.h>
#endif

static const MD5_DIGEST ZERO_MD5 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#ifdef	EXTRA_DEBUG
#define BBOX_TREE_LOG_INFO(item)	log_error_detailed("%s is NULL", __FILE__, __FUNCTION__, __LINE__, item);
#else	//DEBUG
#define BBOX_TREE_LOG_INFO(item)	/*!< NOP */
#endif	//DEBUG

#define	NO_INDEX	0xFFFFFFFF

static __inline__ unsigned int is_extra_first_sub_node(unsigned int extra)
{
	return ((extra & EXTRA_FIRST_SUB_NODE) != 0);
}

void set_all_intersect_update_needed(BBOX_TREE* bbox_tree)
{
	unsigned int i;

	if (bbox_tree != NULL)
	{
		for (i = 0; i < MAX_INTERSECTION_TYPES; i++)
			bbox_tree->intersect[i].intersect_update_needed = 1;
	}
	else BBOX_TREE_LOG_INFO("bbox_tree");
}

static __inline__ void adapt_intersect_list_size(BBOX_TREE* bbox_tree, unsigned int count)
{
	unsigned int size, idx;

	if (count == 0) count = 1;
	idx = bbox_tree->cur_intersect_type;
	size = bbox_tree->intersect[idx].size;
	
	if ((bbox_tree->intersect[idx].count+count) >= size)
	{
		size += max2i(count, size/2);
		bbox_tree->intersect[idx].items = (BBOX_ITEM_DATA*)realloc(bbox_tree->intersect[idx].items, size*sizeof(BBOX_ITEM_DATA));
		bbox_tree->intersect[idx].size = size;
	}
}

static __inline__ void add_intersect_item(BBOX_TREE* bbox_tree, unsigned int index, unsigned int idx)
{
	adapt_intersect_list_size(bbox_tree, 1);
	memcpy(&bbox_tree->intersect[idx].items[bbox_tree->intersect[idx].count], &bbox_tree->items[index].data, sizeof(BBOX_ITEM_DATA));
	bbox_tree->intersect[idx].count++;
}

static __inline__ void add_intersect_items(BBOX_TREE* bbox_tree, unsigned int index, unsigned int count)
{
	unsigned int i;
	
	adapt_intersect_list_size(bbox_tree, count);
	for (i = 0; i < count; i++) add_intersect_item(bbox_tree, index+i, bbox_tree->cur_intersect_type);
}

static __inline__ void add_dyn_intersect_item(BBOX_TREE* bbox_tree, unsigned int node, unsigned int index, unsigned int idx)
{
	adapt_intersect_list_size(bbox_tree, 1);
	memcpy(&bbox_tree->intersect[idx].items[bbox_tree->intersect[idx].count], &bbox_tree->nodes[node].dynamic_objects.items[index].data, sizeof(BBOX_ITEM_DATA));
	bbox_tree->intersect[idx].count++;
}

static __inline__ void add_dyn_intersect_items(BBOX_TREE* bbox_tree, unsigned int node, unsigned int count)
{
	unsigned int i;
	
	adapt_intersect_list_size(bbox_tree, count);
	for (i = 0; i < count; i++) add_dyn_intersect_item(bbox_tree, node, i, bbox_tree->cur_intersect_type);
}

static __inline__ void add_dyn_intersect_data(BBOX_TREE* bbox_tree, unsigned int node, unsigned int index, unsigned int idx)
{
	adapt_intersect_list_size(bbox_tree, 1);
	memcpy(&bbox_tree->intersect[idx].items[bbox_tree->intersect[idx].count], &bbox_tree->nodes[node].dynamic_objects.sub_items[index], sizeof(BBOX_ITEM_DATA));
	bbox_tree->intersect[idx].count++;
}

static __inline__ void add_dyn_intersect_datas(BBOX_TREE* bbox_tree, unsigned int node, unsigned int count)
{
	unsigned int idx;
	
	idx = bbox_tree->cur_intersect_type;	
	adapt_intersect_list_size(bbox_tree, count);
	memcpy(&bbox_tree->intersect[idx].items[bbox_tree->intersect[idx].count], bbox_tree->nodes[node].dynamic_objects.sub_items, count*sizeof(BBOX_ITEM_DATA));
	bbox_tree->intersect[idx].count += count;
}

static __inline__ int check_aabb_in_frustum(const AABBOX bbox, const FRUSTUM frustum, unsigned int in_mask, unsigned int *out_mask)
{
	float m, n;
	unsigned int i, k, result;
	
	result = INSIDE;
	*out_mask = 0;
	
	for (i = 0, k = 1; k <= in_mask; i++, k += k)
	{
		if (k & in_mask)
		{
			float nx, px, ny, py, nz, pz;

			nx = !frustum[i].mask[0] ? bbox.bbmin[X] :  bbox.bbmax[X];
			px = frustum[i].mask[0] ? bbox.bbmin[X] :  bbox.bbmax[X];
			ny = !frustum[i].mask[1] ? bbox.bbmin[Y] :  bbox.bbmax[Y];
			py = frustum[i].mask[1] ? bbox.bbmin[Y] :  bbox.bbmax[Y];
			nz = !frustum[i].mask[2] ? bbox.bbmin[Z] :  bbox.bbmax[Z];
			pz = frustum[i].mask[2] ? bbox.bbmin[Z] :  bbox.bbmax[Z];
			m = (	frustum[i].plane[A] * nx + 
				frustum[i].plane[B] * ny + 
				frustum[i].plane[C] * nz);
			if (m < -frustum[i].plane[D]) return OUTSIDE;
			
			n = (	frustum[i].plane[A] * px + 
				frustum[i].plane[B] * py + 
				frustum[i].plane[C] * pz);
			if (n < -frustum[i].plane[D]) 
			{
				*out_mask |= k;
				result = INTERSECT;
			}
		}
	}

	return result;
}

static __inline__ void add_items(BBOX_TREE* bbox_tree, unsigned int sub_node, const FRUSTUM frustum, unsigned int in_mask)
{
	unsigned int idx1, idx2, size, i, out_mask;

	idx1 = bbox_tree->cur_intersect_type;
	idx2 = bbox_tree->nodes[sub_node].items_index;
	size = bbox_tree->nodes[sub_node].items_count;
		
	for (i = 0; i < size; i++)
	{
		if (check_aabb_in_frustum(bbox_tree->items[idx2+i].bbox, frustum, in_mask, &out_mask) != OUTSIDE) 
			add_intersect_item(bbox_tree, idx2+i, idx1);
	}
}

static __inline__ void add_dyn_items(BBOX_TREE* bbox_tree, unsigned int sub_node, const FRUSTUM frustum, unsigned int in_mask)
{
	unsigned int idx, size, i, out_mask;

	idx = bbox_tree->cur_intersect_type;
	size = bbox_tree->nodes[sub_node].dynamic_objects.index;
		
	for (i = 0; i < size; i++)
	{
		if (check_aabb_in_frustum(bbox_tree->nodes[sub_node].dynamic_objects.items[i].bbox, frustum, in_mask, &out_mask) != OUTSIDE) 
			add_dyn_intersect_item(bbox_tree, sub_node, i, idx);
	}
}

static __inline__ void merge_items(BBOX_TREE* bbox_tree, unsigned int sub_node,
	const FRUSTUM frustum, unsigned int in_mask, AABBOX* bbox)
{
	unsigned int idx1, idx2, size, i, out_mask;

	idx1 = bbox_tree->cur_intersect_type;
	idx2 = bbox_tree->nodes[sub_node].items_index;
	size = bbox_tree->nodes[sub_node].items_count;
		
	for (i = 0; i < size; i++)
	{
		if (check_aabb_in_frustum(bbox_tree->items[idx2+i].bbox, frustum,
			in_mask, &out_mask) != OUTSIDE)
		{
			VMin(bbox->bbmin, bbox->bbmin, bbox_tree->items[idx2+i].bbox.bbmin);
			VMax(bbox->bbmax, bbox->bbmax, bbox_tree->items[idx2+i].bbox.bbmax);
		}
	}
}

static __inline__ void merge_dyn_items(BBOX_TREE* bbox_tree, unsigned int sub_node,
	const FRUSTUM frustum, unsigned int in_mask, AABBOX* bbox)
{
	unsigned int idx, size, i, out_mask;

	idx = bbox_tree->cur_intersect_type;
	size = bbox_tree->nodes[sub_node].dynamic_objects.index;
		
	for (i = 0; i < size; i++)
	{
		if (check_aabb_in_frustum(bbox_tree->nodes[sub_node].dynamic_objects.items[i].bbox,
			frustum, in_mask, &out_mask) != OUTSIDE)
		{
			VMin(bbox->bbmin, bbox->bbmin, bbox_tree->nodes[sub_node].dynamic_objects.items[i].bbox.bbmin);
			VMax(bbox->bbmax, bbox->bbmax, bbox_tree->nodes[sub_node].dynamic_objects.items[i].bbox.bbmax);			
		}
	}
}

static __inline__ void check_sub_nodes(BBOX_TREE* bbox_tree, unsigned int sub_node, const FRUSTUM frustum, unsigned int in_mask)
{
	unsigned int out_mask, result;
	
	if (sub_node != NO_INDEX)
	{
		result = check_aabb_in_frustum(bbox_tree->nodes[sub_node].bbox, frustum, in_mask, &out_mask);
		if (result == INSIDE)
		{
			add_intersect_items(bbox_tree, bbox_tree->nodes[sub_node].items_index, bbox_tree->nodes[sub_node].items_count);
			add_dyn_intersect_items(bbox_tree, sub_node, bbox_tree->nodes[sub_node].dynamic_objects.index);
			add_dyn_intersect_datas(bbox_tree, sub_node, bbox_tree->nodes[sub_node].dynamic_objects.sub_index);
		}
		else
		{
			if (result == INTERSECT)
			{
				add_dyn_items(bbox_tree, sub_node, frustum, out_mask);
				if (	(bbox_tree->nodes[sub_node].nodes[0] == NO_INDEX) &&
					(bbox_tree->nodes[sub_node].nodes[1] == NO_INDEX)) add_items(bbox_tree, sub_node, frustum, out_mask);
				else
				{
					check_sub_nodes(bbox_tree, bbox_tree->nodes[sub_node].nodes[0], frustum, out_mask);
					check_sub_nodes(bbox_tree, bbox_tree->nodes[sub_node].nodes[1], frustum, out_mask);
				}
			}
		}
	}
}

static __inline__ void calc_bbox_sub_nodes(BBOX_TREE* bbox_tree, unsigned int sub_node, const FRUSTUM frustum, unsigned int in_mask, AABBOX* bbox)
{
	unsigned int out_mask, result;
	
	if (sub_node != NO_INDEX)
	{
		result = check_aabb_in_frustum(bbox_tree->nodes[sub_node].bbox, frustum, in_mask, &out_mask);
		if (result == INSIDE)
		{
			VMin(bbox->bbmin, bbox->bbmin, bbox_tree->nodes[sub_node].bbox.bbmin);
			VMax(bbox->bbmax, bbox->bbmax, bbox_tree->nodes[sub_node].bbox.bbmax);
		}
		else
		{
			if (result == INTERSECT)
			{
				merge_dyn_items(bbox_tree, sub_node, frustum, out_mask, bbox);
				if (	(bbox_tree->nodes[sub_node].nodes[0] == NO_INDEX) &&
					(bbox_tree->nodes[sub_node].nodes[1] == NO_INDEX)) merge_items(bbox_tree, sub_node, frustum, out_mask, bbox);
				else
				{
					calc_bbox_sub_nodes(bbox_tree, bbox_tree->nodes[sub_node].nodes[0], frustum, out_mask, bbox);
					calc_bbox_sub_nodes(bbox_tree, bbox_tree->nodes[sub_node].nodes[1], frustum, out_mask, bbox);
				}
			}
		}
	}
}

static __inline__ int light_depht_comp(BBOX_ITEM_DATA* a, BBOX_ITEM_DATA* b)
{
	unsigned int ai, bi;
	float ax, ay, az, bx, by, bz, ad, bd;

	ai = a->ID;
	bi = b->ID;

	if ((lights_list[ai] == NULL) || (lights_list[bi] == NULL)) return 0;
	
	ax = lights_list[ai]->pos_x + cx;
	ay = lights_list[ai]->pos_y + cy;
	az = lights_list[ai]->pos_z;
	bx = lights_list[bi]->pos_x + cx;
	by = lights_list[bi]->pos_y + cy;
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
	BBOX_ITEM_DATA *a, *b;
	unsigned int am, bm;

	a = (BBOX_ITEM_DATA *)in_a;
	b = (BBOX_ITEM_DATA *)in_b;
	
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
					if (am == bm) return memcmp(a->md5, b->md5, sizeof(MD5_DIGEST));
					else return 1;
				}
				return 0;
			}
		}
		else return 1;
	}
}

static __inline__ void build_start_stop(BBOX_TREE* bbox_tree)
{
	unsigned int idx, i, cur_type, type;

	idx = bbox_tree->cur_intersect_type;
	memset(bbox_tree->intersect[idx].start, 0, TYPES_COUNT*sizeof(unsigned int));
	memset(bbox_tree->intersect[idx].stop, 0, TYPES_COUNT*sizeof(unsigned int));

	if (bbox_tree->intersect[idx].count > 0)
	{
		i = 0;
		cur_type = bbox_tree->intersect[idx].items[i].type;
		if (cur_type == TYPE_DELETED) return;
		
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
			}
		}
		bbox_tree->intersect[idx].stop[cur_type] = i;
	}
}

static __inline__ void delete_item_from_intersect_list(BBOX_TREE* bbox_tree, unsigned int ID, unsigned int type)
{
	unsigned int i, j, size, idx;
	int start, stop;
	
	idx = bbox_tree->cur_intersect_type;
	
	for (i = 0; i < MAX_INTERSECTION_TYPES; i++)
	{
		start = bbox_tree->intersect[i].start[type];
		stop = bbox_tree->intersect[i].stop[type];
		for (j = start; j < stop; j++)
		{
			if (bbox_tree->intersect[i].items[j].ID == ID)
			{
				size = stop - j -1;
				if (size > 0) memmove(&bbox_tree->intersect[i].items[j], &bbox_tree->intersect[i].items[j+1], size*sizeof(BBOX_ITEM_DATA));
				bbox_tree->intersect[i].stop[type]--;
			}
		}
	}
}

void check_bbox_tree(BBOX_TREE* bbox_tree, const FRUSTUM frustum, unsigned int mask)
{	
	unsigned int idx;

	if (bbox_tree != NULL)
	{
		idx = bbox_tree->cur_intersect_type;
		if (bbox_tree->intersect[idx].intersect_update_needed > 0)
		{
			bbox_tree->intersect[idx].count = 0;
			check_sub_nodes(bbox_tree, 0, frustum, mask);
			qsort((void *)(bbox_tree->intersect[idx].items), bbox_tree->intersect[idx].count, sizeof(BBOX_ITEM_DATA), comp_items);
			build_start_stop(bbox_tree);
			bbox_tree->intersect[idx].intersect_update_needed = 0;
		}
	}	
	else BBOX_TREE_LOG_INFO("bbox_tree");
}

void calc_scene_bbox(BBOX_TREE* bbox_tree, const FRUSTUM frustum, AABBOX* bbox)
{	
	if (bbox_tree != NULL)
	{
		VFill(bbox->bbmax, -BOUND_HUGE);
		VFill(bbox->bbmin, BOUND_HUGE);
		calc_bbox_sub_nodes(bbox_tree, 0, frustum, 63, bbox);
	}	
	else BBOX_TREE_LOG_INFO("bbox_tree");
}

static __inline__ void free_bbox_tree_data(BBOX_TREE* bbox_tree)
{
	unsigned int i;
	
	if (bbox_tree->items != NULL) 
	{
		free(bbox_tree->items);
		bbox_tree->items = NULL;
	}
	else BBOX_TREE_LOG_INFO("bbox_tree->items");
	for (i = 0; i < bbox_tree->nodes_count; i++)
	{
		if (bbox_tree->nodes[i].dynamic_objects.items != NULL)
		{
			free(bbox_tree->nodes[i].dynamic_objects.items);
			bbox_tree->nodes[i].dynamic_objects.items = NULL;
		}
		if (bbox_tree->nodes[i].dynamic_objects.sub_items != NULL)
		{
			free(bbox_tree->nodes[i].dynamic_objects.sub_items);
			bbox_tree->nodes[i].dynamic_objects.sub_items = NULL;
		}
	}
	if (bbox_tree->nodes != NULL)
	{
		free(bbox_tree->nodes);
		bbox_tree->nodes = NULL;
	}
	else BBOX_TREE_LOG_INFO("bbox_tree->nodes");
}

void clear_bbox_tree(BBOX_TREE* bbox_tree)
{
	unsigned int i;
	
	if (bbox_tree != NULL)
	{
		for (i = 0; i < MAX_INTERSECTION_TYPES; i++)
		{
			memset(bbox_tree->intersect[i].start, 0, TYPES_COUNT*sizeof(unsigned int));
			memset(bbox_tree->intersect[i].stop, 0, TYPES_COUNT*sizeof(unsigned int));
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

int Axis = 0;

static int compboxes(const void *in_a, const void *in_b)
{
	BBOX_ITEM *a, *b;
	float am, bm;

	a = (BBOX_ITEM *)in_a;
	b = (BBOX_ITEM *)in_b;

	am = a->bbox.bbmin[Axis];
	bm = b->bbox.bbmin[Axis];

	if (am < bm)
	{
		return (-1);
	}
	else
	{
		if (am == bm)
		{
			return (0);
		}
		else
		{
			return (1);
		}
	}
}

static __inline__ void build_area_table(BBOX_TREE *bbox_tree, unsigned int a, unsigned int  b, float *areas)
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

static __inline__ void find_axis(BBOX_TREE *bbox_tree, unsigned int first, unsigned int last, unsigned int *ret)
{
	unsigned int i, a1, a2, a3;
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

static __inline__ void calc_bbox(AABBOX *bbox, BBOX_TREE *bbox_tree, unsigned int first, unsigned int last)
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

static __inline__ unsigned int sort_and_split(BBOX_TREE* bbox_tree, unsigned int node, unsigned int* index, unsigned int first, unsigned int last)
{
	unsigned int size, i, j, axis[3];
	int best_loc;
	float *area_left, *area_right;
	float best_index, new_index;

	find_axis(bbox_tree, first, last, axis);

	size = last - first;

	if (size < 1) return (1);

	best_loc = -1;
	
	if (size > 4)
	{

		area_left = (float *)malloc(size * sizeof(float));
		area_right = (float *)malloc(size * sizeof(float));

		for (j = 0; j < 3; j++)
		{
			Axis = axis[j];
			
			qsort((void *)(&(bbox_tree->items[first])), size, sizeof(BBOX_ITEM), compboxes);
			build_area_table(bbox_tree, first, last - 1, area_left);
			build_area_table(bbox_tree, last - 1, first, area_right);
			
			best_index = area_right[0] * 1.5f;

			for (i = 0; i < size - 1; i++)
			{
				new_index = (area_left[i] + area_right[i + 1])*sqrt(((i+1)*(i+1)+(size-i-1)*(size-i-1))/(size*size));

				if (new_index < best_index)
				{
					best_index = new_index;
					best_loc = i + first;
				}
			}
			if (best_loc >= 0) break;
		}

		best_loc = (first + last)/2;
		free(area_left);
		free(area_right);
	}
	
	calc_bbox(&bbox_tree->nodes[node].bbox, bbox_tree, first, last);
	VAssign(bbox_tree->nodes[node].orig_bbox.bbmin, bbox_tree->nodes[node].bbox.bbmin);
	VAssign(bbox_tree->nodes[node].orig_bbox.bbmax, bbox_tree->nodes[node].bbox.bbmax);
	bbox_tree->nodes[node].items_index = first;
	bbox_tree->nodes[node].items_count = size;

	bbox_tree->nodes[node].dynamic_objects.size = 0;
	bbox_tree->nodes[node].dynamic_objects.index = 0;
	bbox_tree->nodes[node].dynamic_objects.sub_size = 0;
	bbox_tree->nodes[node].dynamic_objects.sub_index = 0;
	bbox_tree->nodes[node].dynamic_objects.items = NULL;
	bbox_tree->nodes[node].dynamic_objects.sub_items = NULL;

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

void init_bbox_tree(BBOX_TREE* bbox_tree, BBOX_ITEMS *bbox_items)
{
	unsigned int size, index;

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

static __inline__ void add_aabb_to_list(BBOX_ITEMS *bbox_items, const AABBOX bbox, unsigned int ID, unsigned int type, unsigned int texture_id, const MD5_DIGEST md5)
{
	unsigned int index, size;

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
	bbox_items->items[index].data.type = type;
	bbox_items->items[index].data.extra = 0;
	bbox_items->items[index].data.texture_id = texture_id;
	memcpy(bbox_items->items[index].data.md5, md5, sizeof(MD5_DIGEST));
	bbox_items->items[index].data.ID = ID;
	bbox_items->index = index + 1;
}

void add_light_to_list(BBOX_ITEMS *bbox_items, unsigned int ID, const AABBOX bbox)
{
	add_aabb_to_list(bbox_items, bbox, ID, TYPE_LIGHT, 0, ZERO_MD5);
}

static __inline__ unsigned int get_3D_type(unsigned int blend, unsigned int ground, unsigned int alpha, unsigned int self_lit)
{
	unsigned int type;

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
	}
}

void add_3dobject_to_list(BBOX_ITEMS *bbox_items, unsigned int ID, const AABBOX bbox, unsigned int blend, unsigned int ground, unsigned int alpha, unsigned int self_lit, unsigned int texture_id, const MD5_DIGEST md5)
{
	add_aabb_to_list(bbox_items, bbox, ID, get_3D_type(blend, ground, alpha, self_lit), texture_id, md5);
}

static __inline__ unsigned int get_2D_type(unsigned int alpha)
{
	if (alpha == 0) return TYPE_2D_NO_ALPHA_OBJECT;
	else return TYPE_2D_ALPHA_OBJECT;
}

void add_2dobject_to_list(BBOX_ITEMS *bbox_items, unsigned int ID, const AABBOX bbox, unsigned int alpha, unsigned int texture_id)
{
	add_aabb_to_list(bbox_items, bbox, ID, get_2D_type(alpha), texture_id, ZERO_MD5);
}

static __inline__ unsigned int get_blend_type(unsigned int blend)
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

static __inline__ unsigned int get_particle_type(unsigned int sblend, unsigned int dblend)
{
	return ((get_blend_type(sblend) << 4) + get_blend_type(dblend));
}

void add_particle_sys_to_list(BBOX_ITEMS *bbox_items, unsigned int ID, const AABBOX bbox, unsigned int sblend, unsigned int dblend)
{
	add_aabb_to_list(bbox_items, bbox, ID, TYPE_PARTICLE_SYSTEM, get_particle_type(sblend, dblend), ZERO_MD5);
}

void add_terrain_to_list(BBOX_ITEMS *bbox_items, unsigned int ID, const AABBOX bbox, unsigned int texture_id)
{
	add_aabb_to_list(bbox_items, bbox, ID, TYPE_TERRAIN, texture_id, ZERO_MD5);
}

static __inline__ unsigned int get_water_type(unsigned int reflectiv)
{
	if (reflectiv) return TYPE_REFLECTIV_WATER;
	else return TYPE_NO_REFLECTIV_WATER;
}

void add_water_to_list(BBOX_ITEMS *bbox_items, unsigned int ID, const AABBOX bbox, unsigned int texture_id, unsigned int reflectiv)
{
	add_aabb_to_list(bbox_items, bbox, ID, get_water_type(reflectiv), texture_id, ZERO_MD5);
}

static __inline__ unsigned int check_aabb_aabb(const AABBOX bbox, const AABBOX dyn_bbox, float grow)
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

static __inline__ void add_dynamic_item_to_node(BBOX_TREE *bbox_tree, unsigned int node, const AABBOX bbox, unsigned int ID, unsigned int type, unsigned int texture_id, const MD5_DIGEST md5)
{	
	unsigned int index, size;

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

		bbox_tree->nodes[node].dynamic_objects.items[index].data.ID = ID;
		bbox_tree->nodes[node].dynamic_objects.items[index].data.extra = 0;
		bbox_tree->nodes[node].dynamic_objects.items[index].data.texture_id = texture_id;
		memcpy(bbox_tree->nodes[node].dynamic_objects.items[index].data.md5, md5, sizeof(MD5_DIGEST));
		bbox_tree->nodes[node].dynamic_objects.items[index].data.type = type;
		VAssign(bbox_tree->nodes[node].dynamic_objects.items[index].bbox.bbmin, bbox.bbmin);
		VAssign(bbox_tree->nodes[node].dynamic_objects.items[index].bbox.bbmax, bbox.bbmax);
		bbox_tree->nodes[node].dynamic_objects.index = index + 1;
		VMin(bbox_tree->nodes[node].bbox.bbmin, bbox_tree->nodes[node].bbox.bbmin, bbox.bbmin);
		VMax(bbox_tree->nodes[node].bbox.bbmax, bbox_tree->nodes[node].bbox.bbmax, bbox.bbmax);
	}
}

static __inline__ void add_dynamic_data_to_node(BBOX_TREE *bbox_tree, unsigned int node, const AABBOX bbox, unsigned int ID, unsigned int type, unsigned int texture_id, const MD5_DIGEST md5, unsigned int extra)
{	
	unsigned int index, size;

	if (node != NO_INDEX)
	{
		index = bbox_tree->nodes[node].dynamic_objects.sub_index;
		size = bbox_tree->nodes[node].dynamic_objects.sub_size;

		if (size <= index)
		{
			if (size < 4) size = 4;
			size *= 2;
			bbox_tree->nodes[node].dynamic_objects.sub_items = (BBOX_ITEM_DATA*)realloc(bbox_tree->nodes[node].dynamic_objects.sub_items, 
					size*sizeof(BBOX_ITEM_DATA));
			bbox_tree->nodes[node].dynamic_objects.sub_size = size;
		}

		bbox_tree->nodes[node].dynamic_objects.sub_items[index].ID = ID;
		bbox_tree->nodes[node].dynamic_objects.sub_items[index].extra = extra;
		bbox_tree->nodes[node].dynamic_objects.sub_items[index].texture_id = texture_id;
		memcpy(bbox_tree->nodes[node].dynamic_objects.sub_items[index].md5, md5, sizeof(MD5_DIGEST));
		bbox_tree->nodes[node].dynamic_objects.sub_items[index].type = type;
		bbox_tree->nodes[node].dynamic_objects.sub_index = index + 1;
		VMin(bbox_tree->nodes[node].bbox.bbmin, bbox_tree->nodes[node].bbox.bbmin, bbox.bbmin);
		VMax(bbox_tree->nodes[node].bbox.bbmax, bbox_tree->nodes[node].bbox.bbmax, bbox.bbmax);
	}
}

static __inline__ int add_dynamic_aabb_to_abt_node(BBOX_TREE *bbox_tree, unsigned int node, const AABBOX bbox, unsigned int ID, unsigned int type, unsigned int texture_id, const MD5_DIGEST md5)
{
	unsigned int result, extra;

	if (node != NO_INDEX)
	{
		if (check_aabb_aabb(bbox_tree->nodes[node].orig_bbox, bbox, 1.1f))
		{
			extra = EXTRA_FIRST_SUB_NODE;
			result = add_dynamic_aabb_to_abt_node(bbox_tree, bbox_tree->nodes[node].nodes[0], bbox, ID, type, texture_id, md5);
			if (result == 0)
			{
				result = add_dynamic_aabb_to_abt_node(bbox_tree, bbox_tree->nodes[node].nodes[1], bbox, ID, type, texture_id, md5);
				extra = 0;
			}
			if (result == 0) add_dynamic_item_to_node(bbox_tree, node, bbox, ID, type, texture_id, md5);
			else add_dynamic_data_to_node(bbox_tree, node, bbox, ID, type, texture_id, md5, extra);
			return 1;
		}
		else return 0;
	}
	else return 0;
}

static __inline__ void add_aabb_to_abt(BBOX_TREE *bbox_tree, const AABBOX bbox, unsigned int ID, unsigned int type, unsigned int texture_id, const MD5_DIGEST md5, unsigned int dynamic)
{
	unsigned int result;
	
	if (bbox_tree != NULL)
	{
		result = add_dynamic_aabb_to_abt_node(bbox_tree, 0, bbox, ID, type, texture_id, md5);
		if (result == 0) add_dynamic_item_to_node(bbox_tree, 0, bbox, ID, type, texture_id, md5);
		set_all_intersect_update_needed(bbox_tree);
	}
	else BBOX_TREE_LOG_INFO("bbox_tree");
}

void add_light_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, const AABBOX bbox, unsigned int dynamic)
{
	add_aabb_to_abt(bbox_tree, bbox, ID, TYPE_LIGHT, 0, ZERO_MD5, dynamic);
}

void add_3dobject_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, const AABBOX bbox, unsigned int blend, unsigned int ground, unsigned int alpha, unsigned int self_lit, unsigned int texture_id, const MD5_DIGEST md5, unsigned int dynamic)
{
	add_aabb_to_abt(bbox_tree, bbox, ID, get_3D_type(blend, ground, alpha, self_lit), texture_id, md5, dynamic);
}

void add_2dobject_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, const AABBOX bbox, unsigned int alpha, unsigned int texture_id, unsigned int dynamic)
{
	add_aabb_to_abt(bbox_tree, bbox, ID, get_2D_type(alpha), texture_id, ZERO_MD5, dynamic);
}

void add_particle_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, const AABBOX bbox, unsigned int sblend, unsigned int dblend, unsigned int dynamic)
{
	add_aabb_to_abt(bbox_tree, bbox, ID, TYPE_PARTICLE_SYSTEM, get_particle_type(sblend, dblend), ZERO_MD5, dynamic);
}

void add_terrain_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, const AABBOX bbox, unsigned int texture_id, unsigned int dynamic)
{
	add_aabb_to_abt(bbox_tree, bbox, ID, TYPE_TERRAIN, texture_id, ZERO_MD5, dynamic);
}

void add_water_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, const AABBOX bbox, unsigned int texture_id, unsigned int reflectiv, unsigned int dynamic)
{
	add_aabb_to_abt(bbox_tree, bbox, ID, get_water_type(reflectiv), texture_id, ZERO_MD5, dynamic);
}

static __inline__ void delete_dynamic_item_from_node(BBOX_TREE *bbox_tree, unsigned int node, unsigned int idx)
{
	int index, size, count;
	
	index = bbox_tree->nodes[node].dynamic_objects.index;
	size = bbox_tree->nodes[node].dynamic_objects.size;
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

static __inline__ unsigned int delete_dynamic_aabb_from_node(BBOX_TREE *bbox_tree, unsigned int node, unsigned int ID, unsigned int type);

static __inline__ void delete_dynamic_subitem_from_node(BBOX_TREE *bbox_tree, unsigned int node, unsigned int ID, unsigned int type, unsigned int idx)
{
	int index, size, count;
	
	index = bbox_tree->nodes[node].dynamic_objects.sub_index;
	size = bbox_tree->nodes[node].dynamic_objects.sub_size;

	if (is_extra_first_sub_node(bbox_tree->nodes[node].dynamic_objects.sub_items[idx].extra)) delete_dynamic_aabb_from_node(bbox_tree, bbox_tree->nodes[node].nodes[0], ID, type);
	else delete_dynamic_aabb_from_node(bbox_tree, bbox_tree->nodes[node].nodes[1], ID, type);			

	if (index <= 1)
	{
		size = 0;
		index = 0;
		if (bbox_tree->nodes[node].dynamic_objects.sub_items != NULL) 
		{
			free(bbox_tree->nodes[node].dynamic_objects.sub_items);
			bbox_tree->nodes[node].dynamic_objects.sub_items = NULL;
		}
	}
	else
	{
		if (index < (size/2))
		{
			size /= 2;
			bbox_tree->nodes[node].dynamic_objects.sub_items = (BBOX_ITEM_DATA*)realloc(bbox_tree->nodes[node].dynamic_objects.sub_items, size*sizeof(BBOX_ITEM_DATA));
		}
		
		count = index-idx-1;
		if (count > 0) memmove(&bbox_tree->nodes[node].dynamic_objects.sub_items[idx], &bbox_tree->nodes[node].dynamic_objects.sub_items[idx+1], count*sizeof(BBOX_ITEM_DATA));
		index--;
	}
	bbox_tree->nodes[node].dynamic_objects.sub_index = index;
	bbox_tree->nodes[node].dynamic_objects.sub_size = size;
}

static __inline__ unsigned int dynamic_aabb_is_in_node(BBOX_TREE *bbox_tree, unsigned int node, unsigned int ID, unsigned int type)
{
	unsigned int i, result;

	result = 0;
	
	for (i = 0; i < bbox_tree->nodes[node].dynamic_objects.index; i++)
	{
		if ((bbox_tree->nodes[node].dynamic_objects.items[i].data.ID == ID) && (bbox_tree->nodes[node].dynamic_objects.items[i].data.type == type))
		{
			delete_dynamic_item_from_node(bbox_tree, node, i);
			result = 1;
		}
	}

	for (i = 0; i < bbox_tree->nodes[node].dynamic_objects.sub_index; i++)
	{
		if ((bbox_tree->nodes[node].dynamic_objects.sub_items[i].ID == ID) && (bbox_tree->nodes[node].dynamic_objects.sub_items[i].type == type))
		{
			delete_dynamic_subitem_from_node(bbox_tree, node, ID, type, i);
			result = 2;
		}
	}

	return result;
}

static __inline__ unsigned int delete_dynamic_aabb_from_node(BBOX_TREE *bbox_tree, unsigned int node, unsigned int ID, unsigned int type)
{
	unsigned int i, result, idx1, idx2;
	AABBOX new_bbox;

	if (node != NO_INDEX)
	{		
		result = dynamic_aabb_is_in_node(bbox_tree, node, ID, type);
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

static __inline__ void delete_aabb_from_abt(BBOX_TREE *bbox_tree, unsigned int ID, unsigned int type)
{
	if (bbox_tree != NULL)
	{
		delete_item_from_intersect_list(bbox_tree, ID, type);
		delete_dynamic_aabb_from_node(bbox_tree, 0, ID, type);
	}
	else BBOX_TREE_LOG_INFO("bbox_tree");
}

void delete_3dobject_from_abt(BBOX_TREE *bbox_tree, unsigned int ID, unsigned int blend, unsigned int ground, unsigned int alpha, unsigned int self_lit)
{
	delete_aabb_from_abt(bbox_tree, ID, get_3D_type(blend, ground, alpha, self_lit));
}

void delete_2dobject_from_abt(BBOX_TREE *bbox_tree, unsigned int ID, unsigned int alpha)
{
	delete_aabb_from_abt(bbox_tree, ID, get_2D_type(alpha));
}

void delete_particle_from_abt(BBOX_TREE *bbox_tree, unsigned int ID)
{
	delete_aabb_from_abt(bbox_tree, ID, TYPE_PARTICLE_SYSTEM);
}

void delete_light_from_abt(BBOX_TREE *bbox_tree, unsigned int ID)
{
	delete_aabb_from_abt(bbox_tree, ID, TYPE_LIGHT);
}

void delete_terrain_from_abt(BBOX_TREE *bbox_tree, unsigned int ID)
{
	delete_aabb_from_abt(bbox_tree, ID, TYPE_TERRAIN);
}

void delete_water_from_abt(BBOX_TREE *bbox_tree, unsigned int ID, unsigned int reflectiv)
{
	delete_aabb_from_abt(bbox_tree, ID, get_water_type(reflectiv));
}

BBOX_ITEMS* create_bbox_items(unsigned int size)
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
	unsigned int i, type_ids;
	
	type_ids = 0;

	bbox_tree = (BBOX_TREE*)malloc(sizeof(BBOX_TREE));
	
	bbox_tree->cur_intersect_type = INTERSECTION_TYPE_DEFAULT;
	for (i = 0; i < MAX_INTERSECTION_TYPES; i++)
	{
		bbox_tree->intersect[i].size = 8;
		bbox_tree->intersect[i].count = 0;
		bbox_tree->intersect[i].items = (BBOX_ITEM_DATA*)malloc(8*sizeof(BBOX_ITEM_DATA));
	}
	bbox_tree->nodes_count = 0;
	bbox_tree->nodes = NULL;
	bbox_tree->items_count = 0;
	bbox_tree->items = NULL;
	return bbox_tree;
}
#endif
