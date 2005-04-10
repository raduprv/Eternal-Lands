/*!
 * \file
 * \ingroup help_window
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

/*!
 * \ingroup help_window
 * \brief       displays the help window
 *
 *      Displays the help window.
 *      \bug there's currently a bug associated with this, in that the client causes a segmentation fault, whenever the help icon is clicked.
 *
 * \callgraph
 */
void display_help();

/*!
 * \ingroup help_window
 * \brief Sets the window handler functions for the help window
 *
 *      Sets the window handler functions for the help window
 *
 * \return None
 */
void fill_help_win ();

#endif
