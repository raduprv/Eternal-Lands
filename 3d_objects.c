#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "global.h"

object3d *objects_list[max_obj_3d];
int highest_obj_3d= 0;

void draw_3d_object(object3d * object_id)
{
	float x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;

	e3d_array_vertex *array_vertex;
	e3d_array_normal *array_normal;
	e3d_array_uv_main *array_uv_main;
	e3d_array_uv_detail *clouds_uv;
	e3d_array_order *array_order;

	int is_transparent;
	int is_ground;

	//track the usage
	cache_use(cache_e3d, object_id->e3d_data->cache_ptr);
	if(!(SDL_GetAppState()&SDL_APPACTIVE)) return;	// not actually drawing, fake it

	// check for having to load the arrays
	if(!object_id->e3d_data->array_vertex || !object_id->e3d_data->array_normal || !object_id->e3d_data->array_uv_main || !object_id->e3d_data->array_order)
		{
			load_e3d_detail(object_id->e3d_data);
		}
	array_vertex=object_id->e3d_data->array_vertex;
	array_normal=object_id->e3d_data->array_normal;
	array_uv_main=object_id->e3d_data->array_uv_main;
	array_order=object_id->e3d_data->array_order;

	is_transparent=object_id->e3d_data->is_transparent;
	is_ground=object_id->e3d_data->is_ground;

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

	if(object_id->self_lit && (!is_day || dungeon))
		{
			glDisable(GL_LIGHTING);
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

	z_rot=object_id->z_rot;
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	x_rot=object_id->x_rot;
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	y_rot=object_id->y_rot;
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	check_gl_errors();
	if(!have_multitexture || (!clouds_shadows && !use_shadow_mapping))
		{
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glVertexPointer(3,GL_FLOAT,0,array_vertex);
			glTexCoordPointer(2,GL_FLOAT,0,array_uv_main);
			if(!is_ground)
				{
					int i;
					int materials_no;

					glEnableClientState(GL_NORMAL_ARRAY);
					glNormalPointer(GL_FLOAT,0,array_normal);
	check_gl_errors();
					materials_no=object_id->e3d_data->materials_no;
					for(i=0;i<materials_no;i++)
						{
							get_and_set_texture_id(array_order[i].texture_id);
#ifdef	DEBUG
							// a quick check for errors
							if(array_order[i].start < 0 || array_order[i].count <= 0)
								{
									char str[256];
									sprintf(str, "%s[%d] %s (%d, %d)",
										object_id->file_name, i,
										values_str,
										array_order[i].start, array_order[i].count);
									LogError(str);
								}
#endif	// DEBUG
							if(have_compiled_vertex_array)ELglLockArraysEXT(array_order[i].start, array_order[i].count);
							glDrawArrays(GL_TRIANGLES,array_order[i].start,array_order[i].count);
							if(have_compiled_vertex_array)ELglUnlockArraysEXT();
						}
					glDisableClientState(GL_NORMAL_ARRAY);
				}//is ground
			else
				{
					int i;
					int materials_no;

					glNormal3f(0,0,1);
					materials_no=object_id->e3d_data->materials_no;
					for(i=0;i<materials_no;i++)
						{
							get_and_set_texture_id(array_order[i].texture_id);
#ifdef	DEBUG
							// a quick check for errors
							if(array_order[i].start < 0 || array_order[i].count <= 0)
								{
									char str[256];
									sprintf(str, "%s[%d] %s (%d, %d)",
										object_id->file_name, i,
										values_str,
										array_order[i].start, array_order[i].count);
									LogError(str);
								}
#endif	// DEBUG
							if(have_compiled_vertex_array)ELglLockArraysEXT(array_order[i].start, array_order[i].count);
							glDrawArrays(GL_TRIANGLES,array_order[i].start,array_order[i].count);
							if(have_compiled_vertex_array)ELglUnlockArraysEXT();
						}
				}
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
	else//draw a texture detail
		{
			if(clouds_shadows)
				{
					ELglClientActiveTextureARB(detail_unit);
					glEnableClientState(GL_TEXTURE_COORD_ARRAY);
					glTexCoordPointer(2,GL_FLOAT,0,clouds_uv);
				}
			ELglClientActiveTextureARB(base_unit);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2,GL_FLOAT,0,array_uv_main);
			if(!is_ground)
				{
					int i;
					int materials_no;

					glEnableClientState(GL_NORMAL_ARRAY);
					glVertexPointer(3,GL_FLOAT,0,array_vertex);
					glNormalPointer(GL_FLOAT,0,array_normal);
	check_gl_errors();
					materials_no=object_id->e3d_data->materials_no;
					for(i=0;i<materials_no;i++)
						if(array_order[i].count>0)
							{
								get_and_set_texture_id(array_order[i].texture_id);
								if(have_compiled_vertex_array)ELglLockArraysEXT(array_order[i].start, array_order[i].count);
								glDrawArrays(GL_TRIANGLES,array_order[i].start,array_order[i].count);
								if(have_compiled_vertex_array)ELglUnlockArraysEXT();
							}
					glDisableClientState(GL_NORMAL_ARRAY);
				}//is ground
			else
				{
					int i;
					int materials_no;

					glNormal3f(0,0,1);
					glVertexPointer(3,GL_FLOAT,0,array_vertex);
					glTexCoordPointer(2,GL_FLOAT,0,array_uv_main);
	check_gl_errors();
					materials_no=object_id->e3d_data->materials_no;
					for(i=0;i<materials_no;i++)
						{
							get_and_set_texture_id(array_order[i].texture_id);
#ifdef	DEBUG
							// a quick check for errors
							if(array_order[i].start < 0 || array_order[i].count <= 0)
								{
									char str[256];
									sprintf(str, "%s: %s[%d] %s (%d, %d)",
										object_error_str,
										object_id->file_name, i,
										values_str,
										array_order[i].start, array_order[i].count);
									LogError(str);
								}
#endif	// DEBUG
							if(have_compiled_vertex_array)ELglLockArraysEXT(array_order[i].start, array_order[i].count);
							glDrawArrays(GL_TRIANGLES,array_order[i].start,array_order[i].count);
							if(have_compiled_vertex_array)ELglUnlockArraysEXT();
						}
				}
			ELglClientActiveTextureARB(detail_unit);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			ELglClientActiveTextureARB(base_unit);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
	glPopMatrix();//restore the scene
	check_gl_errors();


	if(object_id->blended)glDisable(GL_BLEND);
	if(object_id->self_lit && (!is_day || dungeon))glEnable(GL_LIGHTING);
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

	//do we have it already?
	e3d_id=cache_find_item(cache_e3d, file_name);
	if(e3d_id) return(e3d_id);
	//e3d not found in the cache, so load it, and store it
	e3d_id=load_e3d(file_name);
	if(e3d_id==NULL) return NULL;
	//and remember it
	e3d_id->cache_ptr=cache_add_item(cache_e3d, e3d_id->file_name, e3d_id, sizeof(*e3d_id));

	return e3d_id;
}

int add_e3d(char * file_name, float x_pos, float y_pos, float z_pos,
			float x_rot, float y_rot, float z_rot, char self_lit, char blended,
			float r, float g, float b)
{
	int i;
	char fname[128];
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
	clean_file_name(fname, file_name, 128);

	returned_e3d=load_e3d_cache(fname);
	if(returned_e3d==NULL)
		{
            char str[256];
            sprintf(str,nasty_error_str,fname);
            LogError(str);

    		//replace it with the null object, to avoid object IDs corruption
    		returned_e3d=load_e3d_cache("./3dobjects/misc_objects/badobject.e3d");
    		if(returned_e3d==NULL)return 0;//umm, not even found the place holder, this is teh SUKC!!!
		}

	// now, allocate the memory
	our_object = calloc(1, sizeof(object3d));

	// and fill it in
	my_strncp(our_object->file_name, fname, 80);
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
	// watch the top end
	if(i >= highest_obj_3d)
		{
			highest_obj_3d= i+1;
		}
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
			ELglActiveTextureARB(detail_unit);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, get_texture_id(ground_detail_text));
			ELglActiveTextureARB(base_unit);
			glEnable(GL_TEXTURE_2D);

		}

	check_gl_errors();
	for(i=0;i<highest_obj_3d;i++)
		{
			object3d	*object_id= objects_list[i];

			if(object_id)
				{
					int dist1, dist2;

					dist1= x-object_id->x_pos;
					dist2= y-object_id->y_pos;
					if(dist1*dist1+dist2*dist2<=29*29)
			         	{
							float x_len, y_len, z_len;
							float radius;

							z_len= object_id->e3d_data->max_z-object_id->e3d_data->min_z;
							x_len= object_id->e3d_data->max_x-object_id->e3d_data->min_x;
							y_len= object_id->e3d_data->max_y-object_id->e3d_data->min_y;
							//do some checks, to see if we really have to display this object
							radius=x_len/2;
							if(radius<y_len/2)radius=y_len/2;
							if(radius<z_len)radius=z_len;
							//not in the middle of the air
							if(SphereInFrustum(object_id->x_pos, object_id->y_pos,
											   object_id->z_pos, radius))
								{
                     				draw_3d_object(object_id);
	//check_gl_errors();
									if (read_mouse_now && mouse_in_sphere(object_id->x_pos, object_id->y_pos, object_id->z_pos, radius))
										anything_under_the_mouse(i, UNDER_MOUSE_3D_OBJ);
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
			ELglActiveTextureARB(detail_unit);
			glDisable(GL_TEXTURE_2D);
			ELglActiveTextureARB(base_unit);
		}
	check_gl_errors();
}

e3d_object * load_e3d(char *file_name)
{
	int vertex_no,faces_no,materials_no;
	FILE *f = NULL;
	e3d_object *cur_object;
	//int transparency=0; unused?
	e3d_header our_header;
	char *our_header_pointer=(char *)&our_header;

	f = fopen(file_name, "rb");
	if(!f)
        {
            char str[120];
            sprintf(str,"%s: %s: %s",reg_error_str,cant_open_file,file_name);
            LogError(str);
            return NULL;
        }

	//load and parse the header
	fread(our_header_pointer, 1, sizeof(e3d_header), f);
	fclose (f);

	faces_no=our_header.face_no;
	vertex_no=our_header.vertex_no;
	materials_no=our_header.material_no;

	// allocate the memory
	cur_object=calloc(1, sizeof(e3d_object));
	// and fill in the data
	my_strncp(cur_object->file_name, file_name, 128);
	cur_object->min_x=our_header.min_x;
	cur_object->min_y=our_header.min_y;
	cur_object->min_z=our_header.min_z;
	cur_object->max_x=our_header.max_x;
	cur_object->max_y=our_header.max_y;
	cur_object->max_z=our_header.max_z;

	cur_object->is_transparent=our_header.is_transparent;
	cur_object->is_ground=our_header.is_ground;
	cur_object->face_no=faces_no;
	cur_object->materials_no=materials_no;

	cur_object->array_order=NULL;
	cur_object->array_vertex=NULL;
	cur_object->array_normal=NULL;
	cur_object->array_uv_main=NULL;

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

	f = fopen(cur_object->file_name, "rb");
	if(!f)
        {
            char str[120];
            sprintf(str,"%s: %s: %s",reg_error_str,cant_open_file,cur_object->file_name);
            LogError(str);
            return NULL;
        }

	//load and parse the header
	fread(our_header_pointer, 1, sizeof(e3d_header), f);
	faces_no=our_header.face_no;	// or should we grab from the cur_object?
	vertex_no=our_header.vertex_no;
	materials_no=our_header.material_no;

	//read the rest of the file (vertex,faces, materials)
	face_list=calloc(faces_no, our_header.face_size);
	fread(face_list, faces_no, our_header.face_size, f);

	vertex_list=calloc(vertex_no, our_header.vertex_size);
  	if(!vertex_list)
		{
			char str[200];
			sprintf(str,"%s: %s: %s",reg_error_str,corrupted_object,cur_object->file_name);
			log_to_console(c_red2,str);
			free(face_list);
			fclose(f);
			return NULL;
		}
	fread(vertex_list, vertex_no, our_header.vertex_size, f);

	material_list=calloc(materials_no, our_header.material_size);
	fread(material_list, materials_no, our_header.material_size, f);

	fclose (f);

	//now, load all the materials, and use the material ID (which isn't used now) to
	//temporary store the texture_ids

	for(i=0;i<materials_no;i++)
		{
			char text_file_name[200];
			int j,k;

			l=strlen(cur_dir);
			for(k=0;k<l;k++)text_file_name[k]=cur_dir[k];
			l=strlen(material_list[i].material_name);
			for(j=0;j<l;j++)text_file_name[k+j]=material_list[i].material_name[j];
			text_file_name[k+j]=0;
/*
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
								sprintf(str,"%s: %s . %s",bad_object,cur_object->file_name,multiple_material_same_texture);
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

	//and memorize the stored arrays
	cur_object->array_order=array_order;
	cur_object->array_vertex=array_vertex;
	cur_object->array_normal=array_normal;
	cur_object->array_uv_main=array_uv_main;

	//release temporary memory
	free(material_list);
	free(vertex_list);
	free(face_list);

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

	m=(-z_rot)*3.1415926/180;
	cos_m=cos(m);
	sin_m=sin(m);

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

}

void clear_clouds_cache()
{
	int i;

	last_clear_clouds=cur_time;
	for(i=0;i<highest_obj_3d;i++)
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
	if(i == highest_obj_3d+1)
		{
			highest_obj_3d= i;
		}
}

Uint32 free_e3d_va(e3d_object *e3d_id)
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
	return(e3d_id->cache_ptr->size - sizeof(*e3d_id));
}

void destroy_e3d(e3d_object *e3d_id)
{
	// release the detailed data
	free_e3d_va(e3d_id);
	// and finally free the main object
	free(e3d_id);
}


