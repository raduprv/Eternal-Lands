#include <stdlib.h>
#include <string.h>
#ifdef MAP_EDITOR2
#include "../map_editor2/global.h"
#else
#include "global.h"
#endif

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
int use_vertex_array=1;
int use_vertex_buffers=0;
int vertex_arrays_built=0;
int have_compiled_vertex_array=0;
int have_point_sprite=0;
int have_arb_compression=0;
int have_s3_compression=0;
int have_sgis_generate_mipmap=0;
int use_mipmaps=0;
int have_arb_shadow=0;
int have_vertex_buffers=0;
int have_framebuffer_object=0;
int have_arb_pixel_shader=0;
int have_arb_vertex_shader=0;
int have_ogsl_pixel_shader=0;
int have_ogsl_vertex_shader=0;
float gamma_var = 1.00f;
float perspective = 0.15f;
float near_plane = 40.0f; // don't cut off anything
#ifdef	USE_FRAMEBUFFER
int have_texture_non_power_of_two = 0;
int use_frame_buffer = 0;
#endif

struct list {
	int i;
	struct list * next;
} * list;

void (APIENTRY * ELglMultiTexCoord2fARB) (GLenum target, GLfloat s, GLfloat t);
void (APIENTRY * ELglMultiTexCoord2fvARB) (GLenum target, const GLfloat *v);
void (APIENTRY * ELglActiveTextureARB) (GLenum texture);
void (APIENTRY * ELglLockArraysEXT) (GLint first, GLsizei count);
void (APIENTRY * ELglUnlockArraysEXT) (void);
void (APIENTRY * ELglClientActiveTextureARB) (GLenum texture);
void (APIENTRY * ELglBindBufferARB)(GLenum target, GLuint buffer);
void (APIENTRY * ELglBufferDataARB)(GLenum target, GLsizeiptrARB size, const void * data, GLenum usage);
void (APIENTRY * ELglGenBuffersARB)(GLsizei no, GLuint *buffer);
void (APIENTRY * ELglDeleteBuffersARB)(GLsizei no, const GLuint *buffer);
void (APIENTRY * ELglGenRenderbuffersEXT)(GLsizei n, GLuint * renderbuffers);
void (APIENTRY * ELglDeleteRenderbuffersEXT)(GLsizei n, const GLuint * renderbuffers);
void (APIENTRY * ELglBindRenderbufferEXT)(GLenum target, GLuint renderbuffer);
void (APIENTRY * ELglRenderbufferStorageEXT)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
void (APIENTRY * ELglGenFramebuffersEXT)(GLsizei n, GLuint * framebuffers);
void (APIENTRY * ELglDeleteFramebuffersEXT)(GLsizei n, const GLuint * framebuffers);
void (APIENTRY * ELglBindFramebufferEXT)(GLsizei n, GLuint framebuffer);
void (APIENTRY * ELglGenProgramsARB)(GLsizei n, GLuint * programs);
void (APIENTRY * ELglDeleteProgramsARB)(GLsizei n, const GLuint * programs);
void (APIENTRY * ELglBindProgramARB)(GLenum type, GLuint program);
void (APIENTRY * ELglProgramStringARB)(GLenum type, GLenum format, GLsizei length, const char * program);
void (APIENTRY * ELglDeleteObjectARB)(GLhandleARB obj);
GLhandleARB (APIENTRY * ELglCreateShaderObjectARB)(GLenum type);
void (APIENTRY * ELglShaderSourceARB)(GLhandleARB shader, GLsizei count, const char ** string, const int * length);
void (APIENTRY * ELglCompileShaderARB)(GLhandleARB shader);
GLhandleARB (APIENTRY * ELglCreateProgramObjectARB)(void);
void (APIENTRY * ELglAttachObjectARB)(GLhandleARB program, GLhandleARB shader);
void (APIENTRY * ELglLinkProgramARB)(GLhandleARB program);
void (APIENTRY * ELglUseProgramObjectARB)(GLhandleARB program);
#ifdef	TERRAIN
GLint (APIENTRY * ELglGetUniformLocationARB)(GLhandleARB program, const char * name);
void (APIENTRY * ELglUniform1iARB)(GLint location, GLint v0);
#endif
#ifdef	USE_FRAMEBUFFER
GLboolean (APIENTRY * ELglIsRenderbufferEXT) (GLuint renderbuffer);
void (APIENTRY * ELglGetRenderbufferParameterivEXT) (GLenum target, GLenum pname, GLint *params);
GLboolean (APIENTRY * ELglIsFramebufferEXT) (GLuint framebuffer);
GLenum (APIENTRY * ELglCheckFramebufferStatusEXT) (GLenum target);
void (APIENTRY * ELglFramebufferTexture1DEXT) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
void (APIENTRY * ELglFramebufferTexture2DEXT) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
void (APIENTRY * ELglFramebufferTexture3DEXT) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
void (APIENTRY * ELglFramebufferRenderbufferEXT) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
void (APIENTRY * ELglGetFramebufferAttachmentParameterivEXT) (GLenum target, GLenum attachment, GLenum pname, GLint *params);
void (APIENTRY * ELglGenerateMipmapEXT) (GLenum target);
#endif

