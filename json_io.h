#ifndef __JSON_IO_H
#define __JSON_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "counters.h"
#include "manufacture.h"

/*!
 * \name Functions to load and save the manufacture window recipes.
 */
/*! @{ */
int json_open_recipes(const char *file_name);
int json_load_recipes(recipe_entry *recipes_store, size_t max_recipes);
int json_save_recipes(const char *file_name, recipe_entry *recipes_store, size_t num_recipes, int current_recipe);
/*! @} */


/*!
 * \name Functions to load and save the quickspells list.
 */
/*! @{ */
int json_load_quickspells(const char *file_name, int *spell_ids, size_t max_num_spell_id);
int json_save_quickspells(const char *file_name, Uint16 *spell_ids, size_t num_spell_id);
/*! @} */


/*!
 * \name Functions to load and save the counters.
 */
/*! @{ */
int json_load_counters(const char *file_name, const char **cat_str, int *entries, size_t num_categories, struct Counter **the_counters);
int json_save_counters(const char *file_name, const char **cat_str, const int *entries, size_t num_categories, const struct Counter **the_counters);
/*! @} */


#ifdef __cplusplus
} // extern "C"
#endif

#endif
