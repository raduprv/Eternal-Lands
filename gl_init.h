/*!
 * \file
 * \ingroup video
 * \brief OpenGL and video initialization related functions.
 */
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

/*!
 * \ingroup video
 * \brief   initializes the selected video mode
 *
 *      Initializes and sets up the selected video mode
 *
 * \sa init_stuff
 * \sa init_video
 */
void setup_video_mode();

/*!
 * \ingroup video
 * \brief   checks if the selected mode is available via OpenGL.
 *
 *      Checks if the selected mode is available via OpenGL.
 *
 * \sa init_video
 */
void check_gl_mode();

/*!
 * \ingroup video
 * \brief   initializes the video engine.
 *
 *      Initializes the video engine.
 *
 * \callgraph
 */
void init_video();

/*!
 * \ingroup video
 * \brief   checks and initializes available OpenGL extensions (GLX)
 *
 *      Checks and enables available OpenGL extensions (GLX)
 *
 * \callgraph
 */
void init_gl_extensions();

/*!
 * \ingroup video
 * \brief   resizes the window to the selected mode.
 *
 *      Resizes the window, after selecting a new video mode.
 *
 */
void resize_root_window();

/*!
 * \ingroup video
 * \brief   sets \a mode to be the new video mode. If \a fs is 0, the new mode will be fullscreen, else it will be a windowed mode.
 *
 *      Sets \a mode to be the new video mode. If \a fs is 0, the new mode will be fullscreen, else it will be a windowed mode.
 *
 * \param fs        flag, indicating whether \a mode will be in fullscreen or in windowed mode.
 * \param mode      the new video mode to use.
 *
 * \callgraph
 */
void set_new_video_mode(int fs,int mode);

/*!
 * \ingroup video
 * \brief   toggles the display from fullscreen to windowed mode and vice versa.
 *
 *      Toggles the display from fullscreen to windowed mode and vice versa.
 *
 * \callgraph
 */
void toggle_full_screen();

/*!
 * \ingroup video
 * \brief       prints OpenGL related errors
 *
 *      Prints OpenGL related errors to the console.
 *
 * \param file  the source file where the error occurred
 * \param func  the function where the error occurred
 * \param line  the line in the source file where the error occurred
 * \retval int
 * \callgraph
 */
int print_gl_errors(const char *file, const char *func, int line);

/*!
 * \name check_gl_errors macro
 */
/*! @{ */
#ifdef	DEBUG
#define check_gl_errors()	print_gl_errors(__FILE__,  __FUNCTION__, __LINE__)
#else	//DEBUG
#define check_gl_errors()	/*!< NOP */
#endif	//DEBUG
/*! @} */

/*!
 * \name OpenGL related constants
 */
/*! @{ */
#ifndef POINT_SIZE_MIN_ARB
#define POINT_SIZE_MIN_ARB 0x8126
#endif

#ifndef COMPRESSED_RGBA_ARB
#define COMPRESSED_RGBA_ARB				0x84EE
#endif

#ifndef COMPRESSED_RGBA_S3TC_DXT5_EXT
#define COMPRESSED_RGBA_S3TC_DXT5_EXT                  0x83F3
#endif
/*! @} */

#endif
