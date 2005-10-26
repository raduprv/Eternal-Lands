#include <stdlib.h>
#include <math.h>
#include <string.h>
#ifdef MAP_EDITOR2
#include "../map_editor2/global.h"
#else
#include "global.h"
#endif

object3d *objects_list[MAX_OBJ_3D];
#ifndef	NEW_FRUSTUM
struct near_3d_object near_3d_objects[MAX_NEAR_3D_OBJECTS];
struct near_3d_object * first_near_3d_object=NULL;
int no_near_3d_objects=0;
#endif
int highest_obj_3d= 0;

#ifndef	NEW_FRUSTUM
struct near_3d_object near_blended_3d_objects[MAX_NEAR_BLENDED_3D_OBJECTS];
struct near_3d_object * first_near_blended_3d_object=NULL;
int no_near_blended_3d_objects=0;

int regenerate_near_objects=1;
#endif

e3d_object *load_e3d (const char *file_name);
void compute_clouds_map(object3d * object_id);

void draw_3d_object(object3d * object_id)
{
	float x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;
	int i, materials_no;

	e3d_array_vertex *array_vertex;
	e3d_array_normal *array_normal;
	e3d_array_uv_main *array_uv_main;
	e3d_array_uv_detail *clouds_uv;
	e3d_array_order *array_order;

	GLuint *vbo;

	int is_transparent;
	int is_ground;

	//track the usage
	cache_use(cache_e3d, object_id->e3d_data->cache_ptr);
	if(!(SDL_GetAppState()&SDL_APPACTIVE)) return;	// not actually drawing, fake it
	if(!object_id->display) return;	// not currently on the map, ignore it

	// check for having to load the arrays
	if(!object_id->e3d_data->array_vertex || !object_id->e3d_data->array_normal || !object_id->e3d_data->array_uv_main || !object_id->e3d_data->array_order){
		load_e3d_detail(object_id->e3d_data);
	}
	
	array_vertex=object_id->e3d_data->array_vertex;
	array_normal=object_id->e3d_data->array_normal;
	array_uv_main=object_id->e3d_data->array_uv_main;
	array_order=object_id->e3d_data->array_order;
	
	vbo=object_id->e3d_data->vbo;

	is_transparent=object_id->e3d_data->is_transparent;
	is_ground=object_id->e3d_data->is_ground;

	CHECK_GL_ERRORS();
	if(have_multitexture && (clouds_shadows||use_shadow_mapping)){
		if(!object_id->clouds_uv)
			compute_clouds_map(object_id);
	}

	CHECK_GL_ERRORS();
	//also, update the last time this object was used
	object_id->last_acessed_time=cur_time;

	clouds_uv=object_id->clouds_uv;

	//debug

	if(object_id->self_lit && (!is_day || dungeon)) {
		glDisable(GL_LIGHTING);
		glColor3f(object_id->r,object_id->g,object_id->b);
	}

	if(is_transparent) {
		//enable alpha filtering, so we have some alpha key
		glEnable(GL_ALPHA_TEST);
		if(is_ground)glAlphaFunc(GL_GREATER,0.23f);
		else glAlphaFunc(GL_GREATER,0.06f);
		glDisable(GL_CULL_FACE);
	}
	
	CHECK_GL_ERRORS();

	glPushMatrix();//we don't want to affect the rest of the scene
	x_pos=object_id->x_pos;
	y_pos=object_id->y_pos;
	z_pos=object_id->z_pos;
	glTranslatef (x_pos, y_pos, z_pos);

	z_rot=object_id->z_rot;
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	x_rot=object_id->x_rot;
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	y_rot=object_id->y_rot;
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	CHECK_GL_ERRORS();
	
	if(have_multitexture && (clouds_shadows||use_shadow_mapping)){
		ELglClientActiveTextureARB(detail_unit);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if(have_vertex_buffers && object_id->cloud_vbo){
			ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, object_id->cloud_vbo);
			glTexCoordPointer(2, GL_FLOAT, 0, 0);
		} else  glTexCoordPointer(2,GL_FLOAT,0,clouds_uv);
		ELglClientActiveTextureARB(base_unit);
	}
	
	if(have_vertex_buffers && vbo[0] && vbo[2]) {
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo[0]);
		glTexCoordPointer(2,GL_FLOAT,0,0);
				
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo[2]);
		glVertexPointer(3,GL_FLOAT,0,0);
	} else {
		glTexCoordPointer(2,GL_FLOAT,0,array_uv_main);
		
		glVertexPointer(3,GL_FLOAT,0,array_vertex);
	}
			
	if(!is_ground) {
		if(have_vertex_buffers && vbo[1]){
			ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo[1]);
			glNormalPointer(GL_FLOAT,0,0);
		} else 	glNormalPointer(GL_FLOAT,0,array_normal);
	}

	CHECK_GL_ERRORS();
		
	materials_no=object_id->e3d_data->materials_no;
					
	if(have_compiled_vertex_array)ELglLockArraysEXT(0, object_id->e3d_data->face_no);
	for(i=0;i<materials_no;i++) {
		get_and_set_texture_id(array_order[i].texture_id);
#ifdef	DEBUG
		// a quick check for errors
		if(array_order[i].start < 0 || array_order[i].count <= 0) {
			LOG_ERROR("%s[%d] %s (%d, %d)",
				object_id->file_name, i,
				values_str,
				array_order[i].start, array_order[i].count);
		}
#endif	// DEBUG
		glDrawArrays(GL_TRIANGLES,array_order[i].start,array_order[i].count);
	}
			
	if(have_compiled_vertex_array)ELglUnlockArraysEXT();
	
	if(have_multitexture && (clouds_shadows||use_shadow_mapping)){
		ELglClientActiveTextureARB(detail_unit);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		ELglClientActiveTextureARB(base_unit);
	}
	
	glPopMatrix();//restore the scene
	CHECK_GL_ERRORS();

	if(object_id->self_lit && (!is_day || dungeon))glEnable(GL_LIGHTING);
	if(is_transparent) {
		glDisable(GL_ALPHA_TEST);
		glEnable(GL_CULL_FACE);
	}

	if(have_vertex_buffers){
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	}
	
	CHECK_GL_ERRORS();
	
	//OK, let's check if our mouse is over...
