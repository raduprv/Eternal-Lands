#ifndef __TIMERS_H__
#define __TIMERS_H__

extern int	my_timer_adjust;
extern int	my_timer_clock;
extern SDL_TimerID draw_scene_timer;

extern SDL_TimerID misc_timer;
extern Uint32 misc_timer_clock;

Uint32 my_timer(Uint32 some_int, void * data);
Uint32 check_misc(Uint32 interval, void * data);
void check_timers(void);

#endif
