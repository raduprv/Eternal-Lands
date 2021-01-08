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

extern const char * ini_filename;
extern float water_tiles_extension;
extern int show_game_seconds;
extern int skybox_update_delay;
extern int skybox_local_weather;
#ifdef NEW_CURSOR
extern int big_cursors;
extern int sdl_cursors;
extern float pointer_size;
#endif // NEW_CURSOR
extern Uint32 max_actor_texture_handles;

extern int write_ini_on_exit; /*< variable that determines if the ini file is rewritten on exit of the program */

extern int video_mode_set;
extern int no_adjust_shadows;
extern int clouds_shadows; /*!< flag that indicates whether the shadows of clouds should be displayed or not */
extern int item_window_on_drop;
extern int mouse_limit;
extern int isometric; /*!< use isometric instead of perspective view */
extern int poor_man; /*!< this flag, if set to true, indicates we are running on a really poor machine */
extern int limit_fps; /*!< the configured max FPS number we should use. If this is 0, the highest possible number will be used. */
extern int max_fps; /*!< the current max fps to use, normally the same as limit_fps, set low when window is not active to reduce processing */
extern int special_effects; /*!< flag indicating whether pretty spell effects should be enabled */
extern int show_reflection; /*!< flag that indicates whether to display reflections or not */
extern char lang[10]; /*!< contains the identifier for the current language. \todo Shouldn't this go into translate.h? */
extern int auto_update; /*!<this flags signals whether or not autoupdates are performed at startup, or not. It requires a restart to have an effect. */
extern int buddy_log_notice; /*!< whether to log buddy logged on/off notices to screen */
extern int clear_mod_keys_on_focus; /*!< trouble shooting option to force mod keys up when gaining focus */

#if !defined(WINDOWS) && !defined(OSX)
extern int use_clipboard; /*!< whether to use CLIPBOARD or PRIMARY for pasting */
#endif

#ifdef  CUSTOM_UPDATE
extern int custom_update; /*!<this flags signals whether or not autoupdates of custom looks is permitted. */
extern int custom_clothing; /*!<this flags signals whether or not custom is displayed. */
#endif  //CUSTOM_UPDATE

#ifdef OSX
extern int square_buttons; /* flag to overcome intel opengl issues on early MacBooks*/
#endif

#ifdef DEBUG
extern int render_skeleton;
extern int render_mesh;
extern int render_bones_id;
extern int render_bones_orientation;
#endif

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
	INI_FILE_VAR,		/*!< for variables names from the ini file */
	IN_GAME_VAR		/*!< for names of variables changed in the games */
} var_name_type;

int get_rotate_chat_log(void);

void change_language(const char *new_lang);

void check_for_config_window_scale(void);

/*!
 * \ingroup config
 * \brief   change the custom scale value for the window by the step value
 *
 * \param increase                          if true, the value is increase by the step, otherwise it is decreased by the step
 * \param changed_window_custom_scale       pointer to the custom scale variable for the window
 *
 * \callgraph
*/
void step_win_scale_factor(int increase, float *changed_window_custom_scale);

/*!
 * \ingroup config
 * \brief   limit the custom scale value for the window to the default
 *
 * \param changed_window_custom_scale       pointer to the custom scale variable for the window
 *
 * \callgraph
*/
void limit_win_scale_to_default(float *changed_window_custom_scale);

/*!
 * \ingroup config
 * \brief   set the custom scale value for the window to the default or initial starting value
 *
 * \param set_default                       if true, the value is set to the default, otherwise it is set to the initial starting value
 * \param changed_window_custom_scale       pointer to the custom scale variable for the window
 *
 * \callgraph
*/
void reset_win_scale_factor(int set_default, float *changed_window_custom_scale);

void update_highdpi_auto_scaling(void);

float get_global_scale(void);

#ifdef JSON_FILES
void set_ready_for_user_files(void);
int get_use_json_user_files(void);
void load_character_options(void);
void save_character_options(void);
#endif