#ifdef MAP_EDITOR2
	if (selected_3d_object == -1 && read_mouse_now && mouse_in_sphere(object_id->x_pos, object_id->y_pos, object_id->z_pos, object_id->e3d_data->radius))
		anything_under_the_mouse(object_id->id, UNDER_MOUSE_3D_OBJ);
#else
	if (read_mouse_now && mouse_in_sphere(object_id->x_pos, object_id->y_pos, object_id->z_pos, object_id->e3d_data->radius))
		anything_under_the_mouse(object_id->id, UNDER_MOUSE_3D_OBJ);
#endif
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
	e3d_id = load_e3d (file_name);
	if (e3d_id == NULL) return NULL;
	//and remember it
	e3d_id->cache_ptr = cache_add_item (cache_e3d, e3d_id->file_name, e3d_id, sizeof(*e3d_id));

	return e3d_id;
}

#ifdef	NEW_FRUSTUM
int add_e3d_at_id (int id, const char *file_name, float x_pos, float y_pos, float z_pos, float x_rot, float y_rot, float z_rot, char self_lit, char blended, float r, float g, float b, unsigned int dynamic)
#else
int add_e3d_at_id (int id, const char *file_name, float x_pos, float y_pos, float z_pos, float x_rot, float y_rot, float z_rot, char self_lit, char blended, float r, float g, float b)
#endif
{
	char fname[128];
	e3d_object *returned_e3d;
	object3d *our_object;
	int i;
#ifdef	NEW_FRUSTUM
	float len_x, len_y, len_z;
	AABBOX bbox;
#endif

	if (id < 0 || id >= MAX_OBJ_3D)
	{
		LOG_ERROR ("Invalid object id %d", id);
		return 0;
	}
	
	if (objects_list[id] != NULL)
	{
		LOG_ERROR ("There's already an object with ID %d", id);
		return 0;
	}

	//convert any '\' in '/'
	clean_file_name (fname, file_name, 128);
	my_tolower(fname);

	returned_e3d=load_e3d_cache(fname);
	if(returned_e3d==NULL)
	{
		LOG_ERROR (nasty_error_str, fname);
    		//replace it with the null object, to avoid object IDs corruption
    		returned_e3d = load_e3d_cache ("./3dobjects/misc_objects/badobject.e3d");
    		if (returned_e3d == NULL)
			return 0; // umm, not even found the place holder, this is teh SUCK!!!
	}

	// now, allocate the memory
	our_object = calloc (1, sizeof (object3d));

	// and fill it in
	my_strncp (our_object->file_name, fname, 80);
	our_object->x_pos = x_pos;
	our_object->y_pos = y_pos;
	our_object->z_pos = z_pos;

	our_object->x_rot = x_rot;
	our_object->y_rot = y_rot;
	our_object->z_rot = z_rot;

	our_object->r = r;
	our_object->g = g;
	our_object->b = b;

	our_object->clouds_uv = NULL;
	our_object->cloud_vbo = 0;

	our_object->self_lit = self_lit;
	our_object->blended = blended;
	our_object->display = 1;
	our_object->state = 0;

	our_object->e3d_data = returned_e3d;
	
	our_object->id = id;

	our_object->flags = 0;
	for(i = 0; i < sizeof(harvestable_objects)/sizeof(harvestable_objects[0].name); i++) {
		if(*harvestable_objects[i].name && strstr(file_name, harvestable_objects[i].name) != NULL) {
			our_object->flags |= OBJ_3D_HARVESTABLE;
			break;
		}
	}
	for(i = 0; i < sizeof(entrable_objects)/sizeof(entrable_objects[0].name); i++) {
		if(*entrable_objects[i].name && strstr(file_name, entrable_objects[i].name) != NULL) {
			our_object->flags |= OBJ_3D_ENTRABLE;
			break;
		}
	}
	if(strcasecmp(strrchr(file_name, '/')+1, "bag1.e3d") == 0) {
		our_object->flags |= OBJ_3D_BAG;
	}

	objects_list[id] = our_object;
	// watch the top end
	if(id >= highest_obj_3d)
	{
		highest_obj_3d = id+1;
	}

#ifdef	NEW_FRUSTUM
	len_x = (returned_e3d->max_x - returned_e3d->min_x);
	len_y = (returned_e3d->max_y - returned_e3d->min_y);
	len_z = (returned_e3d->max_z - returned_e3d->min_z);
	bbox.bbmin[X] = -len_x*0.5f;
	bbox.bbmax[X] = len_x*0.5f;
	bbox.bbmin[Y] = -len_y*0.5f;
	bbox.bbmax[Y] = len_y*0.5f;
	bbox.bbmin[Z] = -len_z*0.5f;
	bbox.bbmax[Z] = len_z*0.5f;
	if ((x_rot != 0.0f) || (y_rot != 0.0f) || (z_rot != 0.0f)) rotate_aabb(&bbox, x_rot, y_rot, z_rot);
	bbox.bbmin[X] += x_pos;
	bbox.bbmin[Y] += y_pos;
	bbox.bbmin[Z] += z_pos;
	bbox.bbmax[X] += x_pos;
	bbox.bbmax[Y] += y_pos;
	bbox.bbmax[Z] += z_pos;
	if ((main_bbox_tree_items != NULL) && (dynamic == 0))  add_3dobject_to_list(main_bbox_tree_items, id, &bbox, blended, returned_e3d->is_ground);
	else add_3dobject_to_abt(main_bbox_tree, id, &bbox, blended, returned_e3d->is_ground, dynamic);
#else
	regenerate_near_objects = 1; // We've added an object..
#endif

	return id;
}

