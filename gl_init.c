#include "global.h"

Uint32 flags;


void setup_video_mode()
{
		if(full_screen)
			{
				if(video_mode==1)
					{
						window_width=640;
						window_height=480;
						bpp=16;
					}
				else
				if(video_mode==2)
					{
						window_width=640;
						window_height=480;
						bpp=32;
					}
				else
				if(video_mode==3)
					{
						window_width=800;
						window_height=600;
						bpp=16;
					}
				else
				if(video_mode==4)
					{
						window_width=800;
						window_height=600;
						bpp=32;
					}
				if(video_mode==5)
					{
						window_width=1024;
						window_height=768;
						bpp=16;
					}
				else
				if(video_mode==6)
					{
						window_width=1024;
						window_height=768;
						bpp=32;
					}
			}
			else //windowed mode
			{
				if(video_mode==1 || video_mode==2)
					{
						window_width=640;
						window_height=480;
					}
				else
				if(video_mode==3 || video_mode==4)
					{
						window_width=780;
						window_height=550;
						log_to_console(c_yellow1,"Window size adjusted to 780x550.");
					}
				if(video_mode==5 || video_mode==6)
					{
						window_width=990;
						window_height=720;
						log_to_console(c_yellow1,"Window size adjusted to 990x720.");
					}
				bpp=0;//autodetect
			}
#ifndef WINDOWS
	bpp=0;//under X, we can't change the desktop BPP
#endif
}

int check_gl_mode()
{
	char str[400];


	if(full_screen)flags=SDL_OPENGL|SDL_FULLSCREEN;
	else flags=SDL_OPENGL;
	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8);

	//now, test if the video mode is OK...
	if(!SDL_VideoModeOK(window_width, window_height, bpp, flags))
		{

			sprintf(str,"Video mode %ix%ix%i with a stencil buffer is not available\nTrying this mode without a stencil buffer...",window_width,window_height,bpp);
			log_to_console(c_red1,str);

			SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 0);
			have_stencil=0;
				//now, test if the video mode is OK...
				if(!SDL_VideoModeOK(window_width, window_height, bpp, flags))
					{
						int old_width;
						int old_height;
						int old_bpp;

						old_width=window_width;
						old_height=window_height;
						old_bpp=bpp;

						window_width=640;
						window_height=480;
						bpp=32;

						sprintf(str,"Video mode %ix%ix%i without a stencil buffer is not available\nTrying the safemode (640x480x32) Full Screen (no stencil)",old_width,old_height,old_bpp);
						log_to_console(c_red1,str);

						full_screen=1;
						video_mode=2;

					}

		}
	else have_stencil=1;

}

void init_video()
{
	int rgb_size[3];

	if( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) == -1 )
    {
		char str[120];
		sprintf(str, "Couldn't initialize SDL: %s\n", SDL_GetError());
		log_error(str);
		SDL_Quit();
		exit(1);
	}

	setup_video_mode();

	/* Detect the display depth */
	if(!bpp)
		{
			if ( SDL_GetVideoInfo()->vfmt->BitsPerPixel <= 8 )
			{
				bpp = 8;
			}
			else
			if ( SDL_GetVideoInfo()->vfmt->BitsPerPixel <= 16 )
			{
				bpp = 16;  /* More doesn't seem to work */
			}
			else bpp=32;
		}

	//adjust the video mode accordingly
	if(bpp==16)
		{
			if(video_mode==2)video_mode=1;
			if(video_mode==4)video_mode=3;
			if(video_mode==6)video_mode=5;
		}
	else
		{
			if(video_mode==1)video_mode=2;
			if(video_mode==3)video_mode=4;
			if(video_mode==5)video_mode=6;

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
	SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 0);
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	check_gl_mode();


	//try to find a stencil buffer (it doesn't always work on Linux)
    if(!SDL_SetVideoMode(window_width, window_height, bpp, flags))
    	{
			log_to_console(c_red1,"Couldn't find a hardware accelerated stencil buffer.\nShadows are not available.");
			if(bpp!=32)log_to_console(c_grey1,"Hint: Try a 32 BPP resolution (if you are under XWindows, set your screen display to 24 or 32 bpp).");
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,16);
			SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,0);
			if(!SDL_SetVideoMode( window_width, window_height, bpp, flags))
			    {
				char str[120];
				sprintf(str, "Couldn't set GL mode: %s\n", SDL_GetError());
				log_error(str);
				SDL_Quit();
				exit(1);
			    }
			have_stencil=0;

    	}
