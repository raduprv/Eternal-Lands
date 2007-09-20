/*!
 * \file
 * \ingroup config
 * \brief config file related functions.
 */
#ifndef __ELCONFIG_H__
#define __ELCONFIG_H__

#include "queue.h"
#ifdef OPTIONS_I18N
#include "translate.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int elconfig_win;
extern int elconfig_menu_x;
extern int elconfig_menu_y;
#ifdef SKY_FPV_CURSOR
extern float z_cull_sq, cut_size;
extern int big_cursors;
extern int sdl_cursors;
extern float pointer_size;
#endif /* SKY_FPV_CURSOR */

/*!
 * The different kinds of options
 */
typedef enum
{
	OPT_BOOL = 1,      // Change variable                   func(int*)
	OPT_STRING,	       // Change string                     func(char*,char*)
	OPT_FLOAT,         // Change float                      func(float*,float*)
	OPT_INT,           // Change int                        func(int*,int)
	OPT_SPECINT = OPT_INT, // Multiple ints, non-default func   func(int*,int)
	OPT_MULTI,         // INT with multiselect widget
	OPT_PASSWORD,
	OPT_FLOAT_F,       // Change float with functions that returns max and min values  func(float*,float*), max/min float func()
	OPT_INT_F,         // Change int with functions that returns max and min values    func(int*,int), max/min int func()
	OPT_BOOL_INI       // Boolean value that is only read from and written to the ini file
} option_type;

/*!
 * var_struct stores the data for a single configuration entry.
 */
typedef struct
{
	option_type type; /*!< type of the variable */
	char	*name; /*!< name of the variable */
	int 	nlen; /*!< length of the \a name */
	char 	*shortname; /*!< shortname of the variable */
	int 	snlen; /*!< length of the \a shortname */
	void 	(*func)(); /*!< routine to execute when this variable is selected. */
	void 	*var; /*!< data for this variable */
	int 	len; /*!< length of the variable */
	int	saved;
//	char 	*message; /*!< In case you want a message to be written when a setting is changed */
#ifdef OPTIONS_I18N
	dichar display;
#else
	char	*short_desc;/*!< The description that will be shown in the label */
	char	*long_desc; /*!< A longer description, shown when the player clicks on the option */
#endif
	struct {
		int tab_id; /*!< The tab ID in which we find this option */
		int label_id; /*!< The label ID associated with this option */
		int widget_id; /*!< Widget ID for things like checkboxes */
	} widgets;
	queue_t *queue; /*!< Queue that holds info for certain widget types. */
} var_struct;

/*!
 * a list of variables of type \see var_struct
 */
struct variables
{
	int no; /*!< current number of allocated \see var_struct in \a var */
	var_struct * var[200]; /*!< fixed array of \a no \see var_struct structures */
};

/*!
 * The type of variable name.
 */
typedef enum
{
	COMMAND_LINE_SHORT_VAR,	/*!< for abbreviated variable names from the command line */
	COMMAND_LINE_LONG_VAR,	/*!< for full variable names from the command line */
	INI_FILE_VAR,		/*!< for variables names from el.ini */
	IN_GAME_VAR		/*!< for names of variables chenged in the games */
} var_name_type;

extern struct variables our_vars; /*!< global variable containing all defined variables */

extern int write_ini_on_exit; /*< variable that determines if el.ini file is rewritten on exit of the program */

extern int elconfig_win;

extern int options_set;

void display_elconfig_win(void);

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

#ifdef __cplusplus
} // extern "C"
#endif

#endif
