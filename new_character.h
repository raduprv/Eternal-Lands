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

#define RAND(min,max) (min + rand () % (max - min + 1))

/*!
 * \name Windows handlers
 */
/*! @{ */
extern int namepass_win;/*! The name & password window*/
extern int color_race_win;/*! The color and race window*/
extern int newchar_advice_win;/*! The char creaion warning warning window*/
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
 * \brief   Initializes the actor after changes
 *
 *      Initializes the actors data to reflect changes. Called from \ref click_newchar_handler. The current actor (if any) will get destroyed before the changes.
 *
 * \callgraph
 */
void change_actor();

/*!
 * \ingroup interface_newchar
 * \brief   Initializes and draws the New Character screen.
 *
 *      Initializes and draws the screen to create new characters.
 *
 * \callgraph
 */
void draw_new_char_screen();

/*!
 * \ingroup interface_newchar
 * \brief   Adds the char \a ch to the selected input box in the character creation interface.
 *
 *      The char \a ch will get added to either of the username, password or password confirmation input fields, depending on which input field is currently selected.
 *
 * \param ch    the char to add
 *
 * \callgraph
 */
void add_char_to_new_character(unsigned char ch);

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
void create_newchar_root_window ();


#ifdef NEW_NEW_CHAR_WINDOW
void resize_newchar_hud_window();
#else
void show_account_win ();
void show_color_race_win();
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif
