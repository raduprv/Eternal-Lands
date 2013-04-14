#include <stdlib.h>
#include <time.h>
#include "timers.h"
#include "actors.h"
#include "actor_scripts.h"
#include "draw_scene.h"
#include "events.h"
#include "global.h"
#include "init.h"
#include "interface.h"
#include "map.h"
#include "multiplayer.h"
#include "pm_log.h"
#include "reflection.h"
#include "rules.h"
#include "update.h"
#include "weather.h"
#ifdef NEW_SOUND
#include "items.h"
#include "sound.h"
#endif
#ifdef PAWN
#include "pawn/elpawn.h"
#endif
#ifdef TIMER_CHECK
#include "asc.h"
#include "errors.h"
#include "translate.h"
#endif

#define	TIMER_RATE 20
int	my_timer_adjust=0;
int	my_timer_clock=0;
SDL_TimerID draw_scene_timer=0;
int normal_animation_timer=0;

#ifdef TIMER_CHECK
static Uint32 last_my_timer=0;
#endif

Uint32 my_timer(Uint32 interval, void * data)
{
	int	new_time;
	SDL_Event e;
	static int normal_animation_loop_count = 2;
	
#ifdef NEW_SOUND
	update_sound(interval);
	update_item_sound(interval);
#endif	//NEW_SOUND

	// adjust the timer clock
	if(my_timer_clock == 0)
	{
		my_timer_clock=SDL_GetTicks();
	}
	else
	{
		my_timer_clock+=(TIMER_RATE-my_timer_adjust);
	}

#ifdef TIMER_CHECK
	last_my_timer=SDL_GetTicks();
#endif
	
	e.type = SDL_USEREVENT;


	if(normal_animation_timer>normal_animation_loop_count && have_a_map)
	{
		if(my_timer_adjust > 0)
		{
			my_timer_adjust--;
		}
		normal_animation_timer=0;
		//update_particles();
		//e.type= SDL_USEREVENT;
		e.user.code= EVENT_UPDATE_PARTICLES;
		SDL_PushEvent(&e);

		water_movement_u+=0.0004f;
		water_movement_v+=0.0002f;
		if(!dungeon && 0)//we do not want clouds movement in dungeons, but we want clouds detail
		{
			clouds_movement_u+=0.0003f;
			clouds_movement_v+=0.0006f;
		}
	}
	normal_animation_timer++;

#ifdef PAWN
	check_pawn_timers ();
#endif

	// find the new interval
	new_time= TIMER_RATE-(SDL_GetTicks()-my_timer_clock);
	if(new_time<10) {
		new_time=10;	//put an absoute minimume in
	}
	
	
	/* Temporary code to test a fix for a client freeze due to too many particles:
	   http://www.eternal-lands.com/forum/index.php?showtopic=47006
	   in_main_event_loop is set true while the main loop is handling SDL events
	   normal_animation_loop_count will be increased when we check and that is the
	   case.  Increasing the count causes EVENT_UPDATE_PARTICLES to be trigger
	   less often an gives the system a chance to catch up.  When in_main_event_loop
	   is seen to be zero, normal_animation_loop_count is decrement down to standard
	   value.  This is very crude but stops the client freeze on my system.
	*/
	{
		extern volatile int in_main_event_loop;
		
		if (in_main_event_loop)
		{
			normal_animation_loop_count++;
			//printf("increase new_time=%i loop_count=%i\n", new_time, normal_animation_loop_count);
		}
		else if (normal_animation_loop_count > 2)
		{
		   	normal_animation_loop_count--;
			//printf("decrease new_time=%i loop_count=%i\n", new_time, normal_animation_loop_count);
		}
	}
	
	
	return new_time;
}


/*The misc timer is called approximately every half second - just put things 
that aren't too critical in here...*/

SDL_TimerID misc_timer=0;
#ifdef TIMER_CHECK
static Uint32 misc_timer_clock=0;
#endif
Uint32 check_misc(Uint32 interval, void * data)
{
#ifdef TIMER_CHECK
	misc_timer_clock=SDL_GetTicks();//This isn't accurate, but it's not needed here...
#endif

	//should we send the heart beat?
	if(!disconnected && last_heart_beat+25 <= time(NULL))
	{
		send_heart_beat();
	}

	if(countdown>0)
	{
		countdown --;
	}
	if (update_countdown > 0)
	{
		update_countdown--;
	}
	return 500;
}

#ifdef TIMER_CHECK
//Checks if any of the timers have suddenly stopped
void check_timers()
{
	if((int)(cur_time-last_my_timer)>10000)//OK, too long has passed, this is most likely a timer failure! log it and restart the timer
	{
		char str[120];
		safe_snprintf(str, sizeof(str), timer_lagging_behind, "draw_scene");
		LOG_ERROR(str);
		LOG_TO_CONSOLE(c_red2,str);
		SDL_RemoveTimer(draw_scene_timer);
		my_timer_clock=0;
		draw_scene_timer = SDL_AddTimer (1000/(18*4), my_timer, NULL);
		last_my_timer=SDL_GetTicks();
	}
	if((int)(cur_time-misc_timer_clock)>10000)
	{
		char str[120];
		safe_snprintf(str, sizeof(str), timer_lagging_behind, "misc");
		LOG_ERROR(str);
		LOG_TO_CONSOLE(c_red2,str);
		SDL_RemoveTimer(misc_timer);
		misc_timer = SDL_AddTimer (500, check_misc, NULL);
		misc_timer_clock=SDL_GetTicks();
	}
}
#endif
