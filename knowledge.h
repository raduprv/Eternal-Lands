/*!
 * \file
 * \ingroup knowledge_win
 * \brief knowledge and knowledge window handling
 */
#ifndef __KNOWLEDGE_H__
#define __KNOWLEDGE_H__

/*!
 * knowledge structure
 */
typedef struct
{
	Uint8 present; /*!< flag, indicating whether this knowledge is present in the knowledge window. */
	Uint8 mouse_over; /*!< flag, indicating whether the mouse is over an entry in the knowledge window */
	char name[40]; /*!< name of the knowledge */
}knowledge;

/*!
 * \name windows handlers
 */
/*! @{ */
extern int knowledge_win; /*!< knowledge windows handler */
/*! @} */

extern int knowledge_menu_x;
extern int knowledge_menu_y;
extern int knowledge_menu_x_len;
extern int knowledge_menu_y_len;
//extern int knowledge_menu_dragged;
//extern int knowledge_scroll_dragged;
extern int knowledge_page_start;

extern knowledge knowledge_list[300]; /*!< global array of knowledgeable items */
extern char knowledge_string[400];

/*!
 * \ingroup knowledge_win
 * \brief
 *
 *      Detail
 *
 * \return None
 */
void display_knowledge();

//int knowledge_mouse_over();
//int check_knowledge_interface();

/*!
 * \ingroup knowledge_win
 * \brief
 *
 *      Detail
 *
 * \param size
 * \param list
 * \return None
 */
void get_knowledge_list(Uint16 size, char *list);

/*!
 * \ingroup knowledge_win
 * \brief
 *
 *      Detail
 *
 * \param idx
 * \return None
 */
void get_new_knowledge(Uint16 idx);
#endif
