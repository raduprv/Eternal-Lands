#include <stdlib.h>
#include <string.h>
#include "global.h"

Uint32 flags;

int window_width=640;
int window_height=480;

int desktop_width;
int desktop_height;

int bpp=0;
int have_stencil=1;
int video_mode;
int full_screen;

int have_multitexture=0;
int use_vertex_array=0;
int use_point_particles=1;
int vertex_arrays_built=0;
int have_compiled_vertex_array=0;
int have_point_sprite=0;
int have_arb_compression=0;
int have_s3_compression=0;
int have_sgis_generate_mipmap=0;
int use_mipmaps=0;
int have_arb_shadow=0;

void (APIENTRY * ELglMultiTexCoord2fARB) (GLenum target, GLfloat s, GLfloat t);
void (APIENTRY * ELglMultiTexCoord2fvARB) (GLenum target, const GLfloat *v);
void (APIENTRY * ELglActiveTextureARB) (GLenum texture);
void (APIENTRY * ELglClientActiveTextureARB) (GLenum texture);
void (APIENTRY * ELglLockArraysEXT) (GLint first, GLsizei count);
void (APIENTRY * ELglUnlockArraysEXT) (void);
void (APIENTRY * ELglClientActiveTextureARB) (GLenum texture);

void setup_video_mode()
{
	if(full_screen)
		{
			switch(video_mode) {
			case 1:
				window_width=640;
				window_height=480;
				bpp=16;
				break;
			case 2:
				window_width=640;
				window_height=480;
				bpp=32;
				break;
			case 3:
				window_width=800;
				window_height=600;
				bpp=16;
				break;
			case 4:
				window_width=800;
				window_height=600;
				bpp=32;
				break;
			case 5:
				window_width=1024;
				window_height=768;
				bpp=16;
				break;
			case 6:
				window_width=1024;
				window_height=768;
				bpp=32;
				break;
			case 7:
				window_width=1152;
				window_height=864;
				bpp=16;
				break;
			case 8:
				window_width=1152;
				window_height=864;
				bpp=32;
				break;
			case 9:
				window_width=1280;
				window_height=1024;
				bpp=16;
				break;
			case 10:
				window_width=1280;
				window_height=1024;
				bpp=32;
				break;
			}
		}
	else //windowed mode
		{
			switch(video_mode) {
			case 1:
			case 2:
				if(window_width != 640 || window_height != 550)
					{
						Uint8 str[100];
						sprintf(str,window_size_adjusted_str,"640x480");
						log_to_console(c_yellow1,str);
					}
				window_width=640;
				window_height=480;
				break;
			case 3:
			case 4:
				if(window_width != 780 || window_height != 550)
					{
						Uint8 str[100];
						sprintf(str,window_size_adjusted_str,"780x550");
						log_to_console(c_yellow1,str);
					}
				window_width=780;
				window_height=550;
				break;
			case 5:
			case 6:
				if(window_width != 990 || window_height != 720)
					{
						Uint8 str[100];
						sprintf(str,window_size_adjusted_str,"990x720");
						log_to_console(c_yellow1,str);
					}
				window_width=990;
				window_height=720;
				break;
			case 7:
			case 8:
				if(window_width != 1070 || window_height != 785)
					{
						Uint8 str[100];
						sprintf(str,window_size_adjusted_str,"1070x785");
						log_to_console(c_yellow1,str);
					}
				window_width=1070;
				window_height=785;
				break;
			case 9:
			case 10:
				if(window_width != 1250 || window_height != 990)
					{
						Uint8 str[100];
						sprintf(str,window_size_adjusted_str,"1250x990");
						log_to_console(c_yellow1,str);
					}
				window_width=1250;
				window_height=990;
				break;
			}
			bpp=0;//autodetect
		}
#ifndef WINDOWS
	bpp=0;//under X, we can't change the desktop BPP
#endif
}

void check_gl_mode()
{
	char str[400];


	if(full_screen)flags=SDL_OPENGL|SDL_FULLSCREEN;
	else flags=SDL_OPENGL;
	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8);

	//now, test if the video mode is OK...
	if(!SDL_VideoModeOK(window_width, window_height, bpp, flags))
		{
			char vid_mode_str[25];
			sprintf(vid_mode_str,"%ix%ix%i",window_width,window_height,bpp);
			sprintf(str,no_stencil_str,vid_mode_str);
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

					sprintf(vid_mode_str,"%ix%ix%i",old_width,old_height,old_bpp);
					sprintf(str,safemode_str,vid_mode_str);
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
			sprintf(str, "%s: %s\n", no_sdl_str, SDL_GetError());
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
	if(bpp==16) {
		if(!(video_mode%2))
			video_mode-=1;
	} else {
		if(video_mode%2)
			video_mode+=1;
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
			log_to_console(c_red1,no_hardware_stencil_str);
			if(bpp!=32)log_to_console(c_grey1,suggest_24_or_32_bit);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,16);
			SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,0);
			if(!SDL_SetVideoMode( window_width, window_height, bpp, flags))
			    {
					char str[120];
					sprintf(str, "%s: %s\n", fail_opengl_mode, SDL_GetError());
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
		log_to_console(c_red1,stencil_falls_back_on_software_accel);
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
		log_to_console(c_red1,last_chance_str);
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
		log_to_console(c_red1,software_mode_str);


	all_ok:;
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

	glFogi(GL_FOG_MODE,GL_LINEAR);
	glFogf(GL_FOG_START,5.0);
	glFogf(GL_FOG_END,35.0);

	SDL_EnableKeyRepeat(200, 100);
	SDL_EnableUNICODE(1);
	build_video_mode_array();
	SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &have_stencil);
	last_texture=-1;	//no active texture

	set_shadow_map_size();
}

