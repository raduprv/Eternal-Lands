#include <stdlib.h>
#include <string.h>
#include "global.h"
#include <math.h>

float mrandom(float max)
{
  return ((float) max * (rand () % 8 ));
}

void draw_body_part_reflection(md2 *model_data,char *cur_frame, int ghost)
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

	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_TRIANGLES);
	for(j=0;j<numFaces;j++)
		{
			x=vertex_pointer[offsetFaces[j].a].x;
			y=vertex_pointer[offsetFaces[j].a].y;
			z=vertex_pointer[offsetFaces[j].a].z;

			glTexCoord2f(offsetTexCoords[offsetFaces[j].at].u,offsetTexCoords[offsetFaces[j].at].v);
			glVertex3f(x,y,z);


			x=vertex_pointer[offsetFaces[j].b].x;
			y=vertex_pointer[offsetFaces[j].b].y;
			z=vertex_pointer[offsetFaces[j].b].z;

			glTexCoord2f(offsetTexCoords[offsetFaces[j].bt].u,offsetTexCoords[offsetFaces[j].bt].v);
			glVertex3f(x,y,z);


			x=vertex_pointer[offsetFaces[j].c].x;
			y=vertex_pointer[offsetFaces[j].c].y;
			z=vertex_pointer[offsetFaces[j].c].z;

			glTexCoord2f(offsetTexCoords[offsetFaces[j].ct].u,offsetTexCoords[offsetFaces[j].ct].v);
			glVertex3f(x,y,z);


		}
	glEnd();

}

void draw_actor_reflection(actor * actor_id)
{
	int i,j;
	double x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;
	//float u,v;     unused?
	float x,y,z;
	int texture_id;
	char *cur_frame;
	char *dest_frame_name;
	//char str[20];     unused?
	int numFrames;
    int numFaces;
    text_coord_md2 *offsetTexCoords;
    face_md2 *offsetFaces;
    frame_md2 *offsetFrames;
    vertex_md2 *vertex_pointer=NULL;

	if(!actor_id->remapped_colors)texture_id=texture_cache[actor_id->texture_id].texture_id;
	else
	{
		//we have remaped colors, we don't store such textures into the cache
		texture_id=actor_id->texture_id;
	}

	cur_frame=actor_id->cur_frame;

	numFaces=actor_id->model_data->numFaces;
	numFrames=actor_id->model_data->numFrames;
	offsetFaces=actor_id->model_data->offsetFaces;
	offsetTexCoords=actor_id->model_data->offsetTexCoords;
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

	if(last_texture!=texture_id)
		{
			glBindTexture(GL_TEXTURE_2D, texture_id);
			last_texture=texture_id;
		}

	if(z_pos==0.0f)//actor is walking, as opposed to flying, get the height underneath
	z_pos=-2.2f+height_map[actor_id->y_tile_pos*tile_map_size_x*6+actor_id->x_tile_pos]*0.2f;

	glPushMatrix();//we don't want to affect the rest of the scene
	glTranslatef(x_pos+0.25f, y_pos+0.25f, z_pos);
	z_rot=-z_rot;
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_TRIANGLES);
	for(j=0;j<numFaces;j++)
		{
			x=vertex_pointer[offsetFaces[j].a].x;
			y=vertex_pointer[offsetFaces[j].a].y;
			z=vertex_pointer[offsetFaces[j].a].z;

			glTexCoord2f(offsetTexCoords[offsetFaces[j].at].u,offsetTexCoords[offsetFaces[j].at].v);
			glVertex3f(x,y,z);


			x=vertex_pointer[offsetFaces[j].b].x;
			y=vertex_pointer[offsetFaces[j].b].y;
			z=vertex_pointer[offsetFaces[j].b].z;

			glTexCoord2f(offsetTexCoords[offsetFaces[j].bt].u,offsetTexCoords[offsetFaces[j].bt].v);
			glVertex3f(x,y,z);


			x=vertex_pointer[offsetFaces[j].c].x;
			y=vertex_pointer[offsetFaces[j].c].y;
			z=vertex_pointer[offsetFaces[j].c].z;

			glTexCoord2f(offsetTexCoords[offsetFaces[j].ct].u,offsetTexCoords[offsetFaces[j].ct].v);
			glVertex3f(x,y,z);


		}
	glEnd();

	glPopMatrix();
}

