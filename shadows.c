#include <string.h>
#include "global.h"
#include <math.h>

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
	depth_map_width=floor_pow2(window_width);
	depth_map_height=floor_pow2(window_height);
	while(depth_map_width>max)depth_map_width/=2;
	while(depth_map_height>max)depth_map_height/=2;	
}

void calc_light_frustum(float light_xrot)
{
	float window_ratio=(GLfloat)window_width/(GLfloat)window_height;
	float max_height=15.0; //TODO: Really calculate this from positions and heights of objects
	float x,y;
	light_xrot=-light_xrot;
	//TODO: Optimize this function a bit.
	//Assuming a max zoom_level of 3.75 and near/far distances of 20.0, we'll set the hscale to the radius of a circle that
	//can just contain the view frustum of the player. To simplify things, we'll assume the view frustum is horizontal.
	light_view_hscale=sqrt(window_ratio*window_ratio*3.75f*3.75f+20.0f*20.0f);
	// For the others, we can just use the parametric ellipse formula to find the value for this angle
	x=light_view_hscale*sin(light_xrot);
	y=max_height*cos(light_xrot);
	light_view_top=sqrt(x*x+y*y);
	y=3.75f*cos(light_xrot);
	light_view_far=sqrt(x*x+y*y);
	x=light_view_hscale*cos(light_xrot);
	y=3.75f*sin(light_xrot);
	light_view_bottom=-sqrt(x*x+y*y);
	x=100.0f*sin(light_xrot);  // A bit better than the real value (infinity)
	y=max_height*cos(light_xrot);
	light_view_near=-sqrt(x*x+y*y);
}

