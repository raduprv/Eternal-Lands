/*!
 * \file
 * \ingroup chat_win
 * \brief Declare the functions used to display the chat console.
 */
#ifndef __CHAT_H__
#define __CHAT_H__

#ifndef OLD_EVENT_HANDLER

extern int use_windowed_chat; /*!< flag indicating whether we use the new windowed chat window or not */

extern int chat_win_text_width; /*!< width of the chat window */

/*!
 * \ingroup chat_win
 * \brief   Update handler for the scrollbar position of the chat window
 *
 *      Updates the scrollbar of the chat window
 *
 * \callgraph
 */
void update_chat_scrollbar ();

/*!
 * \ingroup chat_win
 * \brief   Displays the chat window
 *
 *      Displays the chat window
 *
 * \callgraph
 */
void display_chat ();

#endif // not def OLD_EVENT_HANDLER

#endif // def __CHAT_H__
