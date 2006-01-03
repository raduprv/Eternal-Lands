#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef MAP_EDITOR2
#include "../map_editor2/global.h"
#else
#include "global.h"
#endif

#ifdef OSX
#define GL_EXT_texture_env_combine 1
#endif

// TODO: pbuffers according to Mesa/progs/xdemos/glxpbdemo.c

float proj_on_ground[16];
float ground_plane[4]={0,0,1,0};
float sun_position[4]={400.0, 400.0, 500.0, 0.0};
double light_view_mat[16],light_proj_mat[16],texgen_mat[16],texgen_mat_1d[16];
int shadows_on=0;
int is_day;
int shadows_texture;
int use_shadow_mapping=1;

//TODO: Would like to use TEXTURE_RECTANGLE for cards that support it, but for some reason it doesn't work??
GLenum depth_texture_target=GL_TEXTURE_2D;
int max_shadow_map_size;
GLsizei depth_map_width;
GLsizei depth_map_height;
GLuint depth_map_id=0;
/* Good values:
#define depth_map_scale 15.0
#define light_view_near -30.0
#define light_view_far 6.0*/

GLfloat light_view_hscale=13.0;
GLfloat light_view_top=10.0;
GLfloat light_view_bottom=-10.0;
GLfloat light_view_near=-30.0;
GLfloat light_view_far=6.0;

#ifdef  DEBUG
extern int e3d_count, e3d_total;    // LRNR:stats testing only
extern int cur_e3d_count;
#endif
extern e3d_object   *cur_e3d;

int floor_pow2(int n)
{
	int ret=1;
	if(n<1)return 0;
	while(ret*2<=n)ret*=2;
	return ret;
}

void set_shadow_map_size()
{
	int max=floor_pow2(max_shadow_map_size);

	if (have_texture_non_power_of_two)
	{
		depth_map_width = window_width;
		depth_map_height = window_height;
	}
	else
	{
		depth_map_width=floor_pow2(window_width);
		depth_map_height=floor_pow2(window_height);
	}
	
	if (depth_map_width > max) depth_map_width = max;
	if (depth_map_height > max) depth_map_height = max;
}

void calc_light_frustum(float light_xrot)
{
	float window_ratio=(GLfloat)window_width/(GLfloat)window_height;
	float max_height=20.0; //TODO: Really calculate this from positions and heights of objects
	float x,y;
	float slight, clight;

	light_xrot=-light_xrot;
	clight=cos(light_xrot);
	slight=sin(light_xrot);
	//TODO: Optimize this function a bit.
	//Assuming a max zoom_level of 3.75 and near/far distances of 20.0, we'll set the hscale to the radius of a circle that
	//can just contain the view frustum of the player. To simplify things, we'll assume the view frustum is horizontal.
	//light_view_hscale=sqrt(window_ratio*window_ratio*3.75f*3.75f+12.0f*12.0f);
	light_view_hscale=sqrt(window_ratio*window_ratio+14.0f*14.0f);
	// For the others, we can just use the parametric ellipse formula to find the value for this angle
	x=light_view_hscale*slight;
	y=max_height*clight;
	light_view_top=sqrt(x*x+y*y);
	y=3.75f*clight;
	light_view_far=sqrt(x*x+y*y);
	x=light_view_hscale*clight;
	y=3.75f*slight;
	light_view_bottom=-sqrt(x*x+y*y);
	x=100.0f*slight;  // A bit better than the real value (infinity)
	y=max_height*clight;
	light_view_near=-sqrt(x*x+y*y);
}

