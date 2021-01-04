/*!
 * \file
 * \ingroup actors_utils
 * \brief handling of local and global filter lists.
 */
#ifndef __FILTER_H__
#define __FILTER_H__

#include	"ignore.h"	/* just in case it hasn't been included */

#ifdef __cplusplus
extern "C" {
#endif

extern int have_storage_list; /*!< Flag indicating if we can use a cached responce to \#storage */
extern int use_global_filters; /*!< global flag, indicating whether global filtering is used or not */
extern int caps_filter; /*!< global flag, indicating whether filter of caps is enabled or not */
extern char storage_filter[128]; /*!< string to use as filter when using the \#storage \<name\> command */

extern unsigned char cached_storage_list[8192]; /*!< Copy of the result of \#storage when last sent to server */

/*!
 * \ingroup actors_utils
 * \brief   adds the given \a name to the list of filtered words.
 *
 *      Adds the given \a name to the list of filtered words. The parameter \a save_name indicates whether this entry should be persisted.
 *
 * \param name          the name to add to the list of filtered words
 * \param local		flag indicating whether the filter is local, or global
 * \param save_name     inidicator whether to save this name in the configuration files.
 * \retval int
 */
int add_to_filter_list (const char *name, char local, char save_name);

/*!
 * \ingroup actors_utils
 * \brief       removes the \a name from the current list of filters.
 *
 *      Removes \a name from the current list of filters.
 *
 * \param name      the name to remove from the list.
 * \retval int
 */
int remove_from_filter_list (const char *name);

/*!
 * \ingroup actors_utils
 * \brief   Filters the \a input_text by the current storage_filter
 *
 *      Filters the \a input_text by the current storage_filter
 *
 * \param input_text    the text to filter
 * \param len           the length of \a input_text
 * \param size          the maximum number of bytes in \a input_text
 * \retval int
 * \callgraph
 */
int filter_storage_text (char * input_text, int len, int size);

/*!
 * \ingroup actors_utils
 * \brief   filters the \a input_text of occurrences of words in filter_list and replaces them with
 * 	the associated replacement
 *
 *      Filters the \a input_text of occurrences of words in filter_list and replaces them with the
 *      associated replcement
 *
 * \param input_text    the text to filter
 * \param len           the length of \a input_text
 * \param size          the maximum number of bytes in \a input_text
 * \retval int
 * \callgraph
 */
int filter_text (char *input_text, int len, int size);

/*!
 * \ingroup loadsave
 * \brief Load a filter list from file
 *
 *	Loads a filter list from file \a file_name
 *
 * \param file_name	The name of the input file
 * \param local		Whether to process the filters as global or local
 * \callgraph
 */
void load_filters_list (const char *file_name, char local);

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
int list_filters();

#ifdef DEBUG
void print_filter_list ();
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif
