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