void calc_shadow_matrix()
{
	if(use_shadow_mapping)
		{
#ifndef USE_LISPSM
			float xrot,zrot;

			double scale1d[]={0.0,0.0,0.0,0.0,
					  0.0,0.0,0.0,0.0,
					  0.5,0.0,0.0,0.0,
					  0.5,0.0,0.0,1.0};

			float div_length=1.0f/sqrt(sun_position[0]*sun_position[0]+sun_position[1]*sun_position[1]+sun_position[2]*sun_position[2]);
			sun_position[0]*=div_length;
			sun_position[1]*=div_length;
			sun_position[2]*=div_length;
			// Grumble, Old version of OS X don't have *f trig functions but I'm compiling on a version so I can't just #define my way out
#ifdef OSX
			xrot=-acos(sun_position[2]);
#else
			xrot=-acosf(sun_position[2]);
#endif
			//xrot=-atan2f(sun_position[2],sun_position[0])*180.0f/(float)M_PI;
#ifdef OSX
			zrot=-90.0f-atan2(sun_position[1],sun_position[0])*180.0f/(float)M_PI;
#else
			zrot=-90.0f-atan2f(sun_position[1],sun_position[0])*180.0f/(float)M_PI;
#endif

			glPushMatrix();
			glLoadIdentity();
			calc_light_frustum(xrot);
			xrot*=180.0f/(float)M_PI;
			glOrtho(-light_view_hscale,light_view_hscale,
				light_view_bottom,light_view_top,light_view_near,light_view_far);
			glGetDoublev(GL_MODELVIEW_MATRIX,light_proj_mat);
			glLoadIdentity();
			glRotatef(xrot,1.0f,0.0f,0.0f);
			glRotatef(zrot,0.0f,0.0f,1.0f);
			glGetDoublev(GL_MODELVIEW_MATRIX,light_view_mat);
			glLoadIdentity();
			if(depth_texture_target!=GL_TEXTURE_2D)glScalef(depth_map_width,depth_map_height,0);
			glTranslatef(0.5,0.5,0.5);   // This...
			glScalef(0.5,0.5,0.5);       // ...and this == S
			glMultMatrixd(light_proj_mat);     // Plight
			glMultMatrixd(light_view_mat);     // L^-1
			glGetDoublev(GL_MODELVIEW_MATRIX,texgen_mat);
			glLoadMatrixd(scale1d);      // S
			glMultMatrixd(light_proj_mat);     // Plight
			glMultMatrixd(light_view_mat);     // L^-1
			glGetDoublev(GL_MODELVIEW_MATRIX,texgen_mat_1d);
			glPopMatrix();
#endif
		}
	else
		{
			float dot;

			// dot product of plane and light position
			dot = ground_plane[0] * sun_position[0]
			  + ground_plane[1] * sun_position[1]
			  + ground_plane[2] * sun_position[2]
			  + ground_plane[3] * sun_position[3];

			// first column
			proj_on_ground[0] = dot - sun_position[0] * ground_plane[0];
			proj_on_ground[4] = 0.0f - sun_position[0] * ground_plane[1];
			proj_on_ground[8] = 0.0f - sun_position[0] * ground_plane[2];
			proj_on_ground[12] = 0.0f - sun_position[0] * ground_plane[3];

			// second column
			proj_on_ground[1] = 0.0f - sun_position[1] * ground_plane[0];
			proj_on_ground[5] = dot - sun_position[1] * ground_plane[1];
			proj_on_ground[9] = 0.0f - sun_position[1] * ground_plane[2];
			proj_on_ground[13] = 0.0f - sun_position[1] * ground_plane[3];

			// third column
			proj_on_ground[2] = 0.0f - sun_position[2] * ground_plane[0];
			proj_on_ground[6] = 0.0f - sun_position[2] * ground_plane[1];
			proj_on_ground[10] = dot - sun_position[2] * ground_plane[2];
			proj_on_ground[14] = 0.0f - sun_position[2] * ground_plane[3];

			// fourth column
			proj_on_ground[3] = 0.0f - sun_position[3] * ground_plane[0];
			proj_on_ground[7] = 0.0f - sun_position[3] * ground_plane[1];
			proj_on_ground[11] = 0.0f - sun_position[3] * ground_plane[2];
			proj_on_ground[15] = dot - sun_position[3] * ground_plane[3];
		}
}

