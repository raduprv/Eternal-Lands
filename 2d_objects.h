#ifndef __obj_2d_H__
#define __obj_2d_H__

/**Maximum number of 2d objects in a map*/
#define max_obj_2d 15000 

/**Maximum number of loaded 2d object definitions*/
#define max_obj_2d_def 1000 

#define invalid -1
#define ground 0
#define plant 1
#define fence 2

/** Equivalent to 5 tiles - is not used yet apparently...*/
#define sector_size_x 15 
/** Equivalent to 5 tiles - is not used yet apparently...*/
#define sector_size_y 15 

/** obj_2d_def is loaded from a .2do-file and is shared amongst all objects of that type*/
typedef struct
{
	float u_start;
	float u_end;
	float v_start;
	float v_end;
	float x_size;
	float y_size;
	float alpha_test; /** Use alpha?*/
	int object_type; /** Is this a ground, fence or plant object?
			   * ground: don't change rotation
			   * plant: put in an upwards rotation (x_rot+=90) and set z_rot=-rz
			   * fence: put in an upwards rotation (x_rot+=90)
			   */
	int texture_id;  /** The location in the texture cache. */
}obj_2d_def;

/** The obj_2d determines the position and rotation of the given 2d object. Furthermore it determines the type*/
typedef struct
{
	char file_name[80];
	float x_pos;
	float y_pos;
	float z_pos;
	float x_rot;
	float y_rot;
	float z_rot;
	short sector;
	obj_2d_def *obj_pointer; /** Points to the 2d object type in the obj_2d_def list */
}obj_2d;

/**This is used for searching the 2d object cache for an already existing instance of the object definition*/
typedef struct
{
	char file_name[128];
	obj_2d_def *obj_2d_def_id;
}obj_2d_cache_struct;

extern obj_2d_cache_struct obj_2d_def_cache[max_obj_2d_def]; /** The texture cache array - holds all loaded 2d object definitions*/

extern obj_2d *obj_2d_list[max_obj_2d]; /** The 2d object array - holds all 2d objects on that map*/

extern int map_meters_size_x;
extern int map_meters_size_y;
extern float texture_scale;

/** Draws the 2D object given with object id*/
void draw_2d_object(obj_2d * object_id);

/** Loads the 2d object definition and returns a pointer to the object*/
obj_2d_def * load_obj_2d_def(char *file_name);

/** First checks for an already existing instance of the object given with file_name
 *  If found, it returns a pointer to that object, otherwise it tries loading it using load_obj_2d_def*/
obj_2d_def * load_obj_2d_def_cache(char * file_name);

/** Adds a 2d object at the given location. Returns 0 on failure and the location in the obj_2d_list if it succeeds*/
int add_2d_obj(char * file_name, float x_pos, float y_pos, float z_pos,
			   float x_rot, float y_rot, float z_rot);

/** Parses through the obj_2d_list, checking for an object within the viewing distance (dist_x^2+dist_y^2<=220)*/
void display_2d_objects();

#endif
