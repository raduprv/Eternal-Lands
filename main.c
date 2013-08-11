#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "platform.h"

#ifdef	__GNUC__
 #include <unistd.h>
#ifdef  WINDOWS
 #include   <process.h>
#endif
#endif

#ifdef WINDOWS
 #include <windows.h>
 #undef WRITE_XML
 char   *win_command_line;
#endif //WINDOWS

#include "2d_objects.h"
#include "3d_objects.h"
#include "actor_scripts.h"
#include "asc.h"
#include "astrology.h"
#include "bbox_tree.h"
#include "books.h"
#include "buddy.h"
#include "console.h"
#include "counters.h"
#include "cursors.h"
#include "draw_scene.h"
#include "e3d.h"
#include "elc_private.h"
#include "elconfig.h"
#include "encyclopedia.h"
#include "errors.h"
#include "events.h"
#include "gl_init.h"
#include "icon_window.h"
#include "io/elfilewrapper.h"
#include "init.h"
#include "item_lists.h"
#include "interface.h"
#include "lights.h"
#include "manufacture.h"
#include "map.h"
#include "minimap.h"
#include "multiplayer.h"
#include "particles.h"
#include "pm_log.h"
#include "questlog.h"
#include "queue.h"
#include "reflection.h"
#include "rules.h"
#include "shader/shader.h"
#include "sky.h"
#include "sound.h"
#include "text.h"
#include "timers.h"
#include "translate.h"
#include "textures.h"
#include "url.h"
#include "weather.h"
#ifdef MEMORY_DEBUG
#include "elmemory.h"
#endif
#ifdef PAWN
#include "pawn/elpawn.h"
#endif
#ifdef	CUSTOM_UPDATE
#include "custom_update.h"
#endif	/* CUSTOM_UPDATE */
#ifdef	FSAA
#include "fsaa/fsaa.h"
#endif	/* FSAA */

Uint32 cur_time=0, last_time=0;//for FPS

char version_string[]=VER_STRING;
int	client_version_major=VER_MAJOR;
int client_version_minor=VER_MINOR;
int client_version_release=VER_RELEASE;
int	client_version_patch=VER_BUILD;
int version_first_digit=10;	//protocol/game version sent to server
int version_second_digit=26;

int gargc;
char **  gargv;
/**********************************************************************/

void cleanup_mem(void)
{
	int i;

	destroy_url_list();
	history_destroy();
	command_cleanup();
	queue_destroy(buddy_request_queue);
	cleanup_manufacture();
	cleanup_text_buffers();
	cleanup_fonts();
	cursors_cleanup();
	destroy_all_actors();
	end_actors_lists();
	cleanup_lights();
	/* 2d objects */
	destroy_all_2d_objects();
	/* 3d objects */
	destroy_all_3d_objects();
	/* caches */
	cache_e3d->free_item = &destroy_e3d;
	cache_delete(cache_e3d);
	cache_e3d = NULL;
	free_texture_cache();
	// This should be fixed now  Sir_Odie
	cache_delete(cache_system);
	cache_system = NULL;
	/* map location information */
	for (i = 0; continent_maps[i].name; i++)
	{
	    free(continent_maps[i].name);
	}
	free (continent_maps);

	destroy_hash_table(server_marks);
	
	for (i = 0; i < video_modes_count; i++)
	{
		if (video_modes[i].name)
			free(video_modes[i].name);
	}
	free_shaders();
}

/* temp code to allow my_timer to dynamically adjust partical update rate */
volatile int in_main_event_loop = 0;

