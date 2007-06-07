#include <stdlib.h>
#include <string.h>
#ifdef MAP_EDITOR2
#include "../map_editor2/global.h"
#else
#include "global.h"
#endif
#include "io/e3d_io.h"
#ifdef EYE_CANDY
#include "eye_candy_wrapper.h"
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
int use_compiled_vertex_array=0;
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
int have_texture_non_power_of_two = 0;
int use_frame_buffer = 0;
int gl_extensions_loaded = 0;

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
void (APIENTRY * ELglDeleteObjectARB)(GLhandleARB shader);
void (APIENTRY * ELglGetInfoLogARB)(GLhandleARB object,GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
void (APIENTRY * ELglGetObjectParameterivARB)(GLhandleARB object, GLenum pname, GLint *params);
GLint (APIENTRY * ELglGetUniformLocationARB)(GLhandleARB program, const char * name);
GLint (APIENTRY * ELglGetAttribLocationARB)(GLhandleARB program, const char *name);
void (APIENTRY * ELglUniform1iARB)(GLint location, GLint v0);
void (APIENTRY * ELglUniformMatrix3fvARB)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void (APIENTRY * ELglUniform3fvARB)(GLint location, GLsizei count, const GLfloat* value);
void (APIENTRY * ELglUniform4fvARB)(GLint location, GLsizei count, const GLfloat* value);
void (APIENTRY * ELglVertexAttribPointerARB)(GLuint index, int size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
void (APIENTRY * ELglEnableVertexAttribArrayARB)(GLuint index);
void (APIENTRY * ELglDisableVertexAttribArrayARB)(GLuint index);
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
void (APIENTRY * ELglDrawRangeElementsEXT) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);

void APIENTRY Emul_glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)
{
	glDrawElements(mode, count, type, indices);
}

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
			case 13:
				window_width=1280;
				window_height=800;
				bpp=16;
				break;
			case 14:
				window_width=1280;
				window_height=800;
				bpp=32;
				break;
			case 15:
				window_width=1440;
				window_height=900;
				bpp=16;
				break;
			case 16:
				window_width=1440;
				window_height=900;
				bpp=32;
				break;
			case 17:
				window_width=1680;
				window_height=1050;
				bpp=16;
				break;
			case 18:
				window_width=1680;
				window_height=1050;
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
						char str[100];
						safe_snprintf(str,sizeof(str),window_size_adjusted_str,"640x480");
						LOG_TO_CONSOLE(c_yellow1,str);
					}
				window_width=640;
				window_height=480;
				break;
			case 3:
			case 4:
				if(window_width != 780 || window_height != 550)
					{
						char str[100];
						safe_snprintf(str,sizeof(str),window_size_adjusted_str,"780x550");
						LOG_TO_CONSOLE(c_yellow1,str);
					}
				window_width=780;
				window_height=550;
				break;
			case 5:
			case 6:
				if(window_width != 990 || window_height != 720)
					{
						char str[100];
						safe_snprintf(str,sizeof(str),window_size_adjusted_str,"990x720");
						LOG_TO_CONSOLE(c_yellow1,str);
					}
				window_width=990;
				window_height=720;
				break;
			case 7:
			case 8:
				if(window_width != 1070 || window_height != 785)
					{
						char str[100];
						safe_snprintf(str,sizeof(str),window_size_adjusted_str,"1070x785");
						LOG_TO_CONSOLE(c_yellow1,str);
					}
				window_width=1070;
				window_height=785;
				break;
			case 9:
			case 10:
				if(window_width != 1250 || window_height != 990)
					{
						char str[100];
						safe_snprintf(str,sizeof(str),window_size_adjusted_str,"1250x990");
						LOG_TO_CONSOLE(c_yellow1,str);
					}
				window_width=1250;
				window_height=990;
				break;
			case 11:
			case 12:
				if(window_width != 1600 || window_height != 1200)
				{
					char str[100];
					safe_snprintf(str,sizeof(str),window_size_adjusted_str,"1600x1200");
					LOG_TO_CONSOLE(c_yellow1,str);
				}
				window_width=1600;
				window_height=1200;
				break;
			case 13:
			case 14:
				if(window_width != 1240 || window_height != 780)
				{
					char str[100];
					safe_snprintf(str,sizeof(str),window_size_adjusted_str,"1240x780");
					LOG_TO_CONSOLE(c_yellow1,str);
				}
				window_width=1240;
				window_height=780;
				break;
			case 15:
			case 16:
				if(window_width != 1420 || window_height != 810)
				{
					char str[100];
					safe_snprintf(str,sizeof(str),window_size_adjusted_str,"1420x810");
					LOG_TO_CONSOLE(c_yellow1,str);
				}
				window_width=1420;
				window_height=810;
				break;
			case 17:
			case 18:
				if(window_width != 1620 || window_height != 950)
				{
					char str[100];
					safe_snprintf(str,sizeof(str),window_size_adjusted_str,"1620x950");
					LOG_TO_CONSOLE(c_yellow1,str);
				}
				window_width=1620;
				window_height=950;
				break;
			}
