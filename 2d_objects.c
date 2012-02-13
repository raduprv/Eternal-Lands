#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include "2d_objects.h"
#include "asc.h"
#include "draw_scene.h"
#include "elmemory.h"
#include "errors.h"
#include "init.h"
#include "load_gl_extensions.h"
#include "map.h"
#include "platform.h"
#include "shadows.h"
#include "textures.h"
#include "tiles.h"
#include "translate.h"
#include "io/elfilewrapper.h"
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif
#ifdef CLUSTER_INSIDES
#include "cluster.h"
#endif
#ifdef FSAA
#include "fsaa/fsaa.h"
#endif /* FSAA */

#define INVALID -1
#define GROUND 0
#define PLANT 1
#define FENCE 2

obj_2d *obj_2d_list[MAX_OBJ_2D];

#ifdef FASTER_MAP_LOAD
static obj_2d_def* obj_2d_def_cache[MAX_OBJ_2D_DEF];
static int obj_2d_cache_used = 0;
#else
obj_2d_cache_struct obj_2d_def_cache[MAX_OBJ_2D_DEF];
#endif

int map_meters_size_x;
int map_meters_size_y;
float texture_scale=12.0;

void draw_2d_object(obj_2d *object_id)
{
	float render_x_start,render_y_start,u_start,v_start,u_end,v_end;
	float x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;
	float x_size,y_size;
	int object_type;
	obj_2d_def *obj_def_pointer;

	if(!object_id->display) return;	// not currently on the map, ignore it

	obj_def_pointer=object_id->obj_pointer;

	u_start=obj_def_pointer->u_start;
	u_end=obj_def_pointer->u_end;
 	v_start=obj_def_pointer->v_start;
	v_end=obj_def_pointer->v_end;
	x_size=obj_def_pointer->x_size;
	y_size=obj_def_pointer->y_size;
	render_x_start=-x_size/2.0f;

	object_type=obj_def_pointer->object_type;
	if(object_type==GROUND)render_y_start=-y_size/2;
	else	render_y_start=0;

	glPushMatrix();//we don't want to affect the rest of the scene

	x_pos=object_id->x_pos;
	y_pos=object_id->y_pos;
	z_pos=object_id->z_pos;
	if (object_type != PLANT)
	{
		glMultMatrixf(object_id->matrix);
		z_rot = object_id->z_rot;
	}
	else
	{
		glTranslatef (x_pos, y_pos, 0);

		x_rot = object_id->x_rot + 90;
		y_rot = object_id->y_rot;
		z_rot=-rz;
		glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
		glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
		glRotatef(y_rot, 0.0f, 1.0f, 0.0f);
	}

#ifdef	NEW_TEXTURES
	bind_texture(obj_def_pointer->texture_id);
#else	/* NEW_TEXTURES */
	get_and_set_texture_id(obj_def_pointer->texture_id);
#endif	/* NEW_TEXTURES */

	if (dungeon || (!clouds_shadows && !use_shadow_mapping))
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
			float cos_m,sin_m;

			glBegin(GL_QUADS);

			m=(-z_rot)*M_PI/180;
			cos_m=cos(m);
			sin_m=sin(m);

			x=render_x_start;
			y=render_y_start;
			x1=x*cos_m+y*sin_m;
			y1=y*cos_m-x*sin_m;
			x1=x_pos+x1;
			y1=y_pos+y1;

			ELglMultiTexCoord2fARB(base_unit,u_start,v_start);
			ELglMultiTexCoord2fARB(detail_unit,x1/texture_scale
								 +clouds_movement_u,y1/texture_scale
								 +clouds_movement_v);
			glVertex3f(x,y,z_pos);

			x=render_x_start;
			y=render_y_start+y_size;
			x1=x*cos_m+y*sin_m;
			y1=y*cos_m-x*sin_m;
			x1=x_pos+x1;
			y1=y_pos+y1;

			ELglMultiTexCoord2fARB(base_unit,u_start,v_end);
			ELglMultiTexCoord2fARB(detail_unit,x1/texture_scale
								 +clouds_movement_u,y1/texture_scale
								 +clouds_movement_v);
			glVertex3f(x,y,z_pos);

			x=render_x_start+x_size;
			y=render_y_start+y_size;
			x1=x*cos_m+y*sin_m;
			y1=y*cos_m-x*sin_m;
			x1=x_pos+x1;
			y1=y_pos+y1;

			ELglMultiTexCoord2fARB(base_unit,u_end,v_end);
			ELglMultiTexCoord2fARB(detail_unit,x1/texture_scale
								 +clouds_movement_u,y1/texture_scale
								 +clouds_movement_v);
			glVertex3f(x,y,z_pos);

			x=render_x_start+x_size;
			y=render_y_start;
			x1=x*cos_m+y*sin_m;
			y1=y*cos_m-x*sin_m;
			x1=x_pos+x1;
			y1=y_pos+y1;


			ELglMultiTexCoord2fARB(base_unit,u_end,v_start);
			ELglMultiTexCoord2fARB(detail_unit,x1/texture_scale
								 +clouds_movement_u,y1/texture_scale
								 +clouds_movement_v);
			glVertex3f(x,y,z_pos);
    		glEnd();
		}
	glPopMatrix();//restore the scene
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

