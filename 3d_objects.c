#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "io/e3d_io.h"
#ifdef CLUSTER_INSIDES
#include "cluster.h"
#endif

int use_3d_alpha_blend= 1;
Uint32 highest_obj_3d= 0;
int objects_list_placeholders = 0;
object3d *objects_list[MAX_OBJ_3D];

#ifdef EYE_CANDY
 #include "eye_candy_wrapper.h"
#endif

e3d_object *load_e3d (const char *file_name);
void compute_clouds_map(object3d * object_id);
e3d_object *cur_e3d;
#ifdef  DEBUG
int cur_e3d_count;
int e3d_count, e3d_total;
#endif  //DEBUG

void clear_objects_list_placeholders()
{
	objects_list_placeholders = 0;
}

void inc_objects_list_placeholders()
{
	objects_list_placeholders++;
}

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

void draw_3d_object_detail(object3d * object_id, unsigned int material_index)
{
	Uint8 * data_ptr;
	int vertex_size;

	// check for having to load the arrays
	load_e3d_detail_if_needed(object_id->e3d_data);

	CHECK_GL_ERRORS();
	//also, update the last time this object was used
	object_id->last_acessed_time = cur_time;

	//debug

#ifdef NEW_LIGHTING
	if (
	    ( (use_new_lighting) && (object_id->self_lit && (!(game_minute >= 5 && game_minute < 235) || dungeon))) ||
	    (!(use_new_lighting) && (object_id->self_lit && (!is_day || dungeon)))
	   )
#else
	if (object_id->self_lit && (!is_day || dungeon)) 
#endif
	{
		glColor3f(object_id->r,object_id->g,object_id->b);
	}
	CHECK_GL_ERRORS();

	glPushMatrix();//we don't want to affect the rest of the scene

	glMultMatrixf(object_id->matrix);

	CHECK_GL_ERRORS();

	if (!dungeon && (clouds_shadows || use_shadow_mapping))
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
		vertex_size = get_vertex_size(object_id->e3d_data->vertex_options);

		if (has_normal(object_id->e3d_data->vertex_options))
		{
			glEnableClientState(GL_NORMAL_ARRAY);
			glNormalPointer(GL_FLOAT, vertex_size,
				data_ptr + get_normal_offset(object_id->e3d_data->vertex_options));
		}
		else
			glDisableClientState(GL_NORMAL_ARRAY);

#ifdef	USE_TANGENT
		if (use_tangent && has_tangen(object_id->e3d_data->vertex_options))
		{
			EnableVertexAttribArray(tangent_attribut);
			VertexAttribPointer(tangent_attribut, TANGENT_FLOAT_COUNT, GL_FLOAT,
				GL_FALSE, vertex_size,
				data_ptr + get_tangent_offset(vertex_options));
		}
		else
		{
			DisableVertexAttribArray(tangent_attribut);
		}
#endif	//USE_TANGENT
#ifdef	USE_EXTRA_TEXTURE
		if (use_extra_texture && has_extra_texture(object_id->e3d_data->vertex_options))
		{
			glClientActiveTextureARB(extra_texture_unit);
			ELglActiveTextureARB(extra_texture_unit);
			glTexCoordPointer(EXTRA_TEXTURE_FLOAT_COUNT, GL_FLOAT, vertex_size,
				data_ptr + get_extra_texture_offset(vertex_options));
			ELglActiveTextureARB(base_unit);
			glClientActiveTextureARB(base_unit);
		}
		else
		{
			glClientActiveTextureARB(extra_texture_unit);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glClientActiveTextureARB(base_unit);
		}
#endif	//USE_EXTRA_TEXTURE

		glTexCoordPointer(TEXTURE_FLOAT_COUNT, GL_FLOAT, vertex_size,
			data_ptr + get_texture_offset(object_id->e3d_data->vertex_options));
		glVertexPointer(VERTEX_FLOAT_COUNT, GL_FLOAT, vertex_size,
			data_ptr + get_vertex_offset(object_id->e3d_data->vertex_options));
		if (use_vertex_buffers)
		{
			ELglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
				object_id->e3d_data->indicies_vbo);
		}

		CHECK_GL_ERRORS();

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
	get_and_set_texture_id(object_id->e3d_data->materials[material_index].diffuse_map);

	ELglDrawRangeElementsEXT(GL_TRIANGLES,
		object_id->e3d_data->materials[material_index].triangles_indicies_min,
		object_id->e3d_data->materials[material_index].triangles_indicies_max,
		object_id->e3d_data->materials[material_index].triangles_indicies_count,
		object_id->e3d_data->index_type,
		object_id->e3d_data->materials[material_index].triangles_indicies_index);

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
#ifdef CLUSTER_INSIDES	
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
			cache_use(cache_e3d, objects_list[l]->e3d_data->cache_ptr);
		}
		// and all done
		return;
	}
	
	// find the modes we need
	is_selflit= is_self_lit_3d_object(object_type);
	is_transparent= is_alpha_3d_object(object_type);
	is_ground= is_ground_3d_object(object_type);
	// set the modes we need