void draw_3d_object_shadow_detail(object3d * object_id)
{
#ifndef	NEW_FRUSTUM
	float x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;
#endif

	int materials_no;
	int i;
	char is_transparent;

	e3d_array_vertex *array_vertex;
	e3d_array_uv_main *array_uv_main;
	e3d_array_order *array_order;

	if(!object_id->display) return;	// not currently on the map, ignore it
    if(object_id->blended)return;//blended objects can't have shadows
    //if(object_id->self_lit)return;//light sources can't have shadows
    if(object_id->e3d_data->min_z>=object_id->e3d_data->max_z)return;//we have a flat object
	//track the usage
	cache_use(cache_e3d, object_id->e3d_data->cache_ptr);

	// check for having to load the arrays
	if(!object_id->e3d_data->array_vertex || !object_id->e3d_data->array_normal || !object_id->e3d_data->array_uv_main || !object_id->e3d_data->array_order)
		{
			load_e3d_detail(object_id->e3d_data);
		}

	array_vertex=object_id->e3d_data->array_vertex;
	array_uv_main=object_id->e3d_data->array_uv_main;
	array_order=object_id->e3d_data->array_order;

	is_transparent=object_id->e3d_data->is_transparent;
	materials_no=object_id->e3d_data->materials_no;

	glPushMatrix();//we don't want to affect the rest of the scene
	if(!use_shadow_mapping)glMultMatrixf(proj_on_ground);
#ifdef	NEW_FRUSTUM
	glMultMatrixf(object_id->matrix);
#else
	x_pos=object_id->x_pos;
	y_pos=object_id->y_pos;
	z_pos=object_id->z_pos;
	glTranslatef (x_pos, y_pos, z_pos);

	x_rot=object_id->x_rot;
	y_rot=object_id->y_rot;
	z_rot=object_id->z_rot;
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);
#endif

	// watch for a change
	if(object_id->e3d_data != cur_e3d){
		if(cur_e3d != NULL){
           	if(have_compiled_vertex_array)ELglUnlockArraysEXT();
		}
		if(have_vertex_buffers && object_id->e3d_data->vbo[2]){
			ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, object_id->e3d_data->vbo[2]);
			glVertexPointer(3,GL_FLOAT,0,0);
		} else  glVertexPointer(3,GL_FLOAT,0,array_vertex);

		if(is_transparent)
			{
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				if(have_vertex_buffers && object_id->e3d_data->vbo[0]){
					ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, object_id->e3d_data->vbo[0]);
					glTexCoordPointer(2,GL_FLOAT,0,0);
				} else glTexCoordPointer(2,GL_FLOAT,0,array_uv_main);
			}
		CHECK_GL_ERRORS();
		// lock this new one
		if(have_compiled_vertex_array)ELglLockArraysEXT(0,object_id->e3d_data->face_no);
		// gather statistics
		if(object_id->e3d_data != cur_e3d){
#ifdef  DEBUG
			if(cur_e3d_count > 0 && cur_e3d != NULL){
				e3d_count++;
				e3d_total+= cur_e3d_count;
			}
			cur_e3d_count= 0;
#endif  //DEBUG
			cur_e3d= object_id->e3d_data;
		}
	}
#ifdef  DEBUG
	cur_e3d_count++;
#endif  //DEBUG

	for(i=0;i<materials_no;i++){
		if(array_order[i].count>0)
			{
				CHECK_GL_ERRORS();
				if(is_transparent){
					get_and_set_texture_id(array_order[i].texture_id);
				}
				glDrawArrays(GL_TRIANGLES,array_order[i].start,array_order[i].count);
			}
	}
	//if(have_compiled_vertex_array)ELglUnlockArraysEXT();
	glPopMatrix();//restore the scene
}