#ifdef FASTER_MAP_LOAD
static void parse_2d0(const char* desc, Uint32 len, const char* cur_dir,
	obj_2d_def *def)
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
#ifdef	NEW_TEXTURES
			def->texture_id = load_texture_cached(texture_file_name, tt_mesh);
#else	/* NEW_TEXTURES */
			def->texture_id = load_texture_cache_deferred(texture_file_name, 0);
#endif	/* NEW_TEXTURES */
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
	def->v_start = 1.0f - (float)v_start/file_y_len;
	def->v_end = 1.0f - (float)v_end/file_y_len;
	if (def->alpha_test < 0)
		def->alpha_test = 0;
}

static obj_2d_def* load_obj_2d_def(const char *file_name)
{
	int f_size;
	el_file_ptr file = NULL;
	char cur_dir[200]={0};
	obj_2d_def *cur_object;
	const char *obj_file_mem;
	const char* sep;

	sep = strrchr(file_name, '/');
	if (!sep || sep == file_name)
		*cur_dir = '\0';
	else
		my_strncp(cur_dir, file_name, sep-file_name+1);

	file = el_open(file_name);
	if (!file)
	{
		LOG_ERROR("%s: %s \"%s\": %s\n", reg_error_str, cant_open_file, file_name, strerror(errno));
		return NULL;
	}

	obj_file_mem = el_get_pointer(file);
	if (!obj_file_mem)
	{
		LOG_ERROR("%s: %s (read)\"%s\"\n", reg_error_str, cant_open_file, file_name);
		el_close(file);
		return NULL;
	}
	f_size = el_get_size(file);

	//ok, the file is loaded, so parse it
	cur_object=calloc(1, sizeof(obj_2d_def));
	my_strncp(cur_object->file_name, file_name,
		sizeof(cur_object->file_name));
	parse_2d0(obj_file_mem, f_size, cur_dir, cur_object);

	el_close(file);

	return cur_object;
}
#else  // FASTER_MAP_LOAD
static obj_2d_def* load_obj_2d_def(const char *file_name)
{
	int f_size;
	int i,k,l;
	el_file_ptr file = NULL;
	char cur_dir[200]={0};
	obj_2d_def *cur_object;
	char *obj_file_mem;
	char texture_file_name[256] = {0};
	float x_size,y_size;
	float alpha_test;
	int file_x_len;
	int file_y_len;
	int u_start,u_end,v_start,v_end;

	cur_object=calloc(1, sizeof(obj_2d_def));
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
	if(file == NULL){
		LOG_ERROR("%s: %s \"%s\": %s\n", reg_error_str, cant_open_file, file_name, strerror(errno));
		free(cur_object);
		return NULL;
	}

	obj_file_mem = el_get_pointer(file);
	
	if(obj_file_mem == NULL){
		LOG_ERROR("%s: %s (read)\"%s\"\n", reg_error_str, cant_open_file, file_name);
		el_close(file);
		free(cur_object);
		return NULL;
	}

	f_size = el_get_size(file);

	//ok, the file is loaded, so parse it
	file_x_len=get_integer_after_string("file_x_len:",obj_file_mem,f_size);
	file_y_len=get_integer_after_string("file_y_len:",obj_file_mem,f_size);
	u_start=get_integer_after_string("u_start:",obj_file_mem,f_size);
	u_end=get_integer_after_string("u_end:",obj_file_mem,f_size);
	v_start=get_integer_after_string("v_start:",obj_file_mem,f_size);
	v_end=get_integer_after_string("v_end:",obj_file_mem,f_size);
	x_size=get_float_after_string("x_size:",obj_file_mem,f_size);
	y_size=get_float_after_string("y_size:",obj_file_mem,f_size);
	alpha_test=get_float_after_string("alpha_test:",obj_file_mem,f_size);
	if(alpha_test<0)alpha_test=0;

	//get the proper u/v coordinates
	cur_object->u_start=(float)u_start/file_x_len;
	cur_object->u_end=(float)u_end/file_x_len;
	cur_object->v_start=1.0f-(float)v_start/file_y_len;
	cur_object->v_end=1.0f-(float)v_end/file_y_len;
	cur_object->x_size=x_size;
	cur_object->y_size=y_size;
	cur_object->alpha_test=alpha_test;

	//now  find the texture name
	i=get_string_occurance("texture:",obj_file_mem,40,0);
	obj_file_mem+=i;
	k=0;
	//find the file name
	while(k<128)
		{
			if(obj_file_mem[k]!=' ' && obj_file_mem[k]!=0x0a)break;
			k++;
		}
	//we found the beginning of the file name
	//now, copy the current directory string to the file_name string
	i=strlen(cur_dir);
	l=0;
	while(l<i)
		{
			texture_file_name[l]=cur_dir[l];
			l++;
		}
	while(l<128)
		{
			if(obj_file_mem[k]!=' ' && obj_file_mem[k]!=0x0a
			   && obj_file_mem[k]!=0x0d)
				{
					texture_file_name[l]=obj_file_mem[k];
					k++;
					l++;
				}
			else
				{
					texture_file_name[l]=0;
					break;
				}
		}

#ifdef	NEW_TEXTURES
	cur_object->texture_id = load_texture_cached(texture_file_name, tt_mesh);
#else	/* NEW_TEXTURES */
	cur_object->texture_id=load_texture_cache_deferred(texture_file_name,0);
#endif	/* NEW_TEXTURES */
	//now get the object type
	i=get_string_occurance("type:",obj_file_mem,f_size,0);
	obj_file_mem+=i;
	k=0;
	for(k=0;k<10;k++)
		{
			if(obj_file_mem[k]==0x0a)
				{
					cur_object->object_type=INVALID;
					break;
				}
			if(obj_file_mem[k]==' ')continue;

			if(obj_file_mem[k]=='g' || obj_file_mem[k]=='G')
				{
					cur_object->object_type=GROUND;
					break;
				}

			if(obj_file_mem[k]=='p' || obj_file_mem[k]=='P')
				{
					cur_object->object_type=PLANT;
					break;
				}

			if(obj_file_mem[k]=='f' || obj_file_mem[k]=='F')
				{
					cur_object->object_type=FENCE;
					break;
				}
		}
	el_close(file);

	return cur_object;
}
#endif // FASTER_MAP_LOAD

