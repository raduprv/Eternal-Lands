/*!
 * \file
 * \ingroup display_2d
 * \brief cursor related data types and functions
 */
#ifndef __CURSORS_H__
#define __CURSORS_H__

#include "actors.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \name Cursor types
 */
/*! @{ */
#define CURSOR_EYE 0
#define CURSOR_TALK 1
#define CURSOR_ATTACK 2
#define CURSOR_ENTER 3
#define CURSOR_PICK 4
#define CURSOR_HARVEST 5
#define CURSOR_WALK 6
#define CURSOR_ARROW 7
#define CURSOR_TRADE 8
#define CURSOR_USE_WITEM 9
#define CURSOR_USE 10
#define CURSOR_WAND 11
#define CURSOR_TEXT 12
/*! @} */

/*!
 * \name Type under the cursor
 */
/*! @{ */
#define UNDER_MOUSE_NPC 0
#define UNDER_MOUSE_PLAYER 1
#define UNDER_MOUSE_ANIMAL 2
#define UNDER_MOUSE_3D_OBJ 3
#define UNDER_MOUSE_NOTHING 4
#define UNDER_MOUSE_NO_CHANGE 5
#ifdef MAP_EDITOR2
 #define UNDER_MOUSE_2D_OBJ 6
 #define UNDER_MOUSE_PARTICLE 7
 #define UNDER_MOUSE_LIGHT 8
#endif
/*! @} */

extern actor *actor_under_mouse;
extern int object_under_mouse;
extern int thing_under_the_mouse;
extern int current_cursor;
extern int elwin_mouse;

/*!
 * A cursors_struct contains a Hot Spot and a pointer to the actual cursor.
 */
struct cursors_struct
{
	int hot_x; /*!< x coordinate of the hot spot point. */
	int hot_y; /*!< y coordinate of the hot spot point. */
	Uint8 *cursor_pointer; /*!< pointer to the actual cursor */
};

#ifndef FASTER_MAP_LOAD
/*!
 * contains the names of harvestable items
 */
extern char harvestable_objects[300][80];

/*!
 * contains the name of entrable items
 */
extern char entrable_objects[300][80];
#endif

#ifdef FASTER_MAP_LOAD
/*!
 * \ingroup other
 * \brief Load and initialize the list of harvestable objects
 */
void load_harvestable_list(void);
/*!
 * \ingroup other
 * \brief Test if an object can be harvested
 *
 * Check if the 3d object with file name \a fname should in principle be
 * harvestable. Ultimately, the server decides if an object can be harvested,
 * this function exists to show the correct cursor in the GUI.
 * \return Zero if the object is not in the harvestable list, non-zero
 *    otherwise.
 */
int is_harvestable(const char* fname);

/*!
 * \ingroup other
 * \brief Load and initialize the list of entrable objects
 */
void load_entrable_list(void);
/*!
 * \ingroup other
 * \brief Test if an object can be entered
 *
 * Check if the 3d object with file name \a fname should in principle be
 * entrable. Ultimately, the server decides if an object can be entered,
 * this function exists to show the correct cursor in the GUI.
 * \return Zero if the object is not in the entrable list, non-zero
 *    otherwise.
 */
int is_entrable(const char* fname);
#endif // FASTER_MAP_LOAD

/*!
 * \ingroup other
 * \brief loads and initializes the \see cursors_array global variable
 *
 *      Loads and initializes the \see cursors_array global variable
 *
 */
void load_cursors();

/*!
 * \ingroup display_2d
 * \brief changes the current cursor to the cursor given in \a cursor_id.
 *
 *      Changes the current cursor to the cursor given in \a cursor_id.
 *
 * \param cursor_id     the cursor to switch to
 */
void change_cursor(int cursor_id);

/*!
 * \ingroup other
 * \brief builds all the available cursors and stores them in \see cursors_array.
 *
 *      Builds and initializes all available cursors and stores them in the \see cursors_array.
 *
 * \callgraph
 */
void build_cursors();

/*!
 * \ingroup display_2d
 * \brief checks if the cursor has changed and we need to update displays.
 *
 *      Checks whether the cursor has changed and we need to update displays.
 *
 * \callgraph
 */
void check_cursor_change();

void cursors_cleanup(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
