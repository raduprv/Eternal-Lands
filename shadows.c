#include "global.h"
#include <math.h>

void SetShadowMatrix()
{
	float dot;
	// dot product of plane and light position
	dot = fPlane[0] * fLightPos[0]
		+ fPlane[1] * fLightPos[1]
		+ fPlane[2] * fLightPos[2]
		+ fPlane[3] * fLightPos[3];

	// first column
	fDestMat[0] = dot - fLightPos[0] * fPlane[0];
	fDestMat[4] = 0.0f - fLightPos[0] * fPlane[1];
	fDestMat[8] = 0.0f - fLightPos[0] * fPlane[2];
	fDestMat[12] = 0.0f - fLightPos[0] * fPlane[3];

	// second column
	fDestMat[1] = 0.0f - fLightPos[1] * fPlane[0];
	fDestMat[5] = dot - fLightPos[1] * fPlane[1];
	fDestMat[9] = 0.0f - fLightPos[1] * fPlane[2];
	fDestMat[13] = 0.0f - fLightPos[1] * fPlane[3];

	// third column
	fDestMat[2] = 0.0f - fLightPos[2] * fPlane[0];
	fDestMat[6] = 0.0f - fLightPos[2] * fPlane[1];
	fDestMat[10] = dot - fLightPos[2] * fPlane[2];
	fDestMat[14] = 0.0f - fLightPos[2] * fPlane[3];

	// fourth column
	fDestMat[3] = 0.0f - fLightPos[3] * fPlane[0];
	fDestMat[7] = 0.0f - fLightPos[3] * fPlane[1];
	fDestMat[11] = 0.0f - fLightPos[3] * fPlane[2];
	fDestMat[15] = dot - fLightPos[3] * fPlane[3];
}