int start_rendering()
{
	static int done = 0;
	static void * network_thread_data[2] = { NULL, NULL };
	static Uint32 last_frame_and_command_update = 0;

	SDL_Thread *network_thread;
	queue_t *message_queue;

#ifndef WINDOWS
	SDL_EventState(SDL_SYSWMEVENT,SDL_ENABLE);
#endif
	queue_initialise(&message_queue);
	network_thread_data[0] = message_queue;
	network_thread_data[1] = &done;
	network_thread = SDL_CreateThread(get_message_from_server, network_thread_data);

	/* Loop until done. */
	while( !done )
		{
			SDL_Event event;

			// handle SDL events
			in_main_event_loop = 1;
			while( SDL_PollEvent( &event ) )
				{
					done = HandleEvent(&event);
				}
			in_main_event_loop = 0;

			//advance the clock
			cur_time = SDL_GetTicks();

			//check for network data
			if(!queue_isempty(message_queue)) {
				message_t *message;

				while((message = queue_pop(message_queue)) != NULL)
				{
					process_message_from_server(message->data, message->length);
					free(message->data);
					free(message);
				}
			}
#ifdef	OLC
			olc_process();
#endif	//OLC
			my_tcp_flush(my_socket);    // make sure the tcp output buffer is set
			
			if (have_a_map && cur_time > last_frame_and_command_update + 60) {
				LOCK_ACTORS_LISTS();
				next_command();
				UNLOCK_ACTORS_LISTS();
				move_to_next_frame();
				last_frame_and_command_update = cur_time;
			}

			while (cur_time > next_second_time && real_game_second < 59)
			{
				real_game_second += 1;
				new_second();
				next_second_time += 1000;
			}

#ifdef NEW_SOUND
			weather_sound_control();
#endif	//NEW_SOUND

			if(!limit_fps || (cur_time-last_time && 1000/(cur_time-last_time) <= limit_fps))
			{
				weather_update();

                animate_actors();
				//draw everything
				draw_scene();
				last_time=cur_time;
			}
			else {
				SDL_Delay(1);//give up timeslice for anyone else
			}

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
	LOG_INFO("Client closed");
	SDL_WaitThread(network_thread,&done);
	queue_destroy(message_queue);
	if(pm_log.ppl)free_pm_log();

	//save all local data
	save_local_data(NULL, 0);

#ifdef PAWN
	cleanup_pawn ();
#endif

#ifdef NEW_SOUND
	destroy_sound();		// Cleans up physical elements of the sound system and the streams thread
	clear_sound_data();		// Cleans up the config data
#endif // NEW_SOUND
	ec_destroy_all_effects();
	if (have_a_map)
	{
		destroy_map();
		free_buffers();
	}
	unload_questlog();
	save_item_lists();
	free_emotes();
	free_actor_defs();
	free_books();
	free_vars();
	cleanup_rules();
	save_exploration_map();
	cleanup_counters();
	cleanup_chan_names();
	SDL_RemoveTimer(draw_scene_timer);
	SDL_RemoveTimer(misc_timer);
	end_particles ();
	free_bbox_tree(main_bbox_tree);
	main_bbox_tree = NULL;
	free_astro_buffer();
	free_translations();
	free_skybox();
	/* Destroy our GL context, etc. */
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	SDL_QuitSubSystem(SDL_INIT_TIMER);
/*#ifdef WINDOWS
	// attempt to restart if requested
	if(restart_required > 0){
		LOG_INFO("Restarting %s", win_command_line);
		SDL_CreateThread(system, win_command_line);
	}
#endif  //WINDOWS
*/
#ifdef NEW_SOUND
	final_sound_exit();
#endif
#ifdef	CUSTOM_UPDATE
	stopp_custom_update();
#endif	/* CUSTOM_UPDATE */
	clear_zip_archives();

	destroy_tcp_out_mutex();

	if (use_frame_buffer) free_reflection_framebuffer();

	printf("doing SDL_Quit\n");
	fflush(stderr);
	SDL_Quit( );
	printf("done SDL_Quit\n");
	fflush(stderr);
	cleanup_mem();
	xmlCleanupParser();
	FreeXML();

	exit_logging();

	return(0);
}

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
							if(strchr(gargv[i], '=') != NULL) {	//eg -u=name
								safe_snprintf(str,sizeof(str),"%s",gargv[i]);
							} else if(i>=gargc-1 || gargv[i+1][0] == '-') {	//eg -uname
								safe_snprintf(str,sizeof(str),"%s",gargv[i]);
							} else {	//eg -u name
								safe_snprintf(str,sizeof(str),"%s %s",gargv[i],gargv[i+1]);
							}
							check_var(str+1,COMMAND_LINE_SHORT_VAR);
						}
				}
		}
}

/* We need an additional function as the command line should be read after the config, but this
 * variable is needed to load the correct config.
 */
char * check_server_id_on_command_line()
{
	if (gargc < 2)
		return "";

	// FIXME!! This should parse for -options rather than blindly returning the last option!
	
	return gargv[gargc - 1];
}