#ifdef  NEW_FRUSTUM
void draw_3d_object_shadows(unsigned int object_type)
{
	unsigned int    start, stop;
	unsigned int    i, l;
	int is_transparent;

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
			l = get_intersect_item_ID(main_bbox_tree, i);
			//track the usage
			cache_use(cache_e3d, objects_list[l]->e3d_data->cache_ptr);
		}
		// and all done
		return;
	}

	// find the modes we need
	is_transparent= is_alpha_3d_object(object_type);
	if(is_transparent)
		{
			glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
			glAlphaFunc(GL_GREATER,0.05f);
		}
	else glDisable(GL_TEXTURE_2D);//we don't need textures for non transparent objects
	
	// now loop through each object
	for (i=start; i<stop; i++)
	{
		l = get_intersect_item_ID(main_bbox_tree, i);
		//track the usage
		cache_use(cache_e3d, objects_list[l]->e3d_data->cache_ptr);
		if(!objects_list[l]->display) continue;	// not currently on the map, ignore it
		draw_3d_object_shadow_detail(objects_list[l]);
	}

    if(have_compiled_vertex_array && (cur_e3d != NULL))ELglUnlockArraysEXT();
	if(have_vertex_buffers){
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	}
	if(is_transparent)
		{
			glDisable(GL_ALPHA_TEST);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
	else glEnable(GL_TEXTURE_2D);

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

#else   //NEW_FRUSTUM

void draw_3d_object_shadow(object3d * object_id)
{
	char is_transparent;

	if(!object_id->display) return;	// not currently on the map, ignore it
    if(object_id->blended)return;//blended objects can't have shadows
    //if(object_id->self_lit)return;//light sources can't have shadows
    if(object_id->e3d_data->min_z>=object_id->e3d_data->max_z)return;//we have a flat object

	is_transparent=object_id->e3d_data->is_transparent;
	if(is_transparent)
		{
			glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
			glAlphaFunc(GL_GREATER,0.05f);
		}
	else glDisable(GL_TEXTURE_2D);//we don't need textures for non transparent objects

	draw_3d_object_shadow_detail(object_id);
	
	if(have_compiled_vertex_array)ELglUnlockArraysEXT();
	cur_e3d= NULL;
	if(have_vertex_buffers){
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	}
	if(is_transparent)
		{
			glDisable(GL_ALPHA_TEST);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
	else glEnable(GL_TEXTURE_2D);
}
#endif   //NEW_FRUSTUM

#ifndef MAP_EDITOR2
void draw_enhanced_actor_shadow(actor * actor_id)
{
	double x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;

	glPushMatrix();//we don't want to affect the rest of the scene
	if(!use_shadow_mapping)glMultMatrixf(proj_on_ground);

	x_pos=actor_id->tmp.x_pos;
	y_pos=actor_id->tmp.y_pos;
	z_pos=actor_id->tmp.z_pos;

	if(z_pos==0.0f)//actor is walking, as opposed to flying, get the height underneath
		z_pos=-2.2f+height_map[actor_id->tmp.y_tile_pos*tile_map_size_x*6+actor_id->tmp.x_tile_pos]*0.2f;
	glTranslatef(x_pos+0.25f, y_pos+0.25f, z_pos);

	x_rot=actor_id->tmp.x_rot;
	y_rot=actor_id->tmp.y_rot;
	z_rot=actor_id->tmp.z_rot;

	z_rot+=180;//test
	z_rot=-z_rot;
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);
	cal_render_actor(actor_id);

	glPopMatrix();//restore the scene
}

void draw_actor_shadow(actor * actor_id)
{
	double x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;

	glPushMatrix();//we don't want to affect the rest of the scene
	if(!use_shadow_mapping)glMultMatrixf(proj_on_ground);

	x_pos=actor_id->tmp.x_pos;
	y_pos=actor_id->tmp.y_pos;
	z_pos=actor_id->tmp.z_pos;

	if(z_pos==0.0f)//actor is walking, as opposed to flying, get the height underneath
		z_pos=-2.2f+height_map[actor_id->tmp.y_tile_pos*tile_map_size_x*6+actor_id->tmp.x_tile_pos]*0.2f;

	glTranslatef(x_pos+0.25f, y_pos+0.25f, z_pos);

	x_rot=actor_id->tmp.x_rot;
	y_rot=actor_id->tmp.y_rot;
	z_rot=actor_id->tmp.z_rot;

	z_rot=-z_rot;
	z_rot+=180;//test
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	if (actor_id->calmodel!=NULL) cal_render_actor(actor_id);

	glPopMatrix();//restore the scene
	CHECK_GL_ERRORS();
}

void display_actors_shadow()
{
	int i;
	int x,y;
	x=-cx;
	y=-cy;

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	for(i=0;i<no_near_actors;i++){
		if(!near_actors[i].ghost){
			actor *act=actors_list[near_actors[i].actor];
		
			if(act){
				if(act->is_enhanced_model)
					draw_enhanced_actor_shadow(act);
				else draw_actor_shadow(act);
			}
		}
	}
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}
#endif

void display_shadows()
{
#ifndef	NEW_FRUSTUM
	struct near_3d_object * nobj;

	if(regenerate_near_objects)
		if(!get_near_3d_objects())return;
#endif
	
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glEnableClientState(GL_VERTEX_ARRAY);

#ifndef	NEW_FRUSTUM
	for(nobj=first_near_3d_object;nobj;nobj=nobj->next){
		if(!objects_list[nobj->pos])
			regenerate_near_objects=1;
		else if(!objects_list[nobj->pos]->e3d_data->is_ground && objects_list[nobj->pos]->z_pos>-0.20f )//&& nobj->dist<=900 //It's already limited to max 29*29...
			draw_3d_object_shadow(objects_list[nobj->pos]);
	}

	if(use_shadow_mapping){
		for(nobj=first_near_3d_object;nobj;nobj=nobj->next){
			if(!objects_list[nobj->pos])
				regenerate_near_objects=1;
			else if(objects_list[nobj->pos]->e3d_data->is_ground)//&& nobj->dist<=900 //It's already limited to max 29*29...
				draw_3d_object_shadow(objects_list[nobj->pos]);
		}
	}
#else
	draw_3d_object_shadows(TYPE_3D_NO_BLEND_NO_GROUND_ALPHA_SELF_LIT_OBJECT);
	draw_3d_object_shadows(TYPE_3D_NO_BLEND_NO_GROUND_ALPHA_NO_SELF_LIT_OBJECT);
	draw_3d_object_shadows(TYPE_3D_NO_BLEND_NO_GROUND_NO_ALPHA_SELF_LIT_OBJECT);
	draw_3d_object_shadows(TYPE_3D_NO_BLEND_NO_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT);

	if(use_shadow_mapping)
	{
		draw_3d_object_shadows(TYPE_3D_NO_BLEND_GROUND_ALPHA_SELF_LIT_OBJECT);
		draw_3d_object_shadows(TYPE_3D_NO_BLEND_GROUND_ALPHA_NO_SELF_LIT_OBJECT);
		draw_3d_object_shadows(TYPE_3D_NO_BLEND_GROUND_NO_ALPHA_SELF_LIT_OBJECT);
		draw_3d_object_shadows(TYPE_3D_NO_BLEND_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT);
	}
#endif

	glDisableClientState(GL_VERTEX_ARRAY);
	glCullFace(GL_BACK);
	glDisable(GL_CULL_FACE);
#ifndef MAP_EDITOR2
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.1f, 4.0f);
	display_actors_shadow();
	glDisable(GL_POLYGON_OFFSET_FILL);
	glEnable(GL_TEXTURE_2D);
#endif
}