void draw_enhanced_actor_reflection(actor * actor_id)
{
	//int i,j;     unused?
	double x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;
	//float x,y,z;     unused?
	int texture_id;
	char *cur_frame;
	//char str[20];     unused?
	//int numFrames;     unused?
	//char *dest_frame_name;     unused?
	frame_md2 *offsetFrames;



	offsetFrames=actor_id->body_parts->head->offsetFrames;
	texture_id=actor_id->texture_id;
	//texture_id=texture_cache[actor_id->texture_id].texture_id;

	cur_frame=actor_id->cur_frame;

	x_pos=actor_id->x_pos;
	y_pos=actor_id->y_pos;
	z_pos=actor_id->z_pos;

	x_rot=actor_id->x_rot;
	y_rot=actor_id->y_rot;
	z_rot=actor_id->z_rot;

	z_rot+=180;//test

	if(last_texture!=texture_id)
		{
			glBindTexture(GL_TEXTURE_2D, texture_id);
			last_texture=texture_id;
		}
	if(z_pos==0.0f)//actor is walking, as opposed to flying, get the height underneath
	z_pos=-2.2f+height_map[actor_id->y_tile_pos*tile_map_size_x*6+actor_id->x_tile_pos]*0.2f;

	//z_pos-=water_deepth_offset*2;

	glPushMatrix();//we don't want to affect the rest of the scene
	glTranslatef(x_pos+0.25f, y_pos+0.25f, z_pos);
	z_rot=-z_rot;
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	if(actor_id->body_parts->legs)draw_body_part_reflection(actor_id->body_parts->legs,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->torso)draw_body_part_reflection(actor_id->body_parts->torso,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->head)draw_body_part_reflection(actor_id->body_parts->head,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->weapon)draw_body_part_reflection(actor_id->body_parts->weapon,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->shield)draw_body_part_reflection(actor_id->body_parts->shield,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->helmet)draw_body_part_reflection(actor_id->body_parts->helmet,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->cape)draw_body_part_reflection(actor_id->body_parts->cape,cur_frame,actor_id->ghost);


	//////
	glPopMatrix();//restore the scene
}



void draw_3d_reflection(object3d * object_id)
{
	//float x,y,z,u,v;     unused?
	float x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;

	int materials_no,texture_id;//a,b,c,faces_no     unused?
	int i; //,k,j;     unused?

	e3d_array_vertex *array_vertex;
	e3d_array_normal *array_normal;
	e3d_array_uv_main *array_uv_main;
	//e3d_array_uv_detail *clouds_uv;     unused?
	e3d_array_order *array_order;

	int is_transparent;

	is_transparent=object_id->e3d_data->is_transparent;
	materials_no=object_id->e3d_data->materials_no;


	array_vertex=object_id->e3d_data->array_vertex;
	array_normal=object_id->e3d_data->array_normal;
	array_uv_main=object_id->e3d_data->array_uv_main;
	array_order=object_id->e3d_data->array_order;


	x_pos=object_id->x_pos;
	y_pos=object_id->y_pos;
	z_pos=object_id->z_pos;

	if(z_pos<0)z_pos+=-water_deepth_offset*2;

	x_rot=object_id->x_rot;
	y_rot=object_id->y_rot;
	z_rot=object_id->z_rot;

  if(object_id->self_lit && (night_shadows_on || dungeon))
  {
 	glDisable(GL_LIGHTING);
    glColor3f(object_id->r,object_id->g,object_id->b);
  }

  if(is_transparent)
  	{
	    glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
	    glAlphaFunc(GL_GREATER,0.05f);
	    //glDisable(GL_CULL_FACE);

	}


	glPushMatrix();//we don't want to affect the rest of the scene
	glTranslatef (x_pos, y_pos,z_pos);
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);


			glVertexPointer(3,GL_FLOAT,0,array_vertex);
			glTexCoordPointer(2,GL_FLOAT,0,array_uv_main);
			glNormalPointer(GL_FLOAT,0,array_normal);
				for(i=0;i<materials_no;i++)
					{
						texture_id=get_texture_id(array_order[i].texture_id);
    					if(last_texture!=texture_id)
   					 		{
								glBindTexture(GL_TEXTURE_2D, texture_id);
								last_texture=texture_id;
							}
						glDrawArrays(GL_TRIANGLES,array_order[i].start,array_order[i].count);
					  }

	glPopMatrix();//restore the scene


  if(object_id->self_lit && (night_shadows_on || dungeon))glEnable(GL_LIGHTING);
  if(is_transparent)
  	{
  		glDisable(GL_ALPHA_TEST);
  		//glEnable(GL_CULL_FACE);
	}

}