#ifdef NEW_LIGHTING
	if (
	    ( (use_new_lighting) && (is_selflit && (!(game_minute >= 5 && game_minute < 235) || dungeon))) ||
	    (!(use_new_lighting) && (is_selflit && (!is_day || dungeon)))
	   )
#else
	if (is_selflit && (!is_day || dungeon)) 
#endif
	{
		glDisable(GL_LIGHTING);
	}
#ifdef NEW_LIGHTING
	else if (use_new_lighting)
	{
		reset_material();
	}
#endif

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
		else glAlphaFunc(GL_GREATER,0.06f);
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
		cache_use(cache_e3d, objects_list[l]->e3d_data->cache_ptr);
		if(!objects_list[l]->display) continue;	// not currently on the map, ignore it
#ifdef CLUSTER_INSIDES
		if (objects_list[l]->cluster && objects_list[l]->cluster != cluster)
			// Object is in another cluster as actor, don't show it
			continue;
#endif // CLUSTER_INSIDES
#ifdef  SIMPLE_LOD
		// simple size/distance culling
		dist= (x-objects_list[l]->x_pos)*(x-objects_list[l]->x_pos) + (y-objects_list[l]->y_pos)*(y-objects_list[l]->y_pos);
		if(objects_list[l]->e3d_data->materials && (10000*objects_list[l]->e3d_data->materials[get_3dobject_material(j)].max_size)/(dist) < ((is_transparent)?15:10)) continue;
#endif  //SIMPLE_LOD

		draw_3d_object_detail(objects_list[l], get_3dobject_material(j));

#ifdef MAP_EDITOR2
		if ((selected_3d_object == -1) && read_mouse_now && (get_cur_intersect_type(main_bbox_tree) == INTERSECTION_TYPE_DEFAULT))
#else
		if (read_mouse_now && (get_cur_intersect_type(main_bbox_tree) == INTERSECTION_TYPE_DEFAULT))
#endif
		{
			if (click_line_bbox_intersection(get_intersect_item_bbox(main_bbox_tree, i)))
				anything_under_the_mouse(objects_list[l]->id, UNDER_MOUSE_3D_OBJ);
		}
	}
	
	if (use_compiled_vertex_array && (cur_e3d != NULL))
	{
		ELglUnlockArraysEXT();
	}
	if (!dungeon && (clouds_shadows || use_shadow_mapping))
	{
		ELglActiveTextureARB(detail_unit);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		ELglActiveTextureARB(base_unit);
	}
#ifdef	USE_TANGENT
	if (use_tangent && has_tangen(object_id->e3d_data->vertex_options))
	{
		DisableVertexAttribArray(tangent_attribut);
	}
#endif	//USE_TANGENT
#ifdef	USE_EXTRA_TEXTURE
	if (use_extra_texture && has_extra_texture(object_id->e3d_data->vertex_options))
	{
		glClientActiveTextureARB(GL_TEXTURE2_ARB);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glClientActiveTextureARB(GL_TEXTURE0_ARB);
	}
#endif	//USE_EXTRA_TEXTURE

	if (use_vertex_buffers)
	{
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		ELglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	}
	// restore the settings
#ifdef NEW_LIGHTING
	if (
	    ( (use_new_lighting) && (is_selflit && (!(game_minute >= 5 && game_minute < 235) || dungeon))) ||
	    (!(use_new_lighting) && (is_selflit && (!is_day || dungeon)))
	   )
#else
	if (is_selflit && (!is_day || dungeon)) 
#endif
	{
		glEnable(GL_LIGHTING);
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

	CHECK_GL_ERRORS();

#ifdef  DEBUG
	// final statistics
	if(cur_e3d_count > 0){
		e3d_count++;
		e3d_total+= cur_e3d_count;
	}
	cur_e3d_count= 0;
#endif  //DEBUG
	cur_e3d= NULL;
}

