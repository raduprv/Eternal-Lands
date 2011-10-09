#ifndef __GL_INIT_H__
#define __GL_INIT_H__

#include "../platform.h"
#include "../load_gl_extensions.h"

void init_gl(void);
void handle_window_resize(void);

/*! \name OpenGL extensions variables 
 * @{ */
extern int use_vertex_buffers; /*!< specifies if we use vertex buffer objects or not */
extern int have_vertex_buffers; /*!< flag that indicates whether we have access to using vertex buffer objects or not*/
extern int have_texture_non_power_of_two; /*! < flag that indicates whether the GL_ARB_texture_non_power_of_two extension is supported or not*/
/*! @} */

extern int gl_extensions_loaded; /*< specifies if the OpenGL extensions were loaded or not */

/*!
 * \ingroup video
 * \brief   checks and initializes available OpenGL extensions (GLX)
 *
 *      Checks and enables available OpenGL extensions (GLX)
 *
 * \callgraph
 */
void init_gl_extensions();

#endif
