#ifdef	NEW_FRUSTUM
#ifdef MAP_EDITOR2
#include "../map_editor2/global.h"
#else
#include "global.h"
#endif
#ifndef	BSD
#include <malloc.h>
#endif

void set_all_intersect_update_needed(BBOX_TREE* bbox_tree)
{
	unsigned int i;

	if (bbox_tree != NULL)
	{
		for (i = 0; i < MAX_ITERSECTION_TYPES; i++)
			bbox_tree->intersect[i].intersect_update_needed = 1;
	}
}

#ifdef	FRUSTUM_THREADS
static __inline__ void update_bbox_tree_degeneration(BBOX_TREE* bbox_tree, unsigned int count)
{
	bbox_tree->update_data.bbox_tree_degeneration += count;
	if (bbox_tree->update_data.bbox_tree_degeneration > 100) SDL_CondBroadcast(bbox_tree->update_condition);
}
#endif

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

static __inline__ void add_dyn_intersect_item(BBOX_TREE* bbox_tree, BBOX_TREE_NODE* node, unsigned int index, unsigned int idx)
{
	adapt_intersect_list_size(bbox_tree, 1);
	memcpy(&bbox_tree->intersect[idx].items[bbox_tree->intersect[idx].count], &node->dynamic_objects.items[index].data, sizeof(BBOX_ITEM_DATA));
	bbox_tree->intersect[idx].count++;
}

static __inline__ void add_dyn_intersect_items(BBOX_TREE* bbox_tree, BBOX_TREE_NODE* node, unsigned int count)
{
	unsigned int i;
	
	adapt_intersect_list_size(bbox_tree, count);
	for (i = 0; i < count; i++) add_dyn_intersect_item(bbox_tree, node, i, bbox_tree->cur_intersect_type);
}

static __inline__ void add_dyn_intersect_data(BBOX_TREE* bbox_tree, BBOX_TREE_NODE* node, unsigned int index, unsigned int idx)
{
	adapt_intersect_list_size(bbox_tree, 1);
	memcpy(&bbox_tree->intersect[idx].items[bbox_tree->intersect[idx].count], &node->dynamic_objects.sub_items[index], sizeof(BBOX_ITEM_DATA));
	bbox_tree->intersect[idx].count++;
}

static __inline__ void add_dyn_intersect_datas(BBOX_TREE* bbox_tree, BBOX_TREE_NODE* node, unsigned int count)
{
	unsigned int idx;
	
	idx = bbox_tree->cur_intersect_type;	
	adapt_intersect_list_size(bbox_tree, count);
	memcpy(&bbox_tree->intersect[idx].items[bbox_tree->intersect[idx].count], node->dynamic_objects.sub_items, count*sizeof(BBOX_ITEM_DATA));
	bbox_tree->intersect[idx].count += count;
}

#ifdef	FRUSTUM_THREADS
static __inline__ void add_add_list_item(BBOX_TREE* bbox_tree, unsigned int index, unsigned int idx)
{
	adapt_intersect_list_size(bbox_tree, 1);
	memcpy(&bbox_tree->intersect[idx].items[bbox_tree->intersect[idx].count], &bbox_tree->update_data.list[index].item.data, sizeof(BBOX_ITEM_DATA));
	bbox_tree->intersect[idx].count++;
}
#endif

static __inline__ int check_aabb_in_frustum(AABBOX *bbox, PLANE *frustum, unsigned int in_mask, unsigned int *out_mask)
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

			nx = !frustum[i].mask[0] ? bbox->bbmin[X] :  bbox->bbmax[X];
			px = frustum[i].mask[0] ? bbox->bbmin[X] :  bbox->bbmax[X];
			ny = !frustum[i].mask[1] ? bbox->bbmin[Y] :  bbox->bbmax[Y];
			py = frustum[i].mask[1] ? bbox->bbmin[Y] :  bbox->bbmax[Y];
			nz = !frustum[i].mask[2] ? bbox->bbmin[Z] :  bbox->bbmax[Z];
			pz = frustum[i].mask[2] ? bbox->bbmin[Z] :  bbox->bbmax[Z];
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

static __inline__ void add_items(BBOX_TREE* bbox_tree, BBOX_TREE_NODE* sub_node, PLANE *frustum, unsigned int in_mask)
{
	unsigned int idx1, idx2, size, i, out_mask;

	idx1 = bbox_tree->cur_intersect_type;
	idx2 = sub_node->items_index;
	size = sub_node->items_count;
		
	for (i = 0; i < size; i++)
	{
		if (check_aabb_in_frustum(&bbox_tree->items[idx2+i].bbox, frustum, in_mask, &out_mask) != OUTSIDE) 
			add_intersect_item(bbox_tree, idx2+i, idx1);
	}
}

static __inline__ void add_dyn_items(BBOX_TREE* bbox_tree, BBOX_TREE_NODE* sub_node, PLANE *frustum, unsigned int in_mask)
{
	unsigned int idx, size, i, out_mask;

	idx = bbox_tree->cur_intersect_type;
	size = sub_node->dynamic_objects.index;
		
	for (i = 0; i < size; i++)
	{
		if (check_aabb_in_frustum(&sub_node->dynamic_objects.items[i].bbox, frustum, in_mask, &out_mask) != OUTSIDE) 
			add_dyn_intersect_item(bbox_tree, sub_node, i, idx);
	}
}

static __inline__ void merge_items(BBOX_TREE* bbox_tree, BBOX_TREE_NODE* sub_node,
	PLANE *frustum, unsigned int in_mask, AABBOX* bbox)
{
	unsigned int idx1, idx2, size, i, out_mask;

	idx1 = bbox_tree->cur_intersect_type;
	idx2 = sub_node->items_index;
	size = sub_node->items_count;
		
	for (i = 0; i < size; i++)
	{
		if (check_aabb_in_frustum(&bbox_tree->items[idx2+i].bbox, frustum,
			in_mask, &out_mask) != OUTSIDE)
		{
			VMin(bbox->bbmin, bbox->bbmin, bbox_tree->items[idx2+i].bbox.bbmin);
			VMax(bbox->bbmax, bbox->bbmax, bbox_tree->items[idx2+i].bbox.bbmax);
		}
	}
}

