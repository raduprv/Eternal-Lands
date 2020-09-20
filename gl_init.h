/*!
 * \file
 * \ingroup video
 * \brief OpenGL and video initialization related functions.
 */
#ifndef __GL_INIT_H__
#define __GL_INIT_H__

#include <SDL.h>
#include "load_gl_extensions.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \name window dimensions 
 * @{ */
extern int window_width; /*!< width of the window */
extern int window_height; /*!< height of the window */
/*! @} */

#if !defined(MAP_EDITOR)
extern SDL_Window *el_gl_window; /*!< the sdl window */
#endif

extern int bpp; /*!< color depth to use */
extern int video_mode; /*!< currently selected video mode */
extern int video_user_width; /*!< userdefined window width */
extern int video_user_height; /*!< userdefined window height */
#ifdef WINDOWS
extern int disable_window_adjustment; /*<! Switch off window size adjustment for window borders, task bar and the like */
#endif
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
extern int gl_extensions_loaded; /*!< specifies if the OpenGL extensions were loaded or not */

#ifdef OSX
extern int emulate3buttonmouse;
#endif

#ifdef ANTI_ALIAS
extern int anti_alias; /*!< flag indicating whether anti-aliasing should be enabled */
#endif

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
void init_video(void);

/*!
 * \ingroup video
 * \brief   checks and initializes available OpenGL extensions (GLX)
 *
 *      Checks and enables available OpenGL extensions (GLX)
 *
 * \callgraph
 */
void init_gl_extensions(void);

/*!
 * \ingroup video
 * \brief   resizes the window to the selected mode.
 *
 *      Resizes the window, after selecting a new video mode.
 *
 */
void resize_root_window(void);

/*!
 * \ingroup video
 * \brief   toggles the display from fullscreen to windowed mode and vice versa.
 *
 *      Toggles the display from fullscreen to windowed mode and vice versa.
 *
 * \callgraph
 */
void toggle_full_screen(void);

/*!
 * \name Functions for window size and high-dpi scaling
 */
/*! @{ */
float get_highdpi_scale(void);					/*!< return high-dpi scaling ration - SDL_GL_GetDrawableSize()/SDL_GetWindowSize() */
void highdpi_scale(int *width, int *height);	/*!< multiple the provided values my their high-dpi scaling values */
void update_window_size_and_scale(void);		/*!< sets window width, height and highdpi scale values */
void set_client_window_size(int width, int height); /*!< set the window size as specified without changing the window mode */
/*! @} */

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
 * \name CHECK_GL_ERRORS macro - only done if DEBUG or OPENGL_TRACE defined
 */
/*! @{ */
#if defined DEBUG || defined OPENGL_TRACE
#define CHECK_GL_ERRORS()	print_gl_errors(__FILE__, __LINE__)
#else	//DEBUG
#define CHECK_GL_ERRORS()	/*!< NOP */
#endif	//DEBUG
/*! @} */

/*!
 * \name DO_CHECK_GL_ERRORS macro - always done for client
 */
/*! @{ */
#ifdef ELC
#define DO_CHECK_GL_ERRORS()	print_gl_errors(__FILE__, __LINE__)
#else
#define DO_CHECK_GL_ERRORS()	/*!< NOP */
#endif
/*! @} */

#ifdef __cplusplus
} // extern "C"
#endif

#endif