#ifdef FASTER_MAP_LOAD
static int cache_cmp_string(const void* str, const void *dptr)
{
	const obj_2d_def *def = *((const obj_2d_def**)dptr);
	return strcmp(str, def->file_name);
}

//Tests to see if an obj_2d object is already loaded.
//If it is, return the handle.
//If not, load it, and return the handle
static obj_2d_def* load_obj_2d_def_cache(const char* file_name)
{
	obj_2d_def *def, **defp;
	int i;

	defp = bsearch(file_name, obj_2d_def_cache,
		obj_2d_cache_used, sizeof(obj_2d_def*),
		cache_cmp_string);
	if (defp)
		return *defp;

	//asc not found in the cache, so load it ...
	def = load_obj_2d_def(file_name);

	// no object found, so nothing to store in the cache
	if (def == NULL)
		return NULL;

	// ... and store it
	if (obj_2d_cache_used < MAX_OBJ_2D_DEF)
	{
		for (i = 0; i < obj_2d_cache_used; i++)
		{
			if (strcmp(file_name, obj_2d_def_cache[i]->file_name) <= 0)
			{
				memmove(obj_2d_def_cache+(i+1), obj_2d_def_cache+i,
					(obj_2d_cache_used-i)*sizeof(obj_2d_def*));
				break;
			}
		}
		obj_2d_def_cache[i] = def;
		obj_2d_cache_used++;
	}

	return def;
}
#else  // FASTER_MAP_LOAD
//Tests to see if an obj_2d object is already loaded.
//If it is, return the handle.
//If not, load it, and return the handle
static obj_2d_def* load_obj_2d_def_cache(const char * file_name)
{
	int i;
	obj_2d_def * obj_2d_def_id;

	for(i=0;i<MAX_OBJ_2D_DEF;i++)
		{
			if(!strcasecmp(obj_2d_def_cache[i].file_name, file_name))
				{
					// we found a cached copy, use it
					return obj_2d_def_cache[i].obj_2d_def_id;
				}
			/*
			j=0;
			while(j<file_name_length)
				{
					if(obj_2d_def_cache[i].file_name[j]!=file_name[j])break;
					j++;
				}
			if(file_name_length==j)//ok, obj_2d_def already loaded
				return obj_2d_def_cache[i].obj_2d_def_id;
			*/
		}
	//asc not found in the cache, so load it, and store it
	obj_2d_def_id= load_obj_2d_def(file_name);

	//find a place to store it
	i=0;
	while(i<MAX_OBJ_2D_DEF)
		{
			if(!obj_2d_def_cache[i].file_name[0])//we found a place to store it
				{
					safe_snprintf(obj_2d_def_cache[i].file_name, sizeof(obj_2d_def_cache[i].file_name), "%s", file_name);
					obj_2d_def_cache[i].obj_2d_def_id=obj_2d_def_id;
					return obj_2d_def_id;
				}
			i++;
		}

	return obj_2d_def_id;
}
#endif // FASTER_MAP_LOAD