#ifdef	NEW_FRUSTUM
int add_e3d (const char * file_name, float x_pos, float y_pos, float z_pos, float x_rot, float y_rot, float z_rot, char self_lit, char blended, float r, float g, float b, unsigned int dynamic)
#else
int add_e3d (const char * file_name, float x_pos, float y_pos, float z_pos, float x_rot, float y_rot, float z_rot, char self_lit, char blended, float r, float g, float b)
#endif
{
	int i;
	
	//find a free spot, in the e3d_list
	for(i = 0; i < MAX_OBJ_3D; i++)
	{
		if(objects_list[i] == NULL)
			break;
	}
	
#ifdef	NEW_FRUSTUM
	return add_e3d_at_id (i, file_name, x_pos, y_pos, z_pos, x_rot, y_rot, z_rot, self_lit, blended, r, g, b, dynamic);
#else
	return add_e3d_at_id (i, file_name, x_pos, y_pos, z_pos, x_rot, y_rot, z_rot, self_lit, blended, r, g, b);
#endif
}

#ifndef	NEW_FRUSTUM
void add_near_3d_object(int dist, float radius, int pos, int blended )//Blended objects needs to be in their own list
{
	struct near_3d_object *cur, *nearest, *list, **first;
	int * no;
     
	if(!blended){
		if(no_near_3d_objects >= MAX_NEAR_3D_OBJECTS)
			return;
		list=near_3d_objects;
		no=&no_near_3d_objects;
		first=&first_near_3d_object;
	} else {
		if(no_near_blended_3d_objects >= MAX_NEAR_BLENDED_3D_OBJECTS)
			return;
		list=near_blended_3d_objects;
		no=&no_near_blended_3d_objects;
		first=&first_near_blended_3d_object;
	}
     
	cur=&list[*no];
	cur->pos=pos;
	cur->dist=dist;
	cur->radius=radius;
	cur->next=NULL;
	(*no)++;

	list[*no].dist=0;
	list[*no].radius=0;
	list[*no].pos=0;
	list[*no].next=NULL;

	if(!*first) {
		*first=cur;
		return;
	} else if((*first)->dist>dist){
		cur->next=*first;
		*first=cur;

		return;
	}

	for(nearest=*first;nearest;nearest=nearest->next){
		if(nearest->next && nearest->dist<=dist && nearest->next->dist>=dist){
			cur->next=nearest->next;
			nearest->next=cur;
			return;
		} else if(!nearest->next){
			nearest->next=cur;
			return;
		}
	}
}
#endif

