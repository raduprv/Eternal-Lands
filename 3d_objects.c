#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <SDL.h>
#include "3d_objects.h"
#include "2d_objects.h"
#include "asc.h"
#include "cursors.h"
#include "draw_scene.h"
#include "e3d.h"
#include "errors.h"
#include "global.h"
#include "init.h"
#include "map.h"
#include "particles.h"
#include "platform.h"
#include "shadows.h"
#include "textures.h"
#include "tiles.h"
#include "translate.h"
#include "io/e3d_io.h"
#ifdef CLUSTER_INSIDES
#include "cluster.h"
#endif
#ifdef FSAA
#include "fsaa/fsaa.h"
#endif /* FSAA */

int use_3d_alpha_blend= 1;
#ifndef FASTER_MAP_LOAD
static int objects_list_placeholders = 0;
#endif
object3d *objects_list[MAX_OBJ_3D];

#include "eye_candy_wrapper.h"

e3d_object *load_e3d (const char *file_name);
void compute_clouds_map(object3d * object_id);
e3d_object *cur_e3d;
#ifdef  DEBUG
int cur_e3d_count;
int e3d_count, e3d_total;
#endif  //DEBUG

static int next_obj_3d = 0;

#ifdef FASTER_MAP_LOAD
void inc_objects_list_placeholders()
{
	next_obj_3d++;
}
#else
void clear_objects_list_placeholders()
{
	objects_list_placeholders = 0;
}

void inc_objects_list_placeholders()
{
	objects_list_placeholders++;
}
#endif

static __inline__ void build_clouds_planes(object3d* obj)
{
	float w, cos_w, sin_w;

	w = -obj->z_rot * M_PI / 180.0f;
	cos_w = cos(w);
	sin_w = sin(w);

	obj->clouds_planes[0][0] = cos_w / texture_scale;
	obj->clouds_planes[0][1] = sin_w / texture_scale;
	obj->clouds_planes[0][2] = 1.0f / texture_scale;
	obj->clouds_planes[0][3] = obj->x_pos / texture_scale;
	obj->clouds_planes[1][0] = -sin_w / texture_scale;
	obj->clouds_planes[1][1] = cos_w / texture_scale;
	obj->clouds_planes[1][2] = 1.0f / texture_scale;
	obj->clouds_planes[1][3] = obj->y_pos / texture_scale;
}

void disable_buffer_arrays()
{
	if (use_vertex_buffers)
	{
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		ELglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	}
	if (use_compiled_vertex_array && (cur_e3d != NULL))
	{
		ELglUnlockArraysEXT();
	}
	cur_e3d = NULL;
}

