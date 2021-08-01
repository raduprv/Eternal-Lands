/**
 * @file
 * @ingroup maps
 * @brief loading, saving and handling of maps
 */
#ifndef	_MAP_H_
#define	_MAP_H_

#include <SDL_types.h>
#include "io/map_io.h"

#include "hash.h"
#include "mapwin.h"
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

extern Uint32 map_flags;/**< The map flags - basically this will be obsolete with the next map format, but untill then it's good to have*/

extern char dungeon; /**< inidicates whether we are in a dungeon (no sun) or not */

/** @name ambient color values
 * @{ */
extern float ambient_r;
extern float ambient_g;
extern float ambient_b;
/** @} */

extern int map_type; /**< id of the type of map we are currently using */
extern GLfloat* water_tile_buffer;
extern GLfloat* terrain_tile_buffer;
extern hash_table *server_marks; /**< table of map marks send from the server, save between sessions */
extern int marks_3d; /**< config file flag to enable or disbale display of 3d mark marks */
extern int filter_marks_3d; /**< apply the current mark filter to 3d map marks */

/*! Structure for server marks */
typedef struct _s_mark
{
	int id;
	int x,y;
	char map_name[50];
	char text[100];
} server_mark;


/**
 * @ingroup maps
 * @brief Changes the current map
 *
 * 	Loads the given map, destroys sound objects etc.
 *
 * @param mapname The name of the map
 */
void change_map (const char * mapname);

/**
 * @ingroup maps
 * @brief Loads the map marks for the given mapname into the given buffer
 *
 * 	Loads the map marks for the given \a mapname into the given \a buffer
 *
 * @param mapname filename of the map
 * @param buffer buffer for the map marks
 * @param max maximum number of map marks in \a buffer
 */
void load_marks_to_buffer(char* mapname, marking* buffer, int* max);

/**
 * @ingroup maps
 * @brief Loads the map marks for the current map
 *
 * 	Loads the map marks for the current map
 *
 */
void load_map_marks(void);

/*!
 * \ingroup maps
 * \brief   Saves the user defined markings on maps.
 *
 *      Saves the user defined markings on maps. The markings are stored on a per map basis, i.e. each map gets its own save file, based on the maps .elm filename.
 *
 */
void save_markings(void);

/**
 * @ingroup maps
 * @brief Adds a number of 3d objects to the map
 *
 *	Adds \a nr_objs 3d objects from the server to the current map
 *
 * @param nr_objs The number of objects to add
 * @param data The message from the server with the object list
 * @param len The length of \a data
 * @retval int  0 on error, 1 on success
 * @callgraph
 */
int get_3d_objects_from_server (int nr_objs, const Uint8 *data, int len);

/**
 * @ingroup maps
 * @brief Removes an object from the current map
 *
 *	Removes the object with ID an id from the map
 *
 * @param id The ID of the object to be removed
 * @callgraph
 */
void remove_3d_object_from_server (int id);

/**
 * @ingroup maps
 * @brief Inits the buffer used for terrain.
 *
 * Inits the buffer used for terrain. Must be called every time map
 * size increase, but also should be called every time time map size decrease.
 *
 * @param terrain_buffer_size The new size of the buffer in number of elements.
 * @callgraph
 */
void init_terrain_buffers(int terrain_buffer_size);

/**
 * @ingroup maps
 * @brief Inits the buffer and the portals.
 *
 * Inits the buffer used for terrain and water, also the portals. Must be called every time map
 * size increase, but also should be called every time time map size decrease.
 *
 * @callgraph
 */
void init_buffers(void);

/**
 * @ingroup maps
 * @brief Frees the buffer and the portals.
 *
 * Frees the buffer used for terrain and water, also the portals.
 * Should only be called on client exit.
 *
 * @callgraph
 */
void free_buffers(void);

/**
 * @ingroup maps
 * @brief Removed the current map freeing resources.
 *
 * @callgraph
 */
void destroy_map(void);

/**
 * @ingroup maps
 * @brief Removes any existing severmarks and creates a new empty table.
 *
 * @callgraph
 */
void init_server_markers(void);

/**
 * @ingroup maps
 * @brief Opens the server marks file and loads any marks into the table.
 *
 * @callgraph
 */
void load_server_markings(void);

/**
 * @ingroup maps
 * @brief Saves any current server marks to the server marks file.
 *
 * @callgraph
 */
void save_server_markings(void);

/**
 * @ingroup maps
 * @brief Animate and draw the 3d map marks.
 *
 * @callgraph
 */
void draw_3d_marks(void);

/*
 * Returns the index of the map file name in the continent_maps array.
 * This is used in map.c to save the index of the current map in cur_map.
 * It is here temporarily because if the map sound config files use the map id rather than
 * the map name from the server, sound.c needs to look up the index of the current map.
 * Once map sound config files use server map name rather than id, we can remove this and
 * references to get_cur_map in sound.c.
 */
int get_cur_map (const char * file_name);


#ifdef __cplusplus
} // extern "C"
#endif

#endif	// _MAP_H_
