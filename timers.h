/*!
 * \file
 * \ingroup thread
 */
#ifndef __TIMERS_H__
#define __TIMERS_H__

extern int	my_timer_adjust;             /*!< my_timer_adjust */
extern int	my_timer_clock;              /*!< my_timer_clock */
extern SDL_TimerID draw_scene_timer;     /*!< draw_scene_timer */

extern SDL_TimerID misc_timer;           /*!< misc_timer */

/*!
 * \ingroup thread
 * \brief my_timer
 *
 *      TODO: my_timer
 *
 * \param   some_int
 * \param   data
 * \return  Uint32
 */
Uint32 my_timer(Uint32 some_int, void * data);

/*!
 * \ingroup thread
 * \brief check_misc
 *
 *      TODO: check_misc
 *
 * \param   interval
 * \param   data
 * \return  Uint32
 */
Uint32 check_misc(Uint32 interval, void * data);
#ifdef TIMER_CHECK
/*!
 * \ingroup thread
 * \brief check_timers
 *
 *      TODO: check_timers
 *
 * \param   None
 * \return  None
 */
void check_timers(void);
#endif

#endif
