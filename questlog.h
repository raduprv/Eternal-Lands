/*!
 * \file
 * \ingroup quest_window
 * \brief handles the data and the display of the quest log.
 */
#ifndef __QUESTLOG_H__
#define __QUESTLOG_H__

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \name windows handlers
 */
/*! @{ */
extern int questlog_win; /*!< handle for the questlog window */
/*! @} */

extern int questlog_menu_x;
extern int questlog_menu_y;

/*!
 * \ingroup quest_window
 * \brief Displays the questlog window
 *
 *      Displays the questlog window
 *
 * \callgraph
 */
void display_questlog();

/*!
 * \ingroup quest_window
 * \brief Loads the questlog from the users filesystem.
 *
 *      Loads the questlog from the users filesystem.
 *
 * \callgraph
 */
void load_questlog();

/*!
 * \ingroup quest_window
 * \brief Unloads the questlog and frees up the memory used.
 *
 *      Unloads the questlog and frees up the memory used.
 *
 */
void unload_questlog();

/*!
 * \ingroup quest_window
 * \brief Adds the log specified in t up to the specified length to the users questlog.
 *
 *      Adds the log specified in the parameter t up to the specified length len to the users questlog.
 *
 * \param t     the log to add
 * \param len   the length of t
 *
 * \callgraph
 */
void add_questlog(char *t, int len);

/*!
 * \ingroup quest_window
 * \brief Goes to the entry in the questlog with the specified index.
 *
 *      Goes to the entry in the questlog with the specified index.
 *
 * \param ln    the index for the entry to search for.
 */
void goto_questlog_entry(int ln);

/*!
 * \ingroup quest_window
 * \brief Sets the window handler functions for the quest log window
 *
 *      Sets the window handler functions for the quest log window
 *
 * \callgraph
 */
void fill_questlog_win ();


#ifdef NEW_QUESTLOG

/*!
 * \ingroup quest_window
 * \brief Sets the quest id for the next quest log entry which will be sent.
 *
 *      Sets the quest id for the next quest log entry which will be sent.
 * 
 * \param id	the quest id
 *
 * \callgraph
 */
void set_next_quest_entry_id(int id);

/*!
 * \ingroup quest_window
 * \brief Set the title for the specified quest.
 *
 *      Set the title for the specified quest.
 *
 * \param data	pointer to non null terminated string
 * \param len	the length in bytes of the title
 * 
 * \callgraph
 */
void set_quest_title(const char *data, int len);

/*!
 * \ingroup quest_window
 * \brief Set the specified quest as completed
 *
 *      Set the specified quest as completed, the user interface
 * shows completed and not completed quests differently.
 *
 * \param id	the quest id
 * 
 * \callgraph
 */
void set_quest_finished(int id);

#endif // NEW_QUESTLOG


#ifdef __cplusplus
} // extern "C"
#endif

#endif	//__QUESTLOG_H__