static __inline__ void merge_dyn_items(BBOX_TREE* bbox_tree, BBOX_TREE_NODE* sub_node,
	PLANE *frustum, unsigned int in_mask, AABBOX* bbox)
{
	unsigned int idx, size, i, out_mask;

	idx = bbox_tree->cur_intersect_type;
	size = sub_node->dynamic_objects.index;
		
	for (i = 0; i < size; i++)
	{
		if (check_aabb_in_frustum(&sub_node->dynamic_objects.items[i].bbox,
			frustum, in_mask, &out_mask) != OUTSIDE)
		{
			VMin(bbox->bbmin, bbox->bbmin, sub_node->dynamic_objects.items[i].bbox.bbmin);
			VMax(bbox->bbmax, bbox->bbmax, sub_node->dynamic_objects.items[i].bbox.bbmax);			
		}
	}
}

#ifdef	FRUSTUM_THREADS
static __inline__ void add_add_list_items(BBOX_TREE* bbox_tree, PLANE *frustum, unsigned int in_mask)
{
	unsigned int idx, size, i, out_mask;

	idx = bbox_tree->cur_intersect_type;
	size = bbox_tree->update_data.index;
		
	for (i = 0; i < size; i++)
	{
		if (check_aabb_in_frustum(&bbox_tree->update_data.list[i].item.bbox, frustum, in_mask, &out_mask) != OUTSIDE) 
			add_add_list_item(bbox_tree, i, idx);
	}
}
#endif

static __inline__ void check_sub_nodes(BBOX_TREE* bbox_tree, BBOX_TREE_NODE* sub_node, PLANE *frustum, unsigned int in_mask)
{
	unsigned int out_mask, result;
	
	if (sub_node != NULL)
	{
		result = check_aabb_in_frustum(&sub_node->bbox, frustum, in_mask, &out_mask);
		if (result == INSIDE)
		{
			add_intersect_items(bbox_tree, sub_node->items_index, sub_node->items_count);
			add_dyn_intersect_items(bbox_tree, sub_node, sub_node->dynamic_objects.index);
			add_dyn_intersect_datas(bbox_tree, sub_node, sub_node->dynamic_objects.sub_index);
		}
		else
		{
			if (result == INTERSECT)
			{
				add_dyn_items(bbox_tree, sub_node, frustum, out_mask);
				if (sub_node->nodes[0] == NULL)	add_items(bbox_tree, sub_node, frustum, out_mask);
				else
				{
					check_sub_nodes(bbox_tree, sub_node->nodes[0], frustum, out_mask);
					check_sub_nodes(bbox_tree, sub_node->nodes[1], frustum, out_mask);
				}
			}
		}
	}
}

