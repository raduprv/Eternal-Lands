/*!
 * \file
 * \ingroup actors_utils
 * \brief Handling of local and global ignore lists.
 */
#ifndef __IGNORE_H__
#define __IGNORE_H__

#include "interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \name Array size */
/*! @{ */
#define MAX_IGNORES 1000  /*!< defines the max. number of entries in ignore_list */
/*! @} */

/*!
 * Structure to store information on ignored players.
 */
typedef struct
{
	char name[MAX_USERNAME_LENGTH +1]; /*!< name of the player to ignore */
	char used; /*! flag, indicating whether this ignore_slot is in use or not */
}ignore_slot;

extern ignore_slot ignore_list[MAX_IGNORES]; /*!< global array of names to ignore */

extern int save_ignores; /*!< flag, inidicating whether the ignores should be persisted between different executions. */
extern int use_global_ignores; /*!< flag, indicating whether to use global ignores file or not */

/*!
 * \ingroup actors_utils
 * \brief   Saves the given \a name in the \ref ignore_list.
 *
 *      Saves the given \a name in the \ref ignore_list. If \a save_name is true, the name will also be saved in the local_ignores.txt file.
 *
 * \param name          the name to save in the \ref ignore_list
 * \param save_name     flag, indicating whether the list should be saved in the local_ignores.txt file.
 * \retval int          1, if \a name was successfully added; -1, if \a name is already present in the list; -2, if \ref ignore_list is full.
 *
 * \pre If \a name is already present in \ref ignore_list, the function will return -1.
 */
int add_to_ignore_list(char *name, char save_name);

/*!
 * \ingroup actors_utils
 * \brief   Removes the given \a name from the \ref ignore_list.
 *
 *      Removes the given \a name from the \ref ignore_list. If the name is found, the changes will be saved to the local_ignores.txt file.
 *
 * \param name  the name to remove from \ref ignore_list
 * \retval int  1, if the entry was found and removed, else -1
 */
int remove_from_ignore_list(char *name);


/*!
 * \ingroup actors_utils
 * \brief   Checks if \a name is ignored.
 *
 *      Checks if \a name is ignored.
 *
 * \param name          The name to check the ignore list for
 * \retval int          true (1) if \a name is ignored, else false (0).
 */
int check_if_ignored (const char *name);


/*!
 * \ingroup actors_utils
 * \brief   Checks if the sender of \a input_text is already ignored.
 *
 *      Checks if the sender of \a input_text is already ignored.
 *
 * \param input_text    the message to check for ignored users
 * \param len		the length of \a input_text
 * \param channel        the channel the message comes from
 * \retval int          true (1) if the sender of \a input_text is ignored, else false (0).
 */
int pre_check_if_ignored (const char *input_text, int len, Uint8 channel);

/*!
 * \ingroup loadsave
 * \brief   Loads the ignore lists. The function is called from \ref init_stuff.
 *
 *      Called from \ref init_stuff, the function loads the ignore lists. First the local_ignores.txt file will be loaded and if the \ref use_global_ignores setting is true, the global_ignores.txt file will also be loaded.
 *
 * \callgraph
 */
void load_ignores();

/*!
 * \ingroup actors_utils
 * \brief   Lists the names that are currently ignored by the actor, or a message stating that the actor is not ignoring anyone.
 *
 *      Lists the names that are currently ignored by the actor, if any. The function is called, when the actor uses the #ignores command.
 *
 */
int list_ignores();

#ifdef __cplusplus
} // extern "C"
#endif

#endif