#ifndef	NEW_FRUSTUM
int get_near_3d_objects()
{
	int sx, sy, ex, ey;
	float x, y;
	int i, j, k;

#ifdef MAP_EDITOR2
	no_near_3d_objects = 0;
	no_near_blended_3d_objects = 0;
	first_near_3d_object = NULL;
	first_near_blended_3d_object = NULL;
     
	x = -cx;
	y = -cy;
     
	get_supersector (SECTOR_GET (global_x_pos, global_y_pos), &sx, &sy, &ex, &ey);
#else
	actor *xxx = pf_get_our_actor ();

	if (xxx == NULL) return 0;

	no_near_3d_objects = 0;
	no_near_blended_3d_objects = 0;
	first_near_3d_object = NULL;
	first_near_blended_3d_object = NULL;
     
	x = -cx;
	y = -cy;
     
	get_supersector (SECTOR_GET (xxx->x_pos, xxx->y_pos), &sx, &sy, &ex, &ey);
#endif
	for (i = sx; i <= ex; i++)
	{
		for (j = sy; j <= ey; j++)
		{
			for (k = 0; k < MAX_3D_OBJECTS; k++)
			{
				object3d *object_id;
				int l = sectors[(j*(tile_map_size_x>>2))+i].e3d_local[k];
				if (l == -1) break;
				object_id = objects_list[l];

				if (object_id != NULL && object_id->blended != 20)
				{
					float dist1, dist2;
					float dist;

					dist1 = x - object_id->x_pos;
					dist2 = y - object_id->y_pos;
					dist = dist1*dist1 + dist2*dist2;
					if (dist <= 35*35)
					{
						float x_len, y_len, z_len;
						float radius;

						z_len= object_id->e3d_data->max_z-object_id->e3d_data->min_z;
						x_len= object_id->e3d_data->max_x-object_id->e3d_data->min_x;
						y_len= object_id->e3d_data->max_y-object_id->e3d_data->min_y;
						//do some checks, to see if we really have to display this object
						radius = x_len / 2;
						if (radius < y_len/2) radius = y_len / 2;
						if (radius < z_len) radius = z_len;
						//not in the middle of the air
						if (SphereInFrustum (object_id->x_pos, object_id->y_pos, object_id->z_pos, radius))
						{
							add_near_3d_object (dist, radius, l, object_id->blended);
						}
					}
				}
			}
		}
	}
		
	regenerate_near_objects = 0;
	return 1;
}
#endif

void display_objects()
{
#ifndef	NEW_FRUSTUM
	struct near_3d_object * nobj;
	
	if(regenerate_near_objects||!first_near_3d_object)
		if(!get_near_3d_objects())return;
#else
	unsigned int i, l;

	check_and_update_intersect_list(main_bbox_tree);
#endif
	
	CHECK_GL_ERRORS();
	glEnable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
			
	if(have_multitexture && clouds_shadows){
		//bind the detail texture
		ELglActiveTextureARB(detail_unit);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, get_texture_id(ground_detail_text));
		ELglActiveTextureARB(base_unit);
		glEnable(GL_TEXTURE_2D);
	}

	CHECK_GL_ERRORS();

#ifndef	NEW_FRUSTUM
	for(nobj=first_near_3d_object;nobj;nobj=nobj->next){
		if(!objects_list[nobj->pos])
			regenerate_near_objects=1;
		else if(!objects_list[nobj->pos]->e3d_data->is_ground)
			draw_3d_object(objects_list[nobj->pos]);
	}
#else
	for (i = get_intersect_start(main_bbox_tree, TYPE_3D_NO_BLEND_NO_GROUND_OBJECT); i < get_intersect_stop(main_bbox_tree, TYPE_3D_NO_BLEND_NO_GROUND_OBJECT); i++)
	{
		l = get_intersect_item_ID(main_bbox_tree, i);
#ifdef EXTRA_DEBUG
		if (!objects_list[l])
		{
			ERR();
			continue;
		}
#endif
		draw_3d_object(objects_list[l]);
	}
#endif
	
	glDisableClientState(GL_NORMAL_ARRAY);

	glNormal3f(0,0,1);
	
#ifndef	NEW_FRUSTUM
	for(nobj=first_near_3d_object;nobj;nobj=nobj->next){
		if(!objects_list[nobj->pos])
			regenerate_near_objects=1;
		else if(objects_list[nobj->pos]->e3d_data->is_ground)
			draw_3d_object(objects_list[nobj->pos]);
	}
#else
	for (i = get_intersect_start(main_bbox_tree, TYPE_3D_NO_BLEND_GROUND_OBJECT); i < get_intersect_stop(main_bbox_tree, TYPE_3D_NO_BLEND_GROUND_OBJECT); i++)
	{
		l = get_intersect_item_ID(main_bbox_tree, i);
#ifdef EXTRA_DEBUG
		if (!objects_list[l])
		{
			ERR();
			continue;
		}
#endif
		draw_3d_object(objects_list[l]);
	}
#endif
	
	CHECK_GL_ERRORS();
	glDisable(GL_CULL_FACE);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	if(have_multitexture && clouds_shadows)
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
#ifndef	NEW_FRUSTUM
	struct near_3d_object * nobj;
	
	if(regenerate_near_objects)if(!get_near_3d_objects())return;
	
	if(!no_near_blended_3d_objects)return;
#else
	unsigned int i, l;

	check_and_update_intersect_list(main_bbox_tree);
#endif
	
	CHECK_GL_ERRORS();
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_ONE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
			
	if(have_multitexture && clouds_shadows){
		//bind the detail texture
		ELglActiveTextureARB(detail_unit);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, get_texture_id(ground_detail_text));
		ELglActiveTextureARB(base_unit);
		glEnable(GL_TEXTURE_2D);
	}

	CHECK_GL_ERRORS();