static __inline__ void calc_bbox_sub_nodes(BBOX_TREE* bbox_tree, BBOX_TREE_NODE* sub_node, PLANE *frustum, unsigned int in_mask, AABBOX* bbox)
{
	unsigned int out_mask, result;
	
	if (sub_node != NULL)
	{
		result = check_aabb_in_frustum(&sub_node->bbox, frustum, in_mask, &out_mask);
		if (result == INSIDE)
		{
			VMin(bbox->bbmin, bbox->bbmin, sub_node->bbox.bbmin);
			VMax(bbox->bbmax, bbox->bbmax, sub_node->bbox.bbmax);
		}
		else
		{
			if (result == INTERSECT)
			{
				merge_dyn_items(bbox_tree, sub_node, frustum, out_mask, bbox);
				if (sub_node->nodes[0] == NULL)	merge_items(bbox_tree, sub_node, frustum, out_mask, bbox);
				else
				{
					calc_bbox_sub_nodes(bbox_tree, sub_node->nodes[0], frustum, out_mask, bbox);
					calc_bbox_sub_nodes(bbox_tree, sub_node->nodes[1], frustum, out_mask, bbox);
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
				am = a->sort_data;
				bm = b->sort_data;
				if (am < bm) return -1;
				else
				{
					if (am == bm) return 0;
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
	memset(bbox_tree->intersect[idx].start, 0, TYPES_COUNT*sizeof(IDX_TYPE));
	memset(bbox_tree->intersect[idx].stop, 0, TYPES_COUNT*sizeof(IDX_TYPE));

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
	int i_min, i_max;
	
	idx = bbox_tree->cur_intersect_type;
	
	for (j = 0; j < MAX_ITERSECTION_TYPES; j++)
	{
		i_min = bbox_tree->intersect[j].start[type];
		i_max = bbox_tree->intersect[j].stop[type]-1;
		while (i_min <= i_max)
		{
			i = (i_min+i_max)/2;
			if (bbox_tree->intersect[j].items[i].ID == ID)
			{
				size = bbox_tree->intersect[j].stop[type] - i -1;
				memmove(&bbox_tree->intersect[j].items[i], &bbox_tree->intersect[j].items[i+1], size*sizeof(BBOX_ITEM_DATA));
				bbox_tree->intersect[j].stop[type]--;
				bbox_tree->intersect[j].intersect_update_needed = 1;
				break;
			}
			else
			{
				if (bbox_tree->intersect[j].items[i].ID < ID) i_max = i-1;
				else i_min = i+1;
			}
		}
	}
}

void check_bbox_tree(BBOX_TREE* bbox_tree, FRUSTUM *frustum, unsigned int mask)
{	
	unsigned int idx;

	if (bbox_tree != NULL)
	{
		lock_bbox_tree(bbox_tree);
		idx = bbox_tree->cur_intersect_type;
		if (bbox_tree->intersect[idx].intersect_update_needed > 0)
		{
			bbox_tree->intersect[idx].count = 0;
			check_sub_nodes(bbox_tree, bbox_tree->root_node, *frustum, mask);
#ifdef	FRUSTUM_THREADS
			add_add_list_items(bbox_tree, *frustum, mask);
#endif
			qsort((void *)(bbox_tree->intersect[idx].items), bbox_tree->intersect[idx].count, sizeof(BBOX_ITEM_DATA), comp_items);
			build_start_stop(bbox_tree);
			bbox_tree->intersect[idx].intersect_update_needed = 0;
		}
		unlock_bbox_tree(bbox_tree);
	}	
}

void calc_scene_bbox(BBOX_TREE* bbox_tree, FRUSTUM *frustum, AABBOX* bbox)
{	
	if (bbox_tree != NULL)
	{
		lock_bbox_tree(bbox_tree);
		VFill(bbox->bbmax, -BOUND_HUGE);
		VFill(bbox->bbmin, BOUND_HUGE);
		calc_bbox_sub_nodes(bbox_tree, bbox_tree->root_node, *frustum, 63, bbox);
		unlock_bbox_tree(bbox_tree);
	}	
}

static __inline__ void free_bbox_tree_data(BBOX_TREE* bbox_tree)
{
	unsigned int i;
	
	if (bbox_tree->items != NULL) 
	{
		free(bbox_tree->items);
		bbox_tree->items = NULL;
	}
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
}

void clear_bbox_tree(BBOX_TREE* bbox_tree)
{
	unsigned int i;
	
	if (bbox_tree != NULL)
	{
		lock_bbox_tree(bbox_tree);
		for (i = 0; i < MAX_ITERSECTION_TYPES; i++)
		{
			memset(bbox_tree->intersect[i].start, 0, TYPES_COUNT*sizeof(IDX_TYPE));
			memset(bbox_tree->intersect[i].stop, 0, TYPES_COUNT*sizeof(IDX_TYPE));
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
#ifdef	FRUSTUM_THREADS
		if (bbox_tree->update_data.list != NULL)
		{
			free(bbox_tree->update_data.list);
			bbox_tree->update_data.list = NULL;
		}
#endif
		unlock_bbox_tree(bbox_tree);
	}
}

void free_bbox_tree(BBOX_TREE* bbox_tree)
{
#ifdef	FRUSTUM_THREADS
	int ret;
#endif

	if (bbox_tree != NULL)
	{
#ifdef	FRUSTUM_THREADS
		lock_bbox_tree(bbox_tree);
		bbox_tree->done = 1;
		unlock_bbox_tree(bbox_tree);
		SDL_CondBroadcast(bbox_tree->update_condition);
		SDL_WaitThread(bbox_tree->thread_id, &ret);
		SDL_DestroyCond(bbox_tree->update_condition);
#endif
		clear_bbox_tree(bbox_tree);
		SDL_DestroyMutex(bbox_tree->bbox_tree_mutex);
		bbox_tree->bbox_tree_mutex = NULL;
		free(bbox_tree);
	}
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

	memcpy(bbox->bbmin, bmin, sizeof(VECTOR3));
	memcpy(bbox->bbmax, bmax, sizeof(VECTOR3));
}

static __inline__ unsigned int sort_and_split(BBOX_TREE* bbox_tree, BBOX_TREE_NODE *node, unsigned int* index, unsigned int first, unsigned int last)
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
	
	calc_bbox(&node->bbox, bbox_tree, first, last);
	node->items_index = first;
	node->items_count = size;

	node->dynamic_objects.size = 0;
	node->dynamic_objects.index = 0;
	node->dynamic_objects.sub_size = 0;
	node->dynamic_objects.sub_index = 0;
	node->dynamic_objects.items = NULL;
	node->dynamic_objects.sub_items = NULL;

	if (best_loc < 0) 
	{
		node->nodes[0] = NULL;
		node->nodes[1] = NULL;
		return 1;
	}
	else
	{
		if (*index+2 >= bbox_tree->nodes_count) 
		{
			bbox_tree->nodes_count *= 2;
			bbox_tree->nodes = (BBOX_TREE_NODE*)realloc(bbox_tree->nodes, bbox_tree->nodes_count*sizeof(BBOX_TREE_NODE));
		}
		node->nodes[0] = &bbox_tree->nodes[(*index)+0];
		node->nodes[1] = &bbox_tree->nodes[(*index)+1];
		*index += 2;
		sort_and_split(bbox_tree, node->nodes[0], index, first, best_loc + 1);
		sort_and_split(bbox_tree, node->nodes[1], index, best_loc + 1, last);
		return 0;
	}
}

void init_bbox_tree(BBOX_TREE* bbox_tree, BBOX_ITEMS *bbox_items)
{
	unsigned int size, index;

	if ((bbox_items != NULL) && (bbox_items->index > 0))
	{
		size = bbox_items->index;	
		index = 1;
		bbox_tree->nodes_count = 2*size;
		bbox_tree->nodes = (BBOX_TREE_NODE*)malloc(size*2*sizeof(BBOX_TREE_NODE));
		bbox_tree->root_node = &bbox_tree->nodes[0];
		bbox_tree->items_count = size;
		bbox_tree->items = (BBOX_ITEM*)malloc(size*sizeof(BBOX_ITEM));
		memcpy(bbox_tree->items, bbox_items->items, size*sizeof(BBOX_ITEM));
		sort_and_split(bbox_tree, bbox_tree->root_node, &index, 0, size);
		bbox_tree->nodes_count = index;
		bbox_tree->nodes = (BBOX_TREE_NODE*)realloc(bbox_tree->nodes, index*sizeof(BBOX_TREE_NODE));
	}
	set_all_intersect_update_needed(bbox_tree);
#ifdef	FRUSTUM_THREADS
	bbox_tree->update_data.bbox_tree_degeneration = 0;
	bbox_tree->update_data.size = 0;
	bbox_tree->update_data.index = 0;
	bbox_tree->update_data.list = NULL;
#endif
}

static __inline__ void add_aabb_to_list(BBOX_ITEMS *bbox_items, AABBOX *bbox, unsigned int ID, unsigned int type, unsigned int sort_data)
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
	memcpy(bbox_items->items[index].bbox.bbmin, bbox->bbmin, sizeof(VECTOR3));
	memcpy(bbox_items->items[index].bbox.bbmax, bbox->bbmax, sizeof(VECTOR3));
	bbox_items->items[index].data.type = type;
	bbox_items->items[index].data.sort_data = sort_data;
	bbox_items->items[index].data.ID = ID;
	bbox_items->index = index + 1;
}

void add_light_to_list(BBOX_ITEMS *bbox_items, unsigned int ID, AABBOX *bbox)
{
	add_aabb_to_list(bbox_items, bbox, ID, TYPE_LIGHT, 0);
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

void add_3dobject_to_list(BBOX_ITEMS *bbox_items, unsigned int ID, AABBOX *bbox, unsigned int blend, unsigned int ground, unsigned int alpha, unsigned int self_lit, unsigned int texture_id)
{
	add_aabb_to_list(bbox_items, bbox, ID, get_3D_type(blend, ground, alpha, self_lit), texture_id);
}

static __inline__ unsigned int get_2D_type(unsigned int alpha)
{
	if (alpha == 0) return TYPE_2D_NO_ALPHA_OBJECT;
	else return TYPE_2D_ALPHA_OBJECT;
}

void add_2dobject_to_list(BBOX_ITEMS *bbox_items, unsigned int ID, AABBOX *bbox, unsigned int alpha, unsigned int texture_id)
{
	add_aabb_to_list(bbox_items, bbox, ID, get_2D_type(alpha), texture_id);
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

void add_particle_sys_to_list(BBOX_ITEMS *bbox_items, unsigned int ID, AABBOX *bbox, unsigned int sblend, unsigned int dblend)
{
	add_aabb_to_list(bbox_items, bbox, ID, TYPE_PARTICLE_SYSTEM, get_particle_type(sblend, dblend));
}

void add_terrain_to_list(BBOX_ITEMS *bbox_items, unsigned int ID, AABBOX *bbox, unsigned int texture_id)
{
	add_aabb_to_list(bbox_items, bbox, ID, TYPE_TERRAIN, texture_id);
}

static __inline__ unsigned int get_water_type(unsigned int reflectiv)
{
	if (reflectiv) return TYPE_REFLECTIV_WATER;
	else return TYPE_NO_REFLECTIV_WATER;
}

void add_water_to_list(BBOX_ITEMS *bbox_items, unsigned int ID, AABBOX *bbox, unsigned int texture_id, unsigned int reflectiv)
{
	add_aabb_to_list(bbox_items, bbox, ID, get_water_type(reflectiv), texture_id);
}

static __inline__ unsigned int check_aabb_aabb(AABBOX *bbox, AABBOX *dyn_bbox, AABBOX *new_bbox, float grow)
{
	VECTOR3 len;
	float old_v, new_v;

	VMin(new_bbox->bbmin, bbox->bbmin, dyn_bbox->bbmin);
	VMax(new_bbox->bbmax, bbox->bbmax, dyn_bbox->bbmax);

	VSub(len, bbox->bbmax, bbox->bbmin);
	old_v = len[X] * len[Y] * len[Z];
	VSub(len, new_bbox->bbmax, new_bbox->bbmin);
	new_v = len[X] * len[Y] * len[Z];

	if ((new_v / old_v) > grow) return 0;
	else return 1;
}

static __inline__ void add_dynamic_item_to_node(BBOX_TREE_NODE *node, AABBOX *bbox, unsigned int ID, unsigned int type, unsigned int sort_data)
{	
	unsigned int index, size;

	if (node != NULL)
	{
		index = node->dynamic_objects.index;
		size = node->dynamic_objects.size;

		if (size <= index)
		{
			if (size < 4) size = 4;
			size *= 2;
			node->dynamic_objects.items = (BBOX_ITEM*)realloc(node->dynamic_objects.items, size*sizeof(BBOX_ITEM));
			node->dynamic_objects.size = size;
		}

		node->dynamic_objects.items[index].data.ID = ID;
		node->dynamic_objects.items[index].data.sort_data = sort_data;
		node->dynamic_objects.items[index].data.type = type;
		memcpy(node->dynamic_objects.items[index].bbox.bbmin, bbox->bbmin, sizeof(VECTOR3));
		memcpy(node->dynamic_objects.items[index].bbox.bbmax, bbox->bbmax, sizeof(VECTOR3));
		node->dynamic_objects.index = index + 1;
		VMin(node->bbox.bbmin, node->bbox.bbmin, bbox->bbmin);
		VMax(node->bbox.bbmax, node->bbox.bbmax, bbox->bbmax);
	}
}

static __inline__ void add_dynamic_data_to_node(BBOX_TREE_NODE *node, AABBOX *bbox, unsigned int ID, unsigned int type, unsigned int sort_data)
{	
	unsigned int index, size;

	if (node != NULL)
	{
		index = node->dynamic_objects.sub_index;
		size = node->dynamic_objects.sub_size;

		if (size <= index)
		{
			if (size < 4) size = 4;
			size *= 2;
			node->dynamic_objects.sub_items = (BBOX_ITEM_DATA*)realloc(node->dynamic_objects.sub_items, 
					size*sizeof(BBOX_ITEM_DATA));
			node->dynamic_objects.sub_size = size;
		}

		node->dynamic_objects.sub_items[index].ID = ID;
		node->dynamic_objects.sub_items[index].sort_data = sort_data;
		node->dynamic_objects.sub_items[index].type = type;
		node->dynamic_objects.sub_index = index + 1;
		VMin(node->bbox.bbmin, node->bbox.bbmin, bbox->bbmin);
		VMax(node->bbox.bbmax, node->bbox.bbmax, bbox->bbmax);
	}
}

static __inline__ int add_dynamic_aabb_to_abt_node(BBOX_TREE_NODE *node, AABBOX *bbox, unsigned int ID, unsigned int type, unsigned int sort_data)
{
	AABBOX new_bbox;
	unsigned int r;

	if (node != NULL)
	{
		if (check_aabb_aabb(&node->bbox, bbox, &new_bbox, 1.1f))
		{
			r = add_dynamic_aabb_to_abt_node(node->nodes[0], bbox, ID, type, sort_data);
			if (r == 0) r = add_dynamic_aabb_to_abt_node(node->nodes[1], bbox, ID, type, sort_data);
			if (r == 0) add_dynamic_item_to_node(node, bbox, ID, type, sort_data);
			else add_dynamic_data_to_node(node, bbox, ID, type, sort_data);
			return 1;
		}
		else return 0;
	}
	else return 0;
}

#ifdef	FRUSTUM_THREADS
static __inline__ void add_objects_to_update_list(BBOX_TREE *bbox_tree, AABBOX *bbox, unsigned int ID, unsigned int type, unsigned int sort_data, unsigned int dynamic)
{
	unsigned int size, index;
	
	index = bbox_tree->update_data.index;
	size = bbox_tree->update_data.size;
	
	if (index >= size)
	{
		size += max2i(4, size/2);
		bbox_tree->update_data.list = 
			(BBOX_UPDATE_ITEM_DATA*)realloc(bbox_tree->update_data.list, size*sizeof(BBOX_UPDATE_ITEM_DATA));
		bbox_tree->update_data.size = size;
	}
	bbox_tree->update_data.list[index].item.data.ID = ID;
	bbox_tree->update_data.list[index].item.data.type = type;
	bbox_tree->update_data.list[index].item.data.sort_data = sort_data;
	memcpy(bbox_tree->update_data.list[index].item.bbox.bbmim, bbox->bbmin, sizeof(VECTOR3));
	memcpy(bbox_tree->update_data.list[index].item.bbox.bbmax, bbox->bbmax, sizeof(VECTOR3));
	bbox_tree->update_data.list[index].dynamic = dynamic;
	bbox_tree->update_data.index = index + 1;
	update_bbox_tree_degeneration(bbox_tree, 1);
}
#endif

static __inline__ void add_aabb_to_abt(BBOX_TREE *bbox_tree, AABBOX *bbox, unsigned int ID, unsigned int type, unsigned int sort_data, unsigned int dynamic)
{
	if (bbox_tree != NULL)
	{
		lock_bbox_tree(bbox_tree);
#ifdef	FRUSTUM_THREADS
		add_objects_to_update_list(bbox_tree, bbox, ID, type, sort_data, dynamic);
#else
		add_dynamic_aabb_to_abt_node(bbox_tree->root_node, bbox, ID, type, sort_data);
#endif
		set_all_intersect_update_needed(bbox_tree);
		unlock_bbox_tree(bbox_tree);
	}
}

void add_light_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, AABBOX *bbox, unsigned int dynamic)
{
	add_aabb_to_abt(bbox_tree, bbox, ID, TYPE_LIGHT, 0, dynamic);
}

void add_3dobject_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, AABBOX *bbox, unsigned int blend, unsigned int ground, unsigned int alpha, unsigned int self_lit, unsigned int texture_id, unsigned int dynamic)
{
	add_aabb_to_abt(bbox_tree, bbox, ID, get_3D_type(blend, ground, alpha, self_lit), texture_id, dynamic);
}

void add_2dobject_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, AABBOX *bbox, unsigned int alpha, unsigned int texture_id, unsigned int dynamic)
{
	add_aabb_to_abt(bbox_tree, bbox, ID, get_2D_type(alpha), texture_id, dynamic);
}

void add_particle_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, AABBOX *bbox, unsigned int sblend, unsigned int dblend, unsigned int dynamic)
{
	add_aabb_to_abt(bbox_tree, bbox, ID, TYPE_PARTICLE_SYSTEM, get_particle_type(sblend, dblend), dynamic);
}

void add_terrain_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, AABBOX *bbox, unsigned int texture_id, unsigned int dynamic)
{
	add_aabb_to_abt(bbox_tree, bbox, ID, TYPE_TERRAIN, texture_id, dynamic);
}

void add_water_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, AABBOX *bbox, unsigned int texture_id, unsigned int reflectiv, unsigned int dynamic)
{
	add_aabb_to_abt(bbox_tree, bbox, ID, get_water_type(reflectiv), texture_id, dynamic);
}

