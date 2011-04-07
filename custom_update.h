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
 * \brief	Starts the custom update
 *
 * 		The function starts the custom updates in a background thread.
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
 * \brief	Starts the update.
 *
 * 		Starts the update.
 */
int command_update(char *text, int len);

/*!
 * \ingroup	update
 * \brief	Prints to console the update state.
 *
 * 		Prints to the el console the current update status.
 */
int command_update_status(char *text, int len);

#ifdef __cplusplus
}
#endif

#endif	/* UUID_3d88ab37_44ba_4e1a_9b92_8caaedef1ca0 */