void setup_video_mode(int fs, int mode)
{
	if(fs)
		{
			switch(mode) {
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
			case 11:
				window_width=1600;
				window_height=1200;
				bpp=16;
				break;
			case 12:
				window_width=1600;
				window_height=1200;
				bpp=32;
				break;
			}
		}
	else //windowed mode
		{
			switch(mode) {
			case 1:
			case 2:
				if(window_width != 640 || window_height != 480)
					{
						Uint8 str[100];
						snprintf(str,sizeof(str),window_size_adjusted_str,"640x480");
						LOG_TO_CONSOLE(c_yellow1,str);
					}
				window_width=640;
				window_height=480;
				break;
			case 3:
			case 4:
				if(window_width != 780 || window_height != 550)
					{
						Uint8 str[100];
						snprintf(str,sizeof(str),window_size_adjusted_str,"780x550");
						LOG_TO_CONSOLE(c_yellow1,str);
					}
				window_width=780;
				window_height=550;
				break;
			case 5:
			case 6:
				if(window_width != 990 || window_height != 720)
					{
						Uint8 str[100];
						snprintf(str,sizeof(str),window_size_adjusted_str,"990x720");
						LOG_TO_CONSOLE(c_yellow1,str);
					}
				window_width=990;
				window_height=720;
				break;
			case 7:
			case 8:
				if(window_width != 1070 || window_height != 785)
					{
						Uint8 str[100];
						snprintf(str,sizeof(str),window_size_adjusted_str,"1070x785");
						LOG_TO_CONSOLE(c_yellow1,str);
					}
				window_width=1070;
				window_height=785;
				break;
			case 9:
			case 10:
				if(window_width != 1250 || window_height != 990)
					{
						Uint8 str[100];
						snprintf(str,sizeof(str),window_size_adjusted_str,"1250x990");
						LOG_TO_CONSOLE(c_yellow1,str);
					}
				window_width=1250;
				window_height=990;
				break;
			case 11:
			case 12:
				if(window_width != 1600 || window_height != 1200)
				{
					Uint8 str[100];
					snprintf(str,sizeof(str),window_size_adjusted_str,"1600x1200");
					LOG_TO_CONSOLE(c_yellow1,str);
				}
				window_width=1600;
				window_height=1200;
				break;
			}
//TODO: Add wide screen resolutions
//1280x800
//1400x1050
//1440x900
//1680x1050
			bpp=0;//autodetect
		}
#ifndef WINDOWS
	bpp=0;//under X, we can't change the desktop BPP
#endif
}

