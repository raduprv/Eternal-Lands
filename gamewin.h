/*!
 * \file
 * \ingroup root_win
 * \brief   Handles the root window (game window) display.
 */
#ifndef __GAMEWIN_H__
#define __GAMEWIN_H__

#ifdef WINDOW_CHAT

/*! \name windows handlers 
 * @{ */
extern int game_win; /*!< the root (game) window */
/*! @} */

/*!
 * \ingroup events
 * \brief Converts a 32-bit key code to an unsigned character
 *
 *	Utility function for keypress handlers that converts the 32-bit key code
 to an unsigned character
 *
 * \param key The full key code
 * \retval Uint8
 * \callgraph
 */
Uint8 key_to_char (Uint32 unikey);

/*!
 * \ingroup events
 * \brief Checks for a quit or full screen key press
 *
 *      Checks if a keypress is for quitting the game or toggling the full screen mode.
 *
 * \param key
 * \retval int
 * \callgraph
 */

int check_quit_or_fullscreen (Uint32 key);
/*!
 * \ingroup events
 * \brief common handler for normal character input
 *
 *      Handles normal characters for the game, console and map windows
 *
 * \param ch
 * \retval int
 * \callgraph
 */
int text_input_handler (Uint8 ch);

/*!
 * \ingroup events
 * \brief handles common keyboard events for the root window
 *
 *      Handles common keyboard events for the root window
 *
 * \param key
 * \param unikey
 * \retval int
 * \callgraph
 */
int keypress_root_common (Uint32 key, Uint32 unikey);

/*!
 * \ingroup root_win
 * \brief Creates the game (root) window of the game.
 *
 *      Creates the game window, aka root window of the game
 *
 * \callgraph
 */
void create_game_window ();

#endif

#endif // def __GAMEWIN_H__
