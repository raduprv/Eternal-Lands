/*!
 * \file
 * \brief 	Handles rendering and loading 2d objects
 * \ingroup 	display
 */
#ifndef __obj_2d_H__
#define __obj_2d_H__

/*!
 *\name 	2D Object array sizes
 *		The sizes of the arrays where we keep 2d objects and 2d object definitions
 */
/*! \{ */
#define max_obj_2d 15000 /*!<Maximum number of 2d objects in a map*/

#define max_obj_2d_def 1000 /*!<Maximum number of loaded 2d object definitions*/
/*! \} */

/*!
 *\name 	2D object types
 *		These defines sets the 2d object types
 */

/*! \{ */
#define invalid -1 /*!< Invalid object*/
#define ground 0 /*!< Ground object*/
#define plant 1 /*!< Plants and other items that needs to follow the cameras z rotation*/
#define fence 2 /*!< Fences - will be put in 90 degrees angle (x_rot+=90)*/
/*! \} */

#define sector_size_x 15 /*!< Equivalent to 5 tiles - is not used yet apparently...*/
#define sector_size_y 15 /*!< Equivalent to 5 tiles - is not used yet apparently...*/

/*! 
 * obj_2d_def is loaded from a .2do-file and is shared amongst all objects of that type in the obj_2d_def_cache array
 */
typedef struct
{
	float u_start;
	float u_end;
	float v_start;
	float v_end;
	float x_size;
	float y_size;
	float alpha_test; /*!< Use alpha?*/
	int object_type; /*!< Is this a ground, fence or plant object?
			   * ground: don't change rotation
			   * plant: put in an upwards rotation (x_rot+=90) and set z_rot=-rz
			   * fence: put in an upwards rotation (x_rot+=90)
			   */
	int texture_id;  /*!< The location in the texture cache. */
}obj_2d_def;

/*! 
 * The obj_2d determines the position and rotation of the given 2d object. Furthermore it determines the type
 */
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
	obj_2d_def *obj_pointer; /**< Points to the 2d object type in the obj_2d_def list */
}obj_2d;

/*! 
 * This is used for searching the 2d object cache for an already existing instance of the object definition
 */
typedef struct
{
	char file_name[128];
	obj_2d_def *obj_2d_def_id;
}obj_2d_cache_struct;

extern obj_2d_cache_struct obj_2d_def_cache[max_obj_2d_def]; /*!< The 2d object cache array - holds all loaded 2d object definitions*/

extern obj_2d *obj_2d_list[max_obj_2d]; /*!< The 2d object array - holds all 2d objects on that map*/

extern int map_meters_size_x;
extern int map_meters_size_y;
extern float texture_scale;

/*!
 * \ingroup	display_2d
 * \brief	Draws the 2D object pointed to by object_id
 * 
 * 		Draws the 2D object given with object_id. It is called from display_2d_objects(); if it's within the viewing distance
 * 		
 * \param	object_id A pointer to the 2d object id
 * \return 	None
 * \sa		display_2d_objects
 */
void draw_2d_object(obj_2d * object_id);

/*! 
 * \ingroup	display_2d
 * \brief	Displays the 2dobjects in the obj_2d_list array
 *
 *         	Parses through the obj_2d_list, checking for an object within the viewing distance (dist_x^2+dist_y^2<=220)
 *         	
 * \param	None
 * \return 	None
 * \sa 		draw_2d_object
 * \sa		obj_2d_list
 */
void display_2d_objects();

/*! 
 * \ingroup	load_2d
 * \brief  	Loads the 2d object definition from a .2d0-file (ascii)
 * 
 * 	   	This function parses the .2do-file as given by file_name. The .2do file has informaton about the texture, x, y, z sizes and rotations used by the 2d object
 * 	   	Have a look at the objects in ./2dobjects/ground/ 
 * 	   	
 * \param 	file_name The filename of the object we wish to load
 * \return 	A pointer to the loaded 2d object on succes, otherwise NULL
 * \sa		obj_2d_def
 * \sa		obj_2d_def_cache_struct
 * \sa		obj_2d_def_cache
 */
obj_2d_def * load_obj_2d_def(char *file_name);

/*!
 * \ingroup	load_2d
 * \brief	Finds the 2d object in the 2d object cache, or adds it to it.
 * 
 * 		Checks for an already existing instance of the object given with file_name in the 2d object cache.
 * 		
 * \param	file_name The filename of the 2d object definition we wish to load
 * \return 	On succes it returns a pointer to the loaded 2d object, otherwise it tries loading it using load_obj_2d_def - if this fails as well, it returns NULL
 */
obj_2d_def * load_obj_2d_def_cache(char * file_name);

/*!
 * \ingroup	load_2d
 * \brief	Adds a 2d object at the given location. 
 * 
 * 		Adds a 2d object at the given location.
 * 		It's usually called in the map loading process. Requires a location and rotation for the 2d object, that's loaded from the file given by the first parameter
 * 		
 * \param	file_name The filename of the object we wish to add
 * \param	x_pos The x position
 * \param	y_pos The y position
 * \param	z_pos The z position
 * \param	x_rot The x rotation
 * \param	y_rot The y rotation
 * \param	z_rot The z rotation
 * \return 	Returns 0 on failure and the location in the obj_2d_list if it succeeds
 */
int add_2d_obj(char * file_name, float x_pos, float y_pos, float z_pos,
			   float x_rot, float y_rot, float z_rot);
#endif
