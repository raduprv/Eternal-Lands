/*!
 * \file
 * \ingroup	network_text
 * \brief	The implementation of the buddy list
 */
#ifndef __BUDDY_H__
#define __BUDDY_H__
#include "elwindows.h"

/*!
 * The buddy structure containing the name and the type.
 */
typedef struct
{
   char name[32]; /*!< name of your buddy */
   unsigned char type;
}_buddy;
#define	MAX_BUDDY	100 /*!< Maximum number of buddies on the buddy list*/

extern int buddy_menu_x; /*!< The default x position of the buddy window*/
extern int buddy_menu_y; /*!< The default y position of the buddy window*/
extern int buddy_menu_x_len; /*!< The buddy window width*/
extern int buddy_menu_y_len; /*!< The buddy window height*/

/*!
 * \name windows handlers
 */
/*! @{ */
extern int buddy_win; /*!< The identifier of the buddy window */
/*! @} */

/*!
 * \ingroup	other
 * \brief	Initiates the buddy list
 *
 * 		Inititates the buddy list (sets all types to 255)
 *
 * \sa init_stuff
 */
void init_buddy();

/*!
 * \ingroup	buddy_win
 * \brief	Compares 2 buddies and specifies which one should be on top of the other
 *
 * 		Compares 2 buddies and specifies which one should be on top of the other - is used for sorting the buddy list using qsort.
 *
 * \param	arg1 The first buddy*
 * \param	arg2 The second buddy*
 * \retval int	Returns either a negative or positive integer depending on whether arg1 or arg2 is to be placed higher in the array that's being sorted.
 * \sa display_buddy_handler
 */
int compare2( const void *arg1, const void *arg2);

/*!
 * \ingroup	buddy_win
 * \brief	The callback for handling mouseclicks in the buddy-list
 *
 * 		The callback for handling mouseclicks in the buddy-list - will display a buddy's name at the bottom of the screen (/\<name\> ) ready for writing him/her a message.
 * 
 * \param	win The window that called the function
 * \param	mx The mouse' x position
 * \param	my The mouse' y position
 * \param	flags The flags
 * \retval int	Returns true
 * \callgraph
 */
int click_buddy_handler(window_info *win, int mx, int my, Uint32 flags);

/*!
 * \ingroup	buddy_win
 * \brief	Draws the buddy list
 *
 * 		This is the main function for drawing the buddy list window
 *
 * \param	win The window that's being drawn
 * \retval int	Returns true
 * \callgraph
 */
int display_buddy_handler(window_info *win);

/*!
 * \ingroup	buddy_win
 * \brief	The drag callback for the buddy window
 *
 * 		The drag callback for the buddy window - handles dragging the scrollbar etc.
 *
 * \param	win The window that called the function
 * \param	mx The mouse' x position
 * \param	my The mouse' y position
 * \param	flags The flags
 * \param	dx The delta x
 * \param	dy The delta y
 * \retval int	Returns 1 if the mouse is being dragged within the scrollbar, otherwise 0.
 * \sa display_buddy
 */
int drag_buddy_handler(window_info *win, int mx, int my, Uint32 flags, int dx, int dy);
	
/*!
 * \ingroup	buddy_win
 * \brief	Initiates the buddy window
 *
 * 		The function is used for initiating the buddy window or setting an existing buddy window to be displayed.
 *
 * \callgraph
 */
void display_buddy();

/*!
 * \ingroup	network_text
 * \brief	Adds a buddy to the buddy list
 * 
 * 		Adds a buddy of the given type to the buddy list.
 *
 * \param	n The name of the buddy
 * \param	t The type of buddy
 * \param	len The length of the name
 *
 * \sa process_message_from_server
 */
void add_buddy(char *n, int t, int len);

/*!
 * \ingroup	network_text
 * \brief	Removes a buddy from the network data
 * 
 * 		Removes a buddy from the buddy list.
 *
 * \param	n The name of the buddy to remove
 * \param	len The length of the name
 *
 * \sa process_message_from_server
 */
void del_buddy(char *n, int len);
#endif
