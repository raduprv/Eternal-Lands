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

/*!
 * \name quick window orientation constants
 */
/*! @{ */
#define HORIZONTAL 2
#define VERTICAL 1
/*! @} */

typedef enum
{
	HUD_INTERFACE_NEW_CHAR, /*!< the interface for the character creation screen */
	HUD_INTERFACE_GAME,     /*!< the interface for the game */
	HUD_INTERFACE_LAST      /*!< the last interface used */
} hud_interface;

#define MAX_EXP_LEVEL 180
extern Uint32 exp_lev[MAX_EXP_LEVEL];

extern int hud_text;
extern int hud_x;
extern int hud_y;
extern int show_help_text;
extern int always_enlarge_text;
extern int logo_click_to_url;
extern char LOGO_URL_LINK[128];		/*!< the link clicking the EL logo sends you to */

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
 * Shows the different hud related windows, i.e. the icons, the stats bar,
 * the miscellaneous (compass and clock) and the quickbar window if they have
 * been created before. If none of them has been created nothing will be done.
 *
 * \pre
 * If any of \ref icons_win, \ref stats_bar_win, \ref misc_win or \ref quickbar_win
 * is <= 0, no action will be performed.
 *
 * \callgraph
 */
void show_hud_windows (void);

/*!
 * \ingroup other
 * \brief Shows specifically hud windows that are relocatable and so may have been hidden when changing game/map/console modes.
 *
 * \callgraph
 */
void show_moveable_hud_windows(void);

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
 * \ingroup other
 * \brief Hide specifically hud windows that are currently relocated off the hud bars.
 *
 * \callgraph
 */
void hide_moved_hud_windows(void);

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
 * \ingroup hud
 * \brief check if mouse over.
 *
 * \retval	1, if mouse over relavant hud element
 * \callgraph
 */
int hud_mouse_over(window_info *win, int mx, int my);

/*!
 * \ingroup hud
 * \brief check if mouse click in hud
 *
 * \retval	1, if mouse click used
 * \callgraph
 */
int hud_click(window_info *win, int mx, int my, Uint32 flags);

/*!
 * \ingroup hud
 * \brief get the size of the hud logo, its square
 *
 * \retval	the size in pixels
 * \callgraph
 */
int get_hud_logo_size(void);

/*!
 * \ingroup windows
 * \brief Shows or hides the specified window
 *
 *      Shows the specified window by calling the appropriate display function if the window was not created before, else \ref toggle_window is called.
 *
 * \param managed_win   the window to show
 *
 * \pre If \a managed_win is either of MW_ITEMS, MW_SPELLS or MW_MANU window and the trade window is currently active, and error message will get logged to the console and the functions returns.
 * \callgraph
 */
void view_window(enum managed_window_enum managed_win);

/*!
 * \ingroup windows
 * \brief Shows the selected \a tab of the given \a managed_win.
 *
 *      Shows the selected \a tab of the given \a managed_win.
 *
 * \param managed_win    the window
 * \param col_id    the id of the tab collection
 * \param tab       the id of the tab to show
 *
 * \pre If \a managed_win is already visisble and \a tab is the currently selected tab, the window will be hidden.
 * \pre If \a managed_win is already visisble but \a tab is currently not selected, then \a tab will be selected.
 * \callgraph
 */
void view_tab (enum managed_window_enum managed_win, int col_id, int tab);

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
 * \brief   	The \#exp command, show current exp levels in console.
 * \retval	1, so command not passed to server
 */
int show_exp(char *text, int len);

#ifdef __cplusplus
} // extern "C"
#endif


#endif	//__HUD_H
