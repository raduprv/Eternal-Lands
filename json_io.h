#ifndef __JSON_IO_H
#define __JSON_IO_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \name Functions to load and save the manufacture window recipes.
 */
/*! @{ */
int json_open_recipes(const char *file_name);
int json_load_recipes(recipe_entry *recipes_store, size_t max_recipes);
int json_save_recipes(const char *file_name, recipe_entry *recipes_store, size_t num_recipes, int current_recipe);
/*! @} */


#ifdef __cplusplus
} // extern "C"
#endif

#endif