void draw_3d_object_shadow(object3d * object_id)
{
	float x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;

	int materials_no,texture_id;
	int i;
	char is_transparent;

	e3d_array_vertex *array_vertex;
	e3d_array_uv_main *array_uv_main;
	e3d_array_order *array_order;

    if(object_id->blended)return;//blended objects can't have shadows
    if(object_id->self_lit)return;//light sources can't have shadows
    if(!(object_id->e3d_data->min_z-object_id->e3d_data->max_z))return;//we have a flat object

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
	glMultMatrixf(fDestMat);
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

	for(i=0;i<materials_no;i++)
		{
			if(is_transparent)
				{
					texture_id=get_texture_id(array_order[i].texture_id);
					if(last_texture!=texture_id)
						{
							last_texture=texture_id;
							glBindTexture(GL_TEXTURE_2D, texture_id);
						}
				}
			//if(have_compiled_vertex_array)ELglLockArraysEXT(array_order[i].start, array_order[i].count);
			glDrawArrays(GL_TRIANGLES,array_order[i].start,array_order[i].count);
			//if(have_compiled_vertex_array)ELglUnlockArraysEXT();
		}

	glPopMatrix();//restore the scene

	glDisableClientState(GL_VERTEX_ARRAY);
	if(is_transparent)
		{
			glDisable(GL_ALPHA_TEST);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
	else glEnable(GL_TEXTURE_2D);

}

void display_shadows()
{
	int i;
	int x,y;

	x=(int)-cx;
	y=(int)-cy;
	glEnable(GL_CULL_FACE);
	for(i=0;i<max_obj_3d;i++)
		{
			if(objects_list[i])
				 {
				if(!objects_list[i]->e3d_data->is_ground && objects_list[i]->z_pos>-0.20f)
					{
						 int dist1;
						 int dist2;

						 dist1=x-(int)objects_list[i]->x_pos;
						 dist2=y-(int)objects_list[i]->y_pos;
						 if(dist1*dist1+dist2*dist2<=20*20)
							draw_3d_object_shadow(objects_list[i]);
					 }
                 }
		}
	glDisable(GL_CULL_FACE);
}

void display_night_shadows(int phase)
{
	int i,j;
	int x,y;
	int closest_light=-1;
	int next_closest_light=-1;
	float closest_light_dist=100.0f;
	float next_closest_light_dist=100.0f;

	x=(int)-cx;
	y=(int)-cy;
	glEnable(GL_CULL_FACE);
	for(i=0;i<max_obj_3d;i++)
		{
			closest_light=-1;
			next_closest_light=-1;
			closest_light_dist=100.0f;
			next_closest_light_dist=100.0f;

			if(objects_list[i])
				 {
				if(!objects_list[i]->e3d_data->is_ground && objects_list[i]->z_pos>-0.20f)
					{
					 float dist1;
					 float dist2;
					 float x_pos,y_pos;

					 x_pos=objects_list[i]->x_pos;
					 y_pos=objects_list[i]->y_pos;
					 dist1=x-(int)x_pos;
					 dist2=y-(int)y_pos;
					 if(dist1*dist1+dist2*dist2<=20*20)
						{
							//now, find the closest, or next closest light source, according to
							//the phase value
							for(j=0;j<max_lights;j++)
							 {
							  if(lights_list[j])
							   {
								 float dist1;
								 float dist2;
								 float light_dist;

								 dist1=x_pos-lights_list[j]->pos_x;
								 dist2=y_pos-lights_list[j]->pos_y;
								 light_dist=sqrt(dist1*dist1+dist2*dist2);
								 if(light_dist<=8)
									{
										if(light_dist<closest_light_dist)
											{
												//update the closest and next closest light
												next_closest_light_dist=closest_light_dist;
												next_closest_light=closest_light;
												closest_light=j;
												closest_light_dist=light_dist;
											}
										else
										if(light_dist<next_closest_light_dist)
											{
												//update the closest and next
												next_closest_light=j;
												next_closest_light_dist=light_dist;
											}
									}

							   }
							 }
							if(phase==1)if(closest_light!=-1)
								{
									fLightPos[0]=lights_list[closest_light]->pos_x;
									fLightPos[1]=lights_list[closest_light]->pos_y;
									fLightPos[2]=lights_list[closest_light]->pos_z;
									fLightPos[3]=1.0f;
									SetShadowMatrix();
									draw_3d_object_shadow(objects_list[i]);
								}
							if(phase==2)if(next_closest_light!=-1)
								{
									fLightPos[0]=lights_list[next_closest_light]->pos_x;
									fLightPos[1]=lights_list[next_closest_light]->pos_y;
									fLightPos[2]=lights_list[next_closest_light]->pos_z;
									fLightPos[3]=1.0f;
									SetShadowMatrix();
									draw_3d_object_shadow(objects_list[i]);
								}
						}//object in range
                 }//object nonground
			 }//object exist
		}//main for
	glDisable(GL_CULL_FACE);
}

void display_3d_ground_objects()
{
	int i;
	int x,y;

	x=(int)-cx;
	y=(int)-cy;
	glEnable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	if(have_multitexture && clouds_shadows)
		{
			//bind the detail texture
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, texture_cache[ground_detail_text].texture_id);
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_2D);

		}
	for(i=0;i<max_obj_3d;i++)
		{
			if(objects_list[i])
				 {
					 if(objects_list[i]->e3d_data->is_ground)
						{
					 		int dist1;
					 		int dist2;

							dist1=x-(int)objects_list[i]->x_pos;
							dist2=y-(int)objects_list[i]->y_pos;
							if(dist1*dist1+dist2*dist2<=25*25)
								{
									/*
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
										*/
										draw_3d_object(objects_list[i]);
								}
						}
                 }
		}
	if(have_multitexture && clouds_shadows)
		{
			//disable the second texture unit
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glDisable(GL_TEXTURE_2D);
			glActiveTextureARB(GL_TEXTURE0_ARB);
		}
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_CULL_FACE);
}