void check_gl_mode()
{
	char str[400];

	flags = SDL_OPENGL;
	if(full_screen) {
		flags |= SDL_FULLSCREEN;
	}
	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8);

	//now, test if the video mode is OK...
	if(!SDL_VideoModeOK(window_width, window_height, bpp, flags))
		{
			char vid_mode_str[25];
			snprintf (vid_mode_str, sizeof (vid_mode_str), "%ix%ix%i", window_width, window_height, bpp);
			snprintf(str,sizeof(str),no_stencil_str,vid_mode_str);
			LOG_TO_CONSOLE(c_red1,str);

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

					snprintf (vid_mode_str, sizeof (vid_mode_str), "%ix%ix%i", old_width, old_height, old_bpp);
					snprintf(str,sizeof(str),safemode_str,vid_mode_str);
					LOG_TO_CONSOLE(c_red1,str);

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
			log_error("%s: %s\n", no_sdl_str, SDL_GetError());
			SDL_Quit();
			exit(1);
		}

	setup_video_mode(full_screen, video_mode);

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
			LOG_TO_CONSOLE(c_red1,no_hardware_stencil_str);
			if(bpp!=32)LOG_TO_CONSOLE(c_grey1,suggest_24_or_32_bit);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,16);
			SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,0);
			if(!SDL_SetVideoMode( window_width, window_height, bpp, flags))
			    {
					log_error("%s: %s\n", fail_opengl_mode, SDL_GetError());
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
		if(have_hardware != -1) {
			//let the user know there is a problem
			LOG_TO_CONSOLE(c_red1,stencil_falls_back_on_software_accel);
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
			if(have_hardware != -1) {
				//wtf, this really shouldn't happen....
				//let's try a default mode, maybe Quake 2's mode, and pray it works
				LOG_TO_CONSOLE(c_red1,last_chance_str);
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
				if(have_hardware != -1) {
					//wtf, this really shouldn't happen....
					//let's try a default mode, maybe Quake 2's mode, and pray it works
					LOG_TO_CONSOLE(c_red1,software_mode_str);
				}
			}
		}
	}
#endif

	SDL_WM_SetIcon(SDL_LoadBMP("icon.bmp"), NULL);
	/* Set the window manager title bar */
#ifdef MAP_EDITOR2
	SDL_WM_SetCaption( "Map Editor", "mapeditor" );
#else
	SDL_WM_SetCaption( "Eternal Lands", "eternallands" );
#endif

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	//glDepthFunc(GL_LEQUAL);
    glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_NORMALIZE);
	glClearStencil(0);

#ifdef ANTI_ALIAS
	if (anti_alias) {
		glHint(GL_POINT_SMOOTH_HINT,   GL_NICEST);	
		glHint(GL_LINE_SMOOTH_HINT,    GL_NICEST);	
		glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);	
		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POLYGON_SMOOTH);
	} else {
		glHint(GL_POINT_SMOOTH_HINT,   GL_FASTEST);	
		glHint(GL_LINE_SMOOTH_HINT,    GL_FASTEST);	
		glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);	
		glDisable(GL_POINT_SMOOTH);
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_POLYGON_SMOOTH);
	}
