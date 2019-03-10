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

// How long a username is allowed to be. This define allows for the trailing NULL
#define MAX_USERNAME_LENGTH (15 + 1)

extern int login_root_win; /*!< ID for the login root window */
extern int login_text; /*!< ID for the background texture */
extern char active_username_str[MAX_USERNAME_LENGTH]; /*!< the username of the actor */
extern char active_password_str[MAX_USERNAME_LENGTH]; /*!< the password of the actor */

#define VALID_PASSWORD_CHAR(ch) (ch>=33 && ch<126)

/*!
 * \name Getters and setters for current username and password.
 */
/*! @{ */
const char * get_username(void);
const char * get_lowercase_username(void);
const char * get_password(void);
void set_username(const char * new_username);
void set_password(const char * new_password);
int valid_username_pasword(void);
/*! @} */

/*!
 * \ingroup interface_login
 * \brief   Loads the textures for the opening interface.
 *
 *      Loads the textures for the opening interface by calling \ref load_texture_cache with the appropriate bitmaps.
 *
 * \callgraph
 */
void init_login_screen (void);

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