//if there is any reflecting tile, returns 1, otherwise 0
int find_reflection()
{
	int x_start,x_end,y_start,y_end;
	int x,y;
	float x_scaled,y_scaled;

	//get only the tiles around the camera
	//we have the axes inverted, btw the go from 0 to -255
	if(cx<0)x=(cx*-1)/3;
	else x=cx/3;
	if(cy<0)y=(cy*-1)/3;
	else y=cy/3;
	x_start=(int)x-4;
	y_start=(int)y-4;
	x_end=(int)x+4;
	y_end=(int)y+4;
	if(x_start<0)x_start=0;
	if(x_end>=tile_map_size_x)x_end=tile_map_size_x-1;
	if(y_start<0)y_start=0;
	if(y_end>=tile_map_size_y)y_end=tile_map_size_y-1;
	for(y=y_start;y<=y_end;y++)
		{
			y_scaled=y*3.0f;
			for(x=x_start;x<=x_end;x++)
				{
					x_scaled=x*3.0f;
					if(!check_tile_in_frustrum(x_scaled,y_scaled))continue;//outside of the frustrum
					if(!tile_map[y*tile_map_size_x+x])return 1;
				}
		}
	return 0;
}

int find_local_reflection(int x_pos,int y_pos,int range)
{
	int x_start,x_end,y_start,y_end;
	int x,y;

	//get only the tiles around the camera
	//we have the axes inverted, btw the go from 0 to -255
	if(x_pos<0)x=(x_pos*-1)/3;
	else x=x_pos/3;
	if(y_pos<0)y=(y_pos*-1)/3;
	else y=y_pos/3;
	x_start=(int)x-range;
	y_start=(int)y-range;
	x_end=(int)x+range;
	y_end=(int)y+range;
	if(x_start<0)x_start=0;
	if(x_end>=tile_map_size_x)x_end=tile_map_size_x-1;
	if(y_start<0)y_start=0;
	if(y_end>=tile_map_size_y)y_end=tile_map_size_y-1;
	for(y=y_start;y<=y_end;y++)
		{
			for(x=x_start;x<=x_end;x++)
				{
					if(!tile_map[y*tile_map_size_x+x])return 1;
				}
		}
return 0;
}


void display_3d_reflection()
{
	int i;
	int x,y;
	double water_clipping_p[4]={0,0,-1,water_deepth_offset};
	float window_ratio;

	window_ratio=(GLfloat)window_width/(GLfloat)window_height;


	x=-cx;
	y=-cy;
/*
	//test
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();
	glOrtho( -3.0*window_ratio, 3.0*window_ratio, 6.0, 0.0, -40.0, 40.0 );
	glMatrixMode(GL_MODELVIEW);						// Select The Modelview Matrix

*/
	glEnable(GL_CLIP_PLANE0);
	glClipPlane(GL_CLIP_PLANE0, water_clipping_p);

	glDisable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	set_material(0.1f,0.2f,0.3f);
	glPushMatrix();
	glScalef(1.0f, 1.0f, -1.0f);
	//first, render only the submerged objects, with the clipping plane enabled
	for(i=0;i<max_obj_3d;i++)
		{
			if(objects_list[i])
			     {
			         int dist1;
			         int dist2;
			         float dist;

					 if(!objects_list[i]->e3d_data->is_ground)
					 	{
			         		dist1=x-objects_list[i]->x_pos;
			         		dist2=y-objects_list[i]->y_pos;
			         		dist=dist1*dist1+dist2*dist2;
			         		if(dist<=21*21)
			         			{
									float x_len;
									float y_len;
									float z_len;
									float radius;

									z_len=objects_list[i]->e3d_data->max_z-objects_list[i]->e3d_data->min_z;
									x_len=objects_list[i]->e3d_data->max_x-objects_list[i]->e3d_data->min_x;
									y_len=objects_list[i]->e3d_data->max_y-objects_list[i]->e3d_data->min_y;
									//do some checks, to see if we really have to display this object
									if(x_len<5 && y_len<5 && z_len<4 && !find_local_reflection(objects_list[i]->x_pos,objects_list[i]->y_pos,1))continue;

									radius=x_len/2;
									if(radius<y_len/2)radius=y_len/2;
									if(radius<z_len)radius=z_len;
									//not in the middle of the air
									if(SphereInFrustum(objects_list[i]->x_pos,objects_list[i]->y_pos,
									objects_list[i]->z_pos,radius))
                     				draw_3d_reflection(objects_list[i]);
								}
						}
                 }
		}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glNormal3f(0.0f,0.0f,1.0f);
	for(i=0;i<max_actors;i++)
		{
			if(actors_list[i])
			 {
			         int dist1;
			         int dist2;

			         dist1=x-actors_list[i]->x_pos;
			         dist2=y-actors_list[i]->y_pos;
			         if(dist1*dist1+dist2*dist2<=100)
			         if(!actors_list[i]->ghost)
			         	{
							if(actors_list[i]->is_enhanced_model)
							draw_enhanced_actor_reflection(actors_list[i]);
                     		else draw_actor_reflection(actors_list[i]);
						}
             }
		}
	glPopMatrix();
	reset_material();
/*
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();
	glOrtho( -3.0*window_ratio, 3.0*window_ratio, -3.0, 3.0, -40.0, 40.0 );
	glMatrixMode(GL_MODELVIEW);						// Select The Modelview Matrix
*/

	glDisable(GL_CLIP_PLANE0);
}

