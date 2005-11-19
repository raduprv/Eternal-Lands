/*!
 * \file
 * \ingroup video
 * \brief OpenGL and video initialization related functions.
 */
#ifndef __GL_INIT_H__
#define __GL_INIT_H__

/*! \name window dimensions 
 * @{ */
extern int window_width; /*!< width of the window */
extern int window_height; /*!< height of the window */
/*! @} */

extern int bpp; /*!< color depth to use */
extern int video_mode; /*!< currently selected video mode */
extern int full_screen; /*!< flag that inidicates whether we are in fullscreen or windowed mode */
extern float gamma_var; /*!< The current gamma value */
extern float perspective; /*!< The perspective "angle". Higher values mean higher distortion. Default is 0.15f */
extern float near_plane; /*!< The distance of the near clipping plane to your actor (devided by zoom_level). */

/*! \name OpenGL extensions variables 
 * @{ */
extern int have_stencil; /*!< flag that indicates whether we have the stencil extension or not. \todo shouldn't this go to gl_init.h to all the other OpenGL related variables? */
extern int have_multitexture; /*! indicates whether we have the multitexture extension or not */
extern int use_vertex_array; /*!< specifies if we use vertex arrays or not */
extern int use_vertex_buffers; /*!< specifies if we use vertex buffer objects or not */
extern int vertex_arrays_built; /*!< flag that indicates whether the vertex array was already initialized or not */
extern int have_compiled_vertex_array; /*!< indicates whether we have the compiled vertex array extension or not */
extern int have_point_sprite; /*!< indicates whether we have point sprites or not */
extern int have_arb_compression; /*!< flag that indicates whether we have the ARB compression extension or not */
extern int have_s3_compression; /*!< flag that indicates whether we have the S3 compression extension or not */
extern int have_vertex_buffers; /*!< flag that indicates whether we have access to using vertex buffer objects or not*/
extern int have_framebuffer_object; /*!< flag that indicates whether the GL_EXT_framebuffer_object extension is supported or not*/
extern int have_shaders;

extern int use_mipmaps; /*!< indicates whether we use mipmaps or not */

extern int have_arb_shadow;
#ifdef	TERRAIN
extern int have_ogsl_pixel_shader;
extern int have_ogsl_vertex_shader;
#endif
extern int have_texture_non_power_of_two; /*! < flag that indicates whether the GL_ARB_texture_non_power_of_two extension is supported or not*/
#ifdef	USE_FRAMEBUFFER
extern int use_frame_buffer; /*!< specifies if we use frame buffer or not */

#endif
/*! @} */

// Grum
// necessary for OSX?
#ifndef APIENTRY
#define APIENTRY
#endif

/*! \name Function pointers to GLX implementation functions
 * @{ */
