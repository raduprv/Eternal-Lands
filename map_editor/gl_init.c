#include <SDL.h>
#include "global.h"
#include "../asc.h"

#if defined(SDL2)
SDL_Window *el_gl_window = NULL;
static SDL_GLContext el_gl_context = NULL;
static SDL_version el_gl_linked;
#endif

int use_vertex_buffers=0;
int have_vertex_buffers=0;
int have_texture_non_power_of_two = 0;
int gl_extensions_loaded = 0;
float anisotropic_filter = 1.0f;

void Emul_glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)
{
	glDrawElements(mode, count, type, indices);
}

void init_gl_extensions()
{
	init_opengl_extensions();

	/*	GL_ARB_multitexture			*/
	if (have_extension(arb_multitexture))
	{
		have_multitexture = get_texture_units();
	}
	else
	{
		have_multitexture = 0;
	}
	/*	GL_ARB_multitexture			*/

	/*	GL_EXT_draw_range_elements		*/
	if (!have_extension(ext_draw_range_elements))
	{
		ELglDrawRangeElementsEXT = (PFNGLDRAWRANGEELEMENTSEXTPROC)&Emul_glDrawRangeElements;
	}
	/*	GL_EXT_draw_range_elements		*/

	/*	GL_ARB_texture_non_power_of_two		*/
	if (have_extension(arb_texture_non_power_of_two))
	{
		have_texture_non_power_of_two = 1;
	}
	/*	GL_ARB_texture_non_power_of_two		*/

	CHECK_GL_ERRORS();
	gl_extensions_loaded = 1;
}

void init_gl()
{
#if defined(SDL2)
	if(SDL_Init(SDL_INIT_VIDEO) == -1)
	{
		fprintf(stderr, "%s: %s\n", "SDL_Init() failed", SDL_GetError());
		SDL_Quit();
		exit(1);
	}

	el_gl_window = SDL_CreateWindow("Eternal Lands Editor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, SDL_WINDOW_OPENGL);
	if (el_gl_window == NULL)
	{
		fprintf(stderr, "%s: %s\n", "SDL_CreateWindow() failed", SDL_GetError());
		SDL_Quit();
		exit(1);
	}

	el_gl_context = SDL_GL_CreateContext(el_gl_window);
	if (el_gl_context == NULL)
	{
		LOG_ERROR("%s: %s\n", "SDL_GL_CreateContext() Failed", SDL_GetError());
		SDL_Quit();
		exit(1);
	}

	SDL_GetVersion(&el_gl_linked);

	SDL_GetWindowSize(el_gl_window, &window_width, &window_height);

	SDL_SetWindowMinimumSize(el_gl_window, 640,  480);
#if SDL_VERSION_ATLEAST(2, 0, 5)
	if (SDL_VERSIONNUM(el_gl_linked.major, el_gl_linked.minor, el_gl_linked.patch) >= 2005)
	{
		SDL_SetWindowResizable(el_gl_window, SDL_TRUE);
		SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
	}
#endif

//	SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 1 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	/* Set the window icon */
	SDL_SetWindowIcon(el_gl_window, SDL_LoadBMP("mapeditor.ico"));

#else
    int rgb_size[3];
#ifdef DEBUG
    if( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE) == -1 )
#else
    if( SDL_Init(SDL_INIT_VIDEO) == -1 )
#endif
        {
            char str[120];
            sprintf(str, "Couldn't initialize SDL: %s\n", SDL_GetError());
            log_error(__FILE__, __LINE__, str);
            SDL_Quit();
            exit(1);
        }
    if ( SDL_GetVideoInfo()->vfmt->BitsPerPixel <= 8 ){
        bpp=8;
    } else {
        bpp=16;
    }
    
    switch(bpp)
        {
            case 8:
                 rgb_size[0] = 2;
                 rgb_size[1] = 3;
                 rgb_size[2] = 3;
                 break;
            case 15:
            case 16:
            	 rgb_size[0] = 5;
            	 rgb_size[1] = 5;
            	 rgb_size[2] = 5;
            	 break;
      	    default:
      	   		 rgb_size[0] = 8;
		      	 rgb_size[1] = 8;
		      	 rgb_size[2] = 8;
		      	 break;
       }
      	 
  	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, rgb_size[0] );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, rgb_size[1] );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, rgb_size[2] );
//	SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 1 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	
    if ( SDL_SetVideoMode( window_width, window_height, bpp, SDL_OPENGL|SDL_RESIZABLE) == NULL )
	    {
			char str[120];
			sprintf(str, "Couldn't set GL mode: %s\n", SDL_GetError());
			log_error(__FILE__, __LINE__, str);
			SDL_Quit();
			exit(1);
        }
    
	/* Set the window manager title bar */
	SDL_WM_SetCaption( "Eternal Lands Editor", "testgl" );
	SDL_WM_SetIcon(SDL_LoadBMP("mapeditor.ico"), NULL);
#endif
}

void handle_window_resize()
{
	unload_texture_cache();
	if(minimap_tex) {glDeleteTextures(1,&minimap_tex);minimap_tex=0;}
	
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	
	init_gl();

	window_resize();
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
//	glDepthFunc(GL_LEQUAL);
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_NORMALIZE);
	glClearColor( 0.0, 0.0, 0.0, 0.0 );
	glClearStencil(0);
#if !defined(SDL2)
	SDL_EnableKeyRepeat (200, 100);
	SDL_EnableUNICODE(1);
#endif

	map_has_changed=1;
	reset_material();
	init_lights();
}