void draw_3d_object_detail(object3d * object_id, Uint32 material_index, Uint32 use_lightning,
	Uint32 use_textures, Uint32 use_extra_textures)
{
	e3d_vertex_data* vertex_layout;
	Uint8 * data_ptr;

	// check for having to load the arrays
	load_e3d_detail_if_needed(object_id->e3d_data);

	CHECK_GL_ERRORS();
	//also, update the last time this object was used
	object_id->last_acessed_time = cur_time;

	//debug

	if (object_id->self_lit && (!is_day || dungeon) && use_lightning) 
	{
		glColor3fv(object_id->color);
	}

	CHECK_GL_ERRORS();

	glPushMatrix();//we don't want to affect the rest of the scene

	glMultMatrixf(object_id->matrix);

	CHECK_GL_ERRORS();

	if (!dungeon && (clouds_shadows || use_shadow_mapping) && use_extra_textures)
	{
		VECTOR4 plane;

		ELglActiveTextureARB(detail_unit);
		memcpy(plane, object_id->clouds_planes[0], sizeof(VECTOR4));
		plane[3] += clouds_movement_u;
		glTexGenfv(GL_S, GL_EYE_PLANE, plane);
		memcpy(plane, object_id->clouds_planes[1], sizeof(VECTOR4));
		plane[3] += clouds_movement_v;
		glTexGenfv(GL_T, GL_EYE_PLANE, plane);
		ELglActiveTextureARB(base_unit);
	}

	// watch for a change
	if (object_id->e3d_data != cur_e3d)
	{
		if ((cur_e3d != NULL) && (use_compiled_vertex_array))
		{
			ELglUnlockArraysEXT();
		}

		if (use_vertex_buffers)
		{
			ELglBindBufferARB(GL_ARRAY_BUFFER_ARB,
				object_id->e3d_data->vertex_vbo);
			data_ptr = 0;
		}
		else
		{
			data_ptr = object_id->e3d_data->vertex_data;
		}

		vertex_layout = object_id->e3d_data->vertex_layout;

		if ((vertex_layout->normal_count > 0) && use_lightning)
		{
			glEnableClientState(GL_NORMAL_ARRAY);
			glNormalPointer(vertex_layout->normal_type, vertex_layout->size,
				data_ptr + vertex_layout->normal_offset);
		}
		else
		{
			glDisableClientState(GL_NORMAL_ARRAY);
			glNormal3f(0.0f, 0.0f, 1.0f);
		}

		if (use_textures)
		{
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(vertex_layout->texture_count, vertex_layout->texture_type,
				vertex_layout->size, data_ptr + vertex_layout->texture_offset);
		}
		else
		{
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}

		glVertexPointer(vertex_layout->position_count, vertex_layout->position_type,
			vertex_layout->size, data_ptr + vertex_layout->position_offset);

		if (use_vertex_buffers)
		{
			ELglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
				object_id->e3d_data->indices_vbo);
		}

		// lock this new one
		if (use_compiled_vertex_array)
		{
			ELglLockArraysEXT(0, object_id->e3d_data->vertex_no);
		}
		// gather statistics
		if (object_id->e3d_data != cur_e3d)
		{
#ifdef  DEBUG
			if ((cur_e3d_count > 0) && (cur_e3d != NULL))
			{
				e3d_count++;
				e3d_total += cur_e3d_count;
			}
			cur_e3d_count = 0;
#endif    //DEBUG
			cur_e3d = object_id->e3d_data;
		}
	}
#ifdef  DEBUG
	cur_e3d_count++;
#endif  //DEBUG

	if (use_textures)
	{
		glEnable(GL_TEXTURE_2D);
#ifdef	NEW_TEXTURES
		bind_texture(object_id->e3d_data->materials[material_index].texture);
#else	/* NEW_TEXTURES */
		get_and_set_texture_id(object_id->e3d_data->materials[material_index].texture);
#endif	/* NEW_TEXTURES */
	}
	else
	{
		glDisable(GL_TEXTURE_2D);		
	}


	if (use_draw_range_elements && ELglDrawRangeElementsEXT)
		ELglDrawRangeElementsEXT(GL_TRIANGLES,
			object_id->e3d_data->materials[material_index].triangles_indices_min,
			object_id->e3d_data->materials[material_index].triangles_indices_max,
			object_id->e3d_data->materials[material_index].triangles_indices_count,
			object_id->e3d_data->index_type,
			object_id->e3d_data->materials[material_index].triangles_indices_index);
	else
		glDrawElements(GL_TRIANGLES,
			object_id->e3d_data->materials[material_index].triangles_indices_count,
			object_id->e3d_data->index_type,
			object_id->e3d_data->materials[material_index].triangles_indices_index);

	glPopMatrix();//restore the scene
	CHECK_GL_ERRORS();

	//OK, let's check if our mouse is over...
#ifdef MAP_EDITOR2
	if (selected_3d_object == -1 && read_mouse_now && mouse_in_sphere(object_id->x_pos, object_id->y_pos, object_id->z_pos, object_id->e3d_data->radius))
		anything_under_the_mouse(object_id->id, UNDER_MOUSE_3D_OBJ);
#endif
}