void calc_shadow_matrix()
{
	if(use_shadow_mapping)
		{
			float xrot,zrot;

			double scale1d[]={0.0,0.0,0.0,0.0,
					  0.0,0.0,0.0,0.0,
					  0.5,0.0,0.0,0.0,
					  0.5,0.0,0.0,1.0};
			
			float div_length=1.0f/sqrt(sun_position[0]*sun_position[0]+sun_position[1]*sun_position[1]+sun_position[2]*sun_position[2]);
			sun_position[0]*=div_length;
			sun_position[1]*=div_length;
			sun_position[2]*=div_length;
			xrot=-acosf(sun_position[2]);
			//xrot=-atan2f(sun_position[2],sun_position[0])*180.0f/3.1415926f;
			zrot=-90.0f-atan2f(sun_position[1],sun_position[0])*180.0f/3.1415926f;
			glPushMatrix();
			glLoadIdentity();
			calc_light_frustum(xrot);
			xrot*=180.0f/3.1415926f;
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

void draw_3d_object_shadow(object3d * object_id)
{
	float x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;

	int materials_no;
	int i;
	char is_transparent;

	e3d_array_vertex *array_vertex;
	e3d_array_uv_main *array_uv_main;
	e3d_array_order *array_order;

    if(object_id->blended)return;//blended objects can't have shadows
    if(object_id->self_lit)return;//light sources can't have shadows
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

	if(is_transparent)
		{
			glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
			glAlphaFunc(GL_GREATER,0.05f);
		}
	else glDisable(GL_TEXTURE_2D);//we don't need textures for non transparent objects

	glPushMatrix();//we don't want to affect the rest of the scene
	if(!use_shadow_mapping)glMultMatrixf(proj_on_ground);
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

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3,GL_FLOAT,0,array_vertex);
	if(is_transparent)
		{
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2,GL_FLOAT,0,array_uv_main);
		}

	if(have_compiled_vertex_array)ELglLockArraysEXT(0,object_id->e3d_data->face_no);
	for(i=0;i<materials_no;i++)
		if(array_order[i].count>0)
			{
				if(is_transparent)
					get_and_set_texture_id(array_order[i].texture_id);
				//if(have_compiled_vertex_array)ELglLockArraysEXT(array_order[i].start, array_order[i].count);
				glDrawArrays(GL_TRIANGLES,array_order[i].start,array_order[i].count);
				//if(have_compiled_vertex_array)ELglUnlockArraysEXT();
			}
	if(have_compiled_vertex_array)ELglUnlockArraysEXT();
	glPopMatrix();//restore the scene

	glDisableClientState(GL_VERTEX_ARRAY);
	if(is_transparent)
		{
			glDisable(GL_ALPHA_TEST);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
	else glEnable(GL_TEXTURE_2D);
}

void draw_model_shadow(md2 *model_data,char *cur_frame, int ghost)
{
	int frame;
	int numFaces;

	//now, go and find the current frame
	frame = get_frame_number(model_data, cur_frame);
	if(frame < 0)return;	//can't draw it
	//track the usage
	cache_use(cache_md2, model_data->cache_ptr);

	numFaces=model_data->numFaces;

	check_gl_errors();
	if(use_vertex_array > 0)
		{
			//TODO: smarter decision making and maybe trigger cleanup?
			if(!model_data->text_coord_array || !model_data->offsetFrames[frame].vertex_array)
				{
					build_md2_va(model_data, &model_data->offsetFrames[frame]);
				}
		}
	// determine the drawing method
	if(use_vertex_array > 0 && model_data->text_coord_array && model_data->offsetFrames[frame].vertex_array)
		{
			glVertexPointer(3,GL_FLOAT,0,model_data->offsetFrames[frame].vertex_array);
			if(have_compiled_vertex_array)ELglLockArraysEXT(0, model_data->numFaces*3);
			glDrawArrays(GL_TRIANGLES, 0, model_data->numFaces*3);
			if(have_compiled_vertex_array)ELglUnlockArraysEXT();
		}
	else
		{
			int i;
			face_md2 *offsetFaces;
			vertex_md2 *vertex_pointer=NULL;
			//setup
			glBegin(GL_TRIANGLES);
			vertex_pointer=model_data->offsetFrames[frame].vertex_pointer;
			offsetFaces=model_data->offsetFaces;
			//draw each triangle
			for(i=0;i<numFaces;i++)
				{
					float x,y,z;

					x=vertex_pointer[offsetFaces[i].a].x;
					y=vertex_pointer[offsetFaces[i].a].y;
					z=vertex_pointer[offsetFaces[i].a].z;
					glVertex3f(x,y,z);

					x=vertex_pointer[offsetFaces[i].b].x;
					y=vertex_pointer[offsetFaces[i].b].y;
					z=vertex_pointer[offsetFaces[i].b].z;
					glVertex3f(x,y,z);

					x=vertex_pointer[offsetFaces[i].c].x;
					y=vertex_pointer[offsetFaces[i].c].y;
					z=vertex_pointer[offsetFaces[i].c].z;
					glVertex3f(x,y,z);
				}
			glEnd();
		}
	check_gl_errors();
}


void draw_enhanced_actor_shadow(actor * actor_id)
{
	double x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;
	char *cur_frame;

	cur_frame=actor_id->cur_frame;

	glPushMatrix();//we don't want to affect the rest of the scene
	if(!use_shadow_mapping)glMultMatrixf(proj_on_ground);

	x_pos=actor_id->x_pos;
	y_pos=actor_id->y_pos;
	z_pos=actor_id->z_pos;

	if(z_pos==0.0f)//actor is walking, as opposed to flying, get the height underneath
		z_pos=-2.2f+height_map[actor_id->y_tile_pos*tile_map_size_x*6+actor_id->x_tile_pos]*0.2f;
	glTranslatef(x_pos+0.25f, y_pos+0.25f, z_pos);

	x_rot=actor_id->x_rot;
	y_rot=actor_id->y_rot;
	z_rot=actor_id->z_rot;
	z_rot+=180;//test
	z_rot=-z_rot;
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	if(actor_id->body_parts->legs)draw_model_shadow(actor_id->body_parts->legs,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->torso)draw_model_shadow(actor_id->body_parts->torso,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->head)draw_model_shadow(actor_id->body_parts->head,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->weapon)draw_model_shadow(actor_id->body_parts->weapon,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->shield)draw_model_shadow(actor_id->body_parts->shield,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->helmet)draw_model_shadow(actor_id->body_parts->helmet,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->cape)draw_model_shadow(actor_id->body_parts->cape,cur_frame,actor_id->ghost);

	glPopMatrix();//restore the scene
}

void draw_actor_shadow(actor * actor_id)
{
	int i;
	double x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;
	char *cur_frame;

	//now, go and find the current frame
	cur_frame=actor_id->cur_frame;
	i = get_frame_number(actor_id->model_data, cur_frame);
	if(i < 0)return;	//can't draw it

	glPushMatrix();//we don't want to affect the rest of the scene
	if(!use_shadow_mapping)glMultMatrixf(proj_on_ground);

	x_pos=actor_id->x_pos;
	y_pos=actor_id->y_pos;
	z_pos=actor_id->z_pos;
	if(z_pos==0.0f)//actor is walking, as opposed to flying, get the height underneath
		z_pos=-2.2f+height_map[actor_id->y_tile_pos*tile_map_size_x*6+actor_id->x_tile_pos]*0.2f;

	glTranslatef(x_pos+0.25f, y_pos+0.25f, z_pos);

	x_rot=actor_id->x_rot;
	y_rot=actor_id->y_rot;
	z_rot=actor_id->z_rot;
	z_rot=-z_rot;
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	draw_model_shadow(actor_id->model_data,cur_frame,actor_id->ghost);

	glPopMatrix();//restore the scene
	check_gl_errors();
}

void display_actors_shadow()
{
	int i;
	int x,y;
	x=-cx;
	y=-cy;
	if(use_vertex_array > 0)
		{
			glEnableClientState(GL_VERTEX_ARRAY);
			//glEnableClientState(GL_NORMAL_ARRAY);
		}
	for(i=0;i<max_actors;i++)
		{
			if(actors_list[i])
			if(!actors_list[i]->ghost)	//of course ghosts don't get shadows
				{
					int dist1;
					int dist2;

					dist1=x-actors_list[i]->x_pos;
					dist2=y-actors_list[i]->y_pos;
					if(dist1*dist1+dist2*dist2<=12*12)
							{
								if(actors_list[i]->is_enhanced_model)
									draw_enhanced_actor_shadow(actors_list[i]);
								else draw_actor_shadow(actors_list[i]);
							}
				}
		}
	if(use_vertex_array > 0)
		{
			//glDisableClientState(GL_NORMAL_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);
		}
}

void display_shadows()
{
	int i;
	int x,y;

	x=-cx;
	y=-cy;
	glEnable(GL_CULL_FACE);
	for(i=0;i<highest_obj_3d;i++)
		{
			if(objects_list[i])
				{
					if(use_shadow_mapping || (!objects_list[i]->e3d_data->is_ground && objects_list[i]->z_pos>-0.20f))
						{
							int dist1;
							int dist2;

							dist1=x-objects_list[i]->x_pos;
							dist2=y-objects_list[i]->y_pos;
							if(dist1*dist1+dist2*dist2<=900)
								draw_3d_object_shadow(objects_list[i]);
						}
				}
		}
	glDisable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);
	display_actors_shadow();
	glEnable(GL_TEXTURE_2D);
}

