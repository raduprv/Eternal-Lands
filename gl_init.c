#include "global.h"

void init_gl()
{
    int rgb_size[3];
#ifdef DEBUG
    if( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE) == -1 )
#else
    if( SDL_Init(SDL_INIT_VIDEO) == -1 )
#endif
        {
            char str[120];
            sprintf(str, "Couldn't initialize SDL: %s\n", SDL_GetError());
            log_error(str);
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
			log_error(str);
			SDL_Quit();
			exit(1);
        }
    
	/* Set the window manager title bar */
	SDL_WM_SetCaption( "Eternal Lands Editor", "testgl" );
	SDL_WM_SetIcon(SDL_LoadBMP("mapeditor.ico"), NULL);
}

void handle_window_resize()
{
    int i,alpha;
    for(i=0;i<1000;i++)
    	{
	   		if(texture_cache[i].file_name[0])
	       		{
					glDeleteTextures(1,&texture_cache[i].texture_id);
					texture_cache[i].texture_id=0;//force a reload
				}
        }
	
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	
	init_gl();

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
	
	for(i=0;i<1000;i++)
		{
			if(texture_cache[i].file_name[0])
				{
	            	alpha=texture_cache[i].alpha;
	            	//our texture was freed, we have to reload it
	        		if(alpha==0)texture_cache[i].texture_id=load_bmp8_color_key(texture_cache[i].file_name);
	            	else
						texture_cache[i].texture_id=load_bmp8_fixed_alpha(texture_cache[i].file_name, alpha);
				}
		}
	reset_material();
	init_lights();
	init_colors();
}            