extern void (APIENTRY * ELglMultiTexCoord2fARB) (GLenum target, GLfloat s, GLfloat t);
extern void (APIENTRY * ELglActiveTextureARB) (GLenum texture);
extern void (APIENTRY * ELglClientActiveTextureARB) (GLenum texture);
extern void (APIENTRY * ELglLockArraysEXT) (GLint first, GLsizei count);
extern void (APIENTRY * ELglUnlockArraysEXT) (void);
extern void (APIENTRY * ELglBindBufferARB)(GLenum target, GLuint buffer);
extern void (APIENTRY * ELglGenBuffersARB)(GLsizei no, GLuint *buffer);
extern void (APIENTRY * ELglDeleteBuffersARB)(GLsizei no, const GLuint *buffer);
extern void (APIENTRY * ELglBufferDataARB)(GLenum target, GLsizeiptrARB size, const void * data, GLenum usage);
extern void (APIENTRY * ELglGenRenderbuffersEXT)(GLsizei n, GLuint * renderbuffers);
extern void (APIENTRY * ELglDeleteRenderbuffersEXT)(GLsizei n, const GLuint * renderbuffers);
extern void (APIENTRY * ELglBindRenderbufferEXT)(GLenum target, GLuint renderbuffer);
extern void (APIENTRY * ELglRenderbufferStorageEXT)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
extern void (APIENTRY * ELglGenFramebuffersEXT)(GLsizei n, GLuint * framebuffers);
extern void (APIENTRY * ELglDeleteFramebuffersEXT)(GLsizei n, const GLuint * framebuffers);
extern void (APIENTRY * ELglBindFramebufferEXT)(GLsizei n, GLuint framebuffer);
extern void (APIENTRY * ELglGenProgramsARB)(GLsizei n, GLuint * programs);
extern void (APIENTRY * ELglDeleteProgramsARB)(GLsizei n, const GLuint * programs);
extern void (APIENTRY * ELglBindProgramARB)(GLenum type, GLuint program);
extern void (APIENTRY * ELglProgramStringARB)(GLenum type, GLenum format, GLsizei length, const char * program);
extern GLhandleARB (APIENTRY * ELglCreateShaderObjectARB)(GLenum type);
extern void (APIENTRY * ELglShaderSourceARB)(GLhandleARB shader, GLsizei count, const char ** string, const int * length);
extern void (APIENTRY * ELglCompileShaderARB)(GLhandleARB shader);
extern GLhandleARB (APIENTRY * ELglCreateProgramObjectARB)(void);
extern void (APIENTRY * ELglAttachObjectARB)(GLhandleARB program, GLhandleARB shader);
extern void (APIENTRY * ELglLinkProgramARB)(GLhandleARB program);
extern void (APIENTRY * ELglUseProgramObjectARB)(GLhandleARB program);
#ifdef	TERRAIN
extern GLint (APIENTRY * ELglGetUniformLocationARB)(GLhandleARB program, const char * name);
extern void (APIENTRY * ELglUniform1iARB)(GLint location, GLint v0);
#endif
#ifdef	USE_FRAMEBUFFER
extern GLboolean (APIENTRY * ELglIsRenderbufferEXT) (GLuint renderbuffer);
extern void (APIENTRY * ELglGetRenderbufferParameterivEXT) (GLenum target, GLenum pname, GLint *params);
extern GLboolean (APIENTRY * ELglIsFramebufferEXT) (GLuint framebuffer);
extern GLenum (APIENTRY * ELglCheckFramebufferStatusEXT) (GLenum target);
extern void (APIENTRY * ELglFramebufferTexture1DEXT) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern void (APIENTRY * ELglFramebufferTexture2DEXT) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern void (APIENTRY * ELglFramebufferTexture3DEXT) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
extern void (APIENTRY * ELglFramebufferRenderbufferEXT) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
extern void (APIENTRY * ELglGetFramebufferAttachmentParameterivEXT) (GLenum target, GLenum attachment, GLenum pname, GLint *params);
extern void (APIENTRY * ELglGenerateMipmapEXT) (GLenum target);
#endif
/*! @} */

/*!
 * \ingroup video
 * \brief   initializes the selected video mode
 *
 *      Initializes and sets up the selected video mode
 *
 * \param fs 0 for windowed mode, non-zero for fullscreen
 * \param mode the number of the video mode
 *
 * \sa init_stuff
 * \sa init_video
 */
void setup_video_mode(int fs, int mode);

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
 * \name CHECK_GL_ERRORS macro
 */
/*! @{ */
#ifdef	DEBUG
#define CHECK_GL_ERRORS()	print_gl_errors(__FILE__,  __FUNCTION__, __LINE__)
#else	//DEBUG
#define CHECK_GL_ERRORS()	/*!< NOP */
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
#ifdef	USE_FRAMEBUFFER
#ifndef GL_FRAMEBUFFER_EXT
#define GL_FRAMEBUFFER_EXT				0x8D40
#endif
#ifndef GL_RENDERBUFFER_EXT
#define GL_RENDERBUFFER_EXT				0x8D41
#endif
#ifndef GL_FRAMEBUFFER_COMPLETE_EXT
#define GL_FRAMEBUFFER_COMPLETE_EXT			0x8CD5
#endif
#ifndef GL_COLOR_ATTACHMENT0_EXT
#define GL_COLOR_ATTACHMENT0_EXT			0x8CE0
#endif
#ifndef GL_DEPTH_COMPONENT24
#define GL_DEPTH_COMPONENT24				0x81A6
#endif
#ifndef GL_DEPTH_ATTACHMENT_EXT
#define GL_DEPTH_ATTACHMENT_EXT				0x8D00
#endif
#endif
/*! @} */

#endif