//TODO: Add wide screen resolutions
//1400x1050
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
			safe_snprintf (vid_mode_str, sizeof (vid_mode_str), "%ix%ix%i", window_width, window_height, bpp);
			safe_snprintf(str,sizeof(str),no_stencil_str,vid_mode_str);
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

					safe_snprintf (vid_mode_str, sizeof (vid_mode_str), "%ix%ix%i", old_width, old_height, old_bpp);
					safe_snprintf(str,sizeof(str),safemode_str,vid_mode_str);
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

	//if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE | SDL_INIT_EVENTTHREAD) == -1)	// experimental
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) == -1)
		{
			log_error("%s: %s\n", no_sdl_str, SDL_GetError());
			SDL_Quit();
			exit(1);
		}

#ifdef EYE_CANDY
	ec_clear_textures();
#endif //EYE_CANDY
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

	SDL_WM_SetIcon(SDL_LoadBMP("icon.bmp"), NULL);
	/* Set the window manager title bar */

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

#ifdef MAP_EDITOR2
	SDL_WM_SetCaption( "Map Editor", "mapeditor" );
#else
	SDL_WM_SetCaption( win_principal, "eternallands" );
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
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

#ifdef EYE_CANDY
	ec_load_textures();
#endif //EYE_CANDY

	check_options();
}

