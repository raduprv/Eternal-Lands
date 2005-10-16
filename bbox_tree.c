#ifdef	NEW_FRUSTUM
#include "global.h"
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include "bbox_tree.h"

static __inline__ void add_intersect_item(BBOX_TREE* bbox_tree, unsigned int index)
{

	memcpy(&bbox_tree->intersect_items[bbox_tree->intersect_index], &bbox_tree->items[index].data, sizeof(BBOX_ITEM_DATA));
	bbox_tree->intersect_index++;
}

static __inline__ void add_intersect_items(BBOX_TREE* bbox_tree, unsigned int index, unsigned int count)
{
	int i;
	for (i = 0; i < count; i++) add_intersect_item(bbox_tree, index+i);		
}

static __inline__ void add_dyn_intersect_item(BBOX_TREE* bbox_tree, BBOX_TREE_NODE* node, unsigned int index)
{
	memcpy(&bbox_tree->intersect_items[bbox_tree->intersect_index], &node->dynamic_objects.items[index].data, sizeof(BBOX_ITEM_DATA));
	bbox_tree->intersect_index++;
}

static __inline__ void add_dyn_intersect_items(BBOX_TREE* bbox_tree, BBOX_TREE_NODE* node, unsigned int count)
{
	int i;
	for (i = 0; i < count; i++) add_dyn_intersect_item(bbox_tree, node, i);		
}

static __inline__ void add_dyn_intersect_data(BBOX_TREE* bbox_tree, BBOX_TREE_NODE* node, unsigned int index)
{
	memcpy(&bbox_tree->intersect_items[bbox_tree->intersect_index], &node->dynamic_objects.sub_items[index], sizeof(BBOX_ITEM_DATA));
	bbox_tree->intersect_index++;
}

static __inline__ void add_dyn_intersect_datas(BBOX_TREE* bbox_tree, BBOX_TREE_NODE* node, unsigned int count)
{
	int i;
	for (i = 0; i < count; i++) add_dyn_intersect_data(bbox_tree, node, i);		
}

static __inline__ int check_aabb_in_frustum(AABBOX *bbox, FRUSTUM *frustum, unsigned int in_mask, unsigned int *out_mask)
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

			nx = bbox->bbmin[X+frustum[0][i].mask[0]*3];
			ny = bbox->bbmin[Y+frustum[0][i].mask[1]*3];
			nz = bbox->bbmin[Z+frustum[0][i].mask[2]*3];
			px = bbox->bbmin[X+frustum[0][i].mask[4]*3];
			py = bbox->bbmin[Y+frustum[0][i].mask[5]*3];
			pz = bbox->bbmin[Z+frustum[0][i].mask[6]*3];
			m = (	frustum[0][i].plane[A] * nx + 
				frustum[0][i].plane[B] * ny + 
				frustum[0][i].plane[C] * nz);
			if (m < -frustum[0][i].plane[D]) return OUTSIDE;
			
			n = (	frustum[0][i].plane[A] * px + 
				frustum[0][i].plane[B] * py + 
				frustum[0][i].plane[C] * pz);
			if (n < -frustum[0][i].plane[D]) 
			{
				*out_mask |= k;
				result = INTERSECT;
			}
		}
	}

	return result;
}

static __inline__ void add_items(BBOX_TREE* bbox_tree, BBOX_TREE_NODE* sub_node, FRUSTUM *frustum, unsigned int in_mask)
{
	unsigned int idx2, size, i, result, out_mask;

	idx2 = sub_node->items_index;
	size = sub_node->items_count;
		
	for (i = 0; i < size; i++)
	{
		result = check_aabb_in_frustum(&bbox_tree->items[idx2+i].bbox, frustum, in_mask, &out_mask);
		if (result != OUTSIDE) add_intersect_item(bbox_tree, idx2+i);
	}
}

static __inline__ void add_dyn_items(BBOX_TREE* bbox_tree, BBOX_TREE_NODE* sub_node, FRUSTUM *frustum, unsigned int in_mask)
{
	unsigned int size, i, result, out_mask;

	size = sub_node->dynamic_objects.index;
		
	for (i = 0; i < size; i++)
	{
		result = check_aabb_in_frustum(&sub_node->dynamic_objects.items[i].bbox, frustum, in_mask, &out_mask);
		if (result != OUTSIDE) 
			add_dyn_intersect_item(bbox_tree, sub_node, i);
	}
}

static __inline__ void check_sub_nodes(BBOX_TREE* bbox_tree, BBOX_TREE_NODE* sub_node, FRUSTUM *frustum, unsigned int in_mask)
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

void check_bbox_tree(BBOX_TREE* bbox_tree, FRUSTUM *frustum)
{	
	if (bbox_tree != NULL)
	{
		bbox_tree->intersect_index = 0;
		check_sub_nodes(bbox_tree, bbox_tree->root_node, frustum, 63);
	}	
}

