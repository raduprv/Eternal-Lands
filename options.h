/*!
 * \file
 * \ingroup options_window
 * \brief handles the display of the options window.
 */
#ifndef __OPTIONS_H__
#define __OPTIONS_H__

/*!
 * \name Option Modes
 */
/*! @{ */
#define NONE 0		//0000b
#define OPTION 1	//0001b
#define VIDEO_MODE 2 	//0010b
/*! @} */

/*!
 * \name windows handlers
 */
/*! @{ */
extern int options_win; /*!< options windows handler */
/*! @} */

extern int options_menu_x;
extern int options_menu_y;
extern int options_menu_x_len;
extern int options_menu_y_len;
//extern int options_menu_dragged; // has been commented before, IMO safe for removal.

/*!
 * \ingroup options_window
 * \brief Displays the options window.
 * 
 *      Displays the options window.
 *
 * \callgraph
 */
void display_options_menu();

#endif