static __inline__ unsigned int dynamic_aabb_is_in_node(BBOX_TREE_NODE *node, unsigned int ID, unsigned int type, unsigned int *index)
{
	unsigned int i;

	for (i = 0; i < node->dynamic_objects.index; i++)
	{
		if ((node->dynamic_objects.items[i].data.ID == ID) && (node->dynamic_objects.items[i].data.type == type))
		{
			*index = i;
			return 1;
		}
	}

	for (i = 0; i < node->dynamic_objects.sub_index; i++)
	{
		if ((node->dynamic_objects.sub_items[i].ID == ID) && (node->dynamic_objects.sub_items[i].type == type))
		{
			*index = i;
			return 2;
		}
	}

	return 0;
}

static __inline__ unsigned int delete_dynamic_aabb_from_node(BBOX_TREE *bbox_tree, BBOX_TREE_NODE *node, unsigned int ID, unsigned int type)
{
#ifdef	FRUSTUM_THREADS
	unsigned int ret, idx;
#else
	unsigned int i, ret, idx, index, size;
	AABBOX new_bbox;
#endif

	if (node != NULL)
	{		
		ret = dynamic_aabb_is_in_node(node, ID, type, &idx);
		if (ret != 0)
		{
			if (ret == 1)
			{
#ifdef	FRUSTUM_THREADS
				node->dynamic_objects.items[idx].data.type = TYPE_DELETED;
#else
				index = node->dynamic_objects.index;
				size = node->dynamic_objects.size;
				if (index <= 1)
				{
					size = 0;
					index = 0;
					if (node->dynamic_objects.items != NULL) 
					{
						free(node->dynamic_objects.items);
						node->dynamic_objects.items = NULL;
					}
				}
				else
				{
					if (index < (size/2))
					{
						size /= 2;
						node->dynamic_objects.items = (BBOX_ITEM*)realloc(node->dynamic_objects.items, 
								size*sizeof(BBOX_ITEM));
					}

					memmove(&node->dynamic_objects.items[idx], &node->dynamic_objects.items[idx+1], 
							(index-idx-1)*sizeof(BBOX_ITEM));
					index--;
				}
				node->dynamic_objects.index = index;
				node->dynamic_objects.size = size;
#endif
			}
			else
			{
				if (ret == 2)
				{			
#ifdef	FRUSTUM_THREADS
					node->dynamic_objects.sub_items[idx].type = TYPE_DELETED;
#else
					index = node->dynamic_objects.sub_index;
					size = node->dynamic_objects.sub_size;
#endif
					delete_dynamic_aabb_from_node(bbox_tree, node->nodes[0], ID, type);
					delete_dynamic_aabb_from_node(bbox_tree, node->nodes[1], ID, type);			
#ifndef	FRUSTUM_THREADS
					if (index <= 1)
					{
						size = 0;
						index = 0;
						if (node->dynamic_objects.sub_items != NULL) 
						{
							free(node->dynamic_objects.sub_items);
							node->dynamic_objects.sub_items = NULL;
						}
					}
					else
					{
						if (index < (size/2))
						{
							size /= 2;
							node->dynamic_objects.sub_items = 
								(BBOX_ITEM_DATA*)realloc(node->dynamic_objects.sub_items, 
									size*sizeof(BBOX_ITEM_DATA));
						}
						
						memmove(&node->dynamic_objects.sub_items[idx], 
								&node->dynamic_objects.sub_items[idx+1], 
								(index-idx-1)*sizeof(BBOX_ITEM_DATA));
						index--;
					}
					node->dynamic_objects.sub_index = index;
					node->dynamic_objects.sub_size = size;
#endif
				}
			}

#ifndef	FRUSTUM_THREADS
			if ((node->nodes[0] != NULL) && (node->nodes[1] != NULL))
			{
				VMin(new_bbox.bbmin, node->nodes[0]->bbox.bbmin, node->nodes[1]->bbox.bbmin);
				VMax(new_bbox.bbmax, node->nodes[0]->bbox.bbmax, node->nodes[1]->bbox.bbmax);
			}
			else
			{
				VFill(new_bbox.bbmin, BOUND_HUGE);
				VFill(new_bbox.bbmax, -BOUND_HUGE);
			
				for (i = 0; i < node->items_count; i++)
				{
					VMin(new_bbox.bbmin, new_bbox.bbmin, bbox_tree->items[node->items_index+i].bbox.bbmin);
					VMax(new_bbox.bbmax, new_bbox.bbmax, bbox_tree->items[node->items_index+i].bbox.bbmax);
				}
			}

			for (i = 0; i < node->dynamic_objects.index; i++)
			{
				VMin(new_bbox.bbmin, new_bbox.bbmin, node->dynamic_objects.items[i].bbox.bbmin);
				VMax(new_bbox.bbmax, new_bbox.bbmax, node->dynamic_objects.items[i].bbox.bbmax);
			}

			memcpy(node->bbox.bbmin, new_bbox.bbmin, sizeof(VECTOR3));
			memcpy(node->bbox.bbmax, new_bbox.bbmax, sizeof(VECTOR3));
#endif
			return 1;
		}
	}
	return 0;
}