void display_3d_ground_objects()
{
	int i;
	int x,y;

	x=-cx;
	y=-cy;
	glEnable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	if(have_multitexture && clouds_shadows)
		{
			//bind the detail texture
			ELglActiveTextureARB(detail_unit);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, get_texture_id(ground_detail_text));
			ELglActiveTextureARB(base_unit);
			glEnable(GL_TEXTURE_2D);

		}
	for(i=0;i<highest_obj_3d;i++)
		{
			if(objects_list[i])
				{
					int dist1;
					int dist2;

					if(objects_list[i]->e3d_data->is_ground)
					 	{
			         		dist1=x-objects_list[i]->x_pos;
			         		dist2=y-objects_list[i]->y_pos;
			         		if(dist1*dist1+dist2*dist2<=700)
								{
									float x_len;
									float y_len;
									float z_len;
									float radius;

									z_len=objects_list[i]->e3d_data->max_z-objects_list[i]->e3d_data->min_z;
									x_len=objects_list[i]->e3d_data->max_x-objects_list[i]->e3d_data->min_x;
									y_len=objects_list[i]->e3d_data->max_y-objects_list[i]->e3d_data->min_y;

									radius=x_len/2;
									if(radius<y_len/2)radius=y_len/2;
									if(radius<z_len)radius=z_len;
									//not in the middle of the air
									if(SphereInFrustum(objects_list[i]->x_pos,objects_list[i]->y_pos,
													   objects_list[i]->z_pos,radius))
										draw_3d_object(objects_list[i]);
								}
						}
				}
		}
	if(have_multitexture && clouds_shadows)
		{
			//disable the second texture unit
			ELglActiveTextureARB(detail_unit);
			glDisable(GL_TEXTURE_2D);
			ELglActiveTextureARB(base_unit);
		}
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_CULL_FACE);
}

