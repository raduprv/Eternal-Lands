#include <string.h>
#include "global.h"
#include <math.h>

void SetShadowMatrix()

{

               float dot;
               // dot product of plane and light position
               dot =      fPlane[0] * fLightPos[0] +
                          fPlane[1] * fLightPos[1] +
                          fPlane[2] * fLightPos[2] +
                          fPlane[3] * fLightPos[3];



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

	//float x,y,z,u,v;     unused?
	float x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;

	int materials_no,texture_id;//a,b,c     unused?
	int i;//k     unused?
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

	x_pos=object_id->x_pos;
	y_pos=object_id->y_pos;
	z_pos=object_id->z_pos;

	x_rot=object_id->x_rot;
	y_rot=object_id->y_rot;
	z_rot=object_id->z_rot;

  if(is_transparent)
  	{
	    glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
	    glAlphaFunc(GL_GREATER,0.05f);
	}
	else glDisable(GL_TEXTURE_2D);//we don't need textures for non transparent objects


	glPushMatrix();//we don't want to affect the rest of the scene
	glMultMatrixf(fDestMat);
	glTranslatef (x_pos, y_pos, z_pos);
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
							glBindTexture(GL_TEXTURE_2D, texture_id);
							last_texture=texture_id;
						}
				}
			glDrawArrays(GL_TRIANGLES,array_order[i].start,array_order[i].count);
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void draw_body_part_shadow(md2 *model_data,char *cur_frame, int ghost)
{
	int i,j;
	//float u,v;     unused?
	float x,y,z;
	char *dest_frame_name;
	//char str[20];     unused?
	int numFrames;
    int numFaces;
    text_coord_md2 *offsetTexCoords;
    face_md2 *offsetFaces;
    frame_md2 *offsetFrames;
    vertex_md2 *vertex_pointer=NULL;

	numFaces=model_data->numFaces;
	numFrames=model_data->numFrames;
	offsetFaces=model_data->offsetFaces;
	offsetTexCoords=model_data->offsetTexCoords;
	offsetFrames=model_data->offsetFrames;


	//now, go and find the current frame
	i=0;
	while(i<numFrames)
	{
		dest_frame_name=(char *)&offsetFrames[i].name;
		//dest_frame_name=offsetFrames[i].name;
		if(strcmp(cur_frame,dest_frame_name)==0)//we found the current frame
			{
				vertex_pointer=offsetFrames[i].vertex_pointer;
				break;
			}
		i++;
	}

	i=0;
	if(vertex_pointer==NULL)//if there is no frame, use idle01
		{
			char str[120];
			sprintf(str, "couldn't find frame: %s\n",cur_frame);
			log_error(str);
			while(i<numFrames)
			{
				dest_frame_name=(char *)&offsetFrames[i].name;
				if(strcmp("idle01",dest_frame_name)==0)//we found the current frame
					{
						vertex_pointer=offsetFrames[i].vertex_pointer;
						break;
					}
				i++;
			}
		}

	if(vertex_pointer==NULL)// this REALLY shouldn't happen...
		{
			char str[120];
			sprintf(str, "couldn't find frame: %s\n",cur_frame);
			log_error(str);
			return;
		}
	glBegin(GL_TRIANGLES);
	for(j=0;j<numFaces;j++)
		{
			x=vertex_pointer[offsetFaces[j].a].x;
			y=vertex_pointer[offsetFaces[j].a].y;
			z=vertex_pointer[offsetFaces[j].a].z;
			glVertex3f(x,y,z);
			x=vertex_pointer[offsetFaces[j].b].x;
			y=vertex_pointer[offsetFaces[j].b].y;
			z=vertex_pointer[offsetFaces[j].b].z;
			glVertex3f(x,y,z);
			x=vertex_pointer[offsetFaces[j].c].x;
			y=vertex_pointer[offsetFaces[j].c].y;
			z=vertex_pointer[offsetFaces[j].c].z;
			glVertex3f(x,y,z);


		}
	glEnd();

}




void draw_enhanced_actor_shadow(actor * actor_id)
{
	//int i,j;     unused?
	double x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;
	//float x,y,z;     unused?
	char *cur_frame;
	//char str[20];     unused?
	//int numFrames;     unused?
	//char *dest_frame_name;     unused?
	frame_md2 *offsetFrames;



	offsetFrames=actor_id->body_parts->head->offsetFrames;
	//texture_id=texture_cache[actor_id->texture_id].texture_id;

	cur_frame=actor_id->cur_frame;

	x_pos=actor_id->x_pos;
	y_pos=actor_id->y_pos;
	z_pos=actor_id->z_pos;

	x_rot=actor_id->x_rot;
	y_rot=actor_id->y_rot;
	z_rot=actor_id->z_rot;

	z_rot+=180;//test

	//now, go and find the current frame

	if(z_pos==0.0f)//actor is walking, as opposed to flying, get the height underneath
	z_pos=-2.2f+height_map[actor_id->y_tile_pos*tile_map_size_x*6+actor_id->x_tile_pos]*0.2f;

	glPushMatrix();//we don't want to affect the rest of the scene
	glMultMatrixf(fDestMat);
	glTranslatef(x_pos+0.25f, y_pos+0.25f, z_pos);
	z_rot=-z_rot;
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	if(actor_id->body_parts->legs)draw_body_part_shadow(actor_id->body_parts->legs,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->torso)draw_body_part_shadow(actor_id->body_parts->torso,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->head)draw_body_part_shadow(actor_id->body_parts->head,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->weapon)draw_body_part_shadow(actor_id->body_parts->weapon,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->shield)draw_body_part_shadow(actor_id->body_parts->shield,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->helmet)draw_body_part_shadow(actor_id->body_parts->helmet,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->cape)draw_body_part_shadow(actor_id->body_parts->cape,cur_frame,actor_id->ghost);

	glPopMatrix();//restore the scene
}