#ifdef CLUSTER_INSIDES
int get_2d_bbox (int id, AABBOX* box)
{
	const obj_2d* obj;
	const obj_2d_def* def;
	float len_x;
	float len_y;

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
			box->bbmin[X] *= M_SQRT2;
			box->bbmax[X] *= M_SQRT2;
			box->bbmin[Y] *= M_SQRT2;
			box->bbmax[Y] *= M_SQRT2;
		}
	}
	box->bbmin[Z] = obj->z_pos;
	box->bbmax[Z] = obj->z_pos;

	matrix_mul_aabb (box, obj->matrix);

	return 1;
}
#endif // CLUSTER_INSIDES

#ifdef FASTER_MAP_LOAD
int add_2d_obj(int id_hint, const char* file_name,
	float x_pos, float y_pos, float z_pos,
	float x_rot, float y_rot, float z_rot, unsigned int dynamic)
{
	int id;
	char fname[128];
	obj_2d_def *returned_obj_2d_def;
	obj_2d *our_object;
#ifndef CLUSTER_INSIDES
	float len_x, len_y;
#endif
	unsigned int alpha_test, texture_id;
	AABBOX bbox;

	id = id_hint;
	if (obj_2d_list[id])
	{
		// occupied, find a free spot in the obj_2d_list
		for(id = 0; id < MAX_OBJ_2D; id++)
		{
			if(!obj_2d_list[id])
				break;
		}
		if (id >= MAX_OBJ_2D)
		{
			LOG_ERROR("2D object list is full");
			return -1;
		}
	}

	// convert filename to lower case and replace any '\' by '/'
	clean_file_name(fname, file_name, sizeof(fname));

	returned_obj_2d_def = load_obj_2d_def_cache(fname);
	if(!returned_obj_2d_def)
	{
		LOG_ERROR ("%s: %s: %s", reg_error_str, cant_load_2d_object, fname);
		return -1;
	}

	our_object = calloc(1, sizeof(obj_2d));
	our_object->x_pos = x_pos;
	our_object->y_pos = y_pos;
	our_object->z_pos = z_pos;
	our_object->x_rot = x_rot;
	our_object->y_rot = y_rot;
	our_object->z_rot = z_rot;
	our_object->obj_pointer = returned_obj_2d_def;
	our_object->display = 1;
	our_object->state = 0;

	obj_2d_list[id] = our_object;

#ifdef CLUSTER_INSIDES
	if (returned_obj_2d_def->object_type == PLANT)
	{
		x_rot += 90.0f;
		z_rot = 0.0f;
	}
	else if (returned_obj_2d_def->object_type == FENCE)
	{
		x_rot += 90.0f;
	}
	calc_rotation_and_translation_matrix(our_object->matrix, x_pos, y_pos, 0.0f, x_rot, y_rot, z_rot);

	our_object->cluster = get_cluster((int)(x_pos/0.5f), (int)(y_pos/0.5f));
	current_cluster = our_object->cluster;
#else
	len_x = (returned_obj_2d_def->x_size);
	len_y = (returned_obj_2d_def->y_size);
	bbox.bbmin[X] = -len_x*0.5f;
	bbox.bbmax[X] = len_x*0.5f;
	if (returned_obj_2d_def->object_type == GROUND)
	{
		bbox.bbmin[Y] = -len_y*0.5f;
		bbox.bbmax[Y] = len_y*0.5f;
	}
	else
	{
		bbox.bbmin[Y] = 0.0f;
		bbox.bbmax[Y] = len_y;
		if (returned_obj_2d_def->object_type == PLANT)
		{
			x_rot += 90.0f;
			z_rot = 0.0f;
			bbox.bbmin[X] *= M_SQRT2;
			bbox.bbmax[X] *= M_SQRT2;
			bbox.bbmin[Y] *= M_SQRT2;
			bbox.bbmax[Y] *= M_SQRT2;
		}
		else if (returned_obj_2d_def->object_type == FENCE)
		{
			x_rot += 90.0f;
		}
	}
	bbox.bbmin[Z] = z_pos;
	bbox.bbmax[Z] = z_pos;

	calc_rotation_and_translation_matrix(our_object->matrix, x_pos, y_pos, 0.0f, x_rot, y_rot, z_rot);
	matrix_mul_aabb(&bbox, our_object->matrix);
#endif // CLUSTER_INSIDES

	alpha_test = returned_obj_2d_def->alpha_test ? 1 : 0;
	texture_id = returned_obj_2d_def->texture_id;

#ifdef CLUSTER_INSIDES
	if (get_2d_bbox(id, &bbox))
	{
		if (main_bbox_tree_items != NULL && dynamic == 0)
			add_2dobject_to_list(main_bbox_tree_items, id, bbox, alpha_test, texture_id);
		else
			add_2dobject_to_abt(main_bbox_tree, id, bbox, alpha_test, texture_id, dynamic);
	}
#else
	if (main_bbox_tree_items != NULL && dynamic == 0)
		add_2dobject_to_list(main_bbox_tree_items, id, bbox, alpha_test, texture_id);
	else
		add_2dobject_to_abt(main_bbox_tree, id, bbox, alpha_test, texture_id, dynamic);
#endif // CLUSTER_INSIDES

	return id;
}
#else  // FASTER_MAP_LOAD
int add_2d_obj(char * file_name, float x_pos, float y_pos, float z_pos,
			   float x_rot, float y_rot, float z_rot, unsigned int dynamic)
{
	int i;//,len,k;
	char	fname[128];
	obj_2d_def *returned_obj_2d_def;
	obj_2d *our_object;
#ifndef CLUSTER_INSIDES
	float len_x, len_y;
#endif
	unsigned int alpha_test, texture_id;
	AABBOX bbox;

	//find a free spot, in the obj_2d_list
	for(i=0; i<MAX_OBJ_2D; i++)
		{
			if(!obj_2d_list[i])break;
		}

	// but first convert to lower case and replace any '\' by '/'
	clean_file_name(fname, file_name, sizeof(fname));

	returned_obj_2d_def=load_obj_2d_def_cache(fname);
	if(!returned_obj_2d_def)
	{
		LOG_ERROR ("%s: %s: %s", reg_error_str, cant_load_2d_object, fname);
		return -1;
	}

	our_object = calloc(1, sizeof(obj_2d));
	my_strncp(our_object->file_name, fname, 80);
	our_object->x_pos=x_pos;
	our_object->y_pos=y_pos;
	our_object->z_pos=z_pos;

	our_object->x_rot=x_rot;
	our_object->y_rot=y_rot;
	our_object->z_rot=z_rot;
	our_object->obj_pointer=returned_obj_2d_def;
	our_object->display= 1;
	our_object->state= 0;

	obj_2d_list[i]=our_object;

#ifdef CLUSTER_INSIDES
	if (returned_obj_2d_def->object_type == PLANT)
	{
		x_rot += 90.0f;
		z_rot = 0.0f;
	}
	else if (returned_obj_2d_def->object_type == FENCE)
	{
		x_rot += 90.0f;
	}
	calc_rotation_and_translation_matrix(our_object->matrix, x_pos, y_pos, 0.0f, x_rot, y_rot, z_rot);

	our_object->cluster = get_cluster ((int)(x_pos/0.5f), (int)(y_pos/0.5f));
	current_cluster = our_object->cluster;
#else
	len_x = (returned_obj_2d_def->x_size);
	len_y = (returned_obj_2d_def->y_size);
	bbox.bbmin[X] = -len_x*0.5f;
	bbox.bbmax[X] = len_x*0.5f;
	if (returned_obj_2d_def->object_type == GROUND)
	{
		bbox.bbmin[Y] = -len_y*0.5f;
		bbox.bbmax[Y] = len_y*0.5f;
	}
	else
	{
		bbox.bbmin[Y] = 0.0f;
		bbox.bbmax[Y] = len_y;
		if (returned_obj_2d_def->object_type == PLANT)
		{
			x_rot += 90.0f;
			z_rot = 0.0f;
			bbox.bbmin[X] *= M_SQRT2;
			bbox.bbmax[X] *= M_SQRT2;
			bbox.bbmin[Y] *= M_SQRT2;
			bbox.bbmax[Y] *= M_SQRT2;
		}
		else if (returned_obj_2d_def->object_type == FENCE) x_rot += 90.0f;
	}
	bbox.bbmin[Z] = z_pos;
	bbox.bbmax[Z] = z_pos;
	
	calc_rotation_and_translation_matrix(our_object->matrix, x_pos, y_pos, 0.0f, x_rot, y_rot, z_rot);
	matrix_mul_aabb(&bbox, our_object->matrix);
#endif // CLUSTER_INSIDES

	if (returned_obj_2d_def->alpha_test) alpha_test = 1;
	else alpha_test = 0;

	texture_id = returned_obj_2d_def->texture_id;
	
#ifdef CLUSTER_INSIDES
	if (get_2d_bbox (i, &bbox))
	{
		if (main_bbox_tree_items != NULL && dynamic == 0)
			add_2dobject_to_list (main_bbox_tree_items, i, bbox, alpha_test, texture_id);
		else
			add_2dobject_to_abt (main_bbox_tree, i, bbox, alpha_test, texture_id, dynamic);
	}
#else
	if ((main_bbox_tree_items != NULL) && (dynamic == 0)) add_2dobject_to_list(main_bbox_tree_items, i, bbox, alpha_test, texture_id);
	else add_2dobject_to_abt(main_bbox_tree, i, bbox, alpha_test, texture_id, dynamic);
#endif // CLUSTER_INSIDES
	
	return i;
}
#endif // FASTER_MAP_LOAD

