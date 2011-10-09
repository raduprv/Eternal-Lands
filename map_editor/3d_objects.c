#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "e3d.h"
#include "../platform.h"
#include "../asc.h"
#include "global.h"

#include "../md5.h"
#include "../io/e3d_io.h"

typedef struct
{
	float x, y, z;
} float3;

static __inline__ void get_texture_object_linear_plane(float obj_z_rot, float obj_x_pos, float obj_y_pos, float* s_plane, float* t_plane)
{
	float w, cos_w, sin_w;

	w = -obj_z_rot * M_PI / 180.0f;
	cos_w = cos(w);
	sin_w = sin(w);
	
	s_plane[0] = cos_w / texture_scale;
	s_plane[1] = sin_w / texture_scale;
	s_plane[2] = 1.0f / texture_scale;
	s_plane[3] = obj_x_pos / texture_scale + clouds_movement_u;
	t_plane[0] = -sin_w / texture_scale;
	t_plane[1] = cos_w / texture_scale;
	t_plane[2] = 1.0f / texture_scale;
	t_plane[3] = obj_y_pos / texture_scale + clouds_movement_v;
}

void draw_3d_object(object3d * object_id)
{
	int i;
	float s_plane[4], t_plane[4];
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

	if (have_multitexture && clouds_shadows)
	{
		glClientActiveTextureARB(GL_TEXTURE1_ARB);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		ELglActiveTextureARB(GL_TEXTURE1_ARB);
		get_texture_object_linear_plane(object_id->z_rot, object_id->x_pos, object_id->y_pos, s_plane, t_plane);
		glTexGenfv(GL_S, GL_EYE_PLANE, s_plane);
		glTexGenfv(GL_T, GL_EYE_PLANE, t_plane);
		ELglActiveTextureARB(GL_TEXTURE0_ARB);
	
		glClientActiveTextureARB(GL_TEXTURE0_ARB);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	e3d_enable_vertex_arrays(object_id->e3d_data, 1, 1);

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
		}
		else
		{
			glDisable(GL_ALPHA_TEST);
			glEnable(GL_CULL_FACE);
		}

#ifdef	NEW_TEXTURES
		bind_texture(object_id->e3d_data->materials[i].texture);
#else	/* NEW_TEXTURES */
		get_and_set_texture_id(object_id->e3d_data->materials[i].texture);
#endif	/* NEW_TEXTURES */

		ELglDrawRangeElementsEXT(GL_TRIANGLES,
			object_id->e3d_data->materials[i].triangles_indices_min,
			object_id->e3d_data->materials[i].triangles_indices_max,
			object_id->e3d_data->materials[i].triangles_indices_count,
			object_id->e3d_data->index_type,
			object_id->e3d_data->materials[i].triangles_indices_index);
	}

	glPopMatrix();//restore the scene
	CHECK_GL_ERRORS();

	if (have_multitexture && clouds_shadows)
	{
		glClientActiveTextureARB(GL_TEXTURE1_ARB);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glClientActiveTextureARB(GL_TEXTURE0_ARB);
	}

	e3d_disable_vertex_arrays();
	glDisable(GL_COLOR_MATERIAL);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	if (object_id->blended) glDisable(GL_BLEND);
	if (object_id->self_lit && (night_shadows_on || dungeon)) glEnable(GL_LIGHTING);
	if (object_id->e3d_data->materials[object_id->e3d_data->material_no-1].options)
	{
		glDisable(GL_ALPHA_TEST);
		glEnable(GL_CULL_FACE);
	}
	CHECK_GL_ERRORS();
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

	for (i = 0; i < MAX_E3D_CACHE; i++)
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

	// allocate the memory
	e3d_id = (e3d_object*)malloc(sizeof(e3d_object));
	if (e3d_id == NULL) 
	{
		LOG_ERROR("Can't alloc data for file \"%s\"!", file_name);
		return NULL;
	}
	// and fill in the data
	memset(e3d_id, 0, sizeof(e3d_object));
	if (e3d_id == NULL) 
	{
		LOG_ERROR("Memset Error for file \"%s\"!", file_name);
		return NULL;
	}
	my_strncp(e3d_id->file_name, file_name, sizeof(e3d_id->file_name));	
	e3d_id = load_e3d_detail(e3d_id);

	if (e3d_id == NULL) 
	{
		LOG_ERROR("Can't load file \"%s\"!", file_name);
		return NULL;
	}


	//find a place to store it
	i=0;
	while (i < MAX_E3D_CACHE)
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

