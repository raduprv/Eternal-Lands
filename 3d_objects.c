#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "global.h"


void draw_3d_object(object3d * object_id)
{
	float x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;

	int materials_no,texture_id;
	int i;

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
									log_error(str);
								}
#endif	// DEBUG
						//if(have_compiled_vertex_array)glLockArraysEXT(array_order[i].start, array_order[i].count);
						glDrawArrays(GL_TRIANGLES,array_order[i].start,array_order[i].count);
						//if(have_compiled_vertex_array)glUnlockArraysEXT();
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
    						//if(last_texture!=texture_id)
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
									log_error(str);
								}
#endif	// DEBUG
						//if(have_compiled_vertex_array)glLockArraysEXT(array_order[i].start, array_order[i].count);
						glDrawArrays(GL_TRIANGLES,array_order[i].start,array_order[i].count);
						//if(have_compiled_vertex_array)glUnlockArraysEXT();
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
							//if(have_compiled_vertex_array)glLockArraysEXT(array_order[i].start, array_order[i].count);
							glDrawArrays(GL_TRIANGLES,array_order[i].start,array_order[i].count);
							//if(have_compiled_vertex_array)glUnlockArraysEXT();
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
#ifdef	DEBUG
							// a quick check for errors
							if(array_order[i].start < 0 || array_order[i].count <= 0)
								{
									char str[256];
									sprintf(str, "Object error for %s[%d] values (%d, %d)\n",
										object_id->file_name, i,
										array_order[i].start, array_order[i].count);
									log_error(str);
								}
