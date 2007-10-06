/*!
 * \file
 * \ingroup interface_login
 * \brief   Functions to handle the login.
 */
#ifndef __LOGINWIN_H__
#define __LOGINWIN_H__

#ifdef __cplusplus
extern "C" {
#endif

extern int login_root_win; /*!< ID for the login root window */
extern int login_text; /*!< ID for the background texture */

/*!
 * \ingroup interface_login
 * \brief   Loads the textures for the opening interface.
 *
 *      Loads the textures for the opening interface by calling \ref load_texture_cache with the appropriate bitmaps.
 *
 * \callgraph
 */
void init_login_screen ();

/*!
 * \ingroup interface_login
 * \brief   Sets the error string used when a login error occurs.
 *
 *      Sets the error string used when a login error occurs to be \a msg.
 *
 * \param msg       the message for the login error
 * \param len       the length of \a msg
 * \param print_err if non-zero, prefix the message with an error string
 * \callgraph
 */
void set_login_error (const char *msg, int len, int print_err);

/*!
 * \ingroup interface_login
 * \brief   Creates the root window for the login interface.
 *
 *      Creates the root window for the login interface using the given \a width and \a height, if it was not created before. This functions also sets the event handlers for the \ref ELW_HANDLER_DISPLAY, \ref ELW_HANDLER_MOUSEOVER, \ref ELW_HANDLER_CLICK, \ref ELW_HANDLER_KEYPRESS and \ref ELW_HANDLER_RESIZE events.
 *
 * \param width     the width of the login window
 * \param height    the height of the login window
 * \callgraph
 *
 * \pre If \ref login_root_win >= 0, this function won't perform any action.
 */
void create_login_root_window (int width, int height);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // def __LOGINWIN_H__
