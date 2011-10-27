/*!
 * \file
 * \ingroup move_actors
 * \brief client side implementation of the pathfinding algorithm
 */
#ifndef __PATHFINDER_H__
#define __PATHFINDER_H__

#include <SDL_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \name Pathfinder limitting
 * @{
 *      This limits the number of attempts the pathfinder will make before giving up.
 */
#define	MAX_PATHFINDER_ATTEMPTS 200000
/*! @} */

/*!
 * \name Pathfinder states
 * @{
 *      This enumeration declares the different states the pathfinder can be in.
 */
enum {
	PF_STATE_NONE=0,
	PF_STATE_OPEN,
	PF_STATE_CLOSED
};
/*! @} */

/*!
 * a structure to store the data of tiles related to pathfinding
 */
typedef struct
{
	Uint32 open_pos;
	Sint32 x;
	Sint32 y;
	Uint16 f;
	Uint16 g;

	Uint8 state; /*!< the current state pathfinder states */
	Uint8 z;

	void *parent;
} PF_TILE;

/*!
 * this list stores information about open (non-blocked) paths
 */
typedef struct
{
	PF_TILE **tiles; /*!< an array of \see PF_TILE structures */
	int count; /*!< number of elements in tiles */
} PF_OPEN_LIST;

extern PF_TILE *pf_tile_map; /*!< a list of \see PF_TILE structures that form the path */
extern PF_TILE *pf_dst_tile; /*!< the \see PF_TILE struct that defines our destination tile of the path */
extern int pf_follow_path; /*!< flag, that indicates whether we should follow the path or not */

/*!
 * \ingroup move_actors
 * \brief Finds a path to the given position
 *
 *      Finds a path from the current position to the given target position (x,y).
 *
 * \param x     x coordinate of the target position
 * \param y     y coordinate of the target position
 * \retval int
 * \callgraph
 */
int pf_find_path(int x, int y);

/*!
 * \ingroup move_actors
 * \brief Clears the current path and frees up the memory used
 *
 *      Clears the current path and frees up the memory used
 *
 */
void pf_destroy_path();

/*!
 * \ingroup move_actors
 * \brief Moves the actor along the calculated path
 *
 *      Moves the actor along the calculated path
 *
 * \callgraph
 */
void pf_move();

/*!
 * \ingroup move_actors
 * \brief Moves the actor to the mouse position where the last click occurred
 *
 *      Moves the actor to the mouse position where the last click occurred
 *
 * \callgraph
 */
void pf_move_to_mouse_position();

/*!
 * \ingroup move_actors
 * \brief Calculates the tile coordinates of the given mouse position
 *
 *      Calculates the tile coordinates of the given mouse position.
 *      If the return value is zero, *px and *py are undefined.
 *
 * \param mouse_x  x coordinate of mouse
 * \param mouse_y  y coordinate of mouse
 * \param px       return address for x tile coordinate
 * \param py       return address for y tile coordinate
 * \retval int     1 if the mouse over the map, 0 otherwise.
 *
 * \callgraph
 */
int pf_get_mouse_position(int mouse_x, int mouse_y, int * px, int * py);

/*!
 * \ingroup move_actors
 * \brief Calculates the tile coordinates of the given mouse position
 *
 *      Calculates the tile coordinates of the given mouse position
 *      for variable size map. If the return value is zero, *px and *py are undefined.
 *
 * \param mouse_x  x coordinate of mouse
 * \param mouse_y  y coordinate of mouse
 * \param px       return address for x tile coordinate
 * \param py       return address for y tile coordinate
 * \param tile_x   the map z size
 * \param tile_y   the map y size
 * \retval int     1 if the mouse over the map, 0 otherwise.
 *
 * \callgraph
 */
int pf_get_mouse_position_extended(int mouse_x, int mouse_y, int * px, int * py, int tile_x, int tile_y);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __PATHFINDER_H__ */
