/*!
 * \file
 * \ingroup config
 * \brief config file related functions.
 */
#ifndef __ELCONFIG_H__
#define __ELCONFIG_H__

/*!
 * var_struct stores the data for a single configuration entry.
 */
typedef struct
{
	int 	type; /*!< type of the variable */
	char	name[52]; /*!< name of the variable */
	int 	nlen; /*!< length of the \a name */
	char 	shortname[12]; /*!< shortname of the variable */
	int 	snlen; /*!< length of the \a shortname */
	void 	(*func)(); /*!< routine to execute when this variable is selected. */
	void 	*var; /*!< data for this variable */
	int 	len; /*!< length of the variable */
#ifdef ELCONFIG
	int	saved;
#endif
//	char 	*message; //In case you want a message to be written when a setting is changed
} var_struct;

/*!
 * a list of variables of type \see var_struct
 */
struct variables
{
	int no; /*!< current number of allocated \see var_struct in \a var */
	var_struct * var[100]; /*!< fixed array of \a no \see var_struct structures */
};

extern struct variables our_vars; /*!< global variable containing all defined variables */

/*!
 * \ingroup config
 * \brief   checks whether we have a variable with the given \a str as name and the given \a type.
 *
 *      Checks whether we have a variable with the given \a str as name and the given \a type.
 *
 * \param str       the name of the variable to check
 * \param type      the type of the variable
 * \retval int      0 if \a str is found, else !=0.
 *
 * \sa read_command_line
 * \sa read_config
 */
int check_var(char * str, int type);

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

#endif
