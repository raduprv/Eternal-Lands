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
 * \return None
 */
void change_actor();

/*!
 * \ingroup interface_newchar
 * \brief
 *
 *      Detail
 *
 * \return None
 */
void check_for_input();

/*!
 * \ingroup interface_newchar
 * \brief
 *
 *      Detail
 *
 * \return None
 */
void draw_new_char_screen();

/*!
 * \ingroup interface_newchar
 * \brief
 *
 *      Detail
 *
 * \param ch
 * \return None
 */
void add_char_2_pass(unsigned char ch);

/*!
 * \ingroup interface_newchar
 * \brief
 *
 *      Detail
 *
 * \param ch
 * \return None
 */
void add_char_2_un(unsigned char ch);

/*!
 * \ingroup interface_newchar
 * \brief
 *
 *      Detail
 *
 * \param ch
 * \return None
 */
void add_char_2_conf(unsigned char ch);

/*!
 * \ingroup interface_newchar
 * \brief
 *
 *      Detail
 *
 * \param ch
 * \return None
 */
void add_char_to_new_character(unsigned char ch);

/*!
 * \ingroup interface_newchar
 * \brief
 *
 *      Detail
 *
 * \return None
 */
void login_from_new_char();

#endif
