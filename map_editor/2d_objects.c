#include "2d_objects.h"
#include "draw_scene.h"
#include "global.h"
#include "../asc.h"
#include "../platform.h"
#include "../textures.h"
#include "../io/elfilewrapper.h"

#define INVALID -1
#define GROUND 0
#define PLANT 1
#define FENCE 2

void draw_2d_object(obj_2d * object_id)
{
	float render_x_start,render_y_start,u_start,v_start,u_end,v_end;
	float x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;
	float x_size,y_size;
	float alpha_test;
	int object_type;
	obj_2d_def *obj_def_pointer;

	x_pos=object_id->x_pos;
	y_pos=object_id->y_pos;
	z_pos=object_id->z_pos;

	x_rot=object_id->x_rot;
	y_rot=object_id->y_rot;
	z_rot=object_id->z_rot;

	obj_def_pointer=object_id->obj_pointer;

	u_start=obj_def_pointer->u_start;
	u_end=obj_def_pointer->u_end;
 	v_start=obj_def_pointer->v_start;
	v_end=obj_def_pointer->v_end;
	x_size=obj_def_pointer->x_size;
	y_size=obj_def_pointer->y_size;
	alpha_test=obj_def_pointer->alpha_test;
	render_x_start=-x_size/2.0f;
	object_type=obj_def_pointer->object_type;
	if (object_type == GROUND)
		render_y_start = -y_size/2;
	else
		render_y_start = 0;

	//find out what kind of object we have
	if(object_type == FENCE)
	{
		x_rot += 90;
	}
	else if(object_type == PLANT)
	{
		x_rot+=90;
		z_rot=-rz;
	}

	glPushMatrix();//we don't want to affect the rest of the scene
	glTranslatef (x_pos, y_pos, 0);
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

    glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
    if(alpha_test!=0)
    glAlphaFunc(GL_GREATER,alpha_test);
    else
    glAlphaFunc(GL_GREATER,0.18f);

	bind_texture(obj_def_pointer->texture_id);

	if(!have_multitexture || !clouds_shadows)
		{
			glBegin(GL_QUADS);

			glTexCoord2f(u_start,v_start);
			glVertex3f(render_x_start,render_y_start,z_pos);

			glTexCoord2f(u_start,v_end);
			glVertex3f(render_x_start,render_y_start+y_size,z_pos);

			glTexCoord2f(u_end,v_end);
			glVertex3f(render_x_start+x_size,render_y_start+y_size,z_pos);

			glTexCoord2f(u_end,v_start);
			glVertex3f(render_x_start+x_size,render_y_start,z_pos);
    		glEnd();
		}
	else
		{
			float m,x,y,x1,y1;

			//bind the detail texture
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glEnable(GL_TEXTURE_2D);
			bind_texture_unbuffered(ground_detail_text);
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_2D);

			glBegin(GL_QUADS);

			m=(-z_rot)*3.1415926/180;

			x=render_x_start;
			y=render_y_start;
			x1=x*cos(m)+y*sin(m);
			y1=y*cos(m)-x*sin(m);
			x1=x_pos+x1;
			y1=y_pos+y1;

			glMultiTexCoord2fARB(GL_TEXTURE0_ARB,u_start,v_start);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB,x1/texture_scale+clouds_movement_u,y1/texture_scale+clouds_movement_v);
			glVertex3f(render_x_start,render_y_start,z_pos);

			x=render_x_start;
			y=render_y_start+y_size;
			x1=x*cos(m)+y*sin(m);
			y1=y*cos(m)-x*sin(m);
			x1=x_pos+x1;
			y1=y_pos+y1;

			glMultiTexCoord2fARB(GL_TEXTURE0_ARB,u_start,v_end);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB,x1/texture_scale+clouds_movement_u,y1/texture_scale+clouds_movement_v);
			glVertex3f(render_x_start,render_y_start+y_size,z_pos);

			x=render_x_start+x_size;
			y=render_y_start+y_size;
			x1=x*cos(m)+y*sin(m);
			y1=y*cos(m)-x*sin(m);
			x1=x_pos+x1;
			y1=y_pos+y1;

			glMultiTexCoord2fARB(GL_TEXTURE0_ARB,u_end,v_end);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB,x1/texture_scale+clouds_movement_u,y1/texture_scale+clouds_movement_v);
			glVertex3f(render_x_start+x_size,render_y_start+y_size,z_pos);

			x=render_x_start+x_size;
			y=render_y_start;
			x1=x*cos(m)+y*sin(m);
			y1=y*cos(m)-x*sin(m);
			x1=x_pos+x1;
			y1=y_pos+y1;


			glMultiTexCoord2fARB(GL_TEXTURE0_ARB,u_end,v_start);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB,x1/texture_scale+clouds_movement_u,y1/texture_scale+clouds_movement_v);
			glVertex3f(render_x_start+x_size,render_y_start,z_pos);
    		glEnd();
    		//disable the multitexturing
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glDisable(GL_TEXTURE_2D);
			glActiveTextureARB(GL_TEXTURE0_ARB);
		}

    glDisable(GL_ALPHA_TEST);
	glPopMatrix();//restore the scene
}

