#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "global.h"


void draw_3d_object(object3d * object_id)
{
	//float x,y,z,u,v; unused?
	float x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;

	int materials_no,texture_id; //,a,b,c; unused?
	int i; //,k; unused?

	e3d_array_vertex *array_vertex;
	e3d_array_normal *array_normal;
	e3d_array_uv_main *array_uv_main;
	e3d_array_uv_detail *clouds_uv;
	e3d_array_order *array_order;

	int is_transparent;
	int is_ground;

	is_transparent=object_id->e3d_data->is_transparent;
	is_ground=object_id->e3d_data->is_ground;
	materials_no=object_id->e3d_data->materials_no;


	array_vertex=object_id->e3d_data->array_vertex;
	array_normal=object_id->e3d_data->array_normal;
	array_uv_main=object_id->e3d_data->array_uv_main;
	array_order=object_id->e3d_data->array_order;

	check_gl_errors();
	if(have_multitexture && clouds_shadows)
		if(!object_id->clouds_uv)
			compute_clouds_map(object_id);

	check_gl_errors();
	//also, update the last time this object was used
	object_id->last_acessed_time=cur_time;

	clouds_uv=object_id->clouds_uv;

	x_pos=object_id->x_pos;
	y_pos=object_id->y_pos;
	z_pos=object_id->z_pos;

	x_rot=object_id->x_rot;
	y_rot=object_id->y_rot;
	z_rot=object_id->z_rot;

	//debug

	if(object_id->blended)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE,GL_ONE);
		}

	if(object_id->self_lit && (night_shadows_on || dungeon))
		{
			glDisable(GL_LIGHTING);
			//set_material(object_id->r,object_id->g,object_id->b);
			glColor3f(object_id->r,object_id->g,object_id->b);
		}

	if(is_transparent)
		{
			//enable alpha filtering, so we have some alpha key
			glEnable(GL_ALPHA_TEST);
			if(is_ground)glAlphaFunc(GL_GREATER,0.23f);
			else glAlphaFunc(GL_GREATER,0.06f);
			glDisable(GL_CULL_FACE);
		}
	check_gl_errors();


	glPushMatrix();//we don't want to affect the rest of the scene
	glTranslatef (x_pos, y_pos, z_pos);
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	check_gl_errors();
	if(!have_multitexture || !clouds_shadows)
		{
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glVertexPointer(3,GL_FLOAT,0,array_vertex);
			glTexCoordPointer(2,GL_FLOAT,0,array_uv_main);
			if(!is_ground)
				{
					glEnableClientState(GL_NORMAL_ARRAY);
					glNormalPointer(GL_FLOAT,0,array_normal);
	check_gl_errors();
					for(i=0;i<materials_no;i++)
						{
	check_gl_errors();
							texture_id=get_texture_id(array_order[i].texture_id);
							if(last_texture!=texture_id)
								{
									glBindTexture(GL_TEXTURE_2D, texture_id);
									last_texture=texture_id;
								}
#ifdef	DEBUG
							// a quick check for errors
							if(array_order[i].start < 0 || array_order[i].count <= 0)
								{
									char str[256];
									sprintf(str, "%s[%d] values (%d, %d)\n",
										object_id->file_name, i,
										array_order[i].start, array_order[i].count);
									LogError(str);
								}
#endif	// DEBUG
							glDrawArrays(GL_TRIANGLES,array_order[i].start,
										 array_order[i].count);
	check_gl_errors();
						}
					glDisableClientState(GL_NORMAL_ARRAY);
				}//is ground
			else
				{
					glNormal3f(0,0,1);

					for(i=0;i<materials_no;i++)
						{
	check_gl_errors();
							texture_id=get_texture_id(array_order[i].texture_id);
    						if(last_texture!=texture_id)
   						 		{
									glBindTexture(GL_TEXTURE_2D, texture_id);
									last_texture=texture_id;
								}
#ifdef	DEBUG
							// a quick check for errors
							if(array_order[i].start < 0 || array_order[i].count <= 0)
								{
									char str[256];
									sprintf(str, "%s[%d] values (%d, %d)\n",
										object_id->file_name, i,
										array_order[i].start, array_order[i].count);
									LogError(str);
								}
#endif	// DEBUG
							glDrawArrays(GL_TRIANGLES,array_order[i].start,
										 array_order[i].count);
	check_gl_errors();
						}
				}
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		}
	else//draw a texture detail
		{
			glClientActiveTextureARB(GL_TEXTURE1_ARB);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2,GL_FLOAT,0,clouds_uv);
			glClientActiveTextureARB(GL_TEXTURE0_ARB);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2,GL_FLOAT,0,array_uv_main);

			if(!is_ground)
				{
					glEnableClientState(GL_NORMAL_ARRAY);
					glVertexPointer(3,GL_FLOAT,0,array_vertex);
					glNormalPointer(GL_FLOAT,0,array_normal);
	check_gl_errors();

					for(i=0;i<materials_no;i++)
						{
	check_gl_errors();
							texture_id=get_texture_id(array_order[i].texture_id);
							if(last_texture!=texture_id)
								{
									glBindTexture(GL_TEXTURE_2D, texture_id);
									last_texture=texture_id;
								}
							glDrawArrays(GL_TRIANGLES,array_order[i].start,
										 array_order[i].count);
	check_gl_errors();
						}
					glDisableClientState(GL_NORMAL_ARRAY);
				}//is ground
			else
				{
					glNormal3f(0,0,1);
					glVertexPointer(3,GL_FLOAT,0,array_vertex);
					glTexCoordPointer(2,GL_FLOAT,0,array_uv_main);
	check_gl_errors();

					for(i=0;i<materials_no;i++)
						{
	check_gl_errors();
							texture_id=get_texture_id(array_order[i].texture_id);
    						if(last_texture!=texture_id)
   						 		{
									glBindTexture(GL_TEXTURE_2D, texture_id);
									last_texture=texture_id;
								}
							// a quick check for errors
							if(array_order[i].start < 0 || array_order[i].count <= 0)
								{
									char str[256];
									sprintf(str, "Object error for %s[%d] values (%d, %d)\n",
										object_id->file_name, i,
										array_order[i].start, array_order[i].count);
									log_error(str);
								}
							glDrawArrays(GL_TRIANGLES,array_order[i].start,
										 array_order[i].count);
	check_gl_errors();
						}
				}

			glClientActiveTextureARB(GL_TEXTURE1_ARB);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glClientActiveTextureARB(GL_TEXTURE0_ARB);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
	glPopMatrix();//restore the scene
	check_gl_errors();


	if(object_id->blended)glDisable(GL_BLEND);
	if(object_id->self_lit && (night_shadows_on || dungeon))glEnable(GL_LIGHTING);
	if(is_transparent)
		{
			glDisable(GL_ALPHA_TEST);
			glEnable(GL_CULL_FACE);
		}
	check_gl_errors();

}