void draw_3d_objects(unsigned int object_type)
{
	unsigned int    start, stop;
	unsigned int    i, l;
	int is_selflit, is_transparent, is_ground;
#ifdef  SIMPLE_LOD
	int x, y, dist;
#endif
#ifdef CLUSTER_INSIDES_OLD
	short cluster = get_actor_cluster ();
#endif

#ifdef SIMPLE_LOD
	x= -camera_x;
	y= -camera_y;
#endif

 	cur_e3d= NULL;
#ifdef  DEBUG
	cur_e3d_count= 0;
#endif  //DEBUG

	get_intersect_start_stop(main_bbox_tree, object_type, &start, &stop);
	// nothing to draw?
	if(start >= stop){
		return;
	}

	// reduce CPU usage while minimized
	if(!(SDL_GetAppState()&SDL_APPACTIVE)){
		// not actually drawing, fake it
		// now loop through each object
		for (i=start; i<stop; i++)
		{
			int	j;

			j = get_intersect_item_ID(main_bbox_tree, i);
			l = get_3dobject_index(j);
			if (objects_list[l] == NULL) continue;
			//track the usage
			cache_use(objects_list[l]->e3d_data->cache_ptr);
		}
		// and all done
		return;
	}

	// find the modes we need
	is_selflit= is_self_lit_3d_object(object_type);
	is_transparent= is_alpha_3d_object(object_type);
	is_ground= is_ground_3d_object(object_type);
	// set the modes we need
	if (is_selflit && (!is_day || dungeon))
	{
		glDisable(GL_LIGHTING);
	}

#ifdef	FSAA
	if (fsaa > 1)
	{
		glEnable(GL_MULTISAMPLE);
	}
#endif	/* FSAA */
	if(is_transparent) {
#ifdef	NEW_ALPHA
		if(use_3d_alpha_blend){
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		}
#endif	//NEW_ALPHA
		//enable alpha filtering, so we have some alpha key
		glEnable(GL_ALPHA_TEST);
		if(is_ground)glAlphaFunc(GL_GREATER,0.23f);
#ifdef OLD_MISC_OBJ_DIR
		else glAlphaFunc(GL_GREATER,0.06f);
#else
		else glAlphaFunc(GL_GREATER,0.3f);
#endif
		glDisable(GL_CULL_FACE);
	}

/*
	// NOTICE: The below code is an ASSUMPTION that appropriate client
	// states will be used!
*/
	if (!dungeon && (clouds_shadows||use_shadow_mapping))
	{
		ELglActiveTextureARB(detail_unit);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		ELglActiveTextureARB(base_unit);
	}
	// now loop through each object
	for (i=start; i<stop; i++)
	{
		int	j;

		j = get_intersect_item_ID(main_bbox_tree, i);
		l = get_3dobject_index(j);
		if (objects_list[l] == NULL) continue;
		//track the usage
		cache_use(objects_list[l]->e3d_data->cache_ptr);
		if(!objects_list[l]->display) continue;	// not currently on the map, ignore it
#ifdef CLUSTER_INSIDES_OLD
		if (objects_list[l]->cluster && objects_list[l]->cluster != cluster)
			// Object is in another cluster as actor, don't show it
			continue;
#endif // CLUSTER_INSIDES_OLD
#ifdef  SIMPLE_LOD
		// simple size/distance culling
		dist= (x-objects_list[l]->x_pos)*(x-objects_list[l]->x_pos) + (y-objects_list[l]->y_pos)*(y-objects_list[l]->y_pos);
		if(objects_list[l]->e3d_data->materials && (10000*objects_list[l]->e3d_data->materials[get_3dobject_material(j)].max_size)/(dist) < ((is_transparent)?15:10)) continue;
#endif  //SIMPLE_LOD

		draw_3d_object_detail(objects_list[l], get_3dobject_material(j), 1, 1, 1);

#ifdef MAP_EDITOR2
		if ((selected_3d_object == -1) && read_mouse_now && (get_cur_intersect_type(main_bbox_tree) == INTERSECTION_TYPE_DEFAULT))
#else
		if (read_mouse_now && (get_cur_intersect_type(main_bbox_tree) == INTERSECTION_TYPE_DEFAULT))
#endif
		{
			anything_under_the_mouse(objects_list[l]->id, UNDER_MOUSE_3D_OBJ);
		}
	}
	
	if (!dungeon && (clouds_shadows || use_shadow_mapping))
	{
		ELglActiveTextureARB(detail_unit);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		ELglActiveTextureARB(base_unit);
	}

	disable_buffer_arrays();

	// restore the settings
	if (is_selflit && (!is_day || dungeon)) 
	{
		glEnable(GL_LIGHTING);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
	if(is_transparent) {
		glEnable(GL_CULL_FACE);
#ifdef	NEW_ALPHA
		if(use_3d_alpha_blend){
			glDisable(GL_BLEND);
		}
#endif	//NEW_ALPHA
		glDisable(GL_ALPHA_TEST);
	}
#ifdef	FSAA
	if (fsaa > 1)
	{
		glDisable(GL_MULTISAMPLE);
	}
#endif	/* FSAA */

	CHECK_GL_ERRORS();

#ifdef  DEBUG
	// final statistics
	if(cur_e3d_count > 0){
		e3d_count++;
		e3d_total+= cur_e3d_count;
	}
	cur_e3d_count= 0;
#endif  //DEBUG
}

//Tests to see if an e3d object is already loaded. If it is, return the handle.
//If not, load it, and return the handle
static e3d_object *load_e3d_cache(const char* file_name)
{
	e3d_object *e3d_id;

	//do we have it already?
	e3d_id = cache_find_item(cache_e3d, file_name);
	if (e3d_id) return e3d_id;

	//e3d not found in the cache, so load it, and store it
	// allocate the memory
	e3d_id = calloc(1, sizeof(e3d_object));
	if (!e3d_id)
	{
		LOG_ERROR("Can't allocate data for file \"%s\"!", file_name);
		return NULL;
	}
	// and fill in the data
	my_strncp(e3d_id->file_name, file_name, sizeof(e3d_id->file_name));

	e3d_id = load_e3d_detail(e3d_id);
	if (!e3d_id)
	{
		LOG_ERROR("Can't load file \"%s\"!", file_name);
		return NULL;
	}

	e3d_id->cache_ptr = cache_add_item(cache_e3d, e3d_id->file_name,
		e3d_id, sizeof(*e3d_id));

	return e3d_id;
}

int add_e3d_at_id(int id, const char* file_name,
	float x_pos, float y_pos, float z_pos,
	float x_rot, float y_rot, float z_rot, char self_lit, char blended,
	float r, float g, float b, unsigned int dynamic)
{
	char fname[128];
#ifdef FASTER_MAP_LOAD
	const char *fbase;
#endif
	e3d_object *returned_e3d;
	object3d *our_object;
	int i;
	AABBOX bbox;
	unsigned int texture_id;
	unsigned int is_transparent, ground;

	if (id < 0 || id >= MAX_OBJ_3D)
	{
		LOG_ERROR("Invalid object id %d", id);
		return -1;
	}

	if (objects_list[id])
	{
		LOG_ERROR("There's already an object with ID %d", id);
		return -1;
	}

	// convert to lower case and replace any '\' by '/'
	clean_file_name(fname, file_name, sizeof(fname));
#ifdef FASTER_MAP_LOAD
	fbase = strrchr(fname, '/');
	if (fbase)
		fbase++;
	else
		fbase = fname;
#endif

	returned_e3d = load_e3d_cache(fname);
	if (!returned_e3d)
	{
		LOG_ERROR(nasty_error_str, fname);
		//replace it with the null object, to avoid object IDs corruption
#ifdef OLD_MISC_OBJ_DIR
		returned_e3d = load_e3d_cache("./3dobjects/misc_objects/badobject.e3d");
#else
		returned_e3d = load_e3d_cache("./3dobjects/badobject.e3d");
#endif
		if (!returned_e3d)
			return -1; // umm, not even found the place holder, this is teh SUCK!!!
	}
	// now, allocate the memory
	our_object = calloc(1, sizeof (object3d));

	// and fill it in
	my_strncp(our_object->file_name, fname, sizeof(our_object->file_name));
	our_object->x_pos = x_pos;
	our_object->y_pos = y_pos;
	our_object->z_pos = z_pos;

	our_object->x_rot = x_rot;
	our_object->y_rot = y_rot;
	our_object->z_rot = z_rot;

	our_object->color[0] = r;
	our_object->color[1] = g;
	our_object->color[2] = b;
	our_object->color[3] = 0.0f;

	our_object->self_lit = self_lit;
	our_object->blended = blended;
	our_object->display = 1;
	our_object->state = 0;

	build_clouds_planes(our_object);

	our_object->e3d_data = returned_e3d;

	our_object->id = id;

	our_object->flags = 0;

#ifdef FASTER_MAP_LOAD
	if (is_harvestable(fbase))
		our_object->flags |= OBJ_3D_HARVESTABLE;
	if (is_entrable(fbase))
		our_object->flags |= OBJ_3D_ENTRABLE;
	if (*fbase && strcasecmp(fbase, "bag1.e3d") == 0)
		our_object->flags |= OBJ_3D_BAG;
	if (*fbase && strcasecmp(fbase, "branch1.e3d") == 0)
		our_object->flags |= OBJ_3D_MINE;
#else  // FASTER_MAP_LOAD
	for(i = 0; i < sizeof(harvestable_objects)/sizeof(harvestable_objects[0]); i++) {
		if(*harvestable_objects[i] && strstr(file_name, harvestable_objects[i]) != NULL) {
			our_object->flags |= OBJ_3D_HARVESTABLE;
			break;
		}
	}
	for(i = 0; i < sizeof(entrable_objects)/sizeof(entrable_objects[0]); i++) {
		if(*(entrable_objects[i]) && strstr(file_name, entrable_objects[i]) != NULL) {
			our_object->flags |= OBJ_3D_ENTRABLE;
			break;
		}
	}
	if (strcasecmp(file_name, "") && strcasecmp(strrchr(file_name, '/')+1, "bag1.e3d") == 0) {
		our_object->flags |= OBJ_3D_BAG;
	}
	if (strcasecmp(file_name, "") && strcasecmp(strrchr(file_name, '/')+1, "branch1.e3d") == 0) {
		our_object->flags |= OBJ_3D_MINE;
	}
#endif // FASTER_MAP_LOAD

#ifdef CLUSTER_INSIDES
	our_object->cluster = get_cluster ((int)(x_pos/0.5f), (int)(y_pos/0.5f));
	current_cluster = our_object->cluster;
#endif

	objects_list[id] = our_object;
	// watch the top end
	if (id >= next_obj_3d)
		next_obj_3d = id+1;

	calc_rotation_and_translation_matrix(our_object->matrix, x_pos, y_pos, z_pos, x_rot, y_rot, z_rot);

	// watch for needing to load the detailed information
	//load_e3d_detail_if_needed(returned_e3d);

	ground = returned_e3d->vertex_layout->normal_count == 0;

	for (i = 0; i < returned_e3d->material_no; i++)
	{
		bbox.bbmin[X] = returned_e3d->materials[i].min_x;
		bbox.bbmax[X] = returned_e3d->materials[i].max_x;
		bbox.bbmin[Y] = returned_e3d->materials[i].min_y;
		bbox.bbmax[Y] = returned_e3d->materials[i].max_y;
		bbox.bbmin[Z] = returned_e3d->materials[i].min_z;
		bbox.bbmax[Z] = returned_e3d->materials[i].max_z;

		matrix_mul_aabb(&bbox, our_object->matrix);
		texture_id = returned_e3d->materials[i].texture;
		is_transparent = returned_e3d->materials[i].options != 0;
		if (main_bbox_tree_items != NULL && dynamic == 0)
			add_3dobject_to_list(main_bbox_tree_items,
				get_3dobject_id(id, i), bbox, blended, ground,
				is_transparent, self_lit, texture_id);
		else
			add_3dobject_to_abt(main_bbox_tree,
				get_3dobject_id(id, i), bbox, blended, ground,
				is_transparent, self_lit, texture_id, dynamic);
	}

	add_ec_effect_to_e3d(our_object);
	ec_add_object_obstruction(our_object, returned_e3d, 2.0);
//	LOG_DEBUG_VERBOSE("Bounding: %f, %f, %f -> %f, %f, %f\n", returned_e3d->min_x, returned_e3d->min_y, returned_e3d->min_z, returned_e3d->max_x, returned_e3d->max_y, returned_e3d->max_z);
//	LOG_DEBUG_VERBOSE("Rotation: %f, %f, %f\n", our_object->x_rot, our_object->y_rot, our_object->z_rot);

	return id;
}

int add_e3d(const char* file_name, float x_pos, float y_pos, float z_pos,
	float x_rot, float y_rot, float z_rot, char self_lit, char blended,
	float r, float g, float b, unsigned int dynamic)
{
	int i;
#ifndef FASTER_MAP_LOAD
	int j = 0;
#endif

#ifdef FASTER_MAP_LOAD
	if (next_obj_3d < MAX_OBJ_3D && !objects_list[next_obj_3d])
		return add_e3d_at_id(next_obj_3d, file_name, x_pos, y_pos, z_pos,
			x_rot, y_rot, z_rot, self_lit, blended,
			r, g, b, dynamic);

	// Oh my, next_obj_3d is not free. Find a free spot in the e3d_list,
	// but don't count on IDs being correct.
#endif
	for (i = 0; i < MAX_OBJ_3D; i++)
	{
		if (!objects_list[i])
		{
#ifndef FASTER_MAP_LOAD
			if (j < objects_list_placeholders)
				j++;
			else
#endif
			return add_e3d_at_id(i, file_name, x_pos, y_pos, z_pos,
				x_rot, y_rot, z_rot, self_lit, blended,
				r, g, b, dynamic);
		}
	}

	// No free spot available
	return -1;
}

#ifdef NEW_SOUND
char * get_3dobject_at_location(float x_pos, float y_pos)
{
	int i;
	float offset = 0.5f;
	for (i = 0; i < MAX_OBJ_3D; i++)
	{
		if (objects_list[i]
			&& objects_list[i]->x_pos > (x_pos - offset) && objects_list[i]->x_pos < (x_pos + offset)
			&& objects_list[i]->y_pos > (y_pos - offset) && objects_list[i]->y_pos < (y_pos + offset)
			&& objects_list[i]->display)
		{
			return objects_list[i]->file_name;
		}
	}
	return "";
}
#endif // NEW_SOUND

void display_objects()
{	
	CHECK_GL_ERRORS();
	glEnable(GL_CULL_FACE);
	glEnable(GL_COLOR_MATERIAL);
	glEnableClientState(GL_VERTEX_ARRAY);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			
	if (!dungeon && clouds_shadows)
	{
		//bind the detail texture
		ELglActiveTextureARB(detail_unit);
		glEnable(GL_TEXTURE_2D);
#ifdef	NEW_TEXTURES
		bind_texture_unbuffered(ground_detail_text);
#else	/* NEW_TEXTURES */
		glBindTexture(GL_TEXTURE_2D, get_texture_id(ground_detail_text));
#endif	/* NEW_TEXTURES */
		ELglActiveTextureARB(base_unit);
		glEnable(GL_TEXTURE_2D);
	}

	CHECK_GL_ERRORS();

	draw_3d_objects(TYPE_3D_NO_BLEND_NO_GROUND_NO_ALPHA_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_NO_BLEND_NO_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_NO_BLEND_GROUND_NO_ALPHA_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_NO_BLEND_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT);

	CHECK_GL_ERRORS();
	glDisable(GL_CULL_FACE);
	glDisable(GL_COLOR_MATERIAL);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	if (!dungeon && clouds_shadows)
	{
		//disable the second texture unit
		ELglActiveTextureARB(detail_unit);
		glDisable(GL_TEXTURE_2D);
		ELglActiveTextureARB(base_unit);
	}
	CHECK_GL_ERRORS();
}

void display_ground_objects()
{
	CHECK_GL_ERRORS();
	glEnable(GL_CULL_FACE);
	glEnable(GL_COLOR_MATERIAL);
	glEnableClientState(GL_VERTEX_ARRAY);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	if (!dungeon && clouds_shadows)
	{
		//bind the detail texture
		ELglActiveTextureARB(detail_unit);
		glEnable(GL_TEXTURE_2D);
#ifdef	NEW_TEXTURES
		bind_texture_unbuffered(ground_detail_text);
#else	/* NEW_TEXTURES */
		glBindTexture(GL_TEXTURE_2D, get_texture_id(ground_detail_text));
#endif	/* NEW_TEXTURES */
		ELglActiveTextureARB(base_unit);
		glEnable(GL_TEXTURE_2D);
	}

	CHECK_GL_ERRORS();

	draw_3d_objects(TYPE_3D_NO_BLEND_GROUND_ALPHA_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_NO_BLEND_GROUND_ALPHA_NO_SELF_LIT_OBJECT);

	CHECK_GL_ERRORS();
	glDisable(GL_CULL_FACE);
	glDisable(GL_COLOR_MATERIAL);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	if (!dungeon && clouds_shadows)
	{
		//disable the second texture unit
		ELglActiveTextureARB(detail_unit);
		glDisable(GL_TEXTURE_2D);
		ELglActiveTextureARB(base_unit);
	}
	CHECK_GL_ERRORS();
}

void display_alpha_objects()
{
	CHECK_GL_ERRORS();
	glEnable(GL_COLOR_MATERIAL);
	glEnableClientState(GL_VERTEX_ARRAY);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			
	if (!dungeon && clouds_shadows)
	{
		//bind the detail texture
		ELglActiveTextureARB(detail_unit);
		glEnable(GL_TEXTURE_2D);
#ifdef	NEW_TEXTURES
		bind_texture_unbuffered(ground_detail_text);
#else	/* NEW_TEXTURES */
		glBindTexture(GL_TEXTURE_2D, get_texture_id(ground_detail_text));
#endif	/* NEW_TEXTURES */
		ELglActiveTextureARB(base_unit);
		glEnable(GL_TEXTURE_2D);
	}

	CHECK_GL_ERRORS();

	draw_3d_objects(TYPE_3D_NO_BLEND_NO_GROUND_ALPHA_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_NO_BLEND_NO_GROUND_ALPHA_NO_SELF_LIT_OBJECT);

	CHECK_GL_ERRORS();
	glDisable(GL_COLOR_MATERIAL);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	if (!dungeon && clouds_shadows)
	{
		//disable the second texture unit
		ELglActiveTextureARB(detail_unit);
		glDisable(GL_TEXTURE_2D);
		ELglActiveTextureARB(base_unit);
	}
	CHECK_GL_ERRORS();
}

void display_blended_objects()
{	
	CHECK_GL_ERRORS();
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_ONE);
	glEnable(GL_COLOR_MATERIAL);
	glEnableClientState(GL_VERTEX_ARRAY);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			
	if (!dungeon && clouds_shadows)
	{
		//bind the detail texture
		ELglActiveTextureARB(detail_unit);
		glEnable(GL_TEXTURE_2D);
#ifdef	NEW_TEXTURES
		bind_texture_unbuffered(ground_detail_text);
#else	/* NEW_TEXTURES */
		glBindTexture(GL_TEXTURE_2D, get_texture_id(ground_detail_text));
#endif	/* NEW_TEXTURES */
		ELglActiveTextureARB(base_unit);
		glEnable(GL_TEXTURE_2D);
	}

	CHECK_GL_ERRORS();

	draw_3d_objects(TYPE_3D_BLEND_GROUND_NO_ALPHA_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_BLEND_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_BLEND_GROUND_ALPHA_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_BLEND_GROUND_ALPHA_NO_SELF_LIT_OBJECT);

	draw_3d_objects(TYPE_3D_BLEND_NO_GROUND_NO_ALPHA_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_BLEND_NO_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_BLEND_NO_GROUND_ALPHA_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_BLEND_NO_GROUND_ALPHA_NO_SELF_LIT_OBJECT);

	CHECK_GL_ERRORS();
	glDisable(GL_CULL_FACE);
	glDisable(GL_COLOR_MATERIAL);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_BLEND);
	if (!dungeon && clouds_shadows)
	{
		//disable the second texture unit
		ELglActiveTextureARB(detail_unit);
		glDisable(GL_TEXTURE_2D);
		ELglActiveTextureARB(base_unit);
	}
	CHECK_GL_ERRORS();
}