static void parse_2d0(const char* desc, Uint32 len, const char* cur_dir, obj_2d_def *def)
{
	char name[256], value[256];
	const char *cp, *cp_end;
	Uint32 i;

	int file_x_len = -1, file_y_len = -1;
	int u_start = -1, u_end = -1, v_start = -1, v_end = -1;

	def->x_size = def->y_size = def->alpha_test = -1;

	cp = desc;
	cp_end = cp + len;
	while (1)
	{
		// skip whitespace
		while (cp < cp_end && isspace(*cp))
			cp++;
		if (cp >= cp_end) break;

		// copy the key
		i = 0;
		while (cp < cp_end && i < sizeof(name)-1 && !isspace(*cp) && *cp != ':' && *cp != '=')
			name[i++] = *cp++;
		name[i] = '\0';
		if (cp >= cp_end) break;

		// skip separators
		while (cp < cp_end && (isspace(*cp) || *cp == ':' || *cp == '='))
		{
			if (*cp == '\n')
				break;
			cp++;
		}
		if (cp >= cp_end) break;
		if (*cp == '\n') continue; // no value

		// copy value
		i = 0;
		while (cp < cp_end && i < sizeof(value)-1 && !isspace(*cp))
			value[i++] = *cp++;
		value[i] = '\0';

		if (!strcasecmp(name, "file_x_len"))
			file_x_len = atoi(value);
		else if (!strcasecmp(name, "file_y_len"))
			file_y_len = atoi(value);
		else if (!strcasecmp(name, "u_start"))
			u_start = atoi(value);
		else if (!strcasecmp(name, "u_end"))
			u_end = atoi(value);
		else if (!strcasecmp(name, "v_start"))
			v_start = atoi(value);
		else if (!strcasecmp(name, "v_end"))
			v_end = atoi(value);
		else if (!strcasecmp(name, "x_size"))
			def->x_size = atof(value);
		else if (!strcasecmp(name, "y_size"))
			def->y_size = atof(value);
		else if (!strcasecmp(name, "alpha_test"))
			def->alpha_test = atof(value);
		else if (!strcasecmp(name, "texture"))
		{
			char texture_file_name[256];
			safe_snprintf(texture_file_name, sizeof(texture_file_name),
				"%s/%s", cur_dir, value);
			def->texture_id = load_texture_cached(texture_file_name, tt_mesh);
		}
		else if (!strcmp(name, "type"))
		{
			switch (*value)
			{
				case 'g':
				case 'G': def->object_type = GROUND; break;
				case 'p':
				case 'P': def->object_type = PLANT; break;
				case 'f':
				case 'F': def->object_type = FENCE; break;
				default:  def->object_type = INVALID;
			}
		}
	}

	def->u_start = (float)u_start/file_x_len;
	def->u_end = (float)u_end/file_x_len;
	def->v_start = (float)v_start/file_y_len;
	def->v_end = (float)v_end/file_y_len;
	if (def->alpha_test < 0)
		def->alpha_test = 0;
}

obj_2d_def * load_obj_2d_def(char *file_name)
{
  int f_size;
  int i,l;
  el_file_ptr file = NULL;
  char cur_dir[200]={0};
  obj_2d_def *cur_object;
  char *obj_file_mem;

  cur_object=(obj_2d_def*) calloc(1, sizeof(obj_2d_def));
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


  file = el_open(file_name);
  if(file == NULL)
  {
    log_error(__FILE__, __LINE__, "Error: Can't open file: %s\n", file_name);
    free(cur_object);
    return NULL;
  }
  
  obj_file_mem = el_get_pointer(file);
  f_size = el_get_size(file);
	parse_2d0(obj_file_mem, f_size, cur_dir, cur_object);

	el_close(file);

	return cur_object;
}

//Tests to see if an obj_2d object is already loaded. If it is, return the handle.
//If not, load it, and return the handle
obj_2d_def * load_obj_2d_def_cache(char * file_name)
{
	int i;
	int j;
	int file_name_lenght;
	obj_2d_def * obj_2d_def_id;

	file_name_lenght=strlen(file_name);

	for(i=0;i<MAX_OBJ_2D_DEF;i++)
		{
			j=0;
			while(j<file_name_lenght)
				{
					if(obj_2d_def_cache[i].file_name[j]!=file_name[j])break;
					j++;
				}
			if(file_name_lenght==j)//ok, obj_2d_def already loaded
			return obj_2d_def_cache[i].obj_2d_def_id;
		}
	//asc not found in the cache, so load it, and store it
	obj_2d_def_id=load_obj_2d_def(file_name);

	//find a place to store it
	i=0;
	while(i<MAX_OBJ_2D_DEF)
		{
			if(!obj_2d_def_cache[i].file_name[0])//we found a place to store it
				{
					sprintf(obj_2d_def_cache[i].file_name, "%s", file_name);
					obj_2d_def_cache[i].obj_2d_def_id=obj_2d_def_id;
					return obj_2d_def_id;
				}
			i++;
		}

	return obj_2d_def_id;
}