#ifdef WINDOWS
//try to see if we get hardware acceleration, or the windows generic shit
	{
			int len;
			GLubyte *my_string;
			int have_hardware;

			my_string=(GLubyte *)glGetString(GL_RENDERER);
			len=strlen(my_string);
			have_hardware=get_string_occurance("gdi generic",my_string,len,0);
			if(have_hardware==-1)goto all_ok;
			//let the user know there is a problem
			log_to_console(c_red1,"Hmm... This mode seems to fall back in software 'acceleration'.\nTrying to disable the stencil buffer.");
			//first, shut down this mode we have now.
			SDL_QuitSubSystem(SDL_INIT_VIDEO);//there is no other way to destroy this evil video mode...
			SDL_Init(SDL_INIT_VIDEO);//restart SDL
			SDL_GL_SetAttribute( SDL_GL_RED_SIZE, rgb_size[0] );
			SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, rgb_size[1] );
			SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, rgb_size[2] );
			SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 0);
			SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24);
			SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 0);
			SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1);
			if(full_screen)flags=SDL_OPENGL|SDL_FULLSCREEN;
			SDL_SetVideoMode(window_width, window_height, bpp, flags);
			have_stencil=0;

			my_string=(GLubyte *)glGetString(GL_RENDERER);
			len=strlen(my_string);
			have_hardware=get_string_occurance("gdi generic",my_string,len,0);
			if(have_hardware==-1)goto all_ok;
			//wtf, this really shouldn't happen....
			//let's try a default mode, maybe Quake 2's mode, and pray it works
			log_to_console(c_red1,"Hmm... No luck without a stencil buffer either...\nLet's try one more thing...");
			SDL_QuitSubSystem(SDL_INIT_VIDEO);//there is no other way to destroy this evil video mode...
			SDL_Init(SDL_INIT_VIDEO);//restart SDL
			SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
			SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
			SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
			SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 0);
			SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24);
			SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 0);
			SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1);
			flags=SDL_OPENGL|SDL_FULLSCREEN;
			full_screen=1;
			video_mode=2;
			window_width=640;
			window_height=480;
			bpp=32;
			SDL_SetVideoMode(window_width, window_height, bpp, flags);
			//see if it worked...
			my_string=(GLubyte *)glGetString(GL_RENDERER);
			len=strlen(my_string);
			have_hardware=get_string_occurance("gdi generic",my_string,len,0);
			if(have_hardware==-1)goto all_ok;
			//wtf, this really shouldn't happen....
			//let's try a default mode, maybe Quake 2's mode, and pray it works
			log_to_console(c_red1,"Damn, it seems that you are out of luck, we are in the software mode now, so the game will be veeeeery slow. If you DO have a 3D accelerated card, try to update your OpenGl drivers...");


all_ok:
	}
#endif

	SDL_WM_SetIcon(SDL_LoadBMP("icon.bmp"), NULL);
	/* Set the window manager title bar */
	SDL_WM_SetCaption( "Eternal Lands", "eternallands" );

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	//glDepthFunc(GL_LEQUAL);
    glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_NORMALIZE);
	glClearColor( 0.0, 0.0, 0.0, 0.0 );
	glClearStencil(0);

	SDL_EnableKeyRepeat(200, 100);
	SDL_EnableUNICODE(1);
	build_video_mode_array();
	SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &have_stencil);
}


void resize_window()
{
	float window_ratio;
	if (window_height==0)window_height=1;			// Prevent A Divide By Zero

	glViewport(0, 0, window_width, window_height);					// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();							// Reset The Projection Matrix

	window_ratio=(GLfloat)window_width/(GLfloat)window_height;

	//reference one
	glOrtho( -3.0*window_ratio, 3.0*window_ratio, -3.0, 3.0, -40.0, 40.0 );

	//some zoom test
	//glOrtho( -3.6*window_ratio, 3.6*window_ratio, -3.6, 3.6, -40.0, 40.0 );

	glMatrixMode(GL_MODELVIEW);						// Select The Modelview Matrix
	glLoadIdentity();							// Reset The Modelview Matrix
}


void set_new_video_mode(int fs,int mode)
{
	int i;
	int alpha;

	full_screen=fs;
	video_mode=mode;

	//now, clear all the textures...
	for(i=0;i<1000;i++)
		{
			if(texture_cache[i].file_name[0])
				{
					glDeleteTextures(1,&texture_cache[i].texture_id);
					texture_cache[i].texture_id=0;//force a reload
				}
		}

	//do the same for the actors textures...
	i=0;
	while(i<1000)
		{
			if(actors_list[i])
				{
					if(actors_list[i]->remapped_colors || actors_list[i]->is_enhanced_model)//if it is not remapable, then it is already in the cache
						{
							glDeleteTextures(1,&actors_list[i]->texture_id);
							actors_list[i]->texture_id=0;
						}
				}
			i++;
		}

	//destroy the current context
	SDL_QuitSubSystem(SDL_INIT_VIDEO);


	//setup_video_mode();
	init_video();
	resize_window();
	init_lights();
	disable_local_lights();
	reset_material();

	//reload the cursors
	load_cursors();
	build_cursors();
	change_cursor(current_cursor);


	//now, reload the textures
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

	//do the same for the actors textures...
	i=0;
	while(i<1000)
		{
			if(actors_list[i])
				{
					if(actors_list[i]->remapped_colors)//if it is not remapable, then it is already in the cache
						{
							//reload the skin
							actors_list[i]->texture_id=load_bmp8_remapped_skin(actors_list[i]->skin_name,
							150,actors_list[i]->skin,actors_list[i]->hair,actors_list[i]->shirt,
							actors_list[i]->pants,actors_list[i]->boots);
						}
					if(actors_list[i]->is_enhanced_model)
						{
							actors_list[i]->texture_id=load_bmp8_enhanced_actor(actors_list[i]->body_parts, 255);
						}
				}
			i++;
		}

	//it is dependent on the window height...
	init_peace_icons_position();
	new_minute();

}

void toggle_full_screen()
{
	full_screen=!full_screen;
	set_new_video_mode(full_screen,video_mode);
	build_video_mode_array();
}




