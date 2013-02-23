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

/**
 * @ingroup maps
 * @brief Loads an empty map
 *
 * 	Loads an empty map in case no other map file was found
 *
 * @param name the filename of the map that failed to load.
 * @retval int  0 if nomap.elm failed to load, otherwise 1 is returned.
 */
int load_empty_map(void);

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

void destroy_map();

typedef struct _s_mark{

	int id;
	int x,y;
	char map_name[50];
	char text[100];

} server_mark;


void init_server_markers(void);
void load_server_markings(void);
void save_server_markings(void);
void animate_map_markers(void);
void add_server_markers(void);
void display_map_markers(void); //draw text
void display_map_marks(void); //draw cross
void change_3d_marks(int *rel);
extern hash_table *server_marks;
extern float mark_z_rot;
extern int marks_3d;
#define MARK_CLIP_POS 20
#define MARK_DIST 20




#ifdef __cplusplus
} // extern "C"
#endif

#endif	// _MAP_H_
