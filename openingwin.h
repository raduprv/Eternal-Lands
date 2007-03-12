/*!
 * \file
 * \ingroup interface_opening
 * \brief   Function to handle the opening root window.
 */
#ifndef __OPENINGWIN_H__
#define __OPENINGWIN_H__

#ifdef __cplusplus
extern "C" {
#endif

extern int opening_root_win; /*!< handler for the opening window */

/*!
 * \ingroup interface_opening
 * \brief   Creates the root window for the opening interface
 *
 *      Creates a root windowm, if it hasn't created before, for the opening interface with the specified \a width and \a height and sets the window handlers for the display, keypress and click events.
 *
 * \param width     the width of the root window
 * \param height    the height of the root window.
 * \callgraph
 *
 * \pre If \ref opening_root_win is >=0, this function won't perform any actions. This means, the window has been created before.
 */
void create_opening_root_window (int width, int height);

/*!
 * \ingroup interface_opening
 * \brief Adjusts opening interface to new zoom value
 *
 *      Adjusts opening interface to new zoom value
 *
 * \param nlines
 * \callgraph
 */
void opening_win_update_zoom();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // def __OPENINGWIN_H__
