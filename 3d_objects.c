#include <stdlib.h>
#include <math.h>
#include "global.h"

void draw_3d_object(object3d * object_id)
{
	float x,y,z,u,v;
	float x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;

	int faces_no,materials_no,texture_id,a,b,c;
	int i,k;
	e3d_face *faces_list;
	e3d_vertex *vertex_list;
	e3d_material *material_list;
	char is_transparent;
	int is_ground;

	faces_no=object_id->e3d_data->face_no;
	faces_list=object_id->e3d_data->faces;
	vertex_list=object_id->e3d_data->vertexes;
	is_transparent=object_id->e3d_data->is_transparent;
	is_ground=object_id->e3d_data->is_ground;

	x_pos=object_id->x_pos;
	y_pos=object_id->y_pos;
	z_pos=object_id->z_pos;

	x_rot=object_id->x_rot;
	y_rot=object_id->y_rot;
	z_rot=object_id->z_rot;


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
	    glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
	    if(is_ground)glAlphaFunc(GL_GREATER,0.23f);
	    else glAlphaFunc(GL_GREATER,0.06f);
	    glDisable(GL_CULL_FACE);
	}


	glPushMatrix();//we don't want to affect the rest of the scene
	glTranslatef (x_pos, y_pos, z_pos);
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	if(!have_multitexture || !clouds_shadows)
		{
			glBegin(GL_TRIANGLES);
			for(i=0;i<faces_no;i++)
				{
					a=faces_list[i].a;
					b=faces_list[i].b;
					c=faces_list[i].c;

					texture_id=get_texture_id(faces_list[i].material);


    				if(last_texture!=texture_id)
   					 	{
							glEnd();
							glBindTexture(GL_TEXTURE_2D, texture_id);
							glBegin(GL_TRIANGLES);
							last_texture=texture_id;
						}

					if(!is_ground)
					{
						glNormal3f(vertex_list[a].nx,vertex_list[a].ny,vertex_list[a].nz);
						glTexCoord2f(faces_list[i].au,faces_list[i].av);
						glVertex3f(vertex_list[a].x,vertex_list[a].y,vertex_list[a].z);

						glNormal3f(vertex_list[b].nx,vertex_list[b].ny,vertex_list[b].nz);
						glTexCoord2f(faces_list[i].bu,faces_list[i].bv);
						glVertex3f(vertex_list[b].x,vertex_list[b].y,vertex_list[b].z);

						glNormal3f(vertex_list[c].nx,vertex_list[c].ny,vertex_list[c].nz);
						glTexCoord2f(faces_list[i].cu,faces_list[i].cv);
						glVertex3f(vertex_list[c].x,vertex_list[c].y,vertex_list[c].z);
					}
					else
					{
						glNormal3f(0,0,1);

						glTexCoord2f(faces_list[i].au,faces_list[i].av);
						glVertex3f(vertex_list[a].x,vertex_list[a].y,vertex_list[a].z);

						glTexCoord2f(faces_list[i].bu,faces_list[i].bv);
						glVertex3f(vertex_list[b].x,vertex_list[b].y,vertex_list[b].z);

						glTexCoord2f(faces_list[i].cu,faces_list[i].cv);
						glVertex3f(vertex_list[c].x,vertex_list[c].y,vertex_list[c].z);

					}
				}

			glEnd();
		}
	else//draw a texture detail
		{
			float m,x,y,x1,y1;

			m=(-z_rot)*3.1415926/180;
			//bind the detail texture
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,  texture_cache[ground_detail_text].texture_id);
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_2D);

			glBegin(GL_TRIANGLES);
			for(i=0;i<faces_no;i++)
				{
					a=faces_list[i].a;
					b=faces_list[i].b;
					c=faces_list[i].c;

					texture_id=get_texture_id(faces_list[i].material);


    				if(last_texture!=texture_id)
   					 	{
							glEnd();
							glBindTexture(GL_TEXTURE_2D, texture_id);
							glBegin(GL_TRIANGLES);
							last_texture=texture_id;
						}

					if(!is_ground)
					{
						x=vertex_list[a].x;
						y=vertex_list[a].y;
						x1=x*cos(m)+y*sin(m);
						y1=y*cos(m)-x*sin(m);
						x1=x_pos+x1;
						y1=y_pos+y1;

						glNormal3f(vertex_list[a].nx,vertex_list[a].ny,vertex_list[a].nz);
						glMultiTexCoord2fARB(GL_TEXTURE0_ARB,faces_list[i].au,faces_list[i].av);
						glMultiTexCoord2fARB(GL_TEXTURE1_ARB,(x1+vertex_list[a].z)/texture_scale+clouds_movement_u,
						(y1+vertex_list[a].z)/texture_scale+clouds_movement_v);
						glVertex3f(x,y,vertex_list[a].z);

						x=vertex_list[b].x;
						y=vertex_list[b].y;
						x1=x*cos(m)+y*sin(m);
						y1=y*cos(m)-x*sin(m);
						x1=x_pos+x1;
						y1=y_pos+y1;

						glNormal3f(vertex_list[b].nx,vertex_list[b].ny,vertex_list[b].nz);
						glMultiTexCoord2fARB(GL_TEXTURE0_ARB,faces_list[i].bu,faces_list[i].bv);
						glMultiTexCoord2fARB(GL_TEXTURE1_ARB,(x1+vertex_list[b].z)/texture_scale+clouds_movement_u,
						(y1+vertex_list[b].z)/texture_scale+clouds_movement_v);
						glVertex3f(x,y,vertex_list[b].z);

						x=vertex_list[c].x;
						y=vertex_list[c].y;
						x1=x*cos(m)+y*sin(m);
						y1=y*cos(m)-x*sin(m);
						x1=x_pos+x1;
						y1=y_pos+y1;

						glNormal3f(vertex_list[c].nx,vertex_list[c].ny,vertex_list[c].nz);
						glMultiTexCoord2fARB(GL_TEXTURE0_ARB,faces_list[i].cu,faces_list[i].cv);
						glMultiTexCoord2fARB(GL_TEXTURE1_ARB,(x1+vertex_list[c].z)/texture_scale+clouds_movement_u,
						(y1+vertex_list[c].z)/texture_scale+clouds_movement_v);
						glVertex3f(x,y,vertex_list[c].z);
					}
					else
					{
						glNormal3f(0,0,1);

						x=vertex_list[a].x;
						y=vertex_list[a].y;
						x1=x*cos(m)+y*sin(m);
						y1=y*cos(m)-x*sin(m);
						x1=x_pos+x1;
						y1=y_pos+y1;

						glMultiTexCoord2fARB(GL_TEXTURE0_ARB,faces_list[i].au,faces_list[i].av);
						glMultiTexCoord2fARB(GL_TEXTURE1_ARB,(x1+vertex_list[a].z)/texture_scale+clouds_movement_u,
						(y1+vertex_list[a].z)/texture_scale+clouds_movement_v);
						glVertex3f(vertex_list[a].x,vertex_list[a].y,vertex_list[a].z);

						x=vertex_list[b].x;
						y=vertex_list[b].y;
						x1=x*cos(m)+y*sin(m);
						y1=y*cos(m)-x*sin(m);
						x1=x_pos+x1;
						y1=y_pos+y1;

						glMultiTexCoord2fARB(GL_TEXTURE0_ARB,faces_list[i].bu,faces_list[i].bv);
						glMultiTexCoord2fARB(GL_TEXTURE1_ARB,(x1+vertex_list[b].z)/texture_scale+clouds_movement_u,
						(y1+vertex_list[b].z)/texture_scale+clouds_movement_v);
						glVertex3f(vertex_list[b].x,vertex_list[b].y,vertex_list[b].z);

						x=vertex_list[c].x;
						y=vertex_list[c].y;
						x1=x*cos(m)+y*sin(m);
						y1=y*cos(m)-x*sin(m);
						x1=x_pos+x1;
						y1=y_pos+y1;

						glMultiTexCoord2fARB(GL_TEXTURE0_ARB,faces_list[i].cu,faces_list[i].cv);
						glMultiTexCoord2fARB(GL_TEXTURE1_ARB,(x1+vertex_list[c].z)/texture_scale+clouds_movement_u,
						(y1+vertex_list[c].z)/texture_scale+clouds_movement_v);
						glVertex3f(vertex_list[c].x,vertex_list[c].y,vertex_list[c].z);

					}
				}
			//disable the second texture unit
			glEnd();
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glDisable(GL_TEXTURE_2D);
			glActiveTextureARB(GL_TEXTURE0_ARB);
		}

	glPopMatrix();//restore the scene


  if(object_id->blended)glDisable(GL_BLEND);
  if(object_id->self_lit && (night_shadows_on || dungeon))glEnable(GL_LIGHTING);
  if(is_transparent)
  	{
  		glDisable(GL_ALPHA_TEST);
  		glEnable(GL_CULL_FACE);
	}



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

