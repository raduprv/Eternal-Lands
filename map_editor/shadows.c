#include "global.h"
#include <math.h>

GLuint depth_map_id = 0;
int is_day = 1;

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

#include "../e3d_object.h"

void draw_3d_object_shadow(object3d * object_id)
{
	int i;
	float x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;

	//also, update the last time this object was used
	object_id->last_acessed_time=cur_time;

	if(object_id->blended)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE,GL_ONE);
	}

	set_emission(object_id);

	CHECK_GL_ERRORS();

	glPushMatrix();//we don't want to affect the rest of the scene
	x_pos = object_id->x_pos;
	y_pos = object_id->y_pos;
	z_pos = object_id->z_pos;
	glTranslatef (x_pos, y_pos, z_pos);

	x_rot = object_id->x_rot;
	y_rot = object_id->y_rot;
	z_rot = object_id->z_rot;
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	CHECK_GL_ERRORS();

	e3d_enable_vertex_arrays(object_id->e3d_data, 0, 1);
		
	CHECK_GL_ERRORS();

	for (i = 0; i < object_id->e3d_data->material_no; i++)
	{
		if (object_id->e3d_data->materials[i].options)
		{
			//enable alpha filtering, so we have some alpha key
			glEnable(GL_ALPHA_TEST);
			if (object_id->e3d_data->vertex_layout->normal_count == 0) glAlphaFunc(GL_GREATER, 0.23f);
			else glAlphaFunc(GL_GREATER, 0.06f);
			glDisable(GL_CULL_FACE);
			glEnable(GL_TEXTURE_2D);
#ifdef	NEW_TEXTURES
			bind_texture(object_id->e3d_data->materials[i].texture);
#else	/* NEW_TEXTURES */
			get_and_set_texture_id(object_id->e3d_data->materials[i].texture);
#endif	/* NEW_TEXTURES */
		}
		else
		{
			glDisable(GL_ALPHA_TEST);
			glEnable(GL_CULL_FACE);
			glDisable(GL_TEXTURE_2D);
		}

		ELglDrawRangeElementsEXT(GL_TRIANGLES,
			object_id->e3d_data->materials[i].triangles_indices_min,
			object_id->e3d_data->materials[i].triangles_indices_max,
			object_id->e3d_data->materials[i].triangles_indices_count,
			object_id->e3d_data->index_type,
			object_id->e3d_data->materials[i].triangles_indices_index);
	}

	glPopMatrix();//restore the scene
	CHECK_GL_ERRORS();

	e3d_disable_vertex_arrays();
	glDisable(GL_COLOR_MATERIAL);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	if (object_id->e3d_data->materials[object_id->e3d_data->material_no-1].options)
	{
		glDisable(GL_ALPHA_TEST);
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glEnable(GL_TEXTURE_2D);
	}
	CHECK_GL_ERRORS();
}


void display_shadows()
{
	int i;
	int x,y;

	x=(int)-camera_x;
	y=(int)-camera_y;
	glEnable(GL_CULL_FACE);
	for (i = 0; i < MAX_OBJ_3D; i++)
		{
			if(objects_list[i] && objects_list[i]->blended!=20)
				 {
				if ((objects_list[i]->e3d_data->vertex_layout->normal_count > 0)
					&& objects_list[i]->z_pos>-0.20f)
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

	x=(int)-camera_x;
	y=(int)-camera_y;
	glEnable(GL_CULL_FACE);
	for (i = 0; i < MAX_OBJ_3D; i++)
		{
			closest_light=-1;
			next_closest_light=-1;
			closest_light_dist=100.0f;
			next_closest_light_dist=100.0f;

			if(objects_list[i] && objects_list[i]->blended!=20)
				 {
				if ((objects_list[i]->e3d_data->vertex_layout->normal_count > 0)
					&& objects_list[i]->z_pos>-0.20f)
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
							for (j = 0; j < MAX_LIGHTS; j++)
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

	x=(int)-camera_x;
	y=(int)-camera_y;
	glEnable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	if(have_multitexture && clouds_shadows)
		{
			//bind the detail texture
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glEnable(GL_TEXTURE_2D);
#ifdef	NEW_TEXTURES
			bind_texture_unbuffered(ground_detail_text);
#else	/* NEW_TEXTURES */
			glBindTexture(GL_TEXTURE_2D, get_texture_id(ground_detail_text));
#endif	/* NEW_TEXTURES */
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_2D);

		}
	for (i = 0; i < MAX_OBJ_3D; i++)
		{
			if(objects_list[i] && objects_list[i]->blended!=20)
				 {
					if (objects_list[i]->e3d_data->vertex_layout->normal_count > 0)
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

	x=(int)-camera_x;
	y=(int)-camera_y;
	glEnable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	if(have_multitexture && clouds_shadows)
		{
			//bind the detail texture
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glEnable(GL_TEXTURE_2D);
#ifdef	NEW_TEXTURES
			bind_texture_unbuffered(ground_detail_text);
#else	/* NEW_TEXTURES */
			glBindTexture(GL_TEXTURE_2D, get_texture_id(ground_detail_text));
#endif	/* NEW_TEXTURES */
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_2D);

		}
	for (i = 0; i < MAX_OBJ_3D; i++)
		{
			if(objects_list[i] && objects_list[i]->blended!=20)
				 {
					if (objects_list[i]->e3d_data->vertex_layout->normal_count > 0)
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