void display_3d_ground_objects()
{
#ifndef	NEW_FRUSTUM
	struct near_3d_object *nobj;

	if(regenerate_near_objects)if(!get_near_3d_objects())return;
#endif

	glEnable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	if(have_multitexture && !dungeon && clouds_shadows)
		{
			//bind the detail texture
			ELglActiveTextureARB(detail_unit);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, get_texture_id(ground_detail_text));
			ELglActiveTextureARB(base_unit);
			glEnable(GL_TEXTURE_2D);
		}

   	glNormal3f(0,0,1);

#ifndef	NEW_FRUSTUM
    	for(nobj=first_near_3d_object;nobj;nobj=nobj->next){
		if(!objects_list[nobj->pos])
			regenerate_near_objects=1;
		else if(objects_list[nobj->pos]->e3d_data->is_ground && nobj->dist<=700)
    	        	draw_3d_object(objects_list[nobj->pos]);
    	}
#else
	draw_3d_objects(TYPE_3D_NO_BLEND_GROUND_ALPHA_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_NO_BLEND_GROUND_ALPHA_NO_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_NO_BLEND_GROUND_NO_ALPHA_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_NO_BLEND_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT);
#endif

	if(have_multitexture && !dungeon && clouds_shadows)
		{
			//disable the second texture unit
			ELglActiveTextureARB(detail_unit);
			glDisable(GL_TEXTURE_2D);
			ELglActiveTextureARB(base_unit);
		}
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_CULL_FACE);
}

void display_3d_non_ground_objects()
{
#ifndef	NEW_FRUSTUM
	struct near_3d_object * nobj;

	if(regenerate_near_objects)if(!get_near_3d_objects())return;
#endif

	//we don't want to be affected by 2d objects and shadows
	anything_under_the_mouse(0,UNDER_MOUSE_NO_CHANGE);
	
	glEnable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	if(have_multitexture && !dungeon && clouds_shadows)
		{
			//bind the detail texture
			ELglActiveTextureARB(detail_unit);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, get_texture_id(ground_detail_text));
			ELglActiveTextureARB(base_unit);
			glEnable(GL_TEXTURE_2D);
		}

#ifndef	NEW_FRUSTUM
	for(nobj=first_near_3d_object;nobj;nobj=nobj->next){
		if(!objects_list[nobj->pos])
			regenerate_near_objects=1;
		else if(!objects_list[nobj->pos]->e3d_data->is_ground)
			draw_3d_object(objects_list[nobj->pos]);
	}
#else
	draw_3d_objects(TYPE_3D_NO_BLEND_NO_GROUND_ALPHA_SELF_LIT_OBJECT);
	draw_3d_objects( TYPE_3D_NO_BLEND_NO_GROUND_ALPHA_NO_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_NO_BLEND_NO_GROUND_NO_ALPHA_SELF_LIT_OBJECT);
	draw_3d_objects(TYPE_3D_NO_BLEND_NO_GROUND_NO_ALPHA_NO_SELF_LIT_OBJECT);
#endif

	if(have_multitexture && !dungeon && clouds_shadows)
		{
			//disable the second texture unit
			ELglActiveTextureARB(detail_unit);
			glDisable(GL_TEXTURE_2D);
			ELglActiveTextureARB(base_unit);
		}
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_CULL_FACE);
}