void init_gl_extensions()
{
	Uint8 * extensions;
	int ext_str_len;
	char str[150];
	//now load the multitexturing extension
	ELglActiveTextureARB = SDL_GL_GetProcAddress("glActiveTextureARB");
	ELglMultiTexCoord2fARB = SDL_GL_GetProcAddress("glMultiTexCoord2fARB");
	ELglMultiTexCoord2fvARB	= SDL_GL_GetProcAddress("glMultiTexCoord2fvARB");
	ELglClientActiveTextureARB = SDL_GL_GetProcAddress("glClientActiveTextureARB");
	ELglLockArraysEXT = SDL_GL_GetProcAddress("glLockArraysEXT");
	ELglUnlockArraysEXT = SDL_GL_GetProcAddress("glUnlockArraysEXT");

	//see if we really have multitexturing
	extensions=(GLubyte *)glGetString(GL_EXTENSIONS);
	ext_str_len=strlen(extensions);
	if(ELglActiveTextureARB && ELglMultiTexCoord2fARB && ELglMultiTexCoord2fvARB && ELglClientActiveTextureARB)
		{
			have_multitexture=get_string_occurance("GL_ARB_multitexture",extensions,ext_str_len,0);
			if(have_multitexture==-1)
				{
					have_multitexture=0;
					log_to_console(c_red1,gl_ext_no_multitexture);
				}
			else
				{
					glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB,&have_multitexture);
					sprintf(str,gl_ext_found,"GL_ARB_multitexture");
					log_to_console(c_green2,str);
				}
		}
	else
		{
			have_multitexture=0;
			log_to_console(c_red1,gl_ext_no_multitexture);
		}
	if(ELglLockArraysEXT && ELglUnlockArraysEXT)
		{
			have_compiled_vertex_array=get_string_occurance("GL_EXT_compiled_vertex_array",extensions,ext_str_len,0);
			if(have_compiled_vertex_array < 0)
				{
					have_compiled_vertex_array=0;
					sprintf(str,gl_ext_not_found,"GL_EXT_compiled_vertex_array");
					log_to_console(c_red1,str);
				}
			else 
				{
					sprintf(str,gl_ext_found,"GL_EXT_compiled_vertex_array");
					log_to_console(c_green2,str);
				}
		}
	else
		{
			have_compiled_vertex_array=0;
			sprintf(str,gl_ext_not_found,"GL_EXT_compiled_vertex_array");
			log_to_console(c_red1,str);

		}

		have_point_sprite=get_string_occurance("GL_ARB_point_sprite",extensions,ext_str_len,0)>=0 || get_string_occurance("GL_NV_point_sprite",extensions,ext_str_len,0)>=0;
		if(!have_point_sprite)
			{
				have_point_sprite=0;
				sprintf(str,gl_ext_not_found,"GL_*_point_sprite");
				log_to_console(c_red1,str);
			}
		else if(!use_point_particles)
			{
				have_point_sprite=0;
				sprintf(str,gl_ext_found_not_used,"GL_*_point_sprite");
				log_to_console(c_green2,str);
			}
		else 
			{
				have_point_sprite=1;
				sprintf(str,gl_ext_found,"GL_*_point_sprite");
				log_to_console(c_green2,str);
			}
		have_arb_compression=get_string_occurance("GL_ARB_texture_compression",extensions,ext_str_len,0);

		if(have_arb_compression<0) have_arb_compression=0;
		else
			{
				have_arb_compression=1;
				sprintf(str,gl_ext_found,"GL_ARB_texture_compression");
				log_to_console(c_green2,str);
			}
		have_s3_compression=get_string_occurance("GL_EXT_texture_compression_s3tc",extensions,ext_str_len,0);
		if(have_s3_compression<0) have_s3_compression=0;
		else
			{
				have_s3_compression=1;
				sprintf(str,gl_ext_found,"GL_EXT_texture_compression_s3tc");
				log_to_console(c_green2,str);
			}

		have_sgis_generate_mipmap=get_string_occurance("GL_SGIS_generate_mipmap",extensions,ext_str_len,0);
		if(have_sgis_generate_mipmap<0)
			{
				have_sgis_generate_mipmap=0;
				use_mipmaps=0;
				sprintf(str,gl_ext_not_found,"GL_SGIS_generate_mipmap");
				log_to_console(c_red1,str);
			}
		else if(!use_mipmaps)
			{
				have_sgis_generate_mipmap=0;
				sprintf(str,gl_ext_found_not_used,"GL_SGIS_generate_mipmap");
				log_to_console(c_green2,str);
			}
		else 
			{
				have_sgis_generate_mipmap=1;
				sprintf(str,gl_ext_found,"GL_SGIS_generate_mipmap");
				log_to_console(c_green2,str);
			}

		have_arb_shadow=get_string_occurance("GL_ARB_shadow",extensions,ext_str_len,0);
		if(have_arb_shadow<0)have_arb_shadow=0;
		else
			{
				have_arb_shadow=1;
				sprintf(str,gl_ext_found,"GL_ARB_shadow");
				log_to_console(c_green2,str);
			}

		if((have_multitexture<3 || !have_arb_shadow) && use_shadow_mapping)
			{
				use_shadow_mapping=0;
				log_to_console(c_red1,disabled_shadow_mapping);
			}

	check_gl_errors();
}