//Tests to see if an e3d object is already loaded. If it is, return the handle.
//If not, load it, and return the handle
e3d_object * load_e3d_cache(char * file_name)
{
	int i;
	int j;
	int file_name_lenght;
	e3d_object * e3d_id;

	file_name_lenght=strlen(file_name);

	for(i=0;i<1000;i++)
		{
			j=0;
			while(j<file_name_lenght)
				{
					if(e3d_cache[i].file_name[j]!=file_name[j])break;
					j++;
				}
			if(file_name_lenght==j)//ok, e3d already loaded
				return e3d_cache[i].e3d_id;
		}
	//e3d not found in the cache, so load it, and store it
	e3d_id=load_e3d(file_name);
	if(e3d_id==NULL)return NULL;

	//find a place to store it
	i=0;
	while(i<1000)
		{
			if(!e3d_cache[i].file_name[0])//we found a place to store it
				{
					sprintf(e3d_cache[i].file_name, "%s", file_name);
					e3d_cache[i].e3d_id=e3d_id;
					return e3d_id;
				}
			i++;
		}

	return e3d_id;
}

int add_e3d(char * file_name, float x_pos, float y_pos, float z_pos, 
			float x_rot, float y_rot, float z_rot, char self_lit, char blended,
			float r, float g, float b)
{
	//int texture_id; unused?
	int i,len,k;
	e3d_object *returned_e3d;
	object3d *our_object;
	//short sector; unused?

	our_object = calloc(1, sizeof(object3d));

	//find a free spot, in the e3d_list
	i=0;
	while(i<max_obj_3d)
		{
			if(!objects_list[i])break;
			i++;
		}

	//but first convert any '\' in '/'
	len=strlen(file_name);
	for(k=0;k<len;k++)if(file_name[k]=='\\')file_name[k]='/';

	returned_e3d=load_e3d_cache(file_name);
	if(returned_e3d==NULL)
		{
            char str[120];
            sprintf(str,"Error: Something nasty happened while trying to process: %s\n",file_name);
            log_error(str);

    		//replace it with the null object, to avoid object IDs corruption
    		returned_e3d=load_e3d_cache("./3dobjects/misc_objects/badobject.e3d");
    		if(returned_e3d==NULL)return 0;//umm, not even found the place holder, this is teh SUKC!!!
		}


	sprintf(our_object->file_name,"%s",file_name);
	our_object->x_pos=x_pos;
	our_object->y_pos=y_pos;
	our_object->z_pos=z_pos;

	our_object->x_rot=x_rot;
	our_object->y_rot=y_rot;
	our_object->z_rot=z_rot;

	our_object->r=r;
	our_object->g=g;
	our_object->b=b;

	our_object->clouds_uv=NULL;

	our_object->self_lit=self_lit;
	our_object->blended=blended;

	our_object->e3d_data=returned_e3d;

	objects_list[i]=our_object;
	return i;
}

