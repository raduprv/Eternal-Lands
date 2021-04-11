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
void add_questlog(const unsigned char *t, int len);

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
void set_next_quest_entry_id(Uint16 id);

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
void set_quest_finished(Uint16 id);


/*!
 * \ingroup quest_window
 * \brief Check if we have a quest id and waiting for an entry.
 *
 *      Check if we have a quest id and waiting for an entry.
 *
 * return true if if answer is yes.
 * \callgraph
 */
int waiting_for_questlog_entry(void);


/*!
 * \ingroup quest_window
 * \brief Clear the state that we are waiting for an entry.
 *
 *      Clear the state that we are waiting for an entry..
 *
 * \callgraph
 */
void clear_waiting_for_questlog_entry(void);


/*!
 * \ingroup quest_window
 * \brief Write the questlog options to the cfg file structure.
 *
 *      Write the questlog options to the cfg file structure.
 *
 * \callgraph
 */
unsigned int get_options_questlog(void);


/*!
 * \ingroup quest_window
 * \brief  Read the questlog options from the cfg file structure.
 *
 *      Read the questlog options from the cfg file structure.
 *
 * return true if if answer is yes.
 * \callgraph
 */
void set_options_questlog(unsigned int cfg_options);


#ifdef JSON_FILES
/*!
 * \ingroup quest_window
 * \brief Write the questlog options to the client state file.
 *
 *      Write the questlog options to the client state file.
 *
 * \callgraph
 */
void write_options_questlog(const char *dict_name);


/*!
 * \ingroup quest_window
 * \brief Read the questlog options from the client state file.
 *
 *      Read the questlog options from the client state file.
 *
 * \callgraph
 */
void read_options_questlog(const char *dict_name);
#endif


/*!
 * \ingroup quest_window
 * \brief  Draw a context menu like highlight.
 *
 *        Draw a context menu like highlight, i,e, shaded from top to bottom.
 *
 * \callgraph
 */
void draw_highlight(int topleftx, int toplefty, int widthx, int widthy, size_t col);


#ifdef __cplusplus
} // extern "C"
#endif

#endif	//__QUESTLOG_H__
