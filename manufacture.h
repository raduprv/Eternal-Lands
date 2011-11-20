/*!
 * \file
 * \ingroup manufacture_window
 * \brief display of the manufacture window
 */
#ifndef __MANUFACTURE_H__
#define __MANUFACTURE_H__

#include "items.h"

#ifdef __cplusplus
extern "C" {
#endif

extern item manufacture_list[ITEM_NUM_ITEMS]; /*!< global list of manufacturable items */

extern int manufacture_menu_x;
extern int manufacture_menu_y;

/*!
 * \name windows handlers
 */
/*! @{ */
extern int manufacture_win; /*!< manufacture windows handler */
/*! @} */

/*!
 * \ingroup manufacture_window
 * \brief Sets up the \ref manufacture_list.
 *
 *      Initializes the \ref manufacture_list used when an actor is manufacturing items with the \ref manufacture_win window.
 *
 */
void build_manufacture_list();

/*!
 * \ingroup manufacture_window
 * \brief Displays the manufacture window.
 *
 *      Displays the \ref manufacture_win window. If the window was not shown before it will first initialized.
 *
 * \callgraph
 */
void display_manufacture_menu();

/*!
 * \ingroup manufacture_window
 * \brief Manufacture items.
 *
 *      Sends the MANUFACTURE_THIS message to the server.
 *
 * \param quantity	the number of items to manufacture, max 255
 * \param mixbut_empty_str message for user if no items to mix
 */
int mix_handler(Uint8 quantity, const char* mixbut_empty_str);

/*!
 * \ingroup manufacture_window
 * \brief Load manufacture recipes.
 *
 * \callgraph
 */
void load_recipes();

/*!
 * \ingroup manufacture_window
 * \brief Load manufacture recipes.
 *
 * \callgraph
 */
void save_recipes();

/*!
 * \ingroup manufacture_window
 * \brief Assign name to unnamed recipe matching the last mixed ingredients.
 *
 * \param name to assign to a matching recipe
 * \callgraph
 */
void check_for_recipe_name(const char *name);


/*!
 * \ingroup manufacture_window
 * \brief Called on exit - free memory and clean up.
 *
 * \callgraph
 */
void cleanup_manufacture(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
