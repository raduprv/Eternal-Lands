#ifdef	NEW_FRUSTUM
#include "global.h"
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include "bbox_tree.h"

static __inline__ void add_intersect_item(BBOX_TREE* bbox_tree, unsigned int index)
{
	memcpy(&bbox_tree->intersect_items[bbox_tree->intersect_index], &bbox_tree->items[index], sizeof(BBOX_ITEM));
	bbox_tree->intersect_index++;
}

static __inline__ int check_aabb_in_frustum(AABBOX *bbox, FRUSTUM *frustum, unsigned int in_mask, unsigned int *out_mask)
{
	float m, n;
	unsigned int i, k, result;
	
	result = INSIDE;
	*out_mask = 63;
	
	for (i = 0, k = 1; k <= in_mask; i++, k += k)
	{
		if (k & in_mask)
		{
			float nx, px, ny, py, nz, pz;

			nx = bbox->bbmin[X+frustum[0][i].mask[0]*4];
			ny = bbox->bbmin[Y+frustum[0][i].mask[1]*4];
			nz = bbox->bbmin[Z+frustum[0][i].mask[2]*4];
			px = bbox->bbmin[X+frustum[0][i].mask[4]*4];
			py = bbox->bbmin[Y+frustum[0][i].mask[5]*4];
			pz = bbox->bbmin[Z+frustum[0][i].mask[6]*4];
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
	unsigned int idx1, idx2, size, i, result, out_mask;

	idx1 = bbox_tree->intersect_index;
	idx2 = sub_node->items_index;
	size = sub_node->items_count;
		
	for (i = 0; i < size; i++)
	{
		result = check_aabb_in_frustum(&bbox_tree->items[idx2+i].bbox, frustum, in_mask, &out_mask);
		if (result != OUTSIDE) add_intersect_item(bbox_tree, idx2+i);
	}
}

static __inline__ void check_sub_nodes(BBOX_TREE* bbox_tree, BBOX_TREE_NODE* sub_node, FRUSTUM *frustum, unsigned int in_mask)
{
	unsigned int out_mask, result;
	
	if (sub_node != NULL)
	{
		result = check_aabb_in_frustum(&sub_node->bbox, frustum, in_mask, &out_mask);
		if ((result == INSIDE) || ((result == INTERSECT) && (sub_node->nodes[0] == NULL) && (sub_node->nodes[1] == NULL)))
		{
			add_items(bbox_tree, sub_node, frustum, out_mask);
		}
		else
		{
			if (result == INTERSECT)
			{
				check_sub_nodes(bbox_tree, sub_node->nodes[0], frustum, out_mask);
				check_sub_nodes(bbox_tree, sub_node->nodes[1], frustum, out_mask);
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
	VECTOR4 bmin, bmax, len;

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

	Make_Vector4(bmin, BOUND_HUGE, BOUND_HUGE, BOUND_HUGE, BOUND_HUGE);
	Make_Vector4(bmax, -BOUND_HUGE, -BOUND_HUGE, -BOUND_HUGE, -BOUND_HUGE);

	for (i = a; i != (b + dir); i += dir)
	{
		VMin4(bmin, bmin, bbox_tree->items[i].bbox.bbmin);
		VMax4(bmax, bmax, bbox_tree->items[i].bbox.bbmax);
		VSub4(len, bmax, bmin);

		areas[i - imin] = len[X] * len[Y] * len[Z];
	}
}

static __inline__ void find_axis(BBOX_TREE *bbox_tree, unsigned int first, unsigned int last, unsigned int *ret)
{
	unsigned int i, a1, a2, a3;
	VECTOR4 bmin, bmax;
	float d, e;

	Make_Vector4(bmin, BOUND_HUGE, BOUND_HUGE, BOUND_HUGE, 0.0f);
	Make_Vector4(bmax, -BOUND_HUGE, -BOUND_HUGE, -BOUND_HUGE, 0.0f);

	for (i = first; i < last; i++)
	{
		VMin4(bmin, bmin, bbox_tree->items[i].bbox.bbmin);
		VMax4(bmax, bmax, bbox_tree->items[i].bbox.bbmax);
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
	VECTOR4 bmin, bmax;

	Make_Vector4(bmin, BOUND_HUGE, BOUND_HUGE, BOUND_HUGE, 0.0f);
	Make_Vector4(bmax, -BOUND_HUGE, -BOUND_HUGE, -BOUND_HUGE, 0.0f);

	for (i = first; i < last; i++)
	{
		VMin4(bmin, bmin, bbox_tree->items[i].bbox.bbmin);
		VMax4(bmax, bmax, bbox_tree->items[i].bbox.bbmax);
	}

	memcpy(bbox->bbmin, bmin, sizeof(VECTOR4));
	memcpy(bbox->bbmax, bmax, sizeof(VECTOR4));
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
	bbox_tree->intersect_items = (BBOX_ITEM*)malloc(size*sizeof(BBOX_ITEM));
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
		size = size * 2;
		bbox_items->items = (BBOX_ITEM*)realloc(bbox_items->items, size*sizeof(BBOX_ITEM));
		bbox_items->size = size;
	}
	
	VAssign4(bbox_items->items[index].bbox.bbmin, bbox->bbmin, 0.0f);
	VAssign4(bbox_items->items[index].bbox.bbmax, bbox->bbmax, 0.0f);
	bbox_items->items[index].type = type;
	bbox_items->items[index].ID = ID;
	bbox_items->index = index + 1;
}

void add_light_to_abt(BBOX_ITEMS *bbox_items, unsigned int ID, float pos_x, float pos_y, float pos_z, float diff_r, float diff_g, float diff_b, float att, int exp, float clamp)
{
	float h, r;
	AABBOX bbox;

	h = max(abs(diff_r), max(abs(diff_g), abs(diff_b)));

	r = (h/clamp-1)/att;

	switch (exp) 
	{
		case 1: 
			break;
		case 2: 
			r = sqrt(r);
			break;
		case 3:
			r = pow(r, 1.0f/3.0f);
			break;
		case 4:
			r = sqrt(sqrt(r));
			break;
		default:
			return;
	}
	
	bbox.bbmin[X] = pos_x - r;
	bbox.bbmin[Y] = pos_y - r;
	bbox.bbmin[Z] = pos_z - r;
	bbox.bbmax[X] = pos_x + r;
	bbox.bbmax[Y] = pos_y + r;
	bbox.bbmax[Z] = pos_z + r;
	
	add_aabb_to_abt(bbox_items, &bbox, ID, TYPE_LIGHT);
}

void add_3dobject_to_abt(BBOX_ITEMS *bbox_items, unsigned int ID, AABBOX *bbox)
{
	add_aabb_to_abt(bbox_items, bbox, ID, TYPE_3D_OBJECT);
}

void add_2dobject_to_abt(BBOX_ITEMS *bbox_items, unsigned int ID, AABBOX *bbox)
{
	add_aabb_to_abt(bbox_items, bbox, ID, TYPE_2D_OBJECT);
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
	bbox_tree->intersect_items = (BBOX_ITEM*)malloc(size*sizeof(BBOX_ITEM));
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
