/*!
 * \file
 * \brief Handling of sigils and spells
 * \ingroup spells_win
 */
#ifndef __SPELLS_H__
#define __SPELLS_H__

#define SIGILS_NO 50 /*!< maximum number of sigils available to a player */

/*!
 * sigil_def is used to store items of a sigil like its image, name and description.
 */
typedef struct
{
	int sigil_img; /*!< a handle for the image to use with this sigil */
	char name[32]; /*!< the name of the sigil */
	char description[64]; /*!< description for a sigil */
	int have_sigil; /*!< flag, indicating whether a player has bought this sigil or not */
}sigil_def;

sigil_def sigils_list[SIGILS_NO]; /*!< global variable to store the data up to \see SIGILS_NO sigil */

extern Sint8 active_spells[10]; /*!< list of the 10 last used spells */

extern int sigil_win; /*!< handle for the sigil (spell) window */
extern int sigil_menu_x;
extern int sigil_menu_y;
extern int sigil_menu_x_len;
extern int sigil_menu_y_len;
//extern int sigil_menu_dragged;

extern int sigils_text;
extern Uint8 spell_text[256];
extern int sigils_we_have; /*!< the number of sigils the player has */
extern int have_error_message;

/*!
 * \ingroup spells_win
 * \brief Repeats the last used spell.
 *
 *      Repeats the spell that was last used. This is initiated by pressing Ctrl+R. \sa hotkey
 *
 * \return None
 */
void repeat_spell();

/*!
 * \ingroup other
 * \brief Initializes the \see sigil_list global variable.
 *
 *      Initializes and sets up the global variable \see sigil_list to its initial state.
 *
 * \return None
 */
void make_sigils_list();

/*!
 * \ingroup spells_win
 * \brief get_active_spell
 *
 *      get_active_spell(int,int)
 *
 * \param pos   Position
 * \param spell Spell
 * \return None
 */
void get_active_spell(int pos, int spell);

/*!
 * \ingroup spells_win
 * \brief remove_active_spell
 *
 *      remove_active_spell(int)
 *
 * \param pos   Position
 * \return None
 */
void remove_active_spell(int pos);

/*!
 * \ingroup spells_win
 * \brief get_active_spell_list
 *
 *      get_active_spell_list(Uint8*)
 *
 * \param my_spell_list current spell list
 * \return None
 */
void get_active_spell_list(Uint8 *my_spell_list);

/*!
 * \ingroup spells_win
 * \brief display_spells_we_have
 *
 *      display_spells_we_have()
 *
 * \return None
 */
void display_spells_we_have();

/*!
 * \ingroup spells_win
 * \brief display_sigils_menu
 *
 *      display_sigils_menu
 *
 * \return None
 */
void display_sigils_menu();

/*!
 * \ingroup spells_win
 * \brief get_sigils_we_have
 *
 *      get_sigils_we_have(Uint32)
 *
 * \param sigils_we_have
 * \return None
 */
void get_sigils_we_have(Uint32 sigils_we_have);
#endif