#ifdef CLUSTER_INSIDES
int get_2d_bbox (int id, AABBOX* box)
{
	const obj_2d* obj;
	const obj_2d_def* def;
	float len_x;
	float len_y;
	float x_rot, z_rot;
	MATRIX4x4 matrix;

	if (id < 0 || id >= MAX_OBJ_2D || obj_2d_list[id] == NULL)
		return 0;

	obj = obj_2d_list[id];
	def = obj->obj_pointer;
	if (def == NULL)
		return 0;

	len_x = def->x_size;
	len_y = def->y_size;

	box->bbmin[X] = -len_x*0.5f;
	box->bbmax[X] =  len_x*0.5f;
	if (def->object_type == GROUND)
	{
		box->bbmin[Y] = -len_y*0.5f;
		box->bbmax[Y] =  len_y*0.5f;
	}
	else
	{
		box->bbmin[Y] = 0.0f;
		box->bbmax[Y] = len_y;
		if (def->object_type == PLANT)
		{
#ifdef  M_SQRT2
			box->bbmin[X] *= M_SQRT2;
			box->bbmax[X] *= M_SQRT2;
			box->bbmin[Y] *= M_SQRT2;
			box->bbmax[Y] *= M_SQRT2;
#else   //M_SQRT2
			box->bbmin[X] *= sqrt(2);
			box->bbmax[X] *= sqrt(2);
			box->bbmin[Y] *= sqrt(2);
			box->bbmax[Y] *= sqrt(2);
#endif  //M_SQRT2
		}
	}
	box->bbmin[Z] = obj->z_pos;
	box->bbmax[Z] = obj->z_pos;

	x_rot = obj->x_rot;
	z_rot = obj->z_rot;
	if (def->object_type == PLANT)
	{
		x_rot += 90.0f;
		z_rot = 0.0f;
	}
	else if (def->object_type == FENCE)
	{
		x_rot += 90.0f;
	}

	calc_rotation_and_translation_matrix (matrix, 
	                                      obj->x_pos, obj->y_pos, 0.0f, 
	                                      x_rot, obj->y_rot, z_rot);
	matrix_mul_aabb (box, matrix);

	return 1;
}
#endif // CLUSTER_INSIDES

int add_2d_obj(char * file_name, float x_pos, float y_pos, float z_pos, float x_rot, float y_rot, float z_rot)
{
	size_t i;
	char fname[80];
	obj_2d_def *returned_obj_2d_def;
	obj_2d *our_object;
	short sector;

	our_object =(obj_2d*) calloc(1, sizeof(obj_2d));

	//find a free spot, in the obj_2d_list
	i=0;
	while(i<MAX_OBJ_2D)
		{
			if(!obj_2d_list[i])break;
			i++;
		}

	//but first convert any '\' in '/'
	clean_file_name(fname, file_name, sizeof(fname));

	returned_obj_2d_def=load_obj_2d_def_cache(fname);
	if(!returned_obj_2d_def)
	{
		log_error(__FILE__, __LINE__, "Error: Can't load 2d object: %s\n", fname);
		return 0;
	}

	sprintf(our_object->file_name,"%s",fname);
	our_object->x_pos=x_pos;
	our_object->y_pos=y_pos;
	our_object->z_pos=z_pos;

	our_object->x_rot=x_rot;
	our_object->y_rot=y_rot;
	our_object->z_rot=z_rot;
	our_object->obj_pointer=returned_obj_2d_def;
	obj_2d_list[i]=our_object;

	//get the current sector
	sector=(short)((y_pos/sector_size_y)*(map_meters_size_x/sector_size_x)+(x_pos/sector_size_x));
	our_object->sector=sector;

	return i;
}

void display_2d_objects()
{

	int i;
	int x,y;

	x= (int)-camera_x;
	y= (int)-camera_y;
	glDisable(GL_CULL_FACE);
	for(i=0;i<MAX_OBJ_2D;i++)
		{
			if(obj_2d_list[i])
			     {
			         int dist1;
			         int dist2;

			         dist1= x-(int)obj_2d_list[i]->x_pos;
			         dist2= y-(int)obj_2d_list[i]->y_pos;
			         if(dist1*dist1+dist2*dist2<=((40*40)*(zoom_level/15.75)))
                     	draw_2d_object(obj_2d_list[i]);
                 }
		}
}
