/*!
 * \file
 * \ingroup windows
 * \brief handling and displaying the HUD
 */
#ifndef	__HUD_H
#define	__HUD_H

#include <SDL_types.h>
#include "elwindows.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	HUD_INTERFACE_NEW_CHAR, /*!< the interface for the character creation screen */
	HUD_INTERFACE_GAME,     /*!< the interface for the game */
	HUD_INTERFACE_LAST      /*!< the last interface used */
} hud_interface;

extern Uint32 exp_lev[200];
extern int hud_text;
extern int hud_x;
extern int hud_y;
extern int show_help_text;
extern int always_enlarge_text;

/*!
 * \ingroup other
 * \brief Checks if the keypress is an item use
 *
 *	returns 1 if the key is a item keypress, otherwise 0.
 */
int action_item_keys(Uint32 key);

/*!
 * \ingroup other
 * \brief Initializes anything hud related
 *
 *      Initializes anything related to the hud, i.e. the hud frame, icons, stats display, quickbar and misc. items like compass and clock.
 *
 * \param	type Whether it's for the game window or the new character window
 * \callgraph
 */
void init_hud_interface (hud_interface type);

/*!
 * \ingroup other
 * \brief Called on client exit - free memory and generally clean up
 *
 * \callgraph
 */
void cleanup_hud(void);

/*!
 * \ingroup other
 * \brief Shows the different hud related windows if they have already been created.
 *
 *      Shows the different hud related windows, i.e. the icons, the stats bar, the miscellaneous (compass and clock) and the quickbar window if they have been created before. If none of them has been created nothing will be done.
 *
 * \pre If any of \ref icons_win, \ref stats_bar_win, \ref misc_win and \ref quickbar_win is <= 0, no action will be performed.
 *
 * \callgraph
 */
void show_hud_windows (void);

/*!
 * \ingroup other
 * \brief Hides the different hud related windows, if they are visible.
 *
 *      Hides the different hud related windows, i.e. the icons, the stats bar, the miscellaneous (compass and clock) and the quickbar window if they are visible. If none of them is visible nothing will be done.
 *
 * \pre If none of \ref icons_win, \ref stats_bar_win, \ref misc_win and \ref quickbar_win is >= 0 (i.e. created before and visible) no action will be performed.
 * \callgraph
 */
void hide_hud_windows (void);

/*!
 * \ingroup display_2d
 * \brief Draws the hud interface related items.
 *
 *      Draws the hud interface related items.
 *
 * \callgraph
 */
void draw_hud_interface(window_info *win);

/*!
 * \ingroup windows
 * \brief Get the window ID pointer using the name string
 *
 * \param name		the name of the window
 *
 *	returns if sucessful, a pointer to the window id variable, otherwise NULL.
 * 
 * \callgraph
 */
int* get_winid(const char *name);

/*!
 * \ingroup windows
 * \brief Shows the window pointed to by \a win
 *
 *      Shows the window pointed to by \a win by calling the appropriate display_*_win function if \a win was not created before, else \ref toggle_window is called.
 *
 * \param win   the id of the window to show
 * \param id    unused
 *
 * \pre If \a win is either of \ref items_win, \ref sigil_win or \ref manufacture_win and the \ref trade_win is currently active, and error message will get logged to the console and the functions returns.
 * \callgraph
 */
void view_window(int * win, int id);

/*!
 * \ingroup windows
 * \brief Shows the selected \a tab of the given \a window.
 *
 *      Shows the selected \a tab of the given \a window.
 *
 * \param window    the id of the window
 * \param col_id    the id of the tab collection
 * \param tab       the id of the tab to show
 *
 * \pre If \a window is already visisble and \a tab is the currently selected tab, the window will be hidden.
 * \pre If \a window is already visisble but \a tab is currently not selected, then \a tab will be selected.
 * \callgraph
 */
void view_tab (int *window, int *col_id, int tab);

/*!
 * \ingroup windows
 * \brief Shows the \a message at the given position (\a x, \a y).
 *
 *      Shows the \a message at the given position (\a x, \a y) using the small font.
 *
 * \param message   the help message to show
 * \param x         the x coordinate of the position to draw the help message
 * \param y         the y coordinate of the position to draw the help message
 * \param scale     the multiplier for the text size
 *
 * \callgraph
 */
void show_help(const char *message, int x, int y, float scale);

/*!
 * \ingroup windows
 * \brief Shows the \a message at the given position (\a x, \a y).
 *
 *      Shows the \a message at the given position (\a x, \a y) using the default font.
 *
 * \param message   the help message to show
 * \param x         the x coordinate of the position to draw the help message
 * \param y         the y coordinate of the position to draw the help message
 * \param scale     the multiplier for the text size
 *
 * \callgraph
 */
void show_help_big(const char *message, int x, int y, float scale);
void show_help_coloured_scaled(const char *help_message, int x, int y, float r, float g, float b, int use_big_font, float size);

/*!
 * \ingroup windows
 * \brief Check if we need to enlarge text.
 *
 *      If the "Always Enlarge Text" option on the HUB tab is set, return
 *      true.  Otherwise test if either Ctrl or Alt is pressed and return
 *      true if one of those is set.
 *
 *	returns true if text should be enlarged.
 * 
 * \callgraph
 */
int enlarge_text(void);

/*!
 * \ingroup other
 * \brief   Initializes the levels table.
 *
 *      Initializes the experience levels table.
 *
 * \sa init_stuff
 */
void build_levels_table();

/*!
 * \ingroup other
 * \brief   	The #exp command, show current exp levels in console.
 * \retval	1, so command not passed to server
 */
int show_exp(char *text, int len);

#ifdef __cplusplus
} // extern "C"
#endif


#endif	//__HUD_H