void render_light_view()
{
#ifdef NEW_FRUSTUM
	unsigned int cur_intersect_type;
#endif
	if(use_shadow_mapping)
		{
#ifdef USE_LISPSM
			MATRIX4x4D ModelViewMatrix;
			MATRIX4x4D ProjectionMatrix;
			VECTOR3D lightDir;
			double d;
			
			glGetDoublev(GL_MODELVIEW_MATRIX, ModelViewMatrix);
			glGetDoublev(GL_PROJECTION_MATRIX, ProjectionMatrix);
			
			lightDir[X] = -sun_position[X];
			lightDir[Y] = -sun_position[Y];
			lightDir[Z] = -sun_position[Z];
			
			d = sqrt(lightDir[X]*lightDir[X] + lightDir[Y]*lightDir[Y] + lightDir[Z]*lightDir[Z]);
			lightDir[X] /= d;
			lightDir[Y] /= d;
			lightDir[Z] /= d;
			
			calculate_Light_Matrix(1, 1.0, lightDir, ModelViewMatrix,
				ProjectionMatrix, light_view_mat, light_proj_mat);
#endif
			glPushAttrib(GL_ALL_ATTRIB_BITS);
			if(!depth_map_id)
				{
#ifndef USE_LISPSM
					GLint depthbits=16;
					GLenum internalformat=GL_DEPTH_COMPONENT16_ARB;
#endif
					glGenTextures(1,&depth_map_id);
					glBindTexture(depth_texture_target,depth_map_id);
					CHECK_GL_ERRORS();
#ifndef USE_LISPSM
					glGetIntegerv(GL_DEPTH_BITS,&depthbits);
					if(depthbits==24)internalformat=GL_DEPTH_COMPONENT24_ARB;
					else if(depthbits==32)internalformat=GL_DEPTH_COMPONENT32_ARB;
					glTexImage2D(depth_texture_target,0,internalformat,
						     depth_map_width,depth_map_height,
						     0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
					glTexParameteri(depth_texture_target,GL_TEXTURE_COMPARE_MODE_ARB,
							GL_COMPARE_R_TO_TEXTURE_ARB);
					glTexParameteri(depth_texture_target,GL_TEXTURE_COMPARE_FUNC_ARB,GL_LEQUAL);

					glTexParameteri(depth_texture_target,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
					glTexParameteri(depth_texture_target,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
					//TODO: Might want to use CLAMP_TO_BORDER for cards that support it?
					glTexParameteri(depth_texture_target,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
					glTexParameteri(depth_texture_target,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
#else
					glTexImage2D(depth_texture_target,0,GL_DEPTH_COMPONENT,
						     depth_map_width,depth_map_height,
						     0,GL_DEPTH_COMPONENT,GL_UNSIGNED_BYTE,NULL);					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);	
					glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_LUMINANCE);					
#endif
					CHECK_GL_ERRORS();
				}

			glViewport(0,0,depth_map_width,depth_map_height);

			glDisable(GL_LIGHTING);
			glEnable(GL_DEPTH_TEST);
#ifndef MAP_EDITOR2
			if(weather_use_fog())glDisable(GL_FOG);
#endif
			glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
			CHECK_GL_ERRORS();
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadMatrixd(light_proj_mat);
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadMatrixd(light_view_mat);
#ifndef USE_LISPSM
			glTranslatef((int)cx,(int)cy,(int)cz);
#endif
#ifdef NEW_FRUSTUM
			cur_intersect_type = get_cur_intersect_type(main_bbox_tree);
			set_cur_intersect_type(main_bbox_tree, ITERSECTION_TYPE_SHADOW);
			calculate_shadow_frustum();
#endif
			display_shadows();
#ifdef NEW_FRUSTUM
			set_cur_intersect_type(main_bbox_tree, cur_intersect_type);
#endif

			glBindTexture(depth_texture_target,depth_map_id);
			glCopyTexSubImage2D(depth_texture_target,0,0,0,0,0,depth_map_width,depth_map_height);

			CHECK_GL_ERRORS();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
			glClear(GL_DEPTH_BUFFER_BIT);
			glPopAttrib();
			glBindTexture(GL_TEXTURE_2D,0);
			last_texture=-1;
			CHECK_GL_ERRORS();
		}
}

void setup_2d_texgen()
{
#ifndef USE_LISPSM
	GLfloat plane[4];

	glEnable(GL_TEXTURE_GEN_S);
	glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
	plane[0]=texgen_mat[0];plane[1]=texgen_mat[4];plane[2]=texgen_mat[8];plane[3]=texgen_mat[12];
	glTexGenfv(GL_S,GL_EYE_PLANE,plane);

	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
	plane[0]=texgen_mat[1];plane[1]=texgen_mat[5];plane[2]=texgen_mat[9];plane[3]=texgen_mat[13];
	glTexGenfv(GL_T,GL_EYE_PLANE,plane);

	glEnable(GL_TEXTURE_GEN_R);
	glTexGeni(GL_R,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
	plane[0]=texgen_mat[2];plane[1]=texgen_mat[6];plane[2]=texgen_mat[10];plane[3]=texgen_mat[14];
	glTexGenfv(GL_R,GL_EYE_PLANE,plane);

	glEnable(GL_TEXTURE_GEN_Q);
	glTexGeni(GL_Q,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
	plane[0]=texgen_mat[3];plane[1]=texgen_mat[7];plane[2]=texgen_mat[11];plane[3]=texgen_mat[15];
	glTexGenfv(GL_Q,GL_EYE_PLANE,plane);
#else
	const GLdouble x[4] = {1.0, 0.0, 0.0, 0.0};
	const GLdouble y[4] = {0.0, 1.0, 0.0, 0.0};
	const GLdouble z[4] = {0.0, 0.0, 1.0, 0.0};
	const GLdouble w[4] = {0.0, 0.0, 0.0, 1.0};
	const GLdouble bias[16] = {0.5, 0.0, 0.0, 0.0, 
				 	0.0, 0.5, 0.0, 0.0,
					0.0, 0.0, 0.5, 0.0,
					0.5, 0.5, 0.5, 1.0};

	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);
	glEnable(GL_TEXTURE_GEN_Q);

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	
	glTexGendv(GL_S, GL_EYE_PLANE, x);
	glTexGendv(GL_T, GL_EYE_PLANE, y);
	glTexGendv(GL_R, GL_EYE_PLANE, z);
	glTexGendv(GL_Q, GL_EYE_PLANE, w);

	glMatrixMode(GL_TEXTURE);
	glLoadMatrixd(bias);
	glMultMatrixd(light_proj_mat);
	glMultMatrixd(light_view_mat);
	glMatrixMode(GL_MODELVIEW);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
#endif
}

void disable_texgen()
{
#ifndef USE_LISPSM
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
	glDisable(GL_TEXTURE_GEN_Q);
#else
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
	glDisable(GL_TEXTURE_GEN_Q);
#endif
}

void setup_shadow_mapping()
{
#ifndef USE_LISPSM
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -zoom_level*camera_distance);
	glRotatef(rx, 1.0f, 0.0f, 0.0f);
	glRotatef(rz, 0.0f, 0.0f, 1.0f);
	glTranslatef(cx-(int)cx,cy-(int)cy,cz-(int)cz);
#endif
	glBindTexture(depth_texture_target,depth_map_id);
	setup_2d_texgen();

#ifdef OSX
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_COMBINE_ARB);
	glTexEnvi(GL_TEXTURE_ENV,GL_COMBINE_RGB_ARB,GL_INTERPOLATE_ARB);
	glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE0_RGB_ARB,GL_PREVIOUS_ARB);
	glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND0_RGB_ARB,GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE1_RGB_ARB,GL_CONSTANT_ARB);
	glTexEnvfv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR,sun_ambient_light);
	glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND1_RGB_ARB,GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE2_RGB_ARB,GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND2_RGB_ARB,GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV,GL_COMBINE_ALPHA_ARB,GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE0_ALPHA_ARB,GL_PREVIOUS_ARB);
	glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND0_ALPHA_ARB,GL_SRC_ALPHA);