void display_3d_non_ground_objects()
{
	int i;
	int x,y;

	x=(int)-cx;
	y=(int)-cy;
	glEnable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	if(have_multitexture && clouds_shadows)
		{
			//bind the detail texture
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, texture_cache[ground_detail_text].texture_id);
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_2D);

		}
	for(i=0;i<max_obj_3d;i++)
		{
			if(objects_list[i])
				 {
					 if(!objects_list[i]->e3d_data->is_ground)
						{
							int dist1;
							int dist2;

							dist1=x-(int)objects_list[i]->x_pos;
							dist2=y-(int)objects_list[i]->y_pos;
							if(dist1*dist1+dist2*dist2<=25*25)
								{
									/*
									float x_len, y_len, z_len;
									float radius;

									z_len=objects_list[i]->e3d_data->max_z-objects_list[i]->e3d_data->min_z;
									x_len=objects_list[i]->e3d_data->max_x-objects_list[i]->e3d_data->min_x;
									y_len=objects_list[i]->e3d_data->max_y-objects_list[i]->e3d_data->min_y;

									radius=x_len/2;
									if(radius<y_len/2)radius=y_len/2;
									if(radius<z_len)radius=z_len;
fprintf(stderr, "(%f, %f, %f) -> %f == %d\n", objects_list[i]->x_pos,objects_list[i]->y_pos, objects_list[i]->z_pos, radius,SphereInFrustum(objects_list[i]->x_pos,objects_list[i]->y_pos, objects_list[i]->z_pos,radius));
									//not in the middle of the air
									if(SphereInFrustum(objects_list[i]->x_pos,objects_list[i]->y_pos,
													   objects_list[i]->z_pos,radius))
										{*/
											draw_3d_object(objects_list[i]);
										//}
								}
						}
                 }
		}
	if(have_multitexture && clouds_shadows)
		{
			//disable the second texture unit
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glDisable(GL_TEXTURE_2D);
			glActiveTextureARB(GL_TEXTURE0_ARB);
		}
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_CULL_FACE);
}

void draw_sun_shadowed_scene()
{
	int	abs_light;

	display_3d_ground_objects();
	// turning off writing to the color buffer and depth buffer
	glDisable(GL_DEPTH_TEST);

	glDisable(GL_LIGHTING);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	glEnable(GL_STENCIL_TEST);
	// write a one to the stencil buffer everywhere we are about to draw
	glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
	// this is to always pass a one to the stencil buffer where we draw
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

	display_shadows();

	glStencilFunc(GL_EQUAL, 1, 0xFFFFFFFF);
	// don't modify the contents of the stencil buffer
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	//go to the 2d mode, and draw a black rectangle...
	Enter2DMode();
	glDisable(GL_TEXTURE_2D);

	//if(light_level<60)glColor4f(0.0f,0.0f,0.0f,0.73f+(float)light_level*0.008f);
	//if(light_level>60)glColor4f(0.0f,0.0f,0.0f,0.73f+(float)(119-light_level)*0.008f);
	abs_light=light_level;
	if(light_level>59)abs_light=119-light_level;
	//abs_light+=weather_light_offset;
	if(abs_light<0)abs_light=0;
	if(abs_light>59)abs_light=59;

	glColor4f(0.0f,0.0f,0.0f,0.73f+(float)abs_light*0.008f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
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
}

void draw_night_shadowed_scene()
{
	display_3d_ground_objects();
	// turning off writing to the color buffer and depth buffer
	glDisable(GL_DEPTH_TEST);

	glDisable(GL_LIGHTING);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	glEnable(GL_STENCIL_TEST);
	// write a one to the stencil buffer everywhere we are about to draw
	glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
	// this is to always pass a one to the stencil buffer where we draw
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

	//display the first (closest light)
	display_night_shadows(1);
	//now, change the value of the stencil to 2, to have two distinct shadows
	glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
	glStencilOp(GL_INCR, GL_INCR, GL_INCR);
	//draw the next closest light
	display_night_shadows(2);

	glStencilFunc(GL_EQUAL, 1, 0xFFFFFFFF);
	//glStencilFunc(GL_NOTEQUAL, 0, 0xFFFFFFFF);
	// don't modify the contents of the stencil buffer
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	//go to the 2d mode, and draw a black rectangle...
	Enter2DMode();
	glDisable(GL_TEXTURE_2D);
	//source 1
	glColor4f(0.0f,0.0f,0.0f,0.63f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glBegin(GL_QUADS);
	glVertex3i(0,window_height,0);
	glVertex3i(0,0,0);
	glVertex3i(window_width,0,0);
	glVertex3i(window_width,window_height,0);
	glEnd();

	//source 2
	glStencilFunc(GL_LEQUAL, 2, 0xFFFFFFFF);
	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,0.33f);
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
}

