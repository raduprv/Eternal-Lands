/*!
 * \file
 * \ingroup network_text
 * \brief handles displaying of different dialogues.
 */
#ifndef __DIALOGUES_H__
#define __DIALOGUES_H__

#include <SDL_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \name Response count
 */
#define MAX_RESPONSES 40 /*!< max. number of response entries in \see dialogue_responces */

extern unsigned char dialogue_string[2048]; /*!< buffer for strings in a dialogue */
extern unsigned char npc_name[20]; /*!< buffer for the NPCs name */
extern int cur_portrait; /*!< pointer to the portrait used by a particular NPC */
extern char npc_mark_str[20]; /*!< npc location in map mark - the template (print format) string used */

/*!
 * \name portrait textures
 */
/*! @{ */
#define	MAX_PORTRAITS_TEXTURES	16
extern int portraits_tex[MAX_PORTRAITS_TEXTURES];

extern int use_keypress_dialogue_boxes, use_full_dialogue_window;
/*! @} */

/*!
 * response structure used in dialogues with NPCs. It contains the data of a response from some NPC.
 */
typedef struct{
	char text[200]; /*!< text of the response */

    /*! \name response coordinates @{ */
	int x_start;
	int y_start;
	int x_len;
	int y_len;
	// orig_* is the unadulterated information from the server to save repeatedly recalculating
	int orig_x_start;
	int orig_y_start;
	int orig_x_len;
	int orig_y_len;
    /*! @} */

	int to_actor; /*!< id of the actor to which this response is directed */
	int response_id; /*!< unique id of the response */
	int in_use; /*!< flag whether this response is in use or not */
	int mouse_over; /*!< flag whether the mouse is over this response */
}response;

extern response dialogue_responces[MAX_RESPONSES];

/*! \name windows handlers 
 * @{ */
extern int dialogue_win; /*!< dialogue windows handler */
/*! @} */

extern int dialogue_menu_x;
extern int dialogue_menu_y;
extern int dialogue_menu_x_len;
extern int dialogue_menu_y_len;
//extern int dialogue_menu_dragged;

extern int no_bounding_box;
extern int autoclose_storage_dialogue;
extern int auto_select_storage_option;
extern int dialogue_copy_excludes_responses;
extern int dialogue_copy_excludes_newlines;

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
void display_dialogue();

/*!
 * \ingroup network_text
 * \brief       closes the current dialogue
 *
 *      Closes the current dialogue.
 *
 * \sa close_window
 */
void close_dialogue();

#ifdef __cplusplus
} // extern "C"
#endif

#endif