void display_3d_non_ground_objects()
{
	int i;
	int x,y;

	x=-cx;
	y=-cy;

	//we don't want to be affected by 2d objects and shadows
	anything_under_the_mouse(0,UNDER_MOUSE_NO_CHANGE);

	glEnable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	if(have_multitexture && clouds_shadows)
		{
			//bind the detail texture
			ELglActiveTextureARB(detail_unit);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, get_texture_id(ground_detail_text));
			ELglActiveTextureARB(base_unit);
			glEnable(GL_TEXTURE_2D);

		}
	for(i=0;i<highest_obj_3d;i++)
		{
			if(objects_list[i])
				{
					if(!objects_list[i]->e3d_data->is_ground)
					{
						int dist1;
						int dist2;

						dist1=x-objects_list[i]->x_pos;
						dist2=y-objects_list[i]->y_pos;
							if(dist1*dist1+dist2*dist2<=900)
			         			{
									float x_len, y_len, z_len;
									float radius;

									z_len=objects_list[i]->e3d_data->max_z-objects_list[i]->e3d_data->min_z;
									x_len=objects_list[i]->e3d_data->max_x-objects_list[i]->e3d_data->min_x;
									y_len=objects_list[i]->e3d_data->max_y-objects_list[i]->e3d_data->min_y;

									radius=x_len/2;
									if(radius<y_len/2)radius=y_len/2;
									if(radius<z_len)radius=z_len;
									//not in the middle of the air
									if(SphereInFrustum(objects_list[i]->x_pos,objects_list[i]->y_pos,
												   objects_list[i]->z_pos,radius))
										{
                     						draw_3d_object(objects_list[i]);
											if (read_mouse_now && mouse_in_sphere(objects_list[i]->x_pos, objects_list[i]->y_pos, objects_list[i]->z_pos, radius))
												anything_under_the_mouse(i,UNDER_MOUSE_3D_OBJ);
										}
								}
						}
				}
		}
	if(have_multitexture && clouds_shadows)
		{
			//disable the second texture unit
			ELglActiveTextureARB(detail_unit);
			glDisable(GL_TEXTURE_2D);
			ELglActiveTextureARB(base_unit);
		}
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_CULL_FACE);
}