#endif	// DEBUG
							//if(have_compiled_vertex_array)glLockArraysEXT(array_order[i].start, array_order[i].count);
							glDrawArrays(GL_TRIANGLES,array_order[i].start,array_order[i].count);
							//if(have_compiled_vertex_array)glUnlockArraysEXT();
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
	e3d_object * e3d_id;

	int i;
	int j;
	int file_name_lenght;
	file_name_lenght=strlen(file_name);

	for(i=0;i<max_e3d_cache;i++)
		{
			j=0;
			while(j<file_name_lenght)
				{
					if(e3d_cache[i].file_name[j]!=file_name[j])break;
					j++;
				}
			if(file_name_lenght==j)//ok, e3d already loaded
				{
					e3d_cache[i].flag_for_destruction=0;
					return e3d_cache[i].e3d_id;
				}
		}
	//e3d not found in the cache, so load it, and store it
	e3d_id=load_e3d(file_name);
	if(e3d_id==NULL)return NULL;

	//find a place to store it
	i=0;
	while(i<max_e3d_cache)
		{
			if(!e3d_cache[i].file_name[0])//we found a place to store it
				{
					sprintf(e3d_cache[i].file_name, "%s", file_name);
					e3d_cache[i].e3d_id=e3d_id;
					e3d_cache[i].flag_for_destruction=0;
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
	int i,len,k;
	e3d_object *returned_e3d;
	object3d *our_object;

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

	// now, allocate the memory
	our_object=(object3d*) calloc(1, sizeof(object3d));

	// and fill it in
	snprintf(our_object->file_name,80,"%s",file_name);
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

	x=(int) -cx;
	y=(int) -cy;
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

					dist1=x-(int)objects_list[i]->x_pos;
					dist2=y-(int)objects_list[i]->y_pos;
					if(dist1*dist1+dist2*dist2<=((40*40)*(zoom_level/15.75f)))
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
							//if(SphereInFrustum(objects_list[i]->x_pos,objects_list[i]->y_pos,
							//				   objects_list[i]->z_pos,radius))
								{
                     				draw_3d_object(objects_list[i]);
	check_gl_errors();
									//anything_under_the_mouse(i,UNDER_MOUSE_3D_OBJ);
	//check_gl_errors();
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
	face_list=(e3d_face*) calloc(faces_no, our_header.face_size);
	fread(face_list, faces_no, our_header.face_size, f);

	vertex_list=(e3d_vertex*) calloc(vertex_no, our_header.vertex_size);
  	if(!vertex_list)
		{
			char str[200];
			sprintf(str,"Hmm, object name:%s seems to be corrupted. Skipping the object. Warning: This might cause further problems.",file_name);
			//log_to_console(c_red2,str);
			free(face_list);
			fclose(f);
			return NULL;
		}
	fread(vertex_list, vertex_no, our_header.vertex_size, f);

	material_list=(e3d_material*) calloc(materials_no, our_header.material_size);
	fread(material_list, materials_no, our_header.material_size, f);

	fclose (f);

	//allocate memory for our new, converted structures
	array_order=(e3d_array_order*) calloc(materials_no, sizeof(e3d_array_order));
	array_vertex=(e3d_array_vertex*) calloc(faces_no*3, sizeof(e3d_array_vertex));
	array_normal=(e3d_array_normal*) calloc(faces_no*3, sizeof(e3d_array_normal));
	array_uv_main=(e3d_array_uv_main*) calloc(faces_no*3, sizeof(e3d_array_uv_main));

	// allocate the memory
	cur_object=(e3d_object*) calloc(1, sizeof(e3d_object));
	// and fill in the data
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
/*
			if(cur_object->is_transparent)material_list[i].material_id=load_texture_cache(text_file_name,0);
			else material_list[i].material_id=load_texture_cache(text_file_name,0);
*/
			material_list[i].material_id=load_texture_cache(text_file_name,0);
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
				//check to see if this is the case, and if it is, skip it

				for(l=0;l<materials_no;l++)
					{
						if(material_list[l].material_id==cur_mat && i!=l)
							{
								char str[200];
								size=0;
								start=0;
								sprintf(str,"Bad object: %s . Two or more materials with the same texture name!\n",file_name);
								//log_to_console(c_red2,str);
								log_error(str);
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
	array_detail=(e3d_array_uv_detail*) calloc(face_no*3,sizeof(e3d_array_uv_detail));

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

void destroy_e3d(e3d_object *e3d_id)
{
	if(e3d_id->array_vertex)
		{
			free(e3d_id->array_vertex);
			e3d_id->array_vertex=NULL;
		}

	if(e3d_id->array_normal)
		{
			free(e3d_id->array_normal);
			e3d_id->array_normal=NULL;
		}

	if(e3d_id->array_uv_main)
		{
			free(e3d_id->array_uv_main);
			e3d_id->array_uv_main=NULL;
		}

	if(e3d_id->array_order)
		{
			free(e3d_id->array_order);
			e3d_id->array_order=NULL;
		}
	// and finally free the main object
	free(e3d_id);
}

void flag_for_destruction()
{
	int i;

	for(i=0;i<max_e3d_cache;i++)
	if(e3d_cache[i].file_name[0])
	e3d_cache[i].flag_for_destruction=1;
}

void destroy_the_flagged()
{
	int i;
	
	for(i=0;i<max_e3d_cache;i++)
		{
			if(e3d_cache[i].file_name[0])
			if(e3d_cache[i].flag_for_destruction)
				{
					destroy_e3d(e3d_cache[i].e3d_id);
					e3d_cache[i].file_name[0]=0;
					e3d_cache[i].flag_for_destruction=0;
				}
		}
}


void MulMatrix(float mat1[4][4], float mat2[4][4], float dest[4][4])
{
   int i,j;
   for(i=0; i<4; i++)
      for(j=0; j<4; j++)
         dest[i][j]=mat1[i][0]*mat2[0][j]+mat1[i][1]*mat2[1][j]+mat1[i][2]*mat2[2][j]+mat1[i][3]*mat2[3][j];
}

void VecMulMatrix(float s[3],float mat[4][4],float d[3])
{
    d[0]=s[0]*mat[0][0]+s[1]*mat[1][0]+s[2]*mat[2][0]+mat[3][0];
    d[1]=s[0]*mat[0][1]+s[1]*mat[1][1]+s[2]*mat[2][1]+mat[3][1];
    d[2]=s[0]*mat[0][2]+s[1]*mat[1][2]+s[2]*mat[2][2]+mat[3][2];
}

void CreateIdentityMatrix(float mat[4][4])
{
    mat[0][0]=1; mat[0][1]=0; mat[0][2]=0; mat[0][3]=0;
    mat[1][0]=0; mat[1][1]=1; mat[1][2]=0; mat[1][3]=0;
    mat[2][0]=0; mat[2][1]=0; mat[2][2]=1; mat[2][3]=0;
    mat[3][0]=0; mat[3][1]=0; mat[3][2]=0; mat[3][3]=1;
}

void CreateRotationMatrix(float matrix[4][4],float ax,float ay,float az)
{
   float xmat[4][4], ymat[4][4], zmat[4][4];
   float mat1[4][4], mat2[4][4];
   xmat[0][0]=1;	xmat[0][1]=0;		xmat[0][2]=0;		xmat[0][3]=0;
   xmat[1][0]=0;	xmat[1][1]=cos(ax); xmat[1][2]=sin(ax); xmat[1][3]=0;
   xmat[2][0]=0;	xmat[2][1]=-sin(ax);xmat[2][2]=cos(ax); xmat[2][3]=0;
   xmat[3][0]=0;    xmat[3][1]=0;		xmat[3][2]=0;		xmat[3][3]=1;

   ymat[0][0]=cos(ay);	ymat[0][1]=0;	ymat[0][2]=-sin(ay);ymat[0][3]=0;
   ymat[1][0]=0;		ymat[1][1]=1;	ymat[1][2]=0;		ymat[1][3]=0;
   ymat[2][0]=sin(ay);  ymat[2][1]=0;	ymat[2][2]=cos(ay); ymat[2][3]=0;
   ymat[3][0]=0;        ymat[3][1]=0;	ymat[3][2]=0;	ymat[3][3]=1;

   zmat[0][0]=cos(az);	zmat[0][1]=sin(az);	zmat[0][2]=0;	zmat[0][3]=0;
   zmat[1][0]=-sin(az); zmat[1][1]=cos(az); zmat[1][2]=0;	zmat[1][3]=0;
   zmat[2][0]=0;        zmat[2][1]=0;       zmat[2][2]=1;	zmat[2][3]=0;
   zmat[3][0]=0;        zmat[3][1]=0;       zmat[3][2]=0;	zmat[3][3]=1;

   MulMatrix(matrix,ymat,mat1);
   MulMatrix(mat1,xmat,mat2);
   MulMatrix(mat2,zmat,matrix);
}


void rotatehm(float xrot, float yrot, float zrot, e3d_array_vertex *T, int nv)
{
	float TT[4][4];
	int i;
	CreateIdentityMatrix(TT);
	CreateRotationMatrix(TT,xrot,yrot,zrot);

	for(i=0;i<nv;i++){
		float t[3] = {T[i].x,T[i].y,T[i].z};
		float d[3];
		VecMulMatrix(t,TT,d);
		T[i].x=d[0]; T[i].y=d[1]; T[i].z=d[2];
	}

}

int TriangleTest(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3)
{
	float b0,b1,b2,b3;
	b0 =  (x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1);
	b1 = ((x2 - x0) * (y3 - y0) - (x3 - x0) * (y2 - y0)) / b0;
	b2 = ((x3 - x0) * (y1 - y0) - (x1 - x0) * (y3 - y0)) / b0;
	b3 = ((x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0)) / b0;
	return (b1>0 && b2>0 && b3>0) ? 1 : 0;
}

void clear_e3d_heightmap(int K)
{
	int face_no = objects_list[K]->e3d_data->face_no;
	float x_pos = objects_list[K]->x_pos;
	float y_pos = objects_list[K]->y_pos;
	float min_x = 0, min_y = 0, max_x = 0, max_y = 0;
	e3d_array_vertex *T = objects_list[K]->e3d_data->array_vertex;
	int minx, miny, maxx, maxy;							
	int i, j, h;
	
	e3d_array_vertex *TT = NULL;
	
	// Check if we need to rotate the vertex
	if(objects_list[K]->x_rot!=0.0 || objects_list[K]->y_rot!=0.0 || objects_list[K]->z_rot!=0.0)
	{
		TT = (e3d_array_vertex *)malloc(face_no*3*sizeof(e3d_array_vertex));
		memcpy(TT,T,face_no*3*sizeof(e3d_array_vertex));
		T = TT;
		rotatehm(objects_list[K]->x_rot*(3.14159265/180),objects_list[K]->y_rot*(3.14159265/180),objects_list[K]->z_rot*(3.14159265/180),T,face_no*3);
	}

	// Calculating min and max x and y values of the object
	for(i = 0; i < face_no*3; i++){
		if(T[i].x < min_x) min_x = T[i].x;
		if(T[i].x > max_x) max_x = T[i].x;
		if(T[i].y < min_y) min_y = T[i].y;
		if(T[i].y > max_y) max_y = T[i].y;
	}

	// Calculating min and max positions on the heightmap
	minx = (x_pos + min_x) / 0.5f;
	miny = (y_pos + min_y) / 0.5f;
	maxx = (x_pos + max_x) / 0.5f + 1;
	maxy = (y_pos + max_y) / 0.5f + 1;

	for(i = minx; i < maxx; i++){
		for(j = miny; j< maxy; j++){
			height_map[(j*tile_map_size_x*6)+i]=11;
		}
	}
	// Freeing if there is rotation
	if(TT!=NULL)
		free(TT);
}


int ccw(float p1x, float p1y, float p2x, float p2y, float p3x, float p3y)
{
    double d = ((p1y - p2y) * (p3x - p2x)) - ((p3y - p2y) * (p1x - p2x));
	return (d > 0) ? 1 : ((d < 0) ? 2 : 3);
}
 

int TestLines(float pax, float pay, float pbx, float pby, float pcx, float pcy, float pdx, float pdy)
{
	int ccw1, ccw2, ccw3, ccw4;
	if(pcx == pdx && pcy == pdy) return 0;
	if (ccw (pax, pay, pbx, pby, pcx, pcy) == 3 || ccw (pax, pay, pbx, pby, pdx, pdy) == 3 || ccw (pcx, pcy, pdx, pdy, pax, pay) == 3 || ccw (pcx, pcy, pdx, pdy, pbx, pby) == 3){
		if (between (pax, pay, pbx, pby, pcx, pcy) || between (pax, pay, pbx, pby, pdx, pdy) || between (pcx, pcy, pdx, pdy, pax, pay) || between (pcx, pcy, pdx, pdy, pbx, pby))
			return 1;
	}else{
		ccw1 = (ccw (pax, pay, pbx, pby, pcx, pcy) == 1) ? 1 : 0;
		ccw2 = (ccw (pax, pay, pbx, pby, pdx, pdy) == 1) ? 1 : 0;
		ccw3 = (ccw (pcx, pcy, pdx, pdy, pax, pay) == 1) ? 1 : 0;
		ccw4 = (ccw (pcx, pcy, pdx, pdy, pbx, pby) == 1) ? 1 : 0;
		return (ccw1 ^ ccw2) && (ccw3 ^ ccw4);
	}
	return 0;
}
 
int between(float pax, float pay, float pbx, float pby, float pcx, float pcy)
{

	float p1x = pbx - pax, p1y = pby - pay, p2x = pcx - pax, p2y = pcy - pay;
	if (ccw (pax, pay, pbx, pby, pcx, pcy) != 3)
		return 0;
	return (p2x * p1x + p2y * p1y >= 0) && (p2x * p2x + p2y * p2y <= p1x * p1x + p1y * p1y);
}

void change_heightmap(unsigned char *hm, unsigned char h)
{
	//if(*hm==11 && h>21)return;
	if(*hm<h)*hm=h;
}

void method1(e3d_array_vertex *T, float x_pos, float y_pos, float z_pos, int i, int j)
{
	float x1 = T[0].x + x_pos, x2 = T[1].x + x_pos, x3 = T[2].x + x_pos;
	float y1 = T[0].y + y_pos, y2 = T[1].y + y_pos, y3 = T[2].y + y_pos;
	int h =(((T[0].z + T[1].z + T[2].z)/3+z_pos)+2.2f)/0.2f; // Average height of triangle
	if(h>31)h=31;

	// We determine if the point is in the triangle
	if(TriangleTest(i*0.5f+0.25f, j*0.5f+0.25f, x1, y1, x2, y2, x3, y3)){
		height_map[(j*tile_map_size_x*6)+i]=h;	
		return;
	}
}

void method2(e3d_array_vertex *T, float x_pos, float y_pos, float z_pos, int i, int j)
{
	float x1 = T[0].x + x_pos, x2 = T[1].x + x_pos, x3 = T[2].x + x_pos;
	float y1 = T[0].y + y_pos, y2 = T[1].y + y_pos, y3 = T[2].y + y_pos;
	int h =(((T[0].z + T[1].z + T[2].z)/3+z_pos)+2.2f)/0.2f; // Average height of triangle
	if(h>31)h=31;

	// We determine if the point is in the triangle
	if(TriangleTest(i*0.5f+0.25f, j*0.5f+0.25f, x1, y1, x2, y2, x3, y3)){
		height_map[(j*tile_map_size_x*6)+i]=h;	
		return;
	}
	if(TriangleTest(i*0.5f, j*0.5f, x1, y1, x2, y2, x3, y3)){
		height_map[(j*tile_map_size_x*6)+i]=h;	
		return;
	}
	if(TriangleTest(i*0.5f+0.25f, j*0.5f, x1, y1, x2, y2, x3, y3)){
		height_map[(j*tile_map_size_x*6)+i]=h;	
		return;
	}
	if(TriangleTest(i*0.5f+0.25f, j*0.5f, x1, y1, x2, y2, x3, y3)){
		height_map[(j*tile_map_size_x*6)+i]=h;	
		return;
	}
}

void method3(e3d_array_vertex *T, float x_pos, float y_pos, float z_pos, int i, int j)
{
	float x1 = T[0].x + x_pos, x2 = T[1].x + x_pos, x3 = T[2].x + x_pos;
	float y1 = T[0].y + y_pos, y2 = T[1].y + y_pos, y3 = T[2].y + y_pos;
	unsigned char h =(((T[0].z + T[1].z + T[2].z)/3+z_pos)+2.2f)/0.2f; // Average height of triangle
	if(h>31)h=31;

	// Special case 1: triangle inside rect
	if( (x1 > i*0.5 && x1 < (i+1)*0.5 && y1 > j*0.5 && y1 < (j+1)*0.5)
	||  (x2 > i*0.5 && x2 < (i+1)*0.5 && y2 > j*0.5 && y2 < (j+1)*0.5)
	||  (x3 > i*0.5 && x3 < (i+1)*0.5 && y3 > j*0.5 && y3 < (j+1)*0.5)){
		change_heightmap(&height_map[(j*tile_map_size_x*6)+i],h);
		//height_map[(j*tile_map_size_x*6)+i]=h;	
		//return;
	}
	
	// Special case 2: rectangle inside triangle
	if(TriangleTest(i*0.5f+0.25f, j*0.5f+0.25f, x1, y1, x2, y2, x3, y3)){
		change_heightmap(&height_map[(j*tile_map_size_x*6)+i],h);
		//height_map[(j*tile_map_size_x*6)+i]=h;	
		//return;
	}

	// Checking triangle intersections
	if(TestLines(i*0.5, j*0.5, (i+1)*0.5, j*0.5, x1, y1, x2, y2)
	|| TestLines(i*0.5, j*0.5, (i+1)*0.5, j*0.5, x2, y2, x3, y3)
	|| TestLines(i*0.5, j*0.5, (i+1)*0.5, j*0.5, x3, y3, x1, y1)
	|| TestLines(i*0.5, j*0.5, i*0.5, (j+1)*0.5, x1, y1, x2, y2)
	|| TestLines(i*0.5, j*0.5, i*0.5, (j+1)*0.5, x2, y2, x3, y3)
	|| TestLines(i*0.5, j*0.5, i*0.5, (j+1)*0.5, x3, y3, x1, y1)
	|| TestLines((i+1)*0.5, (j+1)*0.5, (i+1)*0.5, j*0.5, x1, y1, x2, y2)
	|| TestLines((i+1)*0.5, (j+1)*0.5, (i+1)*0.5, j*0.5, x2, y2, x3, y3)
	|| TestLines((i+1)*0.5, (j+1)*0.5, (i+1)*0.5, j*0.5, x3, y3, x1, y1)
	|| TestLines((i+1)*0.5, (j+1)*0.5, i*0.5, (j+1)*0.5, x1, y1, x2, y2)
	|| TestLines((i+1)*0.5, (j+1)*0.5, i*0.5, (j+1)*0.5, x2, y2, x3, y3)
	|| TestLines((i+1)*0.5, (j+1)*0.5, i*0.5, (j+1)*0.5, x3, y3, x1, y1)){
		change_heightmap(&height_map[(j*tile_map_size_x*6)+i],h);
		//height_map[(j*tile_map_size_x*6)+i]=h;	
		//return;
	}


}

void add_e3d_heightmap(int K, int D)
{
	int face_no = objects_list[K]->e3d_data->face_no;
	float x_pos = objects_list[K]->x_pos, y_pos = objects_list[K]->y_pos, z_pos = objects_list[K]->z_pos;
	float min_x = 0, min_y = 0, max_x = 0, max_y = 0;
	e3d_array_vertex *T = objects_list[K]->e3d_data->array_vertex, *TT = NULL;
	int minx, miny, maxx, maxy;							
	int i, j, k, h;
	float b3, x0, x1, x2, x3, y0, y1, y2, y3;
	void (*method)(e3d_array_vertex *T, float x_pos, float y_pos, float z_pos, int i, int j);
	
	
	method = D==1 ? method1 : (D==2)?method2:method3;

	// Check if we need to rotate the vertex
	if(objects_list[K]->x_rot!=0.0 || objects_list[K]->y_rot!=0.0 || objects_list[K]->z_rot!=0.0)
	{
		TT = (e3d_array_vertex *)malloc(face_no*3*sizeof(e3d_array_vertex));
		memcpy(TT,T,face_no*3*sizeof(e3d_array_vertex));
		T = TT;
		rotatehm(objects_list[K]->x_rot*(3.14159265/180),objects_list[K]->y_rot*(3.14159265/180),objects_list[K]->z_rot*(3.14159265/180),T,face_no*3);
	}

	// Calculating min and max x and y values of the object
	for(k = 0; k < face_no*3; k++){
		if(T[k].x < min_x) min_x = T[k].x;
		if(T[k].x > max_x) max_x = T[k].x;
		if(T[k].y < min_y) min_y = T[k].y;
		if(T[k].y > max_y) max_y = T[k].y;
	}
		
	// Calculating min and max positions on the heightmap
	minx = (x_pos + min_x) / 0.5f;
	miny = (y_pos + min_y) / 0.5f;
	maxx = (x_pos + max_x) / 0.5f + 1;
	maxy = (y_pos + max_y) / 0.5f + 1;

	for(i = minx; i < maxx; i++){
		for(j = miny; j< maxy; j++){
			for(k = 0; k < face_no*3; k +=3){
				method(&T[k], x_pos, y_pos, z_pos, i, j);
			}
		}
	}
	// Freeing if there is rotation
	if(TT!=NULL)
		free(TT);
}