#ifndef	NEW_FRUSTUM
	for(nobj=first_near_blended_3d_object;nobj;nobj=nobj->next){
		if(!objects_list[nobj->pos])
			regenerate_near_objects=1;
		else if(!objects_list[nobj->pos]->e3d_data->is_ground)
			draw_3d_object(objects_list[nobj->pos]);
	}
#else
	for (i = get_intersect_start(main_bbox_tree, TYPE_3D_BLEND_NO_GROUND_OBJECT); i < get_intersect_stop(main_bbox_tree, TYPE_3D_BLEND_NO_GROUND_OBJECT); i++)
	{
		l = get_intersect_item_ID(main_bbox_tree, i);
#ifdef EXTRA_DEBUG
		if (!objects_list[l])
		{
			ERR();
			continue;
		}
#endif
		draw_3d_object(objects_list[l]);
	}
#endif
	
	glDisableClientState(GL_NORMAL_ARRAY);

	glNormal3f(0,0,1);
	
#ifndef	NEW_FRUSTUM
	for(nobj=first_near_blended_3d_object;nobj;nobj=nobj->next){
		if(!objects_list[nobj->pos])
			regenerate_near_objects=1;
		else if(objects_list[nobj->pos]->e3d_data->is_ground)
			draw_3d_object(objects_list[nobj->pos]);
	}
#else
	for (i = get_intersect_start(main_bbox_tree, TYPE_3D_BLEND_GROUND_OBJECT); i < get_intersect_stop(main_bbox_tree, TYPE_3D_BLEND_GROUND_OBJECT); i++)
	{
		l = get_intersect_item_ID(main_bbox_tree, i);
#ifdef EXTRA_DEBUG
		if (!objects_list[l])
		{
			ERR();
			continue;
		}
#endif
		draw_3d_object(objects_list[l]);
	}
#endif
	
	CHECK_GL_ERRORS();
	glDisable(GL_CULL_FACE);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_BLEND);
	if(have_multitexture && clouds_shadows)
		{
			//disable the second texture unit
			ELglActiveTextureARB(detail_unit);
			glDisable(GL_TEXTURE_2D);
			ELglActiveTextureARB(base_unit);
		}
	CHECK_GL_ERRORS();
}

e3d_object *load_e3d (const char *file_name)
{
	int vertex_no,faces_no,materials_no;
	FILE *f = NULL;
	e3d_object *cur_object;
	//int transparency=0; unused?
	e3d_header our_header;
	char *our_header_pointer=(char *)&our_header;

	f = my_fopen(file_name, "rb");
	if(!f) return NULL;

	//load and parse the header
	fread(our_header_pointer, 1, sizeof(e3d_header), f);
	fclose (f);

	faces_no=SDL_SwapLE32(our_header.face_no);
	vertex_no=SDL_SwapLE32(our_header.vertex_no);
	materials_no=SDL_SwapLE32(our_header.material_no);

	// allocate the memory
	cur_object=calloc(1, sizeof(e3d_object));
	// and fill in the data
	my_strncp(cur_object->file_name, file_name, 128);
	
#ifdef EL_BIG_ENDIAN
	cur_object->min_x=SwapFloat(our_header.min_x);
	cur_object->min_y=SwapFloat(our_header.min_y);
	cur_object->min_z=SwapFloat(our_header.min_z);
	cur_object->max_x=SwapFloat(our_header.max_x);
	cur_object->max_y=SwapFloat(our_header.max_y);
	cur_object->max_z=SwapFloat(our_header.max_z);
#else
	cur_object->min_x=our_header.min_x;
	cur_object->min_y=our_header.min_y;
	cur_object->min_z=our_header.min_z;
	cur_object->max_x=our_header.max_x;
	cur_object->max_y=our_header.max_y;
	cur_object->max_z=our_header.max_z;
#endif
	
	cur_object->is_transparent=our_header.is_transparent;
	cur_object->is_ground=our_header.is_ground;
	cur_object->face_no=faces_no;
	cur_object->materials_no=materials_no;

	cur_object->array_order=NULL;
	cur_object->array_vertex=NULL;
	cur_object->array_normal=NULL;
	cur_object->array_uv_main=NULL;

	cur_object->vbo[0]=
	cur_object->vbo[1]=
	cur_object->vbo[2]=0;

	return cur_object;
}