void free_bbox_tree(BBOX_TREE* bbox_tree)
{
	if (bbox_tree != NULL)
	{
		if (bbox_tree->items != NULL) free(bbox_tree->items);
		if (bbox_tree->nodes != NULL) free(bbox_tree->nodes);
		if (bbox_tree->intersect_items != NULL) free(bbox_tree->intersect_items);
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

	Make_Vector3(bmin, BOUND_HUGE);
	Make_Vector3(bmax, -BOUND_HUGE);

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

	Make_Vector3(bmin, BOUND_HUGE);
	Make_Vector3(bmax, -BOUND_HUGE);

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

	Make_Vector3(bmin, BOUND_HUGE);
	Make_Vector3(bmax, -BOUND_HUGE);

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

BBOX_TREE* build_bbox_tree(BBOX_ITEMS *bbox_items)
{
	BBOX_TREE* bbox_tree;
	unsigned int size, type_ids, index;
	
	type_ids = 0;
	size = bbox_items->index;
	
	if (size <= 0) return NULL;
	
	bbox_tree = (BBOX_TREE*)malloc(sizeof(BBOX_TREE));
	index = 1;
	bbox_tree->nodes_count = 2*size;
	bbox_tree->nodes = (BBOX_TREE_NODE*)malloc(size*2*sizeof(BBOX_TREE_NODE));
	bbox_tree->root_node = &bbox_tree->nodes[0];
	bbox_tree->items_count = size;
	bbox_tree->items = (BBOX_ITEM*)malloc(size*sizeof(BBOX_ITEM));
	memcpy(bbox_tree->items, bbox_items->items, size*sizeof(BBOX_ITEM));
	sort_and_split(bbox_tree, bbox_tree->root_node, &index, 0, size);
	bbox_tree->intersect_index = 0;
	bbox_tree->intersect_items = (BBOX_ITEM_DATA*)malloc(size*sizeof(BBOX_ITEM_DATA));
	bbox_tree->nodes_count = index;
	bbox_tree->nodes = (BBOX_TREE_NODE*)realloc(bbox_tree->nodes, index*sizeof(BBOX_TREE_NODE));

	return bbox_tree;
}

static __inline__ void add_aabb_to_abt(BBOX_ITEMS *bbox_items, AABBOX *bbox, unsigned int ID, unsigned int type)
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
	bbox_items->items[index].data.ID = ID;
	bbox_items->index = index + 1;
}

void add_light_to_abt(BBOX_ITEMS *bbox_items, unsigned int ID, AABBOX *bbox)
{
	add_aabb_to_abt(bbox_items, bbox, ID, TYPE_LIGHT);
}

void add_3dobject_to_abt(BBOX_ITEMS *bbox_items, unsigned int ID, AABBOX *bbox)
{
	add_aabb_to_abt(bbox_items, bbox, ID, TYPE_3D_OBJECT);
}

void add_2dobject_to_abt(BBOX_ITEMS *bbox_items, unsigned int ID, AABBOX *bbox)
{
	add_aabb_to_abt(bbox_items, bbox, ID, TYPE_2D_OBJECT);
}

void add_particle_to_abt(BBOX_ITEMS *bbox_items, unsigned int ID, AABBOX *bbox)
{
	add_aabb_to_abt(bbox_items, bbox, ID, TYPE_PARTICLE);
}

static __inline__ int check_aabb_aabb(AABBOX *bbox, AABBOX *dyn_bbox, AABBOX *new_bbox, float grow)
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

static __inline__ void add_dynamic_item_to_node(BBOX_TREE_NODE *node, AABBOX *bbox, unsigned int ID, unsigned int type)
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
		node->dynamic_objects.items[index].data.type = type;
		memcpy(node->dynamic_objects.items[index].bbox.bbmin, bbox->bbmin, sizeof(VECTOR3));
		memcpy(node->dynamic_objects.items[index].bbox.bbmax, bbox->bbmax, sizeof(VECTOR3));
		node->dynamic_objects.index = index + 1;
		VMin(node->bbox.bbmin, node->bbox.bbmin, bbox->bbmin);
		VMax(node->bbox.bbmax, node->bbox.bbmax, bbox->bbmax);
	}
}

static __inline__ void add_dynamic_data_to_node(BBOX_TREE_NODE *node, AABBOX *bbox, unsigned int ID, unsigned int type)
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
		node->dynamic_objects.sub_items[index].type = type;
		node->dynamic_objects.sub_index = index + 1;
		VMin(node->bbox.bbmin, node->bbox.bbmin, bbox->bbmin);
		VMax(node->bbox.bbmax, node->bbox.bbmax, bbox->bbmax);
	}
}

