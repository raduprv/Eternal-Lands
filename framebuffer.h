/**
 * @file
 * @ingroup 	display_utils
 * @brief 	Framebuffer utils.
 */
#ifndef	FRAMEBUFFER_H
#define	FRAMEBUFFER_H

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup 	display_utils
 * @brief 	Free framebuffer.
 * 
 * Frees the given frame buffer, render buffer and texture.
 * @param 	fbo The frame buffer.
 * @param 	fbo_depth_buffer The render depth buffer.
 * @param 	fbo_stencil_buffer The render stencil buffer.
 * @param 	fbo_texture The destination texture.
 *  
 * @callgraph
 */
void free_color_framebuffer(GLuint *fbo, GLuint *fbo_depth_buffer, GLuint * fbo_stencil_buffer,
	GLuint *fbo_texture);

/**
 * @ingroup 	display_utils
 * @brief 	Creates a new frame buffer, attachs a new render depth and stencil buffer and a
 * destination texture.
 * 
 * Creates a new frame buffer, attachs a new render depth and stencil buffer and a
 * destination texture.
 * @param	width The width of the texture.
 * @param	height The height of the texture.
 * @param 	fbo The frame buffer.
 * @param 	fbo_depth_buffer The render depth buffer.
 * @param 	fbo_stencil_buffer The render stencil buffer.
 * @param 	fbo_texture The destination texture.
 *  
 * @callgraph
 */
void make_color_framebuffer(int width, int height, GLuint *fbo, GLuint *fbo_depth_buffer,
	GLuint * fbo_stencil_buffer, GLuint *fbo_texture);

/**
 * @ingroup 	display_utils
 * @brief 	Changes the size of the destinatoin texture of a given framebuffer.
 * 
 * Changes the size of the destinatoin texture of a given framebuffer
 * @param	width The width of the texture.
 * @param	height The height of the texture.
 * @param 	fbo The frame buffer.
 * @param 	fbo_depth_buffer The render depth buffer.
 * @param 	fbo_stencil_buffer The render stencil buffer.
 * @param 	fbo_texture The destination texture.
 *  
 * @callgraph
 */
void change_color_framebuffer_size(int width, int height, GLuint *fbo, GLuint *fbo_depth_buffer,
	GLuint * fbo_stencil_buffer, GLuint *fbo_texture);

/**
 * @ingroup 	display_utils
 * @brief 	Free framebuffer.
 * 
 * Frees the given frame buffer and texture.
 * @param 	fbo The frame buffer.
 * @param 	fbo_texture The destination texture.
 *  
 * @callgraph
 */
void free_depth_framebuffer(GLuint *fbo, GLuint *fbo_texture);

/**
 * @ingroup 	display_utils
 * @brief 	Creates a new frame buffer and attachs a new destination texture.
 * 
 * Creates a new frame buffer and attachs a destination texture.
 * @param	width The width of the texture.
 * @param	height The height of the texture.
 * @param 	fbo The frame buffer.
 * @param 	fbo_texture The destination texture.
 *  
 * @callgraph
 */
void make_depth_framebuffer(int width, int height, GLuint *fbo, GLuint *fbo_texture);

/**
 * @ingroup 	display_utils
 * @brief 	Changes the size of the destinatoin texture of a given framebuffer.
 * 
 * Changes the size of the destinatoin texture of a given framebuffer
 * @param	width The width of the texture.
 * @param	height The height of the texture.
 * @param 	fbo The frame buffer.
 * @param 	fbo_texture The destination texture.
 *  
 * @callgraph
 */
void change_depth_framebuffer_size(int width, int height, GLuint *fbo, GLuint *fbo_texture);

/**
 * @ingroup 	display_utils
 * @brief 	Check what formats are supported.
 * 
 * Check what formats are supported and log the result.
 *  
 * @callgraph
 */
void check_fbo_formats();

#ifdef	DEBUG
void print_fbo_errors(const char *file, int line);
#define CHECK_FBO_ERRORS()	print_fbo_errors(__FILE__, __LINE__)
#else	//DEBUG
#define CHECK_FBO_ERRORS()	/**< NOP */
#endif	//DEBUG

#ifdef __cplusplus
} // extern "C"
#endif

#endif
