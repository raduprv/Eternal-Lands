#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef WINDOWS
#include <windows.h>

#endif

#include "global.h"
#include "init.h"

/**********************************************************************/

int start_rendering()
{
    int done=0;
#ifndef WINDOWS
	SDL_EventState(SDL_SYSWMEVENT,SDL_ENABLE);
#endif
	/* Loop until done. */
	while( !done )
		{
			SDL_Event event;

			// handle SDL events
			while( SDL_PollEvent( &event ) )
				{
					done = HandleEvent(&event);
				}
			//advance the clock
			last_time=cur_time;
			cur_time = SDL_GetTicks();
			//check for network data
			get_message_from_server();

			//should we send the heart beat?
			if(last_heart_beat+25000<cur_time)
				{
					Uint8 command;
					last_heart_beat=cur_time;
					command=HEART_BEAT;
					my_tcp_send(my_socket,&command,1);
				}
			//draw everything
			draw_scene();
			//see if we need to exit
			if(exit_now)break;
		}

	save_bin_cfg();
	/* Destroy our GL context, etc. */
	destroy_sound();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	SDL_QuitSubSystem(SDL_INIT_TIMER);
	SDL_Quit( );
	return(0);
}

#ifdef WINDOWS
Main()
#else
	 int main()
#endif
{
	int logo;
	int numtests;
	int slowly;

	logo = 1;
	slowly = 1;
	numtests = 1;

    init_stuff();
    start_rendering();

	return 0;
}

#ifdef WINDOWS
int APIENTRY WinMain (HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow)
{
	Main();
	return 0;
}

#endif