#ifdef	GL_EXTENSION_CHECK
void evaluate_extension(int multitexture_count, const char* extensions)
{
	int has_arb_multitexture;
	int has_arb_occlusion_query;
	int has_arb_point_parameters;
	int has_arb_point_sprite;
	int has_arb_shader_objects;
	int has_arb_shading_language_100;
	int has_arb_shadow;
	int has_arb_texture_compression;
	int has_arb_texture_env_add;
	int has_arb_texture_env_combine;
	int has_arb_texture_env_crossbar;
	int has_arb_texture_rectangle;
	int has_arb_vertex_buffer_object;
	int has_arb_vertex_program;
	int has_arb_vertex_shader;
	int has_arb_fragment_program;
	int has_arb_fragment_shader;
	int has_arb_fragment_shader_shadow;
	int has_ati_fragment_shader;
	int has_ati_texture_env_combine3;
	int has_nv_texture_env_combine4;
	int has_nv_texture_shader;
	int has_nv_texture_shader2;
	int has_ext_draw_range_elements;
	int has_ext_texture_compression_s3tc;
	int has_ext_framebuffer_object;
	int options;

	has_arb_multitexture = multitexture_count;

	has_arb_occlusion_query = strstr(extensions, "GL_ARB_occlusion_query") > 0;
	has_arb_point_parameters = strstr(extensions, "GL_ARB_point_parameters") > 0;
	has_arb_point_sprite = strstr(extensions, "GL_ARB_point_sprite") > 0;
	has_arb_shader_objects = strstr(extensions, "GL_ARB_shader_objects") > 0;
	has_arb_shading_language_100 = strstr(extensions, "GL_ARB_shading_language_100") > 0;
	has_arb_shadow = strstr(extensions, "GL_ARB_shadow") > 0;
	has_arb_texture_compression = strstr(extensions, "GL_ARB_texture_compression") > 0;
	has_arb_texture_env_add = strstr(extensions, "GL_ARB_texture_env_add") > 0;
	has_arb_texture_env_combine = strstr(extensions, "GL_ARB_texture_env_combine") > 0;
	has_arb_texture_env_crossbar = strstr(extensions, "GL_ARB_texture_env_crossbar") > 0;
	has_arb_texture_rectangle = strstr(extensions, "GL_ARB_texture_rectangle") > 0;
	has_arb_vertex_buffer_object = strstr(extensions, "GL_ARB_vertex_buffer_object") > 0;
	has_arb_vertex_program = strstr(extensions, "GL_ARB_vertex_program") > 0;
	has_arb_vertex_shader = strstr(extensions, "GL_ARB_vertex_program") > 0;
	has_arb_fragment_program = strstr(extensions, "GL_ARB_fragment_program") > 0;
	has_arb_fragment_shader = strstr(extensions, "GL_ARB_fragment_shader") > 0;
	has_arb_fragment_shader_shadow = strstr(extensions, "GL_ARB_fragment_program_shadow") > 0;
	has_ati_fragment_shader = strstr(extensions, "GL_ATI_fragment_shader") > 0;
	has_ati_texture_env_combine3 = strstr(extensions, "GL_ATI_texture_env_combine3") > 0;
	has_nv_texture_env_combine4 = strstr(extensions, "GL_NV_texture_env_combine4") > 0;
	has_nv_texture_shader = strstr(extensions, "GL_NV_texture_shader") > 0;
	has_nv_texture_shader2 = strstr(extensions, "GL_NV_texture_shader2") > 0;
	has_ext_draw_range_elements = strstr(extensions, "GL_EXT_draw_range_elements") > 0;
	has_ext_texture_compression_s3tc = strstr(extensions, "GL_EXT_texture_compression_s3tc") > 0;
	has_ext_framebuffer_object = strstr(extensions, "GL_EXT_framebuffer_object") > 0;

	options = (has_arb_multitexture >= 2) && has_arb_texture_env_add &&
		has_arb_texture_env_combine && has_arb_vertex_program &&
		has_arb_texture_compression && has_arb_vertex_buffer_object &&
		has_ext_texture_compression_s3tc;

	if (!options)
	{
		LOG_TO_CONSOLE(c_red1, "Your graphic card/driver don't support the minimum"
			"requirements for the next EL release. Please upgrade your driver."
			" If this don't help, you need a better graphic card.");
		return;
	}

	if (!has_arb_vertex_shader || (has_arb_fragment_program && !has_arb_fragment_shader) ||
		(has_ati_fragment_shader && !has_arb_fragment_program && !has_arb_fragment_shader))
	{
		LOG_TO_CONSOLE(c_yellow1, "Please update your graphic card driver!");
	}

	options = ((has_ati_texture_env_combine3 && has_arb_texture_env_crossbar) ||
		has_nv_texture_env_combine4) && (has_arb_multitexture >= 4) &&
		has_ext_draw_range_elements && has_arb_shadow &&
		has_arb_point_parameters && has_arb_point_sprite;

	if (!options)
	{
		LOG_TO_CONSOLE(c_yellow1, "Your graphic card supports the absolute minimum "
			"requirements for the next EL release, but don't expect that you can use"
			" all features.");
	}
	else
	{
		options = (has_ati_fragment_shader || (has_nv_texture_shader &&
			has_nv_texture_shader2)) && has_arb_occlusion_query &&
			has_arb_texture_rectangle && has_ext_framebuffer_object;
		if (!options)
		{
			LOG_TO_CONSOLE(c_green2, "Your graphic card supports the default "
				"requirements for the next EL release.");
		}
		else
		{
			if (has_arb_fragment_shader && has_arb_shader_objects &&
				has_arb_vertex_shader && has_arb_shading_language_100)
			{
				LOG_TO_CONSOLE(c_blue2, "Your graphic card supports all "
					"features EL will use in the future.");
			}
			else
			{
				LOG_TO_CONSOLE(c_blue2, "Your graphic card supports more than the"
				"default requirements for the next EL release.");
			}
		}
	}
}
#endif	//GL_EXTENSION_CHECK