//Tests to see if an e3d object is already loaded. If it is, return the handle.
//If not, load it, and return the handle
e3d_object *load_e3d_cache (const char * file_name)
{
	e3d_object *e3d_id;

	//do we have it already?
	e3d_id = cache_find_item (cache_e3d, file_name);
	if (e3d_id != NULL) return e3d_id;

	//e3d not found in the cache, so load it, and store it
	// allocate the memory
	e3d_id = (e3d_object*)malloc(sizeof(e3d_object));
	if (e3d_id == NULL) 
	{
		LOG_ERROR("Can't alloc data for file \"%s\"!", file_name);
		return NULL;
	}
	// and fill in the data
	memset(e3d_id, 0, sizeof(e3d_object));
	if (e3d_id == NULL) 
	{
		LOG_ERROR("Memset Error for file \"%s\"!", file_name);
		return NULL;
	}
	my_strncp(e3d_id->file_name, file_name, sizeof(e3d_id->file_name));
	
	e3d_id = load_e3d_detail(e3d_id);

	if (e3d_id == NULL) 
	{
		LOG_ERROR("Can't load file \"%s\"!", file_name);
		return NULL;
	}

	e3d_id->cache_ptr = cache_add_item (cache_e3d, e3d_id->file_name, e3d_id, sizeof(*e3d_id));
	
	return e3d_id;
}

int add_e3d_at_id (int id, const char *file_name, float x_pos, float y_pos, float z_pos, float x_rot, float y_rot, float z_rot, char self_lit, char blended, float r, float g, float b, unsigned int dynamic)
{
	char fname[128];
	e3d_object *returned_e3d;
	object3d *our_object;
	int i;
	AABBOX bbox;
	unsigned int texture_id;
	unsigned int is_transparent, ground;
/*
        if (!strncmp(file_name, "./3dobjects/trees/treeleaves4", 29))
        {
          return 0;
        }
*/

	if(id < 0 || id >= MAX_OBJ_3D)
	{
		LOG_ERROR ("Invalid object id %d", id);
		return -1;
	}

	if(objects_list[id] != NULL)
	{
		LOG_ERROR("There's already an object with ID %d", id);
		return -1;
	}

	//convert any '\' in '/'
	clean_file_name(fname, file_name, 128);
	my_tolower(fname);

	returned_e3d= load_e3d_cache(fname);
	if(returned_e3d==NULL)
	{
		LOG_ERROR (nasty_error_str, fname);
		//replace it with the null object, to avoid object IDs corruption
		returned_e3d= load_e3d_cache ("./3dobjects/misc_objects/badobject.e3d");
		if(returned_e3d == NULL){
			return 0; // umm, not even found the place holder, this is teh SUCK!!!
		}
	}
	// now, allocate the memory
	our_object= calloc(1, sizeof (object3d));

	// and fill it in
	my_strncp(our_object->file_name, fname, 80);
	our_object->x_pos = x_pos;
	our_object->y_pos = y_pos;
	our_object->z_pos = z_pos;

	our_object->x_rot = x_rot;
	our_object->y_rot = y_rot;
	our_object->z_rot = z_rot;

	our_object->r = r;
	our_object->g = g;
	our_object->b = b;

	our_object->self_lit = self_lit;
	our_object->blended = blended;
	our_object->display = 1;
	our_object->state = 0;

	build_clouds_planes(our_object);

	our_object->e3d_data = returned_e3d;
	
	our_object->id = id;

	our_object->flags = 0;
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
	if(strcasecmp(strrchr(file_name, '/')+1, "bag1.e3d") == 0) {
		our_object->flags |= OBJ_3D_BAG;
	}
#ifdef MINES
	if(strcasecmp(strrchr(file_name, '/')+1, "branch1.e3d") == 0) {
		our_object->flags |= OBJ_3D_MINE;
	}
#endif // MINES

#ifdef CLUSTER_INSIDES
	our_object->cluster = get_cluster ((int)(x_pos/0.5f), (int)(y_pos/0.5f));
#endif

	objects_list[id] = our_object;
	// watch the top end
	if((Uint32)id >= highest_obj_3d)
	{
		highest_obj_3d = id+1;
	}

	calc_rotation_and_translation_matrix(our_object->matrix, x_pos, y_pos, z_pos, x_rot, y_rot, z_rot);
	
	// watch for needing to load the detailed information
	//load_e3d_detail_if_needed(returned_e3d);

	ground = !has_normal(returned_e3d->vertex_options);

	for (i = 0; i < returned_e3d->material_no; i++)
	{	
		bbox.bbmin[X] = returned_e3d->materials[i].min_x;
		bbox.bbmax[X] = returned_e3d->materials[i].max_x;
		bbox.bbmin[Y] = returned_e3d->materials[i].min_y;
		bbox.bbmax[Y] = returned_e3d->materials[i].max_y;
		bbox.bbmin[Z] = returned_e3d->materials[i].min_z;
		bbox.bbmax[Z] = returned_e3d->materials[i].max_z;

		matrix_mul_aabb(&bbox, our_object->matrix);
		texture_id = returned_e3d->materials[i].diffuse_map;
		is_transparent = material_is_transparent(returned_e3d->materials[i].options);
		if ((main_bbox_tree_items != NULL) && (dynamic == 0))  add_3dobject_to_list(main_bbox_tree_items, get_3dobject_id(id, i), bbox, blended, ground, is_transparent, self_lit, texture_id);
		else add_3dobject_to_abt(main_bbox_tree, get_3dobject_id(id, i), bbox, blended, ground, is_transparent, self_lit, texture_id, dynamic);
	}

#ifdef EYE_CANDY
	ec_add_object_obstruction(our_object, returned_e3d, 2.0);
//        printf("Bounding: %f, %f, %f -> %f, %f, %f\n", returned_e3d->min_x, returned_e3d->min_y, returned_e3d->min_z, returned_e3d->max_x, returned_e3d->max_y, returned_e3d->max_z);
//	printf("Rotation: %f, %f, %f\n", our_object->x_rot, our_object->y_rot, our_object->z_rot);
#endif	// EYE_CANDY

	return id;
}

