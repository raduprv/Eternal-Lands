#include "global.h"
#include "timers.h"

#define	TIMER_RATE 20
int	my_timer_adjust=0;
int	my_timer_clock=0;
SDL_TimerID draw_scene_timer=0;
static Uint32 last_my_timer=0;
Uint32 my_timer(Uint32 interval, void * data)
{
	int	new_time;
	SDL_Event e;

	// adjust the timer clock
	if(my_timer_clock == 0)my_timer_clock=SDL_GetTicks();
	else my_timer_clock+=(TIMER_RATE-my_timer_adjust);

	last_my_timer=SDL_GetTicks();
	
	e.type = SDL_USEREVENT;
	e.user.code = EVENT_UPDATE_CAMERA;
	SDL_PushEvent(&e);

	//check the thunders
	thunder_control();

	if(is_raining)update_rain();
	if(normal_animation_timer>2)
		{
			if(my_timer_adjust > 0)my_timer_adjust--;
			normal_animation_timer=0;
    		update_particles();
    		next_command();

		e.type = SDL_USEREVENT;
		e.user.code = EVENT_ANIMATE_ACTORS;
		SDL_PushEvent(&e);
#ifdef CAL3D
			update_cal3d_model();
#endif
    		move_to_next_frame();
    		if(lake_waves_timer>2)
    		    {
    		        lake_waves_timer=0;
    		        make_lake_water_noise();
    		    }
    		lake_waves_timer++;
    		water_movement_u+=0.0004f;
    		water_movement_v+=0.0002f;
    		if(!dungeon && 0)//we do not want clouds movement in dungeons, but we want clouds detail
    			{
    				clouds_movement_u+=0.0003f;
    				clouds_movement_v+=0.0006f;
				}
		}
	normal_animation_timer++;

	// find the new interval
	new_time=TIMER_RATE-(SDL_GetTicks()-my_timer_clock);
	if(new_time<10)	new_time=10;	//put an absoute minimume in
    return new_time;
}


/*The misc timer is called approximately every half second - just put things 
that aren't too critical in here...*/

SDL_TimerID misc_timer=0;
static Uint32 misc_timer_clock=0;
Uint32 check_misc(Uint32 interval, void * data)
{
	misc_timer_clock=SDL_GetTicks();//This isn't accurate, but it's not needed here...
	
	//check the rain
	rain_control();
	
	//should we send the heart beat?
	if(last_heart_beat+25000<cur_time)
		{
			Uint8 command;
			last_heart_beat=cur_time;
			command=HEART_BEAT;
			my_tcp_send(my_socket,&command,1);
		}

	//AFK?
	if(afk_time)
		{
			if(cur_time-last_action_time>afk_time) 
				{
					if(!afk)
						{
							go_afk();
						}
				}
			else if(afk) go_ifk();
		}
	
	return 500;
}

//Checks if any of the timers have suddenly stopped
void check_timers()
{
	if((int)(cur_time-last_my_timer)>500)//OK, too long has passed, this is most likely a timer failure! log it and restart the timer
		{
			log_error("Draw scene timer is lagging behind!\n");
			log_to_console(c_red2,"The draw_scene timer is lagging behind or has stopped, restarting it.");
			SDL_RemoveTimer(draw_scene_timer);
			draw_scene_timer = SDL_AddTimer (1000/(18*4), my_timer, NULL);
			last_my_timer=my_timer_clock=SDL_GetTicks();
		}
	if((int)(cur_time-misc_timer_clock)>1500)
		{
			log_error("Misc timer is lagging behind!\n");
			log_to_console(c_red2,"The misc timer is lagging behind or has suddenly stopped, restarting it.");
			SDL_RemoveTimer(misc_timer);
			misc_timer = SDL_AddTimer (500, check_misc, NULL);
			misc_timer_clock=SDL_GetTicks();
		}
}