e3d_object * load_e3d_detail(e3d_object *cur_object)
{
	int vertex_no,faces_no,materials_no;
	int i,l;
	FILE *f = NULL;
	e3d_vertex *vertex_list;
	e3d_face *face_list;
	e3d_material *material_list;
	char cur_dir[200]={0};
	//int transparency=0; unused?
	e3d_header our_header;
	char *our_header_pointer=(char *)&our_header;
	e3d_array_vertex *array_vertex;
	e3d_array_normal *array_normal;
	e3d_array_uv_main *array_uv_main;
	e3d_array_order *array_order;
	int	mem=0;
	float radius, x_len, y_len, z_len;

	//get the current directory
	l=strlen(cur_object->file_name);
	//parse the string backwards, until we find a /
	while(l>0)
		{
			if(cur_object->file_name[l]=='/' || cur_object->file_name[l]=='\\')break;
			l--;
		}

	i=0;
	if(l)//prevent invalid dir names
		{
			while(l>=0)
				{
					cur_dir[i]=cur_object->file_name[i];
					i++;
					l--;
				}
			cur_dir[i+1]=0;
		}

	f = my_fopen(cur_object->file_name, "rb");
	if(!f) return NULL;

	//load and parse the header
	fread(our_header_pointer, 1, sizeof(e3d_header), f);
	faces_no=SDL_SwapLE32(our_header.face_no);	// or should we grab from the cur_object?
	vertex_no=SDL_SwapLE32(our_header.vertex_no);
	materials_no=SDL_SwapLE32(our_header.material_no);

	//read the rest of the file (vertex,faces, materials)
	face_list=calloc(faces_no, SDL_SwapLE32(our_header.face_size));
	fread(face_list, faces_no, SDL_SwapLE32(our_header.face_size), f);

	vertex_list=calloc(vertex_no, SDL_SwapLE32(our_header.vertex_size));
  	if(!vertex_list)
		{
			char str[200];
			snprintf(str,sizeof(str),"%s: %s: %s",reg_error_str,corrupted_object,cur_object->file_name);
			LOG_TO_CONSOLE(c_red2,str);
			free(face_list);
			face_list = NULL;
			fclose(f);
			return NULL;
		}
	fread(vertex_list, vertex_no, SDL_SwapLE32(our_header.vertex_size), f);

	material_list=calloc(materials_no, SDL_SwapLE32(our_header.material_size));
	fread(material_list, materials_no, SDL_SwapLE32(our_header.material_size), f);

	fclose (f);
	
#ifdef EL_BIG_ENDIAN
	for(i = 0; i < vertex_no; i++)
	{
		vertex_list[i].x = SwapFloat(vertex_list[i].x);
		vertex_list[i].y = SwapFloat(vertex_list[i].y);
		vertex_list[i].z = SwapFloat(vertex_list[i].z);
		vertex_list[i].nx = SwapFloat(vertex_list[i].nx);
		vertex_list[i].ny = SwapFloat(vertex_list[i].ny);
		vertex_list[i].nz = SwapFloat(vertex_list[i].nz);
	}
		
	for(i = 0; i < faces_no; i++)
	{
		face_list[i].au = SwapFloat(face_list[i].au);
		face_list[i].av = SwapFloat(face_list[i].av);
		face_list[i].bu = SwapFloat(face_list[i].bu);
		face_list[i].bv = SwapFloat(face_list[i].bv);
		face_list[i].cu = SwapFloat(face_list[i].cu);
		face_list[i].cv = SwapFloat(face_list[i].cv);
		face_list[i].a = SDL_SwapLE32(face_list[i].a);
		face_list[i].b = SDL_SwapLE32(face_list[i].b);
		face_list[i].c = SDL_SwapLE32(face_list[i].c);
		face_list[i].material = SDL_SwapLE32(face_list[i].material);
	}
#endif
	
	//now, load all the materials, and use the material ID (which isn't used now) to
	//temporary store the texture_ids

	for(i=0;i<materials_no;i++)
		{
			char text_file_name[500];
			snprintf(text_file_name, sizeof(text_file_name), "%s%s", cur_dir, material_list[i].material_name);
/* // FIXME: Why is this commented out?
			if(cur_object->is_transparent)material_list[i].material_id=load_texture_cache(text_file_name,0);
			else material_list[i].material_id=load_texture_cache(text_file_name,255);
*/
			material_list[i].material_id=load_texture_cache(text_file_name,0);
		}

	//assign the proper texture to each face
	for(i=0;i<faces_no;i++)
		{
			face_list[i].material=material_list[face_list[i].material].material_id;
		}

	//allocate memory for our new, converted structures
	//WARNING: this can cause a memory leak if memory was already allocated!
	array_order=calloc(materials_no, sizeof(e3d_array_order));
	array_vertex=calloc(faces_no*3, sizeof(e3d_array_vertex));
	array_normal=calloc(faces_no*3, sizeof(e3d_array_normal));
	array_uv_main=calloc(faces_no*3, sizeof(e3d_array_uv_main));
	mem+=(materials_no*sizeof(e3d_array_order))+
		(faces_no*3*(sizeof(e3d_array_vertex)+sizeof(e3d_array_normal)+sizeof(e3d_array_uv_main)));

	//ok, now do the reconversion, into our vertex arrays...
	{
		int cur_index_array=0;
		for(i=0;i<materials_no;i++)
			{
				int	k;
				int size=0;
				int start=-1;
				int cur_mat=material_list[i].material_id;
				//some horses put two materials with the same name
				//check to see if this si the case, and if it is, skip it
				for(l=0;l<materials_no;l++)
					{
						if(material_list[l].material_id==cur_mat && i!=l)
							{
								char str[200];
								size=0;
								start=0;
								snprintf(str,sizeof(str),"%s: %s . %s",bad_object,cur_object->file_name,multiple_material_same_texture);
								LOG_TO_CONSOLE(c_red2,str);
								goto skip_this_mat;
							}
					}

				for(k=0;k<faces_no;k++)
					{
						//we need to put the faces with the same material in order
						if(face_list[k].material==cur_mat)
							{
								if(start==-1)start=cur_index_array;
								array_vertex[cur_index_array].x=vertex_list[face_list[k].a].x;
								array_vertex[cur_index_array].y=vertex_list[face_list[k].a].y;
								array_vertex[cur_index_array].z=vertex_list[face_list[k].a].z;

								array_normal[cur_index_array].nx=vertex_list[face_list[k].a].nx;
								array_normal[cur_index_array].ny=vertex_list[face_list[k].a].ny;
								array_normal[cur_index_array].nz=vertex_list[face_list[k].a].nz;

								array_uv_main[cur_index_array].u=face_list[k].au;
								array_uv_main[cur_index_array].v=face_list[k].av;
								cur_index_array++;

								array_vertex[cur_index_array].x=vertex_list[face_list[k].b].x;
								array_vertex[cur_index_array].y=vertex_list[face_list[k].b].y;
								array_vertex[cur_index_array].z=vertex_list[face_list[k].b].z;

								array_normal[cur_index_array].nx=vertex_list[face_list[k].b].nx;
								array_normal[cur_index_array].ny=vertex_list[face_list[k].b].ny;
								array_normal[cur_index_array].nz=vertex_list[face_list[k].b].nz;

								array_uv_main[cur_index_array].u=face_list[k].bu;
								array_uv_main[cur_index_array].v=face_list[k].bv;
								cur_index_array++;

								array_vertex[cur_index_array].x=vertex_list[face_list[k].c].x;
								array_vertex[cur_index_array].y=vertex_list[face_list[k].c].y;
								array_vertex[cur_index_array].z=vertex_list[face_list[k].c].z;

								array_normal[cur_index_array].nx=vertex_list[face_list[k].c].nx;
								array_normal[cur_index_array].ny=vertex_list[face_list[k].c].ny;
								array_normal[cur_index_array].nz=vertex_list[face_list[k].c].nz;

								array_uv_main[cur_index_array].u=face_list[k].cu;
								array_uv_main[cur_index_array].v=face_list[k].cv;
								cur_index_array++;

								size+=3;
							}
					}
			skip_this_mat:
				//excellent, we are done with this material
				array_order[i].count=size;
				array_order[i].start=start;
				array_order[i].texture_id=cur_mat;
			}
	}

	//and memorize the stored arrays
	cur_object->array_order=array_order;
	if(have_vertex_buffers){
		//Generate the buffers
		ELglGenBuffersARB(3, cur_object->vbo);
		
		if(cur_object->vbo[0] && cur_object->vbo[1] && cur_object->vbo[2]){
			ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, cur_object->vbo[0]);
			ELglBufferDataARB(GL_ARRAY_BUFFER_ARB, faces_no*3*sizeof(e3d_array_uv_main), array_uv_main, GL_STATIC_DRAW_ARB);
		
			ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, cur_object->vbo[1]);
			ELglBufferDataARB(GL_ARRAY_BUFFER_ARB, faces_no*3*sizeof(e3d_array_normal), array_normal, GL_STATIC_DRAW_ARB);
		
			ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, cur_object->vbo[2]);
			ELglBufferDataARB(GL_ARRAY_BUFFER_ARB, faces_no*3*sizeof(e3d_array_vertex), array_vertex, GL_STATIC_DRAW_ARB);
		
			ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		} else {
			if(cur_object->vbo[0])ELglDeleteBuffersARB(1,&cur_object->vbo[0]);
			if(cur_object->vbo[1])ELglDeleteBuffersARB(1,&cur_object->vbo[1]);
			if(cur_object->vbo[2])ELglDeleteBuffersARB(1,&cur_object->vbo[2]);
			cur_object->vbo[0]=
			cur_object->vbo[1]=
			cur_object->vbo[2]=0;

			log_error("We could not create all 3 vertex buffers! This is a major bug, so report it to the developers!");
		}
	}
	
	cur_object->array_vertex=array_vertex;
	cur_object->array_normal=array_normal;
	cur_object->array_uv_main=array_uv_main;

	//release temporary memory
	free(material_list);
	free(vertex_list);
	free(face_list);

	z_len= cur_object->max_z-cur_object->min_z;
	x_len= cur_object->max_x-cur_object->min_x;
	y_len= cur_object->max_y-cur_object->min_y;
	//do some checks, to see if we really have to display this object
	radius=x_len/2;
	if(radius<y_len/2)radius=y_len/2;
	if(radius<z_len)radius=z_len;

	cur_object->radius=radius;
	
	cache_adj_size(cache_e3d, mem, cur_object);

	return cur_object;
}