void destroy_3d_object(int i)
{
	if (i < 0 || i >= MAX_OBJ_3D || !objects_list[i])
		return;

	ec_remove_obstruction_by_object3d(objects_list[i]);

	delete_3dobject_from_abt(main_bbox_tree, i, objects_list[i]->blended, objects_list[i]->self_lit);
	free(objects_list[i]);
	objects_list[i] = NULL;
	if (i == next_obj_3d-1)
	{
		while (next_obj_3d > 0 && !objects_list[next_obj_3d-1])
			next_obj_3d--;
	}
}

void destroy_all_3d_objects()
{
	int i;

	for (i = 0; i < MAX_OBJ_3D; i++)
	{
		if (objects_list[i])
		{
			ec_remove_obstruction_by_object3d(objects_list[i]);
			if(!cache_find_item(cache_e3d, objects_list[i]->file_name))
				destroy_e3d(objects_list[i]->e3d_data);
			free(objects_list[i]);
			objects_list[i] = NULL; // kill any reference to it
		}
	}

	// reset the top pointer
	next_obj_3d = 0;
}

Uint32 free_e3d_va(e3d_object *e3d_id)
{
	set_all_intersect_update_needed(main_bbox_tree);

	if (e3d_id != NULL)
	{
		if (e3d_id->vertex_data != NULL)
		{
			free(e3d_id->vertex_data);
			e3d_id->vertex_data = NULL;
		}
		if (e3d_id->indices != NULL)
		{
			free(e3d_id->indices);
			e3d_id->indices = NULL;
		}
		if (e3d_id->vertex_vbo != 0)
		{
			ELglDeleteBuffersARB(1, &e3d_id->vertex_vbo);
			e3d_id->vertex_vbo = 0;
		}
		if (e3d_id->indices_vbo != 0)
		{
			ELglDeleteBuffersARB(1, &e3d_id->indices_vbo);
			e3d_id->indices_vbo = 0;
		}
	}

	if (e3d_id->cache_ptr != NULL)
		return (e3d_id->cache_ptr->size - sizeof(*e3d_id));
	else
		return sizeof(*e3d_id);
}

