#include "global.h"

void init_texture_cache()
{
	int i;
	for(i=0;i<1000;i++)
		{
			texture_cache[i].file_name[0]=0;
			texture_cache[i].texture_id=0;
		}
}

void init_e3d_cache()
{
	int i;
	for(i=0;i<1000;i++)
		{
			e3d_cache[i].file_name[0]=0;
			e3d_cache[i].e3d_id=0;
		}
}

void init_2d_obj_cache()
{
	int i;
	for(i=0;i<max_obj_2d_def;i++)
		{
			obj_2d_def_cache[i].file_name[0]=0;
			obj_2d_def_cache[i].obj_2d_def_id=0;
		}
}

void init_stuff()
{
	int i;
	int rgb_size[3];
	int seed;
	Uint32 (*my_timer_pointer) (unsigned int) = my_timer;

#ifdef LINUX
	file_selector = create_fileselection();
#endif	//LINUX

	if( SDL_Init(SDL_INIT_VIDEO) == -1 )
    {
		char str[120];
		sprintf(str, "Couldn't initialize SDL: %s\n", SDL_GetError());
		log_error(str);
		SDL_Quit();
		exit(1);
	}

	/* Detect the display depth */
		if ( SDL_GetVideoInfo()->vfmt->BitsPerPixel <= 8 ) {
			bpp = 8;
		} else {
			bpp = 16;  /* More doesn't seem to work */
		}

	/* Initialize the display */
	switch (bpp) {
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
	//if ( SDL_SetVideoMode( window_width, window_height, bpp, SDL_OPENGL|SDL_RESIZABLE|SDL_FULLSCREEN ) == NULL )
    if ( SDL_SetVideoMode( window_width, window_height, bpp, SDL_OPENGL|SDL_RESIZABLE) == NULL )
	{
		char str[120];
		sprintf(str, "Couldn't set GL mode: %s\n", SDL_GetError());
		log_error(str);
		SDL_Quit();
		exit(1);
	}

	/* Set the window manager title bar */
	SDL_WM_SetCaption( "Eternal Lands Editor", "testgl" );
	SDL_WM_SetIcon(SDL_LoadBMP("mapeditor.ico"), NULL);

	resize_window();
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

	SDL_EnableKeyRepeat (200, 100);

    seed = time (NULL);
  	srand (seed);

	init_texture_cache();
	init_e3d_cache();
	init_2d_obj_cache();
	for(i=0;i<256;i++)tile_list[i]=0;
	for(i=0;i<max_lights;i++)lights_list[i]=0;

	new_map(256,256);
	load_all_tiles();

	//lights setup
	build_global_light_table();
	build_sun_pos_table();
	reset_material();
	init_lights();
	//disable_local_lights();
	init_colors();
	clear_error_log();

   //now load the multitexturing extension
#ifndef LINUX
	glActiveTextureARB		= (PFNGLACTIVETEXTUREARBPROC)		SDL_GL_GetProcAddress("glActiveTextureARB");
	glMultiTexCoord2fARB	= (PFNGLMULTITEXCOORD2FARBPROC)		SDL_GL_GetProcAddress("glMultiTexCoord2fARB");
	if(!glActiveTextureARB || !glMultiTexCoord2fARB)have_multitexture=0;
	else have_multitexture=1;
#else
	have_multitexture=0;
#endif
	if(have_multitexture)ground_detail_text=load_texture_cache("./textures/ground_detail.bmp",255);

	//load the fonts texture
	font_text=load_texture_cache("./textures/font.bmp",0);
	buttons_text=load_texture_cache("./textures/buttons.bmp",0);
	sky_text_1=load_texture_cache("./textures/sky.bmp",70);
	//get the application home dir
#ifndef LINUX
	GetCurrentDirectory(sizeof(exec_path),exec_path);
#else
	exec_path[0]='.';exec_path[1]='/';exec_path[2]=0;
#endif


    if(SDL_InitSubSystem(SDL_INIT_TIMER)<0)
        {
   		    char str[120];
    		sprintf(str, "Couldn't initialize the timer: %s\n", SDL_GetError());
    		log_error(str);
    		SDL_Quit();
	       	exit(1);
        }
	SDL_SetTimer (1000/17, my_timer_pointer);

	SDL_EnableUNICODE(1);

    //we might want to do this later.
}

void resize_window()
{
	float window_ratio;
	if (window_height==0)window_height=1;			// Prevent A Divide By Zero

	glViewport(0, 0, window_width, window_height);					// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();							// Reset The Projection Matrix

	window_ratio=(GLfloat)window_width/(GLfloat)window_height;

	//glOrtho( -3.0*window_ratio, 3.0*window_ratio, -3.0, 3.0, -35.0, 35.0 );

	// Calculate The Aspect Ratio Of The Window
	//gluPerspective(45.0f,(GLfloat)window_width/(GLfloat)window_height,0.1f,1000.0f);
	glOrtho( -1.0*zoom_level*window_ratio, 1.0*zoom_level*window_ratio, -1.0*zoom_level, 1.0*zoom_level, -40.0, 40.0 );

	glMatrixMode(GL_MODELVIEW);						// Select The Modelview Matrix
	glLoadIdentity();							// Reset The Modelview Matrix
}

