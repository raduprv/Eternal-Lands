/*!
 * \file
 * \ingroup network_text
 * \brief handles displaying of different dialogues.
 */
#ifndef __DIALOGUES_H__
#define __DIALOGUES_H__

/*!
 * \name Response count
 */
#define MAX_RESPONSES 40 /*!< max. number of response entries in \see dialogue_responces */

extern char dialogue_string[2048]; /*!< buffer for strings in a dialogue */
extern char npc_name[20]; /*!< buffer for the NPCs name */
extern int cur_portrait; /*!< pointer to the portrait used by a particular NPC */

/*!
 * \name portrait textures
 */
/*! @{ */
extern int portraits1_tex;
extern int portraits2_tex;
extern int portraits3_tex;
extern int portraits4_tex;
extern int portraits5_tex;
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

/*!
 * \ingroup other
 * \brief   builds the response entries for the given \a data.
 *
 *      Builds the response entries for the given \a data.
 *
 * \param data
 * \param total_lenght
 */
void build_response_entries(Uint8 *data,int total_lenght);

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

#endif
