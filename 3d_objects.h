/*!
 * \file
 * \ingroup 	display
 * \brief	Displays the 3d objects
 */
#ifndef __obj_3d_H__
#define __obj_3d_H__

/*!
 * \ingroup 	display_3d
 * \brief 	Draws the 3d object pointed to by object_id
 *
 * 		Is usually called from display_objects. The function draws the 3d object pointed to with object_id, which is a part of the objects_list. 
 * \param 	object_id A pointer to the object3d that is going to be rendered.
 * \return	None
 * \sa 		display_objects
 */
void draw_3d_object(object3d * object_id);

/*!
 * \ingroup 	load_3d
 * \brief	Loads an e3d object from file_name.
 *
 * 		The function loads an e3d object as specified with file_name. 
 *      e3d-objects are binary objects (the e3d_object structure with an e3d_header as the header).
 *
 * \param	file_name The filename we wish to load
 * \return 	On success the function returns a pointer to the loaded object, otherwise it returns NULL.
 * \sa		e3d_object
 * \sa		e3d_header
 * \sa		object3d
 */
e3d_object * load_e3d(char *file_name);

/*!
 * \ingroup	load_3d
 * \brief	Finds the e3d-file given by file_name in the e3d object cache.
 * 
 * 		The function tries finding the e3d object given by file_name in the 3d objects cache
 * 		If it fails it will try loading the file using load_e3d.
 * 		
 * \param	file_name The name of the e3d-file we wish to open
 * \return	Returns a pointer to the e3d object or NULL if it's not found.
 */
e3d_object * load_e3d_cache(char * file_name);

/*!
 * \ingroup	load_3d
 * \brief	Adds a 3d object to the map.
 * 
 * 		It is usually called from load_map. It adds a 3d object to the given position.
 * 		
 * \param	file_name The file name of the 3d object
 * \param	x_pos The x position
 * \param	y_pos The y position
 * \param	z_pos The z position
 * \param	x_rot The x rotation
 * \param	y_rot The y rotation
 * \param	z_rot The z rotation
 * \param	self_lit Whether the object is self-lit (enables/disables lightning)
 * \param	blended Whether the object is blended (enables/disables blending - GL_ONE,GL_ONE)
 * \param	r Red (0<=r<=1)
 * \param	g Green (0<=g<=1)
 * \param	b Blue (0<=b<=1)
 * \return	Returns 0 on error or the position in the objects_list on succes.
 */
int add_e3d(char * file_name, float x_pos, float y_pos, float z_pos, 
                float x_rot, float y_rot, float z_rot, char self_lit, char blended, 
                float r, float g, float b);

/*!
 * \ingroup	display_3d
 * \brief	Displays the 3d objects within the range
 *
 * 		Cycles through the objects_list, and displays the 3d object if it's within a visible distance (dist_x^2+dist_y^2<=29*29)
 * 		
 * \return	None
 */
void display_objects();

/*!
 * \ingroup	display_3d
 * \brief	Calculates the clouds map for multi-texturing
 * 
 * 		Calculate the clouds map for multi-texturing for the given 3d object.
 *
 * \param	object_id The 3d objects we wish to calculate the clouds map over.
 * \return	None
 */
void compute_clouds_map(object3d * object_id);

/*! 
 * \ingroup	display_3d
 * \brief	Clears the clouds cache 
 *
 * 		The function clears the clouds cache, which leads to the clouds map will have to be recalculated for the given object. 
 * 		It is called every 10 seconds.
 * 		
 * \return	None
 */
void clear_clouds_cache();

/*!
 * \ingroup	display_3d
 * \brief	Destroys the 3d object at position i in the objects list
 * 
 * 		Destroyes the 3d object on position i in the objects_list - frees the memory and sets the objects_list[i]=NULL.
 *
 * \param	i The position in the objects_list
 * \return	None
 */
void destroy_3d_object(int i);

#endif
