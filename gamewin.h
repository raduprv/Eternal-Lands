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