void check_log_level_on_command_line()
{
	Uint32 i;

	for (i = 1; i < gargc; i++)
	{
		if (strncmp(gargv[i], "--log_level=", 12) == 0)
		{
			if (strcmp(gargv[i], "--log_level=error") == 0)
			{
				set_log_level(llt_error);
				continue;
			}
			if (strcmp(gargv[i], "--log_level=warning") == 0)
			{
				set_log_level(llt_warning);
				continue;
			}
			if (strcmp(gargv[i], "--log_level=info") == 0)
			{
				set_log_level(llt_info);
				continue;
			}
			if (strcmp(gargv[i], "--log_level=debug") == 0)
			{
				set_log_level(llt_debug);
				continue;
			}
			if (strcmp(gargv[i], "--log_level=debug_verbose") == 0)
			{
				set_log_level(llt_debug_verbose);
				continue;
			}
			continue;
		}
		if (strncmp(gargv[i], "-ll=", 4) == 0)
		{
			if (strcmp(gargv[i], "-ll=e") == 0)
			{
				set_log_level(llt_error);
				continue;
			}
			if (strcmp(gargv[i], "-ll=w") == 0)
			{
				set_log_level(llt_warning);
				continue;
			}
			if (strcmp(gargv[i], "-ll=i") == 0)
			{
				set_log_level(llt_info);
				continue;
			}
			if (strcmp(gargv[i], "-ll=d") == 0)
			{
				set_log_level(llt_debug);
				continue;
			}
			if (strcmp(gargv[i], "-ll=dv") == 0)
			{
				set_log_level(llt_debug_verbose);
				continue;
			}
			continue;
		}
		if (strcmp(gargv[i], "--debug") == 0)
		{
			set_log_level(llt_debug_verbose);
			continue;
		}
	}
}

#ifdef WINDOWS
int Main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
#ifdef MEMORY_DEBUG
	elm_init();
#endif //MEMORY_DEBUG
	gargc=argc;
	gargv=argv;

	// do basic initialization
#ifdef	OLC
	olc_init();
#endif	//OLC
	init_logging("log");

	check_log_level_on_command_line();
	create_tcp_out_mutex();
	init_translatables();
#ifdef	FSAA
	init_fsaa_modes();
#endif	/* FSAA */
	init_vars();

	ENTER_DEBUG_MARK("init stuff");

	init_stuff();

	LEAVE_DEBUG_MARK("init stuff");

	start_rendering();
#ifdef MEMORY_DEBUG
	elm_cleanup();
#endif //MEMORY_DEBUG
#ifdef	OLC
	olc_shutdown();
#endif	//OLC

#ifndef WINDOWS
	// attempt to restart if requested
	if(restart_required > 0){
		LOG_INFO("Restarting %s\n", *argv);
		execv(*argv, argv);
	}
#endif  //WINDOWS

	return 0;
}


#ifdef WINDOWS
// splits a char* into a char ** based on the delimiters
static int makeargv(char *s, char *delimiters, char ***argvp)
{
	int i, numtokens;
	char *snew, *t;

	if ((s == NULL) || (delimiters == NULL) || (argvp == NULL))
		return -1;

	*argvp = NULL;
	snew = s + strspn(s, delimiters);
	if ((t = malloc(strlen(snew) + 1)) == NULL)
		return -1;
	strcpy(t, snew);	// It's fine that this isn't strncpy, since t is sizeof(snew) + 1.

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
static void freemakeargv(char **argv)
{
	if (argv == NULL)
		return;
	if (*argv != NULL)
		free(*argv);
	free(argv);
}

int APIENTRY WinMain (HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow)
{
	char **argv= NULL;
	int argc;

	win_command_line = GetCommandLine();
	argc = makeargv(win_command_line, " \t\n", &argv);

	Main(argc, (char **) argv);
	freemakeargv(argv);

	// attempt to restart if requested
	if(restart_required > 0){
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		LOG_INFO("Restarting %s", win_command_line);
		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );

		CreateProcess(NULL, win_command_line,
			NULL,			// Process handle not inheritable.
			NULL,			// Thread handle not inheritable.
			FALSE,			// Set handle inheritance to FALSE.
			DETACHED_PROCESS,	// Keep this separate
			NULL,			// Use parent's environment block.
			NULL,			// Use parent's starting directory.
			&si,			// Pointer to STARTUPINFO structure.
			&pi);          // Pointer to PROCESS_INFORMATION structure
	}

	return 0;
}

#endif