void resize_window()
{
	float window_ratio;
	//float hud_x_adjust=0;
	//float hud_y_adjust=0;

	if (window_height==0)window_height=1;			// Prevent A Divide By Zero

	//glViewport(0, hud_y, window_width-hud_x, window_height);	// Reset The Current Viewport
	//glViewport(0, 0, window_width-hud_x, -(window_height-hud_y));	// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);					// Select The Projection Matrix
	glLoadIdentity();							// Reset The Projection Matrix

	//window_ratio=(GLfloat)(window_width-hud_x)/(GLfloat)(window_height-hud_y);
	window_ratio=(GLfloat)window_width/(GLfloat)window_height;

	//hud_y_adjust=(2.0/window_height)*hud_y;
	//hud_x_adjust=(2.0/window_width)*hud_x;

	//reference one
	//glOrtho( -3.0*window_ratio, 3.0*window_ratio, -3.0, 3.0, -40.0, 40.0 );

	//some zoom test
	//glOrtho( -3.6*window_ratio, 3.6*window_ratio, -3.6, 3.6, -40.0, 40.0 );

	//new zoom
	glOrtho( -1.0*zoom_level*window_ratio, 1.0*zoom_level*window_ratio, -1.0*zoom_level, 1.0*zoom_level, -40.0, 40.0 );
	//glOrtho( (-1.0-hud_x_adjust)*zoom_level*window_ratio, (1.0-hud_x_adjust)*zoom_level*window_ratio, (-1.0+hud_y_adjust)*zoom_level, (1.0+hud_y_adjust)*zoom_level, -40.0, 40.0 );
	//glOrtho( -1.0*zoom_level*window_ratio, 1.0*zoom_level*window_ratio, -0.0*zoom_level, 2.0*zoom_level, -40.0, 40.0 );

	glMatrixMode(GL_MODELVIEW);					// Select The Modelview Matrix
	glLoadIdentity();							// Reset The Modelview Matrix
	last_texture=-1;	//no active texture
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
	for(i=0;i<max_actors;i++)
		{
			if(actors_list[i])
				{
					if(actors_list[i]->remapped_colors || actors_list[i]->is_enhanced_model)//if it is not remapable, then it is already in the cache
						{
							glDeleteTextures(1,&actors_list[i]->texture_id);
							actors_list[i]->texture_id=0;
						}
				}
		}

	//...and the texture used for shadow mapping
	glDeleteTextures(1,&depth_map_id);
	depth_map_id=0;

	//destroy the current context
	SDL_QuitSubSystem(SDL_INIT_VIDEO);

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
	for(i=0;i<max_actors;i++)
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
		}

	//it is dependent on the window height...
	init_hud_interface();
	new_minute();

}

void toggle_full_screen()
{
	full_screen=!full_screen;
	set_new_video_mode(full_screen,video_mode);
	build_video_mode_array();
}


int print_gl_errors(char *file, char *func, int line)
{
	char str[1024];
	int	glErr, anyErr=GL_NO_ERROR;

	while ((glErr=glGetError()) != GL_NO_ERROR )
		 {
			anyErr=glErr;
#ifdef	GLUT
			snprintf(str, 1024, "OpenGL %s", gluErrorString(glErr));
#else	// GLUT
			snprintf(str, 1024, "OpenGL error %d", glErr);
#endif	// GLUT
			log_error_detailed(str, file, func, line);
		}
	return anyErr;
}
