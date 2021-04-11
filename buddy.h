/*!
 * \file
 * \ingroup	network_text
 * \brief	The implementation of the buddy list
 */
#ifndef __BUDDY_H__
#define __BUDDY_H__

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \ingroup	other
 * \brief	Initiates the buddy list
 *
 * 		Inititates the buddy list (sets all types to 255)
 *
 * \sa init_stuff
 */
void init_buddy(void);

/*!
 * \ingroup	network_text
 * \brief	Adds a buddy to the buddy list
 * 
 * 		Adds a buddy of the given type to the buddy list.
 *
 * \param	name The name of the buddy
 * \param	type The type of buddy
 * \param	len The length of the name
 *
 * \sa process_message_from_server
 */
void add_buddy (const char *name, int type, int len);

/*!
 * \ingroup	network_text
 * \brief	Removes a buddy from the network data
 * 
 * 		Removes a buddy from the buddy list.
 *
 * \param	name The name of the buddy to remove
 * \param	len The length of the name
 *
 * \sa process_message_from_server
 */
void del_buddy (const char *name, int len);

/*!
 * \ingroup	other
 * \brief	Clears the buddy list
 *
 * 		Clears the buddy list
 */
void clear_buddy(void);

void add_buddy_confirmation(char *name);

int is_in_buddylist(const char *name);

int accept_buddy_console_command(const char *name);

void destroy_buddy_queue(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
