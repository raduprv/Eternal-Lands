#include "tiles.h"
#include "global.h"

/**********************************************************************/

int start_rendering()
{
    int done = 0;
    Uint32 last_save_time=0;
	/* Loop until done. */
	while( !done ) {
		SDL_Event event;
		
    		cur_time = SDL_GetTicks();
		/* Check if there's a pending event. */
		
		while( SDL_PollEvent( &event ) )
        		{
				done = HandleEvent(&event);
			}

    		get_world_x_y();
		
		if(!limit_fps || ((cur_time - last_time) && (800/(cur_time-last_time) < (Uint32)limit_fps)))
			{
				draw_scene();
				last_time=cur_time;
			}
		else SDL_Delay(1);

		if(auto_save_time && (cur_time-last_save_time)>(Uint32)auto_save_time)
			{
				last_save_time=cur_time;
				save_map("maps/Autosave.elm");
			}

#ifdef LINUX
		while (gtk_events_pending())
			gtk_main_iteration();
#endif		
	}

	/* Destroy our GL context, etc. */ 
	destroy_map_tiles();
	SDL_SetTimer(0,NULL);
	end_particles ();
	SDL_Quit( );
	return(0);
}

int Main(int argc, char *argv[])
{
  
        //int i
  	//int logo = 1;
	//int numtests = 1;
	//int bpp = 0;
	//int slowly = 1;
	//float gamma = 0.0;
	//int noframe = 1;

#ifdef	LINUX
	gtk_set_locale ();
	gtk_init (&argc, &argv);
#endif	//LINUX
	init_stuff();
	
	start_rendering();

	return 0;
}

#ifdef LINUX
int main(int argc, char *argv[])
{
  return Main(argc, argv);
}
#endif

#ifndef LINUX
int WINAPI WinMain (HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow)
{
Main(0, NULL);
  return 0;
}
#endif
