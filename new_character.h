/*!
 * \file
 * \ingroup interface_newchar
 * \brief handles the interface to create a new character.
 */
#ifndef __NEW_CHARACTER_H__
#define __NEW_CHARACTER_H__

#ifdef __cplusplus
extern "C" {
#endif

#define NEW_CHARACTER_BASE_HUD_X 270
#define RAND(min,max) (min + rand () % (max - min + 1))

/*!
 * \name Windows handlers
 */
/*! @{ */
extern int newchar_root_win; /*!< window handler for the character creation interface root window. */
/*! @} */

/*!
 * \name Books
 */
/*! @{ */
extern int book_human;
extern int book_dwarf;
extern int book_elf;
extern int book_gnome;
extern int book_orchan;;
extern int book_draegoni;
/*! @} */

/*!
 * \ingroup interface_newchar
 * \brief   Sets the error string for errors during character creation.
 *
 *      If some errors occur in character creation interface, this functions sets the error string to be \a msg. This function will get called from \ref process_message_from_server whenever the server sends a \ref CREATE_CHAR_NOT_OK signal.
 *
 * \param msg   the string for the character creation error message
 * \param len   the length of \a msg
 * \callgraph
 */
void set_create_char_error (const char *msg, int len);

/*!
 * \ingroup interface_newchar
 * \brief   Verifies the username and password from the character creation interface and calls \ref send_login_info.
 *
 *      The username and password are verified and copied before \ref send_login_info is called.
 *
 * \callgraph
 */
void login_from_new_char();

/*!
 * \ingroup interface_newchar
 * \brief   Creates the character creation interface screen.
 *
 *      Creates the character creation interface root window, if it was not created earlier, and initializes the event handlers for this window.
 *
 * \callgraph
 *
 * \pre If the character creation interface root window was created before (\ref newchar_root_win < 0), this function won't perform any actions.
 */
void create_newchar_root_window (void);

/*!
 * \ingroup interface_newchar
 * \brief   Resize the new character window.
 *
 *      Destroys all the new charater window elements and recreates them.
 *
 * \callgraph
 */
void resize_newchar_hud_window(void);

/*!
 * \ingroup interface_newchar
 * \brief   Destroy all the new character windows.
 *
 *      Destroy all the new character windows.
 *
 * \callgraph
 */
void destroy_new_character_interface(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
