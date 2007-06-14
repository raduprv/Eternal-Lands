/**
 * @file
 * @ingroup maps
 * @brief loading, saving and handling of maps
 */
#ifndef	_MAP_IO_H_
#define	_MAP_IO_H_

#ifdef __cplusplus
extern "C" {
#endif

extern char map_file_name[256]; /**< filename of the current map */
#define MF_PLAINS 		0x00000001 //It can rain here
#define MF_SNOW 		0x00000002 //It can snow here
#define MF_DESERT 		0x00000004 //It can be darned hot here :P
#define MF_DUNGEON 	0x00000008
#define MF_HOUSE		0x00000010
#define MF_BOAT		0x00000020 //Tilt the ground level slightly?
/*
#define RESERVED 	0x00000040 
#define RESERVED	0x00000080 
#define RESERVED 	0x00000100 
#define RESERVED 	0x00000200 
#define RESERVED 	0x00000400 
#define RESERVED 	0x00000800 
#define RESERVED 	0x00001000 
#define RESERVED 	0x00002000 
#define RESERVED 	0x00004000 
#define RESERVED 	0x00008000 
#define RESERVED 	0x00010000 
#define RESERVED 	0x00020000 
#define RESERVED 	0x00040000 
#define RESERVED 	0x00080000 
#define RESERVED 	0x00100000 
#define RESERVED 	0x00200000 
#define RESERVED 	0x00400000 
#define RESERVED 	0x00800000 
#define RESERVED 	0x01000000
#define RESERVED 	0x02000000
#define RESERVED 	0x04000000
#define RESERVED 	0x08000000 
#define RESERVED 	0x10000000
#define RESERVED 	0x20000000
#define RESERVED 	0x40000000
#define RESERVED 	0x80000000 
*/

/**
 * The structure used for IO of object3d files.
 */
typedef struct
{
	char file_name[80]; /**< the filename of the object3d file */
    
    /** @name Position of the object3d 
     * @{ */
	float x_pos;
	float y_pos;
	float z_pos;
    /** @} */

    /** @name Rotation of the object3d 
     * @{ */
	float x_rot;
	float y_rot;
	float z_rot;
    /** @} */

	char self_lit; /**< indicator if this object3d is self lit or not */
	char blended; /**< indicates whether this object3d is blended with some other objects or not */
	float r,g,b; /**< red, green and blue color values of the object */
	char reserved[24]; /**< reserved for future expansions. */

}object3d_io;

/**
 * This structure defines the data used for IO of obj_2d files.
 */
typedef struct
{
	char file_name[80]; /**< the filename of the obj_2d file. */
    
    /** @name Position of the obj_2d 
     * @{ */
	float x_pos;
	float y_pos;
	float z_pos;
    /** @} */
    
    /** @name Rotation of the obj_2d 
     * @{ */
	float x_rot;
	float y_rot;
	float z_rot;
    /** @} */

	char reserved[24]; /**< reserved for future expansions */
}obj_2d_io;

/**
 * Is used for the IO of lights
 */
typedef struct
{
    /** @name Position of the light 
     * @{ */
	float pos_x;
	float pos_y;
	float pos_z;
    /** @} */
    
    /** @name Color of the light 
     * @{ */
	float r;
	float g;
	float b;
    /** @} */
    
	char reserved[15]; /**< reserved for future expansions */
}light_io;

/**
 * Used to load or save particles to and from a file
 */
typedef struct
{
	char file_name[80]; /**< the filename of the particle file */
    
    /** @name Position of the particle 
     * @{ */
	float x_pos;
	float y_pos;
	float z_pos;
    /** @} */
    
	char reserved[10]; /**< reserved for future expansions */
}particles_io;

/**
 * Defines the header of map files
 */
typedef struct
{
	char file_sig[4]; /**< file magic number: should be "elmf", or else the map is invalid */
    
    /** @name Length of the tile map in x and y direction 
     * @{ */
	int tile_map_x_len;
	int tile_map_y_len;
    /** @} */
    
	int tile_map_offset; /**< offset of the tile map */
	int height_map_offset; /**< offset of the height map */
    
    /** @name obj_3d related fields 
     * @{ */
	int obj_3d_struct_len; /**< size of the \see obj_3d struct */
	int obj_3d_no; /**< id of the current obj_3d */
	int obj_3d_offset; /**< offset for the current obj_3d */
    /** @} */
    
    /** @name obj_2d related fields 
     * @{ */
	int obj_2d_struct_len; /**< size of the \see obj_2d struct */
	int obj_2d_no; /**< id of the current obj_2d */
	int obj_2d_offset; /**< offset of the current obj_2d */
    /** @} */
    
    /** @name lights related fields 
     * @{ */
	int lights_struct_len; /**< size of the \see lights_struct structure */
	int lights_no; /**< id of the current light */
	int lights_offset; /**< offset of the current light */
    /** @} */
    
	char dungeon; /**< inidicated whether we are in a dungeon (no sun) */
    
    /** @name Reserved for future expansions 
     * @{ */
	char res_2;
	char res_3;
	char res_4;
    /** @} */
    
    /** @name ambient color values 
     * @{ */
	float ambient_r;
	float ambient_g;
	float ambient_b;
    /** @} */
    
    /** @name particle related fields 
     * @{ */
	int particles_struct_len; /**< size of the particle struct */
	int particles_no; /**< id of the current particle */
	int particles_offset; /**< offset of the current particle */
    /** @} */
    
    /** @name Reserved for future expansions 
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
    /** @} */

}map_header;

typedef void (update_func) (char *str, float percent);

/**
 * @ingroup maps
 * @brief Loads the map given by \a file_name
 *
 *      Loads the map given by \a file_name, initializes the necessary data for the map and adds the map to the current sector.
 *
 * @param file_name the filename of the map to load.
 * @retval int  0, if the file given by \a file_name could not be opened, or if the file is invalid, i.e. has an invalid magic number, else 1 is returned.
 * @callgraph
 */
int load_map(const char * file_name, update_func *update_function);

#ifdef __cplusplus
} // extern "C"
#endif

#endif	// _MAP_IO_H_
