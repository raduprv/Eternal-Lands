/*!
 * \file
 * \ingroup actors_utils
 * \brief handling of local and global filter lists.
 */
#ifndef __FILTER_H__
#define __FILTER_H__

#include	"ignore.h"	/* just in case it hasn't been included */

/*!
 * \name    max. number of filters for \see filter_list
 * @{
 */
#define max_filters 1000
/*! @} */

/*!
 * a single entry in the \see filter_list
 */
typedef struct
{
	Uint8 name[16]; /*!< the name of the filter entry */
	int len; /*!< length of \a name */
}filter_slot;

extern filter_slot filter_list[max_filters]; /*!< global variable of filters */
extern int filtered_so_far; /*!< number of items that have been filter duing the current execution */
extern int save_filters; /*!< global flag, indicating whether filters should be saved between executions or not */
extern int use_global_filters; /*!< global flag, indicating whether global filtering is used or not */
extern char text_filter_replace[]; /*!< string, that contains the word to replace each entry in \see filter_list with */
extern int caps_filter; /*!< global flag, indicating whether filter of caps is enabled or not */
extern char storage_filter[128]; /*!< string to use as filter when using the #storage \<name\> command */

/*!
 * \ingroup actors_utils
 * \brief   adds the given \a name to the list of filtered words.
 *
 *      Adds the given \a name to the list of filtered words. The parameter \a save_name indicates whether this entry should be persisted.
 *
 * \param name          the name to add to the list of filtered words
 * \param save_name     inidicator whether to save this name in the configuration files.
 * \retval int
 */
int add_to_filter_list(Uint8 *name, char save_name);

/*!
 * \ingroup actors_utils
 * \brief       removes the \a name from the current list of filters.
 *
 *      Removes \a name from the current list of filters.
 *
 * \param name      the name to remove from the list.
 * \retval int
 */
int remove_from_filter_list(Uint8 *name);

/*!
 * \ingroup actors_utils
 * \brief   checks if the given \a name is filtered (aka ignored).
 *
 *      Checks if the given \a name is filtered (aka ignored)
 *
 * \param name      the name to check
 * \retval int
 */
int check_if_filtered(Uint8 *name);

/*!
 * \ingroup actors_utils
 * \brief   filters the \a input_text of occurrences of words in filter_list and replaces them with the string currently stored in text_filter_replace.
 *
 *      Filters the \a input_text of occurrences of words in filter_list and replaces them with the string currently stored in text_filter_replace.
 *
 * \param input_text    the text to filter
 * \param len           the length of \a input_text
 * \retval int
 * \callgraph
 */
int filter_text(Uint8 * input_text, int len);

/*!
 * \ingroup loadsave
 * \brief   loads a list of filters from \a file_name.
 *
 *      Loads a list of filters from the file \a file_name and adds them to the variable filter_list.
 *
 * \param file_name     the filename from where to load the filter specs.
 *
 * \callgraph
 */
void load_filters_list(char * file_name);

/*!
 * \ingroup actors_utils
 * \brief       clears the list of currently defined filters.
 *
 *      Clears the list of currently defined and active filters.
 *
 * \sa load_filters
 */
void clear_filter_list();

/*!
 * \ingroup loadsave
 * \brief       loads the global and local filters from their corresponding files.
 *
 *      Loads both, the global and local filter lists from their corresponding files.
 *
 * \callgraph
 */
void load_filters();

/*!
 * \ingroup actors_utils
 * \brief   list all currently defined filters on the console.
 *
 *      Lists all currently defined filters to the console.
 *
 */
void list_filters();

#endif
