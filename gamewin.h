/*!
 * \file
 * \ingroup root_window
 * \brief   Handles the root window (game window) display.
 */
#ifndef __GAMEWIN_H__
#define __GAMEWIN_H__

#include <SDL_types.h>
#include "elwindows.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int HUD_MARGIN_X;
extern int HUD_MARGIN_Y;
extern int have_mouse;
#ifndef MAP_EDITOR
extern float fps_average;
#endif //!MAP_EDITOR

/*! \name windows handlers 
 * @{ */
extern int game_root_win; /*!< the root (game) window */
/*! @} */

/*! \name configuration options 
 * @{ */
extern int use_old_clicker;
extern int include_use_cursor_on_animals;
extern int cm_banner_disabled;
extern int logo_click_to_url;
extern char LOGO_URL_LINK[128];		/*!< the link clicking the EL logo sends you to */
extern int auto_disable_ranging_lock;
/*! @} */

/*!
 * \brief Return the true if the ranging lock is on.
 *
 *	Return the true if the ranging lock is on.
 *
 * \callgraph
 * \retval int 0 for off, non-zero on
 */
int ranging_lock_is_on(void);

/*!
 * \brief Turn of ranging lock if on and auto disable is active.
 *
 *	Turn of ranging lock if on and auto disable is active.  Give the normal console message.
 *
 * \callgraph
 */
void check_to_auto_disable_ranging_lock(void);

/*!
 * \brief Draw the action graphic for the current cursor.
 *
 *	Draw the action graphic for the current cursor.
 *
 * \callgraph
 */
void draw_special_cursors(void);

/*!
 * \brief Switch between normal and grab mode.
 *
 *	Switch between normal and grab mode.
 *
 * \callgraph
*/
void toggle_have_mouse(void);

/*!
 * \ingroup events
 * \brief Converts a 32-bit key code to an unsigned character
 *
 *	Utility function for keypress handlers that converts the 32-bit key code to an unsigned character
 *
 * \param unikey The full key code
 * \retval Uint8
 * \callgraph
 */
Uint8 key_to_char (Uint32 unikey);

/*!
 * \brief Maintain an input string with key presses
 *
 *	The keypress is appended to the string up to the max.  If the key
 * 	press is the backspace chatacter, the last char is removed.
 * 
 * \retval int 1 if key used, 0 if not used
 *
 */
int string_input(char *text, size_t maxlen, char ch);

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
 * \brief Common handler for normal character input
 *
 *      Handles normal characters for the game, console and map windows
 *
 * \param key
 * \param unikey
 * \retval int
 * \callgraph
 */
int text_input_handler (Uint32 key, Uint32 unikey);

/*!
 * \ingroup events
 * \brief Handles common keyboard events for the root window
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
 * \brief Handles common functions for root window display
 *
 *      Handles common functions for root window display
 *
 * \param win	pointer to the window structure
 * \callgraph
 */
void display_handling_common(window_info *win);

/*!
 * \brief Handles common setup when retuening to the game window.
 *
 *      Handles common setup when retuening to the game window.
 *
 * \callgraph
 */
void return_to_gamewin_common(void);

/*!
 * \ingroup events
 * \brief treat key value as if it was a real key press 
 *
 * \param key
 * \callgraph
 */
void do_keypress(Uint32 key);

/*!
 * \ingroup root_window
 * \brief Creates the game (root) window of the game.
 *
 *      Creates the game window, aka root window of the game
 *
 * \param width the width of the window
 * \param height the height of the window
 * \callgraph
 */
void create_game_root_window (int width, int height);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // def __GAMEWIN_H__