void init_gl_extensions()
{
	char * extensions;
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
	ELglDeleteObjectARB=SDL_GL_GetProcAddress("glDeleteObjectARB");
	ELglGetInfoLogARB=SDL_GL_GetProcAddress("glGetInfoLogARB");
	ELglGetObjectParameterivARB=SDL_GL_GetProcAddress("glGetObjectParameterivARB");
	ELglGetUniformLocationARB=SDL_GL_GetProcAddress("glGetUniformLocationARB");
	ELglGetAttribLocationARB=SDL_GL_GetProcAddress("glGetAttribLocationARB");
	ELglUniform1iARB=SDL_GL_GetProcAddress("glUniform1iARB");
	ELglUniformMatrix3fvARB=SDL_GL_GetProcAddress("glUniformMatrix3fvARB");
	ELglUniform3fvARB=SDL_GL_GetProcAddress("glUniform3fvARB");
	ELglUniform4fvARB=SDL_GL_GetProcAddress("glUniform4fvARB");
	ELglVertexAttribPointerARB=SDL_GL_GetProcAddress("glVertexAttribPointerARB");
	ELglEnableVertexAttribArrayARB=SDL_GL_GetProcAddress("glEnableVertexAttribArrayARB");
	ELglDisableVertexAttribArrayARB=SDL_GL_GetProcAddress("glDisableVertexAttribArrayARB");
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
	ELglDrawRangeElementsEXT=SDL_GL_GetProcAddress("glDrawRangeElementsEXT");

	//see if we really have multitexturing
	extensions=(char *)glGetString(GL_EXTENSIONS);
	ext_str_len=strlen(extensions);
	if(ELglActiveTextureARB && ELglMultiTexCoord2fARB && ELglMultiTexCoord2fvARB && ELglClientActiveTextureARB) {
		have_multitexture=get_string_occurance("GL_ARB_multitexture",extensions,ext_str_len,0);
		if(have_multitexture==-1) {
			have_multitexture=0;
			LOG_TO_CONSOLE(c_red1,gl_ext_no_multitexture);
		} else {
			glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB,&have_multitexture);
			safe_snprintf(str,sizeof(str),gl_ext_found,"GL_ARB_multitexture");
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
			use_compiled_vertex_array=0;
			safe_snprintf(str,sizeof(str),gl_ext_not_found,"GL_EXT_compiled_vertex_array");
			LOG_TO_CONSOLE(c_red1,str);
		} else if (!use_compiled_vertex_array) {
			safe_snprintf(str,sizeof(str),gl_ext_found_not_used,"GL_EXT_compiled_vertex_array");
			LOG_TO_CONSOLE(c_green2,str);
		} else {
			safe_snprintf(str,sizeof(str),gl_ext_found,"GL_EXT_compiled_vertex_array");
			LOG_TO_CONSOLE(c_green2,str);
		}
	} else {
		have_compiled_vertex_array=0;
		safe_snprintf(str,sizeof(str),gl_ext_not_found,"GL_EXT_compiled_vertex_array");
		LOG_TO_CONSOLE(c_red1,str);
	}

	have_point_sprite = get_string_occurance("GL_ARB_point_sprite",extensions,ext_str_len,0)>=0 || get_string_occurance("GL_NV_point_sprite",extensions,ext_str_len,0)>=0;
	if(!have_point_sprite){
		safe_snprintf(str,sizeof(str),gl_ext_not_found,"GL_*_point_sprite");
		LOG_TO_CONSOLE(c_red1,str);
	} else {
		safe_snprintf(str,sizeof(str),gl_ext_found,"GL_*_point_sprite");
		LOG_TO_CONSOLE(c_green2,str);
	}
	
	have_arb_compression=get_string_occurance("GL_ARB_texture_compression",extensions,ext_str_len,0);

	if(have_arb_compression<0) {
		have_arb_compression=0;
	} else {
		have_arb_compression=1;
		safe_snprintf(str,sizeof(str),gl_ext_found,"GL_ARB_texture_compression");
		LOG_TO_CONSOLE(c_green2,str);
	}
	
	have_s3_compression=get_string_occurance("GL_EXT_texture_compression_s3tc",extensions,ext_str_len,0);
	if(have_s3_compression<0) {
		have_s3_compression=0;
	} else {
		have_s3_compression=1;
		safe_snprintf(str,sizeof(str),gl_ext_found,"GL_EXT_texture_compression_s3tc");
		LOG_TO_CONSOLE(c_green2,str);
	}

	have_sgis_generate_mipmap=get_string_occurance("GL_SGIS_generate_mipmap",extensions,ext_str_len,0);
	
	if(have_sgis_generate_mipmap<0)	{
		have_sgis_generate_mipmap=0;
		use_mipmaps=0;
		safe_snprintf(str,sizeof(str),gl_ext_not_found,"GL_SGIS_generate_mipmap");
		LOG_TO_CONSOLE(c_red1,str);
	} else if(!use_mipmaps) {
		have_sgis_generate_mipmap=0;
		safe_snprintf(str,sizeof(str),gl_ext_found_not_used,"GL_SGIS_generate_mipmap");
		LOG_TO_CONSOLE(c_green2,str);
	} else  {
		have_sgis_generate_mipmap=1;
		safe_snprintf(str,sizeof(str),gl_ext_found,"GL_SGIS_generate_mipmap");
		LOG_TO_CONSOLE(c_green2,str);
	}

	have_arb_shadow=get_string_occurance("GL_ARB_shadow",extensions,ext_str_len,0);
	if(have_arb_shadow<0){
		have_arb_shadow=0;
	} else {
		have_arb_shadow=1;
		safe_snprintf(str,sizeof(str),gl_ext_found,"GL_ARB_shadow");
		LOG_TO_CONSOLE(c_green2,str);
	}

	if(get_string_occurance("GL_ARB_vertex_buffer_object",extensions,ext_str_len,0)>=0 && use_vertex_buffers){
		safe_snprintf(str,sizeof(str),gl_ext_found,"GL_ARB_vertex_buffer_object");
		LOG_TO_CONSOLE(c_green2, str);
		have_vertex_buffers=1;
	} else {
		have_vertex_buffers=0;
	}
	
	if(ELglGenRenderbuffersEXT && ELglDeleteRenderbuffersEXT && ELglBindRenderbufferEXT && ELglRenderbufferStorageEXT &&
	   ELglIsRenderbufferEXT && ELglGetRenderbufferParameterivEXT && ELglIsFramebufferEXT && ELglCheckFramebufferStatusEXT &&
	   ELglFramebufferTexture1DEXT && ELglFramebufferTexture2DEXT && ELglFramebufferTexture3DEXT && 
	   ELglFramebufferRenderbufferEXT && ELglGetFramebufferAttachmentParameterivEXT && ELglGenerateMipmapEXT &&
	   ELglGenFramebuffersEXT && ELglDeleteFramebuffersEXT && ELglBindFramebufferEXT && strstr(extensions, "GL_EXT_framebuffer_object")){
		if (ELglCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT)
		{
			safe_snprintf(str,sizeof(str),gl_ext_found,"GL_EXT_framebuffer_object");
			LOG_TO_CONSOLE(c_green2, str);
			have_framebuffer_object = 1;
		}
		else
		{
			safe_snprintf(str,sizeof(str),gl_ext_not_found,"GL_EXT_framebuffer_object");
			LOG_TO_CONSOLE(c_red1, str);
			have_framebuffer_object = 0;
		}
	} else {
		safe_snprintf(str,sizeof(str),gl_ext_not_found,"GL_EXT_framebuffer_object");
		LOG_TO_CONSOLE(c_red1, str);
		have_framebuffer_object = 0;
	}

	if(ELglDrawRangeElementsEXT && strstr(extensions, "GL_EXT_draw_range_elements")){
		safe_snprintf(str,sizeof(str),gl_ext_found,"GL_EXT_draw_range_elements");
		LOG_TO_CONSOLE(c_green2,str);
	} else {
		safe_snprintf(str,sizeof(str),gl_ext_not_found_emul_it,"GL_EXT_draw_range_elements");
		LOG_TO_CONSOLE(c_yellow1,str);
		ELglDrawRangeElementsEXT=&Emul_glDrawRangeElements;
	}

	if (strstr(extensions, "GL_ARB_texture_non_power_of_two"))
	{		
		safe_snprintf(str, sizeof(str), gl_ext_found, "GL_ARB_texture_non_power_of_two");
		LOG_TO_CONSOLE(c_green2, str);
		have_texture_non_power_of_two = 1;
	}

	//Test for ARB{fp,vp}
	if(ELglGenProgramsARB && ELglDeleteProgramsARB && ELglBindProgramARB && ELglProgramStringARB){
		if(strstr(extensions, "GL_ARB_fragment_program")){
			safe_snprintf(str,sizeof(str),gl_ext_found_not_used,"GL_ARB_fragment_program");
			LOG_TO_CONSOLE(c_green2, str);
			have_arb_pixel_shader=1;
		} else {
			/*
			 * safe_snprintf(str,sizeof(str),gl_ext_not_found,"GL_ARB_fragment_program");
			 * LOG_TO_CONSOLE(c_green2, str);
			 */
		}
		if(strstr(extensions, "GL_ARB_vertex_program")){
			safe_snprintf(str,sizeof(str),gl_ext_found_not_used,"GL_ARB_vertex_program");
			LOG_TO_CONSOLE(c_green2, str);
			have_arb_vertex_shader=1;
		} else {
			/*
			 * safe_snprintf(str,sizeof(str),gl_ext_not_found,"GL_ARB_vertex_program");
			 * LOG_TO_CONSOLE(c_green2, str);
			 */
		}
	}  else {
		/*
		 * safe_snprintf(str,sizeof(str),gl_ext_not_found,"GL_ARB_{fragment,vertex}_{program,shader}");
		 * LOG_TO_CONSOLE(c_green2, str);
		 */
	}
	
	//Test for OGSL
	if(ELglCreateShaderObjectARB && ELglShaderSourceARB && ELglCompileShaderARB && ELglCreateProgramObjectARB &&
	   ELglAttachObjectARB && ELglLinkProgramARB && ELglUseProgramObjectARB && 
	   ELglDeleteObjectARB && ELglGetInfoLogARB && ELglGetObjectParameterivARB &&
	   ELglGetUniformLocationARB && ELglGetAttribLocationARB && ELglUniform1iARB && 
	   ELglUniformMatrix3fvARB && ELglUniform3fvARB && ELglUniform4fvARB &&
	   ELglVertexAttribPointerARB && ELglEnableVertexAttribArrayARB && ELglDisableVertexAttribArrayARB && 
	   strstr(extensions,"GL_ARB_shader_objects") && strstr(extensions, "GL_ARB_shading_language_100"))
	{
		if(strstr(extensions,"GL_ARB_vertex_shader"))
		{
			safe_snprintf(str, sizeof(str), gl_ext_found, "GL_ARB_vertex_shader");
			LOG_TO_CONSOLE(c_green2, str);
			have_ogsl_vertex_shader=1;
		}
		
		if(strstr(extensions,"GL_ARB_fragment_shader"))
		{
			safe_snprintf(str, sizeof(str), gl_ext_found, "GL_ARB_fragment_shader");
			LOG_TO_CONSOLE(c_green2, str);
			have_ogsl_pixel_shader=1;
		}
	} 
	else 
	{
		safe_snprintf(str,sizeof(str), gl_ext_not_found, "OpenGL Shading Language");
		LOG_TO_CONSOLE(c_green2, str);
	}

