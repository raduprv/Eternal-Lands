/*!
 * \file
 * \ingroup network_text
 * \brief handles displaying of different dialogues.
 */
#ifndef __DIALOGUES_H__
#define __DIALOGUES_H__

#include <SDL_types.h>
#include "loginwin.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NPC_NAME_BUF_LEN (MAX_USERNAME_LENGTH + 10)
extern unsigned char npc_name[NPC_NAME_BUF_LEN]; /*!< buffer for the NPCs name */
extern int cur_portrait; /*!< pointer to the portrait used by a particular NPC */
extern char npc_mark_str[NPC_NAME_BUF_LEN]; /*!< npc location in map mark - the template (print format) string used */

extern int autoclose_storage_dialogue;
extern int auto_select_storage_option;
extern int dialogue_copy_excludes_responses;
extern int dialogue_copy_excludes_newlines;
extern int use_keypress_dialogue_boxes;
extern int use_full_dialogue_window;

/*!
 * \ingroup other
 * \brief   Load the portrait textures used by dialogues.
 *
 *      Load the portrait textures used by dialogues.
 */
void load_dialogue_portraits(void);

/*!
 * \ingroup other
 * \brief   Clears any previous dialogue responses.
 *
 *      Clears any previous dialogue responses.
 */
void clear_dialogue_responses(void);

/*!
 * \ingroup other
 * \brief   builds the response entries for the given \a data.
 *
 *      Builds the response entries for the given \a data.
 *
 * \param data
 * \param total_length
 */
void build_response_entries (const Uint8 *data,int total_length);

/*!
 * \ingroup network_text
 * \brief       displays the current dialogue
 *
 *      Displays the current dialogue.
 *
 * \callgraph
 */
void display_dialogue(const Uint8 *in_data, int data_length);

/*!
 * \ingroup network_text
 * \brief       closes the current dialogue
 *
 *      Closes the current dialogue.
 *
 * \sa close_window
 */
void close_dialogue(void);

/*!
 * \brief       Frees memory allocated for dialogue functions
 *
 */
void cleanup_dialogues(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