static __inline__ int add_dynamic_aabb_to_abt(BBOX_TREE_NODE *node, AABBOX *bbox, unsigned int ID, unsigned int type)
{
	AABBOX new_bbox;
	int r;
	
	if (node != NULL)
	{
		if (check_aabb_aabb(&node->bbox, bbox, &new_bbox, 1.1f))
		{
			r = add_dynamic_aabb_to_abt(node->nodes[0], bbox, ID, type);
			if (r == 0) r = add_dynamic_aabb_to_abt(node->nodes[1], bbox, ID, type);
			if (r == 0) add_dynamic_item_to_node(node, bbox, ID, type);
			else add_dynamic_data_to_node(node, bbox, ID, type);
			return 1;
		}
		else return 0;
	}
	else return 0;
}

void add_dynamic_light_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, AABBOX *bbox)
{
	add_dynamic_aabb_to_abt(bbox_tree->root_node, bbox, ID, TYPE_LIGHT);
}

void add_dynamic_3dobject_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, AABBOX *bbox)
{
	add_dynamic_aabb_to_abt(bbox_tree->root_node, bbox, ID, TYPE_3D_OBJECT);
}

void add_dynamic_particle_to_abt(BBOX_TREE *bbox_tree, unsigned int ID, AABBOX *bbox)
{
	add_dynamic_aabb_to_abt(bbox_tree->root_node, bbox, ID, TYPE_PARTICLE);
}

static __inline__ int dynamic_aabb_is_in_node(BBOX_TREE_NODE *node, unsigned int ID, unsigned int type, unsigned int *index)
{
	int i;
	
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

static __inline__ void delete_dynamic_aabb_from_node(BBOX_TREE *bbox_tree, BBOX_TREE_NODE *node, unsigned int ID, unsigned int type)
{
	unsigned int i, ret, idx, index, size;
	AABBOX new_bbox;

	if (node != NULL)
	{		
		ret = dynamic_aabb_is_in_node(node, ID, type, &idx);
		
		if (ret != 0)
		{
			if (ret == 1)
			{			
				index = node->dynamic_objects.index;
				size = node->dynamic_objects.size;
				if (index <= 1)
				{
					size = 0;
					index = 0;
					if (node->dynamic_objects.items != NULL) free(node->dynamic_objects.items);
					node->dynamic_objects.items = NULL;
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
			}
			else
			{
				if (ret == 2)
				{			
					index = node->dynamic_objects.sub_index;
					size = node->dynamic_objects.sub_size;
					delete_dynamic_aabb_from_node(bbox_tree, node->nodes[0], ID, type);
					delete_dynamic_aabb_from_node(bbox_tree, node->nodes[1], ID, type);
					
					if (index <= 1)
					{
						size = 0;
						index = 0;
						if (node->dynamic_objects.sub_items != NULL) 
							free(node->dynamic_objects.sub_items);
						node->dynamic_objects.sub_items = NULL;
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
					node->dynamic_objects.index = index;
					node->dynamic_objects.size = size;
				}
			}
		
			if ((node->nodes[0] != NULL) && (node->nodes[1] != NULL))
			{
				VMin(new_bbox.bbmin, node->nodes[0]->bbox.bbmin, node->nodes[1]->bbox.bbmin);
				VMax(new_bbox.bbmax, node->nodes[0]->bbox.bbmax, node->nodes[1]->bbox.bbmax);
			}
			else
			{
				Make_Vector3(new_bbox.bbmin, BOUND_HUGE);
				Make_Vector3(new_bbox.bbmax, -BOUND_HUGE);
			
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
		}
	}
}

void delete_dynamic_3dobject_from_abt(BBOX_TREE *bbox_tree, unsigned int ID)
{
//	delete_dynamic_aabb_from_node(bbox_tree, bbox_tree->root_node, ID, TYPE_3D_OBJECT);
}

void delete_dynamic_particle_from_abt(BBOX_TREE *bbox_tree, unsigned int ID)
{
//	delete_dynamic_aabb_from_node(bbox_tree, bbox_tree->root_node, ID, TYPE_PARTICLE);
}

void delete_dynamic_light_from_abt(BBOX_TREE *bbox_tree, unsigned int ID)
{
//	delete_dynamic_aabb_from_node(bbox_tree, bbox_tree->root_node, ID, TYPE_LIGHT);
}

BBOX_ITEMS* create_bbox_items(unsigned int size)
{
	BBOX_ITEMS* bbox_items;
	
	bbox_items = (BBOX_ITEMS*)malloc(sizeof(bbox_items));
	size = max(8, size);
	bbox_items->size = size;
	bbox_items->index = 0;
	bbox_items->items = (BBOX_ITEM*)malloc(size*sizeof(BBOX_ITEM));

	return bbox_items;
}

void free_bbox_items(BBOX_ITEMS* bbox_items)
{
	if (bbox_items != NULL)
	{
		if (bbox_items->items != NULL) free(bbox_items->items);
		free(bbox_items);
	}
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
	bbox_tree->intersect_index = 0;
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
