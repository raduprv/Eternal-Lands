#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef	__GNUC__
#include <unistd.h>
#endif

#ifdef WINDOWS
#include <windows.h>
#undef WRITE_XML
#endif

#include "global.h"
#include "init.h"

Uint32 cur_time=0, last_time=0;//for FPS

char version_string[]=VER_STRING;
int	client_version_major=VER_MAJOR;
int client_version_minor=VER_MINOR;
int client_version_release=VER_RELEASE;
int	client_version_patch=VER_BUILD;
int version_first_digit=10;	//protocol/game version sent to server
int version_second_digit=9;

int gargc;
char **  gargv;
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

			if(!limit_fps || ((cur_time-last_time) && (800/(cur_time-last_time) < limit_fps)))
				{
					//draw everything
					draw_scene();
					last_time=cur_time;
				}
			else
				SDL_Delay(1);//give up timeslice for anyone else

			//Check the timers to make sure that they are all still alive...
			check_timers();
			
			//cache handling
			if(cache_system)cache_system_maint();
			//see if we need to exit
			if(exit_now)break;
		}
	have_music=0;
	SDL_WaitThread(music_thread,&done);
	if(pm_log.ppl)free_pm_log();
	save_bin_cfg();
	unload_questlog();
	free_icons();
	free_vars();
	unload_e3d_list();	// do we really want to overwrite this file??
	SDL_RemoveTimer(draw_scene_timer);
	SDL_RemoveTimer(misc_timer);
	end_particles_list();
#ifdef CAL3D
	destroy_cal3d_model();
#endif
	/* Destroy our GL context, etc. */
	destroy_sound();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	SDL_QuitSubSystem(SDL_INIT_TIMER);
	SDL_Quit( );
	xmlCleanupParser();
	FreeXML();
	return(0);
}

extern char *optarg;
extern int optind, opterr, optopt;

void	read_command_line()
{
	int i=1;
	if(gargc<2)return;
	for(;i<gargc;i++)
		{
			if(gargv[i][0]=='-')
				{
					if(gargv[i][1]=='-')check_var(gargv[i]+2,1);
					else 
						{
							char str[200];
							snprintf(str,198,"%s %s",gargv[i],gargv[i+1]);
							check_var(str+1,0);
						}
				}
		}
}

#ifdef WINDOWS
int Main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
	gargc=argc;
	gargv=argv;
	
	// do basic initialization
	init_vars();
	init_stuff();

	start_rendering();

	return 0;
}

#ifdef WINDOWS
int APIENTRY WinMain (HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow)
{
	LPWSTR *argv;
	int	argc, i;
	char **targv;

	argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	targv=(char**)malloc(sizeof(char*)*argc);

	for(i=0;i<argc;i++){ //converting unicodes to char
		targv[i]=(char*)calloc(sizeof(char), wcslen(argv[i])+1);
		WideCharToMultiByte(CP_ACP, 0, argv[i], -1, targv[i], wcslen(argv[i]), " ", NULL);
	}

	Main(argc, (char **) targv);

	for(i=0;i<argc;i++){
		free(targv[i]);
	}
	free(targv);

	return 0;
}

#endif
