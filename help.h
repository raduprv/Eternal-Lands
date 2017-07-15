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
 * \ingroup help_window
 * \brief Sets the window handler functions for the help window
 *
 *      Sets the window handler functions for the help window
 *
 * \param window_id  the id of the help window.
 * 
 * \return None
 */
void fill_help_win (int window_id);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