void compute_clouds_map(object3d * object_id)
{
	//float x1,y1,x,y,z,m;
	float m;
	float cos_m,sin_m;
	float x_pos,y_pos;	//,z_pos;
	float z_rot	;//x_rot,y_rot,z_rot;
	int i,face_no;

	e3d_array_vertex *array_vertex;
	e3d_array_uv_detail *array_detail;

	if(!object_id->e3d_data->array_vertex)
		{
			load_e3d_detail(object_id->e3d_data);
		}
	array_vertex=object_id->e3d_data->array_vertex;
	face_no=object_id->e3d_data->face_no;
	array_detail=calloc(face_no*3,sizeof(e3d_array_uv_detail));

	x_pos=object_id->x_pos;
	y_pos=object_id->y_pos;
	//z_pos=object_id->z_pos;

	//x_rot=object_id->x_rot;
	//y_rot=object_id->y_rot;
	z_rot=object_id->z_rot;

	m = -z_rot * M_PI / 180.0f;
	cos_m = cos(m);
	sin_m = sin(m);

	for(i=0;i<face_no*3;i++)
		{
			float x=array_vertex[i].x;
			float y=array_vertex[i].y;
			float z=array_vertex[i].z;
			float x1=x*cos_m+y*sin_m;
			float y1=y*cos_m-x*sin_m;
			x1=x_pos+x1;
			y1=y_pos+y1;

			array_detail[i].u=(x1+z)/texture_scale+clouds_movement_u;
			array_detail[i].v=(y1+z)/texture_scale+clouds_movement_v;
		}

	object_id->clouds_uv=array_detail;

	if(have_vertex_buffers){
		ELglGenBuffersARB(1, &object_id->cloud_vbo);
		
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, object_id->cloud_vbo);
		ELglBufferDataARB(GL_ARRAY_BUFFER_ARB, face_no*3*sizeof(e3d_array_uv_detail), array_detail, GL_STATIC_DRAW_ARB);
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	}

}