void make_lake_water_noise()
{
	int x,y;
	float noise_u,noise_v,noise_z;

	if(no_sound)return;	//ignore the noise if no sound
	for(x=0;x<16;x++)
	for(y=0;y<16;y++)
		{

			noise_u=mrandom(0.001f);
			noise_v=mrandom(0.001f);
			noise_z=mrandom(0.005f);
			if(noise_z<=0)noise_z=-noise_z;

			noise_array[y*16+x].u=noise_u;
			noise_array[y*16+x].v=noise_v;
			noise_array[y*16+x].z=noise_z;

		}

}

void draw_lake_water_tile(float x_pos, float y_pos)
{
	int x,y;
	float fx,fy;
	float x_step,y_step;
	float u_step,v_step;
	float uv_tile=50;

	x_step=3.0f/16.0f;
	y_step=3.0f/16.0f;

	u_step=3.0f/uv_tile;
	v_step=3.0f/uv_tile;

	glBegin(GL_QUADS);
	for(y=0,fy=y_pos;y<16;fy+=y_step,y++)
		{
			for(x=0,fx=x_pos;x<16;fx+=x_step,x++)
				{
					/*
					if(y==15 && x!=15)
					{
 					glTexCoord2f((x_pos+x*x_step)*u_step+noise_array[(y-15)*16+x].u+water_movement_u, (y_pos+y*y_step+y_step)*v_step+noise_array[(y-15)*16+x].v+water_movement_v);
	 				glVertex3f(x_pos+x*x_step,y_pos+y*y_step+y_step, water_deepth_offset);

					glTexCoord2f((x_pos+x*x_step)*u_step+noise_array[y*16+x].u+water_movement_u, (y_pos+y*y_step)*v_step+noise_array[y*16+x].v+water_movement_v);
					glVertex3f(x_pos+x*x_step,y_pos+y*y_step, water_deepth_offset);

					glTexCoord2f((x_pos+x*x_step+x_step)*u_step+noise_array[y*16+x+1].u+water_movement_u, (y_pos+y*y_step)*v_step+noise_array[y*16+x+1].v+water_movement_v);
					glVertex3f(x_pos+x*x_step+x_step, y_pos+y*y_step,water_deepth_offset);

					glTexCoord2f((x_pos+x*x_step+x_step)*u_step+noise_array[(y-15)*16+x+1].u+water_movement_u, (y_pos+y*y_step+y_step)*v_step+noise_array[(y-15)*16+x+1].v+water_movement_v);
					glVertex3f(x_pos+x*x_step+x_step, y_pos+y*y_step+y_step,water_deepth_offset);
					}
					else if(y!=15 && x==15)
					{
 					glTexCoord2f((x_pos+x*x_step)*u_step+noise_array[(y+1)*16+x].u+water_movement_u, (y_pos+y*y_step+y_step)*v_step+noise_array[(y+1)*16+x].v+water_movement_v);
	 				glVertex3f(x_pos+x*x_step,y_pos+y*y_step+y_step, water_deepth_offset);

					glTexCoord2f((x_pos+x*x_step)*u_step+noise_array[y*16+x].u+water_movement_u, (y_pos+y*y_step)*v_step+noise_array[y*16+x].v+water_movement_v);
					glVertex3f(x_pos+x*x_step,y_pos+y*y_step, water_deepth_offset);

					glTexCoord2f((x_pos+x*x_step+x_step)*u_step+noise_array[y*16+x-15].u+water_movement_u, (y_pos+y*y_step)*v_step+noise_array[y*16+x-15].v+water_movement_v);
					glVertex3f(x_pos+x*x_step+x_step, y_pos+y*y_step,water_deepth_offset);

					glTexCoord2f((x_pos+x*x_step+x_step)*u_step+noise_array[(y+1)*16+x-15].u+water_movement_u, (y_pos+y*y_step+y_step)*v_step+noise_array[(y+1)*16+x-15].v+water_movement_v);
					glVertex3f(x_pos+x*x_step+x_step, y_pos+y*y_step+y_step,water_deepth_offset);
					}
					else if(y==15 && x==15)
					{
 					glTexCoord2f((x_pos+x*x_step)*u_step+noise_array[(y-15)*16+x].u+water_movement_u, (y_pos+y*y_step+y_step)*v_step+noise_array[(y-15)*16+x].v+water_movement_v);
	 				glVertex3f(x_pos+x*x_step,y_pos+y*y_step+y_step, water_deepth_offset);

					glTexCoord2f((x_pos+x*x_step)*u_step+noise_array[y*16+x].u+water_movement_u, (y_pos+y*y_step)*v_step+noise_array[y*16+x].v+water_movement_v);
					glVertex3f(x_pos+x*x_step,y_pos+y*y_step, water_deepth_offset);

					glTexCoord2f((x_pos+x*x_step+x_step)*u_step+noise_array[y*16+x-15].u+water_movement_u, (y_pos+y*y_step)*v_step+noise_array[y*16+x-15].v+water_movement_v);
					glVertex3f(x_pos+x*x_step+x_step, y_pos+y*y_step,water_deepth_offset);

					glTexCoord2f((x_pos+x*x_step+x_step)*u_step+noise_array[(y-15)*16+x-15].u+water_movement_u, (y_pos+y*y_step+y_step)*v_step+noise_array[(y-15)*16+x-15].v+water_movement_v);
					glVertex3f(x_pos+x*x_step+x_step, y_pos+y*y_step+y_step,water_deepth_offset);
					}
					else
					{
 					glTexCoord2f((x_pos+x*x_step)*u_step+noise_array[(y+1)*16+x].u+water_movement_u, (y_pos+y*y_step+y_step)*v_step+noise_array[(y+1)*16+x].v+water_movement_v);
	 				glVertex3f(x_pos+x*x_step,y_pos+y*y_step+y_step, water_deepth_offset);

					glTexCoord2f((x_pos+x*x_step)*u_step+noise_array[y*16+x].u+water_movement_u, (y_pos+y*y_step)*v_step+noise_array[y*16+x].v+water_movement_v);
					glVertex3f(x_pos+x*x_step,y_pos+y*y_step, water_deepth_offset);

					glTexCoord2f((x_pos+x*x_step+x_step)*u_step+noise_array[y*16+x+1].u+water_movement_u, (y_pos+y*y_step)*v_step+noise_array[y*16+x+1].v+water_movement_v);
					glVertex3f(x_pos+x*x_step+x_step, y_pos+y*y_step,water_deepth_offset);

					glTexCoord2f((x_pos+x*x_step+x_step)*u_step+noise_array[(y+1)*16+x+1].u+water_movement_u, (y_pos+y*y_step+y_step)*v_step+noise_array[(y+1)*16+x+1].v+water_movement_v);
					glVertex3f(x_pos+x*x_step+x_step, y_pos+y*y_step+y_step,water_deepth_offset);
					}
					*/
 					glTexCoord2f((fx)*u_step+noise_array[((y+1)&15)*16+x].u+water_movement_u, (fy+y_step)*v_step+noise_array[((y+1)&15)*16+x].v+water_movement_v);
	 				glVertex3f(fx,fy+y_step, water_deepth_offset);

					glTexCoord2f((fx)*u_step+noise_array[y*16+x].u+water_movement_u, (fy)*v_step+noise_array[y*16+x].v+water_movement_v);
					glVertex3f(fx,fy, water_deepth_offset);

					glTexCoord2f((fx+x_step)*u_step+noise_array[y*16+((x+1)&15)].u+water_movement_u, (fy)*v_step+noise_array[y*16+((x+1)&15)].v+water_movement_v);
					glVertex3f(fx+x_step, fy,water_deepth_offset);

					glTexCoord2f((fx+x_step)*u_step+noise_array[((y+1)&15)*16+((x+1)&15)].u+water_movement_u, (fy+y_step)*v_step+noise_array[((y+1)&15)*16+((x+1)&15)].v+water_movement_v);
					glVertex3f(fx+x_step, fy+y_step,water_deepth_offset);

				}

		}
	glEnd();
}