#else
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_COMBINE_EXT);
	glTexEnvi(GL_TEXTURE_ENV,GL_COMBINE_RGB_EXT,GL_INTERPOLATE_EXT);
	glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE0_RGB_EXT,GL_PREVIOUS_EXT);
	glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND0_RGB_EXT,GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE1_RGB_EXT,GL_CONSTANT_EXT);
	glTexEnvfv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR,sun_ambient_light);
	glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND1_RGB_EXT,GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE2_RGB_EXT,GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND2_RGB_EXT,GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV,GL_COMBINE_ALPHA_EXT,GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE0_ALPHA_EXT,GL_PREVIOUS_EXT);
	glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND0_ALPHA_EXT,GL_SRC_ALPHA);
#endif
#ifndef USE_LISPSM
	glPopMatrix();
#endif
}

void draw_sun_shadowed_scene(int any_reflection)
{
	if(use_shadow_mapping)
		{
			shadow_unit=GL_TEXTURE0_ARB;
			base_unit=GL_TEXTURE1_ARB;
			detail_unit=GL_TEXTURE2_ARB;

#ifndef MAP_EDITOR2
			if (weather_use_fog()) glDisable(GL_FOG);
#endif

			ELglActiveTextureARB(shadow_unit);
			glEnable(depth_texture_target);
			setup_shadow_mapping();

			ELglActiveTextureARB(detail_unit);
			glDisable(GL_TEXTURE_2D);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glBindTexture(GL_TEXTURE_2D,0);

			ELglClientActiveTextureARB(base_unit);
			ELglActiveTextureARB(base_unit);
			glEnable(GL_TEXTURE_2D);
			last_texture=-1;
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			CHECK_GL_ERRORS();

#ifndef MAP_EDITOR2
			if(weather_use_fog()) glEnable(GL_FOG);
#endif		
#ifdef NEW_FRUSTUM
			main_bbox_tree->intersect[ITERSECTION_TYPE_DEFAULT].intersect_update_needed = 1;
#endif
			CalculateFrustum();
			
			glNormal3f(0.0f,0.0f,1.0f);
			if(any_reflection)draw_lake_tiles();
			draw_tile_map();
#ifdef MAP_EDITOR2
			get_world_x_y ();
			display_mode();
#endif
			CHECK_GL_ERRORS();
			display_2d_objects();
			CHECK_GL_ERRORS();
			anything_under_the_mouse(0, UNDER_MOUSE_NOTHING);
			display_objects();
#ifndef MAP_EDITOR2
			display_actors(1);  // Affects other textures ????????? (FPS etc., unless there's a particle system...)
#endif
			display_blended_objects();

#ifndef MAP_EDITOR2
			if (weather_use_fog()) glDisable(GL_FOG);
#endif

			ELglActiveTextureARB(shadow_unit);
			glDisable(depth_texture_target);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glBindTexture(GL_TEXTURE_2D,0);
			disable_texgen();

			ELglActiveTextureARB(detail_unit);
			glDisable(GL_TEXTURE_2D);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glBindTexture(GL_TEXTURE_2D,0);
			disable_texgen();

			ELglActiveTextureARB(base_unit);
			glDisable(GL_TEXTURE_2D);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glBindTexture(GL_TEXTURE_2D,0);
			disable_texgen();

			shadow_unit=GL_TEXTURE2_ARB;
			base_unit=GL_TEXTURE0_ARB;
			detail_unit=GL_TEXTURE1_ARB;
			ELglActiveTextureARB(base_unit);
			ELglClientActiveTextureARB(base_unit);
			last_texture=-1;
			glBindTexture(GL_TEXTURE_2D,0);
			glEnable(GL_TEXTURE_2D);
		}
	else
		{
#ifndef NEW_WEATHER
			int abs_light;
#endif

			glNormal3f(0.0f,0.0f,1.0f);
			if(any_reflection)draw_lake_tiles();

			draw_tile_map();
#ifdef MAP_EDITOR2
			get_world_x_y ();
			display_mode();
#endif
			CHECK_GL_ERRORS();
			display_2d_objects();
			CHECK_GL_ERRORS();
			anything_under_the_mouse(0, UNDER_MOUSE_NOTHING);
			display_3d_ground_objects();
			// turning off writing to the color buffer and depth buffer
			glDisable(GL_DEPTH_TEST);

			glDisable(GL_LIGHTING);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

			glEnable(GL_STENCIL_TEST);
			// write a one to the stencil buffer everywhere we are about to draw
			glStencilFunc(GL_ALWAYS, 1, 0xffffffff);
			// this is to always pass a one to the stencil buffer where we draw
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

			display_shadows();

			glStencilFunc(GL_EQUAL, 1, 0xFFFFFFFF);
			// don't modify the contents of the stencil buffer
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

			// Lachesis: drawing in 3D mode in order to get the fog correct
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_LIGHTING);
			glDepthMask(GL_FALSE);
			glDisable(GL_DEPTH_TEST);

#ifndef MAP_EDITOR2
			if (weather_use_fog()) glEnable(GL_FOG);
#endif

			glEnable(GL_BLEND);
			// need this function (or both flipped) for correctly working fog too
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

#ifdef NEW_WEATHER
			glColor4f(0.0f, 0.0f, 0.0f, 0.7f*((difuse_light[0] + difuse_light[1] + difuse_light[2])/3.0 - 0.0f));
#else
			abs_light=light_level;
			if(light_level>59)abs_light=119-light_level;

#ifndef MAP_EDITOR2
			abs_light+=weather_light_offset;
#endif
			if(abs_light<0)abs_light=0;
			if(abs_light>59)abs_light=59;

			glColor4f(0.0f,0.0f,0.0f,0.27f - (float)abs_light*0.008f);
#endif
			
			glBegin(GL_QUADS);
				glVertex4f(-cx+20.0f,-cy+20.0f,0.0f,1.0f);
				glVertex4f(-cx+20.0f,-cy-20.0f,0.0f,1.0f);
				glVertex4f(-cx-20.0f,-cy-20.0f,0.0f,1.0f);
				glVertex4f(-cx-20.0f,-cy+20.0f,0.0f,1.0f);
			glEnd();

			glDisable(GL_BLEND);

			glEnable(GL_TEXTURE_2D);
			glDepthMask(GL_TRUE);

			glEnable(GL_DEPTH_TEST);
			glColor4f(1.0f,1.0f,1.0f,1.0f);
			glEnable(GL_LIGHTING);
			glDisable(GL_STENCIL_TEST);

			display_3d_non_ground_objects();
#ifndef MAP_EDITOR2
			display_actors(1);
#endif
			display_blended_objects();

		}
}