#ifdef	GL_EXTENSION_CHECK
	evaluate_extension(have_multitexture, extensions);
#endif	//GL_EXTENSION_CHECK
	CHECK_GL_ERRORS();
	gl_extensions_loaded = 1;
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
					glDeleteTextures(1,(GLuint*)&texture_cache[i].texture_id);
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

		for(i=0; i<cache_e3d->max_item; i++){
			if(!cache_e3d->cached_items[i] )continue;
			obj= cache_e3d->cached_items[i]->cache_item;

			free_e3d_va(obj);
		}
		CHECK_GL_ERRORS();
		
	}

#ifdef EYE_CANDY
	ec_clear_textures();
#endif //EYE_CANDY

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

#ifdef EYE_CANDY
	ec_load_textures();
#endif //EYE_CANDY

	//now, reload the textures
	for(i=0;i<1000;i++)
		{
			if(texture_cache[i].file_name[0])
				{
	            	alpha=texture_cache[i].alpha;
	            	//our texture was freed, we have to reload it
	        		if(alpha<=0) texture_cache[i].texture_id= load_bmp8_color_key(texture_cache[i].file_name, alpha);
	            	else texture_cache[i].texture_id= load_bmp8_fixed_alpha(texture_cache[i].file_name, alpha);
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
							//actors_list[i]->texture_id=load_bmp8_remapped_skin(actors_list[i]->skin_name,
							//												   150,actors_list[i]->skin,actors_list[i]->hair,actors_list[i]->shirt,
							//												   actors_list[i]->pants,actors_list[i]->boots);
						}
					if(actors_list[i]->is_enhanced_model)
						{
							actors_list[i]->texture_id=load_bmp8_enhanced_actor(actors_list[i]->body_parts, 255);
						}
				}
		}
#endif
	
	//it is dependent on the window height...
	init_hud_interface(2);//Last interface
	new_minute();

	set_all_intersect_update_needed(main_bbox_tree);

	// resize the EL root windows
	resize_all_root_windows (window_width, window_height);
	check_options();
	reload_tab_map = 1;
}

void toggle_full_screen()
{
	reload_tab_map = 1;
	full_screen=!full_screen;
	set_new_video_mode(full_screen,video_mode);
	build_video_mode_array();
	SDL_SetGamma(gamma_var, gamma_var, gamma_var);
	SDL_SetModState(KMOD_NONE); // force ALL keys up
}


int print_gl_errors(const char *file, const char *func, int line)
{
	int	glErr, anyErr=GL_NO_ERROR;

	while ((glErr=glGetError()) != GL_NO_ERROR )
	 {
		anyErr=glErr;
//#ifdef	GLUT
//FIXME: this appears to be a GLU call, not GLUT, and we link with GLU normally...
//unless this causes an error on some other compiler, the commented parts should be removed
		log_error_detailed("OpenGL %s", file, func, line, gluErrorString(glErr));
//#else
//		log_error_detailed("OpenGL error %d", file, func, line, glErr);
//#endif // GLUT
	}
	return anyErr;
}
