/*!
 * \file
 * \ingroup interface_newchar
 * \brief handles the interface to create a new character.
 */
#ifndef __NEW_CHARACTER_H__
#define __NEW_CHARACTER_H__

/*!
 * \ingroup interface_newchar
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void change_actor();

/*!
 * \ingroup interface_newchar
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void check_for_input();

/*!
 * \ingroup interface_newchar
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void draw_new_char_screen();

/*!
 * \ingroup interface_newchar
 * \brief
 *
 *      Detail
 *
 * \param ch
 *
 * \sa add_char_to_new_character
 */
void add_char_2_pass(unsigned char ch);

/*!
 * \ingroup interface_newchar
 * \brief
 *
 *      Detail
 *
 * \param ch
 *
 * \sa add_char_to_new_character
 */
void add_char_2_un(unsigned char ch);

/*!
 * \ingroup interface_newchar
 * \brief
 *
 *      Detail
 *
 * \param ch
 *
 * \sa add_char_to_new_character
 */
void add_char_2_conf(unsigned char ch);

/*!
 * \ingroup interface_newchar
 * \brief
 *
 *      Detail
 *
 * \param ch
 *
 * \callgraph
 */
void add_char_to_new_character(unsigned char ch);

/*!
 * \ingroup interface_newchar
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void login_from_new_char();

#ifndef OLD_EVENT_HANDLER

extern int newchar_root_win;

/*!
 * \ingroup interface_newchar
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void create_newchar_root_window ();

#endif // not def OLD_EVENT_HANDLER

#endif