#endif

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
	ELglBindBufferARB = SDL_GL_GetProcAddress("glBindBufferARB");
	ELglGenBuffersARB = SDL_GL_GetProcAddress("glGenBuffersARB");
	ELglDeleteBuffersARB = SDL_GL_GetProcAddress("glDeleteBuffersARB");
	ELglBufferDataARB = SDL_GL_GetProcAddress("glBufferDataARB");
	ELglGenRenderbuffersEXT=SDL_GL_GetProcAddress("glGenRenderbuffersEXT");
	ELglDeleteRenderbuffersEXT=SDL_GL_GetProcAddress("glDeleteRenderbuffersEXT");
	ELglBindRenderbufferEXT=SDL_GL_GetProcAddress("glBindRenderbufferEXT");
	ELglRenderbufferStorageEXT=SDL_GL_GetProcAddress("glRenderbufferStorageEXT");
	ELglGenFramebuffersEXT=SDL_GL_GetProcAddress("glGenFramebuffersEXT");
	ELglDeleteFramebuffersEXT=SDL_GL_GetProcAddress("glDeleteFramebuffersEXT");
	ELglBindFramebufferEXT=SDL_GL_GetProcAddress("glBindFramebufferEXT");
	ELglGenProgramsARB=SDL_GL_GetProcAddress("glGenProgramsARB");
	ELglDeleteProgramsARB=SDL_GL_GetProcAddress("glDeleteProgramsARB");
	ELglBindProgramARB=SDL_GL_GetProcAddress("glBindProgramARB");
	ELglProgramStringARB=SDL_GL_GetProcAddress("glProgramStringARB");
	ELglCreateShaderObjectARB=SDL_GL_GetProcAddress("glCreateShaderObjectARB");
	ELglShaderSourceARB=SDL_GL_GetProcAddress("glShaderSourceARB");
	ELglCompileShaderARB=SDL_GL_GetProcAddress("glCompileShaderARB");
	ELglCreateProgramObjectARB=SDL_GL_GetProcAddress("glCreateProgramObjectARB");
	ELglAttachObjectARB=SDL_GL_GetProcAddress("glAttachObjectARB");
	ELglLinkProgramARB=SDL_GL_GetProcAddress("glLinkProgramARB");
	ELglUseProgramObjectARB=SDL_GL_GetProcAddress("glUseProgramObjectARB");
#ifdef	TERRAIN
	ELglDeleteObjectARB=SDL_GL_GetProcAddress("glDeleteObjectARB");
	ELglGetUniformLocationARB=SDL_GL_GetProcAddress("glGetUniformLocationARB");
	ELglUniform1iARB=SDL_GL_GetProcAddress("glUniform1iARB");
#endif
#ifdef	USE_FRAMEBUFFER
	ELglIsRenderbufferEXT=SDL_GL_GetProcAddress("glIsRenderbufferEXT");
	ELglGetRenderbufferParameterivEXT=SDL_GL_GetProcAddress("glGetRenderbufferParameterivEXT");
	ELglIsFramebufferEXT=SDL_GL_GetProcAddress("glIsFramebufferEXT");
	ELglCheckFramebufferStatusEXT=SDL_GL_GetProcAddress("glCheckFramebufferStatusEXT");
	ELglFramebufferTexture1DEXT=SDL_GL_GetProcAddress("glFramebufferTexture1DEXT");
	ELglFramebufferTexture2DEXT=SDL_GL_GetProcAddress("glFramebufferTexture2DEXT");
	ELglFramebufferTexture3DEXT=SDL_GL_GetProcAddress("glFramebufferTexture3DEXT");
	ELglFramebufferRenderbufferEXT=SDL_GL_GetProcAddress("glFramebufferRenderbufferEXT");
	ELglGetFramebufferAttachmentParameterivEXT=SDL_GL_GetProcAddress("glGetFramebufferAttachmentParameterivEXT");
	ELglGenerateMipmapEXT=SDL_GL_GetProcAddress("glGenerateMipmapEXT");