int add_e3d (const char * file_name, float x_pos, float y_pos, float z_pos, float x_rot, float y_rot, float z_rot, char self_lit, char blended, float r, float g, float b, unsigned int dynamic)
{
	int i, j;

	j = 0;
	
	//find a free spot, in the e3d_list
	for(i = 0; i < MAX_OBJ_3D; i++)
	{
		if(objects_list[i] == NULL)
		{
			if (j < objects_list_placeholders) j++;
			else break;
		}
	}
	
	return add_e3d_at_id (i, file_name, x_pos, y_pos, z_pos, x_rot, y_rot, z_rot, self_lit, blended, r, g, b, dynamic);
}

void display_objects()
{	
	CHECK_GL_ERRORS();
	glEnable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			
	if (!dungeon && clouds_shadows)
	{
		//bind the detail texture
		ELglActiveTextureARB(detail_unit);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, get_texture_id(ground_detail_text));
		ELglActiveTextureARB(base_unit);
		glEnable(GL_TEXTURE_2D);
	}

	CHECK_GL_ERRORS();

	glEnableClientState(GL_NORMAL_ARRAY);
	draw_3d_objects(TYPE_3D_NO_BLEND_NO_GROUND_NO_ALPHA_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_NO_BLEND_NO_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT);

	glDisableClientState(GL_NORMAL_ARRAY);
	glNormal3f(0,0,1);

	draw_3d_objects(TYPE_3D_NO_BLEND_GROUND_NO_ALPHA_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_NO_BLEND_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT);
	
	CHECK_GL_ERRORS();
	glDisable(GL_CULL_FACE);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
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
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			
	if (!dungeon && clouds_shadows)
	{
		//bind the detail texture
		ELglActiveTextureARB(detail_unit);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, get_texture_id(ground_detail_text));
		ELglActiveTextureARB(base_unit);
		glEnable(GL_TEXTURE_2D);
	}

	CHECK_GL_ERRORS();

	glDisableClientState(GL_NORMAL_ARRAY);
	glNormal3f(0,0,1);

	draw_3d_objects(TYPE_3D_NO_BLEND_GROUND_ALPHA_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_NO_BLEND_GROUND_ALPHA_NO_SELF_LIT_OBJECT);
	
	CHECK_GL_ERRORS();
	glDisable(GL_CULL_FACE);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
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
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			
	if (!dungeon && clouds_shadows)
	{
		//bind the detail texture
		ELglActiveTextureARB(detail_unit);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, get_texture_id(ground_detail_text));
		ELglActiveTextureARB(base_unit);
		glEnable(GL_TEXTURE_2D);
	}

	CHECK_GL_ERRORS();

	glDisableClientState(GL_NORMAL_ARRAY);
	glNormal3f(0,0,1);

	glEnableClientState(GL_NORMAL_ARRAY);
	draw_3d_objects(TYPE_3D_NO_BLEND_NO_GROUND_ALPHA_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_NO_BLEND_NO_GROUND_ALPHA_NO_SELF_LIT_OBJECT);

	CHECK_GL_ERRORS();
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
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
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			
	if (!dungeon && clouds_shadows)
	{
		//bind the detail texture
		ELglActiveTextureARB(detail_unit);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, get_texture_id(ground_detail_text));
		ELglActiveTextureARB(base_unit);
		glEnable(GL_TEXTURE_2D);
	}

	CHECK_GL_ERRORS();

	glDisableClientState(GL_NORMAL_ARRAY);
	glNormal3f(0,0,1);

	draw_3d_objects(TYPE_3D_BLEND_GROUND_NO_ALPHA_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_BLEND_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_BLEND_GROUND_ALPHA_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_BLEND_GROUND_ALPHA_NO_SELF_LIT_OBJECT);

	glEnableClientState(GL_NORMAL_ARRAY);
	draw_3d_objects(TYPE_3D_BLEND_NO_GROUND_NO_ALPHA_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_BLEND_NO_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_BLEND_NO_GROUND_ALPHA_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_BLEND_NO_GROUND_ALPHA_NO_SELF_LIT_OBJECT);

	CHECK_GL_ERRORS();
	glDisable(GL_CULL_FACE);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
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
	if ((i < 0) || (i >= MAX_OBJ_3D)) return;
	if (objects_list[i] == NULL) return;

