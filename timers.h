/*!
 * \file
 * \ingroup thread
 */
#ifndef __TIMERS_H__
#define __TIMERS_H__

extern int	my_timer_adjust;
extern int	my_timer_clock;
extern SDL_TimerID draw_scene_timer;

extern SDL_TimerID misc_timer;

Uint32 my_timer(Uint32 some_int, void * data);
Uint32 check_misc(Uint32 interval, void * data);
#ifdef TIMER_CHECK
void check_timers(void);
#endif

#endif