void draw_actor_shadow(actor * actor_id)
{
	int i,j;
	double x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;
	//float u,v;     unused?
	float x,y,z;
	char *cur_frame;
	char *dest_frame_name;
	//char str[20];     unused?


	int numFrames;
    int numFaces;
    face_md2 *offsetFaces;
    frame_md2 *offsetFrames;
    vertex_md2 *vertex_pointer=NULL;

	cur_frame=actor_id->cur_frame;

	numFaces=actor_id->model_data->numFaces;
	numFrames=actor_id->model_data->numFrames;
	offsetFaces=actor_id->model_data->offsetFaces;
	offsetFrames=actor_id->model_data->offsetFrames;

	x_pos=actor_id->x_pos;
	y_pos=actor_id->y_pos;
	z_pos=actor_id->z_pos;

	x_rot=actor_id->x_rot;
	y_rot=actor_id->y_rot;
	z_rot=actor_id->z_rot;

	//now, go and find the current frame
	i=0;
	while(i<numFrames)
	{
		dest_frame_name=(char *)&offsetFrames[i].name;
		//dest_frame_name=offsetFrames[i].name;
		if(strcmp(cur_frame,dest_frame_name)==0)//we found the current frame
			{
				vertex_pointer=offsetFrames[i].vertex_pointer;
				break;
			}
		i++;
	}
	if(vertex_pointer==NULL)// this REALLY shouldn't happen...
		{
			char str[120];
			sprintf(str, "couldn't find frame: %s\n",cur_frame);
			log_error(str);
			return;
		}

	if(z_pos==0.0f)//actor is walking, as opposed to flying, get the height underneath
	z_pos=-2.2f+height_map[actor_id->y_tile_pos*tile_map_size_x*6+actor_id->x_tile_pos]*0.2f;

	glPushMatrix();//we don't want to affect the rest of the scene
	glMultMatrixf(fDestMat);
	glTranslatef(x_pos+0.25f, y_pos+0.25f, z_pos);
	z_rot=-z_rot;
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);
;
	glBegin(GL_TRIANGLES);
	for(j=0;j<numFaces;j++)
		{
			x=vertex_pointer[offsetFaces[j].a].x;
			y=vertex_pointer[offsetFaces[j].a].y;
			z=vertex_pointer[offsetFaces[j].a].z;
			glVertex3f(x,y,z);
			x=vertex_pointer[offsetFaces[j].b].x;
			y=vertex_pointer[offsetFaces[j].b].y;
			z=vertex_pointer[offsetFaces[j].b].z;
			glVertex3f(x,y,z);
			x=vertex_pointer[offsetFaces[j].c].x;
			y=vertex_pointer[offsetFaces[j].c].y;
			z=vertex_pointer[offsetFaces[j].c].z;
			glVertex3f(x,y,z);


		}
	glEnd();

	glPopMatrix();//restore the scene
}

void display_actors_shadow()
{
	int i;
	int x,y;
	x=-cx;
	y=-cy;
	for(i=0;i<1000;i++)
		{
			if(actors_list[i])
			 {
			         int dist1;
			         int dist2;

			         dist1=x-actors_list[i]->x_pos;
			         dist2=y-actors_list[i]->y_pos;
			         if(sqrt(dist1*dist1+dist2*dist2)<=12)
			         if(!actors_list[i]->ghost)
			         	{
							if(actors_list[i]->is_enhanced_model)
							draw_enhanced_actor_shadow(actors_list[i]);
                     		else draw_actor_shadow(actors_list[i]);
						}
             }
		}
}

void display_shadows()
{
	int i;
	int x,y;
	x=-cx;
	y=-cy;
	glEnable(GL_CULL_FACE);
	for(i=0;i<max_obj_3d;i++)
		{
			if(objects_list[i])
			     {
				if(!objects_list[i]->e3d_data->is_ground && objects_list[i]->z_pos>-0.20f)
					{
			    	     int dist1;
			    	     int dist2;

			    	     dist1=x-objects_list[i]->x_pos;
			    	     dist2=y-objects_list[i]->y_pos;
			    	     if(dist1*dist1+dist2*dist2<=400)
                	     draw_3d_object_shadow(objects_list[i]);
					 }
                 }
		}
	glDisable(GL_CULL_FACE);
	display_actors_shadow();
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
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,  texture_cache[ground_detail_text].texture_id);
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_2D);

		}
	for(i=0;i<max_obj_3d;i++)
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
	x=-cx;
	y=-cy;

	//we don't want to be affected by 2d objects and shadows
	anything_under_the_mouse(i,UNDER_MOUSE_NOTHING);

	glEnable(GL_CULL_FACE);
	glEnable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	if(have_multitexture && clouds_shadows)
		{
			//bind the detail texture
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,  texture_cache[ground_detail_text].texture_id);
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_2D);

		}
	for(i=0;i<max_obj_3d;i++)
		{
			if(objects_list[i])
			     {
			         int dist1;
			         int dist2;

			         dist1=x-objects_list[i]->x_pos;
			         dist2=y-objects_list[i]->y_pos;
					 if(!objects_list[i]->e3d_data->is_ground)
			         		if(dist1*dist1+dist2*dist2<=900)
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
									{
                     					draw_3d_object(objects_list[i]);
                     					anything_under_the_mouse(i,UNDER_MOUSE_3D_OBJ);
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
	glDisable(GL_CULL_FACE);
}

void draw_sun_shadowed_scene()
{
	int abs_light;

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

        			abs_light=light_level;
        			if(light_level>59)abs_light=119-light_level;
     				abs_light+=weather_light_offset;
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
