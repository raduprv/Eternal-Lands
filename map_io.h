/*!
 * \file
 * \ingroup maps
 * \brief loading, saving and handling of maps
 */
#ifndef __MAP_IO_H__
#define __MAP_IO_H__

extern char map_file_name[60]; /*!< filename of the current map */

/*!
 * The structure used for IO of object3d files.
 */
typedef struct
{
	char file_name[80]; /*!< the filename of the object3d file */
    
    /*! \name Position of the object3d 
     * @{ */
	float x_pos;
	float y_pos;
	float z_pos;
    /*! @} */

    /*! \name Rotation of the object3d 
     * @{ */
	float x_rot;
	float y_rot;
	float z_rot;
    /*! @} */

	char self_lit; /*!< indicator if this object3d is self lit or not */
	char blended; /*!< indicates whether this object3d is blended with some other objects or not */
	float r,g,b; /*!< red, green and blue color values of the object */
	char reserved[24]; /*!< reserved for future expansions. */

}object3d_io;

/*!
 * This structure defines the data used for IO of obj_2d files.
 */
typedef struct
{
	char file_name[80]; /*!< the filename of the obj_2d file. */
    
    /*! \name Position of the obj_2d 
     * @{ */
	float x_pos;
	float y_pos;
	float z_pos;
    /*! @} */
    
    /*! \name Rotation of the obj_2d 
     * @{ */
	float x_rot;
	float y_rot;
	float z_rot;
    /*! @} */

	char reserved[24]; /*!< reserved for future expansions */
}obj_2d_io;

/*!
 * Is used for the IO of lights
 */
typedef struct
{
    /*! \name Position of the light 
     * @{ */
	float pos_x;
	float pos_y;
	float pos_z;
    /*! @} */
    
    /*! \name Color of the light 
     * @{ */
	float r;
	float g;
	float b;
    /*! @} */
    
	char reserved[15]; /*!< reserved for future expansions */
}light_io;

/*!
 * Used to load or save particles to and from a file
 */
typedef struct
{
	char file_name[80]; /*!< the filename of the particle file */
    
    /*! \name Position of the particle 
     * @{ */
	float x_pos;
	float y_pos;
	float z_pos;
    /*! @} */
    
	char reserved[10]; /*!< reserved for future expansions */
}particles_io;

/*!
 * Defines the header of map files
 */
typedef struct
{
	char file_sig[4]; /*!< file magic number: should be "elmf", or else the map is invalid */
    
    /*! \name Length of the tile map in x and y direction 
     * @{ */
	int tile_map_x_len;
	int tile_map_y_len;
    /*! @} */
    
	int tile_map_offset; /*!< offset of the tile map */
	int height_map_offset; /*!< offset of the height map */
    
    /*! \name obj_3d related fields 
     * @{ */
	int obj_3d_struct_len; /*!< size of the \see obj_3d struct */
	int obj_3d_no; /*!< id of the current obj_3d */
	int obj_3d_offset; /*!< offset for the current obj_3d */
    /*! @} */
    
    /*! \name obj_2d related fields 
     * @{ */
	int obj_2d_struct_len; /*!< size of the \see obj_2d struct */
	int obj_2d_no; /*!< id of the current obj_2d */
	int obj_2d_offset; /*!< offset of the current obj_2d */
    /*! @} */
    
    /*! \name lights related fields 
     * @{ */
	int lights_struct_len; /*!< size of the \see lights_struct structure */
	int lights_no; /*!< id of the current light */
	int lights_offset; /*!< offset of the current light */
    /*! @} */
    
	char dungeon; /*!< inidicated whether we are in a dungeon (no sun) */
    
    /*! \name Reserved for future expansions 
     * @{ */
	char res_2;
	char res_3;
	char res_4;
    /*! @} */
    
    /*! \name ambient color values 
     * @{ */
	float ambient_r;
	float ambient_g;
	float ambient_b;
    /*! @} */
    
    /*! \name particle related fields 
     * @{ */
	int particles_struct_len; /*!< size of the particle struct */
	int particles_no; /*!< id of the current particle */
	int particles_offset; /*!< offset of the current particle */
    /*! @} */
    
    /*! \name Reserved for future expansions 
     * @{ */
	int reserved_8;
	int reserved_9;
	int reserved_10;
	int reserved_11;
	int reserved_12;
	int reserved_13;
	int reserved_14;
	int reserved_15;
	int reserved_16;
	int reserved_17;
    /*! @} */

}map_header;

extern char dungeon; /*!< inidicates whether we are in a dungeon (no sun) or not */

/*! \name ambient color values 
 * @{ */
extern float ambient_r;
extern float ambient_g;
extern float ambient_b;
/*! @} */

extern int map_type; /*!< id of the type of map we are currently using */

/*!
 * \ingroup maps
 * \brief Loads an empty map
 *
 * 	Loads an empty map in case no other map file was found
 *
 * \param name the filename of the map that failed to load.
 * \retval int  0 if nomap.elm failed to load, otherwise 1 is returned.
 */
int load_empty_map();

/*!
 * \ingroup maps
 * \brief Loads the map given by \a file_name
 *
 *      Loads the map given by \a file_name, initializes the necessary data for the map and adds the map to the current sector.
 *
 * \param file_name the filename of the map to load.
 * \retval int  0, if the file given by \a file_name could not be opened, or if the file is invalid, i.e. has an invalid magic number, else 1 is returned.
 * \callgraph
 */
int load_map (const char * file_name);

/*!
 * \ingroup maps
 * \brief Loads the given map marks for the given map
 *
 * 	Loads the map marks for the given map
 *
 */
void load_map_marks();
#endif