static __inline__ void delete_aabb_from_abt(BBOX_TREE *bbox_tree, unsigned int ID, unsigned int type)
{
#ifdef	FRUSTUM_THREADS
	unsigned int i;
#endif

	if (bbox_tree != NULL)
	{
		lock_bbox_tree(bbox_tree);
#ifdef	FRUSTUM_THREADS
		for (i = 0; i < bbox_tree->update_data.index; i++)
		{
			if ((bbox_tree->update_data.list[i].item.data.ID == ID) && 
					(bbox_tree->update_data.list[i].item.data.type == type))
			{
				bbox_tree->update_data.list[i].item.data.type = TYPE_DELETED;
			}
		}
		for (i = 0; i < bbox_tree->items_count; i++)
		{
			if ((bbox_tree->items[i].data.ID == ID) && 
					(bbox_tree->items[i].data.type == type))
			{
				bbox_tree->items[i].data.type = TYPE_DELETED;
			}
		}
#endif
		delete_item_from_intersect_list(bbox_tree, ID, type);
		delete_dynamic_aabb_from_node(bbox_tree, bbox_tree->root_node, ID, type);
#ifdef	FRUSTUM_THREADS
		update_bbox_tree_degeneration(bbox_tree, 1);
#endif
		unlock_bbox_tree(bbox_tree);
	}
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
		free(bbox_items);
	}
}

