#ifdef OSX
#include <libgen.h>
#endif
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
#include "dialogues.h"
#include "draw_scene.h"
#include "e3d.h"
#include "elc_private.h"
#include "elconfig.h"
#include "encyclopedia.h"
#include "errors.h"
#include "events.h"
#include "gl_init.h"
#include "hud.h"
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
#include "password_manager.h"
#include "pm_log.h"
#include "questlog.h"
#include "queue.h"
#include "reflection.h"
#include "rules.h"
#include "session.h"
#include "shader/shader.h"
#include "sky.h"
#include "sound.h"
#include "text.h"
#include "timers.h"
#include "trade_log.h"
#include "translate.h"
#include "textures.h"
#include "update.h"
#include "url.h"
#include "user_menus.h"
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

int exit_now=0;
int restart_required=0;
Uint32 cur_time=0, last_time=0;//for FPS


int gargc;
char **  gargv;
/**********************************************************************/

void cleanup_mem(void)
{
	int i;

	LOG_INFO("destroy_url_list()");
	destroy_url_list();
	LOG_INFO("history_destroy()");
	history_destroy();
	LOG_INFO("command_cleanup()");
	command_cleanup();
	LOG_INFO("destroy_buddy_queue()");
	destroy_buddy_queue();
	LOG_INFO("cleanup_manufacture()");
	cleanup_manufacture();
	LOG_INFO("cleanup_text_buffers()");
	cleanup_text_buffers();
	LOG_INFO("destroy_all_actors()");
	destroy_all_actors();
	LOG_INFO("end_actors_lists()");
	end_actors_lists();
	LOG_INFO("cleanup_lights()");
	cleanup_lights();
	/* 2d objects */
	LOG_INFO("destroy_all_2d_objects()");
	destroy_all_2d_objects();
	LOG_INFO("destroy_all_2d_object_defs()");
	destroy_all_2d_object_defs();
	/* 3d objects */
	LOG_INFO("destroy_all_3d_objects()");
	destroy_all_3d_objects();
	/* caches */
	cache_e3d->free_item = &destroy_e3d;
	LOG_INFO("cache_delete()");
	cache_delete(cache_e3d);
	cache_e3d = NULL;
	LOG_INFO("free_texture_cache()");
	free_texture_cache();
	// This should be fixed now  Sir_Odie
	LOG_INFO("cache_delete()");
	cache_delete(cache_system);
	cache_system = NULL;
	/* map location information */
	LOG_INFO("cleanup_mapinfo()");
	cleanup_mapinfo();

	LOG_INFO("destroy_hash_table()");
	destroy_hash_table(server_marks);

	LOG_INFO("video_modes[]");
	for (i = 0; i < video_modes_count; i++)
	{
		if (video_modes[i].name)
			free(video_modes[i].name);
	}
	LOG_INFO("free_shaders()");
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
	network_thread = SDL_CreateThread(get_message_from_server, "NetworkThread", network_thread_data);

	/* Loop until done. */
	while( !done )
		{
			SDL_Event event;

			// handle SDL events
			in_main_event_loop = 1;
			while( SDL_PollEvent( &event ) && !done)
				{
					done = HandleEvent(&event);
				}
			in_main_event_loop = 0;

			//advance the clock
			cur_time = SDL_GetTicks();

			// update the approximate distance moved
			update_session_distance();

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

			if(!max_fps || (cur_time-last_time && 1000/(cur_time-last_time) <= max_fps))
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
	LOG_INFO("Client closing");
	LOG_INFO("SDL_WaitThread(network_thread)");
	SDL_WaitThread(network_thread,&done);
	LOG_INFO("queue_destroy()");
	queue_destroy(message_queue);
	LOG_INFO("free_pm_log()");
	free_pm_log();

	// window positions are proportionally adjusted ready for the next client run
	LOG_INFO("restore_window_proportionally()");
	restore_window_proportionally();

	//save all local data
	LOG_INFO("save_local_date()");
	save_local_data();

#ifdef PAWN
	LOG_INFO("cleanup_pawn()");
	cleanup_pawn ();
#endif

#ifdef NEW_SOUND
	LOG_INFO("destroy_sound()");
	destroy_sound();		// Cleans up physical elements of the sound system and the streams thread
	LOG_INFO("clear_sound_data()");
	clear_sound_data();		// Cleans up the config data
#endif // NEW_SOUND
	LOG_INFO("ec_destroy_all_effects()");
	ec_destroy_all_effects();
	if (have_a_map)
	{
		LOG_INFO("destroy_map()");
		destroy_map();
		LOG_INFO("free_buffers()");
		free_buffers();
	}
	LOG_INFO("cleanup_dialogues()");
	cleanup_dialogues();
	LOG_INFO("passmngr_destroy()");
	passmngr_destroy();
	LOG_INFO("unload_questlog()");
	unload_questlog();
	LOG_INFO("save_item_lists()");
	save_item_lists();
	LOG_INFO("free_emotes()");
	free_emotes();
	LOG_INFO("free_actor_defs()");
	free_actor_defs();
	LOG_INFO("free_vars()");
	free_vars();
	LOG_INFO("cleanup_rules()");
	cleanup_rules();
	//save_exploration_map();
	LOG_INFO("cleanup_counters()");
	cleanup_counters();
	LOG_INFO("cleanup_chan_names()");
	cleanup_chan_names();
	LOG_INFO("cleanup_hud()");
	cleanup_hud();
	LOG_INFO("destroy_trade_log()");
	destroy_trade_log();
	LOG_INFO("destroy_user_menus()");
	destroy_user_menus();
	LOG_INFO("destroy_all_root_windows()");
	destroy_all_root_windows();
	LOG_INFO("SDL_RemoveTimer()");
	SDL_RemoveTimer(draw_scene_timer);
	LOG_INFO("SDL_RemoveTimer()");
	SDL_RemoveTimer(misc_timer);
	LOG_INFO("end_particles()");
	end_particles ();
	LOG_INFO("free_bbox_tree()");
	free_bbox_tree(main_bbox_tree);
	main_bbox_tree = NULL;
	LOG_INFO("free_astro_buffer()");
	free_astro_buffer();
	LOG_INFO("free_translations()");
	free_translations();
	LOG_INFO("free_skybox()");
	free_skybox();
	/* Destroy our GL context, etc. */
	LOG_INFO("SDL_QuitSubSystem(SDL_INIT_AUDIO)");
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	LOG_INFO("SDL_QuitSubSystem(SDL_INIT_TIMER)");
	SDL_QuitSubSystem(SDL_INIT_TIMER);
/*#ifdef WINDOWS
	// attempt to restart if requested
	if(restart_required > 0){
		LOG_INFO("Restarting %s", win_command_line);
		SDL_CreateThread(system, "MainThread", win_command_line);
	}
#endif  //WINDOWS
*/

#ifdef	CUSTOM_UPDATE
	LOG_INFO("stopp_custom_update()");
	stopp_custom_update();
#endif	/* CUSTOM_UPDATE */
	LOG_INFO("clear_zip_archives()");
	clear_zip_archives();
	LOG_INFO("clean_update()");
	clean_update();

	LOG_INFO("cleanup_tcp()");
	cleanup_tcp();

	if (use_frame_buffer)
	{
		LOG_INFO("free_reflection_framebuffer()");
		free_reflection_framebuffer();
	}

	LOG_INFO("cursors_cleanup()");
	cursors_cleanup();

	LOG_INFO("SDL_Quit()");
	SDL_Quit();
	LOG_INFO("cleanup_mem()");
	cleanup_mem();
	LOG_INFO("xmlCleanupParser()");
	xmlCleanupParser();
	LOG_INFO("FreeXML()");
	FreeXML();

#ifdef NEW_SOUND
	LOG_INFO("final_sound_exit()");
	final_sound_exit();
#endif

	LOG_INFO("exit_logging()");
	exit_logging();

	printf("Exit Complete\n"); fflush(stdout);
	fflush(stdout);

	return(0);
}

void	read_command_line(void)
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

#ifdef OSX
// we need to move to the Data directory so server.lst can be found
void setupWorkingDirectory(const char *argv0, size_t len)
{
	char *path = malloc(len + 1);
	strncpy(path, argv0, len);
	if ( ! ((chdir(dirname(path)) == 0) &&
		(chdir("../Resources/data") == 0)))
		fprintf(stderr, "Failed to change to data directory path\n");
	free(path);
}
#endif

#ifdef WINDOWS
int Main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
#ifdef OSX
	if (argc > 0) // should always be true
		setupWorkingDirectory(argv[0], strlen(argv[0]));
#endif
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
