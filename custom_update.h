/*!
 * \file
 * \ingroup misc
 * \brief custom file update functions
 */
#ifndef UUID_3d88ab37_44ba_4e1a_9b92_8caaedef1ca0
#define UUID_3d88ab37_44ba_4e1a_9b92_8caaedef1ca0

#include <SDL.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 * \ingroup	update
 * \brief	Initialize the custom update
 *
 * 		The function initializes the custom updates threads.
 */
void init_custom_update();

/*!
 * \ingroup	update
 * \brief	Starts the custom update
 *
 * 		The function starts the custom updates in background threads.
 */
void start_custom_update();

/*!
 * \ingroup	update
 * \brief	Stopps the custom update
 *
 * 		The function stopps the custom updates and saves what is done
 *		till now.
 */
void stopp_custom_update();

/*!
 * \ingroup	update
 * \brief	Starts the updates.
 *
 * 		Starts the updates.
 */
int command_update(char *text, int len);

/*!
 * \ingroup	update
 * \brief	Prints to console the update status.
 *
 * 		Prints to the el console the current update status.
 */
int command_update_status(char *text, int len);

extern int custom_update;

#ifdef __cplusplus
}
#endif

#endif	/* UUID_3d88ab37_44ba_4e1a_9b92_8caaedef1ca0 */