#ifdef NEW_SOUND
const char* get_2dobject_at_location(float x_pos, float y_pos)
{
	int i;
	float offset = 0.5f;
	for (i = 0; i < MAX_OBJ_2D; i++)
	{
		if (obj_2d_list[i]
			&& obj_2d_list[i]->x_pos > (x_pos - offset) && obj_2d_list[i]->x_pos < (x_pos + offset)
			&& obj_2d_list[i]->y_pos > (y_pos - offset) && obj_2d_list[i]->y_pos < (y_pos + offset)
			&& obj_2d_list[i]->display && obj_2d_list[i]->obj_pointer->object_type == GROUND)
		{
#ifdef FASTER_MAP_LOAD
			return obj_2d_list[i]->obj_pointer->file_name;
#else
			return obj_2d_list[i]->file_name;
#endif
		}
	}
	return "";
}
#endif // NEW_SOUND

#ifdef MAP_EDITOR2
void get_2d_object_under_mouse()
{
	unsigned int i, l;
	float least_z = 1.0f;
	
	//First draw everyone with the same alpha test
    	
	glPushMatrix();
	glClearDepth(least_z);
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	
	for (i = get_intersect_start(main_bbox_tree, TYPE_2D_NO_ALPHA_OBJECT); i < get_intersect_stop(main_bbox_tree, TYPE_2D_NO_ALPHA_OBJECT); i++)
	{
		l = get_intersect_item_ID(main_bbox_tree, i);
#ifdef EXTRA_DEBUG
		if (!obj_2d_list[l])
		{
			ERR();
			continue;
		}
#endif
		draw_2d_object(obj_2d_list[l]);
		if(evaluate_collision(&least_z)){
			selected_2d_object = l;
		}
	}
	
	//Then draw all that needs a change
	for (i = get_intersect_start(main_bbox_tree, TYPE_2D_ALPHA_OBJECT); i < get_intersect_stop(main_bbox_tree, TYPE_2D_ALPHA_OBJECT); i++)
	{
		l = get_intersect_item_ID(main_bbox_tree, i);
#ifdef EXTRA_DEBUG
		if (!obj_2d_list[l])
		{
			ERR();
			continue;
		}
#endif
		draw_2d_object(obj_2d_list[l]);
		if(evaluate_collision(&least_z)){
			selected_2d_object = l;
		}
	}
	
	glPopMatrix();
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}
#endif

