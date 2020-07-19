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

extern int wanted_num_recipe_entries; /*!< option window set number of recipe entries */
extern int disable_manuwin_keypress; /*!< option to disable key presses in the main manu window */
extern const int max_num_recipe_entries;  /*!< max number of recipe entries */

#define NUM_MIX_SLOTS 6

// we don't need the full item structure so
// use a seperate subset to avoid misunderstanding
// we can then safely save/load just the bits we need
typedef struct
{
	Uint16 id;
	int image_id;
	int quantity;
} recipe_item;

typedef struct
{
	recipe_item items[NUM_MIX_SLOTS];
	char *name;
	int status;
} recipe_entry;

/*!
 * \ingroup manufacture_window
 * \brief Sets up the manufacture list
 *
 * Initializes the manufacture list used when an actor is manufacturing items with
 * the \ref manufacture_win window.
 */
void build_manufacture_list();

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

/*!
 * \ingroup manufacture_window
 * \brief Called when the number of recipe entries is changed in the options window.
 *
 * \callgraph
 */
 void change_num_recipe_entries(int * var, int value);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