#ifdef	FRUSTUM_THREADS
static __inline__ void resize_bbox_items(BBOX_ITEMS* bbox_items)
{
	unsigned int i;
	
	if (bbox_items != NULL)
	{
		qsort(bbox_items->items, bbox_items->index, sizeof(BBOX_ITEM), comp_items);
		for (i = 0; i < bbox_items->index; i++)
		{
			if (bbox_items->items[i].data.type == TYPE_DELETED)
			{
				bbox_items->size = i;
				bbox_items->index = i;
				bbox_items->items = (BBOX_ITEM*)realloc(bbox_items->items, i*sizeof(BBOX_ITEM));
				break;
			}
		}
	}
}

int update_bbox_tree(void *thread_args)
{
	BBOX_TREE *bbox_tree;
	BBOX_ITEMS* new_bbox_items;
	BBOX_TREE new_bbox_tree;
	BBOX_TREE_UPDATE_DATA update_data;
	unsigned int i, index, ID, type, sort_data;
	
	bbox_tree = thread_args;

	// Copy the data of the old bbox_tree
	while (!bbox_tree->done)
	{
		lock_bbox_tree(bbox_tree);
		SDL_CondWait(bbox_tree->update_condition, bbox_tree->bbox_tree_mutex);
	
		new_bbox_items = create_bbox_items(bbox_tree->items_count);
		memcpy(new_bbox_items->items, bbox_tree->items, bbox_tree->items_count*sizeof(BBOX_ITEM));
		new_bbox_items->index = bbox_tree->items_count;
		
		update_data.bbox_tree_degeneration = bbox_tree->update_data.bbox_tree_degeneration;
		update_data.size = bbox_tree->update_data.size;
		update_data.index = bbox_tree->update_data.index;
		update_data.list = (BBOX_UPDATE_ITEM_DATA*)malloc(update_data.index*sizeof(BBOX_UPDATE_ITEM_DATA));
		memcpy(update_data.list, bbox_tree->update_data.list, update_data.index*sizeof(BBOX_UPDATE_ITEM_DATA));
		bbox_tree->update_data.bbox_tree_degeneration = 0;

		unlock_bbox_tree(bbox_tree);

		// Copy done, start building the new bbox_tree
		index = update_data.index;
	
		for (i = 0; i < index; i++)
		{
			if (!update_data.list[i].dynamic)
			{
				type = update_data.list[i].item.data.type;
				if (type != TYPE_DELETED)
				{
					ID = update_data.list[i].item.data.ID;
					sort_data = update_data.list[i].item.data.sort_data;
					add_aabb_to_list(new_bbox_items, &update_data.list[i].item.bbox, ID, type, sort_data);
				}
			}
		}
		resize_bbox_items(new_bbox_items);
		memset(&new_bbox_tree, 0, sizeof(new_bbox_tree));
		init_bbox_tree(&new_bbox_tree, new_bbox_items);
		for (i = 0; i < index; i++)
		{
			if (update_data.list[i].dynamic)
			{
				type = update_data.list[i].item.data.type;
				if (type != TYPE_DELETED)
				{
					ID = update_data.list[i].item.data.ID;
					sort_data = update_data.list[i].item.data.sort_data;
					add_dynamic_aabb_to_abt_node(new_bbox_tree.root_node, &update_data.list[i].item.bbox, ID, type, sort_data);
				}
			}
		}

		// Now replace the old bbox_tree with the new one
		lock_bbox_tree(bbox_tree);
	
		// Free old data
		free_bbox_tree_data(bbox_tree);
	
		// Copy new data
		bbox_tree->root_node = new_bbox_tree.root_node;
		bbox_tree->items_count = new_bbox_tree.items_count;
		bbox_tree->items = new_bbox_tree.items;
		bbox_tree->nodes_count = new_bbox_tree.nodes_count;
		bbox_tree->nodes = new_bbox_tree.nodes;

		// Update the object list
		bbox_tree->update_data.index = bbox_tree->update_data.index - index;
		memmove(bbox_tree->update_data.list, &bbox_tree->update_data.list[index], index*sizeof(BBOX_UPDATE_ITEM_DATA));
		unlock_bbox_tree(bbox_tree);
	}

	return 1;
}
#endif

