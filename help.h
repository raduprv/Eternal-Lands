/*!
 * \file
 * \ingroup help_win
 * \brief handles the display of the help window
 */
#ifndef __HELP_H__
#define __HELP_H__

/*!
 * \names windows handlers
 */
/*! @{ */
extern int help_win; /*!< help window handler */
/*! @} */

/*
int help_menu_x=150;
int help_menu_y=70;
int help_menu_x_len=150;
int help_menu_y_len=200;
*/

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup event_handle
// * \brief   calls the display event handler for the given help window \a win.
// *
// *      Calls the display event handler for the given help window \a win.
// *
// * \param win       the \see window_info pointer that contains the definition for the help window
// * \retval int
// * \callgraph
// */
//int display_help_handler(window_info *win);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup event_handle
// * \brief   calls the click event handler for the given window \a win
// *
// *      Calls the click event handler for the given window \a win.
// *
// * \param win       the window_info pointer that contains the definition for the help window
// * \param mx        x position where the mouse click occurred
// * \param my        y position where the mouse click occurred
// * \param flags     mouseflags
// * \retval int
// * \sa display_help
// */
//int click_help_handler(window_info *win, int mx, int my, Uint32 flags);

/*!
 * \ingroup help_win
 * \brief       displays the help window
 *
 *      Displays the help window.
 *      \bug there's currently a bug associated with this, in that the client causes a segmentation fault, whenever the help icon is clicked.
 *
 * \callgraph
 */
void display_help();

/*!
 * \ingroup help_win
 * \brief Sets the window handler functions for the help window
 *
 *      Sets the window handler functions for the help window
 *
 * \return None
 */
void fill_help_win ();

#endif
