#include "global.h"

#ifdef NEW_E3D_FORMAT
int use_vertex_buffers=0;
int have_vertex_buffers=0;
int have_texture_non_power_of_two = 0;
int gl_extensions_loaded = 0;
float anisotropic_filter = 1.0f;

void (APIENTRY * ELglMultiTexCoord2fARB) (GLenum target, GLfloat s, GLfloat t);
void (APIENTRY * ELglMultiTexCoord2fvARB) (GLenum target, const GLfloat *v);
void (APIENTRY * ELglActiveTextureARB) (GLenum texture);
void (APIENTRY * ELglClientActiveTextureARB) (GLenum texture);
void (APIENTRY * ELglBindBufferARB)(GLenum target, GLuint buffer);
void (APIENTRY * ELglBufferDataARB)(GLenum target, GLsizeiptrARB size, const void * data, GLenum usage);
void (APIENTRY * ELglGenBuffersARB)(GLsizei no, GLuint *buffer);
void (APIENTRY * ELglDeleteBuffersARB)(GLsizei no, const GLuint *buffer);
void (APIENTRY * ELglMultiDrawElementsEXT) (GLenum mode, GLsizei* count, GLenum type, const GLvoid **indices, GLsizei primcount);
void (APIENTRY * ELglDrawRangeElementsEXT) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);

void Emul_glMultiDrawElements(GLenum mode, GLsizei* count, GLenum type, const GLvoid **indices, GLsizei primcount)
{
	int i;

	for (i = 0; i < primcount; i++)
	{ 
		if (count[i] > 0) glDrawElements(mode, count[i], type, indices[i]);
	}
}

void Emul_glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)
{
	glDrawElements(mode, count, type, indices);
}

void init_gl_extensions()
{
	Uint8 * extensions;
	int ext_str_len;
	char str[150];
	//now load the multitexturing extension
	ELglBindBufferARB = SDL_GL_GetProcAddress("glBindBufferARB");
	ELglGenBuffersARB = SDL_GL_GetProcAddress("glGenBuffersARB");
	ELglDeleteBuffersARB = SDL_GL_GetProcAddress("glDeleteBuffersARB");
	ELglBufferDataARB = SDL_GL_GetProcAddress("glBufferDataARB");
	ELglMultiDrawElementsEXT=SDL_GL_GetProcAddress("glMultiDrawElementsEXT");
	ELglDrawRangeElementsEXT=SDL_GL_GetProcAddress("glDrawRangeElementsEXT");

	extensions=(GLubyte *)glGetString(GL_EXTENSIONS);
	ext_str_len=strlen(extensions);

	ELglActiveTextureARB = SDL_GL_GetProcAddress("glActiveTextureARB");
	ELglMultiTexCoord2fARB = SDL_GL_GetProcAddress("glMultiTexCoord2fARB");
	ELglMultiTexCoord2fvARB	= SDL_GL_GetProcAddress("glMultiTexCoord2fvARB");
	ELglClientActiveTextureARB = SDL_GL_GetProcAddress("glClientActiveTextureARB");
	if(ELglActiveTextureARB && ELglMultiTexCoord2fARB && ELglMultiTexCoord2fvARB && ELglClientActiveTextureARB) {
		have_multitexture=get_string_occurance("GL_ARB_multitexture",extensions,ext_str_len,0);
		if(have_multitexture==-1) {
			have_multitexture=0;
		} else {
			glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB,&have_multitexture);
		}
	} else {
		have_multitexture=0;
	}

	if(ELglMultiDrawElementsEXT && strstr(extensions, "GL_EXT_multi_draw_arrays")){
	} else {
		ELglMultiDrawElementsEXT=&Emul_glMultiDrawElements;
	}
	if(ELglDrawRangeElementsEXT && strstr(extensions, "GL_EXT_draw_range_elements")){
	} else {
		ELglDrawRangeElementsEXT=&Emul_glDrawRangeElements;
	}

	if (strstr(extensions, "GL_ARB_texture_non_power_of_two"))
	{		
		have_texture_non_power_of_two = 1;
	}

	CHECK_GL_ERRORS();
	gl_extensions_loaded = 1;
}
#endif

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
	SDL_EnableKeyRepeat (200, 100);
	SDL_EnableUNICODE(1);
	
	for(i=0;i<1000;i++)
		{
			if(texture_cache[i].file_name[0])
				{
	            	alpha=texture_cache[i].alpha;
	            	//our texture was freed, we have to reload it
	        		if(alpha<=0)texture_cache[i].texture_id=load_bmp8_color_key(texture_cache[i].file_name,alpha);
	            	else
						texture_cache[i].texture_id=load_bmp8_fixed_alpha(texture_cache[i].file_name, alpha);
				}
		}
	map_has_changed=1;
	reset_material();
	init_lights();
	init_colors();
}