#endif

	//see if we really have multitexturing
	extensions=(GLubyte *)glGetString(GL_EXTENSIONS);
	ext_str_len=strlen(extensions);
	if(ELglActiveTextureARB && ELglMultiTexCoord2fARB && ELglMultiTexCoord2fvARB && ELglClientActiveTextureARB) {
		have_multitexture=get_string_occurance("GL_ARB_multitexture",extensions,ext_str_len,0);
		if(have_multitexture==-1) {
			have_multitexture=0;
			LOG_TO_CONSOLE(c_red1,gl_ext_no_multitexture);
		} else {
			glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB,&have_multitexture);
			snprintf(str,sizeof(str),gl_ext_found,"GL_ARB_multitexture");
			LOG_TO_CONSOLE(c_green2,str);
		}
	} else {
		have_multitexture=0;
		LOG_TO_CONSOLE(c_red1,gl_ext_no_multitexture);
	}
	
	if(ELglLockArraysEXT && ELglUnlockArraysEXT){
		have_compiled_vertex_array=get_string_occurance("GL_EXT_compiled_vertex_array",extensions,ext_str_len,0);
		if(have_compiled_vertex_array < 0) {
			have_compiled_vertex_array=0;
			snprintf(str,sizeof(str),gl_ext_not_found,"GL_EXT_compiled_vertex_array");
			LOG_TO_CONSOLE(c_red1,str);
		} else {
			snprintf(str,sizeof(str),gl_ext_found,"GL_EXT_compiled_vertex_array");
			LOG_TO_CONSOLE(c_green2,str);
		}
	} else {
		have_compiled_vertex_array=0;
		snprintf(str,sizeof(str),gl_ext_not_found,"GL_EXT_compiled_vertex_array");
		LOG_TO_CONSOLE(c_red1,str);
	}

	have_point_sprite = get_string_occurance("GL_ARB_point_sprite",extensions,ext_str_len,0)>=0 || get_string_occurance("GL_NV_point_sprite",extensions,ext_str_len,0)>=0;
	if(!have_point_sprite){
		use_point_particles = 0;
		snprintf(str,sizeof(str),gl_ext_not_found,"GL_*_point_sprite");
		LOG_TO_CONSOLE(c_red1,str);
	} else if(!use_point_particles) {
		snprintf(str,sizeof(str),gl_ext_found_not_used,"GL_*_point_sprite");
		LOG_TO_CONSOLE(c_green2,str);
	} else {
		snprintf(str,sizeof(str),gl_ext_found,"GL_*_point_sprite");
		LOG_TO_CONSOLE(c_green2,str);
	}
	
	have_arb_compression=get_string_occurance("GL_ARB_texture_compression",extensions,ext_str_len,0);

	if(have_arb_compression<0) {
		have_arb_compression=0;
	} else {
		have_arb_compression=1;
		snprintf(str,sizeof(str),gl_ext_found,"GL_ARB_texture_compression");
		LOG_TO_CONSOLE(c_green2,str);
	}
	
	have_s3_compression=get_string_occurance("GL_EXT_texture_compression_s3tc",extensions,ext_str_len,0);
	if(have_s3_compression<0) {
		have_s3_compression=0;
	} else {
		have_s3_compression=1;
		snprintf(str,sizeof(str),gl_ext_found,"GL_EXT_texture_compression_s3tc");
		LOG_TO_CONSOLE(c_green2,str);
	}

	have_sgis_generate_mipmap=get_string_occurance("GL_SGIS_generate_mipmap",extensions,ext_str_len,0);
	
	if(have_sgis_generate_mipmap<0)	{
		have_sgis_generate_mipmap=0;
		use_mipmaps=0;
		snprintf(str,sizeof(str),gl_ext_not_found,"GL_SGIS_generate_mipmap");
		LOG_TO_CONSOLE(c_red1,str);
	} else if(!use_mipmaps) {
		have_sgis_generate_mipmap=0;
		snprintf(str,sizeof(str),gl_ext_found_not_used,"GL_SGIS_generate_mipmap");
		LOG_TO_CONSOLE(c_green2,str);
	} else  {
		have_sgis_generate_mipmap=1;
		snprintf(str,sizeof(str),gl_ext_found,"GL_SGIS_generate_mipmap");
		LOG_TO_CONSOLE(c_green2,str);
	}

	have_arb_shadow=get_string_occurance("GL_ARB_shadow",extensions,ext_str_len,0);
	if(have_arb_shadow<0){
		have_arb_shadow=0;
	} else {
		have_arb_shadow=1;
		snprintf(str,sizeof(str),gl_ext_found,"GL_ARB_shadow");
		LOG_TO_CONSOLE(c_green2,str);
	}

	if((have_multitexture<3 || !have_arb_shadow) && use_shadow_mapping) {
		use_shadow_mapping=0;
		LOG_TO_CONSOLE(c_red1,disabled_shadow_mapping);
	}

	if(get_string_occurance("GL_ARB_vertex_buffer_object",extensions,ext_str_len,0)>=0 && use_vertex_buffers){
		snprintf(str,sizeof(str),gl_ext_found,"GL_ARB_vertex_buffer_object");
		LOG_TO_CONSOLE(c_green2, str);
		have_vertex_buffers=1;
	} else {
		have_vertex_buffers=0;
	}
	
	if(ELglGenRenderbuffersEXT && ELglDeleteRenderbuffersEXT && ELglBindRenderbufferEXT && ELglRenderbufferStorageEXT &&
#ifdef	USE_FRAMEBUFFER
	   ELglIsRenderbufferEXT && ELglGetRenderbufferParameterivEXT && ELglIsFramebufferEXT && ELglCheckFramebufferStatusEXT &&
	   ELglFramebufferTexture1DEXT && ELglFramebufferTexture2DEXT && ELglFramebufferTexture3DEXT && 
	   ELglFramebufferRenderbufferEXT && ELglGetFramebufferAttachmentParameterivEXT && ELglGenerateMipmapEXT &&
#endif
	   ELglGenFramebuffersEXT && ELglDeleteFramebuffersEXT && ELglBindFramebufferEXT && strstr(extensions, "GL_EXT_framebuffer_object")){
#ifdef	USE_FRAMEBUFFER
		if (ELglCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT)
		{
			snprintf(str,sizeof(str),gl_ext_found,"GL_EXT_framebuffer_object");
			LOG_TO_CONSOLE(c_green2, str);
			have_framebuffer_object = 1;
		}
		else
		{
			snprintf(str,sizeof(str),gl_ext_not_found,"GL_EXT_framebuffer_object");
			LOG_TO_CONSOLE(c_red1, str);
			have_framebuffer_object = 0;
		}
#else
		snprintf(str,sizeof(str),gl_ext_found_not_used,"GL_EXT_framebuffer_object");
		LOG_TO_CONSOLE(c_green2, str);
		have_framebuffer_object=1;
#endif
	} else {
#ifdef	USE_FRAMEBUFFER
		snprintf(str,sizeof(str),gl_ext_not_found,"GL_EXT_framebuffer_object");
		LOG_TO_CONSOLE(c_red1, str);
		have_framebuffer_object=0;
#else
		//snprintf(str,sizeof(str),gl_ext_not_found,"GL_EXT_framebuffer_object");
#endif
	}
	