int add_e3d_at_id (int id, char * file_name, float x_pos, float y_pos, float z_pos, float x_rot, float y_rot, float z_rot, char self_lit, char blended, float r, float g, float b)
{
	char fname[128];
	e3d_object *returned_e3d;
	object3d *our_object;
	
	if (id < 0 || id >= MAX_OBJ_3D)
		return 0;

	//but first convert any '\' in '/'
	clean_file_name(fname, file_name, sizeof(fname));
	my_tolower(fname);

	returned_e3d=load_e3d_cache(fname);
	if(returned_e3d==NULL)
	{
		char str[120];
		sprintf (str, "Error: Something nasty happened while trying to process: %s\n", fname);
		LOG_ERROR(str);

    		//replace it with the null object, to avoid object IDs corruption
    		returned_e3d = load_e3d_cache ("./3dobjects/badobject.e3d");
    		if (returned_e3d == NULL)
			return 0; // umm, not even found the place holder, this is teh SUKC!!!
	}

	// now, allocate the memory
	our_object = calloc (1, sizeof(object3d));

	// and fill it in
	snprintf (our_object->file_name, 80, "%s", fname);
	our_object->x_pos = x_pos;
	our_object->y_pos = y_pos;
	our_object->z_pos = z_pos;

	our_object->x_rot = x_rot;
	our_object->y_rot = y_rot;
	our_object->z_rot = z_rot;

	our_object->color[0] = r;
	our_object->color[1] = g;
	our_object->color[2] = b;
	our_object->color[3] = 0.0f;


	our_object->self_lit = self_lit;
	our_object->blended = blended;

	our_object->e3d_data = returned_e3d;

	objects_list[id] = our_object;
	return id;
}

int add_e3d (char * file_name, float x_pos, float y_pos, float z_pos, float x_rot, float y_rot, float z_rot, char self_lit, char blended, float r, float g, float b)
{
	int id;

	//find a free spot, in the e3d_list
	id = 0;
	while (id < MAX_OBJ_3D)
	{
		if (objects_list[id] == NULL || objects_list[id]->blended==20) 
			break;
		id++;
	}
	
	if (id >= MAX_OBJ_3D) return 0;
	return add_e3d_at_id (id, file_name, x_pos, y_pos, z_pos, x_rot, y_rot, z_rot, self_lit, blended, r, g, b);
}

int add_e3d_keep_deleted (char * file_name, float x_pos, float y_pos, float z_pos, float x_rot, float y_rot, float z_rot, char self_lit, char blended, float r, float g, float b)
{
	int id;

	//find a free spot, in the e3d_list
	id = 0;
	while (id < MAX_OBJ_3D)
	{
		if (objects_list[id] == NULL) 
			break;
		id++;
	}
	
	if (id >= MAX_OBJ_3D) return 0;
	return add_e3d_at_id (id, file_name, x_pos, y_pos, z_pos, x_rot, y_rot, z_rot, self_lit, blended, r, g, b);
}

void display_objects()
{
	int i;
	int x,y;

	x=(int) -camera_x;
	y=(int) -camera_y;
	CHECK_GL_ERRORS();
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
			glBindTexture(GL_TEXTURE_2D,  texture_cache[ground_detail_text].texture_id);
#endif	/* NEW_TEXTURES */
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_2D);
		}

	CHECK_GL_ERRORS();
	for (i = 0; i < MAX_OBJ_3D; i++)
		{
			if(objects_list[i] && objects_list[i]->blended!=20)
				{
					int dist1;
					int dist2;

					dist1=x-(int)objects_list[i]->x_pos;
					dist2=y-(int)objects_list[i]->y_pos;
					if(dist1*dist1+dist2*dist2<=((40*40)*(zoom_level/15.75f)))
						{
	                     				draw_3d_object(objects_list[i]);
						}
				}
		}
	CHECK_GL_ERRORS();
	glDisable(GL_CULL_FACE);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	if(have_multitexture && clouds_shadows)
		{
			//disable the second texture unit
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glDisable(GL_TEXTURE_2D);
			glActiveTextureARB(GL_TEXTURE0_ARB);
		}
	CHECK_GL_ERRORS();
}


void destroy_3d_object(int i)
{
	free(objects_list[i]);
	objects_list[i]=0;
}

