/*!
 * \file
 * \ingroup item
 * \brief Mine handling
 */

#ifndef __MINES_H__
#define __MINES_H__

#include <SDL_types.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define NUM_MINES 200

typedef struct
{
	int x;
	int y;
	int type;
	int obj_3d_id;
} mine;

extern mine mine_list[NUM_MINES];

/*!
 * \ingroup item
 * \brief   Puts the mine \a mine_id on the ground at coordinates (\a mine_x, \a mine_y).
 *
 *      Puts the mine \a mine_id at the coordinates (\a mine_x, \a mine_y) on the ground.
 *
 * \param mine_x     x coordinate of the mines position
 * \param mine_y     y coordinate of the mines position
 * \param mine_type  type of mine being placed
 * \param mine_id    index into \ref mine_list to be used for the mine
 *
 * \callgraph
 */
void put_mine_on_ground(int mine_x, int mine_y, int mine_type, int mine_id);

/*!
 * \ingroup item
 * \brief   Adds the mines given in \a data.
 *
 *      Adds the mines that are given in \a data.
 *
 * \param data  the data from the server for the mines to add
 *
 * \callgraph
 *
 * \note No sanity checks on \a data are performed. This may be a possible bug.
 * \bug No sanity checks on \a data are performed.
 */
void add_mines_from_list (const Uint8 * data);

/*!
 * \ingroup item
 * \brief   Removes the mine with the given index \a which_mine from the \ref mine_list.
 *
 *      Removes the mine at the given index \a which_mine from the \ref mine_list with the "safe" removal effect. The list of mines will be adjusted accordingly.
 *
 * \param which_mine the index into \ref mine_list for the mine to remove
 *
 * \callgraph
 */
void remove_mine(int object_id);

/*!
 * \ingroup item
 * \brief   Removes all the mines from a map.
 *
 *      Removes all the mines on a map.
 *
 * \callgraph
 */
void remove_all_mines();

/*!
 * \ingroup item
 * \brief   Loads the mines configuration data.
 *
 *      Loads the mines configuration data.
 *
 * \callgraph
 */
void load_mines_config();

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // __MINES_H__

