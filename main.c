#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#ifdef NETWORK_THREAD
 #include "queue.h"
#endif //NETWORK_THREAD

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
int version_second_digit=10;
int done = 0;

int gargc;
char **  gargv;
/**********************************************************************/

int start_rendering()
{
	SDL_Thread *music_thread=SDL_CreateThread(update_music, 0);
#ifdef NETWORK_THREAD
	SDL_Thread *network_thread;
	queue_t *message_queue;
#endif //NETWORK_THREAD

#ifndef WINDOWS
	SDL_EventState(SDL_SYSWMEVENT,SDL_ENABLE);
#endif
#ifdef NETWORK_THREAD
	queue_initialise(&message_queue);
	network_thread = SDL_CreateThread(get_message_from_server, message_queue);
#endif //NETWORK_THREAD

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
#ifdef NETWORK_THREAD
			if(!queue_isempty(message_queue)) {
				message_t *message;

				while((message = queue_pop(message_queue)) != NULL)
				{
					process_message_from_server(message->data, message->length);
					free(message->data);
					free(message);
				}
			}
#else
			get_message_from_server();
#endif //NETWORK_THREAD
			if(!limit_fps || ((cur_time-last_time) && (1000/(cur_time-last_time) < limit_fps)))
				{
					//draw everything
					draw_scene();
					last_time=cur_time;
				}
			else
				SDL_Delay(1);//give up timeslice for anyone else

#ifdef TIMER_CHECK
			//Check the timers to make sure that they are all still alive...
			check_timers();
#endif

			//cache handling
			if(cache_system)cache_system_maint();
			//see if we need to exit
			if(exit_now) {
				done = 1;
				break;
			}
		}
	if(!done) {
		done = 1;
	}
	have_music=0;
	SDL_WaitThread(music_thread,&done);
#ifdef NETWORK_THREAD
	SDL_WaitThread(network_thread,&done);
	queue_destroy(message_queue);
#endif //NETWORK_THREAD
	if(pm_log.ppl)free_pm_log();
	
	save_bin_cfg();
#ifdef NEW_CLIENT
	//Save the quickbar spells
	save_quickspells();
#endif
	// save el.ini if asked
	if (write_ini_on_exit) write_el_ini ();
	#ifdef NOTEPAD
	// save notepad contents if the file was loaded
	if (notepad_loaded) notepadSaveFile (NULL, 0, 0, 0);
	#endif
	
	unload_questlog();
	free_icons();
	free_vars();
	cleanup_rules();
	unload_e3d_list();	// do we really want to overwrite this file??
	SDL_RemoveTimer(draw_scene_timer);
	SDL_RemoveTimer(misc_timer);
	end_particles_list();
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
					if(gargv[i][1]=='-')check_var(gargv[i]+2,COMMAND_LINE_LONG_VAR);
					else
						{
							char str[200];
							snprintf(str,198,"%s %s",gargv[i],gargv[i+1]);
							check_var(str+1,COMMAND_LINE_SHORT_VAR);
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
// splits a char* into a char ** based on the delimiters
int makeargv(char *s, char *delimiters, char ***argvp)
{
	int i, numtokens;
	char *snew, *t;

	if ((s == NULL) || (delimiters == NULL) || (argvp == NULL))
		return -1;

	*argvp = NULL;
	snew = s + strspn(s, delimiters);
	if ((t = malloc(strlen(snew) + 1)) == NULL)
		return -1;
	strcpy(t, snew);

	numtokens = 0;
	if (strtok(t, delimiters) != NULL)
		for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++);

	if ((*argvp = malloc((numtokens + 1)*sizeof(char *))) == NULL){
		free(t);
		return -1;
	}
	if (numtokens == 0)
		free(t);
	else{
		strcpy(t, snew);
		**argvp = strtok(t, delimiters);
		for (i = 1; i < numtokens; i++)
			*((*argvp) + i) = strtok(NULL, delimiters);
	}
	*((*argvp) + numtokens) = NULL;
	return numtokens;
}
//frees the char** created by makeargv
void freemakeargv(char **argv)
{
	if (argv == NULL)
		return;
	if (*argv != NULL)
		free(*argv);
	free(argv);
}

int APIENTRY WinMain (HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow)
{
	char *k=GetCommandLine();
	char **argv=NULL;
	int argc=makeargv(k, " \t\n", &argv);
	Main(argc, (char **) argv);
	freemakeargv(argv);
	return 0;
}

#endif
