#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef WINDOWS
#include <windows.h>

#endif

#include "global.h"
#include "init.h"

Uint32 cur_time=0, last_time=0;//for FPS

char version_string[]=VER_STRING;
int	client_version_major=VER_MAJOR;
int client_version_minor=VER_MINOR;
int client_version_release=VER_RELEASE;
int	client_version_patch=VER_BUILD;
int version_first_digit=9;	//protocol/game version sent to server
int version_second_digit=9;

/**********************************************************************/

int start_rendering()
{
    int done=0;
	SDL_Thread *music_thread=SDL_CreateThread(update_music, 0);
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

			if(!limit_fps || ((cur_time-last_time) && (800/(cur_time-last_time) < limit_fps)))
				{
					//draw everything
					draw_scene();
					last_time=cur_time;
				}
			//else
				SDL_Delay(1);//give up timeslice for anyone else
#ifdef	CACHE_SYSTEM
			//cache handling
			if(cache_system)cache_system_maint();
#endif	//CACHE_SYSTEM
			//see if we need to exit
			if(exit_now)break;
		}
	have_music=0;
	SDL_WaitThread(music_thread,&done);
	save_bin_cfg();
	unload_questlog();
	/* Destroy our GL context, etc. */
	destroy_sound();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	SDL_QuitSubSystem(SDL_INIT_TIMER);
	SDL_Quit( );
	xmlCleanupParser();
	FreeXML();
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
