/*!
 * \file
 * \ingroup spells_win
 * \brief Handling of sigils and spells
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

/*!
 * \name windows handlers
 */
/*! @{ */
extern int sigil_win; /*!< handle for the sigil (spell) window */
/*! @} */

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
 * \brief changes the \see active_spell at the given \a pos to the given \a spell.
 *
 *      Changes the \see active_spell at the given \a pos to the given \a spell.
 *
 * \param pos   index in the list of which spell to change
 * \param spell the index of the spell to use.
 * \return None
 */
void get_active_spell(int pos, int spell);

/*!
 * \ingroup spells_win
 * \brief removes the spell at the given \a pos from the list of active spells
 *
 *      Removes the spell at the given \a pos from the list of active spells
 *
 * \param pos   the index into the array of the spell to remove
 * \return None
 */
void remove_active_spell(int pos);

/*!
 * \ingroup spells_win
 * \brief sets the list of \see active_spells to the given list \a my_spell_list.
 *
 *      Sets the list of \see active_spells to the given list \a my_spell_list.
 *
 * \param my_spell_list the spell list to use for \see active_spells.
 * \return None
 */
void get_active_spell_list(Uint8 *my_spell_list);

/*!
 * \ingroup spells_win
 * \brief draws the sigils that the actor currently owns
 *
 *      Draws the sigils that the actor currently owns
 *
 * \return None
 */
void display_spells_we_have();

/*!
 * \ingroup spells_win
 * \brief displays the spells window
 *
 *      Displays the spells window
 *
 * \return None
 */
void display_sigils_menu();

/*!
 * \ingroup spells_win
 * \brief sets the sigils in \see sigil_list according the mask in \a sigils_we_have.
 *
 *      sets the sigils in \see sigil_list according the mask in \a sigils_we_have.
 *
 * \param sigils_we_have    the mask to determine the sigils which the actor has.
 * \return None
 */
void get_sigils_we_have(Uint32 sigils_we_have);
#endif
