/*!
 * \file
 * \ingroup chat_win
 * \brief Declare the functions used to display the chat console.
 */
#ifndef __CHAT_H__
#define __CHAT_H__

#ifdef WINDOW_CHAT

/*! \name chat text constants 
 * @{ */
#define CHAT_WIN_TEXT_WIDTH   500 /*!< width of the text */
#define CHAT_WIN_TEXT_HEIGHT  (18*10) /*!< height of the text: 10 lines in normal font size */
#define CHAT_WIN_SCROLL_WIDTH 20 /*!< width of the scrollbar for the chat window */
/*! @} */

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

#endif // WINDOW_CHAT

#endif // def __CHAT_H__