/*!
 * \ingroup config
 * \brief   returns the long description of the variable with the given \a str name and the given \a type.
 *
 * \param str       the name of the variable to check
 * \param type      the type of the variable name
 * \retval str       the long description or NULL
 *
 * \callgraph
*/
const char *get_option_description(const char *str, var_name_type type);

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
void init_vars(void);

/*!
 * \ingroup other
 * \brief   frees the global \see our_vars variable.
 *
 *      Frees up the memory used by the global \see our_vars variable.
 *
 * \sa start_rendering
 */
void free_vars(void);

/*!
 * \ingroup config
 * \brief   Reads the ini configuration file
 *
 *     Reads the ini configuration file
 *
 * \retval int      0 if reading fails, 1 if successful
 *
 */
int read_el_ini(void);

/*!
 * \ingroup config
 * \brief   Writes the ini configuration file
 *
 *     Writes the current configuration to the ini file
 *
 * \retval int      0 if writing fails, 1 if successful
 *
 */
int write_el_ini(void);

/*!
 * \ingroup other
 * \brief   Checkes the option-vars.
 *
 *      Checks the global option vars ( \see our_vars variable).
 *
 * \callgraph
 */
void check_options(void);

/*!
 * \ingroup other
 * \brief   Toggles the root window of some windows.
 *
 *      Toggles the root window of the buddy, manu, storage, bag, and inv windows to enable them to "float" above the console and map.
 *
 * \callgraph
 */
void change_windows_on_top(int *var);

#ifndef MAP_EDITOR
/*!
 * \ingroup config
 * \brief   Adds another option to a multi-var.
 *
 * Adds an option with identifier \a id and label \a str to the multi-var
 * selection list for variable \a name. If the parameter \a add_button is
 * non-zero, and the widget for the variable exsists, a button will also be
 * added to this widget.
 *
 * \param name       the name of the variable to add to
 * \param str        the text for the option
 * \param id         an optional key for the option
 * \param add_button if non-zero, add a button to the widget for the option
 */
void add_multi_option_with_id(const char* name, const char* str, const char* id, int add_button);
static __inline__ void add_multi_option(const char* name, const char* str)
{
	add_multi_option_with_id(name, str, NULL, 0);
}
/*!
 * \ingroup config
 *
 * Clear a multi-var.
 *
 * Remove all options from the multi-select variable with name \a name.
 *
 * \param name the name of the variable to clear
 */
void clear_multiselect_var(const char* name);
/*!
 * \ingroup config
 *
 * Set a multi-var.
 *
 * Set the selected option in multi-select variable \a name to \a idx. If the
 * parameter \a change_button is non-zero, the corresponding button in the
 * widget is also selected.
 *
 * \param name          the name of the variable to set
 * \param idx           the index of the element to select
 * \param change_button if non-zero, select GUI button as well
 */
void set_multiselect_var(const char* name, int idx, int change_button);
#endif // !MAP_EDITOR

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
 * \param new_value well, the new value
 * \retval	1 if sucessfull, 0 if option not found
 */
int set_var_OPT_INT(const char *str, int new_value);

/*!
 * \ingroup config
 * brief Restore the video mode to that when the client was started.
 */
void restore_starting_video_mode(void);

/*!
 * \ingroup config
 * brief Save the current client window size as user defined mode.
 */
void set_user_defined_video_mode(void);

#endif

void toggle_follow_cam(int * fc);
void toggle_ext_cam(int * ec);
void options_loaded(void);


/*!
 * \ingroup other
 * Set previously stored multi-select variables.
 *
 * Some multi-select variables cannot be reliably set because they are not fully
 * initialized before the ini file is read. The values for these variables are stored,
 * and the variables are set to the correct option afterwards using this function.
 * The initialization is done as follows:
 * 1. if only an index is stored, and it is a valid index, that is used.
 * 2. if both an index and a value are stored, the value overrides the index,
 *    and the option with correct value is selected if it can be found.
 * 3. if no valid value is found, or the value is empty and the index is invalid,
 *    the option is left unchanged, and nothing is done.
 * This function assumes all necessary initialization is done when it is called,
 * and therefore deletes all deferred options.
 */
void check_deferred_options();

#ifdef __cplusplus
} // extern "C"
#endif

#endif