void display_2d_objects()
{
	unsigned int i, l, start, stop;
#ifdef  SIMPLE_LOD
	int dist;
	int x, y;
#endif //SIMPLE_LOD
#ifdef CLUSTER_INSIDES_OLD
	short cluster = get_actor_cluster ();
#endif

#ifdef  SIMPLE_LOD
	x= -camera_x;
	y= -camera_y;
#endif //SIMPLE_LOD

	//First draw everyone with the same alpha test
#ifdef	FSAA
	if (fsaa > 1)
	{
		glEnable(GL_MULTISAMPLE);
	}
#endif	/* FSAA */
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.18f);

	if (!dungeon && !(!clouds_shadows && !use_shadow_mapping))
	{
		if(clouds_shadows)
		{
			//bind the detail texture
			ELglActiveTextureARB(detail_unit);
			glEnable(GL_TEXTURE_2D);
			//glBindTexture(GL_TEXTURE_2D, texture_cache[ground_detail_text].texture_id);
#ifdef	NEW_TEXTURES
			bind_texture_unbuffered(ground_detail_text);
#else	/* NEW_TEXTURES */
			glBindTexture(GL_TEXTURE_2D, get_texture_id(ground_detail_text));
#endif	/* NEW_TEXTURES */
		}
		ELglActiveTextureARB(base_unit);
		glEnable(GL_TEXTURE_2D);
	}

	get_intersect_start_stop(main_bbox_tree, TYPE_2D_NO_ALPHA_OBJECT, &start, &stop);
	for (i = start; i < stop; i++)
	{
		l = get_intersect_item_ID(main_bbox_tree, i);
#ifdef CLUSTER_INSIDES_OLD
		if (obj_2d_list[l]->cluster && obj_2d_list[l]->cluster != cluster)
			// Object is on a different cluster as our actor, don't show it
			continue;
#endif
#ifdef  SIMPLE_LOD
		// simple size/distance culling
		dist= (x-obj_2d_list[l]->x_pos)*(x-obj_2d_list[l]->x_pos) + (y-obj_2d_list[l]->y_pos)*(y-obj_2d_list[l]->y_pos);
		if(/*dist > 10*10 &&*/ 1000*max2f(obj_2d_list[l]->obj_pointer->x_size, obj_2d_list[l]->obj_pointer->y_size)/(dist) < 5) continue;
#endif  //SIMPLE_LOD
		draw_2d_object(obj_2d_list[l]);
	}
	
	//Then draw all that needs a change
	get_intersect_start_stop(main_bbox_tree, TYPE_2D_ALPHA_OBJECT, &start, &stop);
	for (i = start; i < stop; i++)
	{
		l = get_intersect_item_ID(main_bbox_tree, i);
#ifdef  SIMPLE_LOD
		// simple size/distance culling
		dist= (x-obj_2d_list[l]->x_pos)*(x-obj_2d_list[l]->x_pos) + (y-obj_2d_list[l]->y_pos)*(y-obj_2d_list[l]->y_pos);
		if(/*dist > 10*10 &&*/ 1000*max2f(obj_2d_list[l]->obj_pointer->x_size, obj_2d_list[l]->obj_pointer->y_size)/(dist) < 5) continue;
#endif  //SIMPLE_LOD
		glAlphaFunc(GL_GREATER, obj_2d_list[l]->obj_pointer->alpha_test);
		draw_2d_object(obj_2d_list[l]);
	}

	if (!dungeon && !(!clouds_shadows && !use_shadow_mapping))
	{
    		//disable the multitexturing
		ELglActiveTextureARB(detail_unit);
		glDisable(GL_TEXTURE_2D);
		ELglActiveTextureARB(base_unit);
	}

