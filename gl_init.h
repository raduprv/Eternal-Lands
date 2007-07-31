#ifndef __GL_INIT_H__
#define __GL_INIT_H__

#include "../elc/platform.h"

void init_gl(void);
void handle_window_resize(void);

#ifdef NEW_E3D_FORMAT
/*! \name OpenGL extensions variables 
 * @{ */
extern int use_vertex_buffers; /*!< specifies if we use vertex buffer objects or not */
extern int have_vertex_buffers; /*!< flag that indicates whether we have access to using vertex buffer objects or not*/
extern int have_texture_non_power_of_two; /*! < flag that indicates whether the GL_ARB_texture_non_power_of_two extension is supported or not*/
/*! @} */

extern int gl_extensions_loaded; /*< specifies if the OpenGL extensions were loaded or not */

/*! \name Function pointers to GLX implementation functions
 * @{ */
extern void (APIENTRY * ELglMultiTexCoord2fARB) (GLenum target, GLfloat s, GLfloat t);
extern void (APIENTRY * ELglMultiTexCoord2fvARB) (GLenum target, const GLfloat *v);
extern void (APIENTRY * ELglActiveTextureARB) (GLenum texture);
extern void (APIENTRY * ELglClientActiveTextureARB) (GLenum texture);
extern void (APIENTRY * ELglBindBufferARB)(GLenum target, GLuint buffer);
extern void (APIENTRY * ELglGenBuffersARB)(GLsizei no, GLuint *buffer);
extern void (APIENTRY * ELglDeleteBuffersARB)(GLsizei no, const GLuint *buffer);
extern void (APIENTRY * ELglBufferDataARB)(GLenum target, GLsizeiptrARB size, const void * data, GLenum usage);
extern void (APIENTRY * ELglMultiDrawElementsEXT) (GLenum mode, GLsizei* count, GLenum type, const GLvoid **indices, GLsizei primcount);
extern void (APIENTRY * ELglDrawRangeElementsEXT) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
/*! @} */

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

#endif
