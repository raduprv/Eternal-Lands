#ifndef __GL_INIT_H__
#define __GL_INIT_H__

extern int window_width;
extern int window_height;

extern int desktop_width;
extern int desktop_height;

extern int bpp;
extern int video_mode;
extern int full_screen;

extern int have_multitexture;
extern int use_vertex_array;
extern int use_point_particles;
extern int vertex_arrays_built;
extern int have_compiled_vertex_array;
extern int have_point_sprite;
extern int have_arb_compression;
extern int have_s3_compression;
extern int have_sgis_generate_mipmap;
extern int use_mipmaps;
extern int have_arb_shadow;

extern void (APIENTRY * ELglMultiTexCoord2fARB) (GLenum target, GLfloat s, GLfloat t);
extern void (APIENTRY * ELglMultiTexCoord2fvARB) (GLenum target, const GLfloat *v);
extern void (APIENTRY * ELglActiveTextureARB) (GLenum texture);
extern void (APIENTRY * ELglClientActiveTextureARB) (GLenum texture);
extern void (APIENTRY * ELglLockArraysEXT) (GLint first, GLsizei count);
extern void (APIENTRY * ELglUnlockArraysEXT) (void);

void setup_video_mode();
void check_gl_mode();
void init_video();
void init_gl_extensions();
void resize_window();
void set_new_video_mode(int fs,int mode);
void toggle_full_screen();
int print_gl_errors(char *file, char *func, int line);
#ifdef	DEBUG
#define check_gl_errors()	print_gl_errors(__FILE__,  __FUNCTION__, __LINE__)
#else	//DEBUG
#define check_gl_errors()	//NOP
#endif	//DEBUG

#ifndef POINT_SIZE_MIN_ARB
#define POINT_SIZE_MIN_ARB 0x8126
#endif

#ifndef COMPRESSED_RGBA_ARB
#define COMPRESSED_RGBA_ARB				0x84EE
#endif

#ifndef COMPRESSED_RGBA_S3TC_DXT5_EXT
#define COMPRESSED_RGBA_S3TC_DXT5_EXT                  0x83F3
#endif

#endif