BBOX_TREE* build_bbox_tree()
{
	BBOX_TREE* bbox_tree;
	unsigned int i, type_ids;
	
	type_ids = 0;

	bbox_tree = (BBOX_TREE*)malloc(sizeof(BBOX_TREE));
	
	bbox_tree->cur_intersect_type = ITERSECTION_TYPE_DEFAULT;
	for (i = 0; i < MAX_ITERSECTION_TYPES; i++)
	{
		bbox_tree->intersect[i].size = 8;
		bbox_tree->intersect[i].count = 0;
		bbox_tree->intersect[i].items = (BBOX_ITEM_DATA*)malloc(8*sizeof(BBOX_ITEM_DATA));
	}
	bbox_tree->nodes_count = 0;
	bbox_tree->nodes = NULL;
	bbox_tree->root_node = NULL;
	bbox_tree->items_count = 0;
	bbox_tree->items = NULL;
	bbox_tree->bbox_tree_mutex = SDL_CreateMutex();
#ifdef	FRUSTUM_THREADS
	bbox_tree->done = 0;
	bbox_tree->update_condition = SDL_CreateCond();
	bbox_tree->thread_id = SDL_CreateThread(update_bbox_tree, bbox_tree);
	bbox_tree->update_data.bbox_tree_degeneration = 0;
	bbox_tree->update_data.size = 0;
	bbox_tree->update_data.index = 0;
	bbox_tree->update_data.list = NULL;
#endif
	return bbox_tree;
}

