/*!
 * \file
 * \ingroup help_window
 * \brief handles the display of the help window
 */
#ifndef __HELP_H__
#define __HELP_H__

#ifdef __cplusplus
extern "C" {
#endif

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
 * \brief Sets the window handler functions for the help window
 *
 *      Sets the window handler functions for the help window
 *
 * \return None
 */
void fill_help_win ();

#ifdef __cplusplus
} // extern "C"
#endif

#endif
