/*!
 * \file
 * \ingroup 	thread
 * \brief	Contains the timers used in EL
 */
#ifndef __TIMERS_H__
#define __TIMERS_H__

#include <SDL_timer.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int	my_timer_adjust;             /*!< my_timer_adjust */

extern SDL_TimerID draw_scene_timer;     /*!< draw_scene_timer */
extern SDL_TimerID misc_timer;           /*!< misc_timer */

/*!
 * \ingroup 	thread
 * \brief 	The main timer - handles animations and such.
 *
 *      	This is the main timer thread. It handles animations, lake texture effects and weather effects (such as rain). It's normally called every ~14 ms
 *
 * \param   	some_int The last delay
 * \param   	data A void* to data, not used currently
 * \retval Uint32  	Uint32 The delay before it's called the next time
 * \callgraph
 */
Uint32 my_timer(Uint32 some_int, void * data);

/*!
 * \ingroup 	thread
 * \brief 	Checks miscellaneous things such as the AFK time and makes sure that the client stays connected.
 *
 *      	The timer checks miscellaneous things such as the AFK time, and when the last message was send to the server (if >30s the client will be disconnected, so the thread will send a message saying the client is alive every 25th seconds (HEART_BEAT)).
 *
 * \param   	interval The time interval
 * \param   	data A void* to some data. It is not currently used.
 * \retval Uint32  	Uint32 The next interval
 * \callgraph
 */
Uint32 check_misc(Uint32 interval, void * data);
#ifdef TIMER_CHECK
/*!
 * \ingroup 	thread
 * \brief 	If TIMER_CHECK is defined this will make sure that the timers are alive.
 *
 *      	If TIMER_CHECK is defined this will make sure that the timers are alive, but comparing the last time the timers were called with the current time. It is not normally a problem.
 *
 * \callgraph
 */
void check_timers(void);
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif
