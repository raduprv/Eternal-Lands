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

/*!
 * \name windows handlers
 */
/*! @{ */
extern int knowledge_win; /*!< knowledge window handler */
/*! @} */

extern int knowledge_menu_x;
extern int knowledge_menu_y;

extern knowledge knowledge_list[KNOWLEDGE_LIST_SIZE]; /*!< global array of knowledgeable items */
extern char knowledge_string[400];
extern int	knowledge_count;

/*!
 * \ingroup knowledge_window
 * \brief   Displays the knowledge window
 *
 *      Displays the knowledge window. If \ref knowledge_win is less than 0, this function creates and fills the knowledge window first, else it simply selects and shows the already created window.
 *
 * \callgraph
 */
void display_knowledge();

//int knowledge_mouse_over();
//int check_knowledge_interface();

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
 * \callgraph
 */
void fill_knowledge_win ();

/*! 
 * \ingroup knowledge_window
 * \brief Checks any current reading book is known by the cleint and gives warning message if not.
 *
 * \callgraph
 */
void check_book_known();

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

#ifdef __cplusplus
} // extern "C"
#endif

#endif
