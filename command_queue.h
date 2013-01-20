#if !defined(COMMAND_QUEUE_H)
#define COMMAND_QUEUE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <SDL.h>


/*!
 * \ingroup command_queue
 * \brief Set the delay between commands from a single user menu line.
 *
 * \param wait_time_ms the time in milli-seconds.  If less than the mimimum, the minimum is used.
 */
void set_command_queue_wait_time_ms(Uint32 wait_time_ms);


#ifdef __cplusplus
}
#endif

#endif

