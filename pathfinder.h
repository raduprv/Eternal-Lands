/*!
 * \file
 * \ingroup move_actors
 * \brief client side implementation of the pathfinding algorithm
 */
#ifndef __PATHFINDER_H__
#define __PATHFINDER_H__

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
	Uint16 x;
	Uint16 y;
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
 * \brief returns the corresponding PF_TILE struct for the given position
 *
 *      Returns the corresponding PF_TILE structure for the given position
 *
 * \param x          x coordinate of the position
 * \param y          y coordinate of the position
 * \retval PF_TILE*  the corresponding pathfinder tile for the given (x,y) position
 */
PF_TILE *pf_get_tile(int x, int y);

/*!
 * \ingroup move_actors
 * \brief returns the next open tile for the current path.
 *
 *      Returns the next open tile for the current path
 *
 * \retval PF_TILE* a pointer to a \see PF_TILE struct containing the data of the next open tile.
 */
PF_TILE *pf_get_next_open_tile();

/*!
 * \ingroup move_actors
 * \brief adds the current tile to the PF_OPEN_TILE structure.
 *
 *      Adds the current tile to the PF_OPEN_TILE structure
 *
 * \param current       the current PF_TILE
 * \param neighbour
 */
void pf_add_tile_to_open_list(PF_TILE *current, PF_TILE *neighbour);

/*!
 * \ingroup move_actors
 * \brief finds a path to the given position
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
 * \brief clears the current path and frees up the memory used
 *
 *      Clears the current path and frees up the memory used
 *
 */
void pf_destroy_path();

/*!
 * \ingroup move_actors
 * \brief returns the current actor
 *
 *      Returns a pointer to the current actor.
 *
 * \retval actor*   a pointer to the actor
 */
actor *pf_get_our_actor();

/*!
 * \ingroup move_actors
 * \brief moves the actor along the calculated path
 *
 *      Moves the actor along the calculated path
 *
 * \callgraph
 */
void pf_move();

/*!
 * \ingroup move_actors
 * \brief checks whether the tile at the given position is currently occupied by someone or something else.
 *
 *      Checks whether the tile at the given position is currently occupied by someone or something else.
 *
 * \param x     x coordinate of the position to check
 * \param y     y coordinate of the position to check
 * \retval int  0 if the tile at (x,y) isn't occupied, else != 0
 */
int pf_is_tile_occupied(int x, int y);

/*!
 * \ingroup move_actors
 * \brief a callback function used to sync the actors movement with the server
 *
 *      A callback function used to sync the actors movement with the server
 *
 * \param interval
 * \param param
 * \retval Uint32
 */
Uint32 pf_movement_timer_callback(Uint32 interval, void *param);

/*!
 * \ingroup move_actors
 * \brief moves the actor to the mouse position where the last click occurred
 *
 *      Moves the actor to the mouse position where the last click occurred
 *
 * \callgraph
 */
void pf_move_to_mouse_position();

#endif /* __PATHFINDER_H__ */