void display_objects()
{
	int i;
	int x,y;
	x=-cx;
	y=-cy;
	check_gl_errors();
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

	check_gl_errors();
	for(i=0;i<max_obj_3d;i++)
		{
			if(objects_list[i])
				{
					int dist1;
					int dist2;
					int dist;

					dist1=x-objects_list[i]->x_pos;
					dist2=y-objects_list[i]->y_pos;
					dist=dist1*dist1+dist2*dist2;
					if(dist<=29*29)
			         	{
							float x_len;
							float y_len;
							float z_len;
							float radius;

							z_len=objects_list[i]->e3d_data->max_z-objects_list[i]->e3d_data->min_z;
							x_len=objects_list[i]->e3d_data->max_x-objects_list[i]->e3d_data->min_x;
							y_len=objects_list[i]->e3d_data->max_y-objects_list[i]->e3d_data->min_y;
							//do some checks, to see if we really have to display this object
							radius=x_len/2;
							if(radius<y_len/2)radius=y_len/2;
							if(radius<z_len)radius=z_len;
							//not in the middle of the air
							if(SphereInFrustum(objects_list[i]->x_pos,objects_list[i]->y_pos,
											   objects_list[i]->z_pos,radius))
								{
                     				draw_3d_object(objects_list[i]);
	check_gl_errors();
									anything_under_the_mouse(i,UNDER_MOUSE_3D_OBJ);
	check_gl_errors();
								}
						}
				}
		}
	check_gl_errors();
	glDisable(GL_CULL_FACE);
	glDisableClientState(GL_VERTEX_ARRAY);
	if(have_multitexture && clouds_shadows)
		{
			//disable the second texture unit
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glDisable(GL_TEXTURE_2D);
			glActiveTextureARB(GL_TEXTURE0_ARB);
		}
	check_gl_errors();

}

