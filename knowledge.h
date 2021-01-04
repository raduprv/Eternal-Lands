/*!
 * \file
 * \ingroup knowledge_window
 * \brief Knowledge and knowledge window handling
 */
#ifndef __KNOWLEDGE_H__
#define __KNOWLEDGE_H__

#include <SDL_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KNOWLEDGE_LIST_SIZE 1024 /*!< maximum size of the \ref knowledge_list */
#define KNOWLEDGE_NAME_SIZE 40 /*!< maximum size (including termination) of each \ref knowledge_list name string */

/*!
 * knowledge structure
 */
typedef struct
{
	Uint8 present; /*!< flag, indicating whether this knowledge is present in the knowledge window. */
	Uint8 mouse_over; /*!< flag, indicating whether the mouse is over an entry in the knowledge window */
	char name[KNOWLEDGE_NAME_SIZE]; /*!< name of the knowledge */
	Uint8 has_book; /*!< flag, indicating whether the knowledge item has an assosiated book */
}knowledge;

extern knowledge knowledge_list[KNOWLEDGE_LIST_SIZE]; /*!< global array of knowledgeable items */

/*!
 * \brief   Derive the resring complete and total from the output of the server "#research" command.
 *
 * \param knowledge_id  the knowledge id
 *
 * returns	1 if it was a request, otherwise 0 so it was a user request.
 *
 * \callgraph
 */
int get_true_knowledge_info(const char *message);

/*!
 * \brief   When we get HERE_YOUR_STATS, send server #research command to get true research info.
 *
 * \param knowledge_id  the knowledge id
 *
 * \callgraph
 */
void request_true_knowledge_info(void);

/*!
 * \brief   Get the book status tag, read, unread or reading.
 *
 * \param knowledge_id  the knowledge id
 *
 * returns	return the reading status tag, or an empty string if not a valid book id.
 *
 * \callgraph
 */
const char *get_knowledge_state_tag(size_t index);

/*!
 * \ingroup knowledge_window
 * \brief   Gets the known knowledges from the \a list and stores the state in \ref knowledge_list.
 *
 *      Gets the known knowledges from the given \a list and stores the state in \ref knowledge_list.
 *
 * \param size  the size of \a list
 * \param list  a list of knowledges that are already known by the player.
 *
 */
void get_knowledge_list (Uint16 size, const char *list);

/*!
 * \ingroup knowledge_window
 * \brief   Marks the entry in \ref knowledge_list at the given index \a idx as present.
 *
 *      Marks the entry at index \a idx in the \ref knowledge_list as present, aka known to the player.
 *
 * \param idx   the index into \ref knowledge_list that should be marked as present.
 *
 * \note This function does not perform any sanity checks on \a idx. This is a possible bug.
 * \bug Possible bug, because this function doesn't do any sanity checks on the parameter \a idx.
 */
void get_new_knowledge(Uint16 idx);

/*! 
 * \ingroup knowledge_window
 * \brief Sets the window handler functions for the knowledge window
 *
 * 	Sets the \ref ELW_HANDLER_DISPLAY, \ref ELW_HANDLER_CLICK and \ref ELW_HANDLER_MOUSEOVER event handler functions for the knowledge window.
 *
 * \param window_id	id of window created for tab
 * 
 * \callgraph
 */
void fill_knowledge_win (int window_id);

/*! 
 * \ingroup knowledge_window
 * \brief Checks any current reading book is known by the cleint and gives warning message if not.
 *
 * \callgraph
 */
void check_book_known(void);

/*! 
 * \ingroup knowledge_window
 * \brief Returns the eta string for current research.
 *
 * \param str	string to write eta
 * \param size	capacity of string including terminating \0
 * 
 * \callgraph
 */
char *get_research_eta_str(char *str, size_t size);

/*! 
 * \ingroup knowledge_window
 * \brief Returns the progress on the current book in the range 0 to 1.
 *
 * \callgraph
 */
float get_research_fraction(void);

/*! 
 * \ingroup knowledge_window
 * \brief Returns 1 if currently reading, otherwise 0.
 *
 * \callgraph
 */
int is_researching(void);

/*! 
 * \ingroup knowledge_window
 * \brief recalculate rate of research.
 *
 * \callgraph
 */
void update_research_rate(void);

/*!
 * \ingroup knowledge_window
 * \brief set the information string.
 *
 * \callgraph
 */
void set_knowledge_string(const Uint8 *in_data, int data_length);

/*!
 * \ingroup knowledge_window
 * \brief Read the knowledge list from file.
 *
 * \callgraph
 */
void load_knowledge_list(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