#ifdef  EYE_CANDY
	ec_remove_obstruction_by_object3d(objects_list[i]);
#endif

	delete_3dobject_from_abt(main_bbox_tree, i, objects_list[i]->blended, objects_list[i]->self_lit);
	free(objects_list[i]);
	objects_list[i] = NULL;
	if((Uint32)i == highest_obj_3d+1){
		highest_obj_3d = i;
	}
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
		if (e3d_id->indicies != NULL)
		{
			free(e3d_id->indicies);
			e3d_id->indicies = NULL;
		}
		if (e3d_id->vertex_vbo != 0) 
		{		
			ELglDeleteBuffersARB(1, &e3d_id->vertex_vbo);
			e3d_id->vertex_vbo = 0;
		}
		if (e3d_id->indicies_vbo != 0) 
		{		
			ELglDeleteBuffersARB(1, &e3d_id->indicies_vbo);
			e3d_id->indicies_vbo = 0;
		}
	}

	return (e3d_id->cache_ptr->size - sizeof(*e3d_id));
}

void destroy_e3d(e3d_object *e3d_id)
{
	// release the detailed data
	free_e3d_va(e3d_id);
	// free the materials (not free'd in free_e3d_va)
	if(e3d_id->materials) free(e3d_id->materials);
	e3d_id->materials= NULL;
	// and finally free the main object

#ifdef EYE_CANDY
	ec_remove_obstruction_by_e3d_object(e3d_id);
#endif	// EYE_CANDY

	free(e3d_id);
}

// for support of the 1.0.3 server, change if an object is to be displayed or not
void set_3d_object (Uint8 display, const void *ptr, int len)
{
	const Uint32 *id_ptr = ptr;
	
	// first look for the override to process ALL objects
	if (len < sizeof(*id_ptr) ){
		Uint32	i;
		
		for(i=0; i<= highest_obj_3d; i++){
			if (objects_list[i]){
				objects_list[i]->display= display;
			}
		}
	} else {
		int idx = 0;
		
		while (len >= sizeof (*id_ptr))
		{
			Uint32 obj_id = id_ptr[idx];
		
			if (obj_id <= highest_obj_3d && objects_list[obj_id])
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
	if (len < sizeof(*id_ptr) ){
		Uint32	i;
		
		for(i=0; i<= highest_obj_3d; i++){
			if (objects_list[i]){
				objects_list[i]->state= state;
			}
		}
	} else {
		int idx = 0;
	
		while (len >= sizeof(*id_ptr))
		{
			Uint32	obj_id = id_ptr[idx];
		
			if (obj_id <= highest_obj_3d && objects_list[obj_id])
			{
				objects_list[obj_id]->state = state;
				idx++;
				len -= sizeof (*id_ptr);
			}
		}
	}
}