#ifdef	USE_FRAMEBUFFER
	if (strstr(extensions, "GL_ARB_texture_non_power_of_two"))
	{		
		snprintf(str, sizeof(str), gl_ext_found, "GL_ARB_texture_non_power_of_two");
		LOG_TO_CONSOLE(c_green2, str);
		have_texture_non_power_of_two = 1;
	}
#endif

	//Test for ARB{fp,vp}
	if(ELglGenProgramsARB && ELglDeleteProgramsARB && ELglBindProgramARB && ELglProgramStringARB){
		if(strstr(extensions, "GL_ARB_fragment_program")){
			snprintf(str,sizeof(str),gl_ext_found_not_used,"GL_ARB_fragment_program");
			LOG_TO_CONSOLE(c_green2, str);
			have_arb_pixel_shader=1;
		} else {
			/*
			 * snprintf(str,sizeof(str),gl_ext_not_found,"GL_ARB_fragment_program");
			 * LOG_TO_CONSOLE(c_green2, str);
			 */
		}
		if(strstr(extensions, "GL_ARB_vertex_program")){
			snprintf(str,sizeof(str),gl_ext_found_not_used,"GL_ARB_vertex_program");
			LOG_TO_CONSOLE(c_green2, str);
			have_arb_vertex_shader=1;
		} else {
			/*
			 * snprintf(str,sizeof(str),gl_ext_not_found,"GL_ARB_vertex_program");
			 * LOG_TO_CONSOLE(c_green2, str);
			 */
		}
	}  else {
		/*
		 * snprintf(str,sizeof(str),gl_ext_not_found,"GL_ARB_{fragment,vertex}_{program,shader}");
		 * LOG_TO_CONSOLE(c_green2, str);
		 */
	}
	
	//Test for OGSL
	if(ELglCreateShaderObjectARB && ELglShaderSourceARB && ELglCompileShaderARB && ELglCreateProgramObjectARB &&
	   ELglAttachObjectARB && ELglLinkProgramARB && ELglUseProgramObjectARB && 
#ifdef	TERRAIN
	   ELglDeleteObjectARB && ELglGetUniformLocationARB && ELglUniform1iARB &&
#endif
	   strstr(extensions,"GL_ARB_shader_objects") && strstr(extensions, "GL_ARB_shading_language_100")){
		if(strstr(extensions,"GL_ARB_vertex_shader")){
#ifdef	TERRAIN
			snprintf(str,sizeof(str),gl_ext_found,"GL_ARB_vertex_shader");
#else
			snprintf(str,sizeof(str),gl_ext_found_not_used,"GL_ARB_vertex_shader");
#endif
			LOG_TO_CONSOLE(c_green2, str);
			have_ogsl_vertex_shader=1;
		}
		
		if(strstr(extensions,"GL_ARB_fragment_shader")){
#ifdef	TERRAIN
			snprintf(str,sizeof(str),gl_ext_found,"GL_ARB_fragment_shader");
#else
			snprintf(str,sizeof(str),gl_ext_found_not_used,"GL_ARB_fragment_shader");
#endif
			LOG_TO_CONSOLE(c_green2, str);
			have_ogsl_pixel_shader=1;
		}
	} else {
#ifdef	TERRAIN
		snprintf(str,sizeof(str),gl_ext_not_found,"OpenGL Shading Language");
		LOG_TO_CONSOLE(c_green2, str);
#else
		/*
		 * snprintf(str,sizeof(str),gl_ext_not_found,"OpenGL Shading Language");
		 * LOG_TO_CONSOLE(c_green2, str);
		 */
#endif
	}

