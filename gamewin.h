/*!
 * \file
 * \ingroup root_window
 * \brief   Handles the root window (game window) display.
 */
#ifndef __GAMEWIN_H__
#define __GAMEWIN_H__

#include <SDL_types.h>
#include "elwindows.h"
#include "keys.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \name Action types */
/*! @{ */
#define ACTION_WALK 0
#define ACTION_LOOK 1
#define ACTION_USE 2
#define ACTION_USE_WITEM 3
#define ACTION_TRADE 4
#define ACTION_ATTACK 5
#define ACTION_WAND 6
/*! @} */

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
extern int auto_disable_ranging_lock;
extern int target_close_clicked_creature;
extern int open_close_clicked_bag;
extern int show_fps; /*!< flag that indicates whether to display FPS or not */
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
int string_input(char *text, size_t maxlen, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod);

/*!
 * \ingroup events
 * \brief Checks for a quit or full screen key press
 *
 *      Checks if a keypress is for quitting the game or toggling the full screen mode.
 *
 * \param key_code    The key code as defined by SDL
 * \param key_mod     Modifier keys pressed in this event
 * \return 1 if the key event was handled by this function, 0 otherwise
 * \callgraph
 */
int check_quit_or_fullscreen (SDL_Keycode key_code, Uint16 key_mod);

/*!
 * \ingroup events
 * \brief Common handler for normal character input
 *
 *      Handles normal characters for the game, console and map windows
 *
 * \param key_code    The key code as defined by SDL
 * \param key_unicode Unicode value for the key text, if any
 * \param key_mod     Modifier keys pressed in this event
 * \return 1 if the key event was handled by this function, 0 otherwise
 * \callgraph
 */
int text_input_handler (SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod);

/*!
 * \ingroup events
 * \brief Changes the global action mode
 *
 *      Changes the global action mode
 *
 * \param mode the new action more
 * \callgraph
 */
void switch_action_mode(int mode);

/*!
 * \ingroup events
 * \brief Handles common keyboard events for the root window
 *
 *      Handles common keyboard events for the root window
 *
 * \param key_code    The key code as defined by SDL
 * \param key_unicode Unicode value for the key text, if any
 * \param key_mod     Modifier keys pressed in this event
 * \return 1 if the key was handled by this function, 0 otherwise
 * \callgraph
 */
int keypress_root_common (SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod);

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
 * \param key The key to handle
 * \callgraph
 */
void do_keypress(el_key_def key);

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

/*!
 * \brief Return width of the fps text or 0 if disabled.
 *
 * \callgraph
 */
int get_fps_default_width(void);

/*!
 * \name game root window action mode access functions
 */
/*! @{ */
void set_gamewin_action_mode(int new_mode);
int get_gamewin_action_mode(void);
void save_gamewin_action_mode(void);
int retrieve_gamewin_action_mode(void);
static inline int is_gamewin_look_action(void) { return (get_gamewin_action_mode() == ACTION_LOOK); }
static inline void clear_gamewin_look_action(void) { set_gamewin_action_mode(ACTION_WALK); }
static inline void set_gamewin_wand_action(void) { save_gamewin_action_mode(); set_gamewin_action_mode(ACTION_WAND); }
static inline void clear_gamewin_wand_action(void) { if (get_gamewin_action_mode() == ACTION_WAND) set_gamewin_action_mode(retrieve_gamewin_action_mode()); }
static inline void set_gamewin_usewith_action(void) { set_gamewin_action_mode(ACTION_USE_WITEM); }
/*! @} */

#ifdef __cplusplus
} // extern "C"
#endif

#endif // def __GAMEWIN_H__