e3d_object * load_e3d(char *file_name)
{
	int vertex_no,faces_no,materials_no;
	int i,k,l;
	FILE *f = NULL;
	e3d_vertex *vertex_list;
	e3d_face *face_list;
	e3d_material *material_list;
	char cur_dir[200]={0};
	e3d_object *cur_object;
	//int transparency=0; unused?
	e3d_header our_header;
	char *our_header_pointer=(char *)&our_header;
	e3d_array_vertex *array_vertex;
	e3d_array_normal *array_normal;
	e3d_array_uv_main *array_uv_main;
	e3d_array_order *array_order;

	cur_object=calloc(1, sizeof(e3d_object));
	//get the current directory
	l=strlen(file_name);
	//parse the string backwards, until we find a /
	while(l>0)
		{
			if(file_name[l]=='/' || file_name[l]=='\\')break;
			l--;
		}

	i=0;
	if(l)//prevent invalid dir names
		{
			while(l>=0)
				{
					cur_dir[i]=file_name[i];
					i++;
					l--;
				}
			cur_dir[i+1]=0;
		}



	f = fopen(file_name, "rb");
	if(!f)
        {
            char str[120];
            sprintf(str,"Error: Can't open %s\n",file_name);
            log_error(str);
            return NULL;
        }

	//load and parse the header
	fread(our_header_pointer, 1, sizeof(e3d_header), f);

	faces_no=our_header.face_no;
	vertex_no=our_header.vertex_no;
	materials_no=our_header.material_no;

	//read the rest of the file (vertex,faces, materials)
	face_list=calloc(faces_no, our_header.face_size);
	fread(face_list, faces_no, our_header.face_size, f);

	vertex_list=calloc(vertex_no, our_header.vertex_size);
  	if(!vertex_list)
		{
			char str[200];
			sprintf(str,"Hmm, object name:%s seems to be corrupted. Skipping the object. Warning: This might cause further problems.",file_name);
			log_to_console(c_red2,str);
			free(cur_object);
			free(face_list);
			fclose(f);
			return NULL;
		}
	fread(vertex_list, vertex_no, our_header.vertex_size, f);

	material_list=calloc(materials_no, our_header.material_size);
	fread(material_list, materials_no, our_header.material_size, f);

	fclose (f);

	//allocate memory for our new, converted structures
	array_order=calloc(materials_no, sizeof(e3d_array_order));
	array_vertex=calloc(faces_no*3, sizeof(e3d_array_vertex));
	array_normal=calloc(faces_no*3, sizeof(e3d_array_normal));
	array_uv_main=calloc(faces_no*3, sizeof(e3d_array_uv_main));

	cur_object->min_x=our_header.min_x;
	cur_object->min_y=our_header.min_y;
	cur_object->min_z=our_header.min_z;
	cur_object->max_x=our_header.max_x;
	cur_object->max_y=our_header.max_y;
	cur_object->max_z=our_header.max_z;

	cur_object->is_transparent=our_header.is_transparent;
	cur_object->is_ground=our_header.is_ground;
	cur_object->face_no=faces_no;

	//now, load all the materials, and use the material ID (which isn't used now) to
	//temporary store the texture_ids

	for(i=0;i<materials_no;i++)
		{
			char text_file_name[200];
			int j;

			l=strlen(cur_dir);
			for(k=0;k<l;k++)text_file_name[k]=cur_dir[k];
			l=strlen(material_list[i].material_name);
			for(j=0;j<l;j++)text_file_name[k+j]=material_list[i].material_name[j];
			text_file_name[k+j]=0;

			if(cur_object->is_transparent)material_list[i].material_id=load_texture_cache(text_file_name,0);
			else material_list[i].material_id=load_texture_cache(text_file_name,255);



		}

	//assign the proper texture to each face

	for(i=0;i<faces_no;i++)
		{
			face_list[i].material=material_list[face_list[i].material].material_id;
		}

	//ok, now do the reconversion, into our vertex arrays...
	{
		int cur_index_array=0;
		int cur_mat;
		int start;
		int size;

		for(i=0;i<materials_no;i++)
			{
				size=0;
				start=-1;
				cur_mat=material_list[i].material_id;
				//some horses put two materials with the same name
				//check to see if this si the case, and if it is, skip it
				for(l=0;l<materials_no;l++)
					{
						if(material_list[l].material_id==cur_mat && i!=l)
							{
								char str[200];
								size=0;
								start=0;
								sprintf(str,"Bad object: %s . Two or more materials with the same texture name!",file_name);
								log_to_console(c_red2,str);
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

	cur_object->array_order=array_order;
	cur_object->array_vertex=array_vertex;
	cur_object->array_normal=array_normal;
	cur_object->array_uv_main=array_uv_main;
	cur_object->materials_no=materials_no;

	free(material_list);
	free(vertex_list);
	free(face_list);

	return cur_object;
}

void compute_clouds_map(object3d * object_id)
{
	float x1,y1,x,y,z,m;
	float cos_m,sin_m;
	float x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;
	int i,face_no;

	e3d_array_vertex *array_vertex;
	e3d_array_uv_detail *array_detail;

	array_vertex=object_id->e3d_data->array_vertex;
	face_no=object_id->e3d_data->face_no;
	array_detail=calloc(face_no*3,sizeof(e3d_array_uv_detail));

	x_pos=object_id->x_pos;
	y_pos=object_id->y_pos;
	z_pos=object_id->z_pos;

	x_rot=object_id->x_rot;
	y_rot=object_id->y_rot;
	z_rot=object_id->z_rot;

	m=(-z_rot)*3.1415926/180;
	cos_m=cos(m);
	sin_m=sin(m);

	for(i=0;i<face_no*3;i++)
		{
			x=array_vertex[i].x;
			y=array_vertex[i].y;
			z=array_vertex[i].z;
			x1=x*cos_m+y*sin_m;
			y1=y*cos_m-x*sin_m;
			x1=x_pos+x1;
			y1=y_pos+y1;

			array_detail[i].u=(x1+z)/texture_scale+clouds_movement_u;
			array_detail[i].v=(y1+z)/texture_scale+clouds_movement_v;
		}

	object_id->clouds_uv=array_detail;

}

void clear_clouds_cache()
{
	int i;

	last_clear_clouds=cur_time;
	for(i=0;i<max_obj_3d;i++)
		{
			if(objects_list[i])
				{
					if(objects_list[i]->clouds_uv && objects_list[i]->last_acessed_time+20000<cur_time)
						{
							free(objects_list[i]->clouds_uv);
							objects_list[i]->clouds_uv=NULL;
						}
				}
		}
}

void destroy_3d_object(int i)
{
	free(objects_list[i]);
	objects_list[i]=0;
}