void destroy_e3d(e3d_object *e3d_id)
{
	// release the detailed data
	free_e3d_va(e3d_id);
	// free the materials (not free'd in free_e3d_va)
	if(e3d_id->materials) free(e3d_id->materials);
	e3d_id->materials= NULL;
	// and finally free the main object

	ec_remove_obstruction_by_e3d_object(e3d_id);

	free(e3d_id);
}

// for support of the 1.0.3 server, change if an object is to be displayed or not
void set_3d_object (Uint8 display, const void *ptr, int len)
{
	const Uint32 *id_ptr = ptr;

	// first look for the override to process ALL objects
	if (len < sizeof(*id_ptr))
	{
		int i;
		for (i = 0; i < next_obj_3d; i++)
		{
			if (objects_list[i])
				objects_list[i]->display = display;
		}
	}
	else
	{
		int idx = 0;
		while (len >= sizeof(*id_ptr))
		{
			int obj_id = SDL_SwapLE32(id_ptr[idx]);
			if (obj_id < next_obj_3d && objects_list[obj_id])
			{
				objects_list[obj_id]->display = display;
				idx++;
				len -= sizeof (*id_ptr);
			}
		}
	}
}

// for future expansion
void state_3d_object (Uint8 state, const void *ptr, int len)
{
	const Uint32 *id_ptr = ptr;

	// first look for the override to process ALL objects
	if (len < sizeof(*id_ptr))
	{
		int i;
		for (i = 0; i < next_obj_3d; i++)
		{
			if (objects_list[i])
				objects_list[i]->state= state;
		}
	}
	else
	{
		int idx = 0;

		while (len >= sizeof(*id_ptr))
		{
			int obj_id = SDL_SwapLE32(id_ptr[idx]);
			if (obj_id < next_obj_3d && objects_list[obj_id])
			{
				objects_list[obj_id]->state = state;
				idx++;
				len -= sizeof (*id_ptr);
			}
		}
	}
}
