/*!
 * \file
 * \ingroup actors_utils
 * \brief handling of local and global ignore lists.
 */
#ifndef __IGNORE_H__
#define __IGNORE_H__

/*!
 * \name Array size
 *      defines the max. number of entries in \see ignore_list
 */
/*! @{ */
#define max_ignores 1000
/*! @} */

/*!
 * structure to store information on ignored players.
 */
typedef struct
{
	Uint8 name[16]; /*!< name of the player to ignore */
	char used; /*! flag, indicating whether this ignore_slot is in use or not */
}ignore_slot;

extern ignore_slot ignore_list[max_ignores]; /*!< global array of names to ignore */
extern int ignored_so_far; /*!< number of players ignored so far in this execution */
extern int save_ignores; /*!< flag, inidicating whether the ignores should be persisted between different executions. */
extern int use_global_ignores; /*!< flag, indicating whether to use global ignores file or not */

/*!
 * \ingroup actors_utils
 * \brief
 *
 *      Detail
 *
 * \param name
 * \param save_name
 * \return int
 */
int add_to_ignore_list(Uint8 *name, char save_name);

/*!
 * \ingroup actors_utils
 * \brief
 *
 *      Detail
 *
 * \param name
 * \return int
 */
int remove_from_ignore_list(Uint8 *name);

/*!
 * \ingroup actors_utils
 * \brief
 *
 *      Detail
 *
 * \param name
 * \return int
 */
int check_if_ignored(Uint8 *name);

/*!
 * \ingroup actors_utils
 * \brief
 *
 *      Detail
 *
 * \param input_text
 * \param type
 * \return int
 */
int pre_check_if_ignored(Uint8 * input_text, int type);

/*!
 * \ingroup loadsave
 * \brief
 *
 *      Detail
 *
 * \param file_name
 * \return None
 */
void load_ignores_list(char * file_name);

/*!
 * \ingroup actors_utils
 * \brief
 *
 *      Detail
 *
 * \return None
 */
void clear_ignore_list();

/*!
 * \ingroup loadsave
 * \brief
 *
 *      Detail
 *
 * \return None
 */
void load_ignores();

/*!
 * \ingroup actors_utils
 * \brief
 *
 *      Detail
 *
 * \return None
 */
void list_ignores();

#endif