void destroy_e3d(e3d_object *e3d_id)
{
	if (e3d_id != NULL)
	{
		if (e3d_id->vertex_data != NULL)
		{
			free(e3d_id->vertex_data);
			e3d_id->vertex_data = NULL;
		}
		if (e3d_id->indices != NULL)
		{
			free(e3d_id->indices);
			e3d_id->indices = NULL;
		}
		if (e3d_id->materials != NULL)
		{
			free(e3d_id->materials);
			e3d_id->materials= NULL;
			e3d_id->material_no= 0;
		}
		if (e3d_id->vertex_vbo != 0) 
		{		
			ELglDeleteBuffersARB(1, &e3d_id->vertex_vbo);
			e3d_id->vertex_vbo = 0;
		}
		if (e3d_id->indices_vbo != 0) 
		{		
			ELglDeleteBuffersARB(1, &e3d_id->indices_vbo);
			e3d_id->indices_vbo = 0;
		}
		// and finally free the main object
		free(e3d_id);
	}
}

void flag_for_destruction()
{
	int i;

	for (i = 0; i < MAX_E3D_CACHE; i++)
		if (e3d_cache[i].file_name[0])
			e3d_cache[i].flag_for_destruction = 1;
}

void destroy_the_flagged()
{
	int i;
	
	for (i = 0; i < MAX_E3D_CACHE; i++)
	{
		if (e3d_cache[i].file_name[0] && e3d_cache[i].flag_for_destruction)
		{
			destroy_e3d (e3d_cache[i].e3d_id);
			e3d_cache[i].file_name[0] = 0;
			e3d_cache[i].flag_for_destruction = 0;
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


void rotatehm(float xrot, float yrot, float zrot, float3* T, int nv)
{
	float TT[4][4];
	int i;
	float t[3];
	float d[3];

	CreateIdentityMatrix(TT);
	CreateRotationMatrix(TT,xrot,yrot,zrot);

	for (i = 0; i < nv; i++)
	{
		t[0] = T[i].x;
		t[1] = T[i].y;
		t[2] = T[i].z;

		VecMulMatrix(t, TT, d);

		T[i].x = d[0];
		T[i].y = d[1];
		T[i].z = d[2];
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
	float3 *TF;
	void* T;
	int vertex_no;
	int minx, miny, maxx, maxy;							
	int i, j;
	float x_pos, y_pos;
	float min_x, min_y, max_x, max_y;

	vertex_no = objects_list[K]->e3d_data->vertex_no;

	T = objects_list[K]->e3d_data->vertex_data + objects_list[K]->e3d_data->vertex_layout->position_offset;
	TF = malloc(vertex_no * sizeof(float3));
	for (i = 0; i < vertex_no; i++)
	{
		memcpy(TF, T, sizeof(float3));
		T += objects_list[K]->e3d_data->vertex_layout->size;
	}

	x_pos = objects_list[K]->x_pos;
	y_pos = objects_list[K]->y_pos;
	min_x = 0;
	min_y = 0;
	max_x = 0;
	max_y = 0;

	// Check if we need to rotate the vertex
	if(objects_list[K]->x_rot!=0.0 || objects_list[K]->y_rot!=0.0 || objects_list[K]->z_rot!=0.0)
	{
		rotatehm(objects_list[K]->x_rot*(3.14159265/180), objects_list[K]->y_rot*(3.14159265/180), objects_list[K]->z_rot*(3.14159265/180), TF, vertex_no);
	}

	// Calculating min and max x and y values of the object
	for (i = 0; i < vertex_no; i++)
	{
		if (TF[i].x < min_x) min_x = TF[i].x;
		if (TF[i].x > max_x) max_x = TF[i].x;
		if (TF[i].y < min_y) min_y = TF[i].y;
		if (TF[i].y > max_y) max_y = TF[i].y;
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
	free(TF);
}


int ccw(float p1x, float p1y, float p2x, float p2y, float p3x, float p3y)
{
    double d = ((p1y - p2y) * (p3x - p2x)) - ((p3y - p2y) * (p1x - p2x));
	return (d > 0) ? 1 : ((d < 0) ? 2 : 3);
}


int between(float pax, float pay, float pbx, float pby, float pcx, float pcy)
{

	float p1x = pbx - pax, p1y = pby - pay, p2x = pcx - pax, p2y = pcy - pay;
	if (ccw (pax, pay, pbx, pby, pcx, pcy) != 3)
		return 0;
	return (p2x * p1x + p2y * p1y >= 0) && (p2x * p2x + p2y * p2y <= p1x * p1x + p1y * p1y);
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
 
void change_heightmap(unsigned char *hm, unsigned char h)
{
	//if(*hm==11 && h>21)return;
	if(*hm<h)*hm=h;
}

void method1(float3 *T, float x_pos, float y_pos, float z_pos, int i, int j)
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

void method2(float3 *T, float x_pos, float y_pos, float z_pos, int i, int j)
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

void method3(float3 *T, float x_pos, float y_pos, float z_pos, int i, int j)
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
	float3 *T3;
	void *T;
	void* index_pointer;
	unsigned char *u8;
	unsigned short *u16;
	unsigned int *u32;
	float3 *TF;
	int vertex_no;
	int minx, miny, maxx, maxy;							
	int i, j, k, index, size;
	int start, idx, face_no;
	float x_pos, y_pos, z_pos;
	float min_x, min_y, max_x, max_y;
	void (*method)(float3 *T, float x_pos, float y_pos, float z_pos, int i, int j);
	
	
	method = D==1 ? method1 : (D==2)?method2:method3;

	vertex_no = objects_list[K]->e3d_data->vertex_no;

	vertex_no = objects_list[K]->e3d_data->vertex_no;

	T = objects_list[K]->e3d_data->vertex_data + objects_list[K]->e3d_data->vertex_layout->position_offset;
	TF = malloc(vertex_no * sizeof(float3));
	for (i = 0; i < vertex_no; i++)
	{
		memcpy(TF, T, sizeof(float3));
		T += objects_list[K]->e3d_data->vertex_layout->size;
	}

	x_pos = objects_list[K]->x_pos;
	y_pos = objects_list[K]->y_pos;
	z_pos = objects_list[K]->z_pos;
	min_x = 0;
	min_y = 0;
	max_x = 0;
	max_y = 0;

	// Check if we need to rotate the vertex
	if(objects_list[K]->x_rot!=0.0 || objects_list[K]->y_rot!=0.0 || objects_list[K]->z_rot!=0.0)
	{
		rotatehm(objects_list[K]->x_rot*(3.14159265/180), objects_list[K]->y_rot*(3.14159265/180), objects_list[K]->z_rot*(3.14159265/180), T, vertex_no);
	}

	u8 = (unsigned char*)objects_list[K]->e3d_data->indices;
	u16 = (unsigned short*)objects_list[K]->e3d_data->indices;
	u32 = (unsigned int*)objects_list[K]->e3d_data->indices;

	// Calculating min and max x and y values of the object
	for (i = 0; i < vertex_no; i++)
	{
		if (TF[i].x < min_x) min_x = TF[i].x;
		if (TF[i].x > max_x) max_x = TF[i].x;
		if (TF[i].y < min_y) min_y = TF[i].y;
		if (TF[i].y > max_y) max_y = TF[i].y;
	}

	if (objects_list[K]->e3d_data->index_no <= 256) size = 1;
	else
	{
		if (objects_list[K]->e3d_data->index_no <= 256*256) size = 2;
		else size = 4;
	}

	face_no = 0;
	for (i = 0; i < objects_list[K]->e3d_data->material_no; i++)
	{
		face_no += objects_list[K]->e3d_data->materials[i].triangles_indices_count / 3;
	}

	T3 = (float3*)malloc(face_no * 3 * sizeof(float3));

	if (have_vertex_buffers) index_pointer = 0;
	else index_pointer = objects_list[K]->e3d_data->indices;

	index = 0;
	for (i = 0; i < objects_list[K]->e3d_data->material_no; i++)
	{
		start = objects_list[K]->e3d_data->materials[i].triangles_indices_index - index_pointer;
		start /= size;
		for (j = 0; j < objects_list[K]->e3d_data->materials[i].triangles_indices_count; j++)
		{
			if (size == 1) idx = u8[start+j];
			else
			{
				if (size == 2) idx = u16[start+j];
				else idx = u32[start+j];
			}
			T3[index].x = TF[idx].x;
			T3[index].y = TF[idx].y;
			T3[index].z = TF[idx].z;
			index++;
		}
	}
	// Calculating min and max positions on the heightmap
	minx = (x_pos + min_x) / 0.5f;
	miny = (y_pos + min_y) / 0.5f;
	maxx = (x_pos + max_x) / 0.5f + 1;
	maxy = (y_pos + max_y) / 0.5f + 1;

	for(i = minx; i < maxx; i++){
		for(j = miny; j< maxy; j++){
			for(k = 0; k < face_no * 3; k +=3)
			{
				method(&T3[k], x_pos, y_pos, z_pos, i, j);
			}
		}
	}
	free(T3);
	free(TF);
}