int add_e3d(char * file_name, float x_pos, float y_pos, float z_pos, float x_rot, float y_rot, float z_rot, char self_lit, char blended, float r, float g, float b)
{
	int texture_id;
	int i,len,k;
	e3d_object *returned_e3d;
	object3d *our_object;
	short sector;

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
            return 0;
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

  our_object->self_lit=self_lit;
  our_object->blended=blended;

	our_object->e3d_data=returned_e3d;

	//get the current sector
	sector=(y_pos/sector_size_y)*(map_meters_size_x/sector_size_x)+(x_pos/sector_size_x);
	our_object->sector=sector;

	objects_list[i]=our_object;
	return i;
}

void display_objects()
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
			         int dist1;
			         int dist2;
			         int dist;

			         dist1=x-objects_list[i]->x_pos;
			         dist2=y-objects_list[i]->y_pos;
			         dist=dist1*dist1+dist2*dist2;
			         if(dist<=25*25)
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
                     		draw_3d_object(objects_list[i]);
						}
                 }
		}
	glDisable(GL_CULL_FACE);
}

e3d_object * load_e3d(char *file_name)
{
  int f_size,vertex_no,faces_no,materials_no;
  int i,k,l;
  FILE *f = NULL;
  e3d_vertex *vertex_list;
  e3d_face *face_list;
  e3d_material *material_list;
  char cur_dir[200]={0};
  e3d_object *cur_object;
  int transparency=0;
  e3d_header our_header;
  char *our_header_pointer=(char *)&our_header;

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
  fread(vertex_list, vertex_no, our_header.vertex_size, f);

  material_list=calloc(materials_no, our_header.material_size);
  fread(material_list, materials_no, our_header.material_size, f);

  fclose (f);


  cur_object->min_x=our_header.min_x;
  cur_object->min_y=our_header.min_y;
  cur_object->min_z=our_header.min_z;
  cur_object->max_x=our_header.max_x;
  cur_object->max_y=our_header.max_y;
  cur_object->max_z=our_header.max_z;

  cur_object->face_no=faces_no;
  cur_object->vertex_no=vertex_no;
  cur_object->faces=face_list;
  cur_object->vertexes=vertex_list;

  cur_object->is_transparent=our_header.is_transparent;
  cur_object->is_ground=our_header.is_ground;

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
		int test;
		test=cur_object->faces[i].material;
		cur_object->faces[i].material=material_list[cur_object->faces[i].material].material_id;
	}



  free(material_list);

return cur_object;
}
