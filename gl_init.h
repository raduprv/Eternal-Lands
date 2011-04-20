/*!
 * \file
 * \ingroup video
 * \brief OpenGL and video initialization related functions.
 */
#ifndef __GL_INIT_H__
#define __GL_INIT_H__

#include "load_gl_extensions.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \name window dimensions 
 * @{ */
extern int window_width; /*!< width of the window */
extern int window_height; /*!< height of the window */
/*! @} */

extern int bpp; /*!< color depth to use */
extern int video_mode; /*!< currently selected video mode */
extern int video_user_width; /*!< userdefined window width */
extern int video_user_height; /*!< userdefined window height */
extern int disable_window_adjustment; /*<! Switch off window size adjustment for window borders, task bar and the like */
extern int full_screen; /*!< flag that inidicates whether we are in fullscreen or windowed mode */
extern int disable_gamma_adjust;
extern float gamma_var; /*!< The current gamma value */
extern float perspective; /*!< The perspective "angle". Higher values mean higher distortion. Default is 0.15f */
/* near plane not used in FPV. FPV uses fixed near clipping plane of .2 
 */
extern float far_plane; /*!< The distance of the far clipping plane to your actor. */
extern float far_reflection_plane; /*!< The distance of the far clipping plane to your actor. */
extern float near_plane; /*!< The distance of the near clipping plane to your actor (devided by zoom_level). */

/*! \name OpenGL extensions variables 
 * @{ */
extern int have_stencil; /*!< flag that indicates whether we have the stencil extension or not. \todo shouldn't this go to gl_init.h to all the other OpenGL related variables? */
extern int use_vertex_buffers; /*!< specifies if we use vertex buffer objects or not */
extern int use_compiled_vertex_array; /*!< specified if we use compiled vertex array or not */
extern int use_mipmaps; /*!< indicates whether we use mipmaps or not */
extern int use_frame_buffer; /*!< specifies if we use frame buffer or not */
extern int use_draw_range_elements;  /*!< specifies if we use glDrawRangeElements or glDrawElements */
/*! @} */
extern float anisotropic_filter;
extern int gl_extensions_loaded; /*< specifies if the OpenGL extensions were loaded or not */
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
 * \brief   Switch to a different video mode or switch fullscreen state
 *
 * \return  False if the mode is not supported
 *	    True otherwise
 */
int switch_video(int mode, int full_screen);

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
 * \param line  the line in the source file where the error occurred
 * \retval int
 * \callgraph
 */
int print_gl_errors(const char *file, int line);

/*!
 * \name CHECK_GL_ERRORS macro
 */
/*! @{ */
#if defined DEBUG || defined OPENGL_TRACE
#define CHECK_GL_ERRORS()	print_gl_errors(__FILE__, __LINE__)
#else	//DEBUG
#define CHECK_GL_ERRORS()	/*!< NOP */
#endif	//DEBUG
/*! @} */

#ifdef __cplusplus
} // extern "C"
#endif

#endif
