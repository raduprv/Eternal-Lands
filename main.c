
#include "global.h"

/**********************************************************************/

int start_rendering()
{
    int done = 0;
    /*
    static GLuint texture;
    int x=0,y=0;
    int i=0;
    */
	/* Loop until done. */
	while( !done ) {
	  /*
		GLenum gl_error;
		char* sdl_error;
	  */
		SDL_Event event;
/*
    last_time=cur_time;
    get_world_x_y();
    draw_scene();
*/
		/* Check if there's a pending event. */
		
		while( SDL_PollEvent( &event ) )
        {
			done = HandleEvent(&event);
		}

    last_time=cur_time;
    get_world_x_y();
    draw_scene();

#ifdef LINUX
    while (gtk_events_pending())
      gtk_main_iteration();
#endif		
	
	}

	/* Destroy our GL context, etc. */ 
	SDL_Quit( );
	return(0);
}

int Main(int argc, char *argv[])
{
  
        //int i
  	int logo;
	int numtests;
	//int bpp = 0;
	int slowly;
	//float gamma = 0.0;
	//int noframe = 1;

	logo = 1;
	slowly = 1;
	numtests = 1;


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
