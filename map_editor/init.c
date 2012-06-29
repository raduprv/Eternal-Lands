#include <sys/stat.h>
#ifndef WINDOWS
#include <dirent.h>
#include <unistd.h>
#include <locale.h>
#endif
#include "e3d.h"
#include "global.h"
#ifdef EYE_CANDY
#include "../eye_candy_wrapper.h"
#endif
#include "../asc.h"
#include "../io/elpathwrapper.h"
#include "../io/elfilewrapper.h"
#include "../io/fileutil.h"

char lang[10]={"en"};

char datadir[256]={"./"};
char configdir[256]={"./"};

#ifndef	NEW_TEXTURES
void init_texture_cache()
{
	int i;
	for (i = 0; i < TEXTURE_CACHE_MAX; i++)
	{
		texture_cache[i].file_name[0] = 0;
		texture_cache[i].texture_id = 0;
		texture_cache[i].load_err = 0;
	}
}
#endif	// NEW_TEXTURES

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
	for(i=0;i<MAX_OBJ_2D_DEF;i++)
		{
			obj_2d_def_cache[i].file_name[0]=0;
			obj_2d_def_cache[i].obj_2d_def_id=0;
		}
}

void read_config()
{
	// Set our configdir
	const char * tcfg = get_path_config();

	my_strncp (configdir, tcfg , sizeof(configdir));

	if ( !read_el_ini () )
	{
		// oops, the file doesn't exist, give up
		LOG_ERROR("Failure reading mapedit.ini");
		SDL_Quit ();
		exit (1);
	}
}

void init_stuff()
{
	int i;
	int seed;

	chdir(DATA_DIR);
	
#ifndef WINDOWS
	setlocale(LC_NUMERIC,"en_US");
#endif
	init_translatables();

	//create_error_mutex();
	init_crc_tables();
	init_zip_archives();
	cache_system_init(MAX_CACHE_SYSTEM);
	init_texture_cache();

	init_vars();
	
	read_config();

	file_check_datadir();

#ifdef LOAD_XML
	//Well, the current version of the map editor doesn't support having a datadir - will add that later ;-)
	load_translatables();
#endif

#ifdef LINUX
#ifdef GTK2
	init_filters();
#else
	file_selector = create_fileselection();
#endif
#endif	//LINUX

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

	SDL_EnableKeyRepeat (200, 100);

	seed = time (NULL);
  	srand (seed);

	init_texture_cache();
	init_particles ();
	init_e3d_cache();
	init_2d_obj_cache();

	for(i=0; i<256; i++)
        tile_list[i]=0;

	for (i = 0; i < MAX_LIGHTS; i++)
		lights_list[i] = NULL;

	new_map(256,256);
	load_all_tiles();

	//lights setup
	build_global_light_table();
	build_sun_pos_table();
	reset_material();
	init_lights();
	//disable_local_lights();
	//clear_error_log();

	// Setup the new eye candy system
#ifdef	EYE_CANDY
	ec_init();
#endif	//EYE_CANDY

	init_gl_extensions();

	if(have_multitexture)
#ifdef	NEW_TEXTURES
		ground_detail_text = load_texture_cached("./textures/ground_detail.bmp", tt_mesh);
#else	/* NEW_TEXTURES */
		ground_detail_text = load_texture_cache ("./textures/ground_detail.bmp",255);
#endif	/* NEW_TEXTURES */

	//load the fonts texture
	init_fonts();
#ifdef	NEW_TEXTURES
	icons_text=load_texture_cached("./textures/gamebuttons.bmp", tt_gui);
	buttons_text=load_texture_cached("./textures/buttons.bmp", tt_gui);
#else	/* NEW_TEXTURES */
	icons_text=load_texture_cache("./textures/gamebuttons.bmp",0);
	buttons_text=load_texture_cache("./textures/buttons.bmp",0);
#endif	/* NEW_TEXTURES */
	//get the application home dir

	have_multitexture=0;//debug only

#ifndef LINUX
	GetCurrentDirectory(sizeof(exec_path),exec_path);
#else
	exec_path[0]='.';exec_path[1]='/';exec_path[2]=0;
#endif
	init_browser();

    if(SDL_InitSubSystem(SDL_INIT_TIMER)<0)
    { 
        char str[120];
        snprintf(str, sizeof(str), "Couldn't initialize the timer: %s\n", SDL_GetError());
        log_error(__FILE__, __LINE__, str);
        SDL_Quit();
	    exit(1);
    }

	SDL_SetTimer (1000/(18*4), my_timer);

	SDL_EnableUNICODE(1);

    //we might want to do this later.

	// creating windows
	display_browser();
	toggle_window(browser_win);

	display_o3dow();
	toggle_window(o3dow_win);

	display_replace_window();
	toggle_window(replace_window_win);

	display_edit_window();
	toggle_window(edit_window_win);

	create_particles_window ();
}

void window_resize()
{
	float window_ratio;

	if (window_height==0)
        window_height=1;			// Prevent A Divide By Zero

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

