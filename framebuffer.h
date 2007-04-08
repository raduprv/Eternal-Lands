/*!
 * \file
 * \ingroup 	display_utils
 * \brief 	Framebuffer utils.
 */
#ifndef	FRAMEBUFFER_H
#define	FRAMEBUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \ingroup 	display_utils
 * \brief 	Free framebuffer.
 * 
 * Frees the given frame buffer, render buffer and texture.
 * \param 	FBO The frame buffer.
 * \param 	FBORenderBuffer The render buffer.
 * \param 	FBOTexture The destination texture.
 *  
 * \callgraph
 */
void free_color_framebuffer(GLuint *FBO, GLuint *FBORenderBuffer, GLuint *FBOTexture);

/*!
 * \ingroup 	display_utils
 * \brief 	Creates a new frame buffer, attachs a new render buffer and a destination texture.
 * 
 * Creates a new frame buffer, attachs a new render buffer and a destination texture.
 * \param	width The width of the texture.
 * \param	height The height of the texture.
 * \param 	FBO The frame buffer.
 * \param 	FBORenderBuffer The render buffer.
 * \param 	FBOTexture The destination texture.
 *  
 * \callgraph
 */
void make_color_framebuffer(int width, int height, GLuint *FBO, GLuint *FBORenderBuffer, GLuint *FBOTexture);

/*!
 * \ingroup 	display_utils
 * \brief 	Changes the size of the destinatoin texture of a given framebuffer.
 * 
 * Changes the size of the destinatoin texture of a given framebuffer
 * \param	width The width of the texture.
 * \param	height The height of the texture.
 * \param 	FBO The frame buffer.
 * \param 	FBORenderBuffer The render buffer.
 * \param 	FBOTexture The destination texture.
 *  
 * \callgraph
 */
void change_color_framebuffer_size(int width, int height, GLuint *FBO, GLuint *FBORenderBuffer, GLuint *FBOTexture);

/*!
 * \ingroup 	display_utils
 * \brief 	Free framebuffer.
 * 
 * Frees the given frame buffer and texture.
 * \param 	FBO The frame buffer.
 * \param 	FBOTexture The destination texture.
 *  
 * \callgraph
 */
void free_depth_framebuffer(GLuint *FBO, GLuint *FBOTexture);

/*!
 * \ingroup 	display_utils
 * \brief 	Creates a new frame buffer and attachs a new destination texture.
 * 
 * Creates a new frame buffer and attachs a destination texture.
 * \param	width The width of the texture.
 * \param	height The height of the texture.
 * \param 	FBO The frame buffer.
 * \param 	FBOTexture The destination texture.
 *  
 * \callgraph
 */
void make_depth_framebuffer(int width, int height, GLuint *FBO, GLuint *FBOTexture);

/*!
 * \ingroup 	display_utils
 * \brief 	Changes the size of the destinatoin texture of a given framebuffer.
 * 
 * Changes the size of the destinatoin texture of a given framebuffer
 * \param	width The width of the texture.
 * \param	height The height of the texture.
 * \param 	FBO The frame buffer.
 * \param 	FBOTexture The destination texture.
 *  
 * \callgraph
 */
void change_depth_framebuffer_size(int width, int height, GLuint *FBO, GLuint *FBOTexture);


#ifdef __cplusplus
} // extern "C"
#endif

#endif
