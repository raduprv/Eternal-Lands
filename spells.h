/*!
 * \file
 * \ingroup spells_window
 * \brief Handling of sigils and spells
 */
#ifndef __SPELLS_H__
#define __SPELLS_H__

/*!
 * \name windows handlers
 */
/*! @{ */
extern int sigil_win; /*!< handle for the sigil (spell) window */
/*! @} */

extern int sigil_menu_x;
extern int sigil_menu_y;

extern int sigils_text; /*!< an index for the sigils text */
extern Uint8 spell_text[256]; /*!< buffer for the spell text */

extern int have_error_message; /*!< flag that indicates whether we got an error message */

/*!
 * \ingroup spells_window
 * \brief Repeats the last used spell.
 *
 *      Repeats the spell that was last used. This is initiated by pressing Ctrl+R. hotkey
 *
 */
void repeat_spell();

/*!
 * \ingroup other
 * \brief Initializes the sigil_list global variable.
 *
 *      Initializes and sets up the global variable sigil_list to its initial state.
 *
 */
void make_sigils_list();

/*!
 * \ingroup spells_window
 * \brief changes the active_spell at the given \a pos to the given \a spell.
 *
 *      Changes the active_spell at the given \a pos to the given \a spell.
 *
 * \param pos   index in the list of which spell to change
 * \param spell the index of the spell to use.
 */
void get_active_spell(int pos, int spell);

/*!
 * \ingroup spells_window
 * \brief removes the spell at the given \a pos from the list of active spells
 *
 *      Removes the spell at the given \a pos from the list of active spells
 *
 * \param pos   the index into the array of the spell to remove
 */
void remove_active_spell(int pos);

/*!
 * \ingroup spells_window
 * \brief sets the list of active_spells to the given list \a my_spell_list.
 *
 *      Sets the list of active_spells to the given list \a my_spell_list.
 *
 * \param my_spell_list the spell list to use for active_spells.
 */
void get_active_spell_list(Uint8 *my_spell_list);

/*!
 * \ingroup spells_window
 * \brief draws the sigils that the actor currently owns
 *
 *      Draws the sigils that the actor currently owns
 *
 * \callgraph
 */
void display_spells_we_have();

/*!
 * \ingroup spells_window
 * \brief displays the spells window
 *
 *      Displays the spells window
 *
 * \callgraph
 */
void display_sigils_menu();

/*!
 * \ingroup spells_window
 * \brief sets the sigils in sigil_list according the mask in \a sigils_we_have.
 *
 *      sets the sigils in sigil_list according the mask in \a sigils_we_have.
 *
 * \param sigils_we_have    the mask to determine the sigils which the actor has.
 */
void get_sigils_we_have(Uint32 sigils_we_have);
#endif