#ifdef	TERRAIN
	if (!have_ogsl_vertex_shader || !have_ogsl_pixel_shader)
	{
		use_normal_mapping=0;
		LOG_TO_CONSOLE(c_red1,disabled_normal_mapping);
	}
#endif
	CHECK_GL_ERRORS();
}

void resize_root_window()
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

	//new zoom
	if (isometric) {
		glOrtho( -1.0*zoom_level*window_ratio, 1.0*zoom_level*window_ratio, -1.0*zoom_level, 1.0*zoom_level, -near_plane*zoom_level, 60.0 );
	} else {
		// What we call first, OpenGL will apply last!
		// Finally, apply the projection
		glFrustum( -perspective*window_ratio, perspective*window_ratio, -perspective, perspective, 1.0, 60.0*near_plane);
		// third, scale the scene so that the near plane gets the distance zoom_level*near_plane
		glScalef(perspective*near_plane, perspective*near_plane, perspective*near_plane);
		// second, move to the distance that reflects the zoom level
		glTranslatef(0.0f, 0.0f, -zoom_level/perspective);
	}
	// first, move back to the actor
	glTranslatef(0.0f, 0.0f, zoom_level*camera_distance);

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
					CHECK_GL_ERRORS();
				}
		}

#ifndef MAP_EDITOR2
	//do the same for the actors textures...
	for(i=0;i<max_actors;i++)
		{
			if(actors_list[i])
				{
					if(actors_list[i]->remapped_colors || actors_list[i]->is_enhanced_model)//if it is not remapable, then it is already in the cache
						{
							glDeleteTextures(1,&actors_list[i]->texture_id);
							actors_list[i]->texture_id=0;
							CHECK_GL_ERRORS();
						}
				}
		}