void draw_lake_tiles()
{
	int x_start,x_end,y_start,y_end;
	int x,y;
	float x_scaled,y_scaled;
	//int cur_texture;     unused?

	//glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);


	if(last_texture!=texture_cache[sky_text_1].texture_id)
		{
			glBindTexture(GL_TEXTURE_2D, texture_cache[sky_text_1].texture_id);
			last_texture=texture_cache[sky_text_1].texture_id;
		}


	//get only the tiles around the camera
	//we have the axes inverted, btw the go from 0 to -255
	if(cx<0)x=(cx*-1)/3;
	else x=cx/3;
	if(cy<0)y=(cy*-1)/3;
	else y=cy/3;
	x_start=(int)x-5;
	y_start=(int)y-5;
	x_end=(int)x+5;
	y_end=(int)y+5;
	if(x_start<0)x_start=0;
	if(x_end>=tile_map_size_x)x_end=tile_map_size_x-1;
	if(y_start<0)y_start=0;
	if(y_end>=tile_map_size_y)y_end=tile_map_size_y-1;
	for(y=y_start;y<=y_end;y++)
		{
			y_scaled=y*3.0f;
			for(x=x_start;x<=x_end;x++)
				{
					x_scaled=x*3.0f;
					if(!check_tile_in_frustrum(x_scaled,y_scaled))continue;//outside of the frustrum
					if(!tile_map[y*tile_map_size_x+x])draw_lake_water_tile(x_scaled,y_scaled);
				}
		}

	//glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

void draw_sky_background()
{
	Enter2DMode();
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	#define scale_factor 100

			glColor3f(sky_lights_c1[light_level][0]-(float)weather_light_offset/scale_factor,
			sky_lights_c1[light_level][1]-(float)weather_light_offset/scale_factor,
			sky_lights_c1[light_level][2]-(float)weather_light_offset/scale_factor);
			glVertex3i(0,0,0);

			glColor3f(sky_lights_c2[light_level][0]-(float)weather_light_offset/scale_factor,
			sky_lights_c2[light_level][1]-(float)weather_light_offset/scale_factor,
			sky_lights_c2[light_level][2]-(float)weather_light_offset/scale_factor);
			glVertex3i(0,window_height,0);

			glColor3f(sky_lights_c3[light_level][0]-(float)weather_light_offset/scale_factor,
			sky_lights_c3[light_level][1]-(float)weather_light_offset/scale_factor,
			sky_lights_c3[light_level][2]-(float)weather_light_offset/scale_factor);
			glVertex3i(window_width,window_height,0);

			glColor3f(sky_lights_c4[light_level][0]-(float)weather_light_offset/scale_factor,
			sky_lights_c4[light_level][1]-(float)weather_light_offset/scale_factor,
			sky_lights_c4[light_level][2]-(float)weather_light_offset/scale_factor);
			glVertex3i(window_width,0,0);

	glEnd();
	glEnable(GL_TEXTURE_2D);
	Leave2DMode();


}

void draw_dungeon_sky_background()
{
	Enter2DMode();
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	//draw the sky background

			glColor3f(0,0.21f,0.34f);
			glVertex3i(0,0,0);

			glColor3f(0,0.21f,0.34f);
			glVertex3i(0,window_height,0);

			glColor3f(0,0.21f,0.34f);
			glVertex3i(window_width,window_height,0);

			glColor3f(0,0.21f,0.34f);
			glVertex3i(window_width,0,0);

	glEnd();
	glEnable(GL_TEXTURE_2D);
	Leave2DMode();


}



