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
    //static GLuint texture;  unused?
    //int x=0,y=0; unused?
    //int i=0;     unused?
    int done=0;
#ifndef WINDOWS
	SDL_EventState(SDL_SYSWMEVENT,SDL_ENABLE);
#endif
	/* Loop until done. */
	while( !done )
    {
      //GLenum gl_error; unused?
      //char* sdl_error; unused?
      SDL_Event event;


		while( SDL_PollEvent( &event ) )
        {
			done = HandleEvent(&event);
		}

        last_time=cur_time;
        draw_scene();
        if(exit_now)break;
    }

	save_bin_cfg();
	/* Destroy our GL context, etc. */
	Mix_CloseAudio();
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
	int logo; // i unused?
	int numtests;
	//int bpp = 0; unused?
	int slowly;
	//float gamma = 0.0; unused?
	//int noframe = 1;  unused?

	logo = 1;
	slowly = 1;
	numtests = 1;

    init_stuff();
    start_rendering();

	return 0;
}

#ifdef WINDOWS
int STDCALL WinMain (HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow)
{
Main();
  return 0;
}

#endif