#endif

	if(have_vertex_buffers){
		e3d_object * obj;
		for(i=0;i<cache_e3d->max_item;i++){
			if(!cache_e3d->cached_items[i])continue;
			obj=cache_e3d->cached_items[i]->cache_item;

			if(obj->vbo[0]){
				const GLuint buf[3]={obj->vbo[0], obj->vbo[1], obj->vbo[2]};
			
				ELglDeleteBuffersARB(3, buf);

				obj->vbo[0]=0;
				obj->vbo[1]=0;
				obj->vbo[2]=0;
				CHECK_GL_ERRORS();
			}
		}
		
		for(i=0;i<highest_obj_3d;i++){
			if(objects_list[i] && objects_list[i]->cloud_vbo) {
				const GLuint l=objects_list[i]->cloud_vbo;

				ELglDeleteBuffersARB(1, &l);

				objects_list[i]->cloud_vbo=0;
				CHECK_GL_ERRORS();
			}
		}
	}

	//...and the texture used for shadow mapping
	glDeleteTextures(1,&depth_map_id);
	depth_map_id=0;

	//destroy the current context
	SDL_QuitSubSystem(SDL_INIT_VIDEO);

	init_video();
	resize_root_window();
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

	reload_fonts();

#ifndef MAP_EDITOR2
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
#endif

	if(have_vertex_buffers){
		e3d_object * obj;
		
		for(i=0;i<cache_e3d->max_item;i++){
			if(!cache_e3d->cached_items[i])continue;
			obj=cache_e3d->cached_items[i]->cache_item;

			if(!obj->array_uv_main || !obj->array_normal || !obj->array_vertex)continue;

			ELglGenBuffersARB(3, obj->vbo);

			ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, obj->vbo[0]);
			ELglBufferDataARB(GL_ARRAY_BUFFER_ARB, obj->face_no*3*sizeof(e3d_array_uv_main), obj->array_uv_main, GL_STATIC_DRAW_ARB);

			ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, obj->vbo[1]);
			ELglBufferDataARB(GL_ARRAY_BUFFER_ARB, obj->face_no*3*sizeof(e3d_array_normal), obj->array_normal, GL_STATIC_DRAW_ARB);

			ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, obj->vbo[2]);
			ELglBufferDataARB(GL_ARRAY_BUFFER_ARB, obj->face_no*3*sizeof(e3d_array_vertex), obj->array_vertex, GL_STATIC_DRAW_ARB);
					CHECK_GL_ERRORS();
		}
		
		for(i=0;i<highest_obj_3d;i++){
			if(objects_list[i] && objects_list[i]->clouds_uv) {
				ELglGenBuffersARB(1, &objects_list[i]->cloud_vbo);
	
				ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, objects_list[i]->cloud_vbo);
				ELglBufferDataARB(GL_ARRAY_BUFFER_ARB, objects_list[i]->e3d_data->face_no*3*sizeof(e3d_array_uv_detail), objects_list[i]->clouds_uv, GL_STATIC_DRAW_ARB);
					CHECK_GL_ERRORS();
			}
		}
		
		ELglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		CHECK_GL_ERRORS();
	}
	
	//it is dependent on the window height...
	init_hud_interface(2);//Last interface
	new_minute();

	regenerate_near_objects=1;
	regenerate_near_2d_objects=1;

	// resize the EL root windows
	resize_all_root_windows (window_width, window_height);
#ifdef	USE_FRAMEBUFFER
	change_reflection_framebuffer_size(window_width, window_height);
#endif
}

void toggle_full_screen()
{
	full_screen=!full_screen;
	set_new_video_mode(full_screen,video_mode);
	build_video_mode_array();
}


int print_gl_errors(const char *file, const char *func, int line)
{
	int	glErr, anyErr=GL_NO_ERROR;

	while ((glErr=glGetError()) != GL_NO_ERROR )
	 {
		anyErr=glErr;
#ifdef	GLUT
		log_error_detailed("OpenGL %s", file, func, line, gluErrorString(glErr));
#else
		log_error_detailed("OpenGL error %d", file, func, line, glErr);
#endif // GLUT
	}
	return anyErr;
}
