#ifndef __obj_3d_H__
#define __obj_3d_H__

/**Also see e3d.h*/

/** Draws the 3d object pointed to with object_id*/
void draw_3d_object(object3d * object_id);

/** Loads an e3d object - if valid it returns a pointer to this object, otherwise it returns NULL.*/
e3d_object * load_e3d(char *file_name);

/** Checks if the e3dcache contains the e3d object already; 
 * if that's the case, it returns a pointer to that object. 
 * Otherwise it loads the e3d-object using load_e3d(file_name)
 * and returns a pointer to that object. */
e3d_object * load_e3d_cache(char * file_name);

/** Adds an e3d object to the current map*/
int add_e3d(char * file_name, float x_pos, float y_pos, float z_pos, 
			float x_rot, float y_rot, float z_rot, char self_lit, char blended,
			float r, float g, float b);

/** Cycles through the objects_list, and displays the 3d object if it's within a visible distance (dist_x^2+dist_y^2<=29*29)*/
void display_objects();

/** Calculate the clouds map for multi-texturing.*/
void compute_clouds_map(object3d * object_id);

/** Clears the clouds cache (called every 10 seconds)*/
void clear_clouds_cache();

/** Destroyes the 3d object on position i in the objects_list*/
void destroy_3d_object(int i);

#endif
