/*!
 * \file
 * \ingroup windows
 * \brief handling and displaying the HUD
 */
#ifndef	__HUD_H
#define	__HUD_H

#include "elwindows.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \name orientation constants
 */
/*! @{ */
#define HORIZONTAL 2
#define VERTICAL 1
/*! @} */

/*!
 * \name Quickbar defines
 */
/*! @{ */
#define MAX_QUICKBAR_SLOTS 12
/*! @} */

typedef enum
{
	HUD_INTERFACE_NEW_CHAR, /*!< the interface for the character creation screen */
	HUD_INTERFACE_GAME,     /*!< the interface for the game */
	HUD_INTERFACE_LAST      /*!< the last interface used */
} hud_interface;

extern int qb_action_mode; /*!< flag indicating whether we are in quickbar action mode or not */

extern int show_stats_in_hud;
extern int show_statbars_in_hud;
extern int show_action_bar; /*!< saved in the el.ini file, the action points stats bar is display when true */
extern int stats_bar_win; /*!< the window id for the stats bar of the bottom HUD */
extern int watch_this_stats[]; /*!< used for displaying more than 1 stat in the hud */
extern int max_food_level; /*!< normally 45 but can be set from options for people with diffent values (big belly) */

/*!
 * \name windows handlers
 */
/*! @{ */
extern int	quickbar_win; /*!< quickbar windows handler */
/*! @} */

extern int 	quickbar_relocatable; /*!< flag that indicates whether the quickbar is relocatable. */

/*!
 * \ingroup display_2d
 * \brief Initializes the quickbar
 *
 *      Initializes the quickbar, it's event handlers and shows it. If the quickbar has been moved by the player it will be drawn in its new position.
 */
void init_quickbar();

void switch_action_mode(int * mode, int id);

extern int hud_x;
extern int hud_y;

extern int view_analog_clock;
extern int view_digital_clock;
extern int view_knowledge_bar;
extern int view_hud_timer;

extern int quickbar_x;
extern int quickbar_y;
extern int quickbar_dir;
extern int quickbar_draggable;
extern int num_quickbar_slots;

extern int copy_next_LOCATE_ME;

// the main hud handling

/*!
 * \ingroup other
 * \brief Update the hud timer display
 *
 *       Expects a 500ms update interval.  Only call from main thread.
 *
 * \callgraph
*/
void update_hud_timer(void);

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
 * \brief Shows the different hud related windows if they have already been created.
 *
 *      Shows the different hud related windows, i.e. the icons, the stats bar, the miscellaneous (compass and clock) and the quickbar window if they have been created before. If none of them has been created nothing will be done.
 *
 * \pre If any of \ref icons_win, \ref stats_bar_win, \ref misc_win and \ref quickbar_win is <= 0, no action will be performed.
 *
 * \callgraph
 */
void show_hud_windows ();

/*!
 * \ingroup other
 * \brief Hides the different hud related windows, if they are visible.
 *
 *      Hides the different hud related windows, i.e. the icons, the stats bar, the miscellaneous (compass and clock) and the quickbar window if they are visible. If none of them is visible nothing will be done.
 *
 * \pre If none of \ref icons_win, \ref stats_bar_win, \ref misc_win and \ref quickbar_win is >= 0 (i.e. created before and visible) no action will be performed.
 * \callgraph
 */
void hide_hud_windows ();

/*!
 * \ingroup display_2d
 * \brief Draws the hud interface related items.
 *
 *      Draws the hud related items by setting the background color before calling \ref draw_hud_frame.
 *
 * \callgraph
 */
void draw_hud_interface();

/*!
 * \ingroup windows
 * \brief Checks whether a mouse click occurred in the hud.
 *
 *      Checks whether a mouse click occurred in the hud. Only used in non-standard (for example map) modes.
 *
 * \retval int  the return value of \ref click_in_windows
 * \callgraph
 */
int check_hud_interface();

/*!
 * \ingroup display_2d
 * \brief Draws the hud frame.
 *
 *      Draws the hud frame, by setting the texture, then draws the horizontal and vertical bar of the hud and finally the EL logo.
 *
 * \callgraph
 */
void draw_hud_frame();

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


//Functions for the function pointers

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
 * \brief   Views the console window (i.e. switch to console mode)
 *
 *      This is not handled by the window manager, so we have to call this function
 *
 * \param win   unused
 * \param id    unused
 *
 * \callgraph
 */
void view_console_win(int * win, int id);

/*!
 * \ingroup windows
 * \brief Views the map window (i.e. switch to map mode)
 *
 *      Shows or hides the map window depending on which mode is currently active.
 *
 * \param win   unused
 * \param id    unused
 *
 * \callgraph
 */
void view_map_win(int *win, int id);

/*!
 * \ingroup windows
 * \brief Shows the \a message at the given position (\a x, \a y).
 *
 *      Shows the \a message at the given position (\a x, \a y).
 *
 * \param message   the help message to show
 * \param x         the x coordinate of the position to draw the help message
 * \param y         the y coordinate of the position to draw the help message
 *
 * \callgraph
 */
void show_help(const char *message, int x, int y);

/*!
 * \ingroup windows
 * \brief Shows the \a message at the given position and colour (\a x, \a y).
 *
 *      Shows the \a message at the given position and colour (\a x, \a y).
 *
 * \param message   the help message to show
 * \param x         the x coordinate of the position to draw the help message
 * \param y         the y coordinate of the position to draw the help message
 * \param r         the red RGB value for text
 * \param g         the green RGB value for text
 * \param b         the blue RGB value for text
 *
 * \callgraph
 */
void show_help_coloured(const char *help_message, int x, int y, float r, float g, float b);

//stats/health section

/*!
 * \ingroup other
 * \brief   	Initialise the stat bars, (size, position and number), for in the bottom HUB.
 */
void init_stats_display(void);

/*!
 * \ingroup other
 * \brief Update displayed damage value.
 *
 *      The last damage is drawn as a hover over the health bar.
 *
 * \callgraph
 */
void set_last_damage(int quantity);


/*!
 * \ingroup other
 * \brief Update displayed heal value.
 *
 *      The last heal is drawn as a hover over the health bar.
 *
 * \callgraph
 */
void set_last_heal(int quantity);

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
 * \ingroup windows
 * \brief	Sets the flag of the given window
 *
 * 	Sets the flag of the given window
 *
 * \sa get_flags
 */
void change_flags(int win_id, Uint32 flags);

/*!
 * \ingroup windows
 * \brief Gets the flags of the given window 
 *
 * 	Gets the flag of the given window
 *
 * \sa change_flags
 */
Uint32 get_flags(int win_id);

/*!
 * \ingroup other
 * \brief   	The #exp command, show current exp levels in console.
 * \retval	1, so command not passed to server
 */
int show_exp(char *text, int len);

#ifdef __cplusplus
} // extern "C"
#endif

extern Uint32 exp_lev[200];

#endif	//__HUD_H