#if	0
static __inline__ void save_nodes(BBOX_TREE_NODE* node, unsigned long base, FILE file)
{
	unsigned long index;
	unsigned int idx1, idx2, data;
	
	if (node != NULL)
	{
		if (node->nodes[0] != NULL)
		{
			index = node->nodes[0];
			idx1 = (index - base)/sizeof(BBOX_TREE_NODE);
		}
		else idx1 = 0xFFFFFFFF;

		if (node->nodes[1] != NULL)
		{
			index = node->nodes[1];
			idx2 = (index - base)/sizeof(BBOX_TREE_NODE);
		}
		else idx2 = 0xFFFFFFFF;
	
		data = SDL_SwapLE32(idx1);
		fwrite(&data, 1, sizeof(unsigned int), f);
		data = SDL_SwapLE32(idx2);
		fwrite(&data, 1, sizeof(unsigned int), f);
		data = SDL_SwapLE32(node->items_index);
		fwrite(&data, 1, sizeof(unsigned int), f);
		data = SDL_SwapLE32(node->items_count);
		fwrite(&data, 1, sizeof(unsigned int), f);

		save_nodes(node->nodes[0], base, f);
		save_nodes(node->nodes[1], base, f);
	}
}

void save_bbox_tree(BBOX_TREE* bbox_tree, char* file_name)
{
	unsigned int i, size, data;
	float data_f;

	fopen("wb", file_name, f);

	size = SDL_SwapLE32(bbox_tree->items_count);
	fwrite(&size, 1, sizeof(unsigned int), f);
	for (i = 0; i < size; i++)
	{
		data_f = SDL_SwapLEFloat(bbox_tree->items[i].bbox[0][0]);
		fwrite(&data_f, 1, sizeof(float), f);
		data_f = SDL_SwapLEFloat(bbox_tree->items[i].bbox[0][1]);
		fwrite(&data_f, 1, sizeof(float), f);
		data_f = SDL_SwapLEFloat(bbox_tree->items[i].bbox[0][2]);
		fwrite(&data_f, 1, sizeof(float), f);
		data_f = SDL_SwapLEFloat(bbox_tree->items[i].bbox[1][0]);
		fwrite(&data_f, 1, sizeof(float), f);
		data_f = SDL_SwapLEFloat(bbox_tree->items[i].bbox[1][1]);
		fwrite(&data_f, 1, sizeof(float), f);
		data_f = SDL_SwapLEFloat(bbox_tree->items[i].bbox[1][2]);
		fwrite(&data_f, 1, sizeof(float), f);
		data = SDL_SwapLE32(bbox_tree->items[i].type);
		fwrite(&data, 1, sizeof(unsigned int), f);
		data = SDL_SwapLE32(bbox_tree->items[i].ID);
		fwrite(&data, 1, sizeof(unsigned int), f);
	}
	
	size = SDL_SwapLE32(bbox_tree->nodes_count);
	fwrite(&size, 1, sizeof(unsigned int), f);
	save_nodes(bbox_tree->root_node, f);

	fclose(f);
}

static __inline__ void load_nodes(BBOX_TREE* bbox_tree, BBOX_TREE_NODE* node, unsigned long base, FILE* file)
{
	unsigned int idx1, idx2, data;
	
	if (node != NULL)
	{
		fread(&data, 1, sizeof(unsigned int), file);
		idx1 = SDL_SwapLE32(data);
		fread(&data, 1, sizeof(unsigned int), file);
		idx2 = SDL_SwapLE32(data);

		fread(&data, 1, sizeof(unsigned int), file);
		node->items_index = SDL_SwapLE32(data);
		fread(&data, 1, sizeof(unsigned int), file);
		node->items_count = SDL_SwapLE32(data);

		if (idx1 != 0xFFFFFFFF) node->nodes[0] = bbox_tree->nodes[idx1];
		if (idx2 != 0xFFFFFFFF) node->nodes[1] = bbox_tree->nodes[idx2];
	}
}

BBOX_TREE* load_bbox_tree(char* file_name)
{
	BBOX_TREE* bbox_tree;
	unsigned int i, size, data;
	float data_f;

	fopen("rb", file_name, f);

	bbox_tree = (BBOX_TREE*)malloc(sizeof(BBOX_TREE));
	fread(&data, 1, sizeof(unsigned int), f);
	size = SDL_SwapLE32(data);
	bbox_tree->items_count = size;
	bbox_tree->items = (BBOX_ITEM*)malloc(size*sizeof(BBOX_ITEM));
	bbox_tree->intersect_count = 0;
	bbox_tree->intersect_items = (BBOX_ITEM_DATA*)malloc(size*sizeof(BBOX_ITEM_DATA));
	for (i = 0; i < size; i++)
	{
		fread(&data_f, 1, sizeof(float), f);
		bbox_tree->items[i].bbox[0][0] = SDL_SwapLEFloat(data_f);
		fread(&data_f, 1, sizeof(float), f);
		bbox_tree->items[i].bbox[0][1] = SDL_SwapLEFloat(data_f);
		fread(&data_f, 1, sizeof(float), f);
		bbox_tree->items[i].bbox[0][2] = SDL_SwapLEFloat(data_f);
		fread(&data_f, 1, sizeof(float), f);
		bbox_tree->items[i].bbox[1][0] = SDL_SwapLEFloat(data_f);
		fread(&data_f, 1, sizeof(float), f);
		bbox_tree->items[i].bbox[1][1] = SDL_SwapLEFloat(data_f);
		fread(&data_f, 1, sizeof(float), f);
		bbox_tree->items[i].bbox[1][2] = SDL_SwapLEFloat(data_f);
		fread(&data, 1, sizeof(unsigned int), f);
		bbox_tree->items[i].type = SDL_SwapLE32(data);
		fread(&data, 1, sizeof(unsigned int), f);
		bbox_tree->items[i].ID = SDL_SwapLE32(data);
	}
	fread(&data, 1, sizeof(unsigned int), f);
	size = SDL_SwapLE32(data);
	bbox_tree->nodes_count = size;
	bbox_tree->nodes = (BBOX_TREE_NODE*)malloc(size*sizeof(BBOX_TREE_NODE));
	load_nodes(bbox_tree, bbox_tree->root_node, f);

	fclose(f);
}
#endif
#endif
