/*!
 * \file
 * \ingroup spells_window
 * \brief Handling of sigils and spells
 */
#ifndef __SPELLS_H__
#define __SPELLS_H__

#include <SDL_types.h>
#include "elwindows.h"
#include "hud_quickspells_window.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_ACTIVE_SPELLS 10

/*!
 * \name Server spell messages
 */
/*! @{ */
typedef enum {
	S_SUCCES = 1,
	S_FAILED,
	S_INVALID,
	S_SELECT_TARGET,
	S_SELECT_TELE_LOCATION,
	S_NAME,
} spell_errors;
/*! @} */


/*!
 * \name spells exported variables
 */
/*! @{ */
extern int start_mini_spells; /*!< do we start minimized? */
extern int sigils_text; /*!< texture for spell/sigil icons */
extern int spell_result; /*!< spell_errors type - server return status for last spell cast */
extern int show_poison_count; /*!< true if showing poison counts in UI */
/*! @} */


/*!
 * \ingroup spells_window
 * \brief Checks spells to determine if theyìre castable
 *
 *      Checks castability checking reagents, sigils, mana and levels
 *
 */
void check_castability(void);

/*!
 * \ingroup spells_window
 * \brief Repeats the last used spell.
 *
 *      Repeats the spell that was last used. This is initiated by pressing Ctrl+R. hotkey
 *
 */
void repeat_spell(void);

/*!
 * \ingroup other
 * \brief Initializes variables used in magic.
 *
 *      Initializes and sets up the sigils list and other variables used in spell casting.
 *
 */
int init_spells (void);

/*!
 * \ingroup spells_window
 * \brief Changes the active_spell at the given \a pos to the given \a spell.
 *
 *      Changes the active_spell at the given \a pos to the given \a spell.
 *
 * \param pos   index in the list of which spell to change
 * \param spell the index of the spell to use.
 */
void get_active_spell(int pos, int spell);

/*!
 * \ingroup spells_window
 * \brief Removes the spell at the given \a pos from the list of active spells
 *
 *      Removes the spell at the given \a pos from the list of active spells
 *
 * \param pos   the index into the array of the spell to remove
 */
void remove_active_spell(int pos);

/*!
 * \ingroup spells_window
 * \brief Sets the list of active_spells to the given list \a my_spell_list.
 *
 *      Sets the list of active_spells to the given list \a my_spell_list.
 *
 * \param my_spell_list the spell list to use for active_spells.
 */
void get_active_spell_list (const Uint8 *my_spell_list);

/*!
 * \ingroup spells_window
 * \brief Draws the sigils that the actor currently owns
 *
 *      Draws the sigils that the actor currently owns
 *
 * \callgraph
 */
void display_spells_we_have(void);

/*!
 * \ingroup spells_window
 * \brief Sets the sigils in sigil_list according the mask in \a sigils_we_have.
 *
 *      sets the sigils in sigil_list according the mask in \a sigils_we_have.
 *
 * \param sigils_we_have    the mask to determine the sigils which the actor has.
 */
void get_sigils_we_have(Uint32 sigils_we_have, Uint32 sigils2);

/*!
 * \ingroup spells_window
 * \brief Processes a message from the server about the last spell
 *
 * 	Processes a message from the server about the outcome of the last spell, or gets the name of a given spell.
 *
 * \param data The network data
 * \param len The data length
 */
void process_network_spell (const char * data, int len);

/*!
 * \ingroup other
 * \brief Send a spell message to the server
 *
 * 	The preprepared spell message is sent to the server and stored, as last_spell_str and last_spell_len.
 *
 * \param str the spell message
 * \param len the length of the spell message
 */
void send_spell(Uint8 *str, int len);

int we_are_poisoned(void);
void spell_text_from_server(const Uint8 *in_data, int data_length);
#ifdef NEW_SOUND
void restart_active_spell_sounds(void);
#endif
void increment_poison_incidence(void);
void draw_spell_icon_strings(window_info *win);
int command_show_spell(char *text, int len);
void draw_spell_icon(int id,int x_start, int y_start, int gridsize, int alpha, int grayed);
mqbdata* build_quickspell_data(const Uint32 spell_id);

//#define BUFF_DURATION_DEBUG
void here_is_a_buff_duration(Uint8 duration);
void check_then_do_buff_duration_request(void);
int command_buff_duration(char *text, int len);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
