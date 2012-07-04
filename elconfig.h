/*!
 * \file
 * \ingroup config
 * \brief config file related functions.
 */
#ifndef __ELCONFIG_H__
#define __ELCONFIG_H__

#include "queue.h"
#include "translate.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int elconfig_win;
extern int elconfig_menu_x;
extern int elconfig_menu_y;
extern float water_tiles_extension;
extern int show_game_seconds;
extern int skybox_update_delay;
extern int skybox_local_weather;
#ifdef NEW_CURSOR
extern int big_cursors;
extern int sdl_cursors;
extern float pointer_size;
#endif // NEW_CURSOR
#ifdef	NEW_TEXTURES
extern Uint32 max_actor_texture_handles;
#endif	/* NEW_TEXTURES */

extern int write_ini_on_exit; /*< variable that determines if el.ini file is rewritten on exit of the program */

extern int gx_adjust;
extern int gy_adjust;

/*!
 * The different kinds of options
 */
typedef enum
{
	OPT_BOOL = 1,      // Change variable                   func(int*)
	OPT_STRING,        // Change string                     func(char*,char*)
	OPT_FLOAT,         // Change float                      func(float*,float*)
	OPT_INT,           // Change int                        func(int*,int)
	OPT_SPECINT = OPT_INT, // Multiple ints, non-default func   func(int*,int)
	OPT_MULTI,         // INT with multiselect widget
	OPT_MULTI_H,       // INT with multiselect widget, horizontal
	OPT_PASSWORD,
	OPT_FLOAT_F,       // Change float with functions that returns max and min values  func(float*,float*), max/min float func()
	OPT_INT_F,         // Change int with functions that returns max and min values    func(int*,int), max/min int func()
	OPT_BOOL_INI,      // Boolean value that is only read from and written to the ini file
	OPT_INT_INI	   // Int value that is only read from the ini file
} option_type;

/*!
 * The type of variable name.
 */
typedef enum
{
	COMMAND_LINE_SHORT_VAR,	/*!< for abbreviated variable names from the command line */
	COMMAND_LINE_LONG_VAR,	/*!< for full variable names from the command line */
	INI_FILE_VAR,		/*!< for variables names from el.ini */
	IN_GAME_VAR		/*!< for names of variables changed in the games */
} var_name_type;

void display_elconfig_win(void);

int get_rotate_chat_log(void);

void change_language(const char *new_lang);

/*!
 * \ingroup config
 * \brief   checks whether we have a variable with the given \a str as name and the given \a type.
 *
 *      Checks whether we have a variable with the given \a str as name and the given \a type.
 *
 * \param str       the name of the variable to check
 * \param type      the type of the variable name
 * \retval int      0 if \a str is found, else !=0.
 *
 * \sa read_command_line
 * \sa read_config
 */
int check_var(char * str, var_name_type type);

/*!
 * \ingroup other
 * \brief   initializes the global \see our_vars variable.
 *
 *      Initializes the global \see our_vars variable.
 *
 * \callgraph
 */
void init_vars();

/*!
 * \ingroup other
 * \brief   frees the global \see our_vars variable.
 *
 *      Frees up the memory used by the global \see our_vars variable.
 *
 * \sa start_rendering
 */
void free_vars();

/*!
 * \ingroup config
 * \brief   Reads the el.ini configuration file
 *
 *     Reads the el.ini configuration file
 *
 * \retval int      0 if reading fails, 1 if successful
 *
 */
int read_el_ini ();

/*!
 * \ingroup config
 * \brief   Writes the el.ini configuration file
 *
 *     Writes the current configuration to the el.ini file
 *
 * \retval int      0 if writing fails, 1 if successful
 *
 */
int write_el_ini ();

/*!
 * \ingroup other
 * \brief   Checkes the option-vars.
 *
 *      Checks the global option vars ( \see our_vars variable).
 *
 * \callgraph
 */
void check_options();

/*!
 * \ingroup other
 * \brief   Toggles the root window of some windows.
 *
 *      Toggles the root window of the buddy, manu, storage, bag, and inv windows to enable them to "float" above the console and map.
 *
 * \callgraph
 */
void change_windows_on_top(int *var);

/*!
 * \ingroup other
 * \brief   Adds another option to a multi-var.
 *
 *      Adds another option to a multi-var selection list.
 *
 * \param name       the name of the variable to add to
 * \param str      the text for the option
 */
void add_multi_option(char * name, char * str);

void change_windowed_chat (int *wc, int val);

/*!
 * \ingroup other
 * brief Sets the specfied variable (if valid) to unsaved.
 * \param str	the option name
 * \param type	the option type
 * \retval	1 if sucessfull, 0 if option does not exist
 */
int set_var_unsaved(const char *str, var_name_type type);

/*!
 * \ingroup other
 * brief Toggle the specfied OPT_BOOL variable (if valid) and save.
 * \param str	the option name
 * \retval	1 if sucessfull, 0 if name is invalid
 */
int toggle_OPT_BOOL_by_name(const char *str);

#ifdef	ELC
/*!
 * \ingroup other
 * brief Sets the specfied OPT_INT variable's value.
 * \param str	the option name
 * \param new_vale well, the new value
 * \retval	1 if sucessfull, 0 if option not found
 */
int set_var_OPT_INT(const char *str, int new_value);
#endif

void toggle_follow_cam(int * fc);
void toggle_ext_cam(int * ec);
void options_loaded(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