void render_light_view()
{
	if(use_shadow_mapping)
		{

			glPushAttrib(GL_ALL_ATTRIB_BITS);
			if(!depth_map_id)
				{
					GLint depthbits=16;
					GLenum internalformat=GL_DEPTH_COMPONENT16_ARB;
					glGenTextures(1,&depth_map_id);
					glBindTexture(depth_texture_target,depth_map_id);
					check_gl_errors();
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
					check_gl_errors();
				}

			glViewport(0,0,depth_map_width,depth_map_height);

			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1.1f,4.0f);
			glDisable(GL_LIGHTING);
			glEnable(GL_DEPTH_TEST);
			glDisable(GL_FOG);
			glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
			check_gl_errors();
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadMatrixd(light_proj_mat);
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadMatrixd(light_view_mat);
			glTranslatef((int)cx,(int)cy,(int)cz);
			display_shadows();

			glBindTexture(depth_texture_target,depth_map_id);
			glCopyTexSubImage2D(depth_texture_target,0,0,0,0,0,depth_map_width,depth_map_height);

			check_gl_errors();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
			glClear(GL_DEPTH_BUFFER_BIT);
			glPopAttrib();
			glBindTexture(GL_TEXTURE_2D,0);
			last_texture=-1;
			check_gl_errors();
		}
}

void setup_2d_texgen()
{
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
}

void disable_texgen()
{
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
	glDisable(GL_TEXTURE_GEN_Q);
}

void setup_shadow_mapping()
{
	glPushMatrix();
	glLoadIdentity();
	glRotatef(rx, 1.0f, 0.0f, 0.0f);
	glRotatef(rz, 0.0f, 0.0f, 1.0f);
	glTranslatef(cx-(int)cx,cy-(int)cy,cz-(int)cz);
	glBindTexture(depth_texture_target,depth_map_id);
	setup_2d_texgen();
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
	glPopMatrix();
}

void draw_sun_shadowed_scene(int any_reflection)
{
	if(use_shadow_mapping)
		{
			shadow_unit=GL_TEXTURE0_ARB;
			base_unit=GL_TEXTURE1_ARB;
			detail_unit=GL_TEXTURE2_ARB;

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
			check_gl_errors();

			glNormal3f(0.0f,0.0f,1.0f);
			if(any_reflection)draw_lake_tiles();
			draw_tile_map();
			check_gl_errors();
			display_2d_objects();
			check_gl_errors();
			anything_under_the_mouse(0, UNDER_MOUSE_NOTHING);
			display_objects();
			display_actors();  // Affects other textures ????????? (FPS etc., unless there's a particle system...)

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
			int abs_light;

			glNormal3f(0.0f,0.0f,1.0f);
			if(any_reflection)draw_lake_tiles();

			draw_tile_map();
			check_gl_errors();
			display_2d_objects();
			check_gl_errors();
			anything_under_the_mouse(0, UNDER_MOUSE_NOTHING);
			display_3d_ground_objects();
			// turning off writing to the color buffer and depth buffer
			glDisable(GL_DEPTH_TEST);

			glDisable(GL_LIGHTING);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

			glEnable(GL_STENCIL_TEST);
			// write a one to the stencil buffer everywhere we are about to draw
			glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
			// this is to always pass a one to the stencil buffer where we draw
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

			display_shadows();

			glStencilFunc(GL_EQUAL, 1, 0xFFFFFFFF);
			// don't modify the contents of the stencil buffer
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			//go to the 2d mode, and draw a black rectangle...
			Enter2DMode();
			glDisable(GL_TEXTURE_2D);

			abs_light=light_level;
			if(light_level>59)abs_light=119-light_level;
			abs_light+=weather_light_offset;
			if(abs_light<0)abs_light=0;
			if(abs_light>59)abs_light=59;

			glColor4f(0.0f,0.0f,0.0f,0.73f+(float)abs_light*0.008f);
			glEnable(GL_BLEND);
			glBlendFunc(GL_ZERO,GL_SRC_ALPHA);
			glBegin(GL_QUADS);
			glVertex3i(0,window_height,0);
			glVertex3i(0,0,0);
			glVertex3i(window_width,0,0);
			glVertex3i(window_width,window_height,0);
			glEnd();
			glDisable(GL_BLEND);
			glEnable(GL_TEXTURE_2D);

			Leave2DMode();
			glEnable(GL_DEPTH_TEST);
			glColor4f(1.0f,1.0f,1.0f,1.0f);
			glEnable(GL_LIGHTING);
			glDisable(GL_STENCIL_TEST);

			display_3d_non_ground_objects();
			display_actors();
		}
}
