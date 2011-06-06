/*!
 * \file
 * \ingroup root_window
 * \brief   Handles the root window (game window) display.
 */
#ifndef __GAMEWIN_H__
#define __GAMEWIN_H__

#include <SDL_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HUD_MARGIN_X 64
#define HUD_MARGIN_Y 49

/*! \name windows handlers 
 * @{ */
extern int game_root_win; /*!< the root (game) window */
/*! @} */
extern int use_old_clicker;
extern Uint32 next_fps_time;
extern int last_count;
extern float fps_average;
extern int include_use_cursor_on_animals;
extern int have_mouse;
extern int keep_grabbing_mouse;
extern int just_released_mouse;
void toggle_have_mouse();
#ifdef NEW_CURSOR
extern int cursors_tex;
#endif // NEW_CURSOR
extern int cm_banner_disabled;
extern int logo_click_to_url;
void draw_special_cursors();

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