#ifdef	FSAA
	if (fsaa > 1)
	{
		glDisable(GL_MULTISAMPLE);
	}
#endif	/* FSAA */
	glDisable(GL_ALPHA_TEST);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void destroy_2d_object(int i)
{
	if (i < 0 || i >= MAX_OBJ_2D || !obj_2d_list[i])
		return;
	delete_2dobject_from_abt(main_bbox_tree, i, obj_2d_list[i]->obj_pointer->alpha_test);
	free(obj_2d_list[i]);
	obj_2d_list[i] = NULL;
}

void destroy_all_2d_objects()
{
	int i;
	for (i = 0; i < MAX_OBJ_2D; i++)
	{
		free(obj_2d_list[i]);
		obj_2d_list[i] = NULL;
	}
}

// for support of the 1.0.3 server, change if an object is to be displayed or not
void set_2d_object (Uint8 display, const void *ptr, int len)
{
	const Uint32 *id_ptr = ptr;
	
	// first look for the override to process ALL objects
	if (len < sizeof(*id_ptr) ){
		int	i;
		
		for (i = 0; i < MAX_OBJ_2D; i++)
		{
			if (obj_2d_list[i]){
				obj_2d_list[i]->display= display;
			}
		}
	} else {
		int idx = 0;
		
		while(len >= sizeof(*id_ptr)){
			Uint32 obj_id = SDL_SwapLE32(id_ptr[idx]);
			
			if(obj_id < MAX_OBJ_2D && obj_2d_list[obj_id]){
				obj_2d_list[obj_id]->display= display;
				idx++;
				len-= sizeof(*id_ptr);
			}
		}
	}
}

// for future expansion
void state_2d_object (Uint8 state, const void *ptr, int len)
{
	const Uint32 *id_ptr = ptr;
	
	// first look for the override to process ALL objects
	if (len < sizeof(*id_ptr) ){
		int	i;
		
		for (i = 0; i < MAX_OBJ_2D; i++)
		{
			if (obj_2d_list[i]){
				obj_2d_list[i]->state= state;
			}
		}
	} else {
		int idx = 0;
		
		while(len >= sizeof(*id_ptr)){
			Uint32 obj_id = SDL_SwapLE32(id_ptr[idx]);
			
			if(obj_id < MAX_OBJ_2D && obj_2d_list[obj_id]){
				obj_2d_list[obj_id]->state= state;
				idx++;
				len -= sizeof (*id_ptr);
			}
		}
	}
}
