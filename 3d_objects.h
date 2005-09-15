/*!
 * \file
 * \ingroup 	display
 * \brief	Displays the 3d objects
 */
#ifndef __OBJ_3D_H__
#define __OBJ_3D_H__

struct near_3d_object {
       int dist;
       float radius;
       int pos;//pos in the 3d objects list
       struct near_3d_object * next;
};

extern struct near_3d_object near_3d_objects[MAX_NEAR_3D_OBJECTS];
extern struct near_3d_object * first_near_3d_object;
extern int regenerate_near_objects;

/*!
 * \ingroup 	display_3d
 * \brief 	Draws the 3d object pointed to by object_id
 *
 * 		Is usually called from display_objects. The function draws the 3d object pointed to with object_id, which is a part of the objects_list. 
 * \param 	object_id A pointer to the object3d that is going to be rendered.
 *
 * \sa 		display_objects
 * \callgraph
 */
void draw_3d_object(object3d * object_id);

/*!
 * \ingroup	display_3d
 * \brief	Generates a tree of the nearest 3d objects
 *
 * 		Generates a linear tree of the nearest 3d objects - the objects that are nearest are located in the beginning of the tree.
 * 
 * \return	Returns 1 on succes and 0 on failure.
 */
int get_near_3d_objects();

/*!
 * \ingroup	load_3d
 * \brief	Adds a 3d object with a specific ID to the map 
 * 
 * 		Adds a 3d object to the map, at position \a id in the objects_list
 * 
 * \param	id The object ID		
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
 * \retval int	Returns 0 on error or the position in the objects_list on succes.
 * \callgraph
 */
int add_e3d_at_id (int id, const char *file_name, float x_pos, float y_pos, float z_pos, float x_rot, float y_rot, float z_rot, char self_lit, char blended, float r, float g, float b);

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
 * \retval int	Returns 0 on error or the position in the objects_list on succes.
 * 
 * \sa add_e3d_at_id
 *
 * \callgraph
 */
int add_e3d (const char * file_name, float x_pos, float y_pos, float z_pos, float x_rot, float y_rot, float z_rot, char self_lit, char blended, float r, float g, float b);

/*!
 * \ingroup	display_3d
 * \brief	Displays the 3d objects within the range
 *
 * 		Cycles through the objects_list, and displays the 3d object if it's within a visible distance (dist_x^2+dist_y^2<=29*29)
 * 		
 * \callgraph
 */
void display_objects();

/*!
 * \ingroup	display_3d
 * \brief	Displays the blended 3d objects within range
 *
 * 		Cycles through the blended objects list and displays the blended 3d objects within visible range.
 * 		
 * \callgraph
 */
void display_blended_objects();

/*! 
 * \ingroup	display_3d
 * \brief	Clears the clouds cache 
 *
 * 		The function clears the clouds cache, which leads to the clouds map will have to be recalculated for the given object. 
 * 		It is called every 10 seconds.
 * 
 * \callgraph
 */
void clear_clouds_cache();

/*!
 * \ingroup	display_3d
 * \brief	Destroys the 3d object at position i in the objects list
 * 
 * 		Destroyes the 3d object on position i in the objects_list - frees the memory and sets the objects_list[i]=NULL.
 *
 * \param	i The position in the objects_list
 *
 * \callgraph
 */
void destroy_3d_object(int i);

void destroy_e3d(e3d_object *e3d_id);

/*!
 * \ingroup	load_3d
 * \brief	Show or hide one or more 3D map objects
 * 
 * 		Show or hide 3D map objects.
 *		This routine is usually under server control to allow dynamically enabling or disabling seeing objects
 *
 * \param	display_flag whether the objects are to be displayed or not
 * \param	ptr pointer to an array of object ID's to be affected
 * \param	len the length in bytes of the array
 * \callgraph
 */
void set_3d_object (Uint8 display, const void *ptr, int len);

/*!
 * \ingroup	load_3d
 * \brief	Set the state for one or more 3D map objects
 * 
 * 		Set the sate for 3D map objects.
 *		This routine is usually under server control to allow dynamically setting a state for an object, this is for future expansion
 *
 * \param	display_flag whether the objects are to be displayed or not
 * \param	ptr pointer to an array of object ID's to be affected
 * \param	len the length in bytes of the array
 * \callgraph
 */
void state_3d_object (Uint8 state, const void *ptr, int len);

#endif
