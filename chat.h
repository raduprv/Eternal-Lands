/*!
 * \file
 * \ingroup chat_win
 * \brief Declare the functions used to display the chat console.
 */
#ifndef __CHAT_H__
#define __CHAT_H__

extern int use_windowed_chat; /*!< flag indicating whether we use the new windowed chat window or not */

extern int chat_win; /*!< handler for the chat window */

extern int chat_win_text_width; /*!< width of the chat window */

/*!
 * \ingroup chat_win
 * \brief   clear_input_line
 *
 *      Detail
 *
 * \callgraph
 */
void clear_input_line ();

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
 * \brief   Handle a keypress of the root window 
 *
 *      Handles a keypress in the root window as if it were pressed in the chat window input field. 
 *
 * \param key
 * \param unikey
 *
 * \retval int 1 if handled, 0 otherwise
 * \callgraph
 */
int root_key_to_input_field (Uint32 key, Uint32 unikey);

/*!
 * \ingroup chat_win
 * \brief   Paste a text into the input field
 *
 *      Pastes a text line at the current cursor position in the input field
 *
 * \param text the text to paste
 *
 * \callgraph
 */
void paste_in_input_field (const Uint8 *text);

/*!
 * \ingroup chat_win
 * \brief   Displays the chat window
 *
 *      Displays the chat window
 *
 * \callgraph
 */
void display_chat ();

#endif // def __CHAT_H__
