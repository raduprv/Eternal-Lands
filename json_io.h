#ifndef __JSON_IO_H
#define __JSON_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "chat.h"
#include "counters.h"
#include "manufacture.h"

#define USE_JSON_DEBUG(message) {}
//#define USE_JSON_DEBUG(message) {printf("%s:%d %s\n", __FUNCTION__, __LINE__, message);}

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


/*!
 * \name Functions to load and save channel colours.
 */
/*! @{ */
int json_load_channel_colours(const char *file_name, channelcolor *channel_colours, size_t max_channel_colours);
int json_save_channel_colours(const char *file_name, const channelcolor *channel_colours, size_t max_channel_colours);
/*! @} */

/*!
 * \name Functions for character options.
 */
/*! @{ */
int json_character_options_set_file_name(const char *file_name);
int json_character_options_load_file(void);
int json_character_options_save_file(void);
int json_character_options_get_int(const char *var_name, int default_value);
void json_character_options_set_int(const char *var_name, int value);
float json_character_options_get_float(const char *var_name, float default_value);
void json_character_options_set_float(const char *var_name, float value);
int json_character_options_get_bool(const char *var_name, int default_value);
void json_character_options_set_bool(const char *var_name, int value);
int json_character_options_exists(const char *var_name);
void json_character_options_remove(const char *var_name);
/*! @} */

/*!
 * \name Functions for Client State
 */
/*! @{ */
int json_load_cstate(const char *file_name);
int json_save_cstate(const char *file_name);
int json_cstate_get_int(const char *section_name, const char *var_name, int default_value);
void json_cstate_set_int(const char *section_name, const char *var_name, int value);
unsigned int json_cstate_get_unsigned_int(const char *section_name, const char *var_name, unsigned int default_value);
void json_cstate_set_unsigned_int(const char *section_name, const char *var_name, unsigned int value);
float json_cstate_get_float(const char *section_name, const char *var_name, float default_value);
void json_cstate_set_float(const char *section_name, const char *var_name, float value);
int json_cstate_get_bool(const char *section_name, const char *var_name, int default_value);
void json_cstate_set_bool(const char *section_name, const char *var_name, int value);
/*! @} */

#ifdef __cplusplus
} // extern "C"
#endif

#endif
