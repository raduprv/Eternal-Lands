/*!
 * \file
 * \ingroup quest_win
 * \brief handles the data and the display of the quest log.
 */
#ifndef __QUESTLOG_H__
#define __QUESTLOG_H__

/*!
 * the _logdata structure is a linked list with a string as its data.
 */
typedef struct ld
{
	char *msg; /*!< the message to log */
	struct ld *Next; /*!< link to the element in the list. */
}_logdata;

/*!
 * \name windows handlers
 */
/*! @{ */
extern int questlog_win; /*!< handle for the questlog window */
/*! @} */

extern int questlog_menu_x;
extern int questlog_menu_y;
extern int questlog_menu_x_len;
extern int questlog_menu_y_len;

/*!
 * \ingroup quest_win
 * \brief displays the questlog window
 *
 *      Displays the questlog window
 *
 * \return None
 */
void display_questlog();

/*!
 * \ingroup quest_win
 * \brief loads the questlog from the users filesystem.
 *
 *      Loads the questlog from the users filesystem.
 *
 * \return None
 */
void load_questlog();

/*!
 * \ingroup quest_win
 * \brief unloads the questlog and frees up the memory used.
 *
 *      Unloads the questlog and frees up the memory used.
 *
 * \return None
 */
void unload_questlog();

/*!
 * \ingroup quest_win
 * \brief adds the log specified in t up to the specified length to the users questlog.
 *
 *      Adds the log specified in the parameter t up to the specified length len to the users questlog.
 *
 * \param t     the log to add
 * \param len   the length of t
 * \return None
 */
void add_questlog(char *t, int len);

/*!
 * \ingroup quest_win
 * \brief adds the string t up to the given length as a new line to the questlog.
 *
 *      Adds the string t up to the given length as a new line to the questlog
 *
 * \param t     the log to add
 * \param len   the length of t
 * \return None
 */
void add_questlog_line(char *t, int len);

/*!
 * \ingroup quest_win
 * \brief goes to the entry in the questlog with the specified index.
 *
 *      Goes to the entry in the questlog with the specified index.
 *
 * \param ln    the index for the entry to search for.
 * \return None
 */
void goto_questlog_entry(int ln);

/*!
 * \ingroup misc_utils
 * \brief string_fix
 *
 *      string_fix(char*,int)
 *
 * \param t
 * \param len
 * \return None
 */
void string_fix(char *t, int len);

#endif	//__QUESTLOG_H__