void destroy_clouds_cache(object3d * obj)
{
	if(have_vertex_buffers){
		const GLuint l=obj->cloud_vbo;
							
		ELglDeleteBuffersARB(1, &l);
		obj->cloud_vbo=0;
	}
	if(obj->clouds_uv != NULL) {
		free(obj->clouds_uv);
		obj->clouds_uv = NULL;
	}
}

void clear_clouds_cache()
{
	int i;

	last_clear_clouds=cur_time;
	for(i=0;i<highest_obj_3d;i++) {
		if(objects_list[i]){
			if(objects_list[i]->clouds_uv && objects_list[i]->last_acessed_time+20000<cur_time) {
				destroy_clouds_cache(objects_list[i]);
			}
		}
	}
}

void destroy_3d_object(int i)
{
#ifdef	NEW_FRUSTUM
	if ((i < 0) || (i >= MAX_OBJ_3D)) return;
#endif
	if (objects_list[i] == NULL) return;
	destroy_clouds_cache(objects_list[i]);
#ifdef	NEW_FRUSTUM
	delete_3dobject_from_abt(main_bbox_tree, i, objects_list[i]->blended, objects_list[i]->e3d_data->is_ground);
#endif
	free(objects_list[i]);
	objects_list[i] = NULL;
#ifndef	NEW_FRUSTUM
	regenerate_near_objects = 1;
#endif
	if(i == highest_obj_3d+1)
		highest_obj_3d = i;
}

Uint32 free_e3d_va(e3d_object *e3d_id)
{
#ifdef	NEW_FRUSTUM
	set_all_intersect_update_needed(main_bbox_tree);
#else
	regenerate_near_objects = 1;
#endif

	if(e3d_id->array_vertex) {
		free(e3d_id->array_vertex);
		e3d_id->array_vertex=NULL;
	}

	if(e3d_id->array_normal) {
		free(e3d_id->array_normal);
		e3d_id->array_normal=NULL;
	}

	if(e3d_id->array_uv_main) {
		free(e3d_id->array_uv_main);
		e3d_id->array_uv_main=NULL;
	}

	if(e3d_id->array_order) {
		free(e3d_id->array_order);
		e3d_id->array_order=NULL;
	}

	if(have_vertex_buffers){
		const GLuint buf[3]={e3d_id->vbo[0], e3d_id->vbo[1], e3d_id->vbo[2]};
		
		ELglDeleteBuffersARB(3, buf);

		e3d_id->vbo[0]=
		e3d_id->vbo[1]=
		e3d_id->vbo[2]=0;
	}
	
	return(e3d_id->cache_ptr->size - sizeof(*e3d_id));
}

void destroy_e3d(e3d_object *e3d_id)
{
	// release the detailed data
	free_e3d_va(e3d_id);
	// and finally free the main object
	free(e3d_id);
}

// for support of the 1.0.3 server, change if an object is to be displayed or not
void set_3d_object (Uint8 display, const void *ptr, int len)
{
	const Uint32 *id_ptr = ptr;
	
	// first look for the override to process ALL objects
	if (len < sizeof(*id_ptr) ){
		int	i;
		
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
		int	i;
		
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